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

int translateGain(RECEIVER *pRec) {
  if (pRec->cfg.gain < -1 || pRec->cfg.gain > 85) return -1;
  pRec->gRdB = pRec->cfg.gain;
  return 0;
}

int translateFreq(RECEIVER *pRec) {
  if (pRec->cfg.freq < 100000 || pRec->cfg.freq > 2000000000) return -1;
  pRec->rfMHz = pRec->cfg.freq / 1.0e6;
  return 0;
}  

int translateSR(RECEIVER *pRec) {
  switch (pRec->cfg.sr) {
  case 24000:			// m = 64
  case 48000:			// m = 32
  case 96000:			// m = 16
  case 192000:			// m = 8
  case 384000:			// m = 4
  case 768000:			// m = 2
  case 1536000:			// m = 1
    pRec->m = 1536000 / pRec->cfg.sr;
    pRec->fsMHz = double(pRec->cfg.sr * pRec->m) / 1.0e6;
    return 0;
  case 22050:			// m = 64
  case 44100:			// m = 32
  case 88200:			// m = 16
  case 176400:			// m = 8
  case 352800:			// m = 4
  case 705600:			// m = 2
  case 1411200:			// m = 1
    pRec->m = 1411200 / pRec->cfg.sr;
    pRec->fsMHz = double(pRec->cfg.sr * pRec->m) / 1.0e6;
    return 0;
  }
  return -1;
}

