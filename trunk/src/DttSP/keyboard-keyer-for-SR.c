/** 
* @file keyboard-keyer.c
* @brief Functions to implement a keyboard keyer
* @author Frank Brickle, AB2KT and Bob McGwier, N4HY 


This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006, 2007, 2008 by Frank Brickle, AB2KT and Bob McGwier, N4HY
Doxygen comments added by Dave Larsen, KV0S

This derived version is
Copyright (C) 2010-2011 by Alex Lee, 9V1AL

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

The authors can be reached by email at

ab2kt@arrl.net
or
rwmcgwier@gmail.com

or by paper mail at

The DTTS Microwave Society
6 Kathleen Place
Bridgewater, NJ 08807
*/  

#include <fromsys.h>
#include <banal.h>
#include <splitfields.h>
#include <datatypes.h>
#include <bufvec.h>
#include <cxops.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>  
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/socket.h>  
#include <netdb.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#define SAMP_RATE (48000)
#define HUGE_PHASE (1256637061.43593)

#define RING_SIZE (01 << 022)

// added by Alex Lee 25 Dec 2010
#define USBSOFTROCK_CLIENT_COMMAND 19004
#define DTTSP_PORT_CLIENT_BUFSIZE  65536

// in nanoseconds = 100 ms
#define PTT_HANG_TIME_POLLING_INTERVAL 100000000
// HANG_TIME_THRESHOLD in 100ms intervals
#define HANG_TIME_THRESHOLD  15

int hang_time = 0;
pthread_t input, play, PTT_timer;
sem_t ready, reader, writer;

jack_client_t *client;
jack_port_t *lport, *rport;
jack_ringbuffer_t *lring, *rring;
jack_nframes_t size;

BOOLEAN playing = FALSE;
double wpm = 18.0, freq = 700.0, gain = -6.0, ramp = 5.0;

COMPLEX *zout = 0;

/// basic mapping, chars -> morse strings
char *morse_table[128];

// CW tone segments
#define ME_EOF (-1)
#define ME_ZERO (0)
#define ME_RAMP (1)
#define ME_STDY (2)

struct {
  double wpm, rise, fall, curr, incr, rate;
  int type, size;
} morsel;

int ditspacesize, dahspacesize,
    ditstdysize, dahstdysize,
    charspacesize, wordspacesize,
    risesize, fallsize;
double riseincr, fallincr;

struct sockaddr_in Sa;
struct sockaddr_in *pSa;
struct addrinfo	*address;
unsigned short port;
int clen, flags, sock;
char  buff [DTTSP_PORT_CLIENT_BUFSIZE];
int   ssize, used;
int   param;					// added by Alex Lee 21 Oct 2010

int PTT_state = 0;

#define MAX_ESC (512)
#define ESC_L '<'
#define ESC_R '>'

void inlinecmd(char *, int);

void jack_ringbuffer_clear(jack_ringbuffer_t *, int);
void jack_ringbuffer_restart(jack_ringbuffer_t *, int);
void send_sound(COMPLEX *, int);

//------------------------------------------------------------

void open_usbsoftrock(void){

	port = USBSOFTROCK_CLIENT_COMMAND;
	pSa = &Sa;
	address = NULL;
        // create socket 
        if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
          fprintf(stderr, "Couldn't create dttsp_port_client socket\n");
          exit(1);
        }
	fprintf( stderr, "DttSP port %d\n", port);

        // one size fits all
        ssize = DTTSP_PORT_CLIENT_BUFSIZE;
        memset(buff, 0, ssize);
    }


