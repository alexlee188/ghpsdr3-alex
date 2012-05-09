#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <portaudio.h>

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#ifdef _OPENMP
#include <omp.h>
#endif

#include "client.h"
#include "receiver.h"
#include "usrp.h"
#include "usrp_audio.h"
#include "transmitter.h"
#include "util.h"


#define CHANNELS 2       /* 1 = mono 2 = stereo */
#define BUFFER_DISCARDED -1

#define AUDIO_TO_NOTHING 0
#define AUDIO_TO_USRP_MODULATION 1
#define AUDIO_TO_LOCAL_CARD 2

#define AUDIO_TO_NOTHING 0
#define AUDIO_TO_USRP_MODULATION 1
#define AUDIO_TO_LOCAL_CARD 2

static int SAMPLE_RATE;
static int DECIM_FACT;
static int AUDIO_DESTINATION = -1;
static int (*audio_processor)(float*, int) = NULL;
static float audio_buffer[TRANSMIT_BUFFER_SIZE*2]; //Max size
static int AUDIO_BUFFER_SIZE = 0;

static PaStream* stream;

int usrp_local_audio_write_decim (float* left_samples, int mox);
int usrp_drop_audio_buffer(float *outbuf, int mox);


void usrp_disable_path(const char *path) {
    
    if (strcasecmp(path, "tx") == 0) {
        AUDIO_DESTINATION = AUDIO_TO_NOTHING;
        audio_processor = usrp_drop_audio_buffer;
        fprintf(stderr,"Discarding client generated TX baseband samples.\n");
    } else
    if (strcasecmp(path, "rx") == 0) {
        usrp_disable_rx_path();
        fprintf(stderr,"Discarding USRP RX samples\n");
    }
}

void usrp_set_server_audio (char* setting) {
    
    //If audio destination is already set to NOTHING, return
    if (AUDIO_DESTINATION == AUDIO_TO_NOTHING) return;
    
    if (strcasecmp(setting, "card") == 0) {
        AUDIO_DESTINATION = AUDIO_TO_LOCAL_CARD;
        audio_processor = usrp_local_audio_write_decim;
        fprintf(stderr,"Sending client generated audio to LOCAL CARD\n");
    } else
    if (strcasecmp(setting, "usrp") == 0) {
        AUDIO_DESTINATION = AUDIO_TO_USRP_MODULATION;
        audio_processor = usrp_process_tx_modulation;
        fprintf(stderr,"Sending client generated audio to USRP MODULATION\n");
    }
    else {
        fprintf(stderr,"Illegal setting %s for audio-to. Using default: USRP", setting);        
    }
}

int usrp_get_server_audio (void) {
	return AUDIO_DESTINATION;
}

