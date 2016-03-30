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

void user_data_callback(RECEIVER *pRec, short *xi, short *xq) {

  // The buffer received contains 16-bit signed integer IQ samples (2 bytes per sample)
  for (int k = 0; k < pRec->samplesPerPacket; k += 1) {

#if SAMPLE_RATE_TIMING
    pRec->counter += 1;
    if ((pRec->counter % (256*256*256)) == 0) {
      struct timespec time_end, time_diff;
      long double diff_s ;

      clock_gettime(CLOCK_REALTIME, &time_end);
      time_diff = diff(pRec->time_start, time_end);
      diff_s = time_diff.tv_sec + (time_diff.tv_nsec/1E9) ;
      //fprintf(stderr, "diff: %ds::%dns %Lf seconds\n", time_diff.tv_sec, time_diff.tv_nsec, diff_s);
      double osr = ((double)pRec->counter / (diff_s)/1E6);
      double esr = abs(pRec->fsMHz-osr)/pRec->fsMHz;
      fprintf (stderr, "***>>>>>>>>>>> Samples received: %u, %.3f MSPS, err %f\n", pRec->counter, osr, esr);

      // fprintf (stderr, ">>>>>>>>>>>>>> %s: #samples: %d\n", __FUNCTION__, pRec->samplesPerPacket);  
      // fprintf (stderr, ">>>>>>>>>>>>>> LSB first: i: %08f q: %08f\n", *pi, *pq );  
      // fprintf (stderr, ">>>>>>>>>>>>>> i: %f q: %f\n", pRec->input_buffer[pRec->samples], pRec->input_buffer[pRec->samples+BUFFER_SIZE]);

      // start a new measurement cycle
      pRec->time_start = time_end;
      pRec->counter = 0;
    }
#endif

    // average together adjacent samples to low pass filter 
    switch (pRec->m) {
    case 1: pRec->mxi += ((int)*xi++) << 16; pRec->mxq += ((int)*xq++) << 16; pRec->mi += 1; break;
    case 2: pRec->mxi += ((int)*xi++) << (16-1); pRec->mxq += ((int)*xq++) << (16-1); pRec->mi += 1; break;
    case 4: pRec->mxi += ((int)*xi++) << (16-2); pRec->mxq += ((int)*xq++) << (16-2); pRec->mi += 1; break;
    case 8: pRec->mxi += ((int)*xi++) << (16-3); pRec->mxq += ((int)*xq++) << (16-3); pRec->mi += 1; break;
    case 16: pRec->mxi += ((int)*xi++) << (16-4); pRec->mxq += ((int)*xq++) << (16-4); pRec->mi += 1; break;
    case 32: pRec->mxi += ((int)*xi++) << (16-5); pRec->mxq += ((int)*xq++) << (16-5); pRec->mi += 1; break;
    case 64: pRec->mxi += ((int)*xi++) << (16-6); pRec->mxq += ((int)*xq++) << (16-6); pRec->mi += 1; break;
    }

    // accumulate next decimated sample
    if (pRec->mi == pRec->m) {
      // copy into the output buffer, converting to float and scaling
      pRec->input_buffer[pRec->samples+BUFFER_SIZE]  = float(pRec->mxi) / SCALE_FACTOR_32B;
      pRec->input_buffer[pRec->samples]              = float(pRec->mxq) / SCALE_FACTOR_32B;
      pRec->samples++;

      // reset partial decimation
      pRec->mi = 0; pRec->mxi = 0; pRec->mxq = 0;

      // when we have enough samples, send them to the client
      if(pRec->samples==BUFFER_SIZE) {
	send_IQ_buffer(pRec);
	pRec->samples=0;      // signal that the output buffer is empty again
      }
    }
  }
  return;
}

