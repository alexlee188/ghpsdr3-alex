/** 
* @file client.c
* @brief client network interface
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-12
*/
// client.c

/* Copyright (C) 
* 2009 - John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>
#include <samplerate.h>
#ifdef _OPENMP
#include <omp.h>
#endif
/* For fcntl */
#include <fcntl.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "client.h"
#include "ozy.h"
#include "audiostream.h"
#include "main.h"
#include "soundcard.h"
#include "dttsp.h"
#include "buffer.h"
#include "codec2loc.h"
#include "register.h"
#include "rtp.h"
#include "G711A.h"

static int timing=0;

static int rtp_tx_init_done = 0;

static pthread_t client_thread_id, tx_thread_id, rtp_tx_thread_id, memory_thread_id;

#define BASE_PORT 8000
static int port=BASE_PORT;

static int serverSocket;
static int clientSocket;
static struct sockaddr_in server;
static struct sockaddr_in client;

#define SAMPLE_BUFFER_SIZE 4096
static float spectrumBuffer[SAMPLE_BUFFER_SIZE];

#define TX_BUFFER_SIZE 1024
// same as BUFFER_SIZE defined in softrock server
// format is float left_buffer[BUFFER_SIZE] followed by right_buffer[BUFFER_SIZE] non-interleaved
static float tx_buffer[TX_BUFFER_SIZE*2];
static float tx_IQ_buffer[TX_BUFFER_SIZE*2];

#define MIC_NO_OF_FRAMES 4
#define MIC_BUFFER_SIZE  (BITS_SIZE*MIC_NO_OF_FRAMES)
static unsigned char mic_buffer[MIC_BUFFER_SIZE];

#define RTP_BUFFER_SIZE 400
#define RTP_TIMER_NS (RTP_BUFFER_SIZE/8 * 1000000)
static unsigned char rtp_buffer[RTP_BUFFER_SIZE];
int data_in_counter=0;
int iq_buffer_counter = 0;

#define MSG_SIZE 64

// IQ_audio_stream is the HEAD of a queue
// Rx IQ from soundcard is added to the tail of this stream from the IQ thread
// data from head of this queue is sent to QtRadio client in the client thread
TAILQ_HEAD(, audio_entry) IQ_audio_stream;

// Mic_audio_stream is the HEAD of a queue for encoded Mic audio samples from QtRadio
TAILQ_HEAD(, audio_entry) Mic_audio_stream;

// Mic_rtp_stream is the HEAD of a queue for encoded Mic audio samples with RTP from QtRadio
TAILQ_HEAD(, audio_entry) Mic_rtp_stream;

// Client_list is the HEAD of a queue of connected clients
TAILQ_HEAD(, _client_entry) Client_list;

// Number of memory chunks to keep, exceeding which to free
#define MEMORY_LIMIT 50
// Mem_Pool is the HEAD of a queue of memory pool allocated with malloc, to be free()'d with delay
TAILQ_HEAD(, _memory_entry) Memory_Pool;

//
// samplerate library data structures
//
SRC_STATE *mic_sr_state;
double mic_src_ratio;

static float meter;
static float subrx_meter;
int encoding = 0;

static sem_t bufferevent_semaphore, mic_semaphore, memory_semaphore;

void* client_thread(void* arg);
void* tx_thread(void* arg);
void *rtp_tx_thread(void *arg);
int local_rtp_port = LOCAL_RTP_PORT;

void client_send_samples(int size);
void client_set_samples(float* samples,int size);

char* client_samples;
int samples;
int prncountry = -1;
char status_buf[32];

float getFilterSizeCalibrationOffset() {
    int size=1024; // dspBufferSize
    float i=log10((float)size);
    return 3.0f*(11.0f-i);
}

void audio_stream_init(int receiver) {
    init_alaw_tables();
    TAILQ_INIT(&IQ_audio_stream);
    TAILQ_INIT(&Mic_audio_stream);
}

void audio_stream_queue_add(int length) {
    struct audio_entry *item;

        if(send_audio) {
 	    item = malloc(sizeof(*item));
	    item->buf = audio_buffer;
	    item->length = length;
	    sem_wait(&bufferevent_semaphore);
	    TAILQ_INSERT_TAIL(&IQ_audio_stream, item, entries);
	    sem_post(&bufferevent_semaphore);
            allocate_audio_buffer();		// audio_buffer passed on to IQ_audio_stream.  Need new ones.
        }
}

struct audio_entry *audio_stream_queue_remove(){
	struct audio_entry *first_item;
	sem_wait(&bufferevent_semaphore);
	first_item = TAILQ_FIRST(&IQ_audio_stream);
	if (first_item != NULL) TAILQ_REMOVE(&IQ_audio_stream, first_item, entries);
	sem_post(&bufferevent_semaphore);
	return first_item;
}

void audio_stream_queue_free(){
	struct audio_entry *item;

	sem_wait(&bufferevent_semaphore);
	while ((item = TAILQ_FIRST(&IQ_audio_stream)) != NULL){
		TAILQ_REMOVE(&IQ_audio_stream, item, entries);
		free(item->buf);
		free(item);
		}
	sem_post(&bufferevent_semaphore);
}

// this is run from the client thread
void Mic_stream_queue_add(){
   unsigned char *bits;
   struct audio_entry *item;
   int i;

	for (i=0; i < MIC_NO_OF_FRAMES; i++){

		bits = malloc(BITS_SIZE);
		memcpy(bits, &mic_buffer[i*BITS_SIZE], BITS_SIZE);
		item = malloc(sizeof(*item));
		item->buf = bits;
		item->length = BITS_SIZE;
		sem_wait(&mic_semaphore);
		TAILQ_INSERT_TAIL(&Mic_audio_stream, item, entries);
		sem_post(&mic_semaphore);
	}
}

void Mic_stream_queue_free(){
	struct audio_entry *item;

	sem_wait(&mic_semaphore);
	while ((item = TAILQ_FIRST(&Mic_audio_stream)) != NULL) {
		TAILQ_REMOVE(&Mic_audio_stream, item, entries);
		free(item->buf);
		free(item);
		}
	sem_post(&mic_semaphore);
}

