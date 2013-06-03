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
#include <semaphore.h>
#ifdef _OPENMP
#include <omp.h>
#endif


#include "audiostream.h"
#include "client.h"
#include "buffer.h"
#include "codec2loc.h"
#include "util.h"


int audio_buffer_size = AUDIO_BUFFER_SIZE;
int audio_sample_rate=8000;
int audio_channels=1;

/**
 * @brief Client audio stream configuration.
 *
 * This structure is configured by the client to declare the audio
 * stream characteristics it wants to receive.  The configuration is
 * protected by audiostream_sem.  Each time an audio buffer is
 * allocated, the iq_thread consults this configuration and makes a
 * copy.  The audio stream configuration in iq_thread is then unchanged
 * until the buffer is disposed (by either passing it to the client, or
 * discarding it), at which point a new configuration is read.  This
 * prevents iq_thread from having to take the lock at each sample.
 *
 * If, at disposal, the audio stream configuration has changed since the
 * current buffer was allocated, the current buffer id discarded rather
 * than giving the client audio it does not expect. The default
 * configuration is for a buffer containing 50ms of audio, so this means
 * that audio stream configuration changes will cause 50ms of audio to
 * be lost.  If the client makes its buffers deeper or changes the
 * sample rate, this buffer may represent a longer or shorter period of
 * time.  If there is a desire to make it substantially longer, this
 * scheme will have to be reconsidered.
 *
 * The specific members of this structure are discussed in the
 * definition of struct audiostream_config.
 *
 * @see iq_thread
 * @see audiostream_sem
 * @see struct audiostream_config
 */
struct audiostream_config audiostream_conf = { AUDIO_BUFFER_SIZE, 8000, 1, 0 };

/**
 * @brief Protects audiostream_conf.
 *
 * @see audiostream_conf
 */
sem_t audiostream_sem;

/**
 * @brief Audio stream configuration used during sample processing.
 *
 * This instance must be accessed only by iq_thread.  The initial
 * definition must match audiostream_conf exactly!
 */
static struct audiostream_config as_conf_cache = { AUDIO_BUFFER_SIZE,
                                                   8000, 1, 0 };

static unsigned char* audio_buffer=NULL;

static int samples_per_frame, bits_per_frame;

// bits_per_frame is now a variable
#undef BITS_SIZE
#define BITS_SIZE   ((bits_per_frame + 7) / 8)

void * codec2 = NULL;
//unsigned char bits[BITS_SIZE];
unsigned char *bits;
// short codec2_buffer[CODEC2_SAMPLES_PER_FRAME];
short *codec2_buffer;

static int codec2_count=0;

static int audio_stream_buffer_insert=0;

static unsigned char encodetable[65536];

//static struct sdr_thread_id audiostream_tid = SDR_THREAD_ID;

unsigned char alaw(short sample);

void init_alaw_tables();


/* --------------------------------------------------------------------------*/
/**
* @brief put samples to the audio stream
*
* @return
*/


void allocate_audio_buffer(){
    int samplesize;
    sdr_thread_assert_id(&audiostream_tid);
    sem_wait(&audiostream_sem);
    if (audiostream_conf.age > 0) {
        /* The stream configuration has changed, and we need to update
         * our copy to reflect that. */
        memcpy(&as_conf_cache, &audiostream_conf, sizeof(as_conf_cache));
        audiostream_conf.age = 0;
    }
    sem_post(&audiostream_sem);

    /* From this point until the buffer allocated here is handed to the
     * client or discarded, all iq_thread operations are on
     * as_conf_cache, NOT audiostream_conf. */
    switch (as_conf_cache.encoding) {
    case ENCODING_ALAW:
        /* 8 bits per sample */
        samplesize = as_conf_cache.bufsize * as_conf_cache.channels;
        break;
    case ENCODING_PCM:
        /* 16 bit per sample */
        samplesize = as_conf_cache.bufsize * as_conf_cache.channels * 2;
        break;
    case ENCODING_CODEC2:
        /* FIXME: This seems like the wrong place for this? */
        codec2_count = 0;
        /* Force buffer size */
        as_conf_cache.bufsize = BITS_SIZE * NO_CODEC2_FRAMES;
        samplesize = as_conf_cache.bufsize * as_conf_cache.channels;
        break;
    default:
        /* No ENCODING_ALAW2! */
        samplesize = 0;
    }
    audio_buffer = malloc(samplesize + AUDIO_BUFFER_HEADER_SIZE);
}

void audio_stream_reset() {
    /* FIXME: This isn't atomic */
    audio_stream_queue_free();
    Mic_stream_queue_free();
}

/**
 * @brief Process a single stereo audio sample.
 *
 * Operates only on as_conf_cache, and requires no synchronization as
 * long as the calling thread is always iq_thread.
 */
