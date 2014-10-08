/** 
* @file iambic-keyer.c
* @brief Function to implement a iambic keyer
* @author Frank Brickle, AB2KT and Bob McGwier, N4HY

This file is part of a program that implements a Software-Defined Radio.

The code in this file is derived from routines originally written by
Pierre-Philippe Coupard for his CWirc X-chat program. That program
is issued under the GPL and is
Copyright (C) Pierre-Philippe Coupard - 18/06/2003

This derived version is
Copyright (C) 2004-2008 by Frank Brickle, AB2KT and Bob McGwier, N4HY
Doxygen comments added by Dave Larsen, KV0S

This further derived version is
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

#include <linux/rtc.h>
#include <fromsys.h>
#include <banal.h>
#include <splitfields.h>
#include <datatypes.h>
#include <bufvec.h>
#include <cxops.h>
#include <oscillator.h>
#include <cwtones.h>

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

//========================================================================

#define L_KEY_DOWN	  (01 << 00)
#define R_KEY_DOWN	  (01 << 01)

#define NO_TIME_LEFTS_SCHED	(-2)
#define NO_ELEMENT		(-1)
#define DIT			 (0)
#define DAH			 (1)
#define MODE_A			 (0)
#define MODE_B			 (1)
#define NO_PADDLE_SQUEEZE	 (0)
#define PADDLES_SQUEEZED	 (1)
#define PADDLES_RELEASED	 (2)
#define NO_DELAY		 (0)
#define CHAR_SPACING_DELAY	 (1)
#define WORD_SPACING_DELAY	 (2)
#define DEBOUNCE_BUF_MAX_SIZE	 (10)
// added by Alex Lee 25 Dec 2010
#define USBSOFTROCK_CLIENT_COMMAND 19004
#define DTTSP_PORT_CLIENT_BUFSIZE  65536
#define key_FIFO_SIZE		 5
// In nanoseconds.  2000000 is 2ms
#define READ_KEY_INTERVAL	2000000
// 100 ms
#define PTT_HANG_TIME_POLLING_INTERVAL 100000000
// HANG_TIME in 100ms intervals
#define HANG_TIME_THRESHOLD  10
//========================================================================

typedef
struct _keyer_state {
  struct {
    BOOLEAN iambic,	// iambic or straight
      	    mdlmdB,	// set true if mode B
            revpdl;	// paddles reversed
    struct {
      BOOLEAN dit, dah;
    } memory;		// set both true for mode B
    struct {
      BOOLEAN khar, word;
    } autospace;
  } flag;
  int debounce,	// # seconds to read paddles
      mode,	// 0 = mode A, 1 = mode B
      weight;	// 15 -> 85%
  double wpm;	// for iambic keyer
} KeyerStateInfo, *KeyerState;

extern KeyerState newKeyerState(void);
extern void delKeyerState(KeyerState ks);

struct sockaddr_in Sa;
struct sockaddr_in *pSa;
struct addrinfo	*address;
unsigned short port;
int clen, flags, sock;
char  buff [DTTSP_PORT_CLIENT_BUFSIZE];
int   ssize, used;
int   param;					// added by Alex Lee 21 Oct 2010

int PTT_state = 0;
int hang_time = 0;

//------------------------------------------------------------------------

typedef
struct _keyer_logic {
  struct {
    BOOLEAN init;
    struct {
      BOOLEAN dit, dah;
    } prev;
  } flag;
  struct {
    BOOLEAN altrn, // insert alternate element
            psqam; // paddles squeezed after mid-element
    int curr, // -1 = nothing, 0 = dit, 1 = dah
        iamb, //  0 = none, 1 = squeezed, 2 = released
        last; // -1 = nothing, 0 = dit, 1 = dah
  } element;
  struct {
    double beep, dlay, elem, midl;
  } time_left;
  int dlay_type; // 0 = none, 1 = interchar, 2 = interword
} KeyerLogicInfo, *KeyerLogic;

extern KeyerLogic newKeyerLogic(void);
extern void delKeyerLogic(KeyerLogic kl);

extern BOOLEAN klogic(KeyerLogic kl,
		      BOOLEAN dit,
		      BOOLEAN dah,
		      double wpm,
		      int iambicmode,
		      BOOLEAN need_midelemodeB,
		      BOOLEAN want_dit_mem,
		      BOOLEAN want_dah_mem,
		      BOOLEAN autocharspacing,
		      BOOLEAN autowordspacing,
		      int weight,
		      double ticklen);

//========================================================================
//========================================================================
//========================================================================

#define SAMP_RATE (48000)

/// # times key is sampled per sec
#define RTC_RATE (128)		// Note the default max_user_freq is only 64
//#define RTC_RATE (1024)

/// # samples generated during 1 clock tick at RTC_RATE
#define TONE_SIZE (SAMP_RATE / RTC_RATE)

/// ring buffer size
#define RING_SIZE (01 << 022)

KeyerState ks;
KeyerLogic kl;

pthread_t poller, play, key, update, PTT_timer;
sem_t poll_action, clock_fired, keyer_started, update_ok;
int poll_status;

int fdser, fdrtc;

jack_client_t *client;
jack_port_t *lport, *rport;
jack_ringbuffer_t *lring, *rring;
jack_nframes_t size;

CWToneGen gen;
BOOLEAN playing = FALSE, iambic = FALSE;
double wpm = 18.0, freq = 700.0, ramp = 5.0, gain = -3.0;

//========================================================================
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

int read_keys(void){
    send_command("get keys");
    return param;
}

//========================================================================
//========================================================================


/* -------------------------------------------------------------------------- */
/** @brief Read a straight key 
* 
* Get most recent input port status,
* do debouncing,
* then return the key state 
*
* @param ks 
* @return BOOLEAN
*/
/* ---------------------------------------------------------------------------- */
BOOLEAN
read_straight_key(KeyerState ks) {
  int i, j, status;
  static BOOLEAN keystate = 0;
  static int debounce_buf_i = 0,
             debounce_buf[DEBOUNCE_BUF_MAX_SIZE];

  /* Get the key state */
  sem_wait(&poll_action);
  status = poll_status;
  poll_status = 0;

  debounce_buf[debounce_buf_i] = status & (L_KEY_DOWN | R_KEY_DOWN);
  debounce_buf_i++;

  /* If the debounce buffer is full,
     determine the state of the key */
  if (debounce_buf_i >= ks->debounce) {
    debounce_buf_i = 0;

    j = 0;
    for (i = 0; i < ks->debounce; i++)
      if (debounce_buf[i])
	j++;
    keystate = (j > ks->debounce / 2) ? 1 : 0;
  }

  return keystate;
}