void client_init(int receiver) {
    int rc;

    TAILQ_INIT(&Client_list);
    TAILQ_INIT(&Memory_Pool);

    sem_init(&bufferevent_semaphore,0,1);
    sem_init(&mic_semaphore,0,1);
    sem_init(&memory_semaphore,0,1);
    signal(SIGPIPE, SIG_IGN);
    sem_post(&bufferevent_semaphore);
    sem_post(&mic_semaphore);
    sem_post(&memory_semaphore);
    rtp_init();

    port=BASE_PORT+receiver;
    clientSocket=-1;
    rc=pthread_create(&client_thread_id,NULL,client_thread,NULL);

    if(rc != 0) {
        fprintf(stderr,"pthread_create failed on client_thread: rc=%d\n", rc);
    }
    else rc=pthread_detach(client_thread_id);

    rc=pthread_create(&memory_thread_id,NULL,memory_thread,NULL);

    if(rc != 0) {
        fprintf(stderr,"pthread_create failed on memory_thread: rc=%d\n", rc);
    }
    else rc=pthread_detach(memory_thread_id);
}

void *memory_thread(void *arg) {
    memory_entry *item;
    int memory_count;
    int to_free_count;
    int i;

    fprintf(stderr, "memory_thread started...\n");
    while (1){

	usleep(100000);	// sleep 100ms
	memory_count = 0;

	sem_wait(&memory_semaphore);
	TAILQ_FOREACH(item, &Memory_Pool, entries){
		memory_count++;
	}
	if (memory_count > MEMORY_LIMIT){
		to_free_count = memory_count - MEMORY_LIMIT;
		for (i=0; i< to_free_count; i++){
			item = TAILQ_FIRST(&Memory_Pool);
			if (item != NULL){			// should not happen, but check anyway
				TAILQ_REMOVE(&Memory_Pool, item, entries);
				if (item->memory != NULL) free(item->memory);
				free(item);
			}
		}
	}
	sem_post(&memory_semaphore);
    }
}

void tx_init(void){
    int rc;

	mic_src_ratio = (double) sampleRate / 8000.0;
       // create sample rate subobject
        int sr_error;
        mic_sr_state = src_new (
                             //SRC_SINC_BEST_QUALITY,  // NOT USABLE AT ALL on Atom 300 !!!!!!!
                             //SRC_SINC_MEDIUM_QUALITY,
                             SRC_SINC_FASTEST,
                             //SRC_ZERO_ORDER_HOLD,
                             //SRC_LINEAR,
                             2, &sr_error
                           ) ;

        if (mic_sr_state == 0) { 
            fprintf (stderr, "tx_init: SR INIT ERROR: %s\n", src_strerror (sr_error)); 
        } else {
            fprintf (stderr, "tx_init: sample rate init successful with ratio of : %f\n", mic_src_ratio);
	}

        rc=pthread_create(&tx_thread_id,NULL,tx_thread,NULL);
        if(rc != 0) fprintf(stderr,"pthread_create failed on tx_thread: rc=%d\n", rc);
	else rc=pthread_detach(tx_thread_id);
}

void rtp_tx_timer_handler(int);

void rtp_tx_init(void){
	int rc;

	if (rtp_tx_init_done == 0){

		TAILQ_INIT(&Mic_rtp_stream);
		rc=pthread_create(&rtp_tx_thread_id,NULL,rtp_tx_thread,NULL);
		if(rc != 0) fprintf(stderr,"pthread_create failed on rtp_tx_thread: rc=%d\n", rc);
		else rc=pthread_detach(rtp_tx_thread_id);

		rtp_tx_init_done = 1;

		timer_t timerid;
		struct itimerspec	value;
		struct sigevent sev;

		value.it_value.tv_sec = 0;
		value.it_value.tv_nsec = RTP_TIMER_NS;

		value.it_interval.tv_sec = 0;
		value.it_interval.tv_nsec = RTP_TIMER_NS;

		sev.sigev_notify = SIGEV_THREAD;
		sev.sigev_notify_function = rtp_tx_timer_handler;
		sev.sigev_notify_attributes = NULL;

		timer_create (CLOCK_REALTIME, &sev, &timerid);
		timer_settime (timerid, 0, &value, NULL);

	}
}

void rtp_tx_timer_handler(int sv){
    int i;
    short v;
    float fv;
    int length;
    float data_in [TX_BUFFER_SIZE*2]; 		// stereo
    float *mic_data;
    struct audio_entry *item;
    client_entry *client_item;

    if ((client_item = TAILQ_FIRST(&Client_list)) == NULL) return;	// no master client
    if (client_item->rtp != connection_rtp) return;			// not rtp master
	length=rtp_receive(client_item->session,rtp_buffer,RTP_BUFFER_SIZE);
	recv_ts+=RTP_BUFFER_SIZE;		// proceed with timestamp increment as this is timer based	        
	if (length > 0){
	    for(i=0;i<length;i++) {
		v=G711A_decode(rtp_buffer[i]);
		fv=(float)v/32767.0F;   // get into the range -1..+1

		data_in[data_in_counter*2]=fv;
		data_in[(data_in_counter*2)+1]=fv;
		data_in_counter++;

		if (data_in_counter >= TX_BUFFER_SIZE) {
			mic_data = (float*) malloc(TX_BUFFER_SIZE * 2 * sizeof(float));
			memcpy(mic_data, data_in, TX_BUFFER_SIZE*2*sizeof(float));
			item = malloc(sizeof(*item));
			item->buf = (unsigned char*) mic_data;
			item->length = TX_BUFFER_SIZE;
			sem_wait(&mic_semaphore);
			TAILQ_INSERT_TAIL(&Mic_rtp_stream, item, entries);
			sem_post(&mic_semaphore);
			data_in_counter = 0;
		}
	    }
	}
}

void* rtp_tx_thread(void *arg){
    int j;
    float data_out[TX_BUFFER_SIZE*2*24];	// data_in is 8khz (duplicated to stereo) Mic samples.  
						// May be resampled to 192khz or 24x stereo
    SRC_DATA data;
    int rc;
    struct audio_entry *item;

    while (1){
	sem_wait(&mic_semaphore);
	item = TAILQ_FIRST(&Mic_rtp_stream);
	sem_post(&mic_semaphore);
	if (item == NULL){
		usleep(100);
		continue;
		}

            // resample to the sample rate
            data.data_in = (float *)item->buf;
            data.input_frames = TX_BUFFER_SIZE;
            data.data_out = data_out;
            data.output_frames = TX_BUFFER_SIZE*24 ;
            data.src_ratio = mic_src_ratio;
            data.end_of_input = 0;

            rc = src_process (mic_sr_state, &data);
            if (rc) {
                fprintf (stderr,"rtp_tx_thread: SRATE: error: %s (rc=%d)\n", src_strerror (rc), rc);
            } else {
                for (j=0;j< data.output_frames_gen;j++) {
                    // tx_buffer is non-interleaved, LEFT followed by RIGHT data
                    tx_buffer[iq_buffer_counter] = data_out[2*j];
                    tx_buffer[iq_buffer_counter + TX_BUFFER_SIZE] = data_out[(2*j)+1];
                    iq_buffer_counter++;
                    if(iq_buffer_counter>=TX_BUFFER_SIZE) {
                        // use DttSP to process Mic data into tx IQ
                    	if(!hpsdr || mox) {
                            Audio_Callback(tx_buffer, &tx_buffer[TX_BUFFER_SIZE], tx_IQ_buffer, &tx_IQ_buffer[TX_BUFFER_SIZE], TX_BUFFER_SIZE, 1);
                            // send Tx IQ to server, buffer is non-interleaved.
                            ozy_send((unsigned char *)tx_IQ_buffer,sizeof(tx_IQ_buffer),"client");
                        }
                        iq_buffer_counter=0;
                    } // end iq_bufer_counter
                } // end for j
            } // end rc else
 	    sem_wait(&mic_semaphore);
	    TAILQ_REMOVE(&Mic_rtp_stream, item, entries);
	    sem_post(&mic_semaphore);
	    free(item->buf);
	    free(item);
    } // end while (1)
}


