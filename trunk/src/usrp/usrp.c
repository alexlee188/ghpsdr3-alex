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
#include <sys/time.h>
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

#include <signal.h>
#include <string.h>
#include <samplerate.h>

#include <uhd/usrp/multi_usrp.hpp>

#include "client.h"
#include "receiver.h"
#include "transmitter.h"
#include "resampler.h"
#include "usrp.h"
#include "util.h"

//RX output sample rate from USRP
#define USRP_RX_SAMPLERATE 256000
#define USRP_TX_SAMPLERATE 320000 
#define DEFAULT_RX_FREQUENCY 7056000
#define RRESAMPLER_NO_ERROR 0  //Same as resampler

class USRP {
public:
   uhd::usrp::multi_usrp::sptr sdev;
   uhd::device::sptr            dev;
   int receivers;
   bool tx_enabled;
};

static USRP usrp;
static int DECIM_RX = 16;
static int INTERP_RX = 3;  //For the rational resampler
static int DECIM_TX = 3;
static int INTERP_TX = 20;  //For the rational resampler
// for the swap option
static int real_position = 1;
static int imag_position = 0;
//to disable RX samples
static bool drop_rx_samples = false;
static const int TRANSMIT_BUFFER_SIZE_BYTES = TRANSMIT_BUFFER_SIZE*2*sizeof(float);
static int MAX_USRP_RX_BUFFER = 0; //will be initialized
static int MAX_USRP_TX_BUFFER = 0; //will be initialized
static bool tx_resampler_init = true;
static bool usrp_started = false;
static int tx_resampler_id = -1;
static bool RECEIVER_CYCLE = false;
static pthread_mutex_t RECEIVER_CYCLE_lock;

//The Complex zero vector
static const std::complex<float> CPX_ZERO (0.0, 0.0);
static std::vector<std::complex<float> > ZERO_CPX_VECTOR;

/*!
 * The queue item
 */
typedef struct _queue_buffer_entry {
	std::vector<std::complex<float> > *data;
    int data_length;
	TAILQ_ENTRY(_queue_buffer_entry) entries;
} QUEUE_BUFFER_ENTRY;


// TX audio sample is the HEAD of a queue to be forwarded to USRP
TAILQ_HEAD(tx_tailhead, _queue_buffer_entry) tx_audio_iq_stream;
struct tx_tailhead *tx_headp; /* Tail queue head. */

// RX USRP sample is the HEAD of a queue to be processed by server
TAILQ_HEAD(rx_tailhead, _queue_buffer_entry) rx_samples_iq_stream;
struct rx_tailhead *rx_headp; /* Tail queue head. */

#define MAX_QUEUE 50 //Max 50KSAMPLES in queue
#define QUEUE_WAIT_TIME 10000  //Time to wait after a empty queue (uSec)
#define USRP_DROP_WAIT_TIME 2000  //Time to wait after dropping rx samples (uSec)

static pthread_mutex_t tx_queue_lock;
static pthread_mutex_t rx_queue_lock;

static int tx_queue_length = 0;
static int rx_queue_length = 0;

//Defines and initialises the tx buffer queue
void setup_tx_queue(void) {
    
    TAILQ_INIT(&tx_audio_iq_stream); 

    pthread_mutex_init(&tx_queue_lock, NULL );        
}

