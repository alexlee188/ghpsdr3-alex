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


#define BUFFER_SIZE 1024

#include <sys/queue.h>
#include "sdriq.h"

struct tailq_entry {
        SAMPLE_T iq_buf[BUFFER_SIZE*2];

        /*
         * This holds the pointers to the next and previous entries in
         * the tail queue.
         */
        TAILQ_ENTRY(tailq_entry) entries;
};


#define MAX_RECEIVERS 8


typedef 
struct _SdrIqConfig {
    char  start [16];
    char  usb[256];
    int   sr;
    char  stop [16];
    
    struct timespec  time_start;
    struct timespec  time_end;
    struct timespec  time_diff;

    unsigned long ns;
} SDR_IQ_CONFIG;


typedef int bool;
#define true  (1)
#define false (~true)


//NCO spur management commands for ManageNCOSpurOffsets(...)
typedef enum {
	NCOSPUR_CMD_SET,
	NCOSPUR_CMD_STARTCAL,
	NCOSPUR_CMD_READ
}  eNCOSPURCMD ;

#define SPUR_CAL_MAXSAMPLES 300000



typedef struct _receiver {
    int id;
    int audio_socket;
    pthread_t audio_thread_id;
    pthread_t iq_thread_id;
    CLIENT* client;
    int frequency_changed;
    long frequency;
    float input_buffer[BUFFER_SIZE*2];
    float output_buffer[BUFFER_SIZE*2];
    int samples;
    //
    // specific to Hiqsdr 
    //
    SDR_IQ_CONFIG cfg;
    int           frame_counter;

    // 
    // DC offset auto calibration
    // 

	bool   m_NcoSpurCalActive;	//NCO spur reduction variables
	long   m_NcoSpurCalCount;
	double m_NCOSpurOffsetI;
	double m_NCOSpurOffsetQ;


} RECEIVER;

extern RECEIVER receiver[MAX_RECEIVERS];

typedef struct _buffer {
    unsigned long long sequence;
    unsigned short offset;
    unsigned short length;
    unsigned char data[500];
} BUFFER;



void init_receivers(SDR_IQ_CONFIG *);
const char* attach_receiver(int rx,CLIENT* client);
const char* detach_receiver(int rx,CLIENT* client);
const char* set_frequency(CLIENT* client,long f);
const char* set_preamp(CLIENT* client, bool);
const char* set_dither(CLIENT* client, bool);
const char* set_random(CLIENT* client, bool);
const char* set_attenuator(CLIENT* client, int);
void send_IQ_buffer (RECEIVER *pRec);
void ManageNCOSpurOffsets( RECEIVER *pRec, eNCOSPURCMD cmd, double* pNCONullValueI,  double* pNCONullValueQ);
void NcoSpurCalibrate (RECEIVER *pRec);


