/**
* @file hiqsdr.cpp
* @brief HiqSDR server application
* @author Andrea Montefusco, IW0HDV
* @version 1.0
* @date January 2012
*/


/* Copyright (C)
* 2012 - Andrea Montefusco, IW0HDV
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
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
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <vector>
#include "hiqsdr.h"


struct AsynchCtxData {
    void *pUserData;
    HIQSDR_CB udcf ;
    int run;
};

struct PreselItem { // Holds item data from the hiqsdr.config file
	long long freq;
	unsigned int filtNum;
    char *desc;
};

struct Hiqsdr {
    // network parameters
    char ip_addr[256];
    unsigned short ctrl_port;
    unsigned short rx_data_port;
    unsigned short tx_data_port;
    int            ctrl_socket;
    int            data_socket;

    // radio state
    int  fwv;
    long long freq;
    long long bw;
    int  attDb;
    int  antSel;
    bool Xfilters; // Use the 80/40 extended filters
    unsigned int  preSel; // Holds filter number
	std::vector<PreselItem> preInfo; //Holds presel filters table
    int  preamp;

    // asynch thread for receiving data from hardware
    pthread_t      thread_id;
    AsynchCtxData *pacd;
};

// module variables
static struct Hiqsdr hq;
static int init = 0;


// module private utility methods

static int   send_command (struct Hiqsdr *hiq);
static int   send_activation (struct Hiqsdr *hiq);
static int   send_deactivation (struct Hiqsdr *hiq);
static int   ping_hardware (struct Hiqsdr *hiq, int tmo_s = 1);
static void  dump_buffer(unsigned char* buffer, int len /* bytes to dump */) ;
static int   get_data (struct Hiqsdr *hiq, unsigned char *buffer, int buf_len);
static int   open_configuration_file (struct Hiqsdr *hiq);


int hiqsdr_init (const char*hiqsdr_ip, int hiqsdr_bw, long long hiqsdr_f)
{
   if (init) { 
       fprintf (stderr, "%s: WARNING: attempting to double init !\n", __FUNCTION__);
       return -1;
   }
   
   // network data configuration
   strcpy (hq.ip_addr, hiqsdr_ip);
   hq.freq = hiqsdr_f;
   hq.bw = hiqsdr_bw;
   hq.attDb  = 0;
   hq.antSel = 0;
   hq.preSel = 0;
   hq.preamp = 0;
   hq.Xfilters = false;
   hq.rx_data_port = 48247;
   hq.ctrl_port    = hq.rx_data_port+1;   
   hq.tx_data_port = hq.rx_data_port+2;

   open_configuration_file (&hq);

   // setup control socket
   hq.ctrl_socket = socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
   if(hq.ctrl_socket<0) {
       perror("create socket failed for control socket\n");
       return -1;
   }
   // try to connect it (the TCP/IP stack will made only static checks)
   struct sockaddr_in ctrl_addr;
   int ctrl_length = sizeof(ctrl_addr);

   memset(&ctrl_addr,0,ctrl_length);
   ctrl_addr.sin_family = AF_INET;
   inet_pton (AF_INET, hq.ip_addr, &ctrl_addr.sin_addr);
   ctrl_addr.sin_port = htons(hq.ctrl_port);
   int rc = connect (hq.ctrl_socket, (struct sockaddr*)&ctrl_addr, ctrl_length);
   if (rc < 0) {
        perror("connect socket failed for control socket\n");
        return -1;
   }

   // setup data socket
   hq.data_socket = socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
   if(hq.data_socket<0) {
       perror("create socket failed for data socket\n");
       return -1;
   }

   struct sockaddr_in cmd_addr;
   int cmd_length = sizeof(cmd_addr);

   memset(&cmd_addr,0,cmd_length);
   cmd_addr.sin_family = AF_INET;
   cmd_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   cmd_addr.sin_port = htons(hq.rx_data_port);

   if(bind(hq.data_socket,(struct sockaddr*)&cmd_addr,cmd_length)<0) {
       perror("bind socket failed for data socket");
       return -1;
   }
   // try to connect it (the TCP/IP stack will made only static checks)
   struct sockaddr_in data_addr;
   int data_length = sizeof(data_addr);

   memset(&data_addr,0,data_length);
   data_addr.sin_family = AF_INET;
   inet_pton (AF_INET, hq.ip_addr, &data_addr.sin_addr);
   data_addr.sin_port = htons(hq.rx_data_port);
   rc = connect (hq.data_socket, (struct sockaddr*)&data_addr, data_length);
   if (rc < 0) {
        perror("connect socket failed for data socket\n");
        return -1;
   }
   // end of network configuration

   // asynch reception thread data
   hq.pacd      = 0;
   hq.thread_id = 0;

   //
   // check the network reachability of hardware
   // the hardware is left in not active status
   // further command needs in order to receive data
   //
   int ping = ping_hardware (&hq);
   if (ping != 0) {
       fprintf (stderr, "Reachability test failed with error: %d\n", ping);
       hiqsdr_deinit ();
       return -1;
   } else {
       init = 1;
       return 0;
   }

}

