/**
* @file ozy.c
* @brief Ozy protocol implementation
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
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __linux__
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/timeb.h>
#include <sys/stat.h> // for stat
#include <pthread.h>
#include <unistd.h>   // for readlink
#include <limits.h>   // for PATH_MAX
#include <errno.h>
#else
#include "pthread.h"
#endif
#include <assert.h>

#include "client.h"
#include "ozyio.h"
#include "bandscope.h"
#include "receiver.h"
//#include "transmitter.h"
#include "util.h"
#include "metis.h"
#include "ozy.h"

#define THREAD_STACK 32768

#define DEFAULT_OZY_BUFFERS 16
#define OZY_BUFFER_SIZE 512
#define OZY_HEADER_SIZE 8

static int ozy_buffers=DEFAULT_OZY_BUFFERS;

#define SYNC 0x7F

// ozy command and control
#define MOX_DISABLED    0x00
#define MOX_ENABLED     0x01

#define MIC_SOURCE_JANUS 0x00
#define MIC_SOURCE_PENELOPE 0x80
#define CONFIG_NONE     0x00
#define CONFIG_PENELOPE 0x20
#define CONFIG_MERCURY  0x40
#define CONFIG_BOTH     0x60
#define PENELOPE_122_88MHZ_SOURCE 0x00
#define MERCURY_122_88MHZ_SOURCE  0x10
#define ATLAS_10MHZ_SOURCE        0x00
#define PENELOPE_10MHZ_SOURCE     0x04
#define MERCURY_10MHZ_SOURCE      0x08
#define SPEED_48KHZ               0x00
#define SPEED_96KHZ               0x01
#define SPEED_192KHZ              0x02
#define SPEED_384KHZ              0x03

#define MODE_CLASS_E              0x01
#define MODE_OTHERS               0x00

#define ALEX_ATTENUATION_0DB      0x00
#define ALEX_ATTENUATION_10DB     0x01
#define ALEX_ATTENUATION_20DB     0x02
#define ALEX_ATTENUATION_30DB     0x03
#define LT2208_GAIN_OFF           0x00
#define LT2208_GAIN_ON            0x04
#define LT2208_DITHER_OFF         0x00
#define LT2208_DITHER_ON          0x08
#define LT2208_RANDOM_OFF         0x00
#define LT2208_RANDOM_ON          0x10

#define SIMPLEX                   0x00
#define DUPLEX                    0x04


static pthread_t ep6_ep2_io_thread_id;
static pthread_t ep4_io_thread_id;
static pthread_t playback_thread_id;

static int configure=6;
static int rx_frame=0;
static int tx_frame=0;
static int receivers=1;
static int current_receiver=0;

static int speed=1;
static int sample_rate=96000;
static int output_sample_increment=2;

static int timing=0;
static struct timeb rx_start_time;
static struct timeb rx_end_time;
static struct timeb tx_start_time;
static struct timeb tx_end_time;
static int rx_sample_count=0;
static int tx_sample_count=0;

static unsigned char control_in[5]={0x00,0x00,0x00,0x00,0x00};
static unsigned char control_out_metis[5]={
  MOX_DISABLED,
  CONFIG_BOTH | MERCURY_122_88MHZ_SOURCE | MERCURY_10MHZ_SOURCE | MIC_SOURCE_PENELOPE | SPEED_96KHZ,
  MODE_OTHERS,
  ALEX_ATTENUATION_0DB | LT2208_GAIN_OFF | LT2208_DITHER_ON | LT2208_RANDOM_ON,
  SIMPLEX
};

static unsigned char control_out_hermes[5]={
  MOX_DISABLED,
  CONFIG_BOTH | MERCURY_122_88MHZ_SOURCE | MERCURY_10MHZ_SOURCE | MIC_SOURCE_PENELOPE | SPEED_96KHZ,
  MODE_OTHERS,
  ALEX_ATTENUATION_0DB | LT2208_GAIN_OFF | LT2208_DITHER_ON | LT2208_RANDOM_ON,
  DUPLEX // changed from SIMPLEX in order to avoid Hermes frequency lagging
};

static unsigned char *control_out = control_out_metis;

/*
C0
0 0 0 1 0 0 1 x    


C1
0 0 0 0 0 0 0 0
|             |
+-------------+------------ Hermes/PennyLane Drive Level (0-255)1

1 Ignored by Penelope


C2
0 0 0 0 0 0 0 0
| | | | | | | |
| | | | | | | +------------ Hermes/Metis Penelope Mic boost (0 = 0dB, 1 = 20dB)
| | | | | | +-------------- Metis/Penelope or PennyLane Mic/Line-in (0 = mic, 1 = Line-in)
| | | | | +---------------- Hermes – Enable/disable Apollo filter (0 = disable, 1 = enable)
| | | | +------------------ Hermes – Enable/disable Apollo tuner (0 = disable, 1 = enable)
| | | +-------------------- Hermes – Apollo auto tune (0 = end, 1 = start)
| | +---------------------- Hermes – select filter board (0 = Alex, 1 = Apollo)
| +------------------------ Alex   - manual HPF/LPF filter select (0 = disable, 1 = enable)2
+-------------------------- VNA Mode (0 = off, 1 = on)

C3
0 0 0 0 0 0 0 0
| | | | | | | |
| | | | | | | +------------ Alex   -	select 13MHz  HPF (0 = disable, 1 = enable)2
| | | | | | +-------------- Alex   -	select 20MHz  HPF (0 = disable, 1 = enable)2
| | | | | +---------------- Alex   -	select 9.5MHz HPF (0 = disable, 1 = enable)2
| | | | +------------------ Alex   -	select 6.5MHz HPF (0 = disable, 1 = enable)2
| | | +-------------------- Alex   -	select 1.5MHz HPF (0 = disable, 1 = enable)2
| | +---------------------- Alex   -	Bypass all HPFs   (0 = disable, 1 = enable)2
| +------------------------ Alex   -	6M low noise amplifier (0 = disable, 1 = enable)2
+-------------------------- Disable Alex T/R relay (0 = enable, 1 = disable) 


C4
0 0 0 0 0 0 0 0
  | | | | | | |
  | | | | | | +------------ Alex   - 	select 30/20m LPF (0 = disable, 1 = enable) (2)
  | | | | | +-------------- Alex   - 	select 60/40m LPF (0 = disable, 1 = enable) (2)
  | | | | +---------------- Alex   - 	select 80m    LPF (0 = disable, 1 = enable) (2)
  | | | +------------------ Alex   - 	select 160m   LPF (0 = disable, 1 = enable) (2)
  | | +-------------------- Alex   - 	select 6m     LPF (0 = disable, 1 = enable) (2)
  | +---------------------- Alex   - 	select 12/10m LPF (0 = disable, 1 = enable) (2)
  +------------------------ Alex   - 	select 17/15m LPF (0 = disable, 1 = enable) (2)

(2) Only valid when Alex - manual HPF/LPF filter select is enabled
*/