//Defines and initialises the tx buffer queue
void setup_rx_queue(void) {
    
    TAILQ_INIT(&rx_samples_iq_stream); 

    pthread_mutex_init(&rx_queue_lock, NULL );        
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
        
        usrp.tx_enabled = false;        

        // USRP create
        usrp.sdev = uhd::usrp::multi_usrp::make(device_addrs[i]);
        //Lock mboard clocks
        usrp.sdev->set_clock_config(uhd::clock_config_t::internal(), 0);        
        
        //std::cerr << boost::format("USRP Subdev specs: %s\n") % subdev_specs << std::endl;
        usrp_set_subdev_args(rx_subdev_par, tx_subdev_par);            
        
        //set the USRP rx standard sample rate 256000 for minimum bandwidth.
        usrp.sdev->set_rx_rate(USRP_RX_SAMPLERATE);
        //std::cout << boost::format("USRP RX Rate: %f Msps") % (usrp.sdev->get_rx_rate()/1e6) << std::endl; 
        //set the USRP tx standard sample rate 320000 - USRP interpolation is automatic
        usrp.sdev->set_tx_rate(USRP_TX_SAMPLERATE);               
        //std::cout << boost::format("USRP TX Rate: %f Msps") % (usrp.sdev->get_tx_rate()/1e6) << std::endl; 

        usrp_set_frequency(DEFAULT_RX_FREQUENCY); //TEMPORARY      
        sleep(1); //allow for some setup time
        
        usrp.dev = usrp.sdev->get_device();
        MAX_USRP_RX_BUFFER = usrp.dev->get_max_recv_samps_per_packet();
        MAX_USRP_TX_BUFFER = usrp.dev->get_max_send_samps_per_packet();
        
        ZERO_CPX_VECTOR = std::vector<std::complex<float> >(MAX_USRP_RX_BUFFER, CPX_ZERO);
                
        setup_rx_queue();
        setup_tx_queue();
        
        pthread_mutex_init(&RECEIVER_CYCLE_lock, NULL );
        
        resampler_init();

        return true;
    }
    return false;
}


void usrp_deinit (void)
{
  // anything here???
}

//Configures the RX subdevice
void usrp_set_rx_subdevice(const char *rxspec) 
{
    std::cout << boost::format("Setting RX subdevice spec %s") % rxspec << std::endl;
    uhd::usrp::subdev_spec_t s(rxspec);
    usrp.sdev->set_rx_subdev_spec(s);
    std::cerr << boost::format("Current RX subdevice is: %s") % usrp.sdev->get_rx_subdev_name() << std::endl;
}

//Configures the TX subdevice
void usrp_set_tx_subdevice(const char *txspec) 
{    
    std::cout << boost::format("Setting TX subdevice spec %s") % txspec << std::endl;
    uhd::usrp::subdev_spec_t s(txspec);
    usrp.sdev->set_tx_subdev_spec(s);
    std::cerr << boost::format("Current TX subdevice is: %s") % usrp.sdev->get_tx_subdev_name() << std::endl;    
}

