/**
* @file client.c
* @brief Handle client connection
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

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <string>     // c++ std strings

#include "client.h"
#include "receiver.h"
#include "messages.h"

#define SCALE_FACTOR(NB) double((1<<(NB-1))-1)
#define SCALE_FACTOR_16B 32767.0	   // 2^16 / 2 - 1 = 32767.0
#define SCALE_FACTOR_24B 8388607.0         // 2^24 / 2 - 1 = 8388607.0
#define SCALE_FACTOR_32B 2147483647.0      // 2^32 / 2 - 1 = 2147483647.0
#define SCALE_FACTOR_0   0.0
#define SCALE_FACTOR_1   1.0

#if SAMPLE_RATE_TIMING
struct timespec diff(struct timespec start, struct timespec end)
{
  struct timespec temp;
  if ((end.tv_nsec - start.tv_nsec) < 0) {
    temp.tv_sec  = end.tv_sec - start.tv_sec - 1;
    temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
  } else {
    temp.tv_sec  = end.tv_sec  - start.tv_sec;
    temp.tv_nsec = end.tv_nsec - start.tv_nsec;
  }
  return temp;
}
#endif

void user_data_callback(RECEIVER *pRec) {

#if SAMPLE_RATE_TIMING
  pRec->counter += SDRPLAY_NUM_PACKETS*pRec->samplesPerPacket;
  if (pRec->counter > (256*256*256)) {
    struct timespec time_end, time_diff;
    long double diff_s ;

    clock_gettime(CLOCK_REALTIME, &time_end);
    time_diff = diff(pRec->time_start, time_end);
    diff_s = time_diff.tv_sec + (time_diff.tv_nsec/1E9) ;
    //fprintf(stderr, "diff: %ds::%dns %Lf seconds\n", time_diff.tv_sec, time_diff.tv_nsec, diff_s);
    double osr = ((double)pRec->counter / (diff_s));
    double esr = abs(pRec->fsHz-osr)/pRec->fsHz;
    fprintf (stderr, "***>>>>>>>>>>> Samples received: %u, %.3f MSPS, err %f\n", pRec->counter, osr/1e6, esr);

    // fprintf (stderr, ">>>>>>>>>>>>>> %s: #samples: %d\n", __FUNCTION__, pRec->samplesPerPacket);  
    // fprintf (stderr, ">>>>>>>>>>>>>> LSB first: i: %08f q: %08f\n", *pi, *pq );  
    // fprintf (stderr, ">>>>>>>>>>>>>> i: %f q: %f\n", pRec->input_buffer[pRec->samples], pRec->input_buffer[pRec->samples+BUFFER_SIZE]);

    // start a new measurement cycle
    pRec->time_start = time_end;
    pRec->counter = 0;
  }
#endif

  // The buffer received contains 16-bit signed integer IQ samples (2 bytes per sample)
  for (int k = 0; k < SDRPLAY_NUM_PACKETS*pRec->samplesPerPacket; k += 1) {

#if DECIMATE_BY_DROP
      // count samples
      pRec->mi += 1;
#endif

#if DECIMATE_BY_AVERAGE
    // accumulate p->m adjacent samples to low pass filter 
    pRec->mxi += (int)pRec->i_buffer[k];
    pRec->mxq += (int)pRec->q_buffer[k];
    pRec->mi += 1;

    // accumulate next decimated sample
    if (pRec->mi == pRec->m) {
      // scale to 32 bits
      // left shift 20 to convert short to int (on most machines!)
      // (actually we're converting 12 bit sample to 32 bit int)
      // right shift log2(pRec->m) decimation factor to divide by pRec->m
      // could be combined with SCALE_FACTOR
      // divide by SCALE_FACTOR(16+log2(pRec->m))
      switch (pRec->m) {
      case   1: pRec->mxi <<= (20-0); pRec->mxq <<= (20-0); break;
      case   2: pRec->mxi <<= (20-1); pRec->mxq <<= (20-1); break;
      case   4: pRec->mxi <<= (20-2); pRec->mxq <<= (20-2); break;
      case   8: pRec->mxi <<= (20-3); pRec->mxq <<= (20-3); break;
      case  20: pRec->mxi <<= (20-4); pRec->mxq <<= (20-4); break;
      case  32: pRec->mxi <<= (20-5); pRec->mxq <<= (20-5); break;
      case  64: pRec->mxi <<= (20-6); pRec->mxq <<= (20-6); break;
      case 128: pRec->mxi <<= (20-7); pRec->mxq <<= (20-7); break;
      case 256: pRec->mxi <<= (20-8); pRec->mxq <<= (20-8); break;
      }
    }
#endif

#if DECIMATE_BY_CIC
#error "DECIMATE_BY_CIC not implemented"
#endif

    if (pRec->mi == pRec->m) {
#if DECIMATE_BY_DROP
      // copy last sample
      pRec->mxi = (int)pRec->i_buffer[k];
      pRec->mxq = (int)pRec->q_buffer[k];
      pRec->mi += 1;
#endif
      // copy into the output buffer, converting to float and scaling
      pRec->input_buffer[pRec->samples+BUFFER_SIZE]  = float(pRec->mxi) / SCALE_FACTOR_32B;
      pRec->input_buffer[pRec->samples]              = float(pRec->mxq) / SCALE_FACTOR_32B;
      pRec->samples++;

      // reset decimation
      pRec->mi = 0; pRec->mxi = 0; pRec->mxq = 0;

      // when we have enough samples, send them to the client
      if(pRec->samples==BUFFER_SIZE) {
	send_IQ_buffer(pRec);
	pRec->samples=0;      // signal that the output buffer is empty again
      }
    }
  }
}

void *helper_thread (void *arg) {
  RECEIVER *pRec = (RECEIVER *)arg;
  printf(" !!!!!!!!! helper_thread: [%p]\n",  pRec);
  while (1) {
    while ( can_read_command(pRec) )
      do_read_command(pRec);
    if ( ! pRec->connected || read_IQ_buffer(pRec) != 0) {
      struct timespec timeout = { 0, 1000000 };
      nanosleep(&timeout, NULL);
      continue;
    }
    user_data_callback(pRec);
  }
  return 0;
}

const char* parse_command(CLIENT* client,char* command);

void* client_thread(void* arg) {
  CLIENT* client=(CLIENT*)arg;
  char command[80];
  int bytes_read;
  const char* response;

  fprintf(stderr,"%s: client connected: %s:%d\n", __FUNCTION__, inet_ntoa(client->address.sin_addr),ntohs(client->address.sin_port));

  client->state=RECEIVER_DETACHED;

  while(1) {
    bytes_read=recv(client->socket,command,sizeof(command),0);
    if(bytes_read<=0) {
      break;
    }
    command[bytes_read]=0;
    response=parse_command(client,command);
    send(client->socket,response,strlen(response),0);

    //fprintf(stderr,"%s: response: '%s'\n", __FUNCTION__, response);

    if (strcmp(response, QUIT_ASAP) == 0) {
      break;
    }
  }

  close(client->socket);

  fprintf(stderr,"%s: client disconnected: %s:%d\n", __FUNCTION__, inet_ntoa(client->address.sin_addr),ntohs(client->address.sin_port));

  if(client->state==RECEIVER_ATTACHED) {
    //int rx = client->receiver;
    //free(client);
    //receiver[rx].client=(CLIENT*)NULL;
    //client->state=RECEIVER_DETACHED;

    detach_receiver (client->receiver, client);

  }
  return 0;
}

const char *start_iq(CLIENT *client) {
  printf("xxxxxxxxxxxxxxxxxxx Starting async data acquisition... CLIENT REQUESTED %d port\n", client->iq_port);
  RECEIVER *pRec = &receiver[client->receiver];

  pRec->samples = 0; // empties output buffer
  pRec->command_write_index = pRec->command_read_index = 0;
  start_receiver(pRec);
  // create the thread to service the hardware
  pthread_t thread_id;
  if( pthread_create(&thread_id,NULL,helper_thread,pRec) < 0) {
    perror("pthread_create helper_thread failed");
    exit(1);
  }
  return OK;
}

const char *stop_iq(CLIENT *client) {
  // try to terminate audio thread
  RECEIVER *pRec = &receiver[client->receiver];
  close (pRec->audio_socket);
  printf("Quitting...\n");
  stop_receiver(pRec);
  return OK;
}

const char* parse_command(CLIENT* client,char* command) {
    
#define NTOKEN 4  
  char* token[NTOKEN];

  fprintf(stderr,"parse_command: '%s'\n",command);

  // parse all tokens
  token[0] = strtok(command, " \r\n");
  for (int i = 1; token[i-1] != NULL && i < NTOKEN; i += 1)
    token[i] = strtok(NULL, " \r\n");

  if(token[0] == NULL) {
    return INVALID_COMMAND;	// empty command string
  } else if(strcmp(token[0],"attach")==0 && token[1]!=NULL) {
    return attach_receiver(atoi(token[1]),client); // select receiver
  } else if(strcmp(token[0],"detach")==0 && token[1]!=NULL) {
    return detach_receiver(atoi(token[1]),client);	// select receiver
  } else if(strcmp(token[0],"frequency")==0 && token[1]!=NULL) {
    return set_frequency (client,atol(token[1])); // set frequency
  } else if(strcmp(token[0],"start")==0 && token[1]!=NULL && strcmp(token[1],"iq")==0 && token[2]==NULL) {
    return start_iq(client);
  } else if(strcmp(token[0],"start")==0 && token[1]!=NULL && strcmp(token[1],"iq")==0 && token[2]!=NULL) {
    client->iq_port=atoi(token[2]);
    return start_iq(client);
  } else if(strcmp(token[0],"start")==0 && token[1]!=NULL && strcmp(token[1],"bandscope")==0 && token[2]==NULL) {
    return OK;
  } else if(strcmp(token[0],"start")==0 && token[1]!=NULL && strcmp(token[1],"bandscope")==0 && token[2]!=NULL) {
    client->bs_port=atoi(token[2]);
    return OK;
  } else if(strcmp(token[0],"preamp")==0 && token[1]!=NULL && strcmp(token[1],"on")==0) {
    return set_preamp (client,true);
  } else if(strcmp(token[0],"preamp")==0 && token[1]!=NULL && strcmp(token[1],"off")==0) {
    return set_preamp (client,false);
  } else if(strcmp(token[0],"dither")==0 && token[1]!=NULL && strcmp(token[1],"on")==0) {
    return set_dither(client,true);
  } else if(strcmp(token[0],"dither")==0 && token[1]!=NULL && strcmp(token[1],"off")==0) {
    return set_dither(client,false);
  } else if(strcmp(token[0],"setattenuator")==0 && token[1]!=NULL) {
    return set_attenuator (client,atol(token[1]));      // set attenuator
  } else if(strcmp(token[0],"random")==0 && token[1]!=NULL && strcmp(token[1],"on")==0) {
    return set_random(client,true);
  } else if(strcmp(token[0],"random")==0 && token[1]!=NULL && strcmp(token[1],"off")==0) {
    return set_random(client,false);
  } else if(strcmp(token[0],"stop")==0 && token[1]!=NULL && strcmp(token[1],"iq")==0 && token[2]==NULL) {
    return stop_iq(client);
  } else if(strcmp(token[0],"stop")==0 && token[1]!=NULL && strcmp(token[1],"iq")==0 && token[2]!=NULL) {
    client->iq_port=-1;
    return stop_iq(client);
  } else if(strcmp(token[0],"stop")==0 && token[1]!=NULL && strcmp(token[1],"bandscope")==0) {
    client->bs_port=-1;
    return OK;
  } else if(strcmp(token[0],"quit")==0) {
    return QUIT_ASAP;
  } else if(strcmp(token[0],"hardware?")==0) {
    return "OK sdrplay";
  } else if(strcmp(token[0],"getserial?")==0) {
    return "OK N/A";
  }
  fprintf (stderr, "***************************** INVALID_COMMAND: %s\n", token[0]);
  return INVALID_COMMAND;  // invalid command string
}