static unsigned char control_out_hermes_pow[5]={
/* C0 */   0x12 | MOX_DISABLED,                         
/* C1 */   0,                    //   Hermes/PennyLane Drive Level
/* C2 */   0,   
/* C3 */   0,
/* C4 */   0,
};

/*
C0
0 0 0 1 0 1 0 x   

C1
0 0 0 0 0 0 0 0
        | | | |
        | | | +------------ Rx1 pre-amp (0=OFF, 1= ON)
        | | +-------------- Rx2 pre-amp (0=OFF, 1= ON)
        | +---------------- Rx3 pre-amp (0=OFF, 1= ON)
        +------------------ Rx4 pre-amp (0=OFF, 1= ON)

C2
0 0 0 0 0 0 0 0
      | | | | |
      | | | | +------------ TLV320 Line-in Gain bit 0 (3) 
      | | | +-------------- TLV320 Line-in Gain bit 1 (3)
      | | +---------------- TLV320 Line-in Gain bit 2 (3)
      | +------------------ TLV320 Line-in Gain bit 3 (3)
      +-------------------- TLV320 Line-in Gain bit 4 (3)

(3) Sets TLV320 line_boost value when Metis or Hermes is used.

C3
0 0 0 0 0 0 0 0
        | | | |
        | | | +------------ Metis DB9 pin 1 Open Drain Output (0=OFF, 1= ON)
        | | +-------------- Metis DB9 pin 2 Open Drain Output (0=OFF, 1= ON)
        | +---------------- Metis DB9 pin 3 3.3v TTL Output (0=OFF, 1= ON)
        +------------------ Metis DB9 pin 4 3.3v TTL Output (0=OFF, 1= ON)

C4
0 0 0 0 0 0 0 0
      |       |
      +-------+------------ Hermes Input Attenuator (0 - 31dB) [4:0]
*/

static unsigned char control_out_hermes_att[5]={
/* C0 */   0x14 | MOX_DISABLED,
/* C1 */   0x00,                 //   Rx preamps
/* C2 */   0,                    //   TLV320 Line-in Gain
/* C3 */   0,                    //   Metis DB9
/* C4 */   0x20,                 //   Hermes Input Attenuator: ACTIVE
};

static int mox=0;

static int ptt=0;
static int dot=0;
static int dash=0;
static int lt2208ADCOverflow=0;

static unsigned char ozy_firmware_version[9];
static int mercury_software_version=0;
static int penelope_software_version=0;
static int ozy_software_version=0;
static int hermes_software_version=0;

static int forwardPower=0;
static int alexForwardPower=0;
static int alexReversePower=0;
static int AIN3=0;
static int AIN4=0;
static int AIN6=0;
static int IO1=1; // 1 is inactive
static int IO2=1;
static int IO3=1;
static int IO4=1;


static int samples=0;

static float mic_gain=0.26F;
//static float mic_left_buffer[BUFFER_SIZE];
//static float mic_right_buffer[BUFFER_SIZE];

static char ozy_firmware[64] = {0};
static char ozy_fpga[64] = {0};

static unsigned char ozy_output_buffer[OZY_BUFFER_SIZE];
static int ozy_output_buffer_index=OZY_HEADER_SIZE;


static char filename[256];
static int record=0;
static int playback=0;
static int playback_sleep=0;
static FILE* recording;

int metis=0;
int hermes=0;


void  (* write_ozy_output_buffer)(void) = 0;

void ozy_prime();
void* ozy_ep6_ep2_io_thread(void* arg);
void* ozy_ep4_io_thread(void* arg);
void* playback_thread(void* arg);

#ifndef __linux__
#define bool int
bool init_hpsdr();
#endif

void ozy_set_fpga_image(const char *s) {
    strcpy (ozy_fpga, s);
}

void ozy_set_hex_image(const char *s) {
    strcpy (ozy_firmware, s);
}

void ozy_set_buffers(int buffers, int hermes) {
    ozy_buffers=buffers;
    if (hermes) {
       control_out = control_out_hermes;
       write_ozy_output_buffer = write_ozy_output_buffer_hermes;
    } else {
        write_ozy_output_buffer = write_ozy_output_buffer_metis;
    }
}

void ozy_set_metis(int state) {
    metis=state;
}

void ozy_set_hermes(int state) {
    hermes=state;
}

