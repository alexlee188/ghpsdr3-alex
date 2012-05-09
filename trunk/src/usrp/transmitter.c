/**
* @file transmitter.c
* @brief management of transmitting features - header
* @author Alberto Trentadue, IZ0CEZ
* @version 0.1
* @date 2012-02-10
*/

#include <stdlib.h>
#include <stdio.h>
  #ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
  #else
#include <winsock.h>
#include "pthread.h"
  #endif
#include <string.h>
#include <fftw3.h>

#include "buffer.h"
#include "client.h"
#include "receiver.h"
#include "transmitter.h"
#include "messages.h"
#include "usrp_audio.h"

#define SMALL_PACKETS

static short tx_audio_port=TX_AUDIO_PORT; // = 15000 as constant
static bool TRANSMITTER_CYCLE = false;
static pthread_mutex_t TRANSMITTER_CYCLE_lock;

TRANSMITTER transmitter;

void init_transmitter(void) {
    
    struct sockaddr_in audio;
    int on=1;
    int audio_length;
    
    transmitter.client=(CLIENT*)NULL;

    transmitter.tx_audio_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(transmitter.tx_audio_socket<0) {
        perror("Client Handler: create socket failed for server tx audio socket");
        exit(1);
    }

    setsockopt(transmitter.tx_audio_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    audio_length=sizeof(audio);
    memset(&audio,0,audio_length);
    audio.sin_family=AF_INET;
    audio.sin_addr.s_addr=htonl(INADDR_ANY);
    //audio.sin_port=htons(tx_audio_port+(rx->id*2));
    //only 1 transmitter allowed.
    audio.sin_port=htons(tx_audio_port);

    if(bind(transmitter.tx_audio_socket,(struct sockaddr*)&audio,audio_length)<0) {
        perror("Transmitter: bind socket failed for tx server audio socket");
        exit(1);
    }
    fprintf(stderr,"Tx audio on port %d\n", tx_audio_port);        
    
    pthread_mutex_init(&TRANSMITTER_CYCLE_lock, NULL );
}

const char* attach_transmitter(CLIENT* client, const char *rx_attach_message) {

    //TX allowed only if there is a receiver
    if(client->receiver_state!=RECEIVER_ATTACHED) {
        return RECEIVER_NOT_ATTACHED;
    }

    //This allows only RX#0 to TX
    if(client->receiver_num!=0) {
        return RECEIVER_NOT_ZERO;
    }

    client->transmitter_state=TRANSMITTER_ATTACHED;
    transmitter.client = client;

    return rx_attach_message;
}

void stop_tx_audio(CLIENT* client) {

    if(transmitter.client!=client) {
        return;
    }
    //Stops the transmitter thread
    pthread_mutex_lock(&TRANSMITTER_CYCLE_lock); 
    TRANSMITTER_CYCLE = false;
    pthread_mutex_unlock(&TRANSMITTER_CYCLE_lock);        

    client->transmitter_state=TRANSMITTER_DETACHED;
    transmitter.client=(CLIENT*)NULL;    
}

/* 
 * The TX audio thread, consuming audio transferred by the dspserver.
 * 
 * The audio can be used to modulate the carrier or can be diverted to
 * the local audio card (for testing purposes), depending on the
 * command line option.
 */ 
void * tx_audio_thread(void* arg) {    
    CLIENT *client=(CLIENT*)arg;
    RECEIVER *rx=&receiver[client->receiver_num];
    TRANSMITTER *tx=&transmitter;
    struct sockaddr audio;
    int audio_length;
    int old_state, old_type;
    int bytes_read;
    unsigned long long tx_sequence=0L;
    int offset = 0;
    bool buf_loading=false;
        
    BUFFER buffer;

    fprintf(stderr,"Starting tx_audio_thread for client %d\n",rx->id);
    bool do_cycle = TRANSMITTER_CYCLE = true;
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&old_state);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&old_type);    
    
    while(do_cycle) {        

        pthread_mutex_lock(&TRANSMITTER_CYCLE_lock);             
        do_cycle = TRANSMITTER_CYCLE;
        pthread_mutex_unlock(&TRANSMITTER_CYCLE_lock);
        
#ifdef SMALL_PACKETS
            
        if (! do_cycle) break; 
                
        bytes_read=recvfrom(tx->tx_audio_socket,(char*)&buffer,sizeof(buffer),0,&audio,(socklen_t*)&audio_length);
        if(bytes_read<0) {
            fprintf(stderr,"recvfrom socket failed for TX audio. Exiting.");
            exit(1);
        }
        
        //fprintf(stderr,"BUFFER idxs: buf_seq=%Ld, buf_off=%d\n", buffer.sequence, buffer.offset);
        if(buffer.offset==0) { //New Buffer Start
            if (buf_loading) { //End of a loaded buffer: process it
                //REMEMBER: the tx->output_buffer carries 2 channels: 
                //tx->output_buffer[0..TRANSMIT_BUFFER_SIZE-1] and
                //tx->output_buffer[TRANSMIT_BUFFER_SIZE..2*TRANSMIT_BUFFER_SIZE-1]        
                usrp_process_audio_buffer(tx->output_buffer, get_mox(client));                                       
            }
            //Start of a new buffer loading
            offset=0;
            tx_sequence=buffer.sequence;                
            buf_loading = true;
        } else {                
            //buffer loading
            if((tx_sequence != buffer.sequence) || (offset != buffer.offset)) {
                //Frame loss detection
                fprintf(stderr,"Tx audio frames loss! tx_seq=%Ld, buf_seq=%Ld, offset=%d, buf_off=%d.\n",
                    tx_sequence, buffer.sequence, offset, buffer.offset);
                //Realign
                tx_sequence=buffer.sequence;
                offset=buffer.offset;                
            }    
        }              
        memcpy((char *)&tx->output_buffer[buffer.offset/4],(char *)&buffer.data[0],buffer.length);
        offset+=buffer.length; //next expected offset                                                                                        
                
#else
        bytes_read=recvfrom(tx->tx_audio_socket,(char*)tx->output_buffer,
            TRANSMIT_BUFFER_SIZE*2*sizeof(float),0,(struct sockaddr*)&audio,(socklen_t*)&audio_length);
        if(bytes_read<0) {
            perror("recvfrom socket failed for tx audio");
            exit(1);
        }
        //REMEMBER: the tx->output_buffer carries 2 channels: 
        //tx->output_buffer[0..TRANSMIT_BUFFER_SIZE-1] and
        //tx->output_buffer[TRANSMIT_BUFFER_SIZE..2*TRANSMIT_BUFFER_SIZE-1]        
        usrp_process_audio_buffer(tx->output_buffer, get_mox(client));		
        
#endif
        			                                        
    }
    
    fprintf(stderr,"Exiting from tx_audio_thread for client %d\n",rx->id);
    
    return NULL;
}

//The TX samples receiver thread from the remote client
int start_tx_audio_thread(CLIENT * client) {
 
    int rc=pthread_create(&client->tx_thread_id,NULL,tx_audio_thread,client);
    if (rc!=0)
        fprintf(stderr,"Failed to create TX audio thread for rx %d\n",client->receiver_num);
    return rc;   
}

//FUTURE USE...?
void process_microphone_samples(float* samples) {

    // equalizer

    // agc

    // vox

    // gain 0 to 20dB

    // I band pass filter

    // Q band pass filter
    // Q Hilbert transform

    // G3PLX speech processor

    // I band pass filter

    // Q bandpass filter


}

void set_filter(int high,int low) {
    // configure the bandpass filter
}

void bandpass_filter(float* input,float* output) {
}