void *helper_thread (void *arg) {
  RECEIVER *pRec = (RECEIVER *)arg;

  printf(" !!!!!!!!! THREAD: [%p]\n",  pRec);
  while (1) {
    if (pRec->connected) {
      if (read_IQ_buffer(pRec) != 0)
	break;
      user_data_callback(pRec, pRec->i_buffer, pRec->q_buffer);
    }
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

const char* parse_command(CLIENT* client,char* command) {
    
  char* token;

  fprintf(stderr,"parse_command: '%s'\n",command);

  token=strtok(command," \r\n");
  if(token!=NULL) {
    if(strcmp(token,"attach")==0) {
      // select receiver
      token=strtok(NULL," \r\n");
      if(token!=NULL) {
	int rx=atoi(token);
	return attach_receiver(rx,client);
      } else {
	return INVALID_COMMAND;
      }
    } else if(strcmp(token,"detach")==0) {
      // select receiver
      token=strtok(NULL," \r\n");
      if(token!=NULL) {
	int rx=atoi(token);
	return detach_receiver(rx,client);
      } else {
	return INVALID_COMMAND;
      }
    } else if(strcmp(token,"frequency")==0) {
      // set frequency
      token=strtok(NULL," \r\n");
      if(token!=NULL) {
	long f=atol(token);
	return set_frequency (client,f);
      } else {
	return INVALID_COMMAND;
      }
    } else if(strcmp(token,"start")==0) {
      token=strtok(NULL," \r\n");
      if(token!=NULL) {
	if(strcmp (token,"iq")==0) {
	  token=strtok(NULL," \r\n");
	  if(token!=NULL) {
	    client->iq_port=atoi(token);
	  }

	  printf("xxxxxxxxxxxxxxxxxxx Starting async data acquisition... CLIENT REQUESTED %d port\n", client->iq_port);

	  (receiver[client->receiver]).samples = 0; // empties output buffer
	  start_receiver(&receiver[client->receiver]);
	  pthread_t thread_id;
	  // create the thread to listen for TCP connections
	  int r = pthread_create(&thread_id,NULL,helper_thread,&(receiver[client->receiver]));
	  if( r < 0) {
	    perror("pthread_create helper_thread failed");
	    exit(1);
	  }

	} else if(strcmp(token,"bandscope")==0) {
	  token=strtok(NULL," \r\n");
	  if(token!=NULL) {
	    client->bs_port=atoi(token);
	  }
	  return OK;
	} else {
	  // invalid command string
	  return INVALID_COMMAND;
	}
      } else {
	// invalid command string
	return INVALID_COMMAND;
      }
    } else if(strcmp(token,"preamp")==0) {
      // set frequency
      token=strtok(NULL," \r\n");
      if(token!=NULL) {
	if (strcmp(token,"on")==0) {
	  return set_preamp (client,true);
	}
	if (strcmp(token,"off")==0) {
	  return set_preamp (client,false);
	}
	return INVALID_COMMAND;
      } else {
	return INVALID_COMMAND;
      }
    } else if(strcmp(token,"dither")==0) {
      // set frequency
      token=strtok(NULL," \r\n");
      if(token!=NULL) {
	if (strcmp(token,"on")==0) {
	  return set_dither (client,true);
	}
	if (strcmp(token,"off")==0) {
	  return set_dither (client,false);
	}
	return INVALID_COMMAND;
      } else {
	return INVALID_COMMAND;
      }
    } else if(strcmp(token,"setattenuator")==0) {
      // set attenuator
      token=strtok(NULL," \r\n");
      if(token!=NULL) {
	long av=atol(token);
	return set_attenuator (client,av);
      } else {
	return INVALID_COMMAND;
      }
    } else if(strcmp(token,"random")==0) {
      // set frequency
      token=strtok(NULL," \r\n");
      if(token!=NULL) {
	if (strcmp(token,"on")==0) {
	  return set_random (client,true);
	}
	if (strcmp(token,"off")==0) {
	  return set_random (client,false);
	}
	return INVALID_COMMAND;
      } else {
	return INVALID_COMMAND;
      }
    } else if(strcmp(token,"stop")==0) {
      token=strtok(NULL," \r\n");
      if(token!=NULL) {
	if(strcmp(token,"iq")==0) {
	  token=strtok(NULL," \r\n");
	  if(token!=NULL) {
	    client->iq_port=-1;
	  }
	  // try to terminate audio thread
	  close ((receiver[client->receiver]).audio_socket);
	  printf("Quitting...\n");
	  stop_receiver(&receiver[client->receiver]);
	  return OK;
	} else if(strcmp(token,"bandscope")==0) {
	  client->bs_port=-1;
	} else {
	  // invalid command string
	  return INVALID_COMMAND;
	}
      } else {
	// invalid command string
	return INVALID_COMMAND;
      }
    } else if(strcmp(token,"quit")==0) {
      return QUIT_ASAP;

    } else if(strcmp(token,"hardware?")==0) {
      fprintf (stderr, "*****************************\n");
      return "OK sdrplay";

    } else if(strcmp(token,"getserial?")==0) {
      static char buf[50];
      snprintf (buf, sizeof(buf), "OK N/A");
      return buf;

    } else {
      // invalid command string
      fprintf (stderr, "***************************** INVALID_COMMAND: %s\n", token);
      return INVALID_COMMAND;
    }
  } else {
    // empty command string
    return INVALID_COMMAND;
  }
  return INVALID_COMMAND;
}