//------------------------------------------------------------------------


/* -------------------------------------------------------------------------- */
/** @brief Read an iambic key 
* 
* Get most recent input port status,
* do debouncing,
* emulate a straight key,
* then return the emulated key state 
*   
* @param ks 
* @param kl 
* @param ticklen 
* @return BOOLEAN
*/
/* ---------------------------------------------------------------------------- */
BOOLEAN
read_iambic_key(KeyerState ks, KeyerLogic kl, double ticklen) {
  int i, j, status;
  static BOOLEAN dah_debounce_buf[DEBOUNCE_BUF_MAX_SIZE],
                 dit_debounce_buf[DEBOUNCE_BUF_MAX_SIZE];
  static int dah = 0, debounce_buf_i = 0, dit = 0;

  /* Get the key states */
  sem_wait(&poll_action);
  status = poll_status;
  poll_status = 0;

  if (ks->flag.revpdl) {
    dit_debounce_buf[debounce_buf_i] = status & L_KEY_DOWN;
    dah_debounce_buf[debounce_buf_i] = status & R_KEY_DOWN;
  } else {
    dit_debounce_buf[debounce_buf_i] = status & R_KEY_DOWN;
    dah_debounce_buf[debounce_buf_i] = status & L_KEY_DOWN;
  }
  debounce_buf_i++;

  /* If the debounce buffer is full, determine the state of the keys */
  if (debounce_buf_i >= ks->debounce) {

    j = 0;
    for (i = 0; i < ks->debounce; i++)
      if (dah_debounce_buf[i]) j++;
    dah = (j > ks->debounce / 2) ? 1 : 0;

    j = 0;
    for (i = 0; i < ks->debounce; i++)
      if (dit_debounce_buf[i]) j++;
    dit = (j > ks->debounce / 2) ? 1 : 0;

    debounce_buf_i = 0;
  }

  return klogic(kl,
		dit,
		dah,
		ks->wpm,
		ks->mode,
		ks->flag.mdlmdB,
		ks->flag.memory.dit,
		ks->flag.memory.dah,
		ks->flag.autospace.khar,
		ks->flag.autospace.word,
		ks->weight,
		ticklen);
}

