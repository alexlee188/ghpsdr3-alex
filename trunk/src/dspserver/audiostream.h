/**
* @file audiostream.h
* @brief Header files for the audio output stream
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

/* --------------------------------------------------------------------------*/
/**
* @brief put samples to the audio stream
*
* @return
*/

#include <sys/queue.h>

#define ENCODING_ALAW 0
#define ENCODING_PCM 1
#define ENCODING_CODEC2 2
#define ENCODING_ALAW2 3

#define AUDIO_BUFFER_SIZE 400

int audio_buffer_size;
int audio_sample_rate;
int audio_channels;
unsigned char* audio_buffer;
int send_audio;

struct audio_entry {
	unsigned char *buf;
	int length;
	TAILQ_ENTRY(audio_entry) entries;
};


void audio_stream_reset();
void audio_stream_put_samples(short left_sample,short right_sample);
void audio_stream_init(int port);
void allocate_audio_buffer(void);
void init_alaw_tables(void);
void audio_stream_queue_add(int length);
struct audio_entry  *audio_stream_queue_remove(void);
void audio_stream_queue_free(void);
void Mic_stream_queue_free(void);