int send_command ( char *cmdstr ) 
{
    fd_set fds;
    struct timeval tv;
    // are we pointing at the moon?
    if ((!pSa) || sock == -1 || !cmdstr)
      return -1;

    // make local, properly terminated copy of command
    // Needs better error checking
    // TBD
    strcpy (buff, cmdstr);
    strcat (buff, "\n");
    used = strlen (buff);

    // blast it
    if (address == NULL) {
	clen = sizeof(*pSa);
    	memset((char *) pSa, 0, clen);
    	pSa->sin_family = AF_INET;
    	pSa->sin_addr.s_addr = htonl(INADDR_ANY);
    	pSa->sin_port = htons((unsigned short) port);
    	if (sendto(sock, buff, used, flags, (struct sockaddr *) pSa, clen) != used) {
			fprintf (stderr, "%s: error in sendto\n", __FUNCTION__); 
			return -3;
		}
    } else {
		
    	if (sendto(sock, buff, used, flags, address->ai_addr, address->ai_addrlen) != used) {
        	fprintf (stderr, "%s: error in sendto\n", __FUNCTION__); 
        	return -3;
    	}
    }

    // wait a little for ack
   	FD_ZERO(&fds);
   	FD_SET(sock, &fds);
   	tv.tv_sec = 1;
   	tv.tv_usec = 0;
	
   	if (!select(sock + 1, &fds, 0, 0, &tv)) {
      	fprintf (stderr, "%s: error from select, disabling port\n", __FUNCTION__); 
		close (sock);
		sock = -1;
		pSa = NULL;
       	return -4;
   	}
	if (address == NULL) {
   		if (recvfrom(sock, buff, size, flags, (struct sockaddr *) pSa, (socklen_t *)(&clen)) <= 0) {
       		fprintf (stderr, "%s: error in recvfrom\n", __FUNCTION__); 
       		return -5;
   		}
	} else {
   		if (recvfrom(sock, buff, size, flags, address->ai_addr, &address->ai_addrlen) <= 0) 		{
       		fprintf (stderr, "%s: error in recvfrom\n", __FUNCTION__); 
     	  	return -5;
		}
	}

  	if (buff[0] != 'o' || buff[1] != 'k') return -6;
	if(buff[3] == '0') param = 0;
	else if (buff[3] == '1') param = 1;
	else if (buff[3] == '2') param = 2;
	else if (buff[3] == '3') param = 3;
	else param = 0;

    return 0;
}

void PTT_timer_thread(void){
  struct timespec req, rem;
  req.tv_sec = 0;
  req.tv_nsec = PTT_HANG_TIME_POLLING_INTERVAL;
  for (;;) {
      nanosleep(&req, &rem);
      if (PTT_state){
	  hang_time++;
          if (hang_time > HANG_TIME_THRESHOLD){
	 	send_command("set ptt off");
         	PTT_state = 0;
		hang_time = 0;
	        }
	   }
 	}
}

/* -------------------------------------------------------------------------- */
/** @brief Get morse from hash table
* 
* try to map char -> morse string
*
* @param c 
* @return *char
*/
/* ---------------------------------------------------------------------------- */
char *
get_morse(int c) {
  return morse_table[c & 0x7F];
}

/* -------------------------------------------------------------------------- */
/** @brief Run reader thread
*
* translate text input to timed, sub-morse-element
* audio segment specs; parcel the segments out
* one at a time to the sound player
*
* @return void
*
*/
/* ---------------------------------------------------------------------------- */
void
reader_thread(void) {
  BOOLEAN b = TRUE; // we're coming from silence
  int c, e;
  char *m;

  
  // keep reading 1 char at a time
  while ((c = getchar()) != EOF) {
    
    // inline command?
    if (c == ESC_L) {
      int i = 0;
      char buf[MAX_ESC];
      while ((c = getchar()) != EOF) {
	if (c == ESC_R) break;
	buf[i] = c;
	if (++i >= (MAX_ESC - 1)) break;
      }
      if (c == EOF) goto finish;
      buf[i] = 0;
      inlinecmd(buf, i);
      continue;
    }

    /// is char mapped to morse?
    if (m = get_morse(c)) {
      hang_time = 0;			// reset the hang time couter whenever a morse character is starting to be sent
      if (PTT_state == 0){		// Is PTT off?  We need to turn PTT on
	send_command("set ptt on");
	PTT_state = 1;

        sem_wait(&reader);		// send a charspace to allow for RxTx switching delay
        morsel.type = ME_ZERO;
	morsel.size = charspacesize;
        sem_post(&writer);
        };

      /// for each element in morse string
      // (dit/dah, doesn't matter)
      while (e = *m++) {
	// first segment is ramp up...
	sem_wait(&reader);
	morsel.type = ME_RAMP, morsel.size = risesize;
	morsel.curr = 0.0, morsel.incr = riseincr;
	sem_post(&writer);
	
	// ...then steady state...
	// (choose dit/dah here)
	sem_wait(&reader);
	morsel.type = ME_STDY;
	morsel.size = e == '.' ? ditstdysize : dahstdysize;
	sem_post(&writer);
	
	// ...then ramp down...
	sem_wait(&reader);
	morsel.type = ME_RAMP, morsel.size = fallsize;
	morsel.curr = 1.0, morsel.incr = fallincr;
	sem_post(&writer);
	
	// ...finally, post-element pause
	sem_wait(&reader);
	morsel.type = ME_ZERO;
	morsel.size = ditspacesize;
	sem_post(&writer);
      }
      
      // post-character pause
      sem_wait(&reader);
      morsel.type = ME_ZERO;
      // (we already emitted a dit-sized space)
      morsel.size = charspacesize - ditspacesize;
      sem_post(&writer);
      
      // wherever we go next, it won't have been from silence
      b = FALSE;

    } else {
      /// anything else treated as interword space,
      /// which has only one segment (silence)
      sem_wait(&reader);
      morsel.type = ME_ZERO;
      ///  was previous output also interword space?
      if (b)
	// yes, use full duration
	morsel.size = wordspacesize;
      else
	// no, part of duration already played
	morsel.size = wordspacesize - charspacesize;

      b = TRUE;
      sem_post(&writer);
    }

  }
  
 finish:
  // indicate EOF on input
  sem_wait(&reader);
  morsel.type = ME_EOF;
  sem_post(&writer);
  pthread_exit(0);
}