//========================================================================
/* -------------------------------------------------------------------------- */
/** @brief Function of the keyer logic 
* 
* @param kl 
* @param dit 
* @param dah 
* @param wpm 
* @param iambicmode 
* @param need_midlemodeB 
* @param want_dit_mem 
* @param want_dah_mem 
* @param autocharspacing 
* @param autowordspacing 
* @param weight 
* @param ticklen 
* @return BOOLEAN
*/
/* ---------------------------------------------------------------------------- */

BOOLEAN
klogic(KeyerLogic kl,
       BOOLEAN dit,
       BOOLEAN dah,
       double wpm,
       int iambicmode,
       BOOLEAN need_midelemodeB,
       BOOLEAN want_dit_mem,
       BOOLEAN want_dah_mem,
       BOOLEAN autocharspacing,
       BOOLEAN autowordspacing,
       int weight,
       double ticklen) {

  double ditlen = 1200 / wpm;
  int set_which_ele_time_left = NO_TIME_LEFTS_SCHED;

  /** Do we need to initialize the keyer? */
  if (!kl->flag.init) {
    kl->flag.prev.dit = dit;
    kl->flag.prev.dah = dah;
    kl->element.last = kl->element.curr = NO_ELEMENT;
    kl->element.iamb = NO_PADDLE_SQUEEZE;
    kl->element.psqam = 0;
    kl->element.altrn = 0;
    kl->time_left.midl = kl->time_left.beep = kl->time_left.elem = 0;
    kl->time_left.dlay = 0;
    kl->dlay_type = NO_DELAY;
    kl->flag.init = 1;
  }

  /** Decrement the time_lefts */
  kl->time_left.dlay -= kl->time_left.dlay > 0 ? ticklen : 0;
  if (kl->time_left.dlay <= 0) {
    /* If nothing is scheduled to play,
       and we just did a character space delay,
       and we're doing auto word spacing,
       then pause for a word space,
       otherwise resume the normal element time_left countdowns */
    if (kl->time_left.elem <= 0 &&
	kl->dlay_type == CHAR_SPACING_DELAY &&
	autowordspacing) {
      kl->time_left.dlay = ditlen * 4;
      kl->dlay_type = WORD_SPACING_DELAY;
    } else {
      kl->dlay_type = NO_DELAY;
      kl->time_left.midl -= kl->time_left.midl > 0 ? ticklen : 0;
      kl->time_left.beep -= kl->time_left.beep > 0 ? ticklen : 0;
      kl->time_left.elem -= kl->time_left.elem > 0 ? ticklen : 0;
    }
  }

  /** Are both paddles squeezed? */
  if (dit && dah) {
    kl->element.iamb = PADDLES_SQUEEZED;
    /* Are the paddles squeezed past the middle of the element? */
    if (kl->time_left.midl <= 0)
      kl->element.psqam = 1;
  } else if (!dit && !dah && kl->element.iamb == PADDLES_SQUEEZED)
    /* Are both paddles released and we had gotten a squeeze in this element? */
    kl->element.iamb = PADDLES_RELEASED;

  /** Is the current element finished? */
  if (kl->time_left.elem <= 0 && kl->element.curr != NO_ELEMENT) {
    kl->element.last = kl->element.curr;

    /** Should we insert an alternate element? */
    if (((dit && dah) ||
	 (kl->element.altrn &&
	  kl->element.iamb != PADDLES_RELEASED) ||
	 (kl->element.iamb == PADDLES_RELEASED &&
	  iambicmode == MODE_B &&
	  (!need_midelemodeB || kl->element.psqam)))) {
      if (kl->element.last == DAH)
	set_which_ele_time_left = kl->element.curr = DIT;
      else
	set_which_ele_time_left = kl->element.curr = DAH;

    } else {
      /* No more element */
      kl->element.curr = NO_ELEMENT;
      /* Do we do automatic character spacing? */
      if (autocharspacing && !dit && !dah) {
	kl->time_left.dlay = ditlen * 2;
	kl->dlay_type = CHAR_SPACING_DELAY;
      }
    }

    kl->element.altrn = 0;
    kl->element.iamb = NO_PADDLE_SQUEEZE;
    kl->element.psqam = 0;
  }

  /** Is an element not currently being played? */
  if (kl->element.curr == NO_ELEMENT) {
    if (dah)		/* Dah paddle down? */
      set_which_ele_time_left = kl->element.curr = DAH;
    else if (dit)	/* Dit paddle down? */
      set_which_ele_time_left = kl->element.curr = DIT;
  }

  /** Take the dah memory request into account */
  if (kl->element.curr == DIT &&
      !kl->flag.prev.dah &&
      dah &&
      want_dah_mem)
    kl->element.altrn = 1;

  /** Take the dit memory request into account */
  if (kl->element.curr == DAH &&
      !kl->flag.prev.dit &&
      dit &&
      want_dit_mem)
    kl->element.altrn = 1;

  /** If we had a dit or dah scheduled for after a delay,
     and both paddles are up before the end of the delay,
     and we have not requested dit or dah memory,
     forget it
     NB can't happen in full mode B */

  if (kl->time_left.dlay > 0 && !dit && !dah &&
      ((kl->element.curr == DIT && !want_dit_mem) ||
       (kl->element.curr == DAH && !want_dah_mem)))
    set_which_ele_time_left = kl->element.curr = NO_ELEMENT;

  /** Set element time_lefts, if needed */
  switch (set_which_ele_time_left) {
  case NO_ELEMENT:		/* Cancel any element */
    kl->time_left.beep = 0;
    kl->time_left.midl = 0;
    kl->time_left.elem = 0;
    break;

  case DIT:			/* Schedule a dit */
    kl->time_left.beep = (ditlen * (double) weight) / 50;
    kl->time_left.midl = kl->time_left.beep / 2;
    kl->time_left.elem = ditlen * 2;
    break;

  case DAH:			/* Schedule a dah */
    kl->time_left.beep = (ditlen * (double) weight) / 50 + ditlen * 2;
    kl->time_left.midl = kl->time_left.beep / 2;
    kl->time_left.elem = ditlen * 4;
    break;
  }

  kl->flag.prev.dit = dit;
  kl->flag.prev.dah = dah;

  return kl->time_left.beep > 0 && kl->time_left.dlay <= 0;
}

