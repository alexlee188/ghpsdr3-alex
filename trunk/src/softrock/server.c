/**
* @file server.c
* @brief HPSDR server application
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
* This file contains the main program and the command line processing.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <getopt.h>

#include <usb.h>
#include <libusb-1.0/libusb.h>

#include "client.h"
#include "listener.h"
#include "softrock.h"
#include "receiver.h"
#include "operations.h"

void usage(void);

static struct option long_options[] = {
    {"samplerate",required_argument, 0, 's'},
    {"device",required_argument, 0, 'd'},
    {"input",required_argument, 0, 'i'},
    {"output",required_argument, 0, 'o'},
    {"iq",no_argument, 0, 4},
    {"qi",no_argument, 0, 5},
    {"si570",no_argument, 0, 6},
	{"usb2sdr",no_argument, 0, 15},
    {"verbose",no_argument, 0, 'v'},
    {"startupfreq",required_argument, 0, 'f'},
    {"multiplier",required_argument, 0, 'm'},
    {"correctedfreq",required_argument, 0, 10},
    {"serialid",required_argument, 0, 11},
    {"record",required_argument, 0, 'r'},
    {"playback",required_argument, 0, 'p'},
	{"receivers",required_argument, 0, 14},
	{"jack",no_argument, 0, 'j'},
	{0, 0, 0, 0}
};

static char* short_options="s:d:i:o:vf:m:r:p:j";
static int option_index;

int si570=0;
int usb2sdr=0;
double startupFreq=56.32;
double multiplier=4;
int i2cAddress = 0x55;
double fXtall = 114.285;
char* usbSerialID=NULL;
int setByValue = 1;

usb_dev_handle* handle = NULL;
libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
libusb_device_handle *usb2sdr_handle; //a device handle
libusb_context *ctx = NULL; //a libusb session

void ReadFile(char *name, unsigned char **buffer, unsigned long *len)
{
	FILE *file;
	unsigned long fileLen;
	
	//Open file
	file = fopen(name, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s", name);
		return;
	}

	//Get file length
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);
	
	//allocate a buffer to keep the binary
	*buffer = (unsigned char *)malloc(400000);
	if((*buffer) == NULL){
    	printf("DEBUG: allocation PROBLEM\n");       
    }
	else printf("DEBUG: allocation for binary done ok\n");
	
	//Read file contents into buffer
	fread(*buffer, fileLen, 1, file);
	fclose(file);

	*len = fileLen;
}


void process_args(int argc,char* argv[]);

int main(int argc,char* argv[]) {

    int rc;
#ifdef DIRECTAUDIO
    softrock_set_device("/dev/dsp");
#endif
	int r; //for return values
	ssize_t cnt; //holding number of devices in list
	unsigned char *binFile;
	unsigned char tmp;
	unsigned long binlen,i;
	int actual; //used to find out how many bytes were written
	
    process_args(argc,argv);

    if(si570) {
        usb_init();
        rc=usbOpenDevice(&handle,0x16C0,"www.obdev.at",0x05DC,"DG8SAQ-I2C",usbSerialID);
        if (rc!=0) rc=usbOpenDevice(&handle,0x16C0,"SDR-Widget",0x05DC,"Yoyodyne SDR-Widget",usbSerialID);
        if(rc!=0) {
            fprintf(stderr,"Cannot open USB device\n");
            exit(1);
        }
    }

	if(usb2sdr) {
		//printf("usb2sdr selected\n"); //debug
		r = libusb_init(&ctx); //initialize the library for the session we just declared
	    if(r < 0) {
			printf("Init Error %d\n",r);
			return 1;
    	}
		//printf("libusb_init ok\n"); //debug
    	libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation
	    cnt = libusb_get_device_list(ctx, &devs); //get the list of devices
    	if(cnt < 0) {
			printf("Get Device Error\n");		
    	}
		//printf("libusb_get_device_list ok\n"); //debug
		//first check if the USB2SDR is present
		usb2sdr_handle = libusb_open_device_with_vid_pid(ctx, 0x0451, 0x9099); //USB2SDR id while configured
		if(usb2sdr_handle == NULL){
			printf("A configured USB2SDR was not found\n");
			//Lets check for unconfigured USB2SDR
			usb2sdr_handle = NULL;
			usb2sdr_handle = libusb_open_device_with_vid_pid(ctx, 0x0451, 0x9010); //unconfigured device id
			if(usb2sdr_handle == NULL){
				printf("No USB2SDR hardware was found. Please check the USB cable\n");
				return 0;
			}
			else{
				printf("Found unconfigured USB2SDR board\n");
		   		libusb_free_device_list(devs, 1); //free the list, unref the devices in it
				if(libusb_kernel_driver_active(usb2sdr_handle, 0) == 1) { //find out if kernel driver is attached
					printf("Kernel Driver Active\n");
					if(libusb_detach_kernel_driver(usb2sdr_handle, 0) == 0) //detach it
						printf("Kernel Driver Detached\n");
				}
				r = libusb_claim_interface(usb2sdr_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)
				if(r < 0) {
					printf("Cannot Claim Interface 0 of USB2SDR\n");				
				}
				printf("Claimed Interface 0 of USB2SDR\n");
				//now open the bin file
				ReadFile((char*)"USB2SDR.bin", &binFile, &binlen);
				printf("USB2SDR.bin loaded. bytes read:%ld\n",binlen);
				//take care of endianess
				for(i=0;i<binlen;i+=2){
					tmp = binFile[i];
					binFile[i] = binFile[i+1];
					binFile[i+1] = tmp;
				}
				printf("Board firmware image to write is:%ld bytes\n",binlen);
				printf("Sending the firmware:");
				r = libusb_bulk_transfer(usb2sdr_handle, (1 | LIBUSB_ENDPOINT_OUT), binFile, 65536, &actual, 2000); 
				if(r == 0 && actual == 65536) //we wrote the full record bytes successfully
					printf(".");
				else
					printf("1st part write error\n");
				r = libusb_bulk_transfer(usb2sdr_handle, (1 | LIBUSB_ENDPOINT_OUT), &binFile[65536], 65536, &actual, 2000); 
				if(r == 0 && actual == 65536) //we wrote the full record bytes successfully
					printf(".");
				else
					printf("2nd part write error\n");
				r = libusb_bulk_transfer(usb2sdr_handle, (1 | LIBUSB_ENDPOINT_OUT), &binFile[131072], (binlen-131072), &actual, 2000); 
				if(r == 0 && actual == (binlen-131072)) //we wrote the full record bytes successfully
					printf(".\n");
				else
					printf("3rd part write error\n");
				r = libusb_release_interface(usb2sdr_handle, 0); //release the claimed interface
				if(r!=0) {
					printf("Cannot release Interface\n");
				}
				printf("Interface Released\n");
				free(binFile);
				libusb_close(usb2sdr_handle); //close the device we opened
				sleep(3);	//3seconds for the USB2SDR to be detected with its new ID
				
				sleep(2);
				usb2sdr_handle = libusb_open_device_with_vid_pid(ctx, 0x0451, 0x9099); //these are vendorID and productID of loaded USB2SDR
				if(usb2sdr_handle == NULL){
					printf("Cannot open USB2SDR");
					//return 0;
				}
				else{
					printf("USB2SDR found and Opened\n");	
				}
				if(libusb_kernel_driver_active(usb2sdr_handle, 0) == 1) { //find out if kernel driver is attached
					printf("Kernel Driver Active");
					if(libusb_detach_kernel_driver(usb2sdr_handle, 0) == 0){ //detach it
						printf("Kernel Driver Detached!");
					}
				}
				r = libusb_claim_interface(usb2sdr_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)
				if(r < 0) {
					printf("Cannot Claim Interface");
				}
				printf("Claimed interface 0\n");
			}//if unconfigured card was opened
		}
		else{
			printf("Found a configured USB2SDR. Ready for action\n");
		}
		
	}

    init_receivers();

    create_listener_thread();

    create_softrock_thread();

    while(1) {
        sleep(10);
    }
}

void process_args(int argc,char* argv[]) {
    int i;

    // set defaults
    softrock_set_receivers(1);
    softrock_set_sample_rate(96000);
	softrock_set_jack(0);
    //softrock_set_device("/dev/dsp");

    while((i=getopt_long(argc,argv,short_options,long_options,&option_index))!=EOF) {
        switch(i) {
            case 's': // sample rate
				if (long_options[option_index].flag != 0){
					fprintf(stderr,"invalid argument\n");
                	exit(1);
				}
				printf ("option %s", long_options[option_index].name);
               	if (optarg)
                	printf (" with arg %s", optarg);
               	printf ("\n");
				fprintf(stderr,"process_args: samplerate=%s\n",optarg);
                softrock_set_sample_rate(atoi(optarg));
                break;
            case 'd': // device
				fprintf(stderr,"process_args: device=%s\n",optarg);
                softrock_set_device(optarg);
                break;
            case 'i': // input
				fprintf(stderr,"process_args: input=%s\n",optarg);
                softrock_set_input(optarg);
                break;
            case 'o': // output
				fprintf(stderr,"process_args: output=%s\n",optarg);
                softrock_set_output(optarg);
                break;
            case 4: // iq
                softrock_set_iq(1);
                break;
            case 5: // qi
                softrock_set_iq(0);
                break;
            case 6: // si570
                si570=1;
                break;
            case 'v': // verbose
				fprintf(stderr,"Set verbose flag.\n");
                softrock_set_verbose (1);
                break;
            case 'f': // startupfreq
                startupFreq=atof(optarg);
                break;
            case 'm': // multiplier
                multiplier=atof(optarg);
                break;
            case 10: // corrected xtal freq
                fXtall=atof(optarg);
				setByValue = 0;
                break;
            case 11: // serial ID
                usbSerialID=optarg;
                break;
            case 'r': // record
                softrock_set_record(optarg);
                break;
            case 'p': // playback
                softrock_set_playback(optarg);
                break;
			case 14: // receivers
                softrock_set_receivers(atoi(optarg));
                break;
			case 'j': // jack
                softrock_set_jack(1);
                break;
			case 15: //usb2sdr board usage
				usb2sdr = 1;
				break;
            default:
				fprintf(stderr,"invalid argument\n");
				usage(); 
                exit(1);
        }
    }
}

void usage( void )
{
	fprintf(stderr,"Usage options:\n");
	fprintf(stderr,"	-s= or --samplerate  \n");
	fprintf(stderr,"	-d= or --device (OSS audio device) \n");
	fprintf(stderr,"	-i= or --input (Port Audio device for input) \n");
	fprintf(stderr,"	-o= or --output (Port Audio device for output) \n");
	fprintf(stderr,"	--iq or --qi (to swap I and Q channels)  \n");
	fprintf(stderr,"	--si570 (to use a unit having a Si570 oscillater) \n");
	fprintf(stderr,"	-v or --verbose  \n");
	fprintf(stderr,"	-f= or --startupfreq (set Si570 startup frequency) \n");
	fprintf(stderr,"	-m= or --multiplier (set Si570 multiplier) \n");
	fprintf(stderr,"	--correctedfreq (set Si570 corrected frequency) \n");
	fprintf(stderr,"	--serialid  \n");
	fprintf(stderr,"	--record filename (record to this file)\n");
	fprintf(stderr,"	--playback filename (playback this file)  \n");
	fprintf(stderr,"	--receivers (number of receivers) \n");
	fprintf(stderr,"	-j or --jack (use Jack Audio Connection Kit for audio in/out)  \n");
}

	        