int translateBW(RECEIVER *pRec) {
  switch (pRec->cfg.bw) {
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

int translateIF(RECEIVER *pRec) {
  switch (pRec->cfg.ift) {
  case 0: pRec->ifType = mir_sdr_IF_Zero; break;
  case 450: pRec->ifType = mir_sdr_IF_0_450; break;
  case 1620: pRec->ifType = mir_sdr_IF_1_620; break;
  case 2048: pRec->ifType = mir_sdr_IF_2_048; break;
  default: return -1;
  }
  return 0;
}

int init_receivers (SdrPlayConfig *pCfg) 
{

  for(int i=0;i<MAX_RECEIVERS;i++) {
    receiver[i].client  = (CLIENT*)NULL;
    receiver[i].samples = 0; 
    receiver[i].cfg = *pCfg;
    receiver[i].i_buffer = NULL;
    receiver[i].q_buffer = NULL;
    receiver[i].samplesPerPacket = 0;
    receiver[i].connected = 0;
    // validate configuration
    if (translateGain(&receiver[i]) < 0 ||
	translateFreq(&receiver[i]) < 0 ||
	translateSR(&receiver[i]) < 0 ||
	translateBW(&receiver[i]) < 0 ||
	translateIF(&receiver[i]) < 0)
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
  int samplesPerPacket;
  int r = mir_sdr_Init(pRec->gRdB, pRec->fsMHz, pRec->rfMHz, pRec->bwType, pRec->ifType, &samplesPerPacket);

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

    sprintf(response,"OK %d", receiver[rx].cfg.sr);
    fprintf(stdout, "response to attach_receiver: %s\n", response);

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

// Megahertz of band edges, init sets rf front end,
// frequency can only move around inside the band edges
// (but that doesn't make sense, could tune outside the edges
//  and you'd just start dealing more with the filter skirts)
// In any case, the band edges get set by init and you need to
// uninit and re-init to change to a different band.

double band_breaks[] = {
  0.100, 12.0, 30.0, 60.0, 120.0, 250.0, 420.0, 1000.0, 2000.0
};
#define N_BAND_BREAKS ((int)(sizeof(band_breaks)/sizeof(band_breaks[0])))

int find_frequency_band(double fMHz) {
  for (int i = 0; i < N_BAND_BREAKS-1; i += 1)
    if (fMHz >= band_breaks[i] && fMHz < band_breaks[i+1])
      return i;
  return -1;
}

const char* set_frequency (CLIENT* client, long frequency) {
      
    if(client->state==RECEIVER_DETACHED) {
        return CLIENT_DETACHED;
    }

    if(client->receiver<0) {
        return RECEIVER_INVALID;
    }

    RECEIVER *pRec = &receiver[client->receiver];
    double fMHz = double(frequency)/1.0e6;
    int frequency_band = find_frequency_band(fMHz);

    if (frequency_band < 0) {
      fprintf (stderr, "%s: %ld is out of bounds\n", __FUNCTION__, pRec->frequency);
      return OK;
    }

    if (pRec->frequency_did_change == 0) {
      fprintf(stderr, "%s: waiting on previous frequency change\n", __FUNCTION__);
      return OK;
    }

    if (pRec->frequency != frequency) {
      if (pRec->frequency_band != frequency_band) {
	stop_receiver(pRec);
	pRec->cfg.freq = frequency;
	pRec->rfMHz = fMHz;
	start_receiver(pRec);
      } else {
	mir_sdr_SetRf((frequency-pRec->frequency)/1.0e6, 0, 0);
	pRec->frequency_did_change = 0;
      }
      pRec->frequency=frequency;
      pRec->frequency_band=frequency_band;
      pRec->frequency_changed=1;
      fprintf (stderr, "%s: %ld\n", __FUNCTION__, receiver[client->receiver].frequency);
    }
    return OK;
}

const char* set_preamp (CLIENT* client, bool preamp)
{
//  receiver[client->receiver].ppc->preamp = preamp;
//
    return NOT_IMPLEMENTED_COMMAND;
    return OK;
}

const char* set_dither (CLIENT* client, bool dither)
{
//  receiver[client->receiver].ppc->dither = dither;
//
    return NOT_IMPLEMENTED_COMMAND;
    return OK;
}

const char* set_random (CLIENT* client, bool)
{
    return NOT_IMPLEMENTED_COMMAND;
    return OK;
}

const char* set_attenuator (CLIENT* client, int new_level_in_db)
{
    // FIX.ME rec - gain reduction vs gain?
    int r = mir_sdr_SetGr(new_level_in_db, 1, 0);

    if (r != 0)
      fprintf(stderr, "WARNING: Failed to set tuner gain: %d.\n", r);
    else {
      receiver[client->receiver].gRdB = new_level_in_db;
      fprintf(stderr, "Tuner gain reduction set to %d dB.\n", new_level_in_db);
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

void start_receiver(RECEIVER *pRec) {
  int samplesPerPacket;

  int r = mir_sdr_Init(pRec->gRdB, pRec->fsMHz, pRec->rfMHz, pRec->bwType, pRec->ifType, &samplesPerPacket);

  if (r != 0) {
    fprintf(stdout, "mir_sdr_Init returned %d: %s\n", r, error_string(r));
    exit(1);
  }

  pRec->frequency = pRec->cfg.freq;
  pRec->frequency_band = find_frequency_band(pRec->rfMHz);
  pRec->frequency_did_change = 1;
  
  mir_sdr_SetDcMode(4,0);
  mir_sdr_SetDcTrackTime(63);

  if (samplesPerPacket != pRec->samplesPerPacket) {
    if (pRec->i_buffer != NULL) {
      free(pRec->i_buffer);
      free(pRec->q_buffer);
    }
    pRec->i_buffer = (short *)malloc(samplesPerPacket * sizeof(short));
    pRec->q_buffer = (short *)malloc(samplesPerPacket * sizeof(short));
    pRec->samplesPerPacket = samplesPerPacket;
    // reset partial decimation
    pRec->mi = 0; pRec->mxi = 0; pRec->mxq = 0;
  }
  pRec->connected = 1;
}

void stop_receiver(RECEIVER *pRec) {
  int r = mir_sdr_Uninit();
  if (r != 0)
    fprintf(stderr, "WARNING: Failed to stop: %d: %s.\n", r, error_string(r));
  else
    fprintf(stderr, "SDRplay stopped\n");
  pRec->connected = 0;
}

int read_IQ_buffer(RECEIVER *pRec) {
  unsigned int firstSampleNumb;
  int grChanged, rfChanged, fsChanged;
  int r = mir_sdr_ReadPacket(pRec->i_buffer, pRec->q_buffer, &firstSampleNumb, &grChanged, &rfChanged, &fsChanged);
  if (rfChanged)
    pRec->frequency_did_change = 1;
  if ( r != 0 ) {
    fprintf(stderr, " !!!!!!!!! read packet error: [%d] %s\n", r, error_string(r) );
  }
  return r;
}
