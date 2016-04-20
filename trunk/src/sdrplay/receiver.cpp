/**
* @file receiver.c
* @brief manage client attachment to receivers
* @author John Melton, G0ORX/N6LYT
* @version 0.1
* @date 2009-10-13
*/


/* Copyright (C)
* 2009 - John Melton, G0ORX/N6LYT
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <math.h>

#include "messages.h"
#include "client.h"
#include "receiver.h"

#define SMALL_PACKETS

RECEIVER receiver[MAX_RECEIVERS];
static int iq_socket;
static struct sockaddr_in iq_addr;
static int iq_length;

static char response[80];

static unsigned long sequence=0L;

char *error_string(int r) {
  switch(r) {
  case mir_sdr_Success: return (char *)"Success";
  case mir_sdr_Fail: return (char *)"Fail";
  case mir_sdr_InvalidParam: return (char *)"InvalidParam";
  case mir_sdr_OutOfRange: return (char *)"OutOfRange";
  case mir_sdr_GainUpdateError: return (char *)"GainUpdateError";
  case mir_sdr_RfUpdateError: return (char *)"RfUpdateError";
  case mir_sdr_FsUpdateError: return (char *)"FsUpdateError";
  case mir_sdr_HwError: return (char *)"HwError";
  case mir_sdr_AliasingError: return (char *)"AliasingError";
  case mir_sdr_AlreadyInitialised: return (char *)"AlreadyInitialised";
  case mir_sdr_NotInitialised: return (char *)"NotInitialised";
  }
  return (char *)"undefined error value";
}

int translateGain(int gain, RECEIVER *pRec) {
  if (gain < 0 || gain > pRec->maxGain-pRec->minGain) return -1;
  pRec->gRdB = gain;
  pRec->gain_dB = pRec->maxGain - pRec->gRdB;
  return 0;
}

int translateFreq(long freq, RECEIVER *pRec) {
  if (freq < 100000 || freq > 2000000000) return -1;
  pRec->rfHz = double(freq);
  return 0;
}  

// all the sample rates that work with the ozy protocol
// are decimations of hardware sample rates that SDRplay implements
int translateSR(int sr, RECEIVER *pRec) {
  switch (sr) {
  case 24000:			// m = 64
  case 48000:			// m = 32
  case 96000:			// m = 16
  case 192000:			// m = 8
  case 384000:			// m = 4
  case 768000:			// m = 2
  case 1536000:			// m = 1
    pRec->m = 1536000 / sr;
    pRec->fsHz = double(sr * pRec->m);
    return 0;
  case 22050:			// m = 64
  case 44100:			// m = 32
  case 88200:			// m = 16
  case 176400:			// m = 8
  case 352800:			// m = 4
  case 705600:			// m = 2
  case 1411200:			// m = 1
    pRec->m = 1411200 / sr;
    pRec->fsHz = double(sr * pRec->m);
    return 0;
  }
  return -1;
}

int translateBW(int bw, RECEIVER *pRec) {
  switch (bw) {
  case 200000:  pRec->bwType = mir_sdr_BW_0_200; break;
  case 300000:  pRec->bwType = mir_sdr_BW_0_300; break;
  case 600000:  pRec->bwType = mir_sdr_BW_0_600; break;
  case 1536000: pRec->bwType = mir_sdr_BW_1_536; break;
  case 5000000: pRec->bwType = mir_sdr_BW_5_000; break;
  case 6000000: pRec->bwType = mir_sdr_BW_6_000; break;
  case 7000000: pRec->bwType = mir_sdr_BW_7_000; break;
  case 8000000: pRec->bwType = mir_sdr_BW_8_000; break;
  default:
    return -1;
  }
  return 0;
}

int translateIF(int ift, RECEIVER *pRec) {
  switch (ift) {
  case 0: pRec->ifType = mir_sdr_IF_Zero; break;
  case 450: pRec->ifType = mir_sdr_IF_0_450; break;
  case 1620: pRec->ifType = mir_sdr_IF_1_620; break;
  case 2048: pRec->ifType = mir_sdr_IF_2_048; break;
  default: return -1;
  }
  return 0;
}

/* min rf, max rf, min dB, max dB */
band_desc bands[] = {
  {     100000,   11999999, -4,  98 },
  {   12000000,   29999999, -4,  98 },
  {   30000000,   59999999, -4,  98  },
  {   60000000,  119999999,  1, 103 },
  {  120000000,  249999999,  5, 107 },
  {  250000000,  419999999,  9,  94 },
  {  420000000,  999999999,  9,  94 },
  { 1000000000, 2000000000, 24, 105 }
};

