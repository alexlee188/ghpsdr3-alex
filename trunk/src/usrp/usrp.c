/**
* @file server.c
* @brief USRP intefrace handler
* @author Andrea Montefusco IW0HDV, Alberto Trentadue IZ0CEZ
* @author derived from the softrock implementation by John Melton, G0ORX/N6LYT
* @version 0.1
* @date 2009-10-13
*/

/* Copyright (C)
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

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <complex>
#if defined __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#endif
#ifdef _OPENMP
#include <omp.h>
#endif

#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <samplerate.h>

#include <uhd/usrp/multi_usrp.hpp>

#include "client.h"
#include "receiver.h"
#include "transmitter.h"
#include "usrp.h"

//RX output sample rate from USRP
#define USRP_RX_SAMPLERATE 256000
#define USRP_TX_SAMPLERATE 256000 //TODO: what is the rate here???
#define DEFAULT_RX_FREQUENCY 7056000
#define RRESAMPLER_NO_ERROR 0

class USRP {
public:
   uhd::usrp::multi_usrp::sptr sdev;
   uhd::device::sptr            dev;
   int receivers;
   bool tx_enabled;
};

static USRP usrp;
static int DECIM = 16;
static int INTERP = 3;  //For the rational resampler
// for the swap option
static int real_position = 1;
static int imag_position = 0;

// TX audio sample is the HEAD of a queue to be forwarded to USRP
TAILQ_HEAD(, _buffer_entry) tx_audio_iq_stream;
#define MAX_QUEUE 50 //Max 50KSAMPLES in queue
static sem_t tx_audio_semaphore;
static int queue_length = 0;


//Defines and initialises the tx buffer queue
void setup_tx_queue(void) {
    
    TAILQ_INIT(&tx_audio_iq_stream); 

    sem_init(&tx_audio_semaphore,0,1);
    signal(SIGPIPE, SIG_IGN);
    sem_post(&tx_audio_semaphore);
}

//USRP initialisation
bool usrp_init (const char *rx_subdev_par, const char *tx_subdev_par)
{
    uhd::device_addr_t hint;
    hint["type"] = "usrp1";

    //discover the usrps and print the results
    uhd::device_addrs_t device_addrs = uhd::device::find(hint);

    if (device_addrs.size() == 0){
        std::cerr << "*** FATAL: No UHD devices Found" << std::endl;
        return false;
    }

    for (size_t i = 0; i < device_addrs.size(); i++){
        std::cout << "--------------------------------------------------" << std::endl;
        std::cout << "-- UHD Device " << i << std::endl;
        std::cout << "--------------------------------------------------" << std::endl;
        std::cout << device_addrs[i].to_pp_string() << std::endl << std::endl;

        //uhd::device::make(device_addrs[i]); //test make

        // USRP discover
        usrp.sdev = uhd::usrp::multi_usrp::make(device_addrs[i]);
        usrp.dev = usrp.sdev->get_device();
        
        usrp.tx_enabled = false;        
        char subdev_specs[12] = "";
        strcpy(subdev_specs, rx_subdev_par);
        strcat(subdev_specs, " "); strcat(subdev_specs, tx_subdev_par);
        //fprintf(stderr, "USRP Subdev specs: %s\n", subdev_specs);
        usrp_set_subdev_args(subdev_specs);
        
        //set the USRP rx standard sample rate 256000 for minimum bandwidth.
        usrp.sdev->set_rx_rate(USRP_RX_SAMPLERATE);
        //TODO: set the USRP tx standard sample rate ?????
        usrp.sdev->set_tx_rate(USRP_TX_SAMPLERATE);
        std::cout << boost::format("USRP RX Rate: %f Msps...") % (usrp.sdev->get_rx_rate()/1e6) << std::endl;        
        std::cout << boost::format("USRP TX Rate: %f Msps...") % (usrp.sdev->get_tx_rate()/1e6) << std::endl; 

        usrp_set_frequency(DEFAULT_RX_FREQUENCY); //TEMPORARY      
        sleep(1); //allow for some setup time
        
        setup_tx_queue();
        
        return true;
    }
    return false;
}

void usrp_deinit (void)
{
  // anything here???
}

//Configures the RX subdevice
void usrp_set_rx_subdevice(char *rxspec) 
{
    std::cout << boost::format("Setting RX subdevice spec %s") % rxspec << std::endl;
    uhd::usrp::subdev_spec_t s(rxspec);
    usrp.sdev->set_rx_subdev_spec(s);
    std::cerr << boost::format("Current RX subdevice is: %s") % usrp.sdev->get_rx_subdev_name() << std::endl;
}

//Configures the TX subdevice
void usrp_set_tx_subdevice(char *txspec) 
{    
    std::cout << boost::format("Setting TX subdevice spec %s") % txspec << std::endl;
    uhd::usrp::subdev_spec_t s(txspec);
    usrp.sdev->set_tx_subdev_spec(s);
    std::cerr << boost::format("Current TX subdevice is: %s") % usrp.sdev->get_tx_subdev_name() << std::endl;    
}

void usrp_set_subdev_args(const char *subdevs)
{
    char largs[10];
    char* token;
        
    strcpy(largs, subdevs);    
    token=strtok(largs," \r\n");    
    if(token!=NULL) {
        usrp_set_rx_subdevice(token);
        token=strtok(NULL," \r\n");
            if(token!=NULL) {
                usrp_set_tx_subdevice(token);
                usrp.tx_enabled = true;
            } else {
                std::cerr << "Only RX spec found. TX not configured."<< std::endl;
                usrp.tx_enabled = false;
            }
    } else {
        std::cerr << "Missing any USRP subdev specs. Exiting."<< std::endl;
        exit(1);        
    }        
}

void usrp_set_swap_iq(int swp)
{
	if (swp == 1) {
		real_position = 0;
		imag_position = 1;
	} else {
		real_position = 1;
		imag_position = 0;		
	}
}


void usrp_set_receivers(int n)
{
    usrp.receivers = n;
}

int usrp_get_receivers(void)
{
    return usrp.receivers;
}

bool  usrp_is_tx_enabled(void) 
{
    return usrp.tx_enabled;
}

void usrp_set_client_rx_rate (int rate)
{
    switch (rate) {
/*
    IZ0CEZ:This is the target RX sample rates to the client.
           To be used for the rational resampler.
*/
    case  48000: //256K / 16 * 3
        DECIM = 16;        
        break;
    case  96000: //256K / 8 * 3
        DECIM = 8;
        break;
    case 192000: //256K / 4 * 3
        DECIM = 4;
        break;
    default:
        std::cerr << boost::format("Invalid client I/Q RX Rate: %d sps. Exiting") % (rate) << std::endl;
        exit(1);
    }
    std::cout << boost::format("Client I/Q RX Rate: %f sps.") % (rate) << std::endl << std::endl;        

}