int hiqsdr_connect ()
{
    int rc = send_activation (&hq);

    return rc;
}

int hiqsdr_disconnect ()
{
    int rc = send_deactivation (&hq);

    return rc;
}

char *hiqsdr_get_ip_address ()
{
    return hq.ip_addr;
}

int hiqsdr_set_frequency (long long f)
{
   unsigned int cnt = 0;
	
   fprintf (stderr, "%s: %Ld\n", __FUNCTION__, f);
   hq.freq = f;
	
	// Check to see if freq change caused a band filter change.
	for (unsigned int x=0; x<(hq.preInfo.size()-1); ++x) {
		if ((f >= hq.preInfo[x].freq) & (f < hq.preInfo[x+1].freq)){
			cnt = x; // cnt indexes the filter associated with this frequency.
			if (!hq.Xfilters) break; 
		}
	}
	// If freq change caused a filter change then store new filter details.
   if (hq.preSel != hq.preInfo[cnt].filtNum) {
		 hiqsdr_set_preselector(hq.preInfo[cnt].filtNum);
	}	// TODO check and see if send_command sends all commands, maybe
		// we dont need to specifically set the preselector??
   send_command (&hq);
   return 0;
}

int hiqsdr_set_bandwidth (long long b)
{
   fprintf (stderr, "%s: %Ld\n", __FUNCTION__, b);
   hq.bw = b;
   send_command (&hq);
   return 0;
}

/**
 *
 *  3.9 Attenuator setting [15]
 *   
 *  The byte 15 controls the attenuator pins 84, 83, 82, 81, 80. Other bits are
 *  left 0. The attenuator RF2420 (RF Micro Devices) is available at the HiQSDR
 *  PCB and can be used to reduce the input power level to the onboard LNA. The
 *  stepsize is 2dB in the range of 0..44dB plus 4dB insertion loss. The stages are
 *  Bypass, 2dB, 4dB, 8dB, 10dB, 20dB. 
 *
 *  Mapping of control byte 15:
 *
 *     Bit  attenuation stage Location/Name
 *      
 *     0x01      2dB          P84/ATT2dB
 *     0x02      4dB          P83/ATT4dB
 *     0x04      8dB          P82/ATT8dB
 *     0x08     10dB          P81/ATT10dB
 *     0x10     20dB          P80/ATT20dB
 *     0xE0        -          not used
 *      
 *  Please notice that a controlable preamplier is not included on the frontend
 *  PCB. A external preamp can be controlled via X1.
 *
 */

int convert_attenuator (int attDb)
{
    int out = 0;

    if ((attDb - 20) >= 0) {
        out |= 0x10;
        attDb -= 20;
    }
    if ((attDb - 10) >= 0) {
        out |= 0x08;
        attDb -= 10;
    }
    if ((attDb - 8) >= 0) {
        out |= 0x04;
        attDb -= 8;
    }
    if ((attDb - 4) >= 0) {
        out |= 0x02;
        attDb -= 4;
    }
    if ((attDb - 2) >= 0) {
        out |= 0x01;
        attDb -= 2;
    }
    return out;
}

int hiqsdr_set_attenuator (int attDb)
{
   hq.attDb = convert_attenuator (attDb);
   fprintf (stderr, "%s: requested value: %d computed value: %02X\n", __FUNCTION__, attDb, hq.attDb);
   send_command (&hq);
   return 0;
}

int hiqsdr_set_antenna_input (int n)
{
    hq.antSel = n == 0 ? 0x00 : 0x01;
    fprintf (stderr, "%s: antenna: %02X\n", __FUNCTION__, hq.antSel);
    send_command (&hq);
    return 0;
}