band_desc *find_band(int rfHz) {
  for (unsigned int i = 0; i < sizeof(bands)/sizeof(bands[0]); i += 1)
    if (rfHz >= bands[i].minHz && rfHz <= bands[i].maxHz)
      return &bands[i];
  return NULL;
}

void set_gain_limits(RECEIVER *pRec) {
  band_desc *bp = find_band(pRec->rfHz);
  if (bp != NULL) {
    pRec->minGain = bp->minGain;
    pRec->maxGain = bp->maxGain;
  }
}

int get_sample_rate(RECEIVER *pRec) {
  return (int)(pRec->fsHz / pRec->m);
}

int init_receivers (SdrPlayConfig *pCfg) 
{

  for(int i=0;i<MAX_RECEIVERS;i++) {
    receiver[i].client  = (CLIENT*)NULL;
    receiver[i].connected = 0;
    receiver[i].samples = 0; 

    receiver[i].fsHz = 2048e3;
    receiver[i].rfHz = 200e6;
    receiver[i].bwType = mir_sdr_BW_1_536;
    receiver[i].ifType = mir_sdr_IF_Zero;
    receiver[i].samplesPerPacket = 0;
    receiver[i].dcMode = 0;
    receiver[i].gRdB = 80;
    set_gain_limits(&receiver[i]);
    receiver[i].gain_dB = receiver[i].maxGain - receiver[i].gRdB;

    // validate configuration
    if (translateFreq(pCfg->freq, &receiver[i]) < 0 ||
	translateGain(pCfg->gain, &receiver[i]) < 0 ||
	translateSR(pCfg->sr, &receiver[i]) < 0 ||
	translateBW(pCfg->bw, &receiver[i]) < 0 ||
	translateIF(pCfg->ift, &receiver[i]) < 0)
      return -1;
  }

  // verify API version
  float ver;
  if (mir_sdr_ApiVersion(&ver) != mir_sdr_Success || ver != MIR_SDR_API_VERSION) {
    fprintf(stdout, "Mirics API version mismatch\n");
    return -1;
  }

  // verify hardware
  RECEIVER *pRec = &receiver[0];	// there is only one
  int r = mir_sdr_Init(pRec->gRdB, pRec->fsHz / 1e6, pRec->rfHz / 1e6, pRec->bwType, pRec->ifType, &pRec->samplesPerPacket);

  if ( r != mir_sdr_Success)  {
    fprintf(stdout, "No SDRPlay hardware detected: %s\n", error_string(r));
    return -1;
  }

  mir_sdr_Uninit();

  // set up other resources
  iq_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
  if(iq_socket<0) {
    perror("create socket failed for iq_socket");
    exit(1);
  }

  iq_length=sizeof(iq_addr);
  memset(&iq_addr,0,iq_length);
  iq_addr.sin_family=AF_INET;
  iq_addr.sin_addr.s_addr=htonl(INADDR_ANY);
  iq_addr.sin_port=htons(11002);

  if(bind(iq_socket,(struct sockaddr*)&iq_addr,iq_length)<0) {
    perror("bind socket failed for iq socket");
    exit(1);
  }

  return 0;
}

const char* attach_receiver(int rx, CLIENT* client) 
{
    if(client->state==RECEIVER_ATTACHED) {
        return CLIENT_ATTACHED;
    }

    if(receiver[rx].client!=(CLIENT *)NULL) {
        return RECEIVER_IN_USE;
    }
    
    client->state=RECEIVER_ATTACHED;
    receiver[rx].client=client;
    client->receiver=rx;

    sprintf(response,"OK %d", get_sample_rate(&receiver[rx]));

    return response;
}

