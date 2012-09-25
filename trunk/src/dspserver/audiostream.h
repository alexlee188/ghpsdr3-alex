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

/**
 * @brief Possible values for audio stream encoding.
 *
 * The specific values of this enumeration are expressed here to
 * emphasize that this isan externally visible enumeration, used in the
 * client-server protocol.
 */
enum as_encoding {
    ENCODING_ALAW               = 0,
    ENCODING_PCM                = 1,
    ENCODING_CODEC2             = 2,
    ENCODING_ALAW2              = 3
};

enum as_mic_encoding {
    MIC_ENCODING_ALAW		= 0,
    MIC_ENCODING_CODEC2		= 1
};

#define AUDIO_BUFFER_SIZE 2000

struct audio_entry {
	unsigned char *buf;
	int length;
	TAILQ_ENTRY(audio_entry) entries;
};

/**
 * @brief Audio stream configuration definition.
 * 
 * This structure represents the definition of an audio stream configuration.  One copy will be kept for communication between the client and iq_thread, and one copy will be kept in iq_thread for sample processing.
 *
 * @see audiostream_conf
 */
struct audiostream_config {
    int bufsize;                /**< Size of the current buffer in samples    */
    int samplerate;             /**< Sample rate of the current buffer in
                                 *   samples per second                       */
    int channels;               /**< Channels in the current buffer
                                 *   1 = mono
                                 *   2 = stereo                               */
    enum as_encoding encoding;  /**< Wire encoding for the current buffer     */
    enum as_mic_encoding micEncoding;
    int age;                    /**< Age of the current buffer; incremented on
                                 *   each change to the configuration, reset
                                 *   when a new buffer is allocated           */
};

extern struct audiostream_config audiostream_conf;
extern sem_t audiostream_sem;


void audio_stream_reset();
void audio_stream_put_samples(short left_sample,short right_sample);
void audio_stream_init(int port);
void allocate_audio_buffer(void);
void init_alaw_tables(void);
void audio_stream_queue_add(unsigned char *buffer, int length);
struct audio_entry  *audio_stream_queue_remove(void);
void audio_stream_queue_free(void);
void Mic_stream_queue_free(void);