/*
  def ChangeBand(self, band):
    # band is a string: "60", "40", "WWV", etc.
    self.band = band
    self.HiQSDR_Connector_X1 &= ~0x0F   # Mask in the last four bits
    self.HiQSDR_Connector_X1 |= self.conf.HiQSDR_BandDict.get(band, 0) & 0x0F
    self.SetTxLevel()
    self.NewUdpStatus()

The BandDict is set up in ~/.quisk_conf.py with
bandLabels = ['Audio', '160', '80', '40', '30', '20', '17', '15',
        '12', '10', '6', ('Time',) * len(bandTime)] 
*/

int hiqsdr_set_preselector (int p)
{
    hq.preSel = (p & 0x0F);
    fprintf (stderr, "%s: preselector: %02X\n", __FUNCTION__, hq.preSel);
    send_command (&hq);
    return 0;
}


int hiqsdr_get_preselector_desc (unsigned int p, char *pDesc)
{
	unsigned int cnt;
	unsigned int tableSize = hq.preInfo.size();

	// Retrieve the index to the filter number
	for (cnt = 0; cnt < tableSize; ++cnt) {
		if (p == hq.preInfo[cnt].filtNum) break;
	}
	if (cnt < tableSize) {
		strcpy (pDesc, hq.preInfo[cnt].desc);
		return 0;
	} else
		return -1;
}

int hiqsdr_set_preamp (int newstatus)
{
    if (newstatus != 0) hq.preamp = 1;
                   else hq.preamp = 0;
    fprintf (stderr, "%s: preamplifier: %02X\n", __FUNCTION__, hq.preamp);
    send_command (&hq);
    return 0;
}

int hiqsdr_get_preamp (void)
{ 
    return hq.preamp;
}

int hiqsdr_deinit (void)
{
   if (init) hiqsdr_disconnect ();
   close (hq.ctrl_socket);
   close (hq.data_socket);
   init = 0;
   return 0;
}

static void *asynch_input_thread (void *p)
{
    AsynchCtxData *pacd = (AsynchCtxData *)p;

    while (pacd->run) {
        unsigned char buffer [10240];
        int buf_len = get_data (&hq, buffer, sizeof(buffer));

        if (buf_len > 0) {
           int rcb = pacd->udcf ((void *)buffer, buf_len, pacd->pUserData);
           if (rcb < 0) break;  // user callback asked for termination
        } else {
            pacd->udcf ((void *)0, 0, 0);
            break;
        }
    }
    return 0;
}

int hiqsdr_start_asynch_input (HIQSDR_CB cb, void *pud)
{
    int rc;

    if (hq.pacd) delete hq.pacd;

    hq.pacd = new AsynchCtxData;

    hq.pacd->pUserData = pud;
    hq.pacd->udcf = cb;
    hq.pacd->run = 1;

    // create the thread to receive data
    rc = pthread_create(&(hq.thread_id),NULL,asynch_input_thread,(void *)(hq.pacd));
    if(rc < 0) {
        perror("pthread_create asynch_input_thread failed");
    } else {
    }
    return rc;
    
}


int hiqsdr_stop_asynch_input ()
{
    int rc = -1;

    if (hq.thread_id != 0) {
        void *pExit;

        hq.pacd->run = 0;
        rc = pthread_join (hq.thread_id, &pExit);
    }

    hiqsdr_disconnect ();
    return rc;
}

/*
 *
 *  This provides the following samplerates. Of cause only the rates that are
 *  multiple of 48kHz audio rate may make sense.
 *  Decimation decimal decimation hexadecimal Samplerate
 *
 *  1  0x1  1920000
 *  2  0x2  960000
 *  4  0x4  480000
 *  6  0x6  320000
 *  8  0x8  240000
 *  10 0xA  192000
 *  12 0xC  160000
 *  16 0xF  120000
 *  20 0x14 96000
 *  24 0x18 80000
 *  30 0x1E 64000
 *  32 0x20 60000
 *  40 0x28 48000
 *
 */

struct BwTbl {
    int bw;
    unsigned char code;
};


