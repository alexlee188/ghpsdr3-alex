/**
* @file audiostream.c
* @brief audio out put stream (for iPhone)
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-03-10
*/

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
* Foundation, Inc., 59 Temple Place - S
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>


#include "audiostream.h"
#include "client.h"
#include "buffer.h"
#include "codec2loc.h"


int audio_buffer_size = AUDIO_BUFFER_SIZE;
int audio_sample_rate=8000;
int audio_channels=1;
unsigned char* audio_buffer=NULL;
int send_audio=0;

TAILQ_HEAD(, audio_entry) IQ_audio_stream;

void * codec2 = NULL;
unsigned char bits[BITS_SIZE];
short codec2_buffer[CODEC2_SAMPLES_PER_FRAME];

static int sample_count=0;
static int codec2_count=0;

static int audio_stream_buffer_insert=0;

static unsigned char encodetable[65536];

unsigned char alaw(short sample);

void init_alaw_tables();

void audio_stream_init(int receiver) {
    init_alaw_tables();
    TAILQ_INIT(&IQ_audio_stream);
}

/* --------------------------------------------------------------------------*/
/**
* @brief put samples to the audio stream
*
* @return
*/


void allocate_audio_buffer(){
    if (encoding == 0) {
	audio_buffer=(unsigned char*)malloc((audio_buffer_size*audio_channels)+AUDIO_BUFFER_HEADER_SIZE);
	}
    else if (encoding == 1) {
	audio_buffer=(unsigned char*)malloc((audio_buffer_size*audio_channels*2)+AUDIO_BUFFER_HEADER_SIZE); // 2 byte PCM
	}
    else if (encoding == 2) {
    	codec2_count = 0;
	audio_buffer_size = BITS_SIZE*NO_CODEC2_FRAMES;
	audio_buffer=(unsigned char*)malloc(audio_buffer_size*audio_channels + AUDIO_BUFFER_HEADER_SIZE);
	};
}

void audio_stream_reset() {
    audio_stream_buffer_insert=0;
    audio_stream_queue_free();
    if (audio_buffer != NULL) free(audio_buffer);
    allocate_audio_buffer();
}