/* -------------------------------------------------------------------------- */
/** @brief Run sound thread 
* @return void
*/
/* ---------------------------------------------------------------------------- */
void
sound_thread(void) {
  int i, k = 0;
  double ofreq, scale, phase = 0.0;
  COMPLEX z, delta_z;

  // keep looking for sub-element segments, one at a time
  for (;;) {

    // pause for next sub-element segment
    sem_post(&reader);
    sem_wait(&writer);

    // no more data?
    if (morsel.type == ME_EOF)
      break;

    // requires playing some tone?
    if (morsel.type != ME_ZERO) {
      // yes, reset params and
      // set up CORDIC tone generation
      ofreq = freq * 2.0 * M_PI / SAMP_RATE;
      scale = pow(10.0, gain / 20.0);
      if (phase > HUGE_PHASE)
	phase -= HUGE_PHASE;
      z = Cmplx(cos(phase), sin(phase));
      delta_z = Cmplx(cos(ofreq), sin(ofreq));
    }

    // play out this segment
    for (i = 0; i < morsel.size; i++) {

      // make silence
      if (morsel.type == ME_ZERO)
	zout[k] = cxzero;
      
      // make tone
      else {
	z = Cmul(z, delta_z);
	phase += ofreq;
	// is this a ramping segment?
	if (morsel.type == ME_RAMP) {
	  morsel.curr += morsel.incr;
	  zout[k] = Cscl(z, scale * sin(morsel.curr * M_PI / 2.0));
	} else
	  zout[k] = Cscl(z, scale);
      }

      // have we played enough to fill a jack buffer?
      if (++k >= size) {
	// yes, send to output
	send_sound(zout, k);
	// wait until some audio has been drained
	sem_wait(&ready);
	k = 0;
	if (morsel.type != ME_ZERO) {
	  // reset CORDIC
	  if (phase > HUGE_PHASE)
	    phase -= HUGE_PHASE;
	  z = Cmplx(cos(phase), sin(phase));
	  delta_z = Cmplx(cos(ofreq), sin(ofreq));
	}
      }
    }
  }

  // anything left unsent?
  if (k > 0)
    send_sound(zout, k);

  pthread_exit(0);
}

//------------------------------------------------------------------------

/* -------------------------------------------------------------------------- */
/** @brief  Clear jack ring bufferr 
* 
* @param ring 
* @param nbytes 
* @return void
*/
/* ---------------------------------------------------------------------------- */
void
jack_ringbuffer_clear(jack_ringbuffer_t *ring, int nbytes) {
  int i;
  char zero = 0;
  for (i = 0; i < nbytes; i++)
    jack_ringbuffer_write(ring, &zero, 1);
}

/* -------------------------------------------------------------------------- */
/** @brief Restart jack ring buffer 
* 
* @param ring 
* @param nbytes 
* @return void
*/
/* ---------------------------------------------------------------------------- */
void
jack_ringbuffer_restart(jack_ringbuffer_t *ring, int nbytes) {
  jack_ringbuffer_reset(ring);
  jack_ringbuffer_clear(ring, nbytes);
}

