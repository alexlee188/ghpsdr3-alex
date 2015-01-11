/* 
 * File:   ozy.c
 * Author: jm57878
 *
 * Created on 10 March 2009, 20:26
 */


/* Copyright (C) - modifications of the original program by John Melton
* 2011 - Alex Lee, 9V1Al
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
* Foundation, Inc., 59 Temple Pl
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
 
#include "ozy.h"
#include "dttsp.h"
#include "audiostream.h"
#include "client.h"
#include "util.h"

#include "main.h"

/*
 *   ozy interface
 */

#define SMALL_PACKETS

static sem_t ozy_send_semaphore;
static sem_t ozy_cmd_semaphore;

#define USB_TIMEOUT -7
//static struct OzyHandle* ozy;

// Added by Alex lee 18 Aug 2010
double LO_offset = 9000;  // LO offset 9khz

int receiver;

//static char ozy_firmware_version[9];
//int mercury_software_version=0;
//int penelope_software_version=0;
//int ozy_software_version=0;

static pthread_t iq_thread_id;
//static pthread_t keepalive_thread_id;

//static long rxFrequency=7056000;
//static int rxFrequency_changed=1;
//static long txFrequency=7056000;
//static int txFrequency_changed=1;

static int ozy_debug=0;

/** Response buffers passed to ozy_send must be this size */
#define OZY_RESPONSE_SIZE 64

#define BUFFER_SIZE 1024
int buffer_size=BUFFER_SIZE;

float input_buffer[BUFFER_SIZE*3]; // I,Q,Mic
float output_buffer[BUFFER_SIZE*4];

float mic_left_buffer[BUFFER_SIZE];
float mic_right_buffer[BUFFER_SIZE];

int samples=0;

int left_sample;
int right_sample;
int mic_sample;


float left_sample_float;
float right_sample_float;
float mic_sample_float;

short left_rx_sample;
short right_rx_sample;
short left_tx_sample;
short right_tx_sample;

int frames=0;
int usb_output_buffers=0;

int show_software_serial_numbers=1;

unsigned char iq_samples[SPECTRUM_BUFFER_SIZE];

int lt2208ADCOverflow=0;

int class=0;           // default other
int lt2208Dither=1;    // default dither on
int lt2208Random=1;    // default random 0n
int alexAttenuation=0; // default alex attenuation 0Db
int micSource=1;       // default mic source Penelope
int clock10MHz=2;      // default 10 MHz clock source Mercury
int clock122_88MHz=1;  // default 122.88 MHz clock source Mercury
int preamp=0;          // default preamp off

int sampleRate=48000;  // default 48k

int mox=0;             // default not transmitting

int ptt=0;
int dot=0;
int dash=0;

#define COMMAND_PORT 12000
#define SPECTRUM_PORT 13000
#define AUDIO_PORT 15000

int command_socket;
int command_port;
static struct sockaddr_in command_addr;
static socklen_t command_length=sizeof(command_addr);

int audio_socket;
int audio_port;
struct sockaddr_in audio_addr;
socklen_t audio_length=sizeof(audio_addr);

struct sockaddr_in server_audio_addr;
socklen_t server_audio_length=sizeof(server_audio_addr);

static struct sockaddr_in server_addr;
static socklen_t server_length=sizeof(server_addr);

short server_port;

int session;

int hpsdr=0;
int hpsdr_local = 0;


static int local_audio=0;
static int port_audio=0;

#include <samplerate.h>
//
// samplerate library data structures
//
SRC_STATE *sr_state;
double src_ratio;




void dump_udp_buffer(unsigned char* buffer);