void usrp_set_subdev_args(const char *subdev_rx, const char *subdev_tx)
{
    usrp_set_rx_subdevice(subdev_rx);
    if ((subdev_tx == NULL) || (strlen(subdev_tx)==0)) {
        std::cerr << "Only RX spec found. TX not configured."<< std::endl;
        usrp.tx_enabled = false;
    } else {
        usrp_set_tx_subdevice(subdev_tx);
        usrp.tx_enabled = true;
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

void usrp_disable_rx_path(void) 
{
    drop_rx_samples = true;
}

void usrp_set_client_rx_rate (int rate)
{
    switch (rate) {

    //IZ0CEZ:These are the target RX sample rates to the client.
    //       To be used for the rational resampler.

    case  48000: 
        DECIM_RX = 16; //256K / 16 * 3
        INTERP_TX = 20; //48K / 3 * 20 = 320K 
        break;
    case  96000: //256K / 8 * 3
        DECIM_RX = 8;
        INTERP_TX = 10; //96K / 3 * 10 = 320K
        break;
    case 192000: //256K / 4 * 3
        DECIM_RX = 4;
        INTERP_TX = 5; //192K / 3 * 5 = 320K
        break;
    default:
        std::cerr << boost::format("Invalid client I/Q RX Rate: %d sps. Exiting") % (rate) << std::endl;
        exit(1);
    }
    std::cout << boost::format("Client I/Q RX Rate: %f sps.") % (rate) << std::endl;     

}

int usrp_get_client_rx_rate(void)
{
    return (USRP_RX_SAMPLERATE / DECIM_RX) * INTERP_RX;
}

void usrp_set_frequency (double freq)
{
    //set the rx center frequency
    std::cout << boost::format("Setting USRP RX/TX Freq: %f Mhz...") % (freq/1e6) << std::endl;
    usrp.sdev->set_rx_freq(freq);
    std::cout << boost::format("Actual USRP RX Freq: %f Mhz...") % (usrp.sdev->get_rx_freq()/1e6) << std::endl;
    if (usrp.tx_enabled) {
        usrp.sdev->set_tx_freq(freq);
        std::cout << boost::format("Actual USRP TX Freq: %f Mhz...") % (usrp.sdev->get_rx_freq()/1e6) << std::endl;
    }
}

//This is the main sample receiving thread (private)
void *usrp_receiver_thread (void *param)
{            
    uhd::rx_metadata_t md;	    
    std::vector<std::complex<float> > buff(MAX_USRP_RX_BUFFER);       
    int old_state, old_type;
    QUEUE_BUFFER_ENTRY *item;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&old_state);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&old_type);       
    
    //Setup the samples streaming
    uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);    
    stream_cmd.num_samps = 0;
    stream_cmd.stream_now = true;
    usrp.sdev->issue_stream_cmd(stream_cmd);
    
    std::cerr << "Starting the USRP receiver thread cycle" << std::endl;
        
    //TIME BENCHMARKING 
    struct timeval *tod, *tod1;
    tod = (timeval *) malloc(sizeof(*tod));
    tod1 = (timeval *) malloc(sizeof(*tod1));
    int elapsed_usec = 0, diff_usec = 0; 
    long int recv_samps = 0;    
    gettimeofday(tod, NULL);
    
    while (1){	
        
        size_t num_rx_samps = usrp.dev->recv(
                                            &buff.front(), buff.size(), md,
                                            uhd::io_type_t::COMPLEX_FLOAT32,
                                            uhd::device::RECV_MODE_ONE_PACKET
                                           );                                            
        
        recv_samps += num_rx_samps;
        //std::cerr << boost::format("%s: received from USRP %d samples\n") % __FUNCTION__, num_rx_samps << std::endl;
                
        //TIME BECHMARKING
        gettimeofday(tod1, NULL);
        //std::cerr << boost::format("After recv: %d (SAMPS: %d)|\n") % (tod1->tv_usec - tod->tv_usec) % num_rx_samps;
        
        //handle the error codes
        switch(md.error_code){
        case uhd::rx_metadata_t::ERROR_CODE_NONE:
             break;

        case uhd::rx_metadata_t::ERROR_CODE_TIMEOUT:            
            std::cerr << boost::format(
                "Got timeout before all samples received, possible packet loss, exiting loop..."
            ) << std::endl;
            goto done_loop;
                        
        case uhd::rx_metadata_t::ERROR_CODE_OVERFLOW:              
            /*
             * Let the progranm continue.
             * In current version, overflows will be detected by "OOO's" at stderr
             * More elegant management will come in official version.
             * 
            std::cerr << boost::format(
                "Internal USRP receiver buffer overflow, exiting loop..."
            ) << std::endl;  
            goto done_loop;
             */
            break; 

        default:
            std::cerr << boost::format(
                "Got error code 0x%x, exiting loop..."
            ) % md.error_code << std::endl;
            goto done_loop;
        }
        
        //ENQUEUE The buffer
        
        pthread_mutex_lock(&RECEIVER_CYCLE_lock); 
        bool do_add_queue = RECEIVER_CYCLE;
        pthread_mutex_unlock(&RECEIVER_CYCLE_lock);
        
        if (! do_add_queue) continue;
        
        //Copy of the buffer in a NEW allocated buffer
        std::vector<std::complex<float> > *buff2 = new std::vector<std::complex<float> >(buff);

        //If there is space in queue
        if (rx_queue_length < MAX_QUEUE) {    
            //Add container to queue
            item = (QUEUE_BUFFER_ENTRY *) malloc(sizeof(*item));
            item->data = buff2;    
            //std::cerr << boost::format("Add to queue: len %d") % rx_queue_length << std::endl;        
            pthread_mutex_lock(&rx_queue_lock);
            TAILQ_INSERT_TAIL(&rx_samples_iq_stream, item, entries);
            rx_queue_length++;
            pthread_mutex_unlock(&rx_queue_lock);    
        } else {
            //else discard the rx buffer...
            std::cerr << boost::format("RX queue is full: rx buffer not forwarded!") << std::endl;
        }
                        
        //TIME BENCHMARKING 
        /*
        long int prev_usec=tod->tv_usec;
        gettimeofday(tod, NULL);
        elapsed_usec += ((diff_usec = tod->tv_usec - prev_usec) > 0 ? diff_usec : diff_usec + 1000000);        
        std::cerr << boost::format("Actual receiving rate (MSPS): %f") % (recv_samps * 1.0 / elapsed_usec) << std::endl;         
         */
        
    } done_loop:
	std::cerr << "Exiting USRP receiver thread" << std::endl;

    return 0;
}

