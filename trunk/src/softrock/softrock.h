/**
* @file softrock.h
* @brief softrock implementation
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
#if !defined __softrock_H__
#define __softrock_H__

#define PULSEAUDIO
//#define PORTAUDIO
//#define DIRECTAUDIO
#define JACKAUDIO
//#define USE_PIPES //A quick test seems to indicate that ringbuffers work with less overruns.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#define _GNU_SOURCE  /* See feature_test_macros(7) */
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <jack/ringbuffer.h>

#define ADD_RX 1
#define DEC_RX -1

int create_softrock_thread(void);
void softrock_set_device(char* d);
char* softrock_get_device(void);
void softrock_set_receivers(int r);
int softrock_get_receivers(void);
void softrock_set_sample_rate(int r);
int softrock_get_sample_rate();

void softrock_set_input(char* d);
char* softrock_get_input(void);
void softrock_set_output(char* d);
char* softrock_get_output(void);

void softrock_set_iq(int s);
int softrock_get_iq(void);

void softrock_set_record(char* filename);
void softrock_set_playback(char* filename);
void softrock_record_buffer(char* buffer,int length);
void softrock_playback_buffer(char* buffer,int length);

void softrock_set_jack(int flag);
int softrock_get_jack(void);
#ifdef USE_PIPES
int * softrock_get_jack_read_pipe_left(int rx);
int * softrock_get_jack_read_pipe_right(int rx);
int * softrock_get_jack_write_pipe_left(int rx);
int * softrock_get_jack_write_pipe_right(int rx);
#else // Use ringbuffers
jack_ringbuffer_t * softrock_get_jack_rb_left(int rx);
jack_ringbuffer_t * softrock_get_jack_rb_right(int rx);
void delete_jack_ringbuffers(void);
#define JACK_RINGBUFFER_SZ 1048576
#endif
void softrock_set_client_active_rx(int receiver, int inc);
int softrock_get_client_active_rx(int receiver);

void softrock_set_rx_frame(int frame);
int softrock_get_rx_frame(void);

void softrock_set_input_buffers(int buffers);
int softrock_get_input_buffers(void);

int softrock_get_record(void);
int softrock_get_playback(void);

void process_softrock_output_buffer(float* left_output_buffer,float* right_output_buffer);

void softrock_set_verbose(int flag);
int softrock_get_verbose(void);

#endif
