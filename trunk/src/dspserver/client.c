/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil */
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
#include <event2/thread.h>
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
#include "util.h"

static int timing=0;

static int rtp_tx_init_done = 0;

static pthread_t client_thread_id, tx_thread_id, rtp_tx_thread_id;

#define BASE_PORT 8000
static int port=BASE_PORT;

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
static timer_t rtp_tx_timerid;

// For timer based spectrum data (instead of sending one spectrum frame per getspectrum command from clients)
#define SPECTRUM_TIMER_NS (20*1000000)
static timer_t spectrum_timerid;
static unsigned long spectrum_timestamp = 0;

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

//
// samplerate library data structures
//
SRC_STATE *mic_sr_state;
double mic_src_ratio;

static float meter;
static float subrx_meter;
int encoding = 0;

static int send_audio = 0;

static sem_t bufferevent_semaphore, mic_semaphore;

void* client_thread(void* arg);
void* tx_thread(void* arg);
void *rtp_tx_thread(void *arg);
int local_rtp_port = LOCAL_RTP_PORT;

void client_send_samples(int size);
void client_set_samples(float* samples,int size);

char* client_samples;
int samples;
int prncountry = 0;

static void *printcountrythread(void *);
static void printcountry(struct sockaddr_in *);

float getFilterSizeCalibrationOffset() {
    int size=1024; // dspBufferSize
    float i=log10((float)size);
    return 3.0f*(11.0f-i);
}

void audio_stream_init(int receiver) {
    sem_init(&audiostream_sem, 0, 1);
    init_alaw_tables();
    TAILQ_INIT(&IQ_audio_stream);
    TAILQ_INIT(&Mic_audio_stream);
}

void audio_stream_queue_add(unsigned char *buffer, int length) {
    struct audio_entry *item;

    sem_wait(&bufferevent_semaphore);
    if(send_audio) {
        item = malloc(sizeof(*item));
        item->buf = buffer;
        item->length = length;
        TAILQ_INSERT_TAIL(&IQ_audio_stream, item, entries);
    }
    sem_post(&bufferevent_semaphore);
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

    evthread_use_pthreads();

    TAILQ_INIT(&Client_list);

    sem_init(&bufferevent_semaphore,0,1);
    sem_init(&mic_semaphore,0,1);
    signal(SIGPIPE, SIG_IGN);
    rtp_init();
    spectrum_timer_init();

    port=BASE_PORT+receiver;
    rc=pthread_create(&client_thread_id,NULL,client_thread,NULL);

    if(rc != 0) {
        fprintf(stderr,"pthread_create failed on client_thread: rc=%d\n", rc);
    }
    else rc=pthread_detach(client_thread_id);
}

void tx_init(void){
    int rc;

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
            sdr_log(SDR_LOG_INFO, "tx_init: sample rate init successful with ratio of : %f\n", mic_src_ratio);
	}

        rc=pthread_create(&tx_thread_id,NULL,tx_thread,NULL);
        if(rc != 0) fprintf(stderr,"pthread_create failed on tx_thread: rc=%d\n", rc);
	else rc=pthread_detach(tx_thread_id);
}

void rtp_tx_timer_handler(int);
void spectrum_timer_handler(int);

void rtp_tx_init(void){
	int rc;

	if (rtp_tx_init_done == 0){

		TAILQ_INIT(&Mic_rtp_stream);
		rc=pthread_create(&rtp_tx_thread_id,NULL,rtp_tx_thread,NULL);
		if(rc != 0) fprintf(stderr,"pthread_create failed on rtp_tx_thread: rc=%d\n", rc);
		else rc=pthread_detach(rtp_tx_thread_id);

		rtp_tx_init_done = 1;

		struct itimerspec	value;
		struct sigevent sev;

		value.it_value.tv_sec = 0;
		value.it_value.tv_nsec = RTP_TIMER_NS;

		value.it_interval.tv_sec = 0;
		value.it_interval.tv_nsec = RTP_TIMER_NS;

		sev.sigev_notify = SIGEV_THREAD;
		sev.sigev_notify_function = rtp_tx_timer_handler;
		sev.sigev_notify_attributes = NULL;

		timer_create (CLOCK_REALTIME, &sev, &rtp_tx_timerid);
		timer_settime (rtp_tx_timerid, 0, &value, NULL);

	}
}

