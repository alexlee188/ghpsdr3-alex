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
#include <math.h>
#include "hiqsdr.h"
#include "client.h"
#include "receiver.h"
#include "messages.h"

static int counter = 0;


#define SCALE_FACTOR_24B 8388607.0         // 2^24 / 2 - 1 = 8388607.0
#define SCALE_FACTOR_32B 2147483647.0      // 2^32 / 2 - 1 = 2147483647.0
#define SCALE_FACTOR_0   0.0
#define SCALE_FACTOR_1   1.0
#define SMALL_PACKETS

#define ADC_CLIP 0x01

static float tx_input_buffer[BUFFER_SIZE*3]; // IQ data from dspserver



int hpsdr=0;

struct sockaddr_in tx_data_addr;
int tx_data_length = sizeof(tx_data_addr);
int tx_data_port = 48249;
int tx_data_socket;


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


	} else if(strcmp(token,"mox")==0) {
  		   token=strtok(NULL," \r\n");
            	   if(token!=NULL) {
                   int moxtoken=atoi(token);
		   fprintf(stderr,"Moxtoken='%d'\n",moxtoken);
		   hiqsdr_setPTT(moxtoken);

		   return "OK";	                   
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


	



void* tx_iq_thread(void* arg) {
    int tx_iq_socket;
    struct sockaddr_in tx_iq_addr;
    int tx_iq_length = sizeof(tx_iq_addr);
    BUFFER buffer;   
    short tx_send_buffer[3*BUFFER_SIZE];
    int size_waiting_data=0;
    short tx_udp_buffer[601] = { 0 };
    int write_pointer=0 ,read_pointer = 0;
    int i = 0;
    
    
    
     //setup tx_data_socket  -  socket to send tx data to hiqsdr HW
 
    tx_data_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(tx_data_socket<0) {
        perror("create socket failed for tx iq socket ");
        exit (1);
    }

    memset(&tx_data_addr,0,tx_data_length);

    tx_data_addr.sin_family=AF_INET;
    
    inet_pton (AF_INET, hiqsdr_get_ip_address (), &tx_data_addr.sin_addr);
    fprintf(stderr,"tx_ip_address: %s",hiqsdr_get_ip_address ());
    
 
    tx_data_addr.sin_port=htons(tx_data_port);

 
    fprintf(stderr,"tx_iq_thread\n");

    // create a socket to receive iq from the dspserver
 
    tx_iq_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(tx_iq_socket<0) {
        perror("tx_iq_thread: create tx_iq socket failed");
        exit(1);
    }

    memset(&tx_iq_addr,0,tx_iq_length);

    tx_iq_addr.sin_family=AF_INET;
    tx_iq_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    tx_iq_addr.sin_port=htons(AUDIO_PORT);

    if(bind(tx_iq_socket,(struct sockaddr*)&tx_iq_addr,tx_iq_length)<0) {
        perror("tx_iq_thread: bind socket failed for tx iq socket");
        exit(1);
    }

    fprintf(stderr,"tx_iq_thread: iq bound to port %d socket=%d\n",htons(tx_iq_addr.sin_port),tx_iq_socket);
    
    while(1) {
        int bytes_read;
        int offset = 0;
        int bytes_written;
        
        unsigned long rx_sequence = 0;
#ifdef SMALL_PACKETS
        while(1) {
            bytes_read=recvfrom(tx_iq_socket,(char*)&buffer,sizeof(buffer),0,(struct sockaddr*)&tx_iq_addr,(socklen_t *)&tx_iq_length);
            if(bytes_read<0) {
                perror("recvfrom socket failed for tx iq buffer");
                exit(1);
            }
            
                
	    
            if(buffer.offset==0) {
                offset=0;
                rx_sequence=buffer.sequence;
                // start of a frame
       
                memcpy((char *)&tx_input_buffer[buffer.offset/4],(char *)&buffer.data[0],buffer.length);
                offset+=buffer.length;
               //fprintf(stderr,"iq received from dspserver: %f,%f at offset = %d\n",tx_input_buffer[0],tx_input_buffer[1], buffer.offset);
                   
              
               
            } else {
                if((rx_sequence==buffer.sequence) && (offset==buffer.offset)) {
                    memcpy((char *)&tx_input_buffer[buffer.offset/4],(char *)&buffer.data[0],buffer.length);
                
                    
                   
                    offset+=buffer.length;
                    
                    if((hpsdr && (offset==(BUFFER_SIZE*3*4))) || (!hpsdr && (offset==(BUFFER_SIZE*2*4)))) {
                        
                        
                        // if(mox){  - PTT status to be integrated in HIQSDR structure in hiqsdr.cpp
                       
                       
                       // tx_input_buffer is non interleaved - has to be interleaved here!!
                       
						#pragma omp parallel for schedule(static) private(i) 
						
                         for (i=0;i<(BUFFER_SIZE);i=i+2) { // i from 0 to 1023-> inp.buf. from 0 to 2047
							
							 tx_send_buffer[i+1+write_pointer]=(short) (tx_input_buffer[i]*32767.0);            //  take every 2nd samplepair - sample rate reduction
                             tx_send_buffer[i+write_pointer]=(short) (tx_input_buffer[i+(BUFFER_SIZE)]*32767.0);//  to 48 khz - has to be modified to be dynamic
                            
                            
                            }
                            
                    
                            
                            
                            
							write_pointer = write_pointer + BUFFER_SIZE;  //more 1024 shorts written
                            size_waiting_data += BUFFER_SIZE;
                            
                            while (size_waiting_data >=600) {
								//copy 600 shorts to the 2nd position of the udp buffer - first pos. is 0 for hiqsdr
								
								if (((3*BUFFER_SIZE)-read_pointer) >= 600) {  // we can copy in one step 	
							    memcpy((char *)&tx_udp_buffer[1],(char *) &tx_send_buffer[read_pointer],1200);
							    read_pointer+=600;
							   
							
							    }
							    else {  // we have to copy in 2 steps - end and front of the buffer
							    memcpy((char *)&tx_udp_buffer[1],(char *) &tx_send_buffer[read_pointer],2*((3*BUFFER_SIZE)-read_pointer));
							    memcpy((char *)&tx_udp_buffer[1+((3*BUFFER_SIZE)-read_pointer)],(char *) &tx_send_buffer[0],2*(600-((3*BUFFER_SIZE)-read_pointer)));
							    read_pointer=600-((3*BUFFER_SIZE)-read_pointer);
							   
							    }
							    if (read_pointer>=(3*BUFFER_SIZE)) read_pointer=0;
							
							    
								tx_udp_buffer[0]=0;  //not necessary just for safety that the first short is 0
								
								bytes_written=sendto(tx_data_socket,(char*)&tx_udp_buffer,sizeof(tx_udp_buffer),0,(struct sockaddr*)&tx_data_addr,tx_data_length);
							
							 
							
							
								
								if(bytes_written<0) {
									
								fprintf(stderr,"tx udp sendto hiqsdr HW failed with : %d audio_socket=%d\n",bytes_written,tx_iq_socket);
								exit(1);
								} else {
									
																   
								   }
								
								size_waiting_data = size_waiting_data-600;
							}
								
								
								
                            
                        if (write_pointer > (2*BUFFER_SIZE)) write_pointer=0;
                                          
                        offset=0;
                        break;
                    }
                } else {
			fprintf(stderr,"missing TX IQ frames from dspserver\n");
                }
            } // else if(buffer.offset==0)
	} // end while(1)
#else
	

#endif




    } // end while
}
