/** 
* @file client.h
* @brief iPhone network interface
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-12
*/
// client.h

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

#if ! defined __CLIENT_H__
#define __CLIENT_H__

#include <sys/queue.h>
#include <ortp/ortp.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <event.h>
#include <event2/thread.h>
//#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/bufferevent_ssl.h>

// added by KD0OSS ******
int panadapterMode;
int numSamples;
int rxMeterMode;
int txMeterMode;
// **********************

// added by KD0OSS
enum PanadapterMode
{
    PANADAPTER,
    SPECTRUM,
    CSPECTRUM,
    SCOPE,
    PHASE
};

// added by KD0OSS
enum Window
{
//    FIRST = -1,
    RECTANGULAR,
    HANNING,
    WELCH,
    PARZEN,
    BARTLETT,
    HAMMING,
    BLACKMAN2,
    BLACKMAN3,
    BLACKMAN4,
    EXPONENTIAL,
    RIEMANN,
    BLKHARRIS
//    LAST,
};

enum CLIENT_CONNECTION {
	connection_unknown,
	connection_tcp = 0,
	connection_rtp
} client_connection;

typedef struct _client_entry {
        enum CLIENT_CONNECTION rtp;
	struct sockaddr_in client;
	struct bufferevent * bev;
	int fps;
	int frame_counter;
	int samples;
	RtpSession *session;
	TAILQ_ENTRY(_client_entry) entries;
} client_entry;

typedef struct _memory_entry {
	char* memory;
	TAILQ_ENTRY(_memory_entry) entries;
} memory_entry;

void client_init(int receiver);
void rtp_tx_init(void);
void tx_init(void);
void spectrum_init(void);
void spectrum_timer_init(void);
void *spectrum_thread(void *);
void *memory_thread(void *);
void client_set_timing(void);
void setprintcountry(void);
void answer_question(char *message, char *clienttype, struct bufferevent *bev);
char servername[21];
void printversion(void);
static SSL_CTX *evssl_init(void);

extern double mic_src_ratio;

extern struct dspserver_config config;
#define INADDR_DSPSERVER config.dspserver_address

#endif