void *tx_thread(void *arg){
   unsigned char *bits;
   struct audio_entry *item;
   short codec2_buffer[CODEC2_SAMPLES_PER_FRAME];
   int tx_buffer_counter = 0;
   int rc;
   int j, i;
   float data_in [CODEC2_SAMPLES_PER_FRAME*2];		// stereo
   float data_out[CODEC2_SAMPLES_PER_FRAME*2*24];	// 192khz/8khz
   SRC_DATA data;
   void *mic_codec2 = codec2_create();

    while (1){
	sem_wait(&mic_semaphore);
	item = TAILQ_FIRST(&Mic_audio_stream);
	sem_post(&mic_semaphore);
	if (item == NULL){
		usleep(1000);
		continue;
		}
	else {
	   bits = item->buf;		// each frame is BITS_SIZE long
	   // process codec2 encoded mic_buffer
	   codec2_decode(mic_codec2, codec2_buffer, bits);
	   // mic data is mono, so copy to both right and left channels
	   #pragma omp parallel for schedule(static) private(j) 
           for (j=0; j < CODEC2_SAMPLES_PER_FRAME; j++) {
              data_in [j*2] = data_in [j*2+1]   = (float)codec2_buffer[j]/32767.0;
           }

           data.data_in = data_in;
           data.input_frames = CODEC2_SAMPLES_PER_FRAME;
           data.data_out = data_out;
           data.output_frames = CODEC2_SAMPLES_PER_FRAME*24 ;
           data.src_ratio = mic_src_ratio;
           data.end_of_input = 0;

           rc = src_process (mic_sr_state, &data);
           if (rc) {
               fprintf (stderr,"SRATE: error: %s (rc=%d)\n", src_strerror (rc), rc);
           } else {
		for (i=0; i < data.output_frames_gen; i++){
			// tx_buffer is non-interleaved, LEFT followed by RIGHT data
			tx_buffer[tx_buffer_counter] = data_out[2*i];
			tx_buffer[tx_buffer_counter + TX_BUFFER_SIZE] = data_out[2*i+1];
			tx_buffer_counter++;
			if (tx_buffer_counter >= TX_BUFFER_SIZE){
				// use DttSP to process Mic data into tx IQ
				Audio_Callback(tx_buffer, &tx_buffer[TX_BUFFER_SIZE], tx_IQ_buffer, &tx_IQ_buffer[TX_BUFFER_SIZE], TX_BUFFER_SIZE, 1);
				// send Tx IQ to server, buffer is non-interleaved.
                                ozy_send((unsigned char *)tx_IQ_buffer,sizeof(tx_IQ_buffer),"client");
				tx_buffer_counter = 0;
			}
		} // end for i
	    } // end else rc
	    sem_wait(&mic_semaphore);
	    TAILQ_REMOVE(&Mic_audio_stream, item, entries);
	    sem_post(&mic_semaphore);
	    free(item->buf);
	    free(item);
	  } // end else item
	} // end while

	codec2_destroy(mic_codec2);
}

void client_set_timing() {
    timing=1;
}


void do_accept(evutil_socket_t listener, short event, void *arg);
void readcb(struct bufferevent *bev, void *ctx);
void writecb(struct bufferevent *bev, void *ctx);

void
errorcb(struct bufferevent *bev, short error, void *ctx)
{
    client_entry *item, *tmp_item;
    int client_count = 0;
    int rtp_client_count = 0;
    int is_rtp_client = 0;

    if (error & BEV_EVENT_EOF) {
        /* connection has been closed, do any clean up here */
        /* ... */
    } else if (error & BEV_EVENT_ERROR) {
        /* check errno to see what error occurred */
        /* ... */
    } else if (error & BEV_EVENT_TIMEOUT) {
        /* must be a timeout event handle, handle it */
        /* ... */
    }

    time_t tt;
    struct tm *tod;
    time(&tt);
    tod=localtime(&tt);

    for (item = TAILQ_FIRST(&Client_list); item != NULL; item = tmp_item){
	tmp_item = TAILQ_NEXT(item, entries);
	if (item->bev == bev){
	    	fprintf(stderr,"%02d/%02d/%02d %02d:%02d:%02d RX%d: client disconnection from %s:%d\n",
			tod->tm_mday,tod->tm_mon+1,tod->tm_year+1900,tod->tm_hour,tod->tm_min,tod->tm_sec,
			receiver,inet_ntoa(item->client.sin_addr),ntohs(item->client.sin_port));
		if (item->rtp == connection_rtp) {
			rtp_disconnect(item->session);
			is_rtp_client = 1;
		}
		TAILQ_REMOVE(&Client_list, item, entries);
		free(item);
		break;
	}
    }

    TAILQ_FOREACH(item, &Client_list, entries){
	client_count++;
	if (item->rtp == connection_rtp) rtp_client_count++;
    }

    if ((rtp_client_count <= 0) && is_rtp_client) {
	rtp_connected = 0; 	// last rtp client disconnected
	rtp_listening = 0;
    }

    sprintf(status_buf,"%d client(s)", client_count);
    if (toShareOrNotToShare) updateStatus(status_buf);

    if (client_count <= 0) send_audio = 0;
    bufferevent_free(bev);
}



void* client_thread(void* arg) {
 
    int on=1;
    struct event_base *base;
    struct event *listener_event;

    fprintf(stderr,"client_thread\n");

    serverSocket=socket(AF_INET,SOCK_STREAM,0);
    if(serverSocket==-1) {
        perror("client socket");
        return NULL;
    }

    // set timeout on receive
    struct timeval tv;
    tv.tv_sec=10;		// changed from 3 sec to 10 sec for remote connection
    tv.tv_usec=0;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO,(char *)&tv,sizeof tv);

    evutil_make_socket_nonblocking(serverSocket);

