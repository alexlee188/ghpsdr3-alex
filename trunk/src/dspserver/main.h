/** 
* @file main.h
* @brief The main files headers files.
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-11
*/
// main.h
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

// version string moved into main.c

#if ! defined __MAIN_H__
#define __MAIN_H__

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/param.h>

#include "client.h"
#include "dttsp.h"
#include "audiostream.h"
#include "soundcard.h"
#include "ozy.h"
#include "version.h"
#include "codec2loc.h"
#include "register.h"
#include "sdrexport.h"
#include "G711A.h"
#include "rtp.h"
#include "util.h"

#define SERVER_ADDRESS_LENGTH 256
extern const char *version;

struct dspserver_config {
    char soundCardName[80];
    int offset;
    char share_config_file[MAXPATHLEN];
    char server_address[SERVER_ADDRESS_LENGTH];
    int thread_debug;
    int no_correct_iq;
    int16_t client_base_port; // This + rx# is what the QtRadio or glsdr clients connect to.
    int16_t ssl_client_port;
    int16_t server_port; // This port is what communicates with the server (hardware server like hpsdr-server).
    int16_t command_port;
    int16_t spectrum_port;
    int16_t audio_port;
};

#endif