const struct BwTbl bw [] = { 
  { 1920000, 0x1   },
  { 960000 , 0x2   },
  { 480000 , 0x4   },
  { 320000 , 0x6   },
  { 240000 , 0x8   },
  { 192000 , 0xA   },
  { 160000 , 0xC   },
  { 120000 , 0xF   },
  { 96000  , 0x14  },
  { 80000  , 0x18  },
  { 64000  , 0x1E  },
  { 60000  , 0x20  },
  { 48000  , 0x28  }
};          

static unsigned char get_decimation (int bw_hz)
{
    for (unsigned i=0; i < ARRAY_SIZE(bw); ++i) 
        if (bw[i].bw == bw_hz)
           return bw[i].code;
    
    return 0x28;
}

static const double clockrate = 122880000.0 ;

/*
 *  3.2 RX tune phase setting [2,3,4,5]
 *
 *  The RXPhase setting contains 4 bytes. These are calculated from the receiver
 *  frequency setting.
 *
 *  RXPhase = ( RXFrequency / ReferenceClock ) * 2^32 + 0:5 
 *
 *  The bytes need to be aligned with least signicant byte rst. The clockrate
 *  is 122880000 Hz. The RX frequency results in the frequency of the signal that
 *  should be received and the mode that is selected.
 */
static inline unsigned long compute_phase (long long int freq)
{
    return ((freq / clockrate) * (1LL << 32)) + 0.5;
}



/*
 *  Send the activation code to hardware
 *
 *
 *  7.1 Setting the sample destination address
 *
 *  To register your PCs IP address at the frontend for receiving the samples it is
 *  necessary to send a UDP frame with the content of 0x72, 0x72 (two times 0x72).
 *
 *  When the frontend receives this message it will afterwards automatically send
 *  all UDP messages with received samples to this destination. Sending 0x73, 0x73
 *  will cause the hardware to stop sending samples. In both cases the port used is
 *  the same as the Rx sample port.
 *
 *  The IP and Ethernet (MAC) address of the hardware is compiled into the
 *  FPGA program. The IP address should be chosen to be valid on the network.
 *  The Ethernet address must be unique on the network. The default Ethernet
 *  address will work unless there are two frontend on the same network.
 *
 *
 */

static int send_activation (struct Hiqsdr *hiq) {
    unsigned char buffer [] = { 0x72, 0x72 };
    int rc = -1;

    // send the activation buffer
    rc = send (hiq->data_socket, (char*)&buffer, sizeof(buffer), 0);
    if(rc <= 0) {
        perror("sendto failed for activation data");
    } 
    return rc;
}

static int send_deactivation (struct Hiqsdr *hiq) {
    unsigned char buffer [] = { 0x73, 0x73 };
    int rc;

    // send the deactivation buffer
    rc = send (hiq->data_socket, (char*)&buffer, sizeof(buffer), 0);
    if(rc <= 0) {
        perror("sendto failed for deactivation data");
    } 
    return rc;
}



// get audio from a client
static int get_data (struct Hiqsdr *hiq, unsigned char *buffer, int buf_len)
{
    int 
    bytes_read = recv (hiq->data_socket,
                       buffer,
                       buf_len,
                       0
                      );
    if(bytes_read < 0) {
        perror("recv socket failed for audio buffer");
        return 0;
    } else {
        return bytes_read;
    }
}

// create/open/read a preselector configuration file
static int   open_configuration_file (struct Hiqsdr *hiq)
{
    static char  fn [BUFSIZ];
    FILE *fc;
    int nr = 0;

    sprintf (fn, "%s/%s", getenv("HOME"), "hiqsdr.cfg");

    if (( fc = fopen (fn, "r")) == NULL) {
		fprintf (stderr, "No configuration file found in %s\n", fn);
		createConfigFile(fc, fn);
	}
	if (( fc = fopen (fn, "r")) != NULL) {
        while (!feof(fc)) {
            char line [BUFSIZ];
			long long f;
            int n;
            char pd [BUFSIZ];

            if (fgets (line, sizeof(line), fc)) {
                if (line[0] == '#') continue;
                if (sscanf(line, "%d!%lld!%[^\t\n]\n", &n, &f, pd) == 3) {
                    PreselItem temp;
					temp.filtNum = n;
    				temp.freq = f;
					temp.desc = new char [strlen(pd)];
					strcpy (temp.desc, pd);
					n = n;
					hiq->preInfo.push_back(temp);
					nr++;
                }
            }         
        }
    } else { // Can't open file for read. Do error thing.
		perror ("The following error occurred");
		return 0;
    }
    if (nr) { // Print the contents of the preInfo vector (struct Hiqsdr)
		printf("Idx  Filt     Freq     Description\n");
		printf("-----------------------------------\n");
        for (unsigned j=0; j<hiq->preInfo.size(); ++j) {
            printf("%*d %*d %*lld   %s\n",2,j, 5,hiq->preInfo[j].filtNum,
                   10,hiq->preInfo[j].freq, hiq->preInfo[j].desc);
        }
    }
    return nr;
}