int usrp_local_audio_open(int core_bandwidth) {
    int rc;
    PaStreamParameters inputParameters;
    PaStreamParameters outputParameters;
    const PaStreamInfo *info;
    int devices;
    int ddev;
    int i;
    const PaDeviceInfo* deviceInfo;

    //fprintf(stderr,"usrp_audio_open: portaudio\n");

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
    AUDIO_BUFFER_SIZE = TRANSMIT_BUFFER_SIZE*2/DECIM_FACT;

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
    //ATTN: this is an assumption, should work in most cases
    inputParameters.device= (ddev>=0) ? ddev : 0;
    inputParameters.channelCount=2;
    inputParameters.sampleFormat=paFloat32;        
    inputParameters.suggestedLatency=Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;    
    inputParameters.hostApiSpecificStreamInfo=NULL;

    ddev=Pa_GetDefaultOutputDevice();
    //ATTN: this is an assumption, should work in most cases
    outputParameters.device= (ddev>=0) ? ddev : 0;
    outputParameters.channelCount=2;
    outputParameters.sampleFormat=paFloat32;
    outputParameters.suggestedLatency=Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo=NULL;

    fprintf (stderr,"input device=%d output device=%d\n",inputParameters.device,outputParameters.device);
    fprintf (stderr, "Audio sample SR: %d\n", SAMPLE_RATE);
    //rc=Pa_OpenStream(&stream,&inputParameters,&outputParameters,(double)SAMPLE_RATE,(unsigned long)TRANSMIT_BUFFER_SIZE,paNoFlag,NULL,NULL);
    rc=Pa_OpenStream(&stream,&inputParameters,&outputParameters,(double)SAMPLE_RATE,paFramesPerBufferUnspecified,paNoFlag,NULL,NULL);
    //rc=Pa_OpenDefaultStream(&stream,CHANNELS,CHANNELS,paFloat32,SAMPLE_RATE,TRANSMIT_BUFFER_SIZE,NULL, NULL);

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

/*
 * Setup the audio processing 
 */
int usrp_audio_open (int core_bandwidth) {
    int rc=0;         
    switch(AUDIO_DESTINATION) {            
        
            case AUDIO_TO_LOCAL_CARD: 
                rc=usrp_local_audio_open(core_bandwidth);
                break;
                                
            case AUDIO_TO_USRP_MODULATION:
                rc=0;
                break;
                
            case AUDIO_TO_NOTHING:                
                break;                           
        }
    return rc;
}

// Used?
int usrp_local_audio_close() {
    int rc=Pa_Terminate();
    if(rc!=paNoError) {
        fprintf(stderr,"Pa_Terminate failed: %s\n",Pa_GetErrorText(rc));
        exit(1);
    }
    return 0;
}

int usrp_local_audio_write(float* left_samples,float* right_samples) {
    int rc;
    int i;

    // interleave samples
    for(i=0;i<TRANSMIT_BUFFER_SIZE;i++) {
        audio_buffer[i*2]=right_samples[i];
        audio_buffer[(i*2)+1]=left_samples[i];
    }

    rc=Pa_WriteStream(stream,audio_buffer,TRANSMIT_BUFFER_SIZE);
    if(rc!=0) {
        fprintf(stderr,"error writing audio_buffer %s (rc=%d)\n",Pa_GetErrorText(rc),rc);
    }

    return rc;
}

//implements pointer audio_processor
int usrp_local_audio_write_decim (float* samples, int mox) {
    int rc=0, i, j;
    float *left_samples = samples;    
    float *right_samples = &left_samples[TRANSMIT_BUFFER_SIZE];    

    // interleave samples     
    for(i=j=0;i<TRANSMIT_BUFFER_SIZE;i++) {

        if ((i % DECIM_FACT) == 0) {
            audio_buffer[j*2]=right_samples[i];
            audio_buffer[j*2+1]=left_samples[i];
            j++;
        }
    }    

    //TEMP
    //dump_float_buffer_heads(audio_buffer);
    //rc=0;
    if (mox==1)
        rc = Pa_WriteStream (stream, audio_buffer, AUDIO_BUFFER_SIZE);
    if ( rc != 0 ) {
        fprintf(stderr,"error writing audio_buffer %s (rc=%d)\n", Pa_GetErrorText(rc), rc);
    }

    return rc;
}

//implements pointer audio_processor
int usrp_drop_audio_buffer(float *outbuf, int mox) {
    
    //Do nothing stub to drop - or inspect, buffers
    //dump_float_buffer(outbuf);
    return 0;
}

//Proxy function for audio consumers
        //REMEMBER: the outbuf carries I & Q channels: 
        //outbuf[0..TRANSMIT_BUFFER_SIZE-1] and
        //outbuf[TRANSMIT_BUFFER_SIZE..2*TRANSMIT_BUFFER_SIZE-1]
void usrp_process_audio_buffer (float *outbuf, int mox) {
    
    //audio_processor interface function pointer
    int rc=audio_processor(outbuf, mox);    
    if (rc != 0) {
        fprintf(stderr,"USRP Audio buffer processing returns non-zero: %d (AUDIO DESTINATION: %d)\n",
            rc, AUDIO_DESTINATION);        
    }
}