/* -------------------------------------------------------------------------- */
/** @brief Create a new KeyerState 
* @return KeyerState
*/
/* ---------------------------------------------------------------------------- */
KeyerState
newKeyerState(void) {
  return (KeyerState) safealloc(1, sizeof(KeyerStateInfo), "newKeyerState");
}

/* -------------------------------------------------------------------------- */
/** @brief Destroy a KeyerState 
* 
* @param ks 
* @return void
*/
/* ---------------------------------------------------------------------------- */
void
delKeyerState(KeyerState ks) {
  safefree((char *) ks);
}

/* -------------------------------------------------------------------------- */
/** @brief Create new Keyer Logic 
* @return KeyerState
*/
/* ---------------------------------------------------------------------------- */
KeyerLogic
newKeyerLogic(void) {
  return (KeyerLogic) safealloc(1, sizeof(KeyerLogicInfo), "newKeyerLogic");
}

/* -------------------------------------------------------------------------- */
/** @brief Destroy Keyer Logic 
* 
* @param kl 
* @return void
*/
/* ---------------------------------------------------------------------------- */
void
delKeyerLogic(KeyerLogic kl) {
  safefree((char *) kl);
}

//========================================================================

/* -------------------------------------------------------------------------- */
/** @brief Clear a jack ring buffer 
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
/** @brief jack_ringbuffer_restart
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

//------------------------------------------------------------

/* -------------------------------------------------------------------------- */
/** @brief Send a tone 
*
* generated tone -> output ringbuffer
*
* @return void
*/
/* ---------------------------------------------------------------------------- */
void
send_tone(void) {
  if (jack_ringbuffer_write_space(lring) < TONE_SIZE * sizeof(float)) {
    (void) write(2, "overrun tone\n", 13);
    jack_ringbuffer_restart(lring, TONE_SIZE * sizeof(float));
    jack_ringbuffer_restart(rring, TONE_SIZE * sizeof(float));
  } else {
    int i;
    for (i = 0; i < gen->size; i++) {
      float l = CXBreal(gen->buf, i),
	    r = CXBimag(gen->buf, i);
      jack_ringbuffer_write(lring, (char *) &l, sizeof(float));
      jack_ringbuffer_write(rring, (char *) &r, sizeof(float));
    }
  }
}

