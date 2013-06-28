/** 
* \file main.c
* \brief Main file for the GHPSDR Software Defined Radio Graphic Interface. 
* \author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* \version 0.1
* \date 2009-04-11
*
*
* \mainpage GHPSDR 
*  \image html ../ghpsdr.png
*  \image latex ../ghpsdr.png "Screen shot of GHPSDR" width=10cm
*
* \section A Linux based, GTK2+, Radio Graphical User Interface to HPSDR boards through DttSP without Jack.  
* \author John Melton, G0ORX/N6LYT
* \version 0.1
* \date 2009-04-11
* 
* \author Dave Larsen, KV0S, Doxygen comments
*
* These files are design to build a simple 
* high performance  interface under the Linux  operating system.  
*
* This is still very much an Alpha version. It does still have problems and not everything is 
* completed.
*
* To build the application there is a simple Makefile.
*
* To run the application just start ghpsdr once it is built.
*
* Currently it does not include any code to load the FPGA so you must run initozy before 
* running the application. You must also have the latest FPGA code.
*
* Functionally, each band has 3 bandstacks. The frequency/mode/filter settings will be 
* saved when exiting the application for all the bandstack entries.
*
* Tuning can be accomplished by left mouse clicking in the Panadapter/Waterfall window to 
* move the selected frequency to the center of the current filter. A right mouse click will 
* move the selected frequency to the cursor. You can also use the left mouse button to drag 
* the frequency by holding it down while dragging. If you have a scroll wheel, moving the 
* scroll wheel will increment/decrement the frequency by the current step amount.
*
* You can also left mouse click on the bandscope display and it will move to the selected frequency.
* 
* The Setup button pops up a window to adjust the display settings. There are no tests 
* currently if these are set to invalid values.
*
*
* There are some problems when running at other than 48000. Sometimes the audio output will 
* stop although the Panadapter/Waterfall and bandscope continue to function. It usually 
* requires intiozy to be run again to get the audio back.
*
*
* Development of the system is documented at 
* http://javaguifordttsp.blogspot.com/
*
* This code is available at 
* svn://206.216.146.154/svn/repos_sdr_hpsdr/trunk/N6LYT/ghpsdr
*
* More information on the HPSDR project is availble at 
* http://openhpsdr.info
*
*/

/* Copyright (C) 
* 2009 - John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
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

const char *version = "20130628;-opengl"; //YYYYMMDD; text desc

// main.c

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/param.h>

#include "client.h"
#include "dttsp.h"
#include "audiostream.h"
#include "soundcard.h"
#include "ozy.h"
#include "version.h"
#include "codec2loc.h"
#include "register.h"
#include "sdrexport.h"
#include "G711A.h"
#include "rtp.h"
#include "util.h"
#include "main.h"

char propertyPath[128];

enum {
    OPT_SOUNDCARD = 1,
    OPT_RECEIVER,
    OPT_SERVER,
    OPT_OFFSET,
    OPT_TIMING,
    OPT_LOOKUPCOUNTRY,
    OPT_SHARE,
    OPT_SHARECONFIG,
    OPT_LO,
    OPT_HPSDR,
    OPT_NOCORRECTIQ,
    OPT_DEBUG,
    OPT_THREAD_DEBUG
};

struct option longOptions[] = {
    {"soundcard",required_argument, NULL, OPT_SOUNDCARD},
    {"receiver",required_argument, NULL, OPT_RECEIVER},
    {"server",required_argument, NULL, OPT_SERVER},
    {"offset",required_argument, NULL, OPT_OFFSET},
    {"timing",no_argument, NULL, OPT_TIMING},
    {"lookupcountry",no_argument, NULL, OPT_LOOKUPCOUNTRY},
    {"share",no_argument, NULL, OPT_SHARE},
    {"shareconfig",required_argument, NULL, OPT_SHARECONFIG},
    {"lo",required_argument, NULL, OPT_LO},
    {"hpsdr",no_argument, NULL, OPT_HPSDR},
    {"nocorrectiq",no_argument, NULL, OPT_NOCORRECTIQ},
    {"debug",no_argument, NULL, OPT_DEBUG},
#ifdef THREAD_DEBUG
    {"debug-threads",no_argument, NULL, OPT_THREAD_DEBUG},
#endif /* THREAD_DEBUG */
    {0,0,0,0}
};

char* shortOptions="";

void signal_shutdown(int signum);