#ifndef WIN32
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
#endif

    memset(&server,0,sizeof(server));
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=htonl(INADDR_ANY);
    server.sin_port=htons(port);

    if(bind(serverSocket,(struct sockaddr *)&server,sizeof(server))<0) {
        perror("client bind");
        fprintf(stderr,"port=%d\n",port);
        return NULL;
    }

    fprintf(stderr,"client_thread: listening on port %d\n",port);

    if (listen(serverSocket, 5) == -1) {
	perror("client listen");
	exit(1);
    }

    base = event_base_new();
    listener_event = event_new(base, serverSocket, EV_READ|EV_PERSIST, do_accept, (void*)base);

    event_add(listener_event, NULL);
    event_base_dispatch(base);
    return NULL;
}

void do_accept(evutil_socket_t listener, short event, void *arg){

    client_entry *item;
    struct event_base *base = arg;
    struct sockaddr_in ss;
    socklen_t slen = sizeof(ss);
    int client_count = 0;

    int fd = accept(listener, (struct sockaddr*)&ss, &slen);
    if (fd < 0) {
        fprintf(stderr,"accept failed\n");
    } else {
	// add newly connected client to Client_list
	item = malloc(sizeof(*item));
	memcpy(&item->client, &ss, sizeof(ss));

        time_t tt;
        struct tm *tod;
        time(&tt);
        tod=localtime(&tt);
        fprintf(stderr,"%02d/%02d/%02d %02d:%02d:%02d RX%d: client connection from %s:%d\n",
		tod->tm_mday,tod->tm_mon+1,tod->tm_year+1900,tod->tm_hour,tod->tm_min,tod->tm_sec,
		receiver,inet_ntoa(item->client.sin_addr),ntohs(item->client.sin_port));
        if(prncountry == 0){
	    	memcpy(&client, &ss, sizeof(client));
                printcountry();
        }

        struct bufferevent *bev;
        evutil_make_socket_nonblocking(fd);
        bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
        bufferevent_setcb(bev, readcb, writecb, errorcb, NULL);
        bufferevent_setwatermark(bev, EV_READ, MSG_SIZE, 0);
	bufferevent_setwatermark(bev, EV_WRITE, 4096, 0);
        bufferevent_enable(bev, EV_READ|EV_WRITE);
	item->bev = bev;
	item->rtp = connection_unknown;
	TAILQ_INSERT_TAIL(&Client_list, item, entries);
	TAILQ_FOREACH(item, &Client_list, entries){
		client_count++;
	}
	sprintf(status_buf,"%d client(s)", client_count);
        if (toShareOrNotToShare) updateStatus(status_buf);
    }
}

void writecb(struct bufferevent *bev, void *ctx){
	struct audio_entry *item;
	client_entry *client_item;

	sem_wait(&bufferevent_semaphore);
	while ((item = audio_stream_queue_remove()) != NULL){
		TAILQ_FOREACH(client_item, &Client_list, entries){
                        if(client_item->rtp == connection_tcp) {
			    bufferevent_write(client_item->bev, item->buf, item->length);
                            }
			else if (client_item->rtp == connection_rtp)
				rtp_send(client_item->session,&item->buf[AUDIO_BUFFER_HEADER_SIZE], (item->length - AUDIO_BUFFER_HEADER_SIZE));
		}
 
		send_ts += item->length - AUDIO_BUFFER_HEADER_SIZE; // update send_ts for all rtp sessions
		free(item->buf);
		free(item);
	}
	sem_post(&bufferevent_semaphore);
}