static int createConfigFile(FILE *fc, const char *fn)
{
	int nr;
	
    if ((fc = fopen (fn, "w+"))) {
        fprintf (fc, "#\n");
        fprintf (fc, "# HiQSDR preselector configuration file template\n");
        fprintf (fc, "#\n");
        fprintf(fc, "%d!%d!%s\n", 0, 0, "FILTER BYPASS");
		fprintf(fc, "%d!%d!%s\n", 1, 1800000, "160 M");
		fprintf(fc, "%d!%d!%s\n", 0, 2000000, "FILTER BYPASS");
		fprintf(fc, "%d!%d!%s\n", 2, 3500000, "80 M");
		fprintf(fc, "%d!%d!%s\n", 0, 4000000, "FILTER BYPASS");
		fprintf(fc, "%d!%d!%s\n", 3, 7000000, "40 M");
		fprintf(fc, "%d!%d!%s\n", 0, 7300000, "FILTER BYPASS");
		fprintf(fc, "%d!%d!%s\n", 4, 10100000, "30M");
		fprintf(fc, "%d!%d!%s\n", 0, 10150000, "FILTER BYPASS");
		fprintf(fc, "%d!%d!%s\n", 5, 14000000, "20 M");
		fprintf(fc, "%d!%d!%s\n", 0, 14350000, "FILTER BYPASS");
		fprintf(fc, "%d!%d!%s\n", 6, 18068000, "17 M");
		fprintf(fc, "%d!%d!%s\n", 0, 18168000, "FILTER BYPASS");
		fprintf(fc, "%d!%d!%s\n", 7, 21000000, "15 M");
		fprintf(fc, "%d!%d!%s\n", 0, 21450000, "FILTER BYPASS");
		fprintf(fc, "%d!%d!%s\n", 8, 24890000, "12 M");
		fprintf(fc, "%d!%d!%s\n", 0, 24990000, "FILTER BYPASS");
		fprintf(fc, "%d!%d!%s\n", 9, 28000000, "10 M");
		fprintf(fc, "%d!%d!%s\n", 0, 29700000, "FILTER BYPASS");
		fprintf(fc, "%d!%d!%s\n", 10, 51000000, "6 M");
		fprintf(fc, "%d!%d!%s\n", 0, 54000000, "FILTER BYPASS");
		fprintf(fc, "%d!%d!%s\n", 11, 3500000, "80 Xm");
		fprintf(fc, "%d!%d!%s\n", 0, 4000000, "FILTER BYPASS");
		fprintf(fc, "%d!%d!%s\n", 12, 7000000, "40 Xm");
		fprintf(fc, "%d!%d!%s\n", 0, 7200000, "FILTER BYPASS");		
		fclose (fc);
        fprintf (stderr, "Configuration file created at: %s\n", fn);
		nr = 25;
    } else {
		perror ("The following error occurred");
	}
	return nr;
}

/*
 * Send command message to hardware


    want_udp_status is a 14-byte string with numbers in little-endian order:
    [0:2]  'St'
    [2:6]  Rx tune phase
    [6:10] Tx tune phase
    [10]   Tx output level 0 to 255
    [11]   Tx control bits:
        0x01 Enable CW transmit
        0x02 Enable all other transmit
        0x04 Use the HiQSDR extended IO pins not present in
             the 2010 QEX ver 1.0
        0x08 The key is down (software key)
    [12]   Rx control bits
           Second stage decimation less one, 1-39, six bits
    [13]   zero or firmware version number
           The above is used for firmware version 1.0; add eight more
           bytes for version 1.1:
    [14]   X1 connector: Preselect pins 69, 68, 65, 64;
           Preamp pin 63, Tx LED pin 57
    [15]   Attenuator pins 84, 83, 82, 81, 80
    [16]   More bits: AntSwitch pin 41 is 0x01
           Remaining five bytes are sent as zero.
           For version 1.2 include the VNA scan count:

    [17] .. [21] Remaining five bytes are sent as zero.\Uffffffff

 */

