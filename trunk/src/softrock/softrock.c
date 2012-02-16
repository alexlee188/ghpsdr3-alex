/**
 * @file softrock.c
 * @brief Softrock audio implementation
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
* This file is where the threads are started, where the get and set routines
* are for the flags that are set by the command line arguements set in server.c.
*/



#include "client.h"
#include "softrock.h"
#include "softrockio.h"
#include "receiver.h"
#include "util.h"
#include "jackio.h"

int verbose;

static pthread_t softrock_io_thread_id;

static int rx_frame=0;
//static int tx_frame=0;
static int receivers=1;
static int current_receiver=0;
static int active_rx[MAX_RECEIVERS] = {0,0,0,0}; // This flag tells how many clients are using each rx
// using this receiver.

static int use_jack=0;
#ifdef USE_PIPES
int pipe_handle_left[MAX_RECEIVERS][2];
int pipe_handle_right[MAX_RECEIVERS][2];
#else // Use ringbuffers
jack_ringbuffer_t * rb_right[MAX_RECEIVERS];
jack_ringbuffer_t * rb_left[MAX_RECEIVERS];
#endif

static int speed=0;
static int sample_rate=48000;

//static int samples=0;

static int input_buffers;

//static struct sockaddr_in client;
//static int client_length;

static int iq_socket;
static struct sockaddr_in iq_address;
static int iq_address_length;

char device[80];
char input[80];
char output[80];

static int iq=1;


static char filename[256];
static int record=0;
static int playback=0;
static int playback_sleep=0;
static FILE* recording;

void* softrock_io_thread(void* arg);

#if (defined PULSEAUDIO || defined DIRECTAUDIO)
void process_softrock_input_buffer(char* buffer);
#endif

int softrock_init(void);
int init_jack_audio (void);

int create_softrock_thread() {
	int rc, res;
	softrock_init();
#ifdef JACKAUDIO
	if(softrock_get_jack() == 0) 
#endif
		{ //not running jack audio
		// create a thread to read from the audio deice
		rc=pthread_create(&softrock_io_thread_id,NULL,softrock_io_thread,NULL);
		if(rc != 0) {
			if(verbose) fprintf(stderr,"pthread_create failed on softrock_io_thread: rc=%d\n", rc);
			exit(1);
		}
		return 0;
	}
#ifdef JACKAUDIO //(Using callback)
	else {
		for (int i = 0; i < softrock_get_receivers ();i++) {
#ifdef USE_PIPES
			//Create the pipes.
			if (pipe2(&(pipe_handle_left[i][0]),O_NONBLOCK) == 	-1) { // Perhaps make O_NONBLOCK and use perror, see man -s 2,7 pipe
				if (softrock_get_verbose ()) perror( "Problem opening left pipe for Jack transmit for receiver.\n");
			}
			else {
				if (softrock_get_verbose ()) fprintf(stderr, "Left pipe for receiver %d for Jack transmit open.\n",i);
				//fcntl(pipe_handle_left[i][1], F_SETFL, O_NONBLOCK);
			}
			if (pipe2(&(pipe_handle_right[i][0]),O_NONBLOCK) != 0) {
				if (softrock_get_verbose ()) perror( "Problem opening right pipe for Jack transmit for receiver.\n");
			}
			else {
				if (softrock_get_verbose ()) fprintf(stderr, "Right pipe for receiver %d for Jack transmit open.\n",i);
				//fcntl(pipe_handle_right[i][1], F_SETFL, O_NONBLOCK);
			}	
#else // Use ringbuffer
			rb_right[i] = jack_ringbuffer_create(sizeof(float)*JACK_RINGBUFFER_SZ);
			res = jack_ringbuffer_mlock(rb_right[i]);
  			// check if we've locked the memory successfully
  			if ( res ) {
    			fprintf(stderr, "Error locking memory for jack ringbuffer!");
    			return -1;
  			}
			rb_left[i] = jack_ringbuffer_create(sizeof(float)*JACK_RINGBUFFER_SZ);
			res = jack_ringbuffer_mlock(rb_left[i]);
  			// check if we've locked the memory successfully
  			if ( res ) {
    			fprintf(stderr, "Error locking memory for jack ringbuffer!");
    			return -1;
  			}
#endif
			
		}
#ifdef USE_PIPES
		if( softrock_get_verbose ()) fprintf(stderr,"Compiled for Pipes, not jackfifo.\n");
		fprintf(stderr,"Softrock.c &pipe_handle_left[0][0] is: %d \n",&pipe_handle_left[0][0]);
		//fprintf(stderr,"Softrock.c &pipe_handle_left[3][1] is: %d \n",&pipe_handle_left[3][1]);
		fprintf(stderr,"Softrock.c &pipe_handle_left[0][1] is: %d \n",&pipe_handle_left[0][1]);
		fprintf(stderr,"Softrock.c pipe_handle_left[0][0] is: %d \n",pipe_handle_left[0][0]);
		fprintf(stderr,"Softrock.c pipe_handle_left[0][1] is: %d \n",pipe_handle_left[0][1]);
#endif
		if (init_jack_audio() != 0) {
			if(verbose) fprintf(stderr, "There was a problem initializing Jack Audio.\n");
			return 1;
		}
		else{
			return 0;
		}

	}
#endif			
}
#ifndef USE_PIPES
void delete_jack_ringbuffers(void)
{
	for (int i = 0; i < softrock_get_receivers ();i++) {
		if(rb_right[i] != NULL) jack_ringbuffer_free(rb_right[i]);
		if(rb_left[i] != NULL) jack_ringbuffer_free(rb_left[i]);
	}
}
#endif