void readcb(struct bufferevent *bev, void *ctx){
    char *token, *saveptr;
    int i;
    int bytesRead = 0;
    char message[MSG_SIZE];
    client_entry *item, *current_item, *tmp_item;
    time_t tt;
    struct tm *tod;

	if ((item = TAILQ_FIRST(&Client_list)) == NULL) return;	// should not happen.  No clients !!!
	if (item->bev != bev){					// only allow the first client on Client_list to command dspserver as master
	// locate the current_item for this slave client
	    for (current_item = TAILQ_FIRST(&Client_list); current_item != NULL; current_item = tmp_item){
		tmp_item = TAILQ_NEXT(current_item, entries);
		if (current_item->bev == bev){
			break;
		}
	    }
	    if (current_item == NULL) return; // should not happen.

		while ((bytesRead = bufferevent_read(bev, message, MSG_SIZE)) > 3){
		        message[bytesRead-1]=0;					// for Linux strings terminating in NULL
		        token=strtok_r(message," ",&saveptr);
                  	if(token!=NULL) {
		            	i=0;
		            	while(token[i]!=0) {
		               	token[i]=tolower(token[i]);
		               	i++;
                    		}
                                if(strncmp(token,"q",1)==0){	
				   answer_question(message,"slave", bev);
				}else if(strncmp(token,"getspectrum",11)==0) {
				        token=strtok_r(NULL," ",&saveptr);
				        if(token!=NULL) {
		                    	    samples=atoi(token);
                                            if(mox) {
					        Process_Panadapter(1,spectrumBuffer);
					        meter=CalculateTXMeter(1,5); // MIC
					        subrx_meter=-121;
                                            } else {
					        Process_Panadapter(0,spectrumBuffer);
					        meter=CalculateRXMeter(0,0,0)+multimeterCalibrationOffset+getFilterSizeCalibrationOffset();
					        subrx_meter=CalculateRXMeter(0,1,0)+multimeterCalibrationOffset+getFilterSizeCalibrationOffset();
                                            }
					    client_samples=malloc(BUFFER_HEADER_SIZE+samples);
					    client_set_samples(spectrumBuffer,samples);
					    bufferevent_write(bev, client_samples, BUFFER_HEADER_SIZE+samples);
					    free(client_samples);
		                	    } else {
			 		    fprintf(stderr,"Invalid command: '%s'\n",message);
					    }
				} else if(strncmp(token,"setclient",9)==0) {
                                    token=strtok_r(NULL," ",&saveptr);
                                    if(token!=NULL) {
                                        time(&tt);
                                        tod=localtime(&tt);
                                        fprintf(stdout,"%02d/%02d/%02d %02d:%02d:%02d RX%d: client is %s\n",tod->tm_mday,tod->tm_mon+1,tod->tm_year+1900,tod->tm_hour,tod->tm_min,tod->tm_sec,receiver,token);
                                    }
		            	} else if(strncmp(token,"startaudiostream",16)==0) {
				    current_item->rtp = connection_tcp;
		                } else if(strncmp(token,"startrtpstream",14)==0) {
				    current_item->rtp = connection_rtp;
				    
	                        // startrtpstream port encoding samplerate channels				
	                        int error=1;
	                        token=strtok_r(NULL," ",&saveptr);
	                        if(token!=NULL) {
	                            int rtpport=atoi(token);
	                            token=strtok_r(NULL," ",&saveptr);
	                            if(token!=NULL) {
	                                encoding=atoi(token);
	                                token=strtok_r(NULL," ",&saveptr);
	                                if(token!=NULL) {
	                                    audio_sample_rate=atoi(token);
	                                    token=strtok_r(NULL," ",&saveptr);
	                                    if(token!=NULL) {
	                                        audio_channels=atoi(token);
                                        	fprintf(stdout,"%s: %d: startrtpstream: listening on RTP socket\n", __FILE__, __LINE__);
	                                        current_item->session=rtp_listen(inet_ntoa(current_item->client.sin_addr),rtpport);
						answer_question("q-rtpport","slave", bev); // this answers the LOCAL_RTP_PORT
						audio_stream_reset();
	                                        error=0;
	                                        send_audio=1;
	                                        rtp_tx_init();
	                                    }
	                                }
	                            }
	                        }
	                        if(error) {
	                            fprintf(stderr,"Invalid command: '%s'\n",message);
	                  	    }
				} // end startrtpstream
			} // end if !=NULL
		}; // end while
		return;
	}; // end if item->bev != bev

	// So this is the master client
	while ((bytesRead = bufferevent_read(bev, message, MSG_SIZE)) > 3){

                message[bytesRead-1]=0;			// for Linux strings terminating in NULL
                token=strtok_r(message," ",&saveptr);
 		if (token == NULL) continue;

                    if(strncmp(token,"mic", 3)==0){		// This is incoming microphone data, binary data after "mic "
			memcpy(mic_buffer, &message[4], MIC_BUFFER_SIZE);
			Mic_stream_queue_add();
		    }
                    else {	// not mic data, process other commands which are ascii
                    if(token!=NULL) {
		            	i=0;
		            	while(token[i]!=0) {
		               	token[i]=tolower(token[i]);
		               	i++;
                    		}
                        if(strncmp(token,"q",1)==0){	
			    answer_question(message,"master", bev);
 			}else if(strncmp(token,"getspectrum",11)==0) {
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
                            	    samples=atoi(token);
                                    if(mox) {
				        Process_Panadapter(1,spectrumBuffer);
					meter=CalculateTXMeter(1,5); // MIC
				        subrx_meter=-121;
                                    } else {
				        Process_Panadapter(0,spectrumBuffer);
				        meter=CalculateRXMeter(0,0,0)+multimeterCalibrationOffset+getFilterSizeCalibrationOffset();
				        subrx_meter=CalculateRXMeter(0,1,0)+multimeterCalibrationOffset+getFilterSizeCalibrationOffset();
                                    }
				    client_samples=malloc(BUFFER_HEADER_SIZE+samples);
				    client_set_samples(spectrumBuffer,samples);
				    bufferevent_write(bev, client_samples, BUFFER_HEADER_SIZE+samples);
				    free(client_samples);
                        	    } else {
		 		    fprintf(stderr,"Invalid command: '%s'\n",message);
				    }
                    	} else if(strncmp(token,"setfrequency",12)==0) {
                        long long frequency;
                        token=strtok_r(NULL," ",&saveptr);
                        if(token!=NULL) {
                            frequency=atoll(token);
                            ozySetFrequency(frequency);
                        } else {
                            fprintf(stderr,"Invalid command: '%s'\n",message);
                        	}
                    	} else if(strncmp(token,"setpreamp",9)==0) {
                        token=strtok_r(NULL," ",&saveptr);
                        if(token!=NULL) {
                            ozySetPreamp(token);
                        } else {
                            fprintf(stderr,"Invalid command: '%s'\n",message);
                        }
                    	} else if(strncmp(token,"setmode",7)==0) {
		                int mode;
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    mode=atoi(token);
		                    SetMode(0,0,mode);
		                    SetMode(0,1,mode);
				    SetMode(1,0,mode);
				    lastMode=mode;
				    
				    switch (mode){
					case USB: SetTXFilter(1,150, 2850); break;
					case LSB: SetTXFilter(1,-2850, -150); break;
					case AM:
					case SAM: SetTXFilter(1, -2850, 2850); break;
					case FMN: SetTXFilter(1, -4800, 4800); break;
					default: SetTXFilter(1, -4800, 4800);
				    }
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                    	} else if(strncmp(token,"setfilter",9)==0) {
		                int low,high;
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    low=atoi(token);
		                    token=strtok_r(NULL," ",&saveptr);
		                    if(token!=NULL) {
		                      high=atoi(token);
		                      SetRXFilter(0,0,(double)low,(double)high);
		                      SetRXFilter(0,1,(double)low,(double)high);
		                    } else {
		                        fprintf(stderr,"Invalid command: '%s'\n",message);
		                    }
                        } else {
                            fprintf(stderr,"Invalid command: '%s'\n",message);
                            }
                    	} else if(strncmp(token,"setagc",6)==0) {
		                int agc;
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    agc=atoi(token);
		                    SetRXAGC(0,0,agc);
		                    SetRXAGC(0,1,agc);
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                    	} else if(strncmp(token,"setnr",5)==0) {
		                int nr;
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    if(strcmp(token,"true")==0) {
		                        nr=1;
		                    } else {
		                        nr=0;
		                    }
		                    SetNR(0,0,nr);
		                    SetNR(0,1,nr);
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                   	} else if(strncmp(token,"setnb",5)==0) {
		                int nb;
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    if(strcmp(token,"true")==0) {
		                        nb=1;
		                    } else {
		                        nb=0;
		                    }
		                    SetNB(0,0,nb);
		                    SetNB(0,1,nb);
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                    	} else if(strncmp(token,"setsdrom",8)==0) {
		                int state;
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    if(strcmp(token,"true")==0) {
		                        state=1;
		                    } else {
		                        state=0;
		                    }
		                    SetSDROM(0,0,state);
		                    SetSDROM(0,1,state);
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                    	} else if(strncmp(token,"setanf",6)==0) {
		                int anf;
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    if(strcmp(token,"true")==0) {
		                        anf=1;
		                    } else {
		                        anf=0;
		                    }
		                    SetANF(0,0,anf);
		                    SetANF(0,1,anf);
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                    	} else if(strncmp(token,"setrxoutputgain",15)==0) {
		                int gain;
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    gain=atoi(token);
		                    SetRXOutputGain(0,0,(double)gain/100.0);
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                    	} else if(strncmp(token,"setsubrxoutputgain",18)==0) {
		                int gain;
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    gain=atoi(token);
		                    SetRXOutputGain(0,1,(double)gain/100.0);
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                    	} else if(strncmp(token,"startaudiostream",16)==0) {
		                token=strtok_r(NULL," ",&saveptr);
		                if(token==NULL) {
		                    audio_buffer_size= AUDIO_BUFFER_SIZE;
		                } else {
		                    audio_buffer_size=atoi(token);
		                }
		                token=strtok_r(NULL," ",&saveptr);
		                if(token==NULL) {
		                    audio_sample_rate=8000;
		                } else {
		                    audio_sample_rate=atoi(token);
		                    if(audio_sample_rate!=8000 &&
		                       audio_sample_rate!=48000) {
		                        fprintf(stderr,"Invalid audio sample rate: %d\n",audio_sample_rate);
		                        audio_sample_rate=8000;
		                    	}
		                }
		                token=strtok_r(NULL," ",&saveptr);
		                if(token==NULL) {
		                    audio_channels=1;
                        	} else {
                            		audio_channels=atoi(token);
                           		if(audio_channels!=1 &&
                               		audio_channels!=2) {
                                	fprintf(stderr,"Invalid audio channels: %d\n",audio_channels);
                                	audio_channels=1;
                            		}
                        	}
                        
				fprintf(stderr,"starting audio stream at %d with %d channels and buffer size %d\n",audio_sample_rate,audio_channels,audio_buffer_size);
				fprintf(stderr,"and with encoding method %d\n", encoding);
                                item->rtp=connection_tcp;
                        	audio_stream_reset();
                        	send_audio=1;
                        } else if(strncmp(token,"startrtpstream",14)==0) {
                                // startrtpstream port encoding samplerate channels
                                int error=1;
                                token=strtok_r(NULL," ",&saveptr);
                                if(token!=NULL) {
                                    int rtpport=atoi(token);
                                    token=strtok_r(NULL," ",&saveptr);
                                    if(token!=NULL) {
                                        encoding=atoi(token);
                                        token=strtok_r(NULL," ",&saveptr);
                                        if(token!=NULL) {
                                            audio_sample_rate=atoi(token);
                                            token=strtok_r(NULL," ",&saveptr);
                                            if(token!=NULL) {
                                                audio_channels=atoi(token);

fprintf(stderr,"starting rtp: to %s:%d encoding:%d samplerate:%d channels:%d\n",
                inet_ntoa(item->client.sin_addr),rtpport,encoding,audio_sample_rate,audio_channels);
                                                fprintf(stdout,"client.c: startrtpstream port encoding samplerate channels: listening on RTP socket\n");
		                                item->session=rtp_listen(inet_ntoa(item->client.sin_addr),rtpport);
                                                item->rtp=connection_rtp;
						answer_question("q-rtpport","master",bev);
						audio_stream_reset();
                                                error=0;
                                                send_audio=1;
                                                rtp_tx_init();
                                            }
                                        }
                                    }
                                }
                                if(error) {
                                    fprintf(stderr,"Invalid command: '%s'\n",message);
                                }
                        } else if(strncmp(token,"stopaudiostream",15)==0) {
                        	send_audio=0;
                        } else if(strncmp(token,"setencoding",11)==0) {
                        	token=strtok_r(NULL," ",&saveptr);
                        	if(token!=NULL) {
                            		encoding=atoi(token);
			   		if (encoding < 0 || encoding > 2) encoding = 0;
			    		fprintf(stderr,"encoding changed to %d\n", encoding);
                        	} else {
                            	fprintf(stderr,"Invalid command: '%s'\n",message);
                        	}
                        } else if(strncmp(token,"setsubrx",17)==0) {
                        	int state;
                        	token=strtok_r(NULL," ",&saveptr);
                        	if(token!=NULL) {
                            	state=atoi(token);
                            	SetSubRXSt(0,1,state);
				// fprintf(stderr, "setsubrx %d\n", state);
                        	} else {
                            	fprintf(stderr,"Invalid command: '%s'\n",message);
                        	}
		        } else if(strncmp(token,"setsubrxfrequency",17)==0) {
		                int offset;
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    offset=atoi(token);
		                    SetRXOsc(0,1,offset - LO_offset);
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                       } else if(strncmp(token,"setpan",6)==0) {
		                float pan;
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    pan=atof(token);
		                    SetRXPan(0,0,pan);
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                       } else if(strncmp(token,"setsubrxpan",11)==0) {
		                float pan;
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    pan=atof(token);
		                    SetRXPan(0,1,pan);
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                       } else if(strncmp(token,"record",6)==0) {
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    ozySetRecord(token);
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                       } else if(strncmp(token,"setanfvals",10)==0) {
		                int taps;
		                int delay;
		                double gain;
		                double leakage;
		                int error;

		                error=0;
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    taps=atoi(token);
		                } else {
		                    error=1;
		                }
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    delay=atoi(token);
		                } else {
		                    error=1;
		                }
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    gain=atof(token);
		                } else {
		                    error=1;
		                }
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    leakage=atof(token);
		                } else {
		                    error=1;
		                }
		                if(error) {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                } else {
		                    SetANFvals(0,0,taps,delay,gain,leakage);
		                    SetANFvals(0,1,taps,delay,gain,leakage);
		                }
                       } else if(strncmp(token,"setnrvals",9)==0) {
		                int taps;
		                int delay;
		                double gain;
		                double leakage;
		                int error;

		                error=0;
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    taps=atoi(token);
		                } else {
		                    error=1;
		                }
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    delay=atoi(token);
		                } else {
		                    error=1;
		                }
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    gain=atof(token);
		                } else {
		                    error=1;
		                }
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    leakage=atof(token);
		                } else {
		                    error=1;
		                }
		                if(error) {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                } else {
		                    SetNRvals(0,0,taps,delay,gain,leakage);
		                    SetNRvals(0,1,taps,delay,gain,leakage);
		                }
                       } else if(strncmp(token,"setnbvals",9)==0) {
		                double threshold;
		                int error;

		                error=0;
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    threshold=atof(token);
		                } else {
		                    error=1;
		                }
		                if(error) {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                } else {
		                    SetNBvals(0,0,threshold);
		                    SetNBvals(0,1,threshold);
		                }
                       } else if(strncmp(token,"setsdromvals",12)==0) {
		                double threshold;
		                int error;

		                error=0;
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    threshold=atof(token);
		                } else {
		                    error=1;
		                }
		                if(error) {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                } else {
		                    SetSDROMvals(0,0,threshold);
		                    SetSDROMvals(0,1,threshold);
		                }
                       } else if(strncmp(token,"setdcblock",10)==0) {
		                int state;
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    state=atoi(token);
		                    SetRXDCBlock(0,0,state);
		                    SetRXDCBlock(0,1,state);
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                       } else if(strncmp(token,"mox",3)==0) {
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    if(strcmp(token,"on")==0) {
		                        if (txcfg == TXALL){
		                           ozySetMox(1);
		                        }else if(txcfg == TXPASSWD){
									char *thisuser =strtok_r(NULL," ",&saveptr);
		                            if(thisuser!=NULL) {
									   //got user
								       char *thispasswd =strtok_r(NULL," ",&saveptr);
								       if(thispasswd!=NULL) {
										   //got passwd
										   //check password
										   if(chkPasswd(thisuser, thispasswd) == 0){
											   // password OK check freq
											   if (chkFreq(thisuser,  lastFreq , lastMode) == 0){
												   //freqchk passwd good to tx
												   fprintf(stderr,"Mox on from User:%s\n",thisuser);
												   ozySetMox(1);
											   }else{
												   fprintf(stderr,"Mox denied because user %s has no rule for mode %d on %lld! hz\n",thisuser, lastMode,lastFreq);
											   }
										   }else{
											   fprintf(stderr,"Mox on denied because user %s password check failed!\n",thisuser);
										   }
									   }
								    }   
									
								}else{
									fprintf(stderr,"mox denied because tx = \"no\"\n");
								}	
		                    } else if(strcmp(token,"off")==0) {
		                       if (txcfg == TXALL){
		                           ozySetMox(0);
		                        }else if(txcfg == TXPASSWD){
									char *thisuser =strtok_r(NULL," ",&saveptr);
		                            if(thisuser!=NULL) {
									   // got user
									   char *thispasswd =strtok_r(NULL," ",&saveptr);
									   if(thispasswd!=NULL) {
										   //got passwd
										   //check password
										   if(chkPasswd(thisuser, thispasswd) == 0){
											  fprintf(stderr,"Mox off from User:%s\n",thisuser);
											  ozySetMox(0);
										   }else{
											   fprintf(stderr,"Mox off denied because user %s password check failed!\n",thisuser);
										   }
										}
									}
								}
		                    } else {
		                        fprintf(stderr,"Invalid command: '%s'\n",message);
		                    }
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                       } else if(strncmp(token,"settxamcarrierlevel",19)==0) {
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    double pwr=atof(token);
		                    char *thisuser =strtok_r(NULL," ",&saveptr);
		                    if(thisuser!=NULL) {
								char *thispasswd =strtok_r(NULL," ",&saveptr);
								if(thispasswd!=NULL) {
									if(chkPasswd(thisuser, thispasswd) == 0){ 
		                               if(pwr >= 0 &&
		                                 pwr <= 1) {
		                                 SetTXAMCarrierLevel(1,pwr);
									   }
									}else{
									    fprintf(stderr,"SetTXAMCarrierLevel denied because user %s password check failed!\n",thisuser);
									}
								}
		                     } else {
		                        fprintf(stderr,"Invalid command arguement: '%s'\n",message);
		                    }
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                       } else if(strncmp(token,"setsquelchval",13)==0) {
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    float value=atof(token);
		                    SetSquelchVal(0,0,value);
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                       } else if(strncmp(token,"setsubrxquelchval",17)==0) {
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    float value=atof(token);
		                    SetSquelchVal(0,1,value);
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                       } else if(strncmp(token,"setsquelchstate",15)==0) {
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    if(strcmp(token,"on")==0) {
		                        SetSquelchState(0,0,1);
		                    } else if(strcmp(token,"off")==0) {
		                        SetSquelchState(0,0,0);
		                    } else {
		                        fprintf(stderr,"Invalid command: '%s'\n",message);
		                    }
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                       } else if(strncmp(token,"setsubrxquelchstate",19)==0) {
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    if(strcmp(token,"on")==0) {
		                        SetSquelchState(0,1,1);
		                    } else if(strcmp(token,"off")==0) {
		                        SetSquelchState(0,1,0);
		                    } else {
		                        fprintf(stderr,"Invalid command: '%s'\n",message);
		                    }
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                       } else if(strncmp(token,"setspectrumpolyphase",20)==0) {
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    if(strcmp(token,"true")==0) {
		                        SetSpectrumPolyphase(0,1);
		                    } else if(strcmp(token,"false")==0) {
		                        SetSpectrumPolyphase(0,0);
		                    } else {
		                        fprintf(stderr,"Invalid command: '%s'\n",message);
		                    }
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                       } else if(strncmp(token,"setocoutputs",12)==0) {
		                token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    ozySetOpenCollectorOutputs(token);
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                       } else if(strncmp(token,"setclient",9)==0) {
                        	token=strtok_r(NULL," ",&saveptr);
                        	if(token!=NULL) {
                            		time(&tt);
                            		tod=localtime(&tt);
                            		fprintf(stdout,"%02d/%02d/%02d %02d:%02d:%02d RX%d: client is %s\n",tod->tm_mday,tod->tm_mon+1,tod->tm_year+1900,tod->tm_hour,tod->tm_min,tod->tm_sec,receiver,token);
                       		}
 			} else if(strncmp(token,"setiqenable",11)==0) {
				token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
		                    if(strcmp(token,"true")==0) {
		                        SetCorrectIQEnable(1);
					fprintf(stderr,"SetCorrectIQEnable(1)\n"); 
		                    } else if(strcmp(token,"false")==0) {
		                        SetCorrectIQEnable(0);
					fprintf(stderr,"SetCorrectIQEnable(0)\n");
				    } else {	
		                        fprintf(stderr,"Invalid command: '%s'\n",message);
				    }
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
			} else if(strncmp(token,"rxiqmuval",9)==0) {
				token=strtok_r(NULL," ",&saveptr);
		                if(token!=NULL) {
				    fprintf(stderr,"The value of mu sent = '%s'\n",token);
		                    SetCorrectRXIQMu(0, 0, atof(token));
		                } else {
		                    fprintf(stderr,"Invalid command: '%s'\n",message);
		                }
                        } else {
		            fprintf(stderr,"Invalid rxiqmuval command: token: '%s'\n",token);
		            }
                } else {
                    fprintf(stderr,"Invalid command: message: '%s'\n",message);
                	}
		} // end if (mic)
	    } // end while
}