void spectrum_timer_init(void){

	struct itimerspec	value;
	struct sigevent sev;

	value.it_value.tv_sec = 0;
	value.it_value.tv_nsec = SPECTRUM_TIMER_NS;

	value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = SPECTRUM_TIMER_NS;

	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = spectrum_timer_handler;
	sev.sigev_notify_attributes = NULL;

	timer_create (CLOCK_REALTIME, &sev, &spectrum_timerid);
	timer_settime (spectrum_timerid, 0, &value, NULL);
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

    sem_wait(&bufferevent_semaphore);
    client_item = TAILQ_FIRST(&Client_list);
    sem_post(&bufferevent_semaphore);
    if (client_item == NULL) return;	                // no master client
    if (client_item->rtp != connection_rtp) return;	// not rtp master
	length=rtp_receive(client_item->session,rtp_buffer,RTP_BUFFER_SIZE);
	recv_ts+=RTP_BUFFER_SIZE;		        // proceed with timestamp increment as this is timer based	        
	if (length > 0){
	    for(i=0;i<length;i++) {
		v=G711A_decode(rtp_buffer[i]);
		fv=(float)v/32767.0F;                   // get into the range -1..+1

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

void spectrum_timer_handler(int sv){            // this is called every 20 ms
        client_entry *item;
        
        sem_wait(&bufferevent_semaphore);
        item = TAILQ_FIRST(&Client_list);
        sem_post(&bufferevent_semaphore);
        if (item == NULL) return;               // no clients

        sem_wait(&bufferevent_semaphore);
        if(mox) {
            Process_Panadapter(1,spectrumBuffer);
            meter=CalculateTXMeter(1,5);        // MIC
            subrx_meter=-121;
        } else {
            Process_Panadapter(0,spectrumBuffer);
            meter=CalculateRXMeter(0,0,0)+multimeterCalibrationOffset+getFilterSizeCalibrationOffset();
            subrx_meter=CalculateRXMeter(0,1,0)+multimeterCalibrationOffset+getFilterSizeCalibrationOffset();
            }
        TAILQ_FOREACH(item, &Client_list, entries){
            sem_post(&bufferevent_semaphore);
            if(item->fps > 0) {
                if (item->frame_counter-- <= 1) {
                    client_samples=malloc(BUFFER_HEADER_SIZE+item->samples);
                    client_set_samples(spectrumBuffer,item->samples);
                    bufferevent_write(item->bev, client_samples, BUFFER_HEADER_SIZE+item->samples);
                    free(client_samples);
                    item->frame_counter = 50 / item->fps;
                }
            }
            sem_wait(&bufferevent_semaphore);
        }
        sem_post(&bufferevent_semaphore);

        spectrum_timestamp++;
}

void* rtp_tx_thread(void *arg){
    int j;
    float data_out[TX_BUFFER_SIZE*2*24];	// data_in is 8khz (duplicated to stereo) Mic samples.  
						// May be resampled to 192khz or 24x stereo
    SRC_DATA data;
    int rc;
    struct audio_entry *item;

    sdr_thread_register("rtp_tx_thread");

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

   sdr_thread_register("tx_thread");

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
    client_entry *item;
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

    sem_wait(&bufferevent_semaphore);
    for (item = TAILQ_FIRST(&Client_list); item != NULL; item = TAILQ_NEXT(item, entries)){
	if (item->bev == bev){
            char ipstr[16];
            inet_ntop(AF_INET, (void *)&item->client.sin_addr, ipstr, sizeof(ipstr));
            sdr_log(SDR_LOG_INFO, "RX%d: client disconnection from %s:%d\n",
                    receiver, ipstr, ntohs(item->client.sin_port));
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

    sem_post(&bufferevent_semaphore);

    if ((rtp_client_count <= 0) && is_rtp_client) {
	rtp_connected = 0; 	// last rtp client disconnected
	rtp_listening = 0;
    }

    if (toShareOrNotToShare) {
        char status_buf[32];
        sprintf(status_buf, "%d client(s)", client_count);
        updateStatus(status_buf);
    }

    if (client_count <= 0) {
        sem_wait(&bufferevent_semaphore);
        send_audio = 0;
        sem_post(&bufferevent_semaphore);
    }
    bufferevent_free(bev);
}



void* client_thread(void* arg) {
 
    int on=1;
    struct event_base *base;
    struct event *listener_event;
    struct sockaddr_in server;
    int serverSocket;

    sdr_thread_register("client_thread");

    fprintf(stderr,"client_thread\n");

    serverSocket=socket(AF_INET,SOCK_STREAM,0);
    if(serverSocket==-1) {
        perror("client socket");
        return NULL;
    }

    evutil_make_socket_nonblocking(serverSocket);
    evutil_make_socket_closeonexec(serverSocket);

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

    sdr_log(SDR_LOG_INFO, "client_thread: listening on port %d\n", port);

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

    int fd = accept(listener, (struct sockaddr*)&ss, &slen);
    if (fd < 0) {
        sdr_log(SDR_LOG_WARNING, "accept failed\n");
        return;
    }
    char ipstr[16];
    // add newly connected client to Client_list
    item = malloc(sizeof(*item));
    memset(item, 0, sizeof(*item));
    memcpy(&item->client, &ss, sizeof(ss));

    inet_ntop(AF_INET, (void *)&item->client.sin_addr, ipstr, sizeof(ipstr));
    sdr_log(SDR_LOG_INFO, "RX%d: client connection from %s:%d\n",
            receiver, ipstr, ntohs(item->client.sin_port));

    if(prncountry){
        printcountry(&ss);
    }

    struct bufferevent *bev;
    evutil_make_socket_nonblocking(fd);
    evutil_make_socket_closeonexec(fd);
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);
    bufferevent_setcb(bev, readcb, writecb, errorcb, NULL);
    bufferevent_setwatermark(bev, EV_READ, MSG_SIZE, 0);
    bufferevent_setwatermark(bev, EV_WRITE, 4096, 0);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
    item->bev = bev;
    item->rtp = connection_unknown;
    item->fps = 0;
    item->frame_counter = 0;
    sem_wait(&bufferevent_semaphore);
    TAILQ_INSERT_TAIL(&Client_list, item, entries);
    sem_post(&bufferevent_semaphore);

    if (toShareOrNotToShare) {
        int client_count = 0;
        char status_buf[32];

        sem_wait(&bufferevent_semaphore);
        /* NB: Clobbers item */
        TAILQ_FOREACH(item, &Client_list, entries){
    	client_count++;
        }
        sem_post(&bufferevent_semaphore);

        sprintf(status_buf,"%d client(s)", client_count);
        updateStatus(status_buf);
    }
}

void writecb(struct bufferevent *bev, void *ctx){
    struct audio_entry *item;
    client_entry *client_item;

    while ((item = audio_stream_queue_remove()) != NULL){
        sem_wait(&bufferevent_semaphore);
        TAILQ_FOREACH(client_item, &Client_list, entries){
            sem_post(&bufferevent_semaphore);
            if(client_item->rtp == connection_tcp) {
                bufferevent_write(client_item->bev, item->buf, item->length);
            }
            else if (client_item->rtp == connection_rtp)
                rtp_send(client_item->session,&item->buf[AUDIO_BUFFER_HEADER_SIZE], (item->length - AUDIO_BUFFER_HEADER_SIZE));
            sem_wait(&bufferevent_semaphore);
        }
        sem_post(&bufferevent_semaphore);

        send_ts += item->length - AUDIO_BUFFER_HEADER_SIZE; // update send_ts for all rtp sessions
        free(item->buf);
        free(item);
    }
}

/* Commands allowed to slave connections.  The q-* commands are
 * implicitly allowed. */
static char *slave_commands[] = {
    "getspectrum",
    "setclient",
    "startaudiostream",
    "startrtpstream",
    "setfps",
    NULL
};

/* The maximum number of arguments a command can have and pass through
 * the tokenize_cmd tokenizer.  If you need more than this, bump it
 * up. */
#define MAX_CMD_TOKENS 4

/*
 * Tokenize the remaining words of a command, saving them to list and
 * returning the number of tokens found.  Do not attempt to find more
 * than 'tokens' tokens.
 */ 
static int tokenize_cmd(char **saveptr, char *list[], int tokens)
{
    int i = 0;
    char *token;

    if (tokens > MAX_CMD_TOKENS) {
        sdr_log(SDR_LOG_ERROR, "tokenize_cmd called with tokens > MAX_CMD_TOKENS\n");
        tokens = MAX_CMD_TOKENS;
    }
    for (i = 0; i < tokens && (token = strtok_r(NULL, " ", saveptr)); i++) {
        list[i] = token;
    }

    return i;
}

void readcb(struct bufferevent *bev, void *ctx){
    char *cmd, *saveptr;
    char *tokens[MAX_CMD_TOKENS];
    int i;
    int bytesRead = 0;
    char message[MSG_SIZE];
    client_entry *item, *current_item = NULL;
    char *role = "master";
    int slave = 0;
    struct evbuffer *inbuf;

    sem_wait(&bufferevent_semaphore);
    item = TAILQ_FIRST(&Client_list);
    sem_post(&bufferevent_semaphore);
    if (item == NULL) {
        sdr_log(SDR_LOG_ERROR, "readcb called with no clients");
        return;
    }
    if (item->bev != bev){
        client_entry *tmp_item;
        /* Only allow the first client on Client_list to command
         * dspserver as master.  If this client is not the master, we
         * will first determine whether it is allowed to execute the
         * command it is executing, and abort if it is not. */

	// locate the current_item for this slave client
        sem_wait(&bufferevent_semaphore);
        for (current_item = TAILQ_FIRST(&Client_list); current_item != NULL; current_item = tmp_item){
            tmp_item = TAILQ_NEXT(current_item, entries);
            if (current_item->bev == bev){
                break;
            }
        }
        sem_post(&bufferevent_semaphore);
        if (current_item == NULL) {
            sdr_log(SDR_LOG_ERROR, "This slave was not located");
            return;
        }

        role = "slave";
        slave = 1;
    }

    /* The documentation for evbuffer_get_length is somewhat unclear as
     * to the actual definition of "length".  It appears to be the
     * amount of space *available* in the buffer, not occupied by data;
     * However, the code for reading from an evbuffer will read as many
     * bytes as it would return, so this behavior is not different from
     * what was here before. */
    inbuf = bufferevent_get_input(bev);
    while (evbuffer_get_length(inbuf) >= MSG_SIZE) {
        if ((bytesRead = bufferevent_read(bev, message, MSG_SIZE)) != MSG_SIZE) {
            sdr_log(SDR_LOG_ERROR, "Short read from client; shouldn't happen\n");
            return;
        }
        message[bytesRead-1]=0;			// for Linux strings terminating in NULL

        cmd=strtok_r(message," ",&saveptr);
        if (cmd == NULL) continue;

        /* Short circuit for mic data, to ensure it's handled as rapidly
         * as possible. */
        if(!slave && strncmp(cmd,"mic", 3)==0){		// This is incoming microphone data, binary data after "mic "
            memcpy(mic_buffer, &message[4], MIC_BUFFER_SIZE);
            Mic_stream_queue_add();
            continue;
        }

        for (i = 0; cmd[i]; i++) {
            cmd[i] = tolower(cmd[i]);
        }
        
        /* Run permission checks for slave clients */
        if (slave) {
            int invalid = 1;
            if (!strncmp(cmd, "q", 1)) {
                invalid = 0;
            } else {
                for (i = 0; slave_commands[i]; i++) {
                    if (!strcmp(cmd, slave_commands[i])) {
                        invalid = 0;
                        break;
                    }
                }
            }
            if (invalid) {
                sdr_log(SDR_LOG_INFO, "Slave client attempted master command %s\n", cmd);
                continue;
            }
        }

        if(strncmp(cmd,"q",1)==0){	
            answer_question(message,role, bev);
        }else if(strncmp(cmd,"getspectrum",11)==0) {
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            samples=atoi(tokens[0]);

            sem_wait(&bufferevent_semaphore);
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
            sem_post(&bufferevent_semaphore);
            free(client_samples);
        } else if(strncmp(cmd,"setfrequency",12)==0) {
            long long frequency;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            frequency=atoll(tokens[0]);
            ozySetFrequency(frequency);
        } else if(strncmp(cmd,"setpreamp",9)==0) {
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            ozySetPreamp(tokens[0]);
        } else if(strncmp(cmd,"setmode",7)==0) {
            int mode;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            /* FIXME: mode should be sanity checked for sure */
            mode=atoi(tokens[0]);
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
        } else if(strncmp(cmd,"setfilter",9)==0) {
            int low,high;
            if (tokenize_cmd(&saveptr, tokens, 2) != 2)
                goto badcommand;
            low = atoi(tokens[0]);
            high = atoi(tokens[1]);
            SetRXFilter(0,0,(double)low,(double)high);
            SetRXFilter(0,1,(double)low,(double)high);
        } else if(strncmp(cmd,"setagc",6)==0) {
            int agc;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            agc=atoi(tokens[0]);
            SetRXAGC(0,0,agc);
            SetRXAGC(0,1,agc);
        } else if(strncmp(cmd,"setnr",5)==0) {
            int nr = 0;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            if(strcmp(tokens[0],"true")==0) {
                nr=1;
            }
            SetNR(0,0,nr);
            SetNR(0,1,nr);
        } else if(strncmp(cmd,"setnb",5)==0) {
            int nb = 0;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            if(strcmp(tokens[0],"true")==0) {
                nb=1;
            }
            SetNB(0,0,nb);
            SetNB(0,1,nb);
        } else if(strncmp(cmd,"setsdrom",8)==0) {
            int state = 0;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            if(strcmp(tokens[0],"true")==0) {
                state=1;
            }
            SetSDROM(0,0,state);
            SetSDROM(0,1,state);
        } else if(strncmp(cmd,"setanf",6)==0) {
            int anf = 0;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            if(strcmp(tokens[0],"true")==0) {
                anf=1;
            }
            SetANF(0,0,anf);
            SetANF(0,1,anf);
        } else if(strncmp(cmd,"setrxoutputgain",15)==0) {
            int gain;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            gain=atoi(tokens[0]);
            SetRXOutputGain(0,0,(double)gain/100.0);
        } else if(strncmp(cmd,"setsubrxoutputgain",18)==0) {
            int gain;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            gain=atoi(tokens[0]);
            SetRXOutputGain(0,1,(double)gain/100.0);
        } else if(strncmp(cmd,"startaudiostream",16)==0) {
            int ntok, bufsize, rate, channels;
            if (slave) {
                current_item->rtp = connection_tcp;
                continue;
            }
            ntok = tokenize_cmd(&saveptr, tokens, 3);

            /* FIXME: this is super racy */

            bufsize = AUDIO_BUFFER_SIZE;
            rate = 8000;
            channels = 1;

            if (ntok >= 1) {
                /* FIXME: validate! */
                bufsize = atoi(tokens[0]);
            }
            if (ntok >= 2) {
                rate = atoi(tokens[1]);
                if (rate != 8000 && rate != 48000) {
                    sdr_log(SDR_LOG_INFO, "Invalid audio sample rate: %d\n", rate);
                    rate = 8000;
                }
            }
            if (ntok >= 3) {
                channels = atoi(tokens[2]);
                if (channels != 1 && channels != 2) {
                    sdr_log(SDR_LOG_INFO, "Invalid audio channels: %d\n", channels);
                    channels = 1;
                }
            }

            sem_wait(&audiostream_sem);
            audiostream_conf.bufsize = bufsize;
            audiostream_conf.samplerate = rate;
            audiostream_conf.channels = channels;
            audiostream_conf.age++;
            sem_post(&audiostream_sem);

            sdr_log(SDR_LOG_INFO, "starting audio stream at rate %d channels %d bufsize %d encoding %d\n",
                    rate, channels, bufsize, encoding);
            item->rtp=connection_tcp;
            audio_stream_reset();
            sem_wait(&bufferevent_semaphore);
            send_audio=1;
            sem_post(&bufferevent_semaphore);
        } else if(strncmp(cmd,"startrtpstream",14)==0) {
            int rtpport;
            char ipstr[16];
            int encoding, samplerate, channels;

            // startrtpstream port encoding samplerate channels
            if (tokenize_cmd(&saveptr, tokens, 4) != 4)
                goto badcommand;

            /* FIXME: validate! */
            rtpport = atoi(tokens[0]);
            encoding = atoi(tokens[1]);
            samplerate = atoi(tokens[2]);
            channels = atoi(tokens[3]);

            sem_wait(&audiostream_sem);
            audiostream_conf.encoding = encoding;
            audiostream_conf.samplerate = samplerate;
            audiostream_conf.channels = channels;
            audiostream_conf.age++;
            sem_post(&audiostream_sem);

            if (slave) {
                sdr_log(SDR_LOG_INFO, "startrtpstream: listening on RTP socket\n");
                inet_ntop(AF_INET, (void *)&current_item->client.sin_addr, ipstr, sizeof(ipstr));
                current_item->session=rtp_listen(ipstr,rtpport);
                current_item->rtp = connection_rtp;
            } else {
                inet_ntop(AF_INET, (void *)&item->client.sin_addr, ipstr, sizeof(ipstr));
                sdr_log(SDR_LOG_INFO, "starting rtp: to %s:%d encoding %d samplerate %d channels:%d\n",
                        ipstr,rtpport,encoding,samplerate,channels);
                item->session=rtp_listen(ipstr,rtpport);
                item->rtp=connection_rtp;
            }
            answer_question("q-rtpport",role,bev);
            audio_stream_reset();
            sem_wait(&bufferevent_semaphore);
            send_audio=1;
            sem_post(&bufferevent_semaphore);
            rtp_tx_init();
        } else if(strncmp(cmd,"stopaudiostream",15)==0) {
            // send_audio should only be stopped by dspserver when no more clients are connected.
            // not by individual clients.
            /*
            sem_wait(&bufferevent_semaphore);
            send_audio=0;
            sem_post(&bufferevent_semaphore);
            */
        } else if(strncmp(cmd,"setencoding",11)==0) {
            int enc;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            enc=atoi(tokens[0]);
            /* This used to force to 0 on error, now it leaves unchanged */
            if (enc >= 0 && enc <= 2) {
                sem_wait(&audiostream_sem);
                audiostream_conf.encoding = enc;
                audiostream_conf.age++;
                sem_post(&audiostream_sem);
            }
            sdr_log(SDR_LOG_INFO,"encoding changed to %d\n", enc);
        } else if(strncmp(cmd,"setsubrx",17)==0) {
            int state;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            /* FIXME: validate! */
            state=atoi(tokens[0]);
            SetSubRXSt(0,1,state);
        } else if(strncmp(cmd,"setsubrxfrequency",17)==0) {
            int offset;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            /* FIXME: validate! */
            offset=atoi(tokens[0]);
            SetRXOsc(0,1,offset - LO_offset);
        } else if(strncmp(cmd,"setpan",6)==0) {
            float pan;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            /* FIXME: validate! */
            pan=atof(tokens[0]);
            SetRXPan(0,0,pan);
        } else if(strncmp(cmd,"setsubrxpan",11)==0) {
            float pan;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            pan=atof(tokens[0]);
            SetRXPan(0,1,pan);
        } else if(strncmp(cmd,"record",6)==0) {
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            ozySetRecord(tokens[0]);
        } else if(strncmp(cmd,"setanfvals",10)==0) {
            int taps, delay;
            double gain, leakage;

            if (tokenize_cmd(&saveptr, tokens, 4) != 4)
                goto badcommand;
            taps = atoi(tokens[0]);
            delay = atoi(tokens[1]);
            gain = atof(tokens[2]);
            leakage = atof(tokens[3]);

            SetANFvals(0,0,taps,delay,gain,leakage);
            SetANFvals(0,1,taps,delay,gain,leakage);
        } else if(strncmp(cmd,"setnrvals",9)==0) {
            int taps, delay;
            double gain, leakage;

            if (tokenize_cmd(&saveptr, tokens, 4) != 4)
                goto badcommand;
            taps = atoi(tokens[0]);
            delay = atoi(tokens[1]);
            gain = atof(tokens[2]);
            leakage = atof(tokens[3]);
            SetNRvals(0,0,taps,delay,gain,leakage);
            SetNRvals(0,1,taps,delay,gain,leakage);
        } else if(strncmp(cmd,"setnbvals",9)==0) {
            double threshold;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            threshold = atof(tokens[0]);
            SetNBvals(0,0,threshold);
            SetNBvals(0,1,threshold);
        } else if(strncmp(cmd,"setsdromvals",12)==0) {
            double threshold;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            threshold=atof(tokens[0]);
            SetSDROMvals(0,0,threshold);
            SetSDROMvals(0,1,threshold);
        } else if(strncmp(cmd,"setdcblock",10)==0) {
            int state;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            state=atoi(tokens[0]);
            SetRXDCBlock(0,0,state);
            SetRXDCBlock(0,1,state);
        } else if(strncmp(cmd,"mox",3)==0) {
            int ntok;
            if ((ntok = tokenize_cmd(&saveptr, tokens, 3)) < 1)
                goto badcommand;
            if (strcmp(tokens[0],"on")==0) {
                if (txcfg == TXALL){
                    ozySetMox(1);
                }else if(txcfg == TXPASSWD){
                    if (ntok == 3) {
                        char *thisuser = tokens[1];
                        char *thispasswd = tokens[2];
                        if(chkPasswd(thisuser, thispasswd) == 0){
                            // password OK check freq
                            if (chkFreq(thisuser,  lastFreq , lastMode) == 0){
                                //freqchk passwd good to tx
                                sdr_log(SDR_LOG_INFO,"Mox on from User:%s\n",thisuser);
                                ozySetMox(1);
                            }else{
                                sdr_log(SDR_LOG_INFO,"Mox denied because user %s has no rule for mode %d on %lld! hz\n",thisuser, lastMode,lastFreq);
                            }
                        }else{
                            sdr_log(SDR_LOG_INFO,"Mox on denied because user %s password check failed!\n",thisuser);
                        }
                    }
                }else{
                    sdr_log(SDR_LOG_INFO,"mox denied because tx = \"no\"\n");
                }
            } else if(strcmp(tokens[0],"off")==0) {
                if (txcfg == TXALL){
                    ozySetMox(0);
                }else if(txcfg == TXPASSWD){
                    if (ntok == 3) {
                        char *thisuser = tokens[1];
                        char *thispasswd = tokens[2];
                        if(chkPasswd(thisuser, thispasswd) == 0){
                            sdr_log(SDR_LOG_INFO,"Mox off from User:%s\n",thisuser);
                            ozySetMox(0);
                        }else{
                            sdr_log(SDR_LOG_INFO,"Mox off denied because user %s password check failed!\n",thisuser);
                        }
                    }
                }
            } else {
                goto badcommand;
            }
        } else if(strncmp(cmd,"settxamcarrierlevel",19)==0) {
            int ntok;
            double pwr;
            sdr_log(SDR_LOG_INFO,"SetTXAMCarrierLevel: %s  txcfg: %d\n",message,txcfg);
            if ((ntok = tokenize_cmd(&saveptr, tokens, 3)) < 1)
                goto badcommand;
            pwr=atof(tokens[0]);
            if(txcfg == TXPASSWD){
                if (ntok == 3) {
                    char *thisuser = tokens[1];
                    char *thispasswd = tokens[2];
                    if(chkPasswd(thisuser, thispasswd) == 0){ 
                        if(pwr >= 0 && pwr <= 1) {
                            //fprintf(stderr,"SetTXAMCarrierLevel = %f\n", pwr);
                            SetTXAMCarrierLevel(1,pwr);
                        }
                    }else{
                        sdr_log(SDR_LOG_INFO,"SetTXAMCarrierLevel denied because user %s password check failed!\n",thisuser);
                    }
                }
            }if (txcfg == TXALL){
                if(pwr >= 0 &&  pwr <= 1) {
                    sdr_log(SDR_LOG_INFO,"SetTXAMCarrierLevel = %f\n", pwr);
                    SetTXAMCarrierLevel(1,pwr);
                }else{
                    sdr_log(SDR_LOG_INFO,"SetTXAMCarrierLevel denied because Invalid command argument : '%s' txcfg = %d\n",message, txcfg);
                }
            } else{
                fprintf(stderr,"Invalid SetTXAMCarrierLevel command: '%s'\n",message);
            }
        } else if(strncmp(cmd,"setsquelchval",13)==0) {
            float value;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            value=atof(tokens[0]);
            SetSquelchVal(0,0,value);
        } else if(strncmp(cmd,"setsubrxquelchval",17)==0) {
            float value;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            value=atof(tokens[1]);
            SetSquelchVal(0,1,value);
        } else if(strncmp(cmd,"setsquelchstate",15)==0) {
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            if(strcmp(tokens[0],"on")==0) {
                SetSquelchState(0,0,1);
            } else if(strcmp(tokens[0],"off")==0) {
                SetSquelchState(0,0,0);
            } else {
                goto badcommand;
            }
        } else if(strncmp(cmd,"setsubrxquelchstate",19)==0) {
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            if(strcmp(tokens[0],"on")==0) {
                SetSquelchState(0,1,1);
            } else if(strcmp(tokens[0],"off")==0) {
                SetSquelchState(0,1,0);
            } else {
                goto badcommand;
            }
        } else if(strncmp(cmd,"setspectrumpolyphase",20)==0) {
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            if(strcmp(tokens[0],"true")==0) {
                SetSpectrumPolyphase(0,1);
            } else if(strcmp(tokens[0],"false")==0) {
                SetSpectrumPolyphase(0,0);
            } else {
                goto badcommand;
            }
        } else if(strncmp(cmd,"setocoutputs",12)==0) {
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            ozySetOpenCollectorOutputs(tokens[0]);
        } else if(strncmp(cmd,"setclient",9)==0) {
            if (tokenize_cmd(&saveptr, tokens, 1) == 1) {
                sdr_log(SDR_LOG_INFO, "RX%d: client is %s\n", receiver, tokens[0]);
            }
        } else if(strncmp(cmd,"setiqenable",11)==0) {
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            if(strcmp(tokens[0],"true")==0) {
                SetCorrectIQEnable(1);
                sdr_log(SDR_LOG_INFO,"SetCorrectIQEnable(1)\n"); 
            } else if(strcmp(tokens[0],"false")==0) {
                SetCorrectIQEnable(0);
                sdr_log(SDR_LOG_INFO,"SetCorrectIQEnable(0)\n");
            } else {
                goto badcommand;
            }
        } else if(strncmp(cmd,"testbutton",10)==0) {
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            if(strcmp(tokens[0],"true")==0) {
                sdr_log(SDR_LOG_INFO,"The button is pressed: '%s'\n",message);
            } else if(strcmp(tokens[0],"false")==0) {
                sdr_log(SDR_LOG_INFO,"The button is released: '%s'\n",message);
            } else {
                goto badcommand;
            }
        } else if(strncmp(cmd,"testslider",10)==0) {
            int value;
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            value=atoi(tokens[0]);
            fprintf(stdout,"The slider value is '%d'\n",value);
        } else if(strncmp(cmd,"rxiqmuval",9)==0) {
            if (tokenize_cmd(&saveptr, tokens, 1) != 1)
                goto badcommand;
            sdr_log(SDR_LOG_INFO,"The value of mu sent = '%s'\n",tokens[0]);
            SetCorrectRXIQMu(0, 0, atof(tokens[0]));
        } else if(strncmp(cmd,"setfps",6)==0) {
            if (tokenize_cmd(&saveptr, tokens, 2) != 2)
                goto badcommand;
            sdr_log(SDR_LOG_INFO,"Spectrum fps set to = '%s'\n",tokens[1]);
            if (slave) {
                current_item->samples = atoi(tokens[0]);
                current_item->fps = atoi(tokens[1]);
            }
            else  {
                item->samples = atoi(tokens[0]);
                item->fps = atoi(tokens[1]);
            }
        } else {
            fprintf(stderr,"Invalid command: token: '%s'\n",cmd);
        }
        continue;

      badcommand:
        sdr_log(SDR_LOG_INFO, "Invalid command: '%s'\n", message);
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

    // added for header version 2.1
    client_samples[13]=((int)LO_offset>>8)&0xFF; // IF
    client_samples[14]=(int)LO_offset&0xFF;

    slope=(float)SAMPLE_BUFFER_SIZE/(float)size;
    if(mox) {
        extras=-82.62103F;
    } else {
        extras=displayCalibrationOffset;
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
	prncountry = 1;
	fprintf(stderr,"Country Lookup is On\n");
}

void printcountry(struct sockaddr_in *client){
    pthread_t lookup_thread;
    int ret;

    /* This takes advantage of the fact that IPv4 addresses are 32 bits.
     * If or when this code is aware of other types of sockets, the
     * argument here will have to be renegotiated. */
    ret = pthread_create(&lookup_thread, NULL, printcountrythread,
                         (void*)client->sin_addr.s_addr);
    if (ret == 0) pthread_detach(lookup_thread);
}

void *printcountrythread(void *arg)
{
  // looks for the country for the connecting IP
  FILE *fp;
  char path[1035];
  char sCmd[255];
  struct in_addr addr;
  char ipstr[16];

  addr.s_addr = (in_addr_t)arg;
  inet_ntop(AF_INET, (void *)&addr, ipstr, sizeof(ipstr));
  /* Open the command for reading. */
  sprintf(sCmd,"wget -q -O - --post-data 'ip=%s' http://www.selfseo.com/ip_to_country.php 2>/dev/null | sed -e '/ is assigned to /!d' -e 's/.*border=1> \\([^<]*\\).*/\\1/'",
          ipstr);
  fp = popen(sCmd, "r");
  if (fp == NULL) {
    fprintf(stdout,"Failed to run printcountry command\n" );
    return NULL;
  }

  /* Read the output a line at a time - output it. */
  fprintf(stdout,"\nIP %s is from ", ipstr);
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    fprintf(stdout,"%s",  path);
  }

  /* close */
  pclose(fp);

  return NULL;
}

void answer_question(char *message, char *clienttype, struct bufferevent *bev){
	// reply = 4LLqqq:aaaa LL= length (limit 99 + header 3) followed by question : answer
	char *reply; 
	char answer[101]="xxx";
	unsigned short length;
	char len[10];
	char *safeptr;

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
		 char f[50];
		 sprintf(f,"%lld;m;",lastFreq);
		 strcat(answer,f);
		 char m[50];
		 sprintf(m,"%d;",lastMode);	
		 strcat(answer,m); 
	}else if (strcmp(message,"q-rtpport") == 0){
		 strcat(answer,"q-rtpport:");
		 char p[50];
		 sprintf(p,"%d;",local_rtp_port);
		 strcat(answer,p);
	}else if (strncmp(message,"q-cantx",7) == 0){
		 char delims[] = "#";
		 char *result = NULL;
         result = strtok_r( message, delims, &safeptr ); //returns q-cantx
         if ( result != NULL )  result = strtok_r( NULL, delims, &safeptr ); // this should be call/user
		 if ( result != NULL ){
			 if (chkFreq(result,  lastFreq , lastMode) == 0){
				 strcat(answer,"q-cantx:Y");
			 }else{
				 strcat(answer,"q-cantx:N");;
			 } 
		 }else{
		    strcat(answer,"q-cantx:N");
		 }
	}else if (strcmp(message,"q-loffset") == 0){
		 strcat(answer,"q-loffset:");
		 char p[50];
		 sprintf(p,"%f;",LO_offset);
		 strcat(answer,p);
		 //fprintf(stderr,"q-loffset: %s\n",answer);
	}else if (strcmp(message,"q-protocol3") == 0){
		 strcat(answer,"q-protocol3:Y");
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

        free(reply);
}

void printversion(){
	 fprintf(stderr,"dspserver string: %s\n",version);
}
