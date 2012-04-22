/**
* @file resampler.c
* @brief Sampling rate alteration service
* @author Alberto Trentadue, IZ0CEZ
* @version 0.1
* @date 2012-02-20
*/


#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include <samplerate.h>

#include "resampler.h"

#define CHANNELS 2  //Hardcoded for now

SRC_DATA *src_data_objs[MAX_RESAMPLERS];
RESAMPINFO resampler_info[MAX_RESAMPLERS];
static pthread_mutex_t setup_lock;
static bool init_ok = false;

void resampler_init(void) {
    if (init_ok) return;
    init_ok = true;
    pthread_mutex_init(&setup_lock, NULL );
    
    for (int i=0; i<MAX_RESAMPLERS; i++) {
        src_data_objs[i] =(SRC_DATA *)malloc(sizeof(SRC_DATA));
        resampler_info[i].used = false;
    }
}

int resampler_setup_new(int max_frames, int decim, int interp) {
//int resampler_setup_new(float *buf_in, float *buf_out, int decim, int interp) {
        
    int r_id = NO_MORE_RESAMPLERS;
    
    pthread_mutex_lock(&setup_lock);
    for (int i=0; i < MAX_RESAMPLERS; i++) {
        if (! resampler_info[i].used) {
            r_id = i;
            break;
        }
    }
    if (r_id != NO_MORE_RESAMPLERS) {
        resampler_info[r_id].used = true;
        resampler_info[r_id].max_frames = max_frames;
        resampler_info[r_id].decim = decim;
        resampler_info[r_id].interp = interp;        
            
        int max_output_frames = ((max_frames * interp) / decim +1);
                
        src_data_objs[r_id]->data_in = (float *)malloc(sizeof(float)*max_frames*CHANNELS);        
        src_data_objs[r_id]->data_out = (float *)malloc(sizeof(float)*max_output_frames*CHANNELS);        
        src_data_objs[r_id]->src_ratio = interp*1.0/decim;
    } 
    
    if (src_data_objs[r_id]->data_in == NULL || src_data_objs[r_id]->data_in == NULL)
        r_id = FAILED_RESAMPLER;
        
    pthread_mutex_unlock(&setup_lock);
    return r_id;
}

void release_resampler(int r_id) {
    resampler_info[r_id].used = false;
}

void resampler_load_channels(int r_id, float *ch1, float *ch2) {

    unsigned int idx=0;
 //pra_gma omp parallel for schedule(static) private(i)    
    for (idx=0; idx<resampler_info[r_id].max_frames; idx++) {
        src_data_objs[r_id]->data_in[CHANNELS*idx] = ch1[idx];
        src_data_objs[r_id]->data_in[CHANNELS*idx+1] = ch2[idx];        
    }
}

void resampler_load_data(int r_id, int idx, float ch1, float ch2) {

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

void resampler_fetch_data(int r_id, int idx, float *ch1, float *ch2) {
    
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