int create_ozy_thread() {
    int rc;

#ifndef __linux__
    if (init_hpsdr() == 0) exit(9);
#endif

    //for(i=0;i<receivers;i++) {
    //    receiver[i].frequency=7056000L;
    //    receiver[i].frequency_changed=1;
    //}

    if(!playback) {
        ozy_init();
        ozy_prime();
    }

    if(timing) {
        ftime(&rx_start_time);
        ftime(&tx_start_time);
    }

    if(playback) {
        // create a thread to read/write to EP6/EP2
        rc=pthread_create(&playback_thread_id,NULL,playback_thread,NULL);
        if(rc != 0) {
            fprintf(stderr,"pthread_create failed on playback_thread: rc=%d\n", rc);
            exit(1);
        }
    } else {
        // create a thread to read/write to EP6/EP2
        rc=pthread_create(&ep6_ep2_io_thread_id,NULL,ozy_ep6_ep2_io_thread,NULL);
        if(rc != 0) {
            fprintf(stderr,"pthread_create failed on ozy_ep6_io_thread: rc=%d\n", rc);
            exit(1);
        }

        // create a thread to read from EP4
        rc=pthread_create(&ep4_io_thread_id,NULL,ozy_ep4_io_thread,NULL);
        if(rc != 0) {
            fprintf(stderr,"pthread_create failed on ozy_ep4_io_thread: rc=%d\n", rc);
            exit(1);
        }
    }

    return 0;
}

void ozy_set_record(char* f) {
    if(playback||record) {
        fclose(recording);
    }
    strcpy(filename,f);
    recording=fopen(filename,"w");
    record=1;
    playback=0;

fprintf(stderr,"recording\n");
}

void ozy_stop_record() {
    if(record) {
        fclose(recording);
    }
    record=0;
fprintf(stderr,"stopped recording\n");
}

void ozy_set_playback(char* f) {
    if(playback||record) {
        fclose(recording);
    }
    strcpy(filename,f);
    recording=fopen(filename,"r");
    playback=1;
    record=0;

fprintf(stderr,"starting playback: %s\n",filename);
}

void ozy_set_receivers(int r) {
    if(r>MAX_RECEIVERS) {
        fprintf(stderr,"MAX Receivers is 8!\n");
        exit(1);
    }
    receivers=r;
    control_out_metis[4] &= 0xc7;
    control_out_metis[4] |= (r-1)<<3;
    control_out_hermes[4] &= 0xc7;
    control_out_hermes[4] |= (r-1)<<3;
}

int ozy_get_receivers() {
    return receivers;
}

void ozy_set_sample_rate(int r) {
    switch(r) {
        case 48000:
            sample_rate=r;
            speed=0;
            output_sample_increment = 1;
            break;
        case 96000:
            sample_rate=r;
            speed=1;
            output_sample_increment = 2;
            break;
        case 192000:
            sample_rate=r;
            speed=2;
            output_sample_increment = 4;
            break;
        case 384000:
            sample_rate=r;
            speed=3;
            output_sample_increment = 8;
            break;
        default:
            fprintf(stderr,"Invalid sample rate (48000,96000,192000,384000)!\n");
            exit(1);
            break;
    }
    control_out_metis[1] &= 0xfc;
    control_out_metis[1] |= speed;
    control_out_hermes[1] &= 0xfc;
    control_out_hermes[1] |= speed;

    //playback_sleep=(int)((1000000.0/380.0));
    playback_sleep=3090;
    fprintf(stderr,"************** receivers=%d sample_rate=%d playback_sleep=%d\n",receivers,sample_rate,playback_sleep);

}

int ozy_set_playback_sleep(int sleep) {
    playback_sleep=sleep;
    return 0;
}

int ozy_get_sample_rate() {
    return sample_rate;
}


static int file_exists (const char * fileName)
{
   struct stat buf;
   int i = stat ( fileName, &buf );
   return ( i == 0 ) ? 1 : 0 ;
}

#ifdef __linux__
int filePath (char *sOut, const char *sIn) {
    int rc = 0;

    if ((rc = file_exists (sIn))) {
       strcpy (sOut, sIn); 
       rc = 1;
    } else {
      char cwd[PATH_MAX];
      char s[PATH_MAX];
      char xPath [PATH_MAX] = {0};
      char *p;

      int  rc = readlink ("/proc/self/exe", xPath, sizeof(xPath));

      // try to detect the directory from which the executable has been loaded
      if (rc >= 0) {

          if ( (p = strrchr (xPath, '/')) ) *(p+1) = '\0';
          fprintf (stderr, "%d, Path of executable: [%s]\n", rc, xPath);

          strcpy (s, xPath); strcat (s, sIn);

          if ((rc = file_exists (s))) {
             // found in the same dir of executable
             fprintf (stderr, "File: [%s]\n", s);
             strcpy(sOut, s);
          } else { 
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                fprintf(stdout, "Current working dir: %s\n", cwd);

                strcpy (s, cwd); strcat (s, "/"); strcat (s, sIn);
                if ((rc = file_exists (s))) {
                   fprintf (stderr, "File: [%s]\n", s);
                   strcpy(sOut, s);
                }
            }
          }
       } else {
          fprintf (stderr, "%d: %s\n", errno, strerror(errno));
       }
    }
    return rc;
}
#endif