typedef struct __attribute__((__packed__)) {
     /*  1 */   uint8_t         s;
     /*  2 */   uint8_t         t;

  union __attribute__((__packed__)) {
      struct __attribute__((__packed__)) {
     /*  3 */   uint8_t         rxp1;
     /*  4 */   uint8_t         rxp2;
     /*  5 */   uint8_t         rxp3;
     /*  6 */   uint8_t         rxp4;
      } rxf_b;
      struct __attribute__((__packed__)) {
         uint32_t rxf_l;
      };
  };
  union __attribute__((__packed__)) {
      struct __attribute__((__packed__)) {
     /*  7 */   uint8_t         txp1;
     /*  8 */   uint8_t         txp2;
     /*  9 */   uint8_t         txp3;
     /* 10 */   uint8_t         txp4;
      } txf_b;
      struct __attribute__((__packed__)) {
         uint32_t txf_l;
      };
  };

     /* 11 */   uint8_t         txl;
     /* 12 */   uint8_t         txc;
     /* 13 */   uint8_t         rxc;

     /* 14 */   uint8_t         fwv;

     /* 15 */   uint8_t         x1;
     /* 16 */   uint8_t         att;
     /* 17 */   uint8_t         msc;


     /* 18 */   uint8_t         rfu1;
     /* 19 */   uint8_t         rfu2;
     /* 20 */   uint8_t         rfu3;
     /* 21 */   uint8_t         rfu4;
     /* 22 */   uint8_t         rfu5;
  
} control_msg;


static int send_command (struct Hiqsdr *hiq) {

    static control_msg m = { 0 };
    int rc;

    m.s = 'S', m.t = 't'; // Marks the beginning of control packet

    m.rxf_l = compute_phase (hiq->freq);
    m.txf_l = compute_phase (hiq->freq);

    m.txl = 0;    //Tx output level 0 to 255

    m.txc = 0x02  //   all other operation (e.g. SSB)
          | 0x04  //   use the HiQSDR extended IO pins (from FPGA version 1.1 on)
    ;
    m.rxc = get_decimation(hiq->bw) - 1;

    m.fwv = 3;  // FPGA firmware version

    m.x1  = (hiq->preSel & 0x0F)    // preselector et al.
            |
            (hiq->preamp ? 0x10 : 0x00);   // preamplifier
 
    m.att = hiq->attDb;           // input attenuator

    m.msc = hiq->antSel;  // select antenna 

    m.rfu1 = 0; // not currently used
    m.rfu2 = 0; // not currently used
    m.rfu3 = 0; // not currently used
    m.rfu4 = 0; // not currently used
    m.rfu5 = 0; // not currently used

    // send the message buffer
    rc = send (hiq->ctrl_socket, (char*)&m, sizeof(m), 0);
    if(rc <= 0) {
        perror("sendto failed for control message");
    } 
    return rc;
}

int tmo_recvfrom(int s, void *buf, int buf_length, int flag, sockaddr *sockfrom, socklen_t *cl, int tmo_s)
{
    int rc;
    struct timeval t = { 0 };
    fd_set socks;
    FD_ZERO(&socks);
    FD_SET(s, &socks);

    t.tv_sec = tmo_s;
    if ((rc = select(s + 1, &socks, NULL, NULL, &t)) == 1) {
        return recvfrom (s, buf, buf_length, flag, sockfrom, cl );
    } else {
        if (rc != 0) perror("select failed in tmo_recvfrom");
        return rc;
    }
}

