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
#include <string>     // c++ std strings

#include "hiqsdr.h"
#include "client.h"
#include "receiver.h"
#include "messages.h"

static int counter = 0;


#define SCALE_FACTOR_24B 8388607.0         // 2^24 / 2 - 1 = 8388607.0
#define SCALE_FACTOR_32B 2147483647.0      // 2^32 / 2 - 1 = 2147483647.0
#define SCALE_FACTOR_0   0.0
#define SCALE_FACTOR_1   1.0

#define ADC_CLIP 0x01


int user_data_callback(void *buf, int buf_size, void *extra)
{
	// The buffer received contains 24-bit signed integer IQ samples (6 bytes per sample)
	// we save the received IQ samples as 32 bit (msb aligned) integer IQ samples.

/*
    4 Reception frame format

    The received samples are carried by UDP frames with a total fixed payload size
    of 1442 bytes. Each sample consists of 3 Bytes I and 3 Bytes Q data. 
    They are interleaved. 

    The first byte of each UDP frame carries a sequence number that
    is incremented from frame to frame. It can be used to identify a loss of UDP
    messages during reception. 
    The second byte is used for signalling the current status of the reception. 
     
        Bit 0 is used as Key indication. If it is 0 the PTT is issued and if it is 1 the PTT is not active. 
        Bit 1 indicates clipping of the ADC (overrange). 
        
    The data format of the samples is little endian (least signicant
    byte first) with real words at the odd (first) positions and the imaginary words
    at the even (second) positions.
*/

	uint8_t	*samplebuf 	= ((uint8_t*)(buf));
    int nSamples 		= (buf_size - 2)/6;      // each sample is a three byte signed integer

    int32_t i;
    int32_t q;
    unsigned char *pi = (unsigned char *) &i;
    unsigned char *pq = (unsigned char *) &q;


    // skip the first two bytes
    int seq      = *samplebuf++;               // frame sequence
    int adc_clip = ADC_CLIP & (*samplebuf++);  // various flags


	RECEIVER *pRec = (RECEIVER *)extra;

    // sanity check on frame counter
    if (pRec->frame_counter == -1) {
        pRec->frame_counter = ((seq+1) % 256);
    } else {
        if (!(seq == ((pRec->frame_counter++) % 256) )) {
           fprintf (stderr, "WARNING: Expected: %d, found: %d\n", pRec->frame_counter, seq);
           pRec->frame_counter = (seq+1) % 256 ;
        } 
    }


	for (int k=0; k < nSamples; k++) {

        if ((counter++ % 204800) == 0) {
           fprintf (stderr, ">>>>>>>>>>>>>> %s: #samples: %d\n", __FUNCTION__, nSamples);  
           fprintf (stderr, ">>>>>>>>>>>>>> LSB first: i: %02X%02X%02X q: %02X%02X%02X\n",
                    (unsigned char)samplebuf [0], (unsigned char)samplebuf [1], (unsigned char)samplebuf [2], 
                    (unsigned char)samplebuf [3], (unsigned char)samplebuf [4], (unsigned char)samplebuf [5]
                    );  
           fprintf (stderr, ">>>>>>>>>>>>>> i: 0x%08X q: 0x%08X\n", i, q);
           fprintf (stderr, ">>>>>>>>>>>>>> i: %f q: %f\n", 
                    pRec->input_buffer[pRec->samples], pRec->input_buffer[pRec->samples+BUFFER_SIZE]);
           fflush(stderr);
        }

        i = 0, q = 0;                  // zeroes the temporaries 
        memcpy (pi + 1, samplebuf, 3); // copy 24 bit of sample in most significant position
        samplebuf += 3;                // advance to next integer
        memcpy (pq + 1, samplebuf, 3); // copy 24 bit of sample in most significant position 
        samplebuf += 3;                // advance to next integer                            

        // copy into the output buffer, converting to float and rescaling
        pRec->input_buffer[pRec->samples]             = ((float) q)  / SCALE_FACTOR_32B;
        pRec->input_buffer[pRec->samples+BUFFER_SIZE] = ((float) i)  / SCALE_FACTOR_32B;

        pRec->samples++; // next output sample 

        // when we have enough samples, send them to the client
        if(pRec->samples==BUFFER_SIZE) {
            #if 1

            /*
             * Correction for DC offset in samples
             */
            pRec->dc_count += BUFFER_SIZE;
            for (int x=0; x < BUFFER_SIZE; ++x) {

                pRec->dc_sum_q += pRec->input_buffer[x] ;
                pRec->dc_sum_i += pRec->input_buffer[x+BUFFER_SIZE] ;

            }

            if (pRec->dc_count > (pRec->cfg.sr * 2)) {
                pRec->dc_average_i = pRec->dc_sum_i / pRec->dc_count;
                pRec->dc_average_q = pRec->dc_sum_q / pRec->dc_count;
                pRec->dc_sum_i = 0;
                pRec->dc_sum_q = 0;
                pRec->dc_count = 0;
            }


            for (int x=0; x < BUFFER_SIZE; ++x) {

                pRec->input_buffer[x]             -= pRec->dc_average_q;
                pRec->input_buffer[x+BUFFER_SIZE] -= pRec->dc_average_i;

            }
            #endif
            // send I/Q data to clients
            //fprintf (stderr, "%s: sending data.\n", __FUNCTION__);
            send_IQ_buffer(pRec);
            pRec->samples=0;      // signal that the output buffer is empty again
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

                    printf("***************************Starting async data acquisition... CLIENT REQUESTED %d port\n", client->iq_port);

                    (receiver[client->receiver]).samples = 0;
                 	if ( hiqsdr_start_asynch_input ( user_data_callback, &(receiver[client->receiver])) < 0 ) {
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
        } else if(strcmp(token,"selectantenna")==0) {
            // select antenna
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
               long antenna = atol(token);
               return select_antenna (client,antenna);
            } else {
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"selectpresel")==0) {
            // select preselector
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
               long presel = atol(token);
               return select_preselector (client,presel);
            } else {
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"activatepreamp")==0) {
            // activate preamplifier
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
               long preamp = atol(token);
               return set_preamplifier (client,preamp);
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
                    hiqsdr_stop_asynch_input ();
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
            return "OK HiQSDR";

        } else if(strcmp(token,"getserial?")==0) {
            // no serial number concept available in HiQSDR, returns instead the IP address
            static char buf[50];
            snprintf (buf, sizeof(buf), "OK %s", hiqsdr_get_ip_address());
            return buf;

        } else if(strcmp(token,"getpreselector?")==0) {
            // get preselector
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
               long p=atol(token);
               // preselector query
               if (p >= 0 && p < 16) {
                   static char buf[50];
                   static char pd[BUFSIZ];

                   if (!hiqsdr_get_preselector_desc(p, pd)) {
                       snprintf (buf, sizeof(buf), "OK \"%s\"", pd);
                       return buf;
                   } else
                       return INVALID_COMMAND;
               } else 
                   return INVALID_COMMAND;
            } else {
                return INVALID_COMMAND;
            }

        } else if(strcmp(token,"getpreampstatus?")==0) {
            // returns preamp status
            static char buf[50];
            snprintf (buf, sizeof(buf), "OK %d", hiqsdr_get_preamp ());
            return buf;

        } else {
            // invalid command string
            return INVALID_COMMAND;
        }
    } else {
        // empty command string
        return INVALID_COMMAND;
    }
    return INVALID_COMMAND;
}

