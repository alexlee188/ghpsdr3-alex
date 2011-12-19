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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <ortp/ortp.h>

#include "rtp.h"

RtpSession* rtpSession;
int rtp_connected=0;
int recv_ts=0;
int send_ts=0;
int has_more;
int jittcomp=40;
int adapt=1;

void rtp_init() {
    ortp_init();
    ortp_scheduler_init();
    ortp_set_log_level_mask(ORTP_DEBUG|ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR);
    jittcomp=40;
    adapt=1;
    rtp_connected=0;
}

int rtp_connect(char* host,int port) {
    recv_ts=0;
    send_ts=0;
    rtpSession=rtp_session_new(RTP_SESSION_SENDRECV);
    rtp_session_set_scheduling_mode(rtpSession,1);
    rtp_session_set_blocking_mode(rtpSession,1);
    //rtp_session_set_blocking_mode(rtpSession,0);
    //rtp_session_set_local_addr(rtpSession,"127.0.0.1",0);
    rtp_session_set_connected_mode(rtpSession,TRUE);
    rtp_session_set_remote_addr(rtpSession,host,port);
    rtp_session_set_symmetric_rtp(rtpSession,TRUE);
    rtp_session_enable_adaptive_jitter_compensation(rtpSession,adapt);
    rtp_session_set_jitter_compensation(rtpSession,jittcomp);
    rtp_session_set_payload_type(rtpSession,0);
    //rtp_session_signal_connect(rtpSession,"ssrc_changed",(RtpCallback)ssrc_cb,0);
    rtp_session_signal_connect(rtpSession,"ssrc_changed",(RtpCallback)rtp_session_reset,0);

    fprintf(stderr,"RTP initialized socket=%d port=%d\n",rtp_session_get_rtp_socket(rtpSession),rtp_session_get_local_port(rtpSession));

    rtp_connected=1;

    return rtp_session_get_local_port(rtpSession);
}

void rtp_disconnect() {
    rtp_session_destroy(rtpSession);
    rtp_connected=0;
    ortp_global_stats_display();
}

void rtp_send(char* buffer,int length) {
    int rc;
    if(rtp_connected)  {
        rc=rtp_session_send_with_ts(rtpSession,(uint8_t*)buffer,length,send_ts);
        if(rc<=0) {
            fprintf(stderr,"RTP:send rc=%d\n",rc);
        }
        send_ts+=length;
    } else {
        fprintf(stderr,"rtp_send: not connected\n");
    }
}

int rtp_receive(unsigned char* buffer,int length) {
    int rc = -1;

    if (rtp_connected) {
    	rc=rtp_session_recv_with_ts(rtpSession,(uint8_t*)buffer,length,recv_ts,&has_more);
    	recv_ts+=length;
	}
//fprintf(stderr,"rcp_receive: %d has_more=%d recv_ts=%d\n",rc,has_more,recv_ts);
    return rc;
}