/* -------------------------------------------------------------------------- */
/** @brief Send silence 
*
* silence -> output ringbuffer
*
* @return void
*/
/* ---------------------------------------------------------------------------- */
void
send_silence(void) {
  if (jack_ringbuffer_write_space(lring) < TONE_SIZE * sizeof(float)) {
    (void) write(2, "overrun zero\n", 13);
    jack_ringbuffer_restart(lring, TONE_SIZE * sizeof(float));
    jack_ringbuffer_restart(rring, TONE_SIZE * sizeof(float));
  } else {
    int i;
    for (i = 0; i < gen->size; i++) {
      float zero = 0.0;
      jack_ringbuffer_write(lring, (char *) &zero, sizeof(float));
      jack_ringbuffer_write(rring, (char *) &zero, sizeof(float));
    }
  }
}

//------------------------------------------------------------------------


/* -------------------------------------------------------------------------- */
/** @brief Run sound thread  
*
* sound/silence generation
* tone turned on/off asynchronously
*
* @return void
*/
/* ---------------------------------------------------------------------------- */
void
sound_thread(void) {
  for (;;) {
    sem_wait(&clock_fired);

    if (playing) {
      // CWTone keeps playing for awhile after it's turned off,
      // in order to allow for a decay envelope;
      // returns FALSE when it's actually done.
      playing = CWTone(gen);
      send_tone();
    } else {
      send_silence();
      // only let updates run when we've just generated silence
      sem_post(&update_ok);
    }
  }

  pthread_exit(0);
}

//------------------------------------------------------------------------

/* -------------------------------------------------------------------------- */
/** @brief Poll the sound thread 
* @return void
*/
/* ---------------------------------------------------------------------------- */
void
poll_thread(void) {
  struct timespec req, rem;
  req.tv_sec = 0;
  req.tv_nsec = READ_KEY_INTERVAL;
  for (;;) {
    int status;
    nanosleep(&req, &rem);
    status = read_keys();
      if (status & 0x01)
	poll_status |= L_KEY_DOWN;
      if (status & 0x02)
	poll_status |= R_KEY_DOWN;
      sem_post(&poll_action);
    }

  pthread_exit(0);
}

//------------------------------------------------------------------------


/* -------------------------------------------------------------------------- */
/** @brief Run a timed delay on buffer read 
*
* basic heartbeat
* returns actual dur in msec since last tick;
* uses Linux rtc interrupts.
* other strategies will work too, so long as they
* provide a measurable delay in msec.
*
* @return double
*/
/* ---------------------------------------------------------------------------- */
double
timed_delay(void) {
  double del;
  unsigned long data;
  if (read(fdrtc, &data, sizeof(unsigned long)) == -1) {
    perror("read");
    exit(1);
  }
  del = (data >> 010) * 1000 / (double) RTC_RATE;
  return del;
}


/* -------------------------------------------------------------------------- */
/** @brief Read a key condtions 
*
* key down? (real or via keyer logic)
* 
* @param del 
* @return BOOLEAN
*
*/
/* ---------------------------------------------------------------------------- */
BOOLEAN
read_key(double del) {
  if (iambic)
    return read_iambic_key(ks, kl, del);
  else
    return read_straight_key(ks);
}

//------------------------------------------------------------------------


/* -------------------------------------------------------------------------- */
/** @brief Run key thread 
*
* main keyer loop
*
* @return void
*/
/* ---------------------------------------------------------------------------- */
void
key_thread(void) {
  BOOLEAN key_FIFO[key_FIFO_SIZE];	// FIFO buffer for keydowns
					// if RTC is 128, each element is 8 ms delay
  BOOLEAN keydown;

  int i;

  for (i = 0; i < key_FIFO_SIZE; i++) key_FIFO[i] = FALSE;
  i = 0;

  sem_wait(&keyer_started);

  for (;;) {
    double del = timed_delay();		// blocks until next rtc tick

    keydown = key_FIFO[i] = read_key(del);	// read into FIFO current key state
    if (keydown) {
	hang_time = 0;			// mark the time the start of any keydown
      	if (PTT_state == 0){		// Is PTT off?  We need to turn PTT on
		send_command("set ptt on");
		PTT_state = 1;
		}
	}

    
    if (i >= (key_FIFO_SIZE - 1)) {		// end of buffer, wrap around
	keydown = key_FIFO[0];			// take delayed keydown from FIFO
	i = 0;
	}
    else {
	keydown = key_FIFO[i+1];
	i++;
	};

    if (!playing && keydown)
      CWToneOn(gen), playing = TRUE;
    else if (playing && !keydown)
      CWToneOff(gen);

    sem_post(&clock_fired);
  }

  pthread_exit(0);
}