void* iq_thread(void* arg) {
    int iq_socket;
    struct sockaddr_in iq_addr;
    int iq_length = sizeof(iq_addr);
    BUFFER buffer;
    int on=1;
    const char *dspserver_address = (const char *)arg;

    sdr_thread_register("iq_thread");
fprintf(stderr,"iq_thread\n");

    // create a socket to receive iq from the server
    iq_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(iq_socket<0) {
        perror("iq_thread: create iq socket failed");
        exit(1);
    }

    setsockopt(iq_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&iq_addr,0,iq_length);

    iq_addr.sin_family=AF_INET;
    fprintf(stderr,"iq_thread: iq address: %s\n", dspserver_address);
    iq_addr.sin_addr.s_addr=inet_addr(dspserver_address);
    iq_addr.sin_port=htons(SPECTRUM_PORT+(receiver*2));

    if(bind(iq_socket,(struct sockaddr*)&iq_addr,iq_length)<0) {
        perror("iq_thread: bind socket failed for iq socket");
        exit(1);
    }

    fprintf(stderr,"iq_thread: iq bound to port %d socket=%d\n",htons(iq_addr.sin_port),iq_socket);
    fprintf(stderr,"audiostream_conf.samplerate=%d\n", audiostream_conf.samplerate);
    
    while(1) {
        int bytes_read;
        int offset = 0;
        unsigned long rx_sequence = 0;
#ifdef SMALL_PACKETS
        while(1) {
            bytes_read=recvfrom(iq_socket,(char*)&buffer,sizeof(buffer),0,(struct sockaddr*)&iq_addr,(socklen_t *)&iq_length);
            if(bytes_read<0) {
                perror("recvfrom socket failed for iq buffer");
                exit(1);
            }

	    if(ozy_debug) {
		fprintf(stderr,"rcvd UDP packet: sequence=%lld offset=%d length=%d\n",
			buffer.sequence, buffer.offset, buffer.length);
	    }

            if(buffer.offset==0) {
                offset=0;
                rx_sequence=buffer.sequence;
                // start of a frame
                memcpy((char *)&input_buffer[buffer.offset/4],(char *)&buffer.data[0],buffer.length);
                offset+=buffer.length;
            } else {
                if((rx_sequence==buffer.sequence) && (offset==buffer.offset)) {
                    memcpy((char *)&input_buffer[buffer.offset/4],(char *)&buffer.data[0],buffer.length);
                    offset+=buffer.length;
                    if((hpsdr && (offset==(BUFFER_SIZE*3*4))) || (!hpsdr && (offset==(BUFFER_SIZE*2*4)))) {
                        offset=0;
                        break;
                    }
                } else {
			fprintf(stderr,"missing IQ frames\n");
                }
            } // if(buffer.offset==0)
	} // end while(1)
#else
	if (hpsdr)
        	bytes_read=recvfrom(iq_socket,(char*)input_buffer,BUFFER_SIZE*3*4,0,(struct sockaddr*)&iq_addr,&iq_length);
	else 	bytes_read=recvfrom(iq_socket,(char*)input_buffer,BUFFER_SIZE*2*4,0,(struct sockaddr*)&iq_addr,&iq_length);
        if(bytes_read<0) {
            perror("recvfrom socket failed for iq buffer");
            exit(1);
        }
#endif

        Audio_Callback (input_buffer,&input_buffer[BUFFER_SIZE],
                                output_buffer,&output_buffer[BUFFER_SIZE], buffer_size, 0);

        // process the output with resampler
           int rc;
           int j, i;
           float data_in [BUFFER_SIZE*2];
           float data_out[BUFFER_SIZE*2];
           SRC_DATA data;

           assert(buffer_size<=BUFFER_SIZE);
	   #pragma omp parallel for schedule(static) private(j)
           for (j=0; j < buffer_size; j++) {
              data_in [j*2]   = output_buffer[j];              //left_samples[i];
              data_in [j*2+1] = output_buffer[j+BUFFER_SIZE];  // right_samples[i];
           }

           data.data_in = data_in;
           data.input_frames = buffer_size ;

           data.data_out = data_out;
           data.output_frames = buffer_size ;
           data.src_ratio = src_ratio;
           data.end_of_input = 0;

           rc = src_process (sr_state, &data) ;
           if (rc) {
               fprintf (stderr,"SRATE: error: %s (rc=%d)\n", src_strerror (rc), rc);
           } else {
               for (i=0;i < data.output_frames_gen;i++) {
                  left_rx_sample=(short)(data.data_out[i*2]*32767.0);
                  right_rx_sample=(short)(data.data_out[i*2+1]*32767.0);
                  audio_stream_put_samples(left_rx_sample,right_rx_sample);
                   }
	   } // if (rc)


        // send the audio back to the server.
        // This is for HPSDR hardware.
        // NOTE: the stream is yet at the RX sample rate
        // the (crude) rate adaption is done into hpsdr_server (ozy.c - process_ozy_output_buffer() )
        if(hpsdr) {
                if(hpsdr_local && mox) {
                    //
                    // MOX compute the TX data from locally generated microphone data
                    //
                    Audio_Callback (&input_buffer[BUFFER_SIZE*2],&input_buffer[BUFFER_SIZE*2],
                                    &output_buffer[BUFFER_SIZE*2],&output_buffer[BUFFER_SIZE*3], buffer_size, 1);
                } else {
                    //
                    // NO MOX, zeroing the microphone output sample buffer
                    //
                    for(j=0;j<buffer_size;j++) {
                        output_buffer[(BUFFER_SIZE*2)+j]=output_buffer[(BUFFER_SIZE*3)+j]=0.0F;
                    }
                }
                // sending samples when receiveing OR when transmitting in local mode
                if (!mox || (hpsdr && hpsdr_local)) ozy_send((unsigned char *)&output_buffer[0],sizeof(output_buffer),"ozy");
        } // if (hpsdr)
    } // end while
}