void softrock_set_device(char* d) {
	if(verbose) fprintf(stderr,"softrock_set_device %s\n",d);
    strcpy(device,d);
}

char* softrock_get_device() {
	if(verbose) fprintf(stderr,"softrock_get_device %s\n",device);
    return device;
}

void softrock_set_input(char* d) {
	if(verbose) fprintf(stderr,"softrock_set_input %s\n",d);
    strcpy(input,d);
}

char* softrock_get_input() {
	if(verbose) fprintf(stderr,"softrock_get_input %s\n",input);
    return input;
}

void softrock_set_output(char* d) {
if(verbose) fprintf(stderr,"softrock_set_output %s\n",d);
    strcpy(output,d);
}

char* softrock_get_output() {
	if(verbose) fprintf(stderr,"softrock_get_output %s\n",output);
    return output;
}

void softrock_set_receivers(int r) {
    if(r>MAX_RECEIVERS) {
        if(verbose) fprintf(stderr,"MAX Receivers is 8!\n");
        exit(1);
    }
    receivers=r;
}

int softrock_get_receivers() {
    return receivers;
}

void softrock_set_jack(int flag) {
	use_jack = flag;
	if(flag) {
		if(softrock_get_verbose ()) fprintf(stderr,"Using Jack\n");
	}
}

int softrock_get_jack() {
    return use_jack;
}

#ifdef USE_PIPES
int * softrock_get_jack_read_pipe_left(int rx) {
	return &pipe_handle_left[rx][0];
}

int * softrock_get_jack_write_pipe_left(int rx) {
	return &pipe_handle_left[rx][1];
}

int * softrock_get_jack_read_pipe_right(int rx) {
	return &pipe_handle_right[rx][0];
}

 int * softrock_get_jack_write_pipe_right(int rx) {
	return &pipe_handle_right[rx][1];
}
#else // Use ringbuffers
jack_ringbuffer_t * softrock_get_jack_rb_left(int rx) {
	return rb_left[rx];
}

jack_ringbuffer_t * softrock_get_jack_rb_right(int rx) {
	return rb_right[rx];
}
#endif

void softrock_set_client_active_rx(int receiver, int inc) {
	active_rx[receiver] = active_rx[receiver] + inc; // keep track of active receivers.
	fprintf(stderr,"rx %d active %d times.\n",receiver, active_rx[receiver]);
	if ((active_rx[receiver] < -1) || (active_rx[receiver] > MAX_RECEIVERS)) {
		if (softrock_get_verbose () == 1) 
			fprintf(stderr, "Somehow receiver counting is off!\n");
		exit(1);
	}
	else return;
}

int softrock_get_client_active_rx(int receiver) {
	return active_rx[receiver];
}
	
void softrock_set_rx_frame(int frame) {
	rx_frame = frame;
}

int softrock_get_rx_frame() {
    return rx_frame;
}

void softrock_set_input_buffers(int buffers) {
	input_buffers = buffers;
}

int softrock_get_input_buffers() {
    return input_buffers;
}


