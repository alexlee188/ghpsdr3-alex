#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include "usrp_audio.h"

//#define DECIM_FACT         8 

//#define SAMPLE_RATE        (CORE_BANDWIDTH/DECIM_FACT)   /* the sampling rate */
#define CHANNELS           2       /* 1 = mono 2 = stereo */
#define SAMPLES_PER_BUFFER 1024

static int SAMPLE_RATE;
static int DECIM_FACT;
static int ENABLE_AUDIO = 0;

static PaStream* stream;

void usrp_set_server_audio (int setting) {
	ENABLE_AUDIO = setting;
}

int usrp_get_server_audio (void) {
	return ENABLE_AUDIO;
}

int usrp_audio_open (int core_bandwidth) {
    int rc;
    PaStreamParameters inputParameters;
    PaStreamParameters outputParameters;
    const PaStreamInfo *info;
    int devices;
    int ddev;
    int i;
    const PaDeviceInfo* deviceInfo;

    fprintf(stderr,"usrp_audio_open: portaudio\n");

    //
    // compute the sample rate
    //
    switch (core_bandwidth) {
    case 48000:
        DECIM_FACT = 1;
        break;
    case 96000:
        DECIM_FACT = 2;
        break;
    case 192000:
        DECIM_FACT = 4;
        break;
    }
    SAMPLE_RATE = core_bandwidth / DECIM_FACT;

    rc=Pa_Initialize();
    if(rc!=paNoError) {
        fprintf(stderr,"Pa_Initialize failed: %s\n",Pa_GetErrorText(rc));
        exit(1);
    }

    devices=Pa_GetDeviceCount();
    if(devices<0) {
        fprintf(stderr,"Px_GetDeviceCount failed: %s\n",Pa_GetErrorText(devices));
    } else {
        //fprintf(stderr,"default input=%d output=%d devices=%d\n",Pa_GetDefaultInputDevice(),Pa_GetDefaultOutputDevice(),devices);

        for(i=0;i<devices;i++) {
            deviceInfo = Pa_GetDeviceInfo(i);
            fprintf(stderr,"%d - %s\n",i,deviceInfo->name);
            /*
            fprintf(stderr,"maxInputChannels: %d\n",deviceInfo->maxInputChannels);
            fprintf(stderr,"maxOututChannels: %d\n",deviceInfo->maxOutputChannels);
            fprintf(stderr,"defaultLowInputLatency: %f\n",deviceInfo->defaultLowInputLatency);
            fprintf(stderr,"defaultLowOutputLatency: %f\n",deviceInfo->defaultLowOutputLatency);
            fprintf(stderr,"defaultHighInputLatency: %f\n",deviceInfo->defaultHighInputLatency);
            fprintf(stderr,"defaultHighOutputLatency: %f\n",deviceInfo->defaultHighOutputLatency);
            fprintf(stderr,"defaultSampleRate: %f\n",deviceInfo->defaultSampleRate);
             */
        }
    }
    
    ddev=Pa_GetDefaultInputDevice();    
    //TODO: this is an assumption, should work in most cases
    inputParameters.device= (ddev>=0) ? ddev : 0;
    inputParameters.channelCount=2;
    inputParameters.sampleFormat=paFloat32;        
    inputParameters.suggestedLatency=Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;    
    inputParameters.hostApiSpecificStreamInfo=NULL;

    ddev=Pa_GetDefaultOutputDevice();
    //TODO: this is an assumption, should work in most cases
    outputParameters.device= (ddev>=0) ? ddev : 0;
    outputParameters.channelCount=2;
    outputParameters.sampleFormat=paFloat32;
    outputParameters.suggestedLatency=Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo=NULL;

    fprintf (stderr,"input device=%d output device=%d\n",inputParameters.device,outputParameters.device);
    fprintf (stderr, "Audio sample SR: %d\n", SAMPLE_RATE);
    rc=Pa_OpenStream(&stream,&inputParameters,&outputParameters,(double)SAMPLE_RATE,(unsigned long)SAMPLES_PER_BUFFER,paNoFlag,NULL,NULL);
/*
    rc=Pa_OpenDefaultStream(&stream,CHANNELS,CHANNELS,paFloat32,SAMPLE_RATE,SAMPLES_PER_BUFFER,NULL, NULL);
*/
    if(rc!=paNoError) {
        fprintf(stderr,"Pa_OpenStream failed: %s\n",Pa_GetErrorText(rc));
        exit(1);
    }

    rc = Pa_StartStream(stream);
    if(rc!=paNoError) {
        fprintf(stderr,"Pa_StartStream failed: %s\n",Pa_GetErrorText(rc));
        exit(1);
    }

    info = Pa_GetStreamInfo (stream);
    if(info!=NULL) {
        fprintf(stderr,"sample rate wanted=%d got=%f\n",SAMPLE_RATE,info->sampleRate);
    } else {
        fprintf(stderr,"Pa_GetStreamInfo returned NULL\n");
    }

    return 0;
}

int usrp_audio_close() {
    int rc=Pa_Terminate();
    if(rc!=paNoError) {
        fprintf(stderr,"Pa_Terminate failed: %s\n",Pa_GetErrorText(rc));
        exit(1);
    }
    return 0;
}

int usrp_audio_write(float* left_samples,float* right_samples) {
    int rc;
    int i;
    float audio_buffer[SAMPLES_PER_BUFFER*2];

    // interleave samples
    for(i=0;i<SAMPLES_PER_BUFFER;i++) {
        audio_buffer[i*2]=right_samples[i];
        audio_buffer[(i*2)+1]=left_samples[i];
    }

    rc=Pa_WriteStream(stream,audio_buffer,SAMPLES_PER_BUFFER);
    if(rc!=0) {
        fprintf(stderr,"error writing audio_buffer %s (rc=%d)\n",Pa_GetErrorText(rc),rc);
    }

    return rc;
}

int usrp_audio_write_decim (float* left_samples, float* right_samples)   {
    int rc;
    int i;
    float audio_buffer[SAMPLES_PER_BUFFER*2/DECIM_FACT];

    // interleave samples
    for(i=0;i<SAMPLES_PER_BUFFER;i++) {

        if ((i % DECIM_FACT) == 0) {
            audio_buffer[i/DECIM_FACT*2]=right_samples[i];
            audio_buffer[(i/DECIM_FACT*2)+1]=left_samples[i];
        }
    }

    rc = Pa_WriteStream (stream, audio_buffer, SAMPLES_PER_BUFFER/DECIM_FACT);
    if ( rc != 0 ) {
        fprintf(stderr,"error writing audio_buffer %s (rc=%d)\n", Pa_GetErrorText(rc), rc);
    }

    return rc;
}

//Proxy functions for Portaudio consumers
void usrp_process_output_buffer (float *ch1,  float *ch2, int mox) {
    int rc; 
    rc=usrp_audio_write_decim(ch1, ch2);
    /*
    for(i=0;i<SAMPLES_PER_BUFFER;i++) {

        if ((i % DECIM_FACT) == 0) {
            fprintf(stderr,"left sample %d: %f\n",i, ch1[i]);            
        }
    }
     */
    
}

