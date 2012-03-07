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

// TX audio sample is the HEAD of a queue to be forwarded to USRP
TAILQ_HEAD(tailhead, _buffer_entry) tx_audio_iq_stream;
struct tailhead *headp; /* Tail queue head. */
#define MAX_QUEUE 50 //Max 50KSAMPLES in queue
#define QUEUE_WAIT_TIME 10000  //Time to wait after a empty queue (uSec)
#define USRP_DROP_WAIT_TIME 2000  //Time to wait after dropping rx samples (uSec)
//static sem_t tx_audio_semaphore;
static pthread_mutex_t queue_lock;

static int queue_length = 0;

//Defines and initialises the tx buffer queue
void setup_tx_queue(void) {
    
    TAILQ_INIT(&tx_audio_iq_stream); 

    pthread_mutex_init(&queue_lock, NULL );
    signal(SIGPIPE, SIG_IGN);
    
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
        //std::cerr << boost::format("USRP Subdev specs: %s\n") % subdev_specs << std::endl;
        usrp_set_subdev_args(subdev_specs);
        
        //set the USRP rx standard sample rate 256000 for minimum bandwidth.
        usrp.sdev->set_rx_rate(USRP_RX_SAMPLERATE);
        //set the USRP tx standard sample rate : WHERE the interpolation is set???
        usrp.sdev->set_tx_rate(USRP_TX_SAMPLERATE);
        std::cout << boost::format("USRP RX Rate: %f Msps...") % (usrp.sdev->get_rx_rate()/1e6) << std::endl;        
        std::cout << boost::format("USRP TX Rate: %f Msps...") % (usrp.sdev->get_tx_rate()/1e6) << std::endl; 

        usrp_set_frequency(DEFAULT_RX_FREQUENCY); //TEMPORARY      
        sleep(1); //allow for some setup time
        
        setup_tx_queue();
        
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
    std::cout << boost::format("Client I/Q RX Rate: %f sps.") % (rate) << std::endl << std::endl;        

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
     
    //NEW RES LIB    
    int resampler_id = resampler_setup_new(MAX_USRP_RX_BUFFER, DECIM_RX, INTERP_RX);    
    
    if (resampler_id < 0) { //FAILURE!
        std::cerr << "RX Resampler object definition failed. Exiting..." << std::endl;
        exit (1);
    }
    std::cerr << boost::format("RX thread Using resampler %d.") % resampler_id << std::endl;
     
    pRec->samples = 0;
    
    //Setup the samples streaming
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
        //std::cerr << boost::format("%s: received from USRP %d samples\n") % __FUNCTION__, num_rx_samps << std::endl;
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
            std::cerr << boost::format(
                "Internal USRP receiver buffer overflow, exiting loop..."
            ) << std::endl;
            goto done_loop;             

        default:
            std::cerr << boost::format(
                "Got error code 0x%x, exiting loop..."
            ) % md.error_code << std::endl;
            goto done_loop;
        }

        if (drop_rx_samples) {
            usleep(USRP_DROP_WAIT_TIME);
            continue;
        }   
         
        //Here IQ samples are buffered: 
        //Rational resampling is applied at this point
        //Interleaving real/imag in the resampler buffer        
        if (real_position == 1) {
            unsigned int i;
        //#pragma omp parallel for schedule(static) private(i)
            for (i=0; i<num_rx_samps; ++i)
                resampler_load_data(resampler_id, i, buff[i].imag(), buff[i].real());                        
        } else {
            unsigned int i;
        //#pragma omp parallel for schedule(static) private(i)
            for (i=0; i<num_rx_samps; ++i)
                resampler_load_data(resampler_id, i, buff[i].real(), buff[i].imag());                                    
        }
                 
        //Rational resampling API here                
        char *rr_msg = NULL;
        int output_frames_gen = 0;
        int rr_retcode = do_resample(resampler_id, num_rx_samps, &output_frames_gen, rr_msg);         
                
        //std::cerr << boost::format("output_frames_gen %d.") % output_frames_gen << std::endl; 
        
        if (rr_retcode == RRESAMPLER_NO_ERROR) { 
            //Fetch resampled data into the Receiver input buffer
            for (int j=0; j<output_frames_gen ; ++j) {                                                                           

                resampler_fetch_data(resampler_id, j, &pRec->input_buffer[pRec->samples], 
                                       &pRec->input_buffer[pRec->samples+BUFFER_SIZE]);
                 
                pRec->samples++;

                if(pRec->samples==BUFFER_SIZE) {
                    // send I/Q data to client
                    //std::cerr << boost::format("%s: sending data.") % __FUNCTION__ << std::endl;
                    send_IQ_buffer(pRec);
                    pRec->samples=0;
                }
            }
            
        } else {
            //A rational resampler error occurred!            
            std::cerr << boost::format("RX Rational resampling error: %s") % rr_msg << std::endl;
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
    int audio_data_cnt = 0;
    BUFFER_ENTRY *item;
    
    const unsigned int MAX_USRP_TX_BUFFER = usrp.dev->get_max_send_samps_per_packet();
    std::cerr << boost::format("Max packet size: %d") % MAX_USRP_TX_BUFFER << std::endl;
    std::vector<std::complex<float> > buff(MAX_USRP_TX_BUFFER);
    md.start_of_burst = false;
    md.end_of_burst = false; 

    //NEW RES LIB    
    int resampler_id = resampler_setup_new(TRANSMIT_BUFFER_SIZE, DECIM_TX, INTERP_TX);    
    
    if (resampler_id < 0) { //FAILURE!
        std::cerr << "TX Resampler object definition failed. Exiting..." << std::endl;
        exit (1);
    }
    std::cerr << boost::format("TX thread Using resampler %d.") % resampler_id << std::endl;    
 
    std::cerr << "Starting the USRP transmitter thread cycle" << std::endl;              
    
    //CRITICAL SECTION on the TX QUEUE    
    while(1) {        
        //If there is buffer pair in the queue
        pthread_mutex_lock(&queue_lock);            
        item = TAILQ_FIRST(&tx_audio_iq_stream);                   
        if (item != NULL) {            
            //Take next buffer from the queue
            //it is a paired vectors [III...I][QQQ...Q] or vice versa
            audio_data = (float *)item->data;
            
            TAILQ_REMOVE(&tx_audio_iq_stream, item, entries);                    
            queue_length--;
            pthread_mutex_unlock(&queue_lock);
        } else {
            pthread_mutex_unlock(&queue_lock);
            usleep(QUEUE_WAIT_TIME);
            continue;
        }        
        
        //std::cerr << boost::format("Remove from queue: len %d") % queue_length << std::endl;        
                
        //DEBUG TEST
        //std::cerr << boost::format("Dumping audio data buffer #%d") % ++audio_data_cnt << std::endl;              
        //dump_float_buffer_heads(audio_data);
        //continue;
        //DEBUG TEST
        
        //Rational resampler to reach 320KSPS
        //Load resampler with audio received by the dspserver
        if (real_position == 1) 
            resampler_load_channels(resampler_id, audio_data, &audio_data[TRANSMIT_BUFFER_SIZE]);                        
        else
            resampler_load_channels(resampler_id, &audio_data[TRANSMIT_BUFFER_SIZE], audio_data);                        
        
        //CLEANUP: audio_data
        free(item->data);
        free(item);        
        
        //Rational resampling API here                
        char *rr_msg = NULL;
        int output_frames_gen = 0;
        int rr_retcode = do_resample(resampler_id, TRANSMIT_BUFFER_SIZE, &output_frames_gen, rr_msg);                         
        //std::cerr << boost::format("output_frames_gen %d.\n") % output_frames_gen << std::endl;  
                        
        if (rr_retcode == RRESAMPLER_NO_ERROR) {
            
            int acc_tx_samples = 0;
            int pcktcnt = 0;
            //Loop: send 1 I/Q packet of audio data to USRP
            do {
                size_t num_tx_samples = 0;
                float im, rl;
                //Loop 1 packet, or less, sent to USRP
                unsigned int i;
            #pragma omp parallel for schedule(static) private(i)
                for (i=0; (i<MAX_USRP_TX_BUFFER) && (acc_tx_samples<output_frames_gen); ++i) {

                    resampler_fetch_data(resampler_id, i, &im, &rl);
                    buff[i] = std::complex<float>(im,rl);                                                     
                    num_tx_samples++; acc_tx_samples++;                
                }                                                
                //DEBUG TEST
                /*
                fprintf(stderr, "  Re: %+7.4f %+7.4f %+7.4f %+7.4f %+7.4f %+7.4f %+7.4f %+7.4f\n",
                buff[i].real(),buff[i+1].real(),buff[i+2].real(),buff[i+3].real(),
                buff[i+4].real(),buff[i+5].real(),buff[i+6].real(),buff[i+7].real());
                fprintf(stderr, "  Im: %+7.4f %+7.4f %+7.4f %+7.4f %+7.4f %+7.4f %+7.4f %+7.4f\n",
                buff[i].imag(),buff[i+1].imag(),buff[i+2].imag(),buff[i+3].imag(),
                buff[i+4].imag(),buff[i+5].imag(),buff[i+6].imag(),buff[i+7].imag());
                fprintf(stderr,"\n");                
                 */
                //DEBUG TEST
                                                    
                usrp.dev->send(&buff.front(), num_tx_samples, md,
                    uhd::io_type_t::COMPLEX_FLOAT32,
                    uhd::device::SEND_MODE_ONE_PACKET
                    );                
                     
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
                //std::cerr << boost::format("packet sent: %d") % ++pcktcnt;
            } while (acc_tx_samples < output_frames_gen);        
            
            //std::cerr << "Audio Buffer sent" << std::endl;
            
        } else {
            //A rational resampler error occurred!            
            std::cerr << boost::format("TX Rational resampling error: %s") % rr_msg << std::endl;
        }
        
    } done_tx_loop:
    
    return 0;
}

bool usrp_start (RECEIVER *pRec)
{
    //Creates the IQ samples receiving thread
    if (pthread_create(&pRec->rx_thread_id,NULL,usrp_receiver_thread,(void *)pRec)!=0) {
        std::cerr << boost::format(
            "Failed to create USRP receiver thread for rx %d") %pRec->client->receiver_num << std::endl;
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
        audio_data = (unsigned char *) malloc(TRANSMIT_BUFFER_SIZE*2*sizeof(float));
        memcpy(audio_data, outbuf, TRANSMIT_BUFFER_SIZE*2*sizeof(float));
        item = (BUFFER_ENTRY *) malloc(sizeof(*item));
        item->data = audio_data;    
        //std::cerr << boost::format("Add to queue: len %d") % queue_length << std::endl;        
        pthread_mutex_lock(&queue_lock);
        TAILQ_INSERT_TAIL(&tx_audio_iq_stream, item, entries);
        queue_length++;
        pthread_mutex_unlock(&queue_lock);    
        return 0;    
    } else {
        //else discard buffer and return -1    
        return -1;
    }
}