int ozy_init(void) {
    int rc;

    // On Windows, the following is replaced by init_hpsdr() in OzyInit.c
#ifdef __linux__

    if (strlen(ozy_firmware) == 0) filePath (ozy_firmware,"ozyfw-sdr1k.hex");
    if (strlen(ozy_fpga) == 0)     filePath (ozy_fpga,"Ozy_Janus.rbf");

    // open ozy
    rc = ozy_open();
    if (rc != 0) {
        fprintf(stderr,"Cannot locate Ozy\n");
        exit(1);
    }

    // load Ozy FW
    ozy_reset_cpu(1);
    ozy_load_firmware(ozy_firmware);
    ozy_reset_cpu(0);
ozy_close();
    sleep(5);
ozy_open();
    ozy_set_led(1,1);
    ozy_load_fpga(ozy_fpga);
    ozy_set_led(1,0);
    ozy_close();

    ozy_open();
    rc=ozy_get_firmware_string(ozy_firmware_version,8);
    fprintf(stderr,"Ozy FX2 version: %s\n",ozy_firmware_version);
#else
    strcpy(ozy_firmware,"ozyfw-sdr1k.hex");
    strcpy(ozy_fpga,"Ozy_Janus.rbf");
#endif

    return rc;
}

void ozy_prime() {
    int i;

    for(i=0;i<receivers;i++) {
        receiver[i].frequency=7056000L;
        receiver[i].frequency_changed=1;
    }

    memset((char *)&ozy_output_buffer,0,OZY_BUFFER_SIZE);
    while(configure>0) {
fprintf(stderr,"ozy_prime: configure=%d\n",configure);
        write_ozy_output_buffer();
    }


    for(i=0;i<receivers;i++) {
        current_receiver=i;
fprintf(stderr,"ozy_prime: current_receiver=%d\n",current_receiver);
        write_ozy_output_buffer();
    }

    if((receivers%2)==1) {
        write_ozy_output_buffer();
    }

    current_receiver=0;

fprintf(stderr,"server configured for %d receivers at %d\n",receivers,sample_rate);
}

void* ozy_ep6_ep2_io_thread(void* arg) {
    unsigned char input_buffer[OZY_BUFFER_SIZE*ozy_buffers];
    int bytes;
    int i;

    while(1) {
        // read an input buffer (blocks until all bytes read)
        bytes=ozy_read(0x86,input_buffer,OZY_BUFFER_SIZE*ozy_buffers);
        if (bytes < 0) {
            fprintf(stderr,"ozy_ep6_ep2_io_thread: OzyBulkRead read failed %d\n",bytes);
        } else if (bytes != OZY_BUFFER_SIZE*ozy_buffers) {
            fprintf(stderr,"ozy_ep6_ep2_io_thread: OzyBulkRead only read %d bytes\n",bytes);
        } else {
            // process input buffers
            for(i=0;i<ozy_buffers;i++) {
                process_ozy_input_buffer(&input_buffer[i*OZY_BUFFER_SIZE]);
            }
            if(record) {
                bytes=fwrite(input_buffer,sizeof(char),OZY_BUFFER_SIZE*ozy_buffers,recording);
            }
        }

        //current_receiver++;
        //
        //       if(current_receiver==receivers) {
        //           current_receiver=0;
        //       }
    }
}

void* playback_thread(void* arg) {
    unsigned char input_buffer[OZY_BUFFER_SIZE*ozy_buffers];
    int bytes;
    int i;

    while(1) {
        // read an input buffer (blocks until all bytes read)
        bytes=fread(input_buffer,sizeof(char), OZY_BUFFER_SIZE*ozy_buffers,recording);
        if (bytes <= 0) {
            fclose(recording);
fprintf(stderr,"restarting playback: %s\n",filename);
            recording=fopen(filename,"r");
            bytes=fread(input_buffer,sizeof(char), OZY_BUFFER_SIZE*ozy_buffers,recording);
        }
        // process input buffers
        for(i=0;i<ozy_buffers;i++) {
            process_ozy_input_buffer(&input_buffer[i*OZY_BUFFER_SIZE]);
        }

        current_receiver++;

        if(current_receiver==receivers) {
            current_receiver=0;
        }
        usleep(playback_sleep);
    }
}

void write_ozy_output_buffer_metis() {
    int bytes;

    ozy_output_buffer[0]=SYNC;
    ozy_output_buffer[1]=SYNC;
    ozy_output_buffer[2]=SYNC;

    if(configure>0) {
        configure--;
        ozy_output_buffer[3]=control_out[0];
        ozy_output_buffer[4]=control_out[1];
        ozy_output_buffer[5]=control_out[2];
        ozy_output_buffer[6]=control_out[3];
        ozy_output_buffer[7]=control_out[4];
    } else if(receiver[current_receiver].frequency_changed) {
        //if(receivers==1) {
        //    ozy_output_buffer[3]=control_out[0]|0x02;
        //} else {
        ozy_output_buffer[3]=control_out[0]|((current_receiver+2)<<1);
        //}
        ozy_output_buffer[4]=receiver[current_receiver].frequency>>24;
        ozy_output_buffer[5]=receiver[current_receiver].frequency>>16;
        ozy_output_buffer[6]=receiver[current_receiver].frequency>>8;
        ozy_output_buffer[7]=receiver[current_receiver].frequency;
        receiver[current_receiver].frequency_changed=0;
    } else {
        ozy_output_buffer[3]=control_out[0];
        ozy_output_buffer[4]=control_out[1];
        ozy_output_buffer[5]=control_out[2];
        ozy_output_buffer[6]=control_out[3];
        ozy_output_buffer[7]=control_out[4];
    }

    if(metis || hermes) {
        bytes=metis_write(0x02,ozy_output_buffer,OZY_BUFFER_SIZE);
        if(bytes!=OZY_BUFFER_SIZE) {
            perror("OzyBulkWrite failed");
        }
    } else {
        bytes=ozy_write(0x02,ozy_output_buffer,OZY_BUFFER_SIZE);
        if(bytes!=OZY_BUFFER_SIZE) {
            perror("OzyBulkWrite failed");
        }
    }

    if(tx_frame<10) {
        if(metis || hermes) {
            dump_ozy_buffer("sent to Metis:",tx_frame,ozy_output_buffer);
        } else {
            dump_ozy_buffer("sent to Ozy:",tx_frame,ozy_output_buffer);
        }
    }
    tx_frame++;
    current_receiver++;

    if(current_receiver==receivers) {
        current_receiver=0;
    }

}