void ozy_send(unsigned char* data,int length,char* who) {
    static unsigned long tx_sequence = 0;
    BUFFER buffer;
    unsigned short offset=0;
    int rc;

//fprintf(stderr,"ozy_send: %s\n",who);
sem_wait(&ozy_send_semaphore);
#ifdef SMALL_PACKETS
            // keep UDP packets to 512 bytes or less
            //     8 bytes sequency number
            //     2 byte offset
            //     2 byte length
            offset=0;
            while(offset<length) {
                buffer.sequence=tx_sequence;
#ifndef __linux__
                buffer.sequenceHi = 0L;
#endif
                buffer.offset=offset;
                buffer.length=length-offset;
                if(buffer.length>500) buffer.length=500;
//fprintf(stderr,"ozy_send: %lld:%d:%d\n",buffer.sequence,buffer.offset,buffer.length);
                memcpy((char*)&buffer.data[0],(char*)&data[offset],buffer.length);
                rc=sendto(audio_socket,(char*)&buffer,sizeof(buffer),0,(struct sockaddr*)&server_audio_addr,server_audio_length);
                if(rc<=0) {
                    fprintf(stderr,"sendto audio (SMALL_PACKETS) failed: %d audio_socket=%d errno=%d\n",rc,audio_socket,errno);
                    fprintf(stderr,"audio port is %d\n",ntohs(server_audio_addr.sin_port));
                    perror("sendto failed for audio data");
                    exit(1);
                }
                offset+=buffer.length;
            }
            tx_sequence++;

#else
                bytes_written=sendto(audio_socket,data,length,0,(struct sockaddr *)&server_audio_addr,server_audio_length);
                if(bytes_written<0) {
                   fprintf(stderr,"sendto audio failed: %d audio_socket=%d\n",bytes_written,audio_socket);
                   exit(1);
                }
#endif
sem_post(&ozy_send_semaphore);

}


/* --------------------------------------------------------------------------*/
/** 
* @brief send a command
* 
* @param command
* @param response buffer allocated by caller, MUST be OZY_RESPONSE_SIZE bytes
*/
void send_command(char* command, char *response) {
    int rc;

//fprintf(stderr,"send_command: command='%s'\n",command);

    sem_wait(&ozy_cmd_semaphore);
    rc=send(command_socket,command,strlen(command),0);
    if(rc<0) {
        sem_post(&ozy_cmd_semaphore);
        fprintf(stderr,"send command failed: %d: %s\n",rc,command);
        exit(1);
    }

    /* FIXME: This is broken.  It will probably work as long as
     * responses are very small and everything proceeds in lockstep. */
    rc=recv(command_socket,response,OZY_RESPONSE_SIZE,0);
    sem_post(&ozy_cmd_semaphore);
    if(rc<0) {
        fprintf(stderr,"read response failed: %d\n",rc);
    }
    /* FIXME: This is broken, too.  If the response is exactly
     * OZY_RESPONSE_SIZE, we have to truncate it by one byte. */
    if (rc == OZY_RESPONSE_SIZE)
        rc--;
    response[rc]=0;

//fprintf(stderr,"send_command: response='%s'\n",response);
}