static int ping_hardware (struct Hiqsdr *hiq, int tmo_s)
{
    struct sockaddr_in client;
    int client_length = sizeof(client);
    static control_msg m = { 0 };
    int rc = -1;

    //
    // load a short control frame, assuming that the target runs 1.0
    //
    m.s = 'S', m.t = 't'; // Marks the beginning of control packet

    m.rxf_l = compute_phase (hiq->freq);
    m.txf_l = compute_phase (hiq->freq);

    m.txl = 0;    //Tx output level 0 to 255

    m.txc = 0x02; // = all other operation (e.g. SSB)
    m.rxc = get_decimation(hiq->bw) - 1;

    m.fwv = 0;  // FPGA firmware version

    m.x1  = 0;  // preselector et al.
    m.att = 0;  // input attenuator

    m.rfu1 = 0; // not currently used
    m.rfu2 = 0; // not currently used
    m.rfu3 = 0; // not currently used
    m.rfu4 = 0; // not currently used
    m.rfu5 = 0; // not currently used

    // send the message buffer, socket already connected 
    rc = send (hiq->ctrl_socket, (char*)&m, sizeof(m), 0);
    if(rc <= 0) {
        perror("sendto failed for control message");
    } else {
        control_msg ma;

        // wait for answer, timeout in seconds
        int bytes_read = tmo_recvfrom (hiq->ctrl_socket, (void *)&ma, sizeof(ma), 0, (struct sockaddr *)&client, (socklen_t *)&client_length, tmo_s);
        if (bytes_read == 0) {
            fprintf (stderr, "Timeout !\n"); 
        } else {

            if(bytes_read < 0) {
                perror("recvfrom socket failed for control socket buffer");
            } else {
                if (bytes_read >= 14 && bytes_read <= 22) {
                   if (ma.fwv == 0) {
                       fprintf (stderr, "Frame length is %d and firmware version is %02x\n", bytes_read, ma.fwv);
                       hiq->fwv = ma.fwv;
                       rc = 0; //OK, old firmware
                   } else { // new
                       fprintf (stderr, "Frame length is %d and firmware version is %02x\n", bytes_read, ma.fwv);
                       hiq->fwv = ma.fwv;
                       rc = 0;
                   }
                } else {
                    fprintf (stderr, "Unxpected frame length: %d proceed at your risk !\n", bytes_read);
                    rc = 0;
                }
            } 
        }
    }
    return rc;
}

static void dump_buffer(unsigned char* buffer, int len /* bytes */) 
{
    int line = 0;
    for (int i = 0; i < len; ) {

        fprintf (stderr, "%04d: ", line*32);
        int c;
        for (c=0; (c < 32) && (i < len); ++i, c++) {
            fprintf (stderr, "%02x ", buffer[i]);
        }
        
        fprintf (stderr, "\n");
        line+=1;
    }
}


/*
 *
 * The data format of the samples is little endian (least signicant
 * byte rst) with real words at the odd (rst) positions and the imaginary words
 * at the even (second) positions.!
 *
 */

typedef union {
        struct __attribute__((__packed__)) {
                uint8_t         i1;
                uint8_t         i2;
                uint8_t         i3;
                uint8_t         q1;
                uint8_t         q2;
                uint8_t         q3;
                };
} iq_sample;

#define i_sample(x) ((unsigned long)(x.i1 | (x.i2 << 8) | (x.i2 << 16)))
#define q_sample(x) ((unsigned long)(x.q1 | (x.q2 << 8) | (x.q2 << 16)))




#if defined __TEST_MODULE__

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

timespec diff(timespec start, timespec end)
{
	timespec temp;
	if ((end.tv_nsec - start.tv_nsec) < 0) {
		temp.tv_sec  = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	} else {
		temp.tv_sec  = end.tv_sec  - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}
	return temp;
}


int user_data_callback(void *buf, int bsize, void *extra)
{
    int *pndp = (int *) extra;
    if (bsize && (*pndp)) {
        unsigned char* buffer = (unsigned char*)buf;
        int buf_size = bsize;
        fprintf (stderr, "%d: Received: len: %d seq: %d %c %c\n", 
                 (*pndp),
                 buf_size, buffer[0],
                 (buffer[1] & 0x01) ? 'K' : ' ',
                 (buffer[1] & 0x02) ? 'C' : ' '
                 );
        *pndp = (*pndp) - 1;
        return 0;
    } else 
        return -1;
}