void write_ozy_output_buffer_hermes () {

    static int hermes_send_status = 0;

    int bytes;

    ozy_output_buffer[0]=SYNC;
    ozy_output_buffer[1]=SYNC;
    ozy_output_buffer[2]=SYNC;

    if(configure>0) {
        configure--;
        ozy_output_buffer[3]=control_out[0];
        ozy_output_buffer[4]=control_out[1];
        ozy_output_buffer[5]=control_out[2];
        ozy_output_buffer[6]=control_out[3];
        ozy_output_buffer[7]=control_out[4];
    } else {

        switch (hermes_send_status) {
        case 0:
            if(receiver[current_receiver].frequency_changed) {
                // C0
                // 0 0 0 0 0 1 0 x     C1, C2, C3, C4   NCO Frequency in Hz for Receiver_1
                //
                // C0
                // 0 0 0 0 0 1 1 x     C1, C2, C3, C4   NCO Frequency in Hz for Receiver _2 
                //
                // C0
                // 0 0 0 0 1 0 0 x     C1, C2, C3, C4   NCO Frequency in Hz for Receiver _3 
                //
                // C0
                // 0 0 0 0 1 0 1 x     C1, C2, C3, C4   NCO Frequency in Hz for Receiver _4 
                //
                // C0
                // 0 0 0 0 1 1 0 x     C1, C2, C3, C4   NCO Frequency in Hz for Receiver _5 
                //
                // C0
                // 0 0 0 0 1 1 1 x     C1, C2, C3, C4   NCO Frequency in Hz for Receiver _6 
                //
                // C0
                // 0 0 0 1 0 0 0 x     C1, C2, C3, C4   NCO Frequency in Hz for Receiver _7 
                ozy_output_buffer[3]=control_out[0]|((current_receiver+2)<<1);
                ozy_output_buffer[4]=receiver[current_receiver].frequency>>24;
                ozy_output_buffer[5]=receiver[current_receiver].frequency>>16;
                ozy_output_buffer[6]=receiver[current_receiver].frequency>>8;
                ozy_output_buffer[7]=receiver[current_receiver].frequency;
                receiver[current_receiver].frequency_changed=0;

            } else if (mox) {
                //  C0
                //  0 0 0 0 0 0 1 x     C1, C2, C3, C4 NCO Frequency in Hz for Transmitter, Apollo ATU
                //                                     (32 bit binary representation - MSB in C1) 
                ozy_output_buffer[3]=control_out[0]|((1)<<1);
                ozy_output_buffer[4]=receiver[current_receiver].frequency>>24;
                ozy_output_buffer[5]=receiver[current_receiver].frequency>>16;
                ozy_output_buffer[6]=receiver[current_receiver].frequency>>8;
                ozy_output_buffer[7]=receiver[current_receiver].frequency;

            } else {
                ozy_output_buffer[3]=control_out[0];
                ozy_output_buffer[4]=control_out[1];
                ozy_output_buffer[5]=control_out[2];
                ozy_output_buffer[6]=control_out[3];
                ozy_output_buffer[7]=control_out[4];
            }
            hermes_send_status = 1;
            break;

        case 1:
            ozy_output_buffer[3]=control_out_hermes_pow[0];
            ozy_output_buffer[4]=control_out_hermes_pow[1];
            ozy_output_buffer[5]=control_out_hermes_pow[2];
            ozy_output_buffer[6]=control_out_hermes_pow[3];
            ozy_output_buffer[7]=control_out_hermes_pow[4];
            hermes_send_status = 2;
            break;

        case 2:
            ozy_output_buffer[3]=control_out_hermes_att[0];
            ozy_output_buffer[4]=control_out_hermes_att[1];
            ozy_output_buffer[5]=control_out_hermes_att[2];
            ozy_output_buffer[6]=control_out_hermes_att[3];
            ozy_output_buffer[7]=control_out_hermes_att[4];
            hermes_send_status = 0;
            break;

        default:
            hermes_send_status = 0;
        }
    }

    {
        static int rr = 0;
        if (rr & 0x400) {
            dump_ozy_header("HERMES  ", ozy_output_buffer[0], ozy_output_buffer);
            rr = 0;
        }
        rr++ ;
    }

    if (mox) {
        ozy_output_buffer[3] |= 0x01;
    } else {
        ozy_output_buffer[3] &= 0xFE;
    }

    if(metis || hermes) {
        bytes=metis_write(0x02,ozy_output_buffer,OZY_BUFFER_SIZE);
        if(bytes!=OZY_BUFFER_SIZE) {
            perror("OzyBulkWrite failed");
        }
    } else {
        bytes=ozy_write(0x02,ozy_output_buffer,OZY_BUFFER_SIZE);
        if(bytes!=OZY_BUFFER_SIZE) {
            perror("OzyBulkWrite failed");
        }
    }

if(tx_frame<10) {
    if(metis || hermes) {
        dump_ozy_buffer("sent to Metis:",tx_frame,ozy_output_buffer);
    } else {
        dump_ozy_buffer("sent to Ozy:",tx_frame,ozy_output_buffer);
    }
}
    tx_frame++;
    current_receiver++;

    if(current_receiver==receivers) {
        current_receiver=0;
    }

}