const char* detach_receiver (int rx, CLIENT* client) {
    if(client->state==RECEIVER_DETACHED) {
        return CLIENT_DETACHED;
    }

    if(receiver[rx].client!=client) {
        return RECEIVER_NOT_OWNER;
    }
    fprintf(stderr, "detach_receiver: ...");

    client->state=RECEIVER_DETACHED;
    receiver[rx].client = (CLIENT*)NULL;

    return OK;
}

// can write a command if the incremented write pointer is not equal to the read pointer
int can_write_command(RECEIVER *pRec) {
  return ((pRec->command_write_index+1)%NCOMMANDS) != pRec->command_read_index;
}
void do_write_command(RECEIVER *pRec, int cmd) {
  if ( ! can_write_command(pRec)) {
    fprintf(stderr, "attempting to write a non-existent command into circular buffer\n");
    exit(1);
  }
  command *cp = pRec->commands+pRec->command_write_index;
  cp->cmd = cmd;
  cp->gr = pRec->gRdB;		// most of these may not apply
  cp->fs = pRec->fsHz;
  cp->rf = pRec->rfHz;
  cp->bw = pRec->bwType;
  cp->ift = pRec->ifType;
  pRec->command_write_index = (pRec->command_write_index+1)%NCOMMANDS;
}
// can read a command if the read pointer does not equal the write pointer
int can_read_command(RECEIVER *pRec) {
  return pRec->command_read_index != pRec->command_write_index;
}
void do_read_command(RECEIVER *pRec) {
  if ( ! can_read_command(pRec)) {
    fprintf(stderr, "attempting to read a non-existent command from circular buffer\n");
    exit(1);
  }
  command *cp = pRec->commands+pRec->command_read_index;
  switch (cp->cmd) {
  case CMD_INIT: {
    if ( ! pRec->connected) {
      int r = mir_sdr_Init(cp->gr, double(cp->fs) / 1.0e6, double(cp->rf) / 1.0e6, mir_sdr_Bw_MHzT(cp->bw), mir_sdr_If_kHzT(cp->ift), &pRec->samplesPerPacket);
      if (r != 0) {
	fprintf(stdout, "mir_sdr_Init returned %d: %s\n", r, error_string(r));
	exit(1);
      }
      if (pRec->dcMode) mir_sdr_SetDcMode(4,1);
      // reset decimation
      pRec->mi = 0; pRec->mxi = 0; pRec->mxq = 0;
      pRec->connected = 1;
    }
    break;
  }
  case CMD_UNINIT: {
    if (pRec->connected) {
      int r = mir_sdr_Uninit();
      if (r != 0) fprintf(stderr, "WARNING: Failed to stop: %d: %s.\n", r, error_string(r));
      pRec->connected = 0;
    }
    break;
  }
  case CMD_SETRF: {
    if (pRec->connected) {
      int r = mir_sdr_SetRf(double(cp->rf)/1.0, 1, 0);
      if (r != 0) fprintf(stderr, "WARNING: Tuning error: %d: %s.\n", r, error_string(r));
    }
    break;
  }
  case CMD_SETGR: {
    if (pRec->connected) {
      int r = mir_sdr_SetGr(cp->gr, 1, 0);

      if (r != 0) fprintf(stderr, "WARNING: Failed to set tuner gain: %d.\n", r);
    }
    break;
  }
  }
  pRec->command_read_index = (pRec->command_read_index+1)%NCOMMANDS;
}

void start_receiver(RECEIVER *pRec) {
  do_write_command(pRec, CMD_INIT);
}

void stop_receiver(RECEIVER *pRec) {
  do_write_command(pRec, CMD_UNINIT);
}