void usrp_stop_rx_forwarder(void) {

    //Stops the RX forwarder thread
    pthread_mutex_lock(&RECEIVER_CYCLE_lock); 
    RECEIVER_CYCLE = false;
    pthread_mutex_unlock(&RECEIVER_CYCLE_lock); 
}

//This is the rx samples forwarding thread (private)
void *usrp_rx_forwarder_thread (void *param)
{
    RECEIVER *pRec = (RECEIVER *)param; 
    int old_state, old_type;

    //debug
	//std::cerr << inspect_receiver(pRec) << std::endl;
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&old_state);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&old_type);  
    RECEIVER_CYCLE = true;

    pRec->samples = 0;
    
    std::vector<std::complex<float> > *buff;
    QUEUE_BUFFER_ENTRY *item;
    
    //NEW RES LIB    
    int resampler_id = resampler_setup_new(MAX_USRP_RX_BUFFER, DECIM_RX, INTERP_RX);    
    
    if (resampler_id < 0) { //FAILURE!
        std::cerr << "RX Resampler object definition failed. Exiting..." << std::endl;
        exit (1);
    }
    std::cerr << boost::format("RX thread Using resampler %d.") % resampler_id << std::endl;
    
    
    std::cerr << "Starting the RX Data forwarding thread cycle" << std::endl;              
    
    while(1) {
    
        pthread_mutex_lock(&RECEIVER_CYCLE_lock); 
        bool do_cycle = RECEIVER_CYCLE;
        pthread_mutex_unlock(&RECEIVER_CYCLE_lock); 
        if (! do_cycle) break;
        
        //Extract from queue
        //If there is Vector buffer in the queue
        pthread_mutex_lock(&rx_queue_lock);            
        item = TAILQ_FIRST(&rx_samples_iq_stream);                   
        if (item != NULL) {            
            //Take next Vector buffer from the queue
            buff = (std::vector<std::complex<float> > *)item->data;              
            TAILQ_REMOVE(&rx_samples_iq_stream, item, entries);                    
            rx_queue_length--;
            pthread_mutex_unlock(&rx_queue_lock);
        } else {
            pthread_mutex_unlock(&rx_queue_lock);
            usleep(QUEUE_WAIT_TIME);
            continue;
        }  
        
        //RX path is disabled: skip the rest
        if (drop_rx_samples) {
            //Vector cleanup        
            delete buff;
            free(item);            
            continue;
        }           

        size_t num_rx_samps = buff->size();
        
        //Rational resampling is applied at this point
        //Interleaving real/imag in the resampler buffer        
        if (real_position == 1) {
            unsigned int i;            
        #pragma omp parallel for schedule(static) private(i)
            for (i=0; i<num_rx_samps; ++i)
                resampler_load_data(resampler_id, i, (*buff)[i].imag(), (*buff)[i].real());                        
        } else {
            unsigned int i;
        #pragma omp parallel for schedule(static) private(i)
            for (i=0; i<num_rx_samps; ++i)
                resampler_load_data(resampler_id, i, (*buff)[i].real(), (*buff)[i].imag());                                    
        }
        
        //Vector cleanup        
        delete buff;
        free(item);
                         
        //Rational resampling API here                
        char *rr_msg = NULL;
        int output_frames_gen = 0;
        int rr_retcode = do_resample(resampler_id, num_rx_samps, &output_frames_gen, rr_msg);         
                
        //std::cerr << boost::format("output_frames_gen %d.") % output_frames_gen << std::endl;                 
        
        //TIME BECHMARKING
        //gettimeofday(tod1, NULL);
        //std::cerr << boost::format("After resampler: %d |") % (tod1->tv_usec - tod->tv_usec);        
        
        if (rr_retcode == RRESAMPLER_NO_ERROR) { 
            //Fetch resampled data into the Receiver input buffer
            for (int j=0; j<output_frames_gen ; ++j) {                                                                           

                resampler_fetch_data(resampler_id, j, &pRec->input_buffer[pRec->samples], 
                                       &pRec->input_buffer[pRec->samples+RECEIVE_BUFFER_SIZE]);
                 
                pRec->samples++;

                if(pRec->samples==RECEIVE_BUFFER_SIZE) {                                        
                    
                    //send I/Q data to client
                    //std::cerr << boost::format("%s: sending data.") % __FUNCTION__ << std::endl;
                    send_IQ_buffer(pRec);
                    pRec->samples=0;                    
                }
            }
            
        } else {
            //A rational resampler error occurred!            
            std::cerr << boost::format("RX Rational resampling error: %s") % rr_msg << std::endl;
        }        
    }
    release_resampler(resampler_id);
    fprintf(stderr,"Exiting from USRP RX Forwarder thread\n");
    
    return NULL;

}