void client_set_samples(float* samples,int size) {
    int i,j;
    float slope;
    float max;
    int lindex,rindex;
    float extras;

// g0orx binary header

/*
    // first byte is the buffer type
    client_samples[0]=SPECTRUM_BUFFER;
    sprintf(&client_samples[1],"%f",HEADER_VERSION);

    // next 6 bytes contain the main rx s meter
    sprintf(&client_samples[14],"%d",(int)meter);

    // next 6 bytes contain the subrx s meter
    sprintf(&client_samples[20],"%d",(int)subrx_meter);

    // next 6 bytes contain data length
    sprintf(&client_samples[26],"%d",size);

    // next 8 bytes contain the sample rate
    sprintf(&client_samples[32],"%d",sampleRate);

    // next 8 bytes contain the meter - for compatability
    sprintf(&client_samples[40],"%d",(int)meter);
*/

    client_samples[0]=SPECTRUM_BUFFER;
    client_samples[1]=HEADER_VERSION;
    client_samples[2]=HEADER_SUBVERSION;
    client_samples[3]=(size>>8)&0xFF;  // samples length
    client_samples[4]=size&0xFF;
    client_samples[5]=((int)meter>>8)&0xFF; // mainn rx meter
    client_samples[6]=(int)meter&0xFF;
    client_samples[7]=((int)subrx_meter>>8)&0xFF; // sub rx meter
    client_samples[8]=(int)subrx_meter&0xFF;
    client_samples[9]=(sampleRate>>24)&0xFF; // sample rate
    client_samples[10]=(sampleRate>>16)&0xFF;
    client_samples[11]=(sampleRate>>8)&0xFF;
    client_samples[12]=sampleRate&0xFF;

    // addes for header version 2.1
    client_samples[13]=((int)LO_offset>>8)&0xFF; // IF
    client_samples[14]=(int)LO_offset&0xFF;

    slope=(float)SAMPLE_BUFFER_SIZE/(float)size;
    if(mox) {
        extras=-82.62103F;
    } else {
        extras=displayCalibrationOffset+preampOffset;
    }
#pragma omp parallel shared(size, slope, samples, client_samples) private(max, i, lindex, rindex, j)
  {
    #pragma omp for schedule(static)
    for(i=0;i<size;i++) {
        max=-10000.0F;
        lindex=(int)floor((float)i*slope);
        rindex=(int)floor(((float)i*slope)+slope);
        if(rindex>SAMPLE_BUFFER_SIZE) rindex=SAMPLE_BUFFER_SIZE;
        for(j=lindex;j<rindex;j++) {
            if(samples[j]>max) max=samples[j];
        }
        client_samples[i+BUFFER_HEADER_SIZE]=(unsigned char)-(max+extras);
    }
  }
}


