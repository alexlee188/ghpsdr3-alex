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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/queue.h>

#include "sdriq.h"
#include "client.h"
#include "receiver.h"
#include "messages.h"


static int counter = 0;



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


#define SCALE_FACTOR_24B 8388607.0         // 2^24 / 2 - 1 = 8388607.0
#define SCALE_FACTOR_32B 2147483647.0      // 2^32 / 2 - 1 = 2147483647.0
#define SCALE_FACTOR_0   0.0
#define SCALE_FACTOR_1   1.0

#define ADC_CLIP 0x01


int user_data_callback (SAMPLE_T *pi, SAMPLE_T *pq, int nSamples, void *extra)
{
	// The buffer received contains 24-bit signed integer IQ samples (6 bytes per sample)
	// we save the received IQ samples as 32 bit (msb aligned) integer IQ samples.


    // each sample is split into two vector
    //SAMPLE_T *pi = (SAMPLE_T *) ii;
    //SAMPLE_T *pq = (SAMPLE_T *) qq;


    // skip the first two bytes
    //int seq      = *samplebuf++;

	RECEIVER *pRec = (RECEIVER *)extra;

    if (nSamples >0) { 
        pRec->cfg.ns += nSamples;
    }

    #if 0
    if (nSamples > 0) {

        fprintf (stderr, "---> %d\n", nSamples);
        int j;
        for (j=0; j<5; ++j) fprintf(stderr, "[%f+j%f] ", pi[j], pq[j]);
        fprintf (stderr, "\n");
    }
    #endif

	while (nSamples) {

        #if 1
        if ((counter++ % 204800) == 0) {
            long double diff_s ;
            const QUISK_SOUND_STATE *pss = get_state ();

            if (nSamples >0) { 
                pRec->cfg.ns -= nSamples;
            }

            clock_gettime(CLOCK_REALTIME, &pRec->cfg.time_end);
            pRec->cfg.time_diff = diff(pRec->cfg.time_start, pRec->cfg.time_end);
            diff_s = pRec->cfg.time_diff.tv_sec + (pRec->cfg.time_diff.tv_nsec/1E9) ;
            //fprintf(stderr, "diff: %ds::%dns %Lf seconds\n", time_diff.tv_sec, time_diff.tv_nsec, diff_s);
            fprintf (stderr, "***>>>>>>>>>>> Samples received: %lu, %.3Lf kS/s\n", pRec->cfg.ns, ((double)pRec->cfg.ns / (diff_s)/1E3) );


            fprintf (stderr, "***>>>>>>>>>>> Errors: %d overrange: %d poll: %d\n", 
                     pss->read_error, pss->overrange,pss->data_poll_usec);

            fprintf (stderr, "[%s] [%s]\n", pRec->cfg.start, pRec->cfg.stop);
            fprintf (stderr, "%p %p\n", pi, pq);
            fprintf (stderr, ">>>>>>>>>>>>>> %s: #samples: %d\n", __FUNCTION__, nSamples);  
            fprintf (stderr, ">>>>>>>>>>>>>> LSB first: i: %08f q: %08f\n", *pi, *pq );  
            fprintf (stderr, ">>>>>>>>>>>>>> i: %f q: %f\n", 
                     pRec->input_buffer[pRec->samples], pRec->input_buffer[pRec->samples+BUFFER_SIZE]);
            fflush(stderr);

            // start a new measurement cycle
            clock_gettime (CLOCK_REALTIME, &pRec->cfg.time_start);
            pRec->cfg.ns = 0L;
        }
        #endif

        // copy into the output buffer, converting to float
        pRec->input_buffer[pRec->samples]             = *pq ;
        pRec->input_buffer[pRec->samples+BUFFER_SIZE] = *pi ;

        pq++, pi++;      // next input sample
        nSamples--;      // one less
        pRec->samples++; // next output sample 

        #if 1
        // when we have enough samples, send them to the client
        if(pRec->samples==BUFFER_SIZE) {

            if (pRec->m_NcoSpurCalActive == false) {
               int x;
               for (x=0; x < pRec->samples; ++x) {
                  pRec->input_buffer[x]             -= pRec->m_NCOSpurOffsetQ;
                  pRec->input_buffer[x+BUFFER_SIZE] -= pRec->m_NCOSpurOffsetI;
               }
            } else {
               NcoSpurCalibrate (pRec);
            }
           
            // send I/Q data to clients
            //fprintf (stderr, "%s: sending data.\n", __FUNCTION__);
            send_IQ_buffer(pRec);
            pRec->samples=0;      // signal that the output buffer is empty again
        }
        #else

        // when we have enough samples, send them to the client
        if(pRec->samples==BUFFER_SIZE) {
            // send I/Q data to clients

            struct tailq_entry *item;

           /*
             * Each item we want to add to the tail queue must be
             * allocated.
             */
            item = malloc(sizeof(*item));
            if (item == NULL) {
                perror("malloc failed");
                exit(EXIT_FAILURE);
            } else {
                //fprintf(stderr,"%s, inserting\n", __FUNCTION__);
                memcpy (item->iq_buf, pRec->input_buffer, sizeof (BUFFER));
                TAILQ_INSERT_TAIL(&(pRec->iq_tailq_head), item, entries);

            }
            pRec->samples=0;      // signal that the output buffer is empty again
        }
        #endif
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

    fprintf(stderr,"********* parse_command: '%s'\n",command);

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
                    memset (&receiver[client->receiver].cfg.time_start, 0, sizeof(receiver[client->receiver].cfg.time_start)); 
                    memset (&receiver[client->receiver].cfg.time_end  , 0, sizeof(receiver[client->receiver].cfg.time_end)); 
                    memset (&receiver[client->receiver].cfg.time_diff , 0, sizeof(receiver[client->receiver].cfg.time_diff)); 
                    receiver[client->receiver].cfg.ns = 0L;
                    printf("*****Starting async data acquisition on receiver %d... CLIENT REQUESTED %d port\n", client->receiver, client->iq_port);

                    (receiver[client->receiver]).samples = 0;
                 	if ( sdriq_start_asynch_input ( user_data_callback, &(receiver[client->receiver])) < 0 ) {
                 		printf("start async input error\n" );
                        return INVALID_COMMAND;
                 	} else {
                 		printf("start async input: %s\n", "STARTED");
                    }

                    return OK;
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
                    //hiqsdr_stop_asynch_input ();
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
            return "OK SDR-IQ";

        } else if(strcmp(token,"getserial?")==0) {
            static char buf[50];
            snprintf (buf, sizeof(buf), "OK %s", get_serial ());
            return buf;

        } else {
            // invalid command string
            return INVALID_COMMAND;
        }
    }
    // empty command string
    return INVALID_COMMAND;
}