//Starts the USRP RX forwarder thread
int usrp_start_rx_forwarder_thread (CLIENT * client) {
    
    RECEIVER *pRec = &receiver[client->receiver_num];
    int rc =pthread_create(&pRec->rx_fwd_thread_id,NULL,usrp_rx_forwarder_thread,(void *)pRec);
    if (rc!=0)
        std::cerr << boost::format(
            "Failed to create USRP RX forwarder thread for rx %d") % pRec->client->receiver_num << std::endl;
    return rc;     
}

//This is the transmitting thread (private)
void *usrp_transmitter_thread (void *param)
{    
    uhd::tx_metadata_t md;
    uhd::async_metadata_t async_md;        
    int old_state, old_type;
    QUEUE_BUFFER_ENTRY *item;
    std::vector<std::complex<float> > *buff;
    int num_tx_samples = 0;
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&old_state);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&old_type);    
    
    md.start_of_burst = false;
    md.end_of_burst = false; 
 
    std::cerr << "Starting the USRP transmitter thread cycle" << std::endl;              
    
    //CRITICAL SECTION on the TX QUEUE    
    //TODO: benchmark to avoid underruns
    while(1) {        
        //If there is buffer pair in the queue
        pthread_mutex_lock(&tx_queue_lock);            
        item = TAILQ_FIRST(&tx_audio_iq_stream);                   
        if (item != NULL) {            
            //Take next buffer from the queue
            //it is standard Vector of complex float, I and Q
            buff = (std::vector<std::complex<float> > *) item->data;
            num_tx_samples = item->data_length;
            TAILQ_REMOVE(&tx_audio_iq_stream, item, entries);                    
            tx_queue_length--;
            pthread_mutex_unlock(&tx_queue_lock);
            //std::cerr << boost::format("Remove from queue: len %d") % tx_queue_length << std::endl;                                        
            usrp.dev->send(&buff->front(), num_tx_samples, md,
                uhd::io_type_t::COMPLEX_FLOAT32,
                uhd::device::SEND_MODE_ONE_PACKET
                );                                                              
            
            //CLEANUP: buff
            delete buff;
            free(item);                    
            
        } else {
            //If the queue is empty, then keep the USRP busy with all zeros
            //made to handle client disconnects
            pthread_mutex_unlock(&tx_queue_lock);
            //buff = new std::vector<std::complex<float> >(ZERO_CPX_VECTOR);
            //num_tx_samples = MAX_USRP_TX_BUFFER;
        }                
                                                                                                                 
        /*
         * NOT USED FOR NOW
         * 
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
        } */                   
    }  
    return 0;
}

//Starts the core USRP communication threads
bool usrp_start (CLIENT *client)
{
    RECEIVER *pRec = &receiver[client->receiver_num];
    if (pthread_create(&pRec->rx_thread_id,NULL,usrp_receiver_thread,NULL)!=0) {
        std::cerr << boost::format(
            "Failed to create USRP receiver thread for rx %d") % pRec->client->receiver_num << std::endl;
        return false; 
    } else {        
        std::cerr << "USRP receiver thread started." << std::endl;         
        if (usrp.tx_enabled) {           
            //Creates the IQ samples transmitting thread            
            if (pthread_create(&transmitter.thread_id,NULL,usrp_transmitter_thread,(void *)NULL)!=0) {
                std::cerr << boost::format(
                    "Failed to create USRP transmitter thread for rx %d") % pRec->client->receiver_num << std::endl;
                return false; 
            } else
                std::cerr << "USRP transmitter thread started." << std::endl;                                  
        }
        usrp_started = true;
        
    }
    return true;  
}

bool usrp_is_started(void) {
    return usrp_started;
} 

