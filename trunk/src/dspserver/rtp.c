/** 
* @file rtp.c
* @brief rtp network interface
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-12
*/
// rtp.c

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

/* Copyright (C) - 2012 - Alex Lee, 9V1Al
*  modifications of the original program by John Melton
* 
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
* Foundation, Inc., 59 Temple Pl
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <ortp/ortp.h>

#include "rtp.h"

static RtpSession* rtpSession = 0;
int recv_ts=0;
int send_ts=0;
int rtp_receive_has_more = 0;
int jittcomp=40;
int adapt=1;

       int rtp_connected   = 0;
static int rtp_initialized = 0;
static int rtp_listening   = 0;

static sem_t rtp_semaphore;

void rtp_init() {
    sem_init(&rtp_semaphore,0,1);
    sem_post(&rtp_semaphore);

    if (rtp_initialized) {
       fprintf (stderr, "rtp_init: WARNING: double init discarded !!!!!!\n");
       return;
    } else {
       fprintf (stderr, "rtp_init: init ! ****************** \n");
    }
    ortp_init();
    ortp_scheduler_init();
    ortp_set_log_file (stdout);
    ortp_set_log_level_mask(ORTP_DEBUG|ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR);
    jittcomp=40;
    adapt=1;

    rtp_connected   = 0;
    rtp_listening   = 0;
    rtp_initialized = 1;
}

int rtp_listen(const char *remote_addr, unsigned short remote_port) {

    if (rtp_initialized == 0) {
       fprintf (stderr, "rtp_listen: ERROR: attempting to start to listen without init !!!!!!\n");
       return -1;
    } 
    if (rtp_listening) {
       fprintf (stderr, "rtp_listen: WARNING: multiple listening discarded !!!!!!\n");
       if (remote_addr) rtp_session_set_remote_addr	(rtpSession, remote_addr, remote_port );
       fprintf(stderr,"RTP initialized socket=%d local port=%d remote port: %d remote_addr: %s\n",
            rtp_session_get_rtp_socket(rtpSession),rtp_session_get_local_port(rtpSession),
            remote_port, remote_addr
       );
       return rtp_session_get_local_port(rtpSession);
    } else {
       fprintf (stderr, "rtp_listen: listening ! ****************** \n");
    }

    recv_ts=0;
    send_ts=0;
    rtpSession=rtp_session_new(RTP_SESSION_SENDRECV);
    rtp_session_set_scheduling_mode(rtpSession,TRUE);
    rtp_session_set_blocking_mode(rtpSession,TRUE);

    rtp_session_set_local_addr(rtpSession,"0.0.0.0",5004);
    if (remote_addr) rtp_session_set_remote_addr	(rtpSession, remote_addr, remote_port );

    rtp_session_set_connected_mode(rtpSession,TRUE);
    rtp_session_set_symmetric_rtp(rtpSession,TRUE);

    rtp_session_enable_adaptive_jitter_compensation(rtpSession,adapt);
    rtp_session_set_jitter_compensation(rtpSession,jittcomp);
    rtp_session_set_payload_type(rtpSession,0);
    //rtp_session_signal_connect(rtpSession,"ssrc_changed",(RtpCallback)ssrc_cb,0);
    rtp_session_signal_connect(rtpSession,"ssrc_changed",(RtpCallback)rtp_session_reset,0);
    fprintf(stderr,"RTP initialized socket=%d local port=%d remote port: %d remote_addr: %s\n",
            rtp_session_get_rtp_socket(rtpSession),rtp_session_get_local_port(rtpSession),
            remote_port, remote_addr
           );

    sem_wait(&rtp_semaphore);
    rtp_listening = 1;
    rtp_connected = 1;
    sem_post(&rtp_semaphore);
    return rtp_session_get_local_port(rtpSession);
}

void rtp_disconnect() {
   
    if (rtp_listening == 0) {
       fprintf (stderr, "rtp_disconnect: ERROR: attempting to disconnect without listening !!!!!!");
       return;
    }

    sem_wait(&rtp_semaphore);
    rtp_connected=0;
    rtp_listening = 0;
    ortp_global_stats_display();
    rtp_session_destroy(rtpSession);
    sem_post(&rtp_semaphore);

}

void rtp_send(char* buffer,int length) {
    int rc;

    if (rtp_initialized == 0) {
       fprintf (stderr, "rtp_listen: ERROR: attempting to send without init !!!!!!");
       return;
    }

    sem_wait(&rtp_semaphore);
    if(rtp_connected)  {
        rc=rtp_session_send_with_ts(rtpSession,(uint8_t*)buffer,length,send_ts);
        if(rc<=0) {
            fprintf(stderr,"rtp_send: ERROR rc=%d\n",rc);
        }
        send_ts+=length;
    }
    sem_post(&rtp_semaphore);
}

int rtp_receive (unsigned char* buffer,int length) {
    int rc = -1;
    
    if (rtp_initialized == 0) {
       fprintf (stderr, "rtp_receive: ERROR: attempting to receive without init !!!!!!");
       return rc;
    }

    if (rtp_listening == 0) {
       return rc;
    }

    if (rtp_connected){
	    rc=rtp_session_recv_with_ts(rtpSession,(uint8_t*)buffer,length,recv_ts,&rtp_receive_has_more);
	    if(rc > 0) {
		recv_ts+=length;
	    }
    }
    return rc;
}