/* -------------------------------------------------------------------------- */
/** @brief Send sound jack buffer 
* 
* @param buff 
* @param len 
* @return void
*/
/* ---------------------------------------------------------------------------- */
void
send_sound(COMPLEX *buff, int len) {
  if (jack_ringbuffer_write_space(lring) < len * sizeof(float)) {
    (void) write(2, "overrun\n", 8);
    jack_ringbuffer_restart(lring, size * sizeof(float));
    jack_ringbuffer_restart(rring, size * sizeof(float));
  } else {
    int i;
    for (i = 0; i < len; i++) {
      float l = buff[i].re, r = buff[i].im;
      jack_ringbuffer_write(lring, (char *) &l, sizeof(float));
      jack_ringbuffer_write(rring, (char *) &r, sizeof(float));
    }
  }
}

/* -------------------------------------------------------------------------- */
/** @brief private jack_xrun 
* 
* @param arg 
* @return void
*/
/* ---------------------------------------------------------------------------- */
PRIVATE void
jack_xrun(void *arg) {
  char *str = "xrun!\n";
  (void) write(2, str, strlen(str));
}

/* -------------------------------------------------------------------------- */
/** @brief private jack_shutdown 
* 
* @param arg 
* @return void
*/
/* ---------------------------------------------------------------------------- */
PRIVATE void
jack_shutdown(void *arg) {}

/* -------------------------------------------------------------------------- */
/** @brief private jack_callback 
* 
* @param nframes 
* @param arg 
* @return void
*/
/* ---------------------------------------------------------------------------- */
PRIVATE int
jack_callback(jack_nframes_t nframes, void *arg) {
  char *lp, *rp;
  int nwant = nframes * sizeof(float),
      nhave = jack_ringbuffer_read_space(lring);

  lp = jack_port_get_buffer(lport, nframes);
  rp = jack_port_get_buffer(rport, nframes);
  if (nhave >= nwant) {
    jack_ringbuffer_read(lring, lp, nwant);
    jack_ringbuffer_read(rring, rp, nwant);
    sem_post(&ready);
  } else {
    memset(lp, 0, nwant);
    memset(rp, 0, nwant);
  }
	return 0;
}

/* -------------------------------------------------------------------------- */
/** @brief Reset parameter on keyer  
* @return void
*/
/* ---------------------------------------------------------------------------- */
void
resetparam(void) {
  morsel.wpm = wpm;
  morsel.rise = morsel.fall = ramp;
  morsel.rate = SAMP_RATE;

  ditspacesize = SAMP_RATE * 1.2 / morsel.wpm + 0.5;
  dahspacesize = 3 * ditspacesize;
  charspacesize = dahspacesize;
  wordspacesize = 7 * ditspacesize;

  risesize = SAMP_RATE * morsel.rise / 1e3 + 0.5;
  if (risesize > 1)
    riseincr = 1.0 / (risesize - 1);
  else
    riseincr = 1.0;

  fallsize = SAMP_RATE * morsel.fall / 1e3 + 0.5;
  if (fallsize > 1)
    fallincr = -1.0 / (fallsize - 1);
  else
    fallincr = -1.0;

  ditstdysize = ditspacesize - risesize - fallsize;
  dahstdysize = dahspacesize - risesize - fallsize;
}

