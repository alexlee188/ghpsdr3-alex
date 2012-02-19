/**
 * @file jackio.c
 * @brief Softrock implementation
 * @author Rob Frohne, KL7NA "at" arrl "dot" net
 * @version 0.1
 * @date 20011-09-05
 */

/* Copyright (C)
* 2011 Rob Frohne
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
* This file contains the Jack initialization and callback that gets data to
* and from Jack.  This file is used in place of the code in softrockio.c 
* when Jack is used instead of direct audio (oss), portaudio, or pulse audio.
*/

#include "jackio.h"

#ifdef JACKAUDIO
static int frame;
static int buffers;
static jack_ringbuffer_t * rb_left[MAX_RECEIVERS];
static jack_ringbuffer_t * rb_right[MAX_RECEIVERS];


int init_jack_audio()
{
	int error, r;

	const char * capture_rx_port_name[2*MAX_RECEIVERS] = {"system:capture_1","system:capture_2",
		"system:capture_3","system:capture_4","system:capture_5","system:capture_6",
		"system:capture_7","system:capture_8"};
	const char * playback_tx_port_name[2*MAX_RECEIVERS] = {"system:playback_1","system:playback_2",
		"system:playback_3","system:playback_4","system:playback_5","system:playback_6",
		"system:playback_7","system:playback_8"};
	const char * softrock_rx_port_name_left[MAX_RECEIVERS] = {"Softrock RX Port_1_left",
		"Softrock RX Port_2_left","Softrock RX Port_3_left","Softrock RX Port_4_left",};
	const char * softrock_rx_port_name_right[MAX_RECEIVERS] = {"Softrock RX Port_1_right",
		"Softrock RX Port_2_right","Softrock RX Port_3_right","Softrock RX Port_4_right"};
	const char * softrock_tx_port_name_left[MAX_RECEIVERS] = {"Softrock TX Port_1_left",
		"Softrock TX Port_2_left","Softrock TX Port_3_left","Softrock TX Port_4_left",};
	const char * softrock_tx_port_name_right[MAX_RECEIVERS] = {"Softrock TX Port_1_right",
		"Softrock TX Port_2_right","Softrock TX Port_3_right","Softrock TX Port_4_right"};

	frame = softrock_get_rx_frame ();
	buffers = softrock_get_input_buffers();
	//Create a new jack client, then make sure everything went ok.
	softrock_client = jack_client_open("Softrock",(jack_options_t)(!JackServerName),NULL);
	if (softrock_client == 0) {
		fprintf(stderr,"Cannot connect to the jackd as a client.\n");
		jack_cleanup();
		return 1;
	}

	/* Set up Jack */
	//Set up the jack shutdown routine in case we want to do something special on shutdown of jack.
	jack_on_shutdown (softrock_client, jack_shutdown, 0);

	//Check to make sure the buffer size isn't too big.
	if(jack_get_buffer_size (softrock_client)	> BUFFER_SIZE) {
		fprintf(stderr,"Jack Buffers is too large.  Either recompile with a larger BUFFER_SIZE,\n"
		        "or start Jack with a buffer size of %d.\n",BUFFER_SIZE);
		jack_cleanup ();
		return 1;
	}

	//Create and register new audio input ports.
	for(r=0;r < softrock_get_receivers();r++) {	
#ifndef USE_PIPES // Use ringbuffers
		rb_left[r] = softrock_get_jack_rb_left(r);
		rb_right[r] = softrock_get_jack_rb_right(r);
#endif

		audio_input_port_left[r] = jack_port_register(softrock_client, softrock_rx_port_name_left[r], 
		                                              JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
		if (audio_input_port_left[r] == NULL) {
			fprintf(stderr, "Error: jack_port_register returned NULL for %s.\n",softrock_rx_port_name_left[r]);
			jack_cleanup();
			return 1;
		}

		audio_input_port_right[r] = jack_port_register(softrock_client, softrock_rx_port_name_right[r], 
		                                               JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
		if (audio_input_port_right[r] == NULL) {
			fprintf(stderr, "Error: jack_port_register returned NULL for %s.\n",softrock_rx_port_name_right[r]);
			jack_cleanup();
			return 1;
		}
		//Create and register new audio output ports.
		audio_output_port_left[r] = jack_port_register(softrock_client, softrock_tx_port_name_left[r], 
		                                               JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
		if (audio_output_port_left[r] == NULL) {
			fprintf(stderr, "Error: jack_port_register returned NULL for %s.\n",softrock_tx_port_name_left[r]);
			jack_cleanup();
			return 1;
		}
		audio_output_port_right[r] = jack_port_register(softrock_client, softrock_tx_port_name_right[r], 
		                                                JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
		if (audio_output_port_right[r] == NULL) {
			fprintf(stderr, "Error: jack_port_register returned NULL for %s.\n",softrock_tx_port_name_right[r]);
			jack_cleanup();
			return 1;
		}

		//Tell the jackd server what function call when it wants more audio data.
		if((error = jack_set_process_callback(softrock_client, process,0)) != 0) { 
			fprintf(stderr, "Jack could not set the callback, (error %i).\n", error);
			jack_cleanup();
			return 1;
		}

	}


	//Tell jack it's ok to start asking us for audio data.
	if((error = jack_activate(softrock_client)) != 0) {
		fprintf(stderr, "Jack could not activate the client (error %i).\n", error);
		jack_cleanup();
		return 1;
	}
	else if(softrock_get_verbose()) fprintf(stderr,"Activated client.\n");

	//Connect the ports.
	for(r=0;r < softrock_get_receivers();r++) {
		//Connect the rx ports.
		if (jack_connect (softrock_client,capture_rx_port_name[2*r], jack_port_name (audio_input_port_left[r]))) {
			fprintf (stderr, "Cannot connect to port: %s\n",capture_rx_port_name[2*r]);
		}
		else if(softrock_get_verbose()) fprintf(stderr, "Connected to port: %s\n",capture_rx_port_name[2*r]);
		if (jack_connect (softrock_client,capture_rx_port_name[2*r+1], jack_port_name (audio_input_port_right[r]))) {
			fprintf (stderr, "Cannot connect to port:  %s\n",capture_rx_port_name[2*r+1]);
		}
		else if(softrock_get_verbose()) fprintf(stderr, "Connected to port:   %s\n",capture_rx_port_name[2*r+1]);
		//Connect the tx ports.
		if (jack_connect (softrock_client,jack_port_name (audio_output_port_left[r]), playback_tx_port_name[2*r])) {
			fprintf (stderr, "Cannot connect to port: %s\n",playback_tx_port_name[2*r]);
		}
		else if(softrock_get_verbose()) fprintf(stderr, "Connected to port: %s\n",playback_tx_port_name[2*r]);
		if (jack_connect (softrock_client,jack_port_name (audio_output_port_right[r]), playback_tx_port_name[2*r+1])) {
			fprintf (stderr, "Cannot connect to port:  %s\n",playback_tx_port_name[2*r+1]);
		}
		else if(softrock_get_verbose()) fprintf(stderr, "Connected to port:   %s\n",playback_tx_port_name[2*r+1]);
	}
	return 0;
}

/* This is the function that is called when and if jackd shuts down. */
void jack_shutdown (void *arg)
{
	fprintf (stderr, "JACK shutdown\n");
	jack_cleanup();
	abort();
}

/* Close things opened. */
void jack_cleanup(void) {
	if(softrock_client != NULL) {
		jack_deactivate(softrock_client);	
		jack_client_close(softrock_client);
	}
#ifndef USE_PIPES
	delete_jack_ringbuffers();
#endif
}


/* This is the callback process that gets data from jack.*/	
int process(jack_nframes_t number_of_frames, void* arg)
{
	// Start out with current_receiver = 0 (one receiver) fix later.
	jack_nframes_t i;
	static int  num_ovfl = 0, num_ovfr = 0, start_buffer = 0;
	int r;
	jack_default_audio_sample_t *sample_buffer_left[MAX_RECEIVERS];
	jack_default_audio_sample_t *sample_buffer_right[MAX_RECEIVERS];

	static int stop_print = 0;
	// static int num_blocked = 0;

	softrock_set_rx_frame (frame + 1);
	softrock_set_input_buffers(buffers +1);

	float *left_samples, *right_samples;

	for ( r = 0; r < softrock_get_receivers(); r++ ) {
		sample_buffer_left[r] = 
			(jack_default_audio_sample_t *) jack_port_get_buffer(audio_input_port_left[r], number_of_frames);
		sample_buffer_right[r] = 
			(jack_default_audio_sample_t *) jack_port_get_buffer(audio_input_port_right[r], number_of_frames);
		left_samples = &receiver[r].input_buffer[0];
		right_samples = &receiver[r].input_buffer[BUFFER_SIZE];
		if(softrock_get_iq()) {
			for(i=0;i<number_of_frames;i++) {
				left_samples[i]=(float)sample_buffer_left[r][i];
				right_samples[i]=(float)sample_buffer_right[r][i];
				//fprintf(stderr,"%d left=%f right=%f\n",i, left_samples[i],right_samples[i]);
			}
		} else {
			for(i=0;i<number_of_frames;i++) {
				right_samples[i]=(float)sample_buffer_left[r][i];
				left_samples[i]=(float)sample_buffer_right[r][i];
				//fprintf(stderr,"%d left=%f right=%f\n",i, left_samples[i],right_samples[i]);
			}
		}
		send_IQ_buffer(r);

		// Now do the tx part (send output IQ data from the dspserver client to
		// the audio out jacks.
		if (softrock_get_client_active_rx (r) > 0) {
			if (start_buffer++ < BUFF_FILL) return 0;
			int size = sizeof(float)*number_of_frames;
			//fprintf(stderr,"Made it to read tx\n");
			sample_buffer_left[r] = 
				(jack_default_audio_sample_t *) jack_port_get_buffer(audio_output_port_left[r], number_of_frames);
			sample_buffer_right[r] = 
				(jack_default_audio_sample_t *) jack_port_get_buffer(audio_output_port_right[r], number_of_frames);
			if (stop_print == 0) {
#ifdef USE_PIPES
				fprintf(stderr,"jackio.c *softrock get jack pipe left(r) is: %d \n",*softrock_get_jack_read_pipe_left(r));
				fprintf(stderr,"jackio.c r : %d\n",r);
#endif
			}
			if(!softrock_get_iq()) {
#ifdef USE_PIPES
				bytes_read = read(*softrock_get_jack_read_pipe_left(r),sample_buffer_left[r],size);
				//fprintf(stderr,"Read %d bytes on left.\n", bytes_read);
				if (bytes_read != size) {
					fprintf(stderr,"In process, number read  %d  number blocked  %d \n", stop_print, num_blocked);
					if ( bytes_read == -1) { perror("Read");  
						num_blocked++;
					}
					else fprintf(stderr,"There was a problem reading from the left pipe.  Read %d bytes.\n", bytes_read);	
				}

				bytes_read = read(*softrock_get_jack_read_pipe_right(r),sample_buffer_right[r],size);
				if (bytes_read != size) {
					if (bytes_read == -1) perror("Read");
					else fprintf(stderr,"There was a problem reading from the right pipe.  Read %d bytes. \n", bytes_read);
				}
#else // Use ringbuffers
				//put right stuff to read rb here.
				if ( jack_ringbuffer_read_space (rb_left[r]) >= size )
				{
					jack_ringbuffer_read (rb_left[r], (char *)sample_buffer_left[r], size);
					if(num_ovfl > 0) 
					{
						fprintf (stderr, "Left jack buffer has space for read after %d overflows.\n",num_ovfl);
						num_ovfl = 0;
					}
				}
				else
				{
					if (num_ovfl == 0) {
						fprintf(stderr, "No space left to read in jack ringbuffers (left).\n");
					}
					num_ovfl++;
				}

				if ( jack_ringbuffer_read_space (rb_right[r]) >= size )
				{
					jack_ringbuffer_read (rb_right[r], (char *)sample_buffer_right[r], size); 
					if(num_ovfr > 0) 
					{
						fprintf (stderr, "Left jack buffer has space for read after %d overflows.\n",num_ovfr);
						num_ovfr = 0;
					}
				}
				else
				{
					if (num_ovfr == 0) {
						fprintf(stderr, "No space left to read in jack ringbuffers (right).\n");
					}
					num_ovfr++;
				}
#endif
			} else { // qi instead of iq
#ifdef USE_PIPES
				bytes_read = read(*softrock_get_jack_read_pipe_left(r),sample_buffer_right[r],size);
				if (bytes_read  != size) {
					fprintf(stderr,"There was a problem reading from the right pipe.  Read %d bytes.\n", bytes_read);
				}
				bytes_read = read(*softrock_get_jack_read_pipe_right(r),sample_buffer_left[r],size);
				if (bytes_read != size) {
					fprintf(stderr,"There was a problem reading from the right pipe.  Read %d bytes.\n", bytes_read);
				}
#else  // use ringbuffers
				if ( jack_ringbuffer_read_space (rb_left[r]) >= size )
				{
					jack_ringbuffer_read (rb_left[r], (char *)sample_buffer_right[r], size);
					if(num_ovfl > 0) 
					{
						fprintf (stderr, "Left jack buffer has space for read after %d overflows.\n",num_ovfl);
						num_ovfl = 0;
					}
				}
				else
				{
					if (num_ovfl == 0) {
						fprintf(stderr, "No space left to read in jack ringbuffers (left).\n");
					}
					num_ovfl++;
				}

				if ( jack_ringbuffer_read_space (rb_right[r]) >= size )
				{
					jack_ringbuffer_read (rb_right[r], (char *)sample_buffer_left[r], size); 
					if(num_ovfr > 0) 
					{
						fprintf (stderr, "Left jack buffer has space for read after %d overflows.\n",num_ovfr);
						num_ovfr = 0;
					}
				}
				else
				{
					if (num_ovfr == 0) {
						fprintf(stderr, "No space left to read in jack ringbuffers (right).\n");
					}
					num_ovfr++;
				}
#endif
			}
			stop_print++;
		} 
		else
		{
			start_buffer = 0;
		}
	}
	return 0;
}

#endif
	