void test_sample_rates (int sr, int ndp = 1000)
{
    fprintf (stderr, "Measuring real throughput in kS/s [kilo Samples per second].\n");

    fprintf (stderr, "Sending activation command.....");
    int rc = send_activation (&hq);
    fprintf (stderr, " %d\n", rc);

    // exercise control messages
    hiqsdr_set_frequency (3679995LL);
    hiqsdr_set_bandwidth (sr);

    fprintf (stderr, "Listening for %d data packets, please wait.....\n", ndp);

    timespec  time_start, time_end, time_diff;
	clock_gettime (CLOCK_REALTIME, &time_start);

    unsigned long int ts = 0;
    for (int i; i < ndp; ++i) {
        unsigned char buffer [10240];
        int buf_len = get_data (&hq, buffer, sizeof(buffer));

        ts += ( (buf_len - 2) / 6);
    }

	clock_gettime(CLOCK_REALTIME, &time_end);
    time_diff = diff(time_start, time_end);
    long double diff_s = time_diff.tv_sec + (time_diff.tv_nsec/1E9) ;
    //fprintf(stderr, "diff: %ds::%dns %Lf seconds\n", time_diff.tv_sec, time_diff.tv_nsec, diff_s);
    fprintf (stderr, "Samples received: %lu, %.1Lf kS/s\n", ts, ((double)ts / (diff_s)/1E3) );


    fprintf (stderr, "Sending deactivation command.....");
    send_deactivation (&hq);
    fprintf (stderr, " %d\n", rc);
}


int main (int argc, const char** argv)
{
    int ndp, rc;
    long long int f_rx = 3679995LL;
    long long int f_tx = 3676350LL;

    fprintf (stderr, "%lld Hz, Rx phase: %08lX\n", f_rx, compute_phase(f_rx) );
    fprintf (stderr, "%lld Hz, Tx phase: %08lX\n", f_tx, compute_phase(f_tx) );

    f_rx = 7050000LL;
    fprintf (stderr, "%lld Hz, Rx phase: %08lX\n", f_rx, compute_phase(f_rx) );

    f_rx = 30000000LL;
    fprintf (stderr, "%lld Hz, Rx phase: %08lX\n", f_rx, compute_phase(f_rx) );

    int bw = 48000;
    fprintf (stderr, "bandwidth %d, decimation: %d 0x%02x\n", bw, get_decimation(bw), get_decimation(bw) );

    bw = 96000;
    fprintf (stderr, "bandwidth %d, decimation: %d 0x%02x\n", bw, get_decimation(bw), get_decimation(bw) );

    bw = 192000;
    fprintf (stderr, "bandwidth %d, decimation: %d 0x%02x\n", bw, get_decimation(bw), get_decimation(bw) );


    const char* hiqsdr_ip = "192.168.2.196";
    const char* hiqsdr_sr = "48000";
    const char* hiqsdr_f  = "7050000";

    if (argc > 1) {
        hiqsdr_ip = argv[1];
        if (argc > 2) {
            hiqsdr_sr = argv[2];
            if (argc > 3) {
                hiqsdr_f = argv[3];
            }
        }
    }

    if (hiqsdr_init (hiqsdr_ip, atol(hiqsdr_sr), atoll(hiqsdr_f))) {
        fprintf (stderr, "Init failed.\n");
        return 255;
    }

    test_sample_rates (48000);
    test_sample_rates (96000);
    test_sample_rates (192000);
    test_sample_rates (960000, 10000);


    fprintf (stderr, "Sending activation command in order to receive I/Q samples.....");
    rc = send_activation (&hq);
    fprintf (stderr, " %d\n", rc);


    //
    // asynch packet reception test
    // stopping happening internally (error in user callback simulation)
    //
    ndp = 10;
    fprintf (stderr, "\nAsyhchronously listening for %d data packets.....\n", ndp);

    hiqsdr_start_asynch_input (user_data_callback, (void *)&ndp);

    while (ndp) {
        sleep (1);
    };

    hiqsdr_stop_asynch_input (); // necessary in order to avoid memory leaks

    fprintf (stderr, "\n");
    fprintf (stderr, "Sending deactivation command.....");
    send_deactivation (&hq);
    fprintf (stderr, " %d\n\n", rc);

    //
    // asynch packet reception test
    // thread stopped explicitily from outside via hiqsdr_stop_asynch_input()
    //
    rc = send_activation (&hq);
    fprintf (stderr, " %d\n", rc);

    ndp = 100000;
    fprintf (stderr, "\nAsyhchronously listening for %d data packets.....\n", ndp);

    hiqsdr_start_asynch_input (user_data_callback, (void *)&ndp);

    for (int j=0;j<10;j++) {
        usleep (100);
    }

    hiqsdr_stop_asynch_input ();

    fprintf (stderr, "\nAsynch stop issued !\n");
    fprintf (stderr, "Sending deactivation command.....");
    send_deactivation (&hq);
    fprintf (stderr, " %d\n\n", rc);


    hiqsdr_deinit (); 

    return 0;
}



#endif