/* -------------------------------------------------------------------------- */
/** @brief main
* 
* @param argc 
* @param *argv 
* @return int
*/
/* ---------------------------------------------------------------------------- */
int
main(int argc, char **argv) {
  int i;
  jack_status_t jack_status;
  char command_str[80];
  int ifreq;

  for (i = 1; i < argc; i++)
    if (argv[i][0] == '-')
      switch (argv[i][1]) {
      case 'f':
	freq = atof(argv[++i]);
	break;
      case 'w':
	wpm = atof(argv[++i]);
	break;
      case 'g':
	gain = atof(argv[++i]);
	break;
      case 'r':
	ramp = atof(argv[++i]);
	break;
      default:
	fprintf(stderr, "keyboard-keyer [-w wpm] [-f freq] [-g gain_dB] [-r ramp_ms] [infile]\n");
	exit(1);
      }
    else break;

  if (i < argc) {
    if (!freopen(argv[i], "r", stdin))
      perror(argv[i]), exit(1);
    i++;
  }

  //------------------------------------------------------------

  resetparam();

  //------------------------------------------------------------

  if (!(client = jack_client_open("kkyr", JackNullOption, &jack_status)))
    fprintf(stderr, "can't make client -- jack not running?\n"), exit(1);

  fprintf(stderr, "jack status %d\n", jack_status);

  jack_set_process_callback(client, (void *) jack_callback, 0);
  jack_on_shutdown(client, (void *) jack_shutdown, 0);
  jack_set_xrun_callback(client, (void *) jack_xrun, 0);
  size = jack_get_buffer_size(client);

  lport = jack_port_register(client,
			     "ol",
			     JACK_DEFAULT_AUDIO_TYPE,
			     JackPortIsOutput,
			     0);
  if (lport == NULL){
	fprintf(stderr, "jack lport register failed\n");
	exit(1);
	}
  rport = jack_port_register(client,
			     "or",
			     JACK_DEFAULT_AUDIO_TYPE,
			     JackPortIsOutput,
			     0);
  if (rport == NULL){
	fprintf(stderr, "jack rport register failed\n");
	exit(1);
	}
  lring = jack_ringbuffer_create(RING_SIZE);
  rring = jack_ringbuffer_create(RING_SIZE);
  jack_ringbuffer_clear(lring, size * sizeof(float));
  jack_ringbuffer_clear(rring, size * sizeof(float));
  

  jack_activate(client);


  //------------------------------------------------------------

  zout = newvec_COMPLEX(size, "keyb sample buffer");

  //------------------------------------------------------------

  sem_init(&ready, 0, 0);
  sem_init(&reader, 0, 0);
  sem_init(&writer, 0, 0);
  pthread_create(&input, 0, (void *) reader_thread, 0);
  pthread_create(&play, 0, (void *) sound_thread, 0);
  pthread_create(&PTT_timer, 0, (void *) PTT_timer_thread, 0);

  //------------------------------------------------------------

/*
  {
    const char **ports;
    if (!(ports = jack_get_ports(client, 0, 0, JackPortIsPhysical | JackPortIsInput))) {
      fprintf(stderr, "can't find any physical playback ports\n");
      exit(1);
    }
    if (jack_connect(client, jack_port_name(lport), ports[0])) {
      fprintf(stderr, "can't connect left output\n");
      exit(1);
    }
    if (jack_connect(client, jack_port_name(rport), ports[1])) {
      fprintf(stderr, "can't connect right output\n");
      exit(1);
    }
    free(ports);
  }


  fprintf(stderr, "connected to playback ports\n");
*/

  open_usbsoftrock();
  ifreq = freq;	// double -> int
  sprintf(command_str,"set tone %d\n", ifreq);
  send_command(command_str);

  pthread_join(input, 0);
  pthread_join(play, 0);

  fprintf(stderr, "subprocesses ended\n");

  close(sock);

  jack_client_close(client);

  //------------------------------------------------------------

  delvec_COMPLEX(zout);

  //------------------------------------------------------------

  jack_ringbuffer_free(lring);
  jack_ringbuffer_free(rring);
  sem_destroy(&ready);
  sem_destroy(&reader);
  sem_destroy(&writer);

  //------------------------------------------------------------

  exit(0);
}

