/**
* @file receiver.h
* @brief manage client attachment to receivers
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

#define MAX_RECEIVERS 1

#define BUFFER_SIZE 1024

typedef struct {
  int sr;			/* sample rate of base band stream, samples/second */
				/* possibly decimated from adc sample rate */
  int bw;			/* bandwidth in Hertz */
  int gain;			/* gain above 85 dB reduction */
  int ift;			/* if type, kilohertz offset */
  long freq;			/* initial frequency for selecting the RF front end */
} SdrPlayConfig;

#include <mirsdrapi-rsp.h>	/* for pointless typedefs */


#define SAMPLE_RATE_TIMING 1

typedef struct _receiver {
  int id;
  int audio_socket;
  pthread_t audio_thread_id;
  CLIENT* client;

  SdrPlayConfig cfg;

  int connected;

  int frequency_changed;
  int frequency_did_change;	/* ack received with data packet */
  long frequency;
  int frequency_band;
  int m;			/* decimation factor: 1, 2, 4, 8, 16, 32, and 64 implemented */
  int gRdB;			/* gain reduction */
  double fsMHz;			/* adc sample rate */
  double rfMHz;			/* center frequency */
  mir_sdr_Bw_MHzT bwType;	/* bandwidth type */
  mir_sdr_If_kHzT ifType;	/* if type */

  int samplesPerPacket;		/* packet size received from Init */
  float input_buffer[BUFFER_SIZE*2];
  float output_buffer[BUFFER_SIZE*2];
  short *i_buffer;
  short *q_buffer;
  int samples;

  int mi;		// decimation index
  int mxi;		// decimation partial for I
  int mxq;		// decimation partial for Q

#if SAMPLE_RATE_TIMING
  int counter;
  struct timespec  time_start;
#endif

} RECEIVER;

extern RECEIVER receiver[MAX_RECEIVERS];

typedef struct _buffer {
  unsigned long long sequence;
  unsigned short offset;
  unsigned short length;
  unsigned char data[500];
} BUFFER;

int init_receivers(SdrPlayConfig *);
const char* attach_receiver(int rx,CLIENT* client);
const char* detach_receiver(int rx,CLIENT* client);
void start_receiver (RECEIVER *pRec);
void stop_receiver (RECEIVER *pRec);
const char* set_frequency(CLIENT* client,long f);
const char* set_preamp(CLIENT* client, bool);
const char* set_dither(CLIENT* client, bool);
const char* set_random(CLIENT* client, bool);
const char* set_attenuator(CLIENT* client, int);
void send_IQ_buffer (RECEIVER *pRec);
int read_IQ_buffer (RECEIVER *pRec);
