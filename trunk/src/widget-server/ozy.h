/**
* @file ozy.h
* @brief Ozy protocol implementation
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

#ifndef __linux__
int ozy_init(void);
#endif
int ozy_init(void);

void ozy_prime();
void ozy_set_buffers(int buffers);
int create_ozy_thread();
void ozy_set_receivers(int r);
int ozy_get_receivers();
void ozy_set_sample_rate(int r);
int ozy_get_sample_rate();
void ozy_set_preamp(int p);
void ozy_set_dither(int dither);
void ozy_set_random(int random);
void ozy_set_10mhzsource(int source);
void ozy_set_122_88mhzsource(int source);
void ozy_set_micsource(int source);

void ozy_set_timing(int t);

void ozy_set_record(char* filename);
void ozy_set_playback(char* filename);
void ozy_set_playback_sleep(int sleep);

void ozy_set_class(int);


void process_ozy_input_buffer(char* buffer);
void process_ozy_output_buffer(float*, float*, int);

void ozy_set_metis(int state);

void ozy_set_open_collector_outputs(int oc);

void ozy_stop_record(void);



