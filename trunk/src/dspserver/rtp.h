/** 
* @file rtp.h
* @brief rtp network interface
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-12
*/
// rtp.h

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

#define LOCAL_RTP_PORT 5004
#define LOCAL_RTCP_PORT 5005

void rtp_init();
RtpSession* rtp_listen(const char *remote_addr, unsigned short remote_port);
void rtp_disconnect(RtpSession* session);
void rtp_send(RtpSession *session,unsigned char* buffer,int length);
int rtp_receive(RtpSession * session,unsigned char* buffer,int length);

extern unsigned int recv_ts;
extern unsigned int send_ts;
extern int rtp_connected;
extern int rtp_listening;