void setprintcountry()
{
	prncountry = 0;
	fprintf(stderr,"Country Lookup is On\n");
}

void printcountry(){
	pthread_t lookup_thread;
	int ret;

    ret = pthread_create( &lookup_thread, NULL, printcountrythread, (void*) NULL);
    if (ret == 0) pthread_detach(lookup_thread);
		
}

void printcountrythread()
{
  // looks for the country for the connecting IP
  FILE *fp;
  char path[1035];
  char sCmd[255];
  /* Open the command for reading. */
  sprintf(sCmd,"wget -O - --post-data 'ip=%s'  http://www.selfseo.com/ip_to_country.php 2>/dev/null |grep ' is assigned to '|perl -ne 'm/border=1> (.*?)</; print \"$1\\n\" '",inet_ntoa(client.sin_addr));
  fp = popen(sCmd, "r");
  if (fp == NULL) {
    fprintf(stdout,"Failed to run printcountry command\n" );
    return;
  }

  /* Read the output a line at a time - output it. */
  fprintf(stdout,"\nIP %s is from ", inet_ntoa(client.sin_addr));
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    fprintf(stdout,"%s",  path);
  }

  /* close */
  pclose(fp);

  return;
}

void answer_question(char *message, char *clienttype, struct bufferevent *bev){
	// reply = 4LLqqq:aaaa LL= length (limit 99 + header 3) followed by question : answer
	char *reply; 
	char answer[101]="xxx";
	unsigned short length;
	char len[10];
	memory_entry *item = NULL;

	if (strcmp(message,"q-version") == 0){
		 strcat(answer,"q-version:");
		 strcat(answer,version);
	}else if (strcmp(message,"q-server") == 0){
		 strcat(answer,"q-server:");
		 strcat(answer,servername);
		 if (txcfg == TXNONE){
			 strcat(answer," N");
		 }else if (txcfg == TXPASSWD){
			 strcat(answer," P");
		 }else{  // must be TXALL
			 strcat(answer," Y");
		 }
	}else if (strcmp(message,"q-master") == 0){
		 strcat(answer,"q-master:");
		 strcat(answer,clienttype);
	}else if (strcmp(message,"q-info") == 0){
		 strcat(answer,"q-info:");
		 if(strcmp(clienttype,"slave") == 0){
		    strcat(answer,"s;0");
		 }else{
			strcat(answer,"s;1"); 
		 }
		 strcat(answer,";f;");
		 char f[30];
		 sprintf(f,"%lld;m;",lastFreq);
		 strcat(answer,f);
		 char m[10];
		 sprintf(m,"%d;",lastMode);	
		 strcat(answer,m); 
	}else if (strcmp(message,"q-rtpport") == 0){
		 strcat(answer,"q-rtpport:");
		 char p[10];
		 sprintf(p,"%d;",local_rtp_port);
		 strcat(answer,p);
	}else if (strncmp(message,"q-cantx",7) == 0){
		 char delims[] = "#";
		 char *result = NULL;
         result = strtok( message, delims ); //returns q-cantx
         if ( result != NULL )  result = strtok( NULL, delims ); // this should be call/user
		 if ( result != NULL ){
			 if (chkFreq(result,  lastFreq , lastMode) == 0){
				 strcat(answer,"q-cantx:Y");
			 }else{
				 strcat(answer,"q-cantx:N");;
			 } 
		 }else{
		    strcat(answer,"q-cantx:N");
		 }
	}else{
		fprintf(stderr,"Unknown question: %s\n",message);
		return;
	}
	answer[0] = '4';  //ANSWER_BUFFER
	length = strlen(answer) - 3; // 
	if (length > 99){
	   fprintf(stderr,"Oversize reply!!: %s = %u\n",message, length);
	   return;
	}

	sprintf(len,"%02u", length);
	answer[1] = len[0];
	answer[2] = len[1];

	reply = (char *) malloc(length+4);		// need to include the terminating null
	memcpy(reply, answer, length+4);
	bufferevent_write(bev, reply, strlen(answer) );
	
	item = malloc(sizeof(*item));
	item->memory = reply;
	sem_wait(&memory_semaphore);
	TAILQ_INSERT_TAIL(&Memory_Pool, item, entries);
	sem_post(&memory_semaphore);
	
}