void process_ozy_input_buffer(unsigned char* buffer) {
    int b=0;
    int b_max;
    int r;
    int left_sample,right_sample,mic_sample;
    float left_sample_float,right_sample_float,mic_sample_float;

if(rx_frame<10) {
    if(metis || hermes) {
        dump_ozy_buffer("received from Metis:",rx_frame,buffer);
    } else {
        dump_ozy_buffer("received from Ozy:",rx_frame,buffer);
    }
}

    if(buffer[b++]==SYNC && buffer[b++]==SYNC && buffer[b++]==SYNC) {

        // extract control bytes
        control_in[0]=buffer[b++];
        control_in[1]=buffer[b++];
        control_in[2]=buffer[b++];
        control_in[3]=buffer[b++];
        control_in[4]=buffer[b++];

        // extract PTT, DOT and DASH
        ptt=(control_in[0]&0x01)==0x01;
        dash=(control_in[0]&0x02)==0x02;
        dot=(control_in[0]&0x04)==0x04;

        switch((control_in[0]>>3)&0x1F) {
        // C1 
        // 0 0 0 0 0 0 0 0
        //   | | | | | | |
        //   | | | | | | +---------- LT2208 Overflow (1 = active, 0 = inactive)
        //   | | | | | +------------ Hermes I01 (0 = active, 1 = inactive)
        //   | | | | +-------------- Hermes I02 (0 = active, 1 = inactive)
        //   | | | +---------------- Hermes I03 (0 = active, 1 = inactive)
        //   | | +------------------ Hermes I04 (0 = active, 1 = inactive)
        //   | +-------------------- Cyclops PLL locked (0 = unlocked, 1 = locked)
        //   +---------------------- Cyclops - Mercury frequency changed, bit toggles   
        //                  
        case 0:
            lt2208ADCOverflow=control_in[1]&0x01;
            IO1=(control_in[1]&0x02)?0:1;
            IO2=(control_in[1]&0x04)?0:1;
            IO3=(control_in[1]&0x08)?0:1;
            IO4=(control_in[1]&0x10)?0:1;
            if(mercury_software_version!=control_in[2]) {
                mercury_software_version=control_in[2];
                fprintf(stderr,"  Mercury Software version: %d (0x%0X)\n",mercury_software_version,mercury_software_version);
            }
            if(penelope_software_version!=control_in[3]) {
                penelope_software_version=control_in[3];
                fprintf(stderr,"  Penelope Software version: %d (0x%0X)\n",penelope_software_version,penelope_software_version);
            }
            if (hermes) {
                if(hermes_software_version!=control_in[4]) {
                    hermes_software_version=control_in[4];
                    fprintf(stderr,"  Hermes Software version: %d (0x%0X)\n",hermes_software_version,hermes_software_version);
                }
            } else {
                if(ozy_software_version!=control_in[4]) {
                    ozy_software_version=control_in[4];
                    fprintf(stderr,"  Ozy Software version: %d (0x%0X)\n",ozy_software_version,ozy_software_version);
                }
            }
            break;
/*
 *          C0
 *          0 0 0 0 1 x x x    
 *
 *          C1 - Bits 15-8 of Forward Power from Penelope or Hermes* (AIN5)
 *          C2 - Bits 7-0  of Forward Power from Penelope or Hermes* (AIN5)
 *          C3 - Bits 15-8 of Forward Power from Alex or Apollo*(AIN1)
 *          C4 - Bits 7-0  of Forward Power from Alex or Apollo*(AIN1)
 *
 *          Note: All analog levels are 12 bits.
 */
        case 1:
            forwardPower=(control_in[1]<<8)+control_in[2]; // from Penelope or Hermes
            alexForwardPower=(control_in[3]<<8)+control_in[4]; // from Alex or Apollo
            break;
/*
 *          C0
 *          0 0 0 1 0 x x x    
 *
 *          C1 - Bits 15-8 of Reverse Power from Alex or Apollo*(AIN2)
 *          C2 - Bits 7-0  of Reverse Power from Alex or Apollo*(AIN2)
 *          C3 - Bits 15-8 of AIN3 from Penny or Hermes*
 *          C4 - Bits 7-0  of AIN3 from Penny or Hermes*
 *
 *          Note: All analog levels are 12 bits.
 */
        case 2:
            alexReversePower=(control_in[1]<<8)+control_in[2]; // from Alex or Apollo
            AIN3=(control_in[3]<<8)+control_in[4]; // from Penelope or Hermes
            break;
/*
 *          C0
 *          0 0 0 1 1 x x x    
 *
 *          C1 - Bits 15-8 of AIN4 from Penny or Hermes*
 *          C2 - Bits 7-0  of AIN4 from Penny or Hermes*
 *          C3 - Bits 15-8 of AIN6,13.8v supply on Hermes*
 *          C4 - Bits 7-0  of AIN6,13.8v supply on Hermes*
 *
 *          *Note: All analog levels are 12 bits.
 */
        case 3:
            AIN4=(control_in[1]<<8)+control_in[2]; // from Penelope or Hermes
            AIN6=(control_in[3]<<8)+control_in[4]; // from Penelope or Hermes
            break;
        }

        switch(receivers) {
            case 1: b_max=512-0; break;
            case 2: b_max=512-0; break;
            case 3: b_max=512-4; break;
            case 4: b_max=512-10; break;
            case 5: b_max=512-24; break;
            case 6: b_max=512-10; break;
            case 7: b_max=512-20; break;
            case 8: b_max=512-4; break;
            default: assert(0); break;
        }

        // extract the samples
        while(b<b_max) {
            // extract each of the receivers
            for(r=0;r<receivers;r++) {
                left_sample   = (int)((signed char)buffer[b++]) << 16;
                left_sample  += (int)((unsigned char)buffer[b++]) << 8;
                left_sample  += (int)((unsigned char)buffer[b++]);
                right_sample  = (int)((signed char)buffer[b++]) << 16;
                right_sample += (int)((unsigned char)buffer[b++]) << 8;
                right_sample += (int)((unsigned char)buffer[b++]);
                left_sample_float=(float)left_sample/8388607.0; // 24 bit sample
                right_sample_float=(float)right_sample/8388607.0; // 24 bit sample
                receiver[r].input_buffer[samples]=left_sample_float;
                receiver[r].input_buffer[samples+BUFFER_SIZE]=right_sample_float;
            }
                // send to dspserver
                mic_sample    = (int)((signed char) buffer[b++]) << 8;
                mic_sample   += (int)((unsigned char)buffer[b++]);
                if (hermes) {
                    mic_sample_float=(float)mic_sample/32767.0 ;           // 16 bit sample
                } else 
                    mic_sample_float=(float)mic_sample/32767.0 * mic_gain; // 16 bit sample
                receiver[r].input_buffer[samples+BUFFER_SIZE+BUFFER_SIZE]=mic_sample_float;

/*
            mic_sample    = (int)((signed char) buffer[b++]) << 8;
            mic_sample   += (int)((unsigned char)buffer[b++]);
            mic_sample_float=(float)mic_sample/32767.0*mic_gain; // 16 bit sample

            // add to buffer
            mic_left_buffer[samples]=mic_sample_float;
            mic_right_buffer[samples]=0.0f;
*/
            samples++;

            if(timing) {
                rx_sample_count++;
                if(rx_sample_count==sample_rate) {
                    ftime(&rx_end_time);
                    fprintf(stderr,"%d rx samples in %ld ms\n",rx_sample_count,((rx_end_time.time*1000)+rx_end_time.millitm)-((rx_start_time.time*1000)+rx_start_time.millitm));
                    rx_sample_count=0;
                    ftime(&rx_start_time);
                }
            }


            // when we have enough samples send them to the clients
            if(samples==BUFFER_SIZE) {
                //if(ptt||mox) {
                //    process_microphone_samples(mic_left_buffer);
                //}
                // send I/Q data to clients
                for(r=0;r<receivers;r++) {
                    send_IQ_buffer(r);
                }
                samples=0;
            }
        }

    } else {
        fprintf(stderr,"SYNC error\n");
        dump_ozy_buffer("SYNC ERROR",rx_frame,buffer);
        exit(1);
    }


    rx_frame++;
}