/* 
 * REMEMBER: the outbuf carries 2 channels: 
 * outbuf[0..TRANSMIT_BUFFER_SIZE-1] and
 * outbuf[TRANSMIT_BUFFER_SIZE..TRANSMIT_BUFFER_SIZE-1]
 */

//CRITICAL SECTION on the TX QUEUE 
int usrp_process_tx_modulation(float *outbuf, int mox) {
    
    float *audio_data;
    QUEUE_BUFFER_ENTRY *item;    
        
    //std::cerr << boost::format("Max packet size: %d") % MAX_USRP_TX_BUFFER << std::endl;    
    std::vector<std::complex<float> > buff(MAX_USRP_TX_BUFFER);    
    
    //At first call only, instantiate the rational resampler
    if (tx_resampler_init) {        
        //NEW RES LIB    
        tx_resampler_id = resampler_setup_new(TRANSMIT_BUFFER_SIZE, DECIM_TX, INTERP_TX);    
        
        if (tx_resampler_id < 0) { //FAILURE!
            std::cerr << "TX Resampler object definition failed. Exiting..." << std::endl;
            exit (1);
        }
        std::cerr << boost::format("TX thread Using resampler %d.") % tx_resampler_id << std::endl;                    
        tx_resampler_init = false;
    }
        
    audio_data = (float *) malloc(TRANSMIT_BUFFER_SIZE_BYTES);
    if (mox==1)
        memcpy(audio_data, outbuf, TRANSMIT_BUFFER_SIZE_BYTES);
    else
        memset(audio_data, 0, TRANSMIT_BUFFER_SIZE_BYTES);
        
    //Rational resampler to reach 320KSPS
    //Load resampler with audio received by the dspserver
    if (real_position == 1) 
        resampler_load_channels(tx_resampler_id, audio_data, &audio_data[TRANSMIT_BUFFER_SIZE]);                        
    else
        resampler_load_channels(tx_resampler_id, &audio_data[TRANSMIT_BUFFER_SIZE], audio_data);                        

    //Rational resampling API here                
    char *rr_msg = NULL;
    int output_frames_gen = 0;
    int rr_retcode = do_resample(tx_resampler_id, TRANSMIT_BUFFER_SIZE, &output_frames_gen, rr_msg);                         
    //std::cerr << boost::format("output_frames_gen %d.\n") % output_frames_gen << std::endl; 

    //Release audio data
    free(audio_data); 

    if (rr_retcode == RRESAMPLER_NO_ERROR) {
        
        int acc_tx_samples = 0;        
        //Loop: send 1 I/Q packet of audio data to USRP
        do {
            size_t num_tx_samples = 0;
            float im, rl;
            //Fill 1 packet, or less, to be sent IN QUEUE to USRP TX
            unsigned int i;
        //CANNOT USE pragma omp parallel for schedule(static) private(i)
            for (i=0; (i<MAX_USRP_TX_BUFFER) && (acc_tx_samples<output_frames_gen); ++i) {

                resampler_fetch_data(tx_resampler_id, i, &rl, &im);
                buff[i] = std::complex<float>(rl,im);                                                     
                num_tx_samples++; acc_tx_samples++;                
            } 

            //Copy of the buffer in a NEW allocated buffer
            std::vector<std::complex<float> > *buff2 = new std::vector<std::complex<float> >(buff);
                                            
            //If there is space in queue
            if (tx_queue_length < MAX_QUEUE) {    
            //Add container to queue
                item = (QUEUE_BUFFER_ENTRY *) malloc(sizeof(*item));
                item->data = buff2; 
                item->data_length = num_tx_samples;
                //std::cerr << boost::format("Add to queue: len %d") % tx_queue_length << std::endl;        
                pthread_mutex_lock(&tx_queue_lock);
                TAILQ_INSERT_TAIL(&tx_audio_iq_stream, item, entries);
                tx_queue_length++;
                pthread_mutex_unlock(&tx_queue_lock);                      
            } else {
                //else discard buffer    
                std::cerr << boost::format("TX queue is full: tx buffer not forwarded!") << std::endl;
            }            

        } while (acc_tx_samples < output_frames_gen);        
    
    } else {
        //A rational resampler error occurred!            
        std::cerr << boost::format("TX Rational resampling error: %s") % rr_msg << std::endl;
        return -1;
    }    
    return 0;
}