void* keepalive_thread(void* arg) {
    char command[128], response[OZY_RESPONSE_SIZE];
    sprintf(command,"keepalive %d",receiver);
    while(1) {
        sleep(5);
        send_command(command, response);
    }
}

int make_connection() {
    char *token, *saveptr;
    char command[64], response[OZY_RESPONSE_SIZE];
    int result;

    result=0;
    sprintf(command,"attach %d",receiver);
    //sprintf(command,"connect %d %d",receiver,SPECTRUM_PORT+(receiver*2));
    send_command(command, response);

    token=strtok_r(response," ",&saveptr);
    if(token!=NULL) {
        if(strcmp(token,"OK")==0) {
            token=strtok_r(NULL," ",&saveptr);
            if(token!=NULL) {
		int sampleRate;
                result=0;
                sampleRate=atoi(token);
		fprintf(stderr,"connect: sampleRate=%d\n",sampleRate);
		setSpeed (sampleRate);
            } else {
                fprintf(stderr,"invalid response to connect: %s\n",response);
                result=1;
            }
        } else if (strcmp(token,"ERROR")==0) {
            result=1;
        } else {
            fprintf(stderr,"invalid response to connect: %s\n",response);
            result=1;
        }
    }

    sprintf(command,"start iq %d",SPECTRUM_PORT+(receiver*2));
    send_command(command, response);

    return result;
}

void ozyDisconnect() {
    char command[128], response[OZY_RESPONSE_SIZE];
    sprintf(command,"detach %d",receiver);
    send_command(command, response);

    close(command_socket);
    close(audio_socket);
}

int ozySetFrequency(long long ddsAFrequency) {
    char *token;
    char command[64], response[OZY_RESPONSE_SIZE];
    int result;
    char *saveptr;

    result=0;
    sprintf(command,"frequency %lld", (ddsAFrequency - (long long)LO_offset));
    send_command(command, response);
    token=strtok_r(response," ",&saveptr);
    if(token!=NULL) {
        if(strcmp(token,"OK")==0) {
            result=0;
            lastFreq = ddsAFrequency;
        } else if (strcmp(token,"ERROR")==0) {
            result=1;
        } else {
            fprintf(stderr,"invalid response to set frequency: %s\n",response);
            result=1;
        }
    }

    return result;
}

int ozySetPreamp(char* state) {
    char *token, *saveptr;
    char command[64], response[OZY_RESPONSE_SIZE];
    int result;

    result=0;
    sprintf(command,"preamp %s",state);
    send_command(command, response);
    token=strtok_r(response," ",&saveptr);
    if(token!=NULL) {
        if(strcmp(token,"OK")==0) {
            result=0;
        } else if (strcmp(token,"ERROR")==0) {
            result=1;
        } else {
            fprintf(stderr,"invalid response to set preamp: %s\n",response);
            result=1;
        }
    }

    return result;
}

int ozySetRecord(char* state) {
    char *token, *saveptr;
    char command[64], response[OZY_RESPONSE_SIZE];
    int result;

    result=0;
    sprintf(command,"record %s",state);
    send_command(command, response);
    token=strtok_r(response," ",&saveptr);
    if(token!=NULL) {
        if(strcmp(token,"OK")==0) {
            result=0;
        } else if (strcmp(token,"ERROR")==0) {
            result=1;
        } else {
            fprintf(stderr,"invalid response to record: %s\n",response);
            result=1;
        }
    }

    return result;
}

int ozySetMox(int state) {
    char *token, *saveptr;
    char command[64], response[OZY_RESPONSE_SIZE];
    int result;

    result=0;
    mox=state;
    sprintf(command,"mox %d",state);
    send_command(command, response);
    token=strtok_r(response," ",&saveptr);
    if(token!=NULL) {
        if(strcmp(token,"OK")==0) {
            result=0;
        } else if (strcmp(token,"ERROR")==0) {
            result=1;
        } else {
            fprintf(stderr,"invalid response to set mox: %s\n",response);
            result=1;
        }
    }

    return result;
}

int ozySetOpenCollectorOutputs(char* state) {
    char *token, *saveptr;
    char command[64], response[OZY_RESPONSE_SIZE];
    int result;

    result=0;
    sprintf(command,"setocoutputs %s",state);
    send_command(command, response);
    token=strtok_r(response," ",&saveptr);
    if(token!=NULL) {
        if(strcmp(token,"OK")==0) {
            result=0;
        } else if (strcmp(token,"ERROR")==0) {
            result=1;
        } else {
            fprintf(stderr,"invalid response to set oc outputs: %s\n",response);
            result=1;
        }
    }

    return result;
}