int usrp_get_client_rx_rate(void)
{
    return (USRP_RX_SAMPLERATE / DECIM) * INTERP;
}

void usrp_set_frequency (double freq)
{
    //set the rx center frequency
    std::cout << boost::format("Setting USRP RX/TX Freq: %f Mhz...") % (freq/1e6) << std::endl;
    usrp.sdev->set_rx_freq(freq);
    std::cout << boost::format("Actual USRP RX Freq: %f Mhz...") % (usrp.sdev->get_rx_freq()/1e6) << std::endl << std::endl;
    if (usrp.tx_enabled) {
        usrp.sdev->set_tx_freq(freq);
        std::cout << boost::format("Actual USRP RX Freq: %f Mhz...") % (usrp.sdev->get_rx_freq()/1e6) << std::endl << std::endl;        
    }
}


//This is the main sample receiving thread (private)
void *usrp_receiver_thread (void *param)
{
    RECEIVER *pRec = (RECEIVER *)param;    
    //unsigned int total_num_samps = 2E9;
        
    uhd::rx_metadata_t md;
	
    //debug
	//std::cerr << inspect_receiver(pRec) << std::endl;
    
    const int MAX_USRP_RX_BUFFER = usrp.dev->get_max_recv_samps_per_packet();
    std::vector<std::complex<float> > buff(MAX_USRP_RX_BUFFER);
    
    float rresamp_buf_in[MAX_USRP_RX_BUFFER*2];
    int rresamp_buf_out_size = 2*((MAX_USRP_RX_BUFFER*INTERP)/DECIM +1);
    float rresamp_buf_out[rresamp_buf_out_size*2];
    
    SRC_DATA *rresamp_data =(SRC_DATA *)malloc(sizeof(SRC_DATA));
    rresamp_data->data_in = rresamp_buf_in;
    rresamp_data->data_out = rresamp_buf_out;
    rresamp_data->src_ratio = INTERP*1.0/DECIM;
    pRec->samples = 0;
    
    //Setup the sample streaming
    uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);    
    stream_cmd.num_samps = 0;
    stream_cmd.stream_now = true;
    usrp.sdev->issue_stream_cmd(stream_cmd);
    
    std::cerr << "Starting the USRP receiver thread cycle" << std::endl;
    
    while (1){		
        size_t num_rx_samps = usrp.dev->recv(
                                            &buff.front(), buff.size(), md,
                                            uhd::io_type_t::COMPLEX_FLOAT32,
                                            uhd::device::RECV_MODE_ONE_PACKET
                                           );
                                           
        //fprintf (stderr, "%s: received from USRP %d samples\n", __FUNCTION__, num_rx_samps);
        //handle the error codes
        switch(md.error_code){
        case uhd::rx_metadata_t::ERROR_CODE_NONE:
            break;

        case uhd::rx_metadata_t::ERROR_CODE_TIMEOUT:            
            std::cout << boost::format(
                "Got timeout before all samples received, possible packet loss, exiting loop..."
            ) << std::endl;
            goto done_loop;
            
        case uhd::rx_metadata_t::ERROR_CODE_OVERFLOW:
            std::cout << boost::format(
                "Internal USRP receiver buffer overflow, exiting loop..."
            ) << std::endl;
            goto done_loop;

        default:
            std::cout << boost::format(
                "Got error code 0x%x, exiting loop..."
            ) % md.error_code << std::endl;
            goto done_loop;
        }

        //Setup of the rational resampler
        rresamp_data->input_frames = num_rx_samps;
        rresamp_data->output_frames = (num_rx_samps*INTERP) / DECIM;                
        //fprintf (stderr, "Ratio is %f", rresamp_data->src_ratio);
        
        //Here IQ samples are buffered: 
        //Rational resampling is applied at this point
        //1. Interleaving real/imag in the resampler buffer
        unsigned int i;
    #pragma omp parallel for schedule(static) private(i)
        for (i=0; i<num_rx_samps; ++i) {                    
            
            rresamp_data->data_in[2*i+imag_position] = buff[i].imag();
            rresamp_data->data_in[2*i+real_position] = buff[i].real();            
        }
        
        //2.Rational resampling API here
        int rr_retcode = src_simple(rresamp_data, SRC_SINC_FASTEST, 2);
        
        //fprintf (stderr, "Simple resampling done. Obtained %ld samples.\n", rresamp_data->output_frames_gen);        
        if (rr_retcode == RRESAMPLER_NO_ERROR) {
            
            for (int j=0; j<rresamp_data->output_frames_gen ; ++j) {                    
                    
                pRec->input_buffer[pRec->samples]             = rresamp_data->data_out[2*j];
                pRec->input_buffer[pRec->samples+BUFFER_SIZE] = rresamp_data->data_out[2*j+1];

                pRec->samples++;

                if(pRec->samples==BUFFER_SIZE) {
                    // send I/Q data to clients
                    //fprintf (stderr, "%s: sending data.\n", __FUNCTION__);
                    send_IQ_buffer(pRec);
                    pRec->samples=0;
                }
            }
            
        } else {
            //A rational resampler error occurred!            
            fprintf (stderr, "Rational resampling error: %s\n", src_strerror (rr_retcode));
        }
        
    } done_loop:
	std::cerr << "Exiting USRP receiver thread" << std::endl;

    return 0;
}