//------------------------------------------------------------------------


#define MAX_ESC (512)
#define ESC_L '<'
#define ESC_R '>'

/* -------------------------------------------------------------------------- */
/** @brief update thread,  update keyer parameters via text input from stdin
*
* @param<wpm xxx>  -> set keyer speed to xxx
* @param<gain xxx> -> set gain to xxx (dB)
* @param<freq xxx> -> set freq to xxx
* @param <ramp xxx> -> set attack/decay times to xxx ms
* @param <quit>     -> terminate keyer
*
* @return void
*/
/* ---------------------------------------------------------------------------- */
void
update_thread(void) {
  for (;;) {
    int c;
    int ifreq;
    char command_str[80];

    // get or wait for next input char
    if ((c = getchar()) == EOF) goto finish;

    // if we see the beginning of a command,
    if (c == ESC_L) {
      int i = 0;
      char buf[MAX_ESC];

      // gather up the remainder
      while ((c = getchar()) != EOF) {
	if (c == ESC_R) break;
	buf[i] = c;
	if (++i >= (MAX_ESC - 1)) break;
      }
      if (c == EOF) goto finish;
      buf[i] = 0;

      // wait until changes are safe
      sem_wait(&update_ok);

      if (!strncmp(buf, "wpm", 3))
	ks->wpm = wpm = atof(buf + 3);
      else if (!strncmp(buf, "ramp", 4)) {
	ramp = atof(buf + 4);
	setCWToneGenVals(gen, gain, freq, ramp, ramp);
      } else if (!strncmp(buf, "freq", 4)) {
	freq = atof(buf + 4);
        ifreq = freq;
        sprintf(command_str, "set tone %d\n", ifreq);
        send_command(command_str);
	setCWToneGenVals(gen, gain, freq, ramp, ramp);
      } else if (!strncmp(buf, "gain", 4)) {
	gain = atof(buf + 4);
	setCWToneGenVals(gen, gain, freq, ramp, ramp);
      } else if (!strncmp(buf, "quit", 4))
	goto finish;

    } // otherwise go around again
  }

  // we saw an EOF or quit; kill other threads and exit neatly

 finish:
  pthread_cancel(poller);
  pthread_cancel(play);
  pthread_cancel(key);
  pthread_cancel(PTT_timer);
  pthread_exit(0);
}

//------------------------------------------------------------------------

/* -------------------------------------------------------------------------- */
/** @brief private jack_xrun 
* 
* @param arg 
* @return void
*/
/* ---------------------------------------------------------------------------- */
PRIVATE void
jack_xrun(void *arg) {
  char *str = "xrun";
  (void) write(2, str, strlen(str));
}

/* -------------------------------------------------------------------------- */
/** @brief jack_shutdown 
* 
* @param arg 
* @return void
*/
/* ---------------------------------------------------------------------------- */
PRIVATE void
jack_shutdown(void *arg) {}

