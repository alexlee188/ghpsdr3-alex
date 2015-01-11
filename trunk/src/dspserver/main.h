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

#include <sys/param.h>     // for MAXPATHLEN

extern const char *version;

struct dspserver_config {
    char soundCardName[80];
    int offset;
    char share_config_file[MAXPATHLEN];
    char server_address[256];
    char dspserver_address[256];
    int thread_debug;
    int no_correct_iq;
};

extern struct dspserver_config config;

#endif