const char* set_frequency (CLIENT* client, long frequency) {
  if(client->state==RECEIVER_DETACHED) return CLIENT_DETACHED;
  if(client->receiver<0) return RECEIVER_INVALID;

  RECEIVER *pRec = &receiver[client->receiver];
  int f = int(frequency);

  if (f == pRec->rfHz) return OK;

  band_desc *bp = find_band(int(frequency));

  if (bp == pRec->band) {
    pRec->rfHz = f;
    do_write_command(pRec, CMD_SETRF);
  } else {
    pRec->rfHz = f;
    pRec->band = bp;
    set_gain_limits(pRec);
    stop_receiver(pRec);
    start_receiver(pRec);
  }
  return OK;
}

const char* set_preamp (CLIENT* client, bool preamp)
{
    return NOT_IMPLEMENTED_COMMAND;
}

const char* set_dither (CLIENT* client, bool dither)
{
    return NOT_IMPLEMENTED_COMMAND;
}

const char* set_random (CLIENT* client, bool)
{
    return NOT_IMPLEMENTED_COMMAND;
}

const char* set_attenuator (CLIENT* client, int new_level_in_dB)
{
  if(client->state==RECEIVER_DETACHED) return CLIENT_DETACHED;
  if(client->receiver<0) return RECEIVER_INVALID;
  RECEIVER *pRec = &receiver[client->receiver];
  if (pRec->gRdB != new_level_in_dB) {
    pRec->gRdB = new_level_in_dB;
    pRec->gain_dB = pRec->maxGain - new_level_in_dB;
    do_write_command(pRec, CMD_SETGR);
  }
  return OK;
}

void send_IQ_buffer (RECEIVER *pRec) {
    struct sockaddr_in client;
    int client_length;
    unsigned short offset;
    BUFFER buffer;
    int rc;

    if(pRec->client != (CLIENT*)NULL) {
        if(pRec->client->iq_port != -1) {
            // send the IQ buffer

            client_length = sizeof(client);
            memset((char*)&client,0,client_length);
            client.sin_family = AF_INET;
            client.sin_addr.s_addr = pRec->client->address.sin_addr.s_addr;
            client.sin_port = htons(pRec->client->iq_port);

#ifdef SMALL_PACKETS
            // keep UDP packets to 512 bytes or less
            //     8 bytes sequency number
            //     2 byte offset
            //     2 byte length
            offset=0;
            while(offset<sizeof(pRec->input_buffer)) {
                buffer.sequence=sequence;
                buffer.offset=offset;
                buffer.length=sizeof(pRec->input_buffer)-offset;
                if(buffer.length>500) buffer.length=500;
                memcpy ((char*)&buffer.data[0], (char*)&(pRec->input_buffer[offset/4]), buffer.length);
                rc = sendto (iq_socket, (char*)&buffer, sizeof(buffer), 0, (struct sockaddr*)&client,client_length);
                if(rc<=0) {
                    perror("sendto failed for iq data");
                    exit(1);
                } 
                //else {
                //    fprintf (stderr, "%s: sending packet to %s.\n", __FUNCTION__, inet_ntoa(client.sin_addr));
                //}
                offset+=buffer.length;
            }
            sequence++;

            
#else
            rc=sendto(iq_socket,pRec->input_buffer,sizeof(pRec->input_buffer),0,(struct sockaddr*)&client,client_length);
            if(rc<=0) {
                perror("sendto failed for iq data");
                exit(1);
            }
#endif
 
        }
    }
}

int read_IQ_buffer(RECEIVER *pRec) {
  unsigned int firstSampleNumb;
  int grChanged, rfChanged, fsChanged, r;
  for (int i = 0; i < SDRPLAY_NUM_PACKETS; i += 1) {
    int offset = i*pRec->samplesPerPacket;
    r = mir_sdr_ReadPacket(pRec->i_buffer+offset, pRec->q_buffer+offset, &firstSampleNumb, &grChanged, &rfChanged, &fsChanged);
    if ( r != 0 ) {
      fprintf(stderr, " !!!!!!!!! read packet error: [%d] %s\n", r, error_string(r) );
      return r;
    }
  }
  return r;
}