/* -------------------------------------------------------------------------- */
/** @brief jack_callback 
* 
* @param nframes 
* @param arg 
* @return void
*/
/* ---------------------------------------------------------------------------- */
PRIVATE int
jack_callback(jack_nframes_t nframes, void *arg) {
  float *lp, *rp;
  int nbytes = nframes * sizeof(float);
  if (nframes == size) {
    // output: copy from ring to port
    lp = (float *) jack_port_get_buffer(lport, nframes);
    rp = (float *) jack_port_get_buffer(rport, nframes);
    if (jack_ringbuffer_read_space(lring) >= nbytes) {
      jack_ringbuffer_read(lring, (char *) lp, nbytes);
      jack_ringbuffer_read(rring, (char *) rp, nbytes);
    } else { // rb pathology
      memset((char *) lp, 0, nbytes);
      memset((char *) rp, 0, nbytes);
      jack_ringbuffer_reset(lring);
      jack_ringbuffer_reset(rring);
      jack_ringbuffer_clear(lring, TONE_SIZE * sizeof(float));
      jack_ringbuffer_clear(rring, TONE_SIZE * sizeof(float));
      //write(2, "underrun\n", 9); 
    }
  }
  return 0;
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
  char    *clockdev = "/dev/rtc";

  int ifreq;
  char command_str[80];

  for (i = 1; i < argc; i++)
    if (argv[i][0] == '-')
      switch (argv[i][1]) {
      case 'f':
	freq = atof(argv[++i]);
	break;
      case 'i':
	iambic = TRUE;
	break;
      case 'g':
	gain = atof(argv[++i]);
	break;
      case 'r':
	ramp = atof(argv[++i]);
	break;
      case 'w':
	wpm = atof(argv[++i]);
	break;
      default:
	fprintf(stderr,
		"iambic-keyer [-i] [-w wpm] [-g gain_dB] [-r ramp_ms]\n");
	exit(1);
      }
    else break;

  if (i < argc) {
    if (!freopen(argv[i], "r", stdin))
      perror(argv[i]), exit(1);
    i++;
  }

  //------------------------------------------------------------

  gen = newCWToneGen(gain, freq, ramp, ramp, TONE_SIZE, 48000.0);

  //------------------------------------------------------------

  kl = newKeyerLogic();
  ks = newKeyerState();
  ks->flag.iambic = TRUE;
  ks->flag.revpdl = TRUE;
  // set On by default; straight key never sees them,
  // mode A users are on their own
  ks->flag.mdlmdB = ks->flag.memory.dit = ks->flag.memory.dah = TRUE;
  ks->flag.autospace.khar = ks->flag.autospace.word = FALSE;
  ks->debounce = 1;
  ks->mode = MODE_B;
  ks->weight = 50;
  ks->wpm = wpm;

  //------------------------------------------------------------

  if (!(client = jack_client_open("ikyr", JackNullOption, &jack_status))){
      fprintf(stderr, "can't make client -- jack not running?\n");
      exit(1);
  };

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
  rport = jack_port_register(client,
			     "or",
			     JACK_DEFAULT_AUDIO_TYPE,
			     JackPortIsOutput,
			     0);
  lring = jack_ringbuffer_create(RING_SIZE);
  rring = jack_ringbuffer_create(RING_SIZE);
  jack_ringbuffer_clear(lring, TONE_SIZE * sizeof(float));
  jack_ringbuffer_clear(rring, TONE_SIZE * sizeof(float));
  
  //------------------------------------------------------------

  // key
  open_usbsoftrock();
  ifreq = freq;	// double -> int
  sprintf(command_str,"set tone %d\n", ifreq);
  send_command(command_str);


  // rtc
  if ((fdrtc = open(clockdev, O_RDONLY)) == -1) {
    perror(clockdev);
    exit(1);
  }
  if (ioctl(fdrtc, RTC_IRQP_SET, RTC_RATE) == -1) {
    perror("ioctl irqp");
    exit(1);
  }
  if (ioctl(fdrtc, RTC_PIE_ON, 0) == -1) {
    perror("ioctl pie on");
    exit(1);
  }

  //------------------------------------------------------------

  sem_init(&poll_action, 0, 0);
  sem_init(&clock_fired, 0, 0);
  sem_init(&keyer_started, 0, 0);
  sem_init(&update_ok, 0, 0);
  pthread_create(&poller, 0, (void *) poll_thread, 0);
  pthread_create(&play, 0, (void *) sound_thread, 0);
  pthread_create(&key, 0, (void *) key_thread, 0);
  pthread_create(&update, 0, (void *) update_thread, 0);
  pthread_create(&PTT_timer, 0, (void *) PTT_timer_thread, 0);

  //------------------------------------------------------------

  jack_activate(client);

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
*/

  sem_post(&keyer_started);

  pthread_join(poller, 0);
  pthread_join(play, 0);
  pthread_join(key, 0);
  pthread_join(update, 0);
  pthread_join(PTT_timer, 0);
  jack_client_close(client);

  //------------------------------------------------------------

  if (ioctl(fdrtc, RTC_PIE_OFF, 0) == -1) {
    perror("ioctl pie off");
    exit(1);
  }
  close(fdrtc);
  //close(fdser);

  jack_ringbuffer_free(lring);
  jack_ringbuffer_free(rring);

  sem_destroy(&poll_action);
  sem_destroy(&clock_fired);
  sem_destroy(&keyer_started);

  delCWToneGen(gen);
  delKeyerState(ks);
  delKeyerLogic(kl);

  //------------------------------------------------------------

  exit(0);
}