//This is the transmitting thread (private)
void *usrp_transmitter_thread (void *param)
{    
    uhd::tx_metadata_t md;
    uhd::async_metadata_t async_md;
    float *audio_data;
    BUFFER_ENTRY *item;
    
    const unsigned int MAX_USRP_TX_BUFFER = usrp.dev->get_max_send_samps_per_packet();
    std::vector<std::complex<float> > buff(MAX_USRP_TX_BUFFER);
    md.start_of_burst = false;
    md.end_of_burst = false;                
    
    //CRITICAL SECTION on the TX QUEUE
    while(1) {
        //If there is buffer pair in the queue
        sem_wait(&tx_audio_semaphore);
        item = TAILQ_FIRST(&tx_audio_iq_stream);        
        if (item != NULL) {            
            //Take next buffer pair from the queue
            audio_data = (float *)item->data;
            TAILQ_REMOVE(&tx_audio_iq_stream, item, entries);
            queue_length--;
                                                           
        } else {        
            //else get container with all zeros ???
        }
        sem_post(&tx_audio_semaphore);
        //cleanup container
        free(item->data);
        free(item);
                        
        //Loop: send 1 I/Q Buffer of audio data to USRP
        int acc_tx_samples = 0;        
        do {
            size_t num_tx_samples = 0;
            //Loop 1 packet, or less, sent to USRP
            unsigned int i;
        #pragma omp parallel for schedule(static) private(i)
            for (i=0; (i<MAX_USRP_TX_BUFFER) && (acc_tx_samples<TRANSMIT_BUFFER_SIZE); ++i) {                                    
                buff[i].imag() = audio_data[i];
                buff[i].real() = audio_data[TRANSMIT_BUFFER_SIZE+i];
                num_tx_samples++; acc_tx_samples++;                
            }
            
            usrp.dev->send(&buff.front(), num_tx_samples, md,
                uhd::io_type_t::COMPLEX_FLOAT32,
                uhd::device::SEND_MODE_ONE_PACKET
                );

            //Check possible messages
            if (usrp.dev->recv_async_msg(async_md)) {
                
                switch(async_md.event_code){
                case uhd::async_metadata_t::EVENT_CODE_BURST_ACK:
                    break;

                case uhd::async_metadata_t::EVENT_CODE_UNDERFLOW_IN_PACKET:
                    std::cout << boost::format(
                        "Got underflow indication after sending packet. Exiting transmitter thread..."
                    ) << std::endl;
                    goto done_tx_loop;
                    
                case uhd::async_metadata_t::EVENT_CODE_TIME_ERROR:
                    std::cout << boost::format(
                        "Wrong timing in sending packet. Exiting transmitter thread..."
                    ) << std::endl;
                    goto done_tx_loop;
                    
                case uhd::async_metadata_t::EVENT_CODE_SEQ_ERROR:
                    std::cout << boost::format(
                        "Probable packet loss indication after sending packet."
                    ) << std::endl;
                    break;

                default:
                    std::cout << boost::format(
                        "Got error code 0x%x. Exiting transmitter thread..."
                        ) % async_md.event_code << std::endl;
                    goto done_tx_loop;
                }
            }            
            
        } while (acc_tx_samples < TRANSMIT_BUFFER_SIZE);        
        
    } done_tx_loop:
    
    return 0;
}

