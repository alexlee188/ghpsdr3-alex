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

#if ! defined __CLIENT_H__
#define __CLIENT_H__

#include <sys/queue.h>

extern int encoding;
extern int tx_length;
void client_init(int receiver);
void tx_init(void);
void spectrum_init();
void *spectrum_thread(void *);
void client_set_timing();
void printcountry();
void setprintcountry();
void printcountrythread();

struct client_entry {
	struct sockaddr_in client;
	struct bufferevent * bev;
	TAILQ_ENTRY(client_entry) entries;
};

extern double mic_src_ratio;

#endif