void* ozy_ep4_io_thread(void* arg) {
    unsigned char buffer[BANDSCOPE_BUFFER_SIZE*2];
    int bytes;
//    int i;

    while(1) {
        bytes=ozy_read(0x84,(void*)(bandscope.buffer),sizeof(buffer));
        if (bytes < 0) {
            fprintf(stderr,"ozy_ep4_io_thread: OzyBulkRead failed %d bytes\n",bytes);
            exit(1);
        } else if (bytes != BANDSCOPE_BUFFER_SIZE*2) {
            fprintf(stderr,"ozy_ep4_io_thread: OzyBulkRead only read %d bytes\n",bytes);
            exit(1);
        } else {
            // process the buffer
            process_bandscope_buffer(buffer);
        }
    }
}


void process_bandscope_buffer(unsigned char* buffer) {
    int b=0;
    int sample;
    float sample_float;
    int i;

    for(i=0;i<BANDSCOPE_BUFFER_SIZE;i++) {
        sample    = (int)((signed char) buffer[b++]) << 8;
        sample   += (int)((unsigned char)buffer[b++]);
        sample_float=(float)sample/32767.0; // 16 bit sample
        bandscope.buffer[i]=sample_float;
//fprintf(stderr,"%d: %d %f\n",i,sample,sample_float);
    }

    send_bandscope_buffer();
}

