/**
* @file resampler.c
* @brief Sampling rate alteration service
* @author Alberto Trentadue, IZ0CEZ
* @version 0.1
* @date 2012-02-20
*/

#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

#include <samplerate.h>

#include "resampler.h"

#define CHANNELS 2  //Hardcoded for now

SRC_DATA *src_data_objs[MAX_RESAMPLERS];
RESAMPINFO resampler_info[MAX_RESAMPLERS];
static int next_id = 0;
static sem_t setup_semaphore;
static bool init_ok = false;

void resampler_init(void) {
    if (init_ok) return;
    init_ok = true;
    sem_init(&setup_semaphore,0,1);    
    sem_post(&setup_semaphore);
    
    for (int i=0; i<MAX_RESAMPLERS; i++) 
        src_data_objs[i] =(SRC_DATA *)malloc(sizeof(SRC_DATA));
}

int resampler_setup_new(int max_frames, int decim, int interp) {
//int resampler_setup_new(float *buf_in, float *buf_out, int decim, int interp) {
    
    int r_id;
    sem_wait(&setup_semaphore);
    if (next_id < MAX_RESAMPLERS) {
        r_id = next_id++;    
        
        resampler_info[r_id].max_frames = max_frames;
        resampler_info[r_id].decim = decim;
        resampler_info[r_id].interp = interp;        
            
        int max_output_frames = ((max_frames * interp) / decim +1);
                
        src_data_objs[r_id]->data_in = (float *)malloc(sizeof(float)*max_frames*CHANNELS);        
        src_data_objs[r_id]->data_out = (float *)malloc(sizeof(float)*max_output_frames*CHANNELS);        
        src_data_objs[r_id]->src_ratio = interp*1.0/decim;
    } else
        r_id = NO_MORE_RESAMPLERS;
    
    if (src_data_objs[r_id]->data_in == NULL || src_data_objs[r_id]->data_in == NULL)
        r_id = FAILED_RESAMPLER;
        
    sem_post(&setup_semaphore);
    return r_id;
}

void resampler_load_input(int r_id, int idx, float ch1, float ch2) {
                      
    src_data_objs[r_id]->data_in[CHANNELS*idx] = ch1;
    src_data_objs[r_id]->data_in[CHANNELS*idx+1] = ch2;
}

int do_resample(int r_id, int frames, int *out_frames_gen, const char *message) {
    
    src_data_objs[r_id]->input_frames = frames;
    //fprintf(stderr, "input_frm: %ld.\n", src_data_objs[r_id].input_frames);
    src_data_objs[r_id]->output_frames = (frames*resampler_info[r_id].interp) / resampler_info[r_id].decim +1;
    //fprintf(stderr, "output_frm: %ld.\n", src_data_objs[r_id].output_frames);
        
    int rr_retcode = src_simple(src_data_objs[r_id], SRC_SINC_FASTEST, CHANNELS);
    *out_frames_gen = src_data_objs[r_id]->output_frames_gen;
        
    message = src_strerror (rr_retcode);
    
    return rr_retcode;
}

void resampler_fetch_output(int r_id, int idx, float *ch1, float *ch2) {
    
    *ch1 = src_data_objs[r_id]->data_out[CHANNELS*idx];
    *ch2 = src_data_objs[r_id]->data_out[CHANNELS*idx+1];
}

/*
void resampler_copy_data_in(int r_id, int frames, float *copy) {
    for (int i=0; i<CHANNELS*frames; i++) {
        copy[i] = src_data_objs[r_id]->data_in[i];
    }
}

SRC_DATA * get_resampler_src_data(int r_id) {
    return src_data_objs[r_id];
}

void set_resampler_src_data(int r_id, SRC_DATA * data) {
    src_data_objs[r_id] = data;
}
*/
