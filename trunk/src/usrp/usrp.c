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

#include <string.h>
#include <boost/thread.hpp>
#include <samplerate.h>

#include <uhd/usrp/single_usrp.hpp>

#include "client.h"
#include "receiver.h"
#include "usrp.h"
//#include "usrp_audio.h" not for now

//RX output sample rate from USRP
#define USRP_RX_SAMPLERATE 256000
#define DEFAULT_RX_FREQUENCY 7056000
#define RRESAMPLER_NO_ERROR 0

class USRP {
public:
   uhd::usrp::single_usrp::sptr sdev;
   uhd::device::sptr            dev;
   int receivers;
};

static USRP usrp;
static int DECIM = 16;
static int INTERP = 3;  //For the rational resampler, maybe

bool usrp_init (const char *subdev_par)
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
        usrp.sdev = uhd::usrp::single_usrp::make(device_addrs[i]);
        usrp.dev = usrp.sdev->get_device();
        
        usrp_set_subdev_args(subdev_par);
        
        //set the USRP rx standard sample rate 256000 for minimum bandwidth.
        usrp.sdev->set_rx_rate(USRP_RX_SAMPLERATE);
        std::cout << boost::format("USRP RX Rate: %f Msps...") % (usrp.sdev->get_rx_rate()/1e6) << std::endl;        
        
        return true;
    }
    return false;
}

void usrp_deinit (void)
{

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
    //TODO: Implement TX subdevice parsing
}

void usrp_set_subdev_args(const char *sargs)
{
    char largs[10];
    char* token;
        
    strcpy(largs, sargs);    
    token=strtok(largs," \r\n");    
    if(token!=NULL) {
        usrp_set_rx_subdevice(token);
        token=strtok(NULL," \r\n");
            if(token!=NULL) {
                usrp_set_tx_subdevice(token);
            }
    } else {
        std::cerr << "Missing any USRP RX arg. Exiting."<< std::endl;
        exit(1);        
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
    std::cout << boost::format("Setting RX Freq: %f Mhz...") % (freq/1e6) << std::endl;
    usrp.sdev->set_rx_freq(freq);
    std::cout << boost::format("Actual RX Freq: %f Mhz...") % (usrp.sdev->get_rx_freq()/1e6) << std::endl << std::endl;
}

//
//This is the main sample receiving thread (private)
void *usrp_receiver_thread (void *param)
{
    RECEIVER *pRec = (RECEIVER *)param;    
    //unsigned int total_num_samps = 2E9;
        
    size_t num_acc_samps = 0; //number of accumulated samples
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

    usrp_set_frequency(DEFAULT_RX_FREQUENCY); //TEMPORARY

    boost::this_thread::sleep(boost::posix_time::seconds(1)); //allow for some setup time
    
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
            if (num_acc_samps == 0) continue;
            std::cout << boost::format(
                "Got timeout before all samples received, possible packet loss, exiting loop..."
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
        for (unsigned int i=0; i<num_rx_samps; ++i) {                    
            
            rresamp_data->data_in[2*i]   = buff[i].real();
            rresamp_data->data_in[2*i+1] = buff[i].imag();            
        }
        
        //2.Rational resampling API here
        int rr_retcode = src_simple(rresamp_data, SRC_SINC_FASTEST, 2);
        
        //fprintf (stderr, "Simple resampling done. Obtained %ld samples.\n", rresamp_data->output_frames_gen);
        
        if (rr_retcode == RRESAMPLER_NO_ERROR) {
            
            for (int i=0; i<rresamp_data->output_frames_gen ; ++i) {                    
                    
                pRec->input_buffer[pRec->samples]             = rresamp_data->data_out[i];
                pRec->input_buffer[pRec->samples+BUFFER_SIZE] = rresamp_data->data_out[i+1];

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
        
        //num_acc_samps += num_rx_samps;        
    } done_loop:
	std::cerr << "Exiting USRP receiver thread" << std::endl;

    return 0;
}

bool usrp_start (RECEIVER *pRec)
{
    if (pthread_create(&pRec->client->thread_id,NULL,usrp_receiver_thread,(void *)pRec)!=0) {
        fprintf(stderr,"Failed to create USRP receiver thread for rx %d\n",pRec->client->receiver);
        return false; 
    } else {
        return true;
    }
}