void process_ozy_output_buffer(float *left_output_buffer,float *right_output_buffer,
                               float *left_tx_buffer,float *right_tx_buffer,int mox_state) {
    //unsigned char ozy_samples[1024*8];
    int j,c;
    short left_rx_sample;
    short right_rx_sample;
    short left_tx_sample;
    short right_tx_sample;

    mox=mox_state;
    if(mox) {
        control_out[0]|=0x01;
    } else {
        control_out[0]&=0xFE;
    }

    if(!playback) {
        //
        // process the output 
        // skipping all the samples more that 48 kS/s
        // 
        for(j=0,c=0;j<BUFFER_SIZE;j+=output_sample_increment) {

            if(mox) {
                left_rx_sample=0.0;
                right_rx_sample=0.0;
                left_tx_sample=(short)(left_tx_buffer[j]*32767.0);
                right_tx_sample=(short)(right_tx_buffer[j]*32767.0);
            } else {
                left_rx_sample=(short)(left_output_buffer[j]*32767.0);
                right_rx_sample=(short)(right_output_buffer[j]*32767.0);
                left_tx_sample=0.0;
                right_tx_sample=0.0;
            }

/*
            if(mox) {
                left_tx_sample=(short)(left_output_buffer[j]*32767.0);
                right_tx_sample=(short)(right_output_buffer[j]*32767.0);
//fprintf(stderr,"TX left=%d right=%d\n",left_tx_sample,right_tx_sample);
fprintf(stderr,"TX left=%8d right=%8d\r",left_tx_sample,right_tx_sample);
                left_rx_sample=0;
                right_rx_sample=0;
            } else {
                left_rx_sample=(short)(left_output_buffer[j]*32767.0);
                right_rx_sample=(short)(right_output_buffer[j]*32767.0);
//fprintf(stderr,"RX left=%d right=%d\n",left_rx_sample,right_rx_sample);
                left_tx_sample=0;
                right_tx_sample=0;
            }
*/
            ozy_output_buffer[ozy_output_buffer_index++]=left_rx_sample>>8;
            ozy_output_buffer[ozy_output_buffer_index++]=left_rx_sample;
            ozy_output_buffer[ozy_output_buffer_index++]=right_rx_sample>>8;
            ozy_output_buffer[ozy_output_buffer_index++]=right_rx_sample;
            ozy_output_buffer[ozy_output_buffer_index++]=left_tx_sample>>8;
            ozy_output_buffer[ozy_output_buffer_index++]=left_tx_sample;
            ozy_output_buffer[ozy_output_buffer_index++]=right_tx_sample>>8;
            ozy_output_buffer[ozy_output_buffer_index++]=right_tx_sample;

            if(timing) {
                tx_sample_count++;
                if(tx_sample_count==sample_rate/output_sample_increment) {
                    ftime(&tx_end_time);
                    fprintf(stderr,"%d tx samples in %ld ms\n",tx_sample_count,((tx_end_time.time*1000)+tx_end_time.millitm)-((tx_start_time.time*1000)+tx_start_time.millitm));
                    tx_sample_count=0;
                    ftime(&tx_start_time);
                }
            }

            if(ozy_output_buffer_index==OZY_BUFFER_SIZE) {
                write_ozy_output_buffer();
                ozy_output_buffer_index=OZY_HEADER_SIZE;
            }
        }
    }

}

void ozy_set_preamp(int p) {
    control_out[3]=control_out[3]&0xFB;
    control_out[3]=control_out[3]|(p<<2);
    fprintf(stderr,"%s: %d 0x%02X\n", __FUNCTION__, p, control_out[3]);
}

void ozy_set_dither(int dither) {
    control_out[3]=control_out[3]&0xF7;
    control_out[3]=control_out[3]|(dither<<3);
}

void ozy_set_random(int random) {
    control_out[3]=control_out[3]&0xEF;
    control_out[3]=control_out[3]|(random<<4);
}

void ozy_set_10mhzsource(int source) {
    control_out[1]=control_out[1]&0xF3;
    control_out[1]=control_out[1]|(source<<2);
}

void ozy_set_122_88mhzsource(int source) {
    control_out[1]=control_out[1]&0xEF;
    control_out[1]=control_out[1]|(source<<4);
}

void ozy_set_micsource(int source) {
    control_out[1]=control_out[1]&0x7F;
    control_out[1]=control_out[1]|(source<<7);
}

void ozy_set_class(int c) {
    control_out[2]=control_out[2]&0xFE;
    control_out[2]=control_out[2]|c;
}

void ozy_set_timing(int t) {
    timing=t;
}

void ozy_set_open_collector_outputs(int oc) {
    control_out[2]=control_out[2]&0x01;
    control_out[2]=control_out[2]|(oc<<1);
}

void ozy_set_hermes_power(unsigned char p) {
    control_out_hermes_pow[1]=p;    
}

void ozy_set_hermes_mic_boost(int b)
{
    control_out_hermes_pow[2]=control_out_hermes_pow[2]&0xFE;
    control_out_hermes_pow[2]=control_out_hermes_pow[2]|(b & 0x01);
    fprintf(stderr,"%s: %d: 0x%02X\n", __FUNCTION__, b, control_out_hermes_pow[2]);
}

void ozy_set_hermes_lineingain(unsigned char p)
{
    control_out_hermes_att[2]=control_out_hermes_att[2]&0xE0;
    control_out_hermes_att[2]=control_out_hermes_att[2] | (p & 0x1F);
    fprintf(stderr,"%s: %d 0x%02X\n", __FUNCTION__, p, control_out_hermes_att[2]);
}

void ozy_set_hermes_linein(unsigned char p)
{
    control_out_hermes_pow[2]=control_out_hermes_pow[2]&0xFD;
    control_out_hermes_pow[2]=control_out_hermes_pow[2]|((p<<1) & 0x02);
    fprintf(stderr,"%s: %d: 0x%02X\n", __FUNCTION__, p, control_out_hermes_pow[2]);
}

void ozy_set_hermes_att(int av) {
    control_out_hermes_att[4] = (av & 0x1f) | 0x20;    
    fprintf(stderr,"%s: 0x%02X\n", __FUNCTION__, control_out_hermes_att[4]);
}


int ozy_get_hermes_sw_ver(void)
{
    return hermes_software_version;
}

void ozy_set_alex_rx_att (unsigned int a)
{
    a = a & 0x03;
    control_out[3]=control_out[3]&0xFC;
    control_out[3]=control_out[3]|(a);
}

void ozy_set_alex_rx_antenna (unsigned int a)
{
    a = a & 0x03;
    control_out[3]=control_out[3]&0x9F;
    control_out[3]=control_out[3]|(a<<5);

}


void ozy_set_alex_tx_relay (unsigned int t)
{
    control_out[4]=control_out[4]&0xFC;
    control_out[4]=control_out[4]|(t & 0x03);
    fprintf(stderr,"%s: %d: 0x%02X\n", __FUNCTION__, t, control_out[4]);
}


int  ozy_get_adc_overflow (void)
{
    return lt2208ADCOverflow;
}