void softrock_set_sample_rate(int r) {
	if(verbose) fprintf(stderr,"softrock_set_sample_rate %d\n",r);
    switch(r) {
        case 48000:
            sample_rate=r;
            speed=0;
            break;
        case 96000:
            sample_rate=r;
            speed=1;
            break;
        case 192000:
            sample_rate=r;
            speed=2;
            break;
        default:
            if(verbose) fprintf(stderr,"Invalid sample rate (48000,96000,192000)!\n");
            exit(1);
            break;
    }
    playback_sleep=(int)(1024.0/(float)sample_rate*1000000.0);
    if(verbose) fprintf(stderr,"sample_rate=%d playback_sleep=%d\n",sample_rate,playback_sleep);

}

int softrock_get_sample_rate() {
    return sample_rate;
}

void softrock_set_iq(int s) {
    iq=s;
}

int softrock_get_iq() {
    return iq;
}

int softrock_get_record() {
    return record;
}

int softrock_get_playback() {
    return playback;
}

void softrock_set_record(char* f) {
    strcpy(filename,f);
    record=1;
    playback=0;
}

void softrock_set_playback(char* f) {
    strcpy(filename,f);
    record=0;
    playback=1;
}

void softrock_set_verbose(int flag) {
	verbose = flag;
}

int softrock_get_verbose(void) {
	return verbose;
}

int softrock_init(void) {
    int rc;
    int i;

    iq_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(iq_socket<0) {
        perror("create socket failed for iq samples");
        exit(1);
    }

    iq_address_length=sizeof(iq_address);
    memset(&iq_address,0,iq_address_length);
    iq_address.sin_family=AF_INET;
    iq_address.sin_addr.s_addr=htonl(INADDR_ANY);
    iq_address.sin_port=htons(0);

    if(bind(iq_socket,(struct sockaddr*)&iq_address,iq_address_length)<0) {
        perror("bind socket failed for iq socket");
        exit(1);
    }

    if(record) {
        recording=fopen(filename,"w");
    } else if(playback) {
        recording=fopen(filename,"r");
				if(verbose) fprintf(stderr,"opening %s\n",filename);
    }

	// The last line needs to be uncommented, but when it is it 
	// breaks receive.  There is somewhere in the code that when jack is
	// used, pulse audio is still called and without setting it up in 
	// softrock_open() we get a crash.  So find out where that is and
	// fix it!
	if(!softrock_get_jack ())  //If Jack Audio this isn't needed.  
	{
		// open softrock audio  
		rc = softrock_open();
		if (rc != 0) {
			if(verbose) fprintf(stderr,"Cannot open softrock\n");
			return (-1);
		}
	}

    for(i=0;i<receivers;i++) {
        receiver[i].frequency=7056000L;
        receiver[i].frequency_changed=1;
    }

		if(verbose) fprintf(stderr,"server configured for %d receivers at %d\n",receivers,sample_rate);
    return rc;
}

void softrock_record_buffer(char* buffer,int length) {
    int bytes;

    if(record) {
        bytes=fwrite(buffer,sizeof(char),length,recording);
	if (bytes != length) fprintf(stderr, "record write failed %d\n", bytes);
    }
}

void softrock_playback_buffer(char* buffer,int length) {
    int bytes;

    if(playback) {
        usleep(playback_sleep);
        bytes=fread(buffer,sizeof(char),length,recording);
        if(bytes<=0) {
            // assumes eof
            fclose(recording);
            recording=fopen(filename,"r");
						if(verbose) fprintf(stderr,"playback: re-opening %s\n",filename);
            bytes=fread(buffer,sizeof(char),length,recording);
        } else {
					//fprintf(stderr,"playback: read %d bytes\n",bytes);
        }
    }
}


void* softrock_io_thread(void* arg) {
#if (defined PULSEAUDIO || defined PORTAUDIO)
    int rc;
#else    
    unsigned char input_buffer[BUFFER_SIZE*2]; // samples * 2 * 2
    int bytes;
#endif
 
    while(1) {

#ifdef PULSEAUDIO
        // read an input buffer (blocks until all bytes read)
        rc=softrock_read(receiver[current_receiver].input_buffer,&receiver[current_receiver].input_buffer[BUFFER_SIZE]);
        if(rc==0) {
            // process input buffer
            rx_frame++;
            input_buffers++;
            send_IQ_buffer(current_receiver);
        } else {
            if(verbose) fprintf(stderr,"softrock_read returned %d\n",rc);
        }
#endif
#ifdef PORTAUDIO
        // read an input buffer (blocks until all bytes read)
        rc=softrock_read(receiver[current_receiver].input_buffer,&receiver[current_receiver].input_buffer[BUFFER_SIZE]);
        if(rc==0) {
            // process input buffer
            rx_frame++;
            input_buffers++;
            send_IQ_buffer(current_receiver);
        } else {
            if(verbose) fprintf(stderr,"softrock_read returned %d\n",rc);
        }
#endif
#ifdef DIRECTAUDIO
        // read an input buffer (blocks until all bytes read)
        bytes=softrock_read(input_buffer,sizeof(input_buffer));
        if (bytes < 0) {
            if(verbose) fprintf(stderr,"softrock_io_thread: read failed %d\n",bytes);
        } else if (bytes != sizeof(input_buffer)) {
            if(verbose) fprintf(stderr,"sfoftrock_io_thread: only read %d bytes\n",bytes);
        } else {
            // process input buffer
            rx_frame++;
            process_softrock_input_buffer(input_buffer);
        }
        input_buffers++;
#endif
        current_receiver++;

        if(current_receiver==receivers) {
            current_receiver=0;
        }
    }
}