/* --------------------------------------------------------------------------*/
/** 
* @brief Process program arguments 
* 
* @param argc
* @param argv
*/
/* ----------------------------------------------------------------------------*/
void processCommands(int argc,char** argv,struct dspserver_config *config) {
    int c;
    while((c=getopt_long(argc,argv,shortOptions,longOptions,NULL))!=-1) {
        switch(c) {
            case OPT_SOUNDCARD:
                if (strlen(optarg) > sizeof(config->soundCardName) - 1) {
                    fprintf(stderr, "Warning: Sound card name will be truncated\n");
                }
                strncpy(config->soundCardName,optarg,sizeof(config->soundCardName));
                break;
            case OPT_RECEIVER:
                /* FIXME: global */
                receiver=atoi(optarg);
                break;
            case OPT_SERVER:
                /* FIXME: global */
                if (strlen(optarg) > sizeof(config->server_address) - 1) {
                    fprintf(stderr, "Warning: server address will be truncated\n");
                }
                strncpy(config->server_address, optarg, sizeof(config->server_address));
                break;
            case OPT_OFFSET:
                config->offset=atoi(optarg);
                break;
            case OPT_TIMING:
                client_set_timing();
                break;
            case OPT_LOOKUPCOUNTRY:
                setprintcountry();
                break;
            case OPT_SHARE:
                toShareOrNotToShare = 1;
                break;
            case OPT_SHARECONFIG:
                if (strlen(optarg) > sizeof(config->share_config_file) - 1) {
                    fprintf(stderr, "Warning: share config file path is too long for this system\n");
                }
                strncpy(config->share_config_file,optarg, sizeof(config->share_config_file));
                break;
            case OPT_LO:
                /* global */
                LO_offset=atoi(optarg);
                break;
            case OPT_HPSDR:
                ozy_set_hpsdr();
                break;
            case OPT_NOCORRECTIQ:
                config->no_correct_iq = 1;
                break;
            case OPT_DEBUG:
                ozy_set_debug(1);
                break;
            case OPT_THREAD_DEBUG:
                config->thread_debug = 1;
                break;

       default:
                fprintf(stderr,"Usage: \n");
                fprintf(stderr,"  dspserver --receivers N (default 1)\n");
                fprintf(stderr,"            --server 0.0.0.0 (default 127.0.0.1)\n");
                fprintf(stderr,"            --soundcard (machine dependent)\n");
                fprintf(stderr,"            --offset 0 \n");
                fprintf(stderr,"            --share (will register this server for other users \n");
                fprintf(stderr,"                     use the default config file ~/.dspserver.conf) \n");
		fprintf(stderr,"            --lo 0 (if no LO offset desired in DDC receivers, or 9000 in softrocks\n");
		fprintf(stderr,"            --hpsdr (if using hpsdr hardware)\n");
		fprintf(stderr,"            --nocorrectiq (select if using non QSD receivers, like Perseus, HiQSDR, Mercury)\n");
#ifdef THREAD_DEBUG
                fprintf(stderr,"            --debug-threads (enable threading assertions)\n");
#endif /* THREAD_DEBUG */
                exit(1);

        }
    }
}


/* --------------------------------------------------------------------------*/
/** 
* @brief  Main - it all starts here
* 
* @param argc
* @param argv[]
* 
* @return 
*/
/* ----------------------------------------------------------------------------*/

struct dspserver_config config;

int main(int argc,char* argv[]) {
    memset(&config, 0, sizeof(config));
    // Register signal and signal handler
    signal(SIGINT, signal_shutdown);    
    char directory[MAXPATHLEN];
    strcpy(config.soundCardName,"HPSDR");
    strcpy(config.server_address,"127.0.0.1"); // localhost
    strcpy(config.share_config_file, getenv("HOME"));
    strcat(config.share_config_file, "/dspserver.conf");
    processCommands(argc,argv,&config);

#ifdef THREAD_DEBUG
    sdr_threads_init();
#endif /* THREAD_DEBUG */

    fprintf(stderr, "Reading conf file %s\n", config.share_config_file);
    init_register(config.share_config_file); // we now read our conf always
	 // start web registration if set
    if  (toShareOrNotToShare) {
        fprintf(stderr, "Activating Web register\n");
    }
    fprintf(stderr,"gHPSDR rx %d (Version %s)\n",receiver,VERSION);
    printversion();
    setSoundcard(getSoundcardId(config.soundCardName));

    // initialize DttSP
    if(getcwd(directory, sizeof(directory))==NULL) {
        fprintf(stderr,"current working directory path is > MAXPATHLEN");
        exit(1);
    }
    Setup_SDR(directory);
    Release_Update();
    SetTRX(0,0); // thread 0 is for receive
    SetTRX(1,1);  // thread 1 is for transmit
    SetRingBufferOffset(0,config.offset);
    SetThreadProcessingMode(0,RUN_PLAY);
    SetThreadProcessingMode(1,RUN_PLAY);
    SetSubRXSt(0,1,1);
    SetRXOutputGain(0,0,0.20);
    SetRXOsc(0,0, -LO_offset);
    SetRXOsc(0,1, -LO_offset);
    reset_for_buflen(0,1024);
    reset_for_buflen(1,1024);

    client_init(receiver);
    audio_stream_init(receiver);
    audio_stream_reset();

    codec2 = codec2_create(CODEC2_MODE_3200);
    G711A_init();
    ozy_init(config.server_address);

    SetMode(1, 0, USB);
    SetTXFilter(1, 150, 2850);
    SetTXOsc(1, LO_offset);
    SetTXAMCarrierLevel(1, 0.5);

    tx_init();	// starts the tx_thread

#ifdef THREAD_DEBUG
    /* Note that some thread interactions will be lost at startup due to
     * the fact that the subsystem threads are all started.  We can't
     * init this until late, though, or we'll catch initializations
     * performed at boot time as errors. */
    if (config.thread_debug) {
        sdr_threads_debug(TRUE);
    }
#endif /* THREAD_DEBUG */

    while(1) {
        sleep(10000);
    }

 //   codec2_destroy(codec2);
    return 0;
}

void signal_shutdown(int signum)
{
   // catch a ctrl c etc
   printf("Caught signal %d\n",signum);
   // Cleanup and close up stuff here
   close_register();
   if  (toShareOrNotToShare) { 
	   fprintf(stderr,"Please wait while unregistering ...\n");
	   doRemove();
	}
   // Terminate program
   exit(signum);
}