bool usrp_start (RECEIVER *pRec)
{
    //Creates the IQ samples receiving thread
    if (pthread_create(&pRec->client->thread_id,NULL,usrp_receiver_thread,(void *)pRec)!=0) {
        fprintf(stderr,"Failed to create USRP receiver thread for rx %d\n",pRec->client->receiver);
        return false; 
    } else if (usrp.tx_enabled) {
        //Creates the IQ samples transmitting thread
        if (pthread_create(&pRec->client->thread_id,NULL,usrp_transmitter_thread,(void *)NULL)!=0) {
            fprintf(stderr,"Failed to create USRP transmitter thread for rx %d\n",pRec->client->receiver);
            return false; 
        } 
    }      
    return true;  
}


/* 
 * REMEMBER: the outbuf carries 2 channels: 
 * outbuf[0..TRANSMIT_BUFFER_SIZE-1] and
 * outbuf[TRANSMIT_BUFFER_SIZE..TRANSMIT_BUFFER_SIZE-1]
 */
//TODO Transmitting audio to modulation.
//CRITICAL SECTION on the TX QUEUE 
int usrp_process_tx_modulation(float *outbuf, int mox) {

    unsigned char *audio_data;
    BUFFER_ENTRY *item;
    
    //If there is space in queue
    if (queue_length < MAX_QUEUE) {    
        //Add container to queue
        audio_data = (unsigned char *) malloc(TRANSMIT_BUFFER_SIZE*2*4);
        memcpy(audio_data, &outbuf, TRANSMIT_BUFFER_SIZE*2*4);
        item = (BUFFER_ENTRY *) malloc(sizeof(*item));
        item->data = audio_data;    
        sem_wait(&tx_audio_semaphore);
        TAILQ_INSERT_TAIL(&tx_audio_iq_stream, item, entries);
        queue_length++;
        sem_post(&tx_audio_semaphore);
        return 0;    
    } else {
        //else discard buffer and return -1    
        return -1;
    }
}