int ozySendStarCommand(char *command) {
    int result;
    char buf[256]; 
    char response[OZY_RESPONSE_SIZE];

    result=0;
    send_command(command+1, response);  // SKIP the leading '*'
    fprintf(stderr,"response to STAR message: [%s]\n",response);
    snprintf (buf, sizeof(buf), "%s %s", command, response ); // insert a leading '*' in answer 
    strcpy (command, buf);                                    // attach the answer              
    result=0;

    return result;
}


/* --------------------------------------------------------------------------*/
/** 
* @brief Process the ozy input buffer
* 
* @param buffer
*/
void process_ozy_input_buffer(char* buffer) {
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Get the iq samples
* 
* @param samples
*/
void getSpectrumSamples(char *samples) {
    memcpy(samples,iq_samples,SPECTRUM_BUFFER_SIZE);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the speed
* 
* @param speed
*/
void setSpeed(int s) {
	fprintf(stderr,"setSpeed %d\n",s);
	fprintf(stderr,"LO_offset %f\n",LO_offset);

    	sampleRate=s;

	SetSampleRate((double)sampleRate);

	SetRXOsc(0,0, -LO_offset);
	SetRXOsc(0,1, -LO_offset);
	SetTXOsc(1, -LO_offset);

	fprintf(stderr,"%s: %f\n", __FUNCTION__, (double) sampleRate);
	ozy_set_src_ratio();
    //
    // because in HPSDR the TX I/Q stream is expected @ 48 kS/s 
    // indipendently from the RX sample rate, it would seem that here we have to
    // set the transmission resampler is a fixed and different way that when we are using softrocks.
    // However, because into the hpsdr_server process there is in place a crude but nonetheless effective  
    // resampling, the resampling ratio is exactly the same.
    // (see also the function process_ozy_output_buffer in trunk/src/server/ozy.c)
    //
    mic_src_ratio = (double) sampleRate/ 8000.0;
	fprintf(stderr,"%s: mic source ratio: %f\n", __FUNCTION__, (double) mic_src_ratio);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Initialize Ozy Server
* 
* @return 
*/
int ozy_init(const char *server_address, const char *dspserver_address) {
    int rc;
    struct hostent *h;
    int on=1;

    rc=sem_init(&ozy_send_semaphore,0,1);
    if(rc<0) {
        perror("sem_init failed");
    }

    rc = sem_init(&ozy_cmd_semaphore, 0, 1);
    if (rc < 0) {
        perror("ozy command semaphore init failed");
    }

    h=gethostbyname(server_address);
    if(h==NULL) {
        fprintf(stderr,"ozy_init: unknown host %s\n",server_address);
        exit(1);
    }

    server_port=11000;

    // create a socket to send commands to the server
    command_socket=socket(AF_INET,SOCK_STREAM,0);
    if(command_socket<0) {
        perror("ozy_init: create command socket failed");
        exit(1);
    }
   
    setsockopt(command_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&command_addr,0,command_length);

    command_addr.sin_family=AF_INET;
    fprintf(stderr,"ozy_init: command address: %s\n", dspserver_address);
    command_addr.sin_addr.s_addr=inet_addr(dspserver_address);
    command_addr.sin_port=htons(COMMAND_PORT+(receiver*2));

    if(bind(command_socket,(struct sockaddr*)&command_addr,command_length)<0) {
        perror("ozy_init: bind socket failed for command socket");
        exit(1);
    }

    fprintf(stderr,"ozy_init: command bound to port %d socket %d\n",ntohs(command_addr.sin_port),command_socket);



    // create a socket to send audio to the server
    audio_socket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(audio_socket<0) {
        perror("ozy_init: create audio socket failed");
        exit(1);
    }
    // setsockopt(audio_socket, SOL_SOCKET, SO_REUSEADDR, &audio_on, sizeof(audio_on));

    memset(&audio_addr,0,audio_length);
    audio_addr.sin_family=AF_INET;
    fprintf(stderr,"ozy_init: audio address: %s\n", dspserver_address);
    audio_addr.sin_addr.s_addr = inet_addr(dspserver_address);
    audio_addr.sin_port=htons(0);

   if(bind(audio_socket,(struct sockaddr*)&audio_addr,audio_length)<0) {
        perror("ozy_init: bind socket failed for audio socket");
        exit(1);
    }

    fprintf(stderr,"ozy_init: audio bound to port %d socket %d\n",ntohs(audio_addr.sin_port),audio_socket);


    memset(&server_audio_addr,0,server_audio_length);
    server_audio_addr.sin_family=h->h_addrtype;
    memcpy((char *)&server_audio_addr.sin_addr.s_addr,h->h_addr_list[0],h->h_length);
    server_audio_addr.sin_port=htons(AUDIO_PORT+(receiver*2));

    fprintf(stderr,"ozy_init: server audio is in port %d\n",ntohs(server_audio_addr.sin_port));

    // setup the server address and command port
    memset(&server_addr,0,server_length);
    server_addr.sin_family=h->h_addrtype;
    memcpy((char *)&server_addr.sin_addr.s_addr,h->h_addr_list[0],h->h_length);
    server_addr.sin_port=htons(server_port);


    fprintf(stderr,"ozy_init: server %s\n",server_address);

    // connect
    rc=connect(command_socket,(struct sockaddr*)&server_addr,server_length);
    if(rc<0) {
        perror("ozy_init: connect failed");
        exit(1);
    }

    if(make_connection()) {
        fprintf(stderr,"connect failed\n");
        exit(1);
    }

        // create sample rate subobject
        ozy_set_src_ratio();
        int sr_error;
        sr_state = src_new (
                             //SRC_SINC_BEST_QUALITY,  // NOT USABLE AT ALL on Atom 300 !!!!!!!
                             //SRC_SINC_MEDIUM_QUALITY,
                             SRC_SINC_FASTEST,
                             //SRC_ZERO_ORDER_HOLD,
                             //SRC_LINEAR,
                             2, &sr_error
                           ) ;

        if (sr_state == 0) { 
            fprintf (stderr, "ozy_init: SR INIT ERROR: %s\n", src_strerror (sr_error)); 
        } else {
            fprintf (stderr, "ozy_init: sample rate init successfully at ratio: %f\n", src_ratio); 
        }


    // create a thread to listen for iq frames
    rc=pthread_create(&iq_thread_id,NULL,iq_thread,(void*)dspserver_address);
    if(rc != 0) {
        fprintf(stderr,"pthread_create failed on iq_thread: rc=%d\n", rc);
    }

    return rc;
}

void ozy_set_src_ratio(void){
	src_ratio = (double)audiostream_conf.samplerate / ((double) sampleRate);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Close ozy
* 
* @return 
*/
void ozyClose() {
/*
    if(port_audio) {
        close_port_audio();
    }
*/
    src_delete(sr_state);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Get the ADC Overflow 
* 
* @return 
*/
int getADCOverflow() {
    int result=0;
    return result;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief save Ozy state
* 
* @return 
*/
void ozySaveState() {
}

/* --------------------------------------------------------------------------*/
/** 
* @brief resore Ozy state
* 
* @return 
*/
void ozyRestoreState() {
}


void ozy_set_local_audio(int state) {
    local_audio=state;
}

void ozy_set_port_audio(int state) {
    port_audio=state;
}

void ozy_set_debug(int state) {
    ozy_debug=state;
}

void dump_udp_buffer(unsigned char* buffer) {
    int i;
    fprintf(stderr, "udp ...\n");
    for(i=0;i<512;i+=16) {
        fprintf(stderr, "  [%04X] %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X\n",
                i,
                buffer[i],buffer[i+1],buffer[i+2],buffer[i+3],buffer[i+4],buffer[i+5],buffer[i+6],buffer[i+7],
                buffer[i+8],buffer[i+9],buffer[i+10],buffer[i+11],buffer[i+12],buffer[i+13],buffer[i+14],buffer[i+15]
                );
    }
    fprintf(stderr,"\n");
}

void ozy_set_hpsdr() {
    hpsdr=1;
}

void ozy_set_hpsdr_local() {
    ozy_set_hpsdr();
    hpsdr_local=1;
}