#ifdef DIRECTAUDIO
void process_softrock_input_buffer(char* buffer) {
    int b=0;
    int r;
    short left_sample,right_sample;
    //int left_sample,right_sample;
    float left_sample_float,right_sample_float;
    int rc;

        // extract the samples
    while(b<(BUFFER_SIZE*2*2)) {
        // extract each of the receivers
        for(r=0;r<receivers;r++) {
					//fprintf(stderr,"%d: %02X%02X %02X%02X\n",samples,buffer[b]&0xFF,buffer[b+1]&0xFF,buffer[b+2]&0xFF,buffer[b+3]&0xFF);
            left_sample   = (int)((unsigned char)buffer[b++]);
            left_sample  |= (int)((signed char)buffer[b++])<<8;
            //left_sample  += (int)((unsigned char)buffer[b++])<<8;
            //left_sample  += (int)((signed char)buffer[b++])<<16;
            right_sample  = (int)((unsigned char)buffer[b++]);
            right_sample |= (int)((signed char)buffer[b++])<<8;
            //right_sample += (int)((unsigned char)buffer[b++])<<8;
            //right_sample += (int)((signed char)buffer[b++])<<16;
            left_sample_float=(float)left_sample/32767.0; // 16 bit sample
            right_sample_float=(float)right_sample/32767.0; // 16 bit sample
/*
            left_sample_float=(float)left_sample/8388607.0; // 24 bit sample
            right_sample_float=(float)right_sample/8388607.0; // 24 bit sample
*/
            receiver[r].input_buffer[samples]=left_sample_float;
            receiver[r].input_buffer[samples+BUFFER_SIZE]=right_sample_float;

						//fprintf(stderr,"%d: %d %d\n",samples,left_sample,right_sample);
						//fprintf(stderr,"%d: %f %f\n",samples,left_sample_float,right_sample_float);
        }
        samples++;

        // when we have enough samples send them to the clients
        if(samples==BUFFER_SIZE) {
            // send I/Q data to clients
            for(r=0;r<receivers;r++) {
                send_IQ_buffer(r);
            }
            samples=0;
        }
    }

}
#endif

#ifdef PULSEAUDIO
void process_softrock_output_buffer(float* left_output_buffer,float* right_output_buffer) {
    softrock_write(left_output_buffer,right_output_buffer);
}
#endif

#ifdef PORTAUDIO
void process_softrock_output_buffer(float* left_output_buffer,float* right_output_buffer) {
	
	//left first (even indices), then right channel (odd indices)
	write(
    softrock_write(left_output_buffer,right_output_buffer);
    
}
#endif
#ifdef DIRECTAUDIO
void process_softrock_output_buffer(float* left_output_buffer,float* right_output_buffer) {
    int i;
    unsigned char output_buffer[BUFFER_SIZE*2*2];
    int left_sample,right_sample;
    int b=0;

    for(i=0;i<BUFFER_SIZE;i++) {
        left_sample=(int)(left_output_buffer[i]*32767.0F);
        right_sample=(int)(right_output_buffer[i]*32767.0F);
/*
        left_sample=(int)(left_output_buffer[i]*8388607.0F);
        right_sample=(int)(right_output_buffer[i]*8388607.0F);
*/
        output_buffer[b++]=left_sample&0xFF;
        output_buffer[b++]=(left_sample>>8)&0xFF;
        //output_buffer[b++]=(left_sample>>16)&0xFF;
        output_buffer[b++]=right_sample&0xFF;
        output_buffer[b++]=(right_sample>>8)&0xFF;
        //output_buffer[b++]=(right_sample>>16)&0xFF;
    }

    softrock_write(output_buffer,sizeof(output_buffer));
}


#endif