void audio_stream_put_samples(short left_sample,short right_sample) {
	int audio_buffer_length;
	struct audio_entry * item;

    // samples are delivered at 48K
    // output to stream at 8K (1 in 6) or 48K (1 in 1)
    // codec2 encoding works only for 8K

    if(sample_count==0) {
        // use this sample and convert to a-law or PCM or codec2
        if(audio_channels==1) {
            if (encoding == 0) audio_buffer[audio_stream_buffer_insert+AUDIO_BUFFER_HEADER_SIZE]=alaw((left_sample+right_sample)/2);
	    else if (encoding == 1) {
		audio_buffer[audio_stream_buffer_insert*2+AUDIO_BUFFER_HEADER_SIZE] = (left_sample/2+right_sample/2) & 0x00ff;
		audio_buffer[audio_stream_buffer_insert*2+1+AUDIO_BUFFER_HEADER_SIZE] = (left_sample/2+right_sample/2) >> 8;
		}
	    else if (encoding == 2) {
		codec2_buffer[audio_stream_buffer_insert] = (left_sample+right_sample)/2;
		}
            else {
		audio_buffer[audio_stream_buffer_insert+AUDIO_BUFFER_HEADER_SIZE]=alaw((left_sample+right_sample)/2); //encoding == others - defaults to alaw
		}
        } else {
	    if (encoding == 0){
            audio_buffer[(audio_stream_buffer_insert*2)+AUDIO_BUFFER_HEADER_SIZE]=alaw(left_sample);
            audio_buffer[(audio_stream_buffer_insert*2)+1+AUDIO_BUFFER_HEADER_SIZE]=alaw(right_sample);
	    }
	    else if (encoding == 1) {
		audio_buffer[audio_stream_buffer_insert*4+AUDIO_BUFFER_HEADER_SIZE] = left_sample & 0x00ff;
		audio_buffer[audio_stream_buffer_insert*4+1+AUDIO_BUFFER_HEADER_SIZE] = left_sample >> 8;
		audio_buffer[audio_stream_buffer_insert*4+2+AUDIO_BUFFER_HEADER_SIZE] = right_sample & 0x00ff;
		audio_buffer[audio_stream_buffer_insert*4+3+AUDIO_BUFFER_HEADER_SIZE] = right_sample >> 8;
		}
	    else { // encoding == others
            audio_buffer[(audio_stream_buffer_insert*2)+AUDIO_BUFFER_HEADER_SIZE]=alaw(left_sample);
            audio_buffer[(audio_stream_buffer_insert*2)+1+AUDIO_BUFFER_HEADER_SIZE]=alaw(right_sample);
	    }
        }

	audio_stream_buffer_insert++;


	if ((encoding == 2) && (audio_stream_buffer_insert == CODEC2_SAMPLES_PER_FRAME))  {
		codec2_encode(codec2, bits, codec2_buffer);
		memcpy(&audio_buffer[AUDIO_BUFFER_HEADER_SIZE+BITS_SIZE*codec2_count], bits, BITS_SIZE);
		codec2_count++;
		audio_stream_buffer_insert = 0;
		if (codec2_count >= NO_CODEC2_FRAMES){
		    audio_buffer[0]=AUDIO_BUFFER;
	//	    sprintf(&audio_buffer[1],"%f",HEADER_VERSION);
		    audio_buffer_length = BITS_SIZE*NO_CODEC2_FRAMES;
		    sprintf((char *)&audio_buffer[AUDIO_LENGTH_POSITION],"%d ", audio_buffer_length);
		    audio_stream_queue_add(audio_buffer_length+AUDIO_BUFFER_HEADER_SIZE);
		    item = audio_stream_queue_remove();
		    if (item != NULL) {
			free(item->buf);
			free(item);
			}
		    codec2_count = 0;
		    }

	    }

        if((encoding != 2) && (audio_stream_buffer_insert==audio_buffer_size)) {
		audio_buffer[0]=AUDIO_BUFFER;
//		sprintf(&audio_buffer[1],"%f",HEADER_VERSION);
		if (encoding == 1) audio_buffer_length = audio_buffer_size*audio_channels*2;
		else audio_buffer_length = audio_buffer_size*audio_channels;
		sprintf((char *)&audio_buffer[AUDIO_LENGTH_POSITION],"%d ", audio_buffer_length);
		audio_stream_queue_add(audio_buffer_length+AUDIO_BUFFER_HEADER_SIZE);
		item = audio_stream_queue_remove();
		    if (item != NULL) {
			free(item->buf);
			free(item);
			}
	    	audio_stream_buffer_insert=0;
        }
    }
    sample_count++;
    if(audio_sample_rate==48000) {
        sample_count=0;
    } else {
        if(sample_count==6) {
            sample_count=0;
        }
    }
}


void audio_stream_queue_add(int length) {
    int i;
    struct audio_entry *item;


        if(send_audio) {
		item = malloc(sizeof(*item));
		item->buf = audio_buffer;
		item->length = length;
		TAILQ_INSERT_TAIL(&IQ_audio_stream, item, entries);
                }
	allocate_audio_buffer();		// audio_buffer passed on to IQ_audio_stream.  Need new ones.

}

struct audio_entry *audio_stream_queue_remove(){
	struct audio_entry *first_item;
	first_item = TAILQ_FIRST(&IQ_audio_stream);
	if (first_item != NULL) TAILQ_REMOVE(&IQ_audio_stream, first_item, entries);
	return first_item;
}

void audio_stream_queue_free(){
	struct audio_entry *item;

	while ((item = audio_stream_queue_remove()) != NULL){
		free(item->buf);
		free(item);
		}
}

unsigned char alaw(short sample) {
    return encodetable[sample&0xFFFF];
}

void init_alaw_tables() {
    int i;

/*
    for (i = 0; i < 256; i++) {
        int input = i ^ 85;
        int mantissa = (input & 15) << 4;
        int segment = (input & 112) >> 4;
        int value = mantissa + 8;
        if (segment >= 1) value += 256;
        if (segment > 1) value <<= (segment - 1);
        if ((input & 128) == 0) value = -value;
        decodetable[i]=(short)value;
    }
*/
    for(i=0;i<65536;i++) {
        short sample=(short)i;

        int sign=(sample&0x8000) >> 8;
        if(sign != 0){
            sample=(short)-sample;
            sign=0x80;
        }

        if(sample > 32635) sample = 32635;

        int exp=7;
        int expMask;
        for(expMask=0x4000;(sample&expMask)==0 && exp>0; exp--, expMask >>= 1) {
        }
        int mantis = (sample >> ((exp == 0) ? 4 : (exp + 3))) & 0x0f;
        unsigned char alaw = (unsigned char)(sign | exp << 4 | mantis);
        encodetable[i]=(unsigned char)(alaw^0xD5);
    }

}