void audio_stream_put_samples(short left_sample,short right_sample) {
    int audio_buffer_length;

    sdr_thread_assert_id(&audiostream_tid);

    /* FIXME: This really only applies once, at startup */
    if (!audio_buffer) {
        allocate_audio_buffer();
	samples_per_frame = codec2_samples_per_frame( codec2 );
	bits_per_frame = codec2_bits_per_frame( codec2 );
	codec2_buffer = (short *) malloc( sizeof( short ) * samples_per_frame );
	bits = (unsigned char *) malloc( sizeof( unsigned char ) * BITS_SIZE );
    }

    // samples are delivered at 48K or 8K depending on audiostream_conf.samplerate
    // codec2 encoding works only for 8K

        int offset;
        // use this sample and convert to a-law or PCM or codec2
        if(as_conf_cache.channels==1) {
            switch (as_conf_cache.encoding) {
            default: /* ALAW */
            case ENCODING_ALAW:
                offset = audio_stream_buffer_insert + AUDIO_BUFFER_HEADER_SIZE;
                audio_buffer[offset] = alaw((left_sample + right_sample) / 2);
                break;
            case ENCODING_PCM:
                offset = audio_stream_buffer_insert * 2 + AUDIO_BUFFER_HEADER_SIZE;
                /* PCM on the wire is always LE? */
                audio_buffer[offset] = (left_sample/2 + right_sample/2) & 0xff;
                audio_buffer[offset + 1] = (left_sample/2 + right_sample/2) >> 8;
                break;
            case ENCODING_CODEC2:
                codec2_buffer[audio_stream_buffer_insert] = left_sample/2 + right_sample/2;
                break;
            }
        } else {
            switch (as_conf_cache.encoding) {
            default: /* ALAW */
            case ENCODING_ALAW:
                offset = audio_stream_buffer_insert * 2 + AUDIO_BUFFER_HEADER_SIZE;
                audio_buffer[offset] = alaw(left_sample);
                audio_buffer[offset + 1] = alaw(right_sample);
                break;
            case ENCODING_PCM:
                offset = audio_stream_buffer_insert * 4 + AUDIO_BUFFER_HEADER_SIZE;
                audio_buffer[offset] = left_sample & 0x00ff;
                audio_buffer[offset + 1] = left_sample >> 8;
                audio_buffer[offset + 2] = right_sample & 0x00ff;
                audio_buffer[offset + 3] = right_sample >> 8;
                break;
            }
        }

	audio_stream_buffer_insert++;


	if ((as_conf_cache.encoding == ENCODING_CODEC2)
            && (audio_stream_buffer_insert == samples_per_frame))  {
            codec2_encode(codec2, bits, codec2_buffer);
            memcpy(&audio_buffer[AUDIO_BUFFER_HEADER_SIZE+BITS_SIZE*codec2_count], bits, BITS_SIZE);
            codec2_count++;
	    audio_stream_buffer_insert = 0;
            if (codec2_count >= NO_CODEC2_FRAMES){
                audio_buffer[0]=AUDIO_BUFFER;

// g0orx binary header
		audio_buffer_length = BITS_SIZE*NO_CODEC2_FRAMES;
                audio_buffer[1]=HEADER_VERSION;
                audio_buffer[2]=HEADER_SUBVERSION;
                audio_buffer[3]=(audio_buffer_length>>8)&0xFF;
                audio_buffer[4]=audio_buffer_length&0xFF;
		audio_stream_queue_add(audio_buffer, audio_buffer_length+AUDIO_BUFFER_HEADER_SIZE);
		audio_buffer = NULL;	// now that the audio_buffer pointer has been added to queue
		codec2_count = 0;
		audio_stream_buffer_insert = 0;
                //allocate_audio_buffer();
            }
        }

        if ((as_conf_cache.encoding != ENCODING_CODEC2)
           && (audio_stream_buffer_insert==as_conf_cache.bufsize)) {
            audio_buffer[0]=AUDIO_BUFFER;
            audio_buffer_length = as_conf_cache.bufsize * as_conf_cache.channels;
            if (as_conf_cache.encoding == ENCODING_PCM)
                audio_buffer_length *= 2;
            audio_buffer[1]=HEADER_VERSION;
            audio_buffer[2]=HEADER_SUBVERSION;
            /* FIXME: htons */
            audio_buffer[3]=(audio_buffer_length>>8)&0xFF;
            audio_buffer[4]=audio_buffer_length&0xFF;
            audio_stream_queue_add(audio_buffer, audio_buffer_length+AUDIO_BUFFER_HEADER_SIZE);
	    audio_buffer = NULL;	// audio_buffer pointer has already been queued.
            audio_stream_buffer_insert=0;
            //allocate_audio_buffer();
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
    #pragma omp parallel for schedule(static) private(i)
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
