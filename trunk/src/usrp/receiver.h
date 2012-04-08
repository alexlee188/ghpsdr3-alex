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

//The USRP Server can handle 1 receiver only, at this stage.
#define MAX_RECEIVERS 1

#define RECEIVE_BUFFER_SIZE 1024
#define IQ_SERVER_PORT 11002

typedef struct _receiver {
    int id;
    CLIENT* client;
    int frequency_changed;
    long frequency;
    pthread_t rx_thread_id;
    pthread_t rx_fwd_thread_id;
    float input_buffer[RECEIVE_BUFFER_SIZE*2];
    int samples;
} RECEIVER;

extern RECEIVER receiver[MAX_RECEIVERS];

void  init_receivers(void);
const char* attach_receiver(int rx,CLIENT* client);
const char* detach_receiver(int rx,CLIENT* client);
const char* set_frequency(CLIENT* client,long f);
void  send_IQ_buffer(RECEIVER *);

const char* inspect_receiver(RECEIVER* rx);