char *morse_table[128] = {
  /* 000 NUL */ 0, /* 001 SOH */ 0, /* 002 STX */ 0, /* 003 ETX */ 0,
  /* 004 EOT */ 0, /* 005 ENQ */ 0, /* 006 ACK */ 0, /* 007 BEL */ 0,
  /* 008  BS */ 0, /* 009  HT */ 0, /* 010  LF */ 0, /* 011  VT */ 0,
  /* 012  FF */ 0, /* 013  CR */ 0, /* 014  SO */ 0, /* 015  SI */ 0,
  /* 016 DLE */ 0, /* 017 DC1 */ 0, /* 018 DC2 */ 0, /* 019 DC3 */ 0,
  /* 020 DC4 */ 0, /* 021 NAK */ 0, /* 022 SYN */ 0, /* 023 ETB */ 0,
  /* 024 CAN */ 0, /* 025  EM */ 0, /* 026 SUB */ 0, /* 027 ESC */ 0,
  /* 028  FS */ 0, /* 029  GS */ 0, /* 030  RS */ 0, /* 031  US */ 0,
  /* 032  SP */ 0,
  /* 033   ! */ "...-.",	// [SN]
  /* 034   " */ 0,
  /* 035   # */ 0,
  /* 036   $ */ 0,
  /* 037   % */ ".-...",	// [AS]
  /* 038   & */ 0,
  /* 039   ' */ 0,
  /* 040   ( */ "-.--.",	// [KN]
  /* 041   ) */ 0,
  /* 042   * */ "...-.-",	// [SK]
  /* 043   + */ ".-.-.",	// [AR]
  /* 044   , */ "--..--",
  /* 045   - */ "-....-",
  /* 046   . */ ".-.-.-",
  /* 047   / */ "-..-.",
  /* 048   0 */ "-----",
  /* 049   1 */ ".----",
  /* 050   2 */ "..---",
  /* 051   3 */ "...--",
  /* 052   4 */ "....-",
  /* 053   5 */ ".....",
  /* 054   6 */ "-....",
  /* 055   7 */ "--...",
  /* 056   8 */ "---..",
  /* 057   9 */ "----.",
  /* 058   : */ 0,
  /* 059   ; */ 0,
  /* 060   < */ 0,
  /* 061   = */ "-...-",	// [BT]
  /* 062   > */ 0,
  /* 063   ? */ "..__..",	// [IMI]
  /* 064   @ */ ".--.-.",
  /* 065   A */ ".-",
  /* 066   B */ "-...",
  /* 067   C */ "-.-.",
  /* 068   D */ "-..",
  /* 069   E */ ".",
  /* 070   F */ "..-.",
  /* 071   G */ "--.",
  /* 072   H */ "....",
  /* 073   I */ "..",
  /* 074   J */ ".---",
  /* 075   K */ "-.-",
  /* 076   L */ ".-..",
  /* 077   M */ "--",
  /* 078   N */ "-.",
  /* 079   O */ "---",
  /* 080   P */ ".--.",
  /* 081   Q */ "--.-",
  /* 082   R */ ".-.",
  /* 083   S */ "...",
  /* 084   T */ "-",
  /* 085   U */ "..-",
  /* 086   V */ "...-",
  /* 087   W */ ".--",
  /* 088   X */ "-..-",
  /* 089   Y */ "-.--",
  /* 090   Z */ "--..",
  /* 091   [ */ 0,
  /* 092   \ */ 0,
  /* 093   ] */ 0,
  /* 094   ^ */ 0,
  /* 095   _ */ 0,
  /* 096   ` */ 0,
  /* 097   a */ ".-",
  /* 098   b */ "-...",
  /* 099   c */ "-.-.",
  /* 100   d */ "-..",
  /* 101   e */ ".",
  /* 102   f */ "..-.",
  /* 103   g */ "--.",
  /* 104   h */ "....",
  /* 105   i */ "..",
  /* 106   j */ ".---",
  /* 107   k */ "-.-",
  /* 108   l */ ".-..",
  /* 109   m */ "--",
  /* 110   n */ "-.",
  /* 111   o */ "---",
  /* 112   p */ ".--.",
  /* 113   q */ "--.-",
  /* 114   r */ ".-.",
  /* 115   s */ "...",
  /* 116   t */ "-",
  /* 117   u */ "..-",
  /* 118   v */ "...-",
  /* 119   w */ ".--",
  /* 120   x */ "-..-",
  /* 121   y */ "-.--",
  /* 122   z */ "--..",
  /* 123   { */ 0,
  /* 124   | */ 0,
  /* 125   } */ 0,
  /* 126   ~ */ 0,
  /* 127 DEL */ 0
};

/* -------------------------------------------------------------------------- */
/** @brief inline command 
* 
* @param buf 
* @param len 
* @return void
*/
/* ---------------------------------------------------------------------------- */
void
inlinecmd(char *buf, int len) {
char command_str[80];
int ifreq;

  if (!buf || len < 1) return;
  if (!strncmp(buf, "wpm", 3)) {
    wpm = atof(buf + 3);
    resetparam();
  } else if (!strncmp(buf, "ramp", 4)) {
    ramp = atof(buf + 4);
    resetparam();
  } else if (!strncmp(buf, "freq", 4)) {
    freq = atof(buf + 4);
    ifreq = freq;
    sprintf(command_str, "set tone %d\n", ifreq);
    send_command(command_str);
  } else if (!strncmp(buf, "gain", 4))
    gain = atof(buf + 4);
}
