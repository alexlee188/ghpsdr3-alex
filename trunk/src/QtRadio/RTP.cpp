#include <QDebug>
#include <QThread>

#include "RTP.h"

RTP::RTP() {
qDebug() << "RTP::RTP";
    initialized=0;
    remote_set=0;
    jittcomp=100;
    adapt=TRUE;
    send_ts=0;
    timestamp_jump_limit = 500;

    ortp_init();
    ortp_scheduler_init();
    ortp_set_log_level_mask(ORTP_DEBUG|ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR);

}

RTP::~RTP() {
}

int RTP::init(const char* host,int port) {
    //signal(SIGINT,stop_handler);

    qDebug() << "RTP::init host [" << host << "] port [" << port << "]";
    rtpSession=rtp_session_new(RTP_SESSION_SENDRECV);
    rtp_session_set_scheduling_mode(rtpSession,1);
    rtp_session_set_blocking_mode(rtpSession,1);
    rtp_session_set_local_addr(rtpSession,"0.0.0.0",5004);

    rtp_session_set_remote_addr(rtpSession,host,port);
    qDebug() << "RTP connect to remote: [" << host << "] [" << port << "]";

    rtp_session_set_connected_mode(rtpSession,TRUE);
    rtp_session_set_symmetric_rtp(rtpSession,TRUE);

    rtp_session_enable_adaptive_jitter_compensation(rtpSession,adapt);
    rtp_session_set_jitter_compensation(rtpSession,jittcomp);
    rtp_session_set_payload_type(rtpSession,0);
    rtp_session_set_time_jump_limit	(rtpSession, timestamp_jump_limit);
    rtp_session_signal_connect(rtpSession,"ssrc_changed",(RtpCallback)rtp_session_reset,0);
    rtp_session_signal_connect(rtpSession,"timestamp_jump",(RtpCallback)rtp_session_resync,0);

    qDebug() << "RTP initialized socket=" <<  rtp_session_get_rtp_socket(rtpSession) 
             << " local port= " << rtp_session_get_local_port(rtpSession);

#if 1
    /*
     *  send first packet in order to help to establish session
     */
    unsigned char fake [] = "AAAAAAAAAAAAAAAA";
    rtp_session_send_with_ts(rtpSession,(uint8_t*)fake,16,send_ts);
#endif
    emit rtp_set_session(rtpSession);

    initialized=1;

    return rtp_session_get_local_port(rtpSession);
}

#if 0
void RTP::setRemote(char* host,int port) {
    qDebug() << "RTP::setRemote " << host <<":" << port;
    rtp_session_set_remote_addr(rtpSession,host,port);
    remote_set=1;
}
#endif


void RTP::shutdown(){
        rtp_session_destroy(rtpSession);
        ortp_global_stats_display();
        initialized = 0;
        remote_set=0;
}

void RTP::send(unsigned char* buffer,int length) {
    if(initialized)  {
        rtp_session_send_with_ts(rtpSession,(uint8_t*)buffer,length,send_ts);
        send_ts+=length;
    }
    if (buffer != NULL) free(buffer);
}

void RTP::dump_buffer(unsigned char* buffer,int length) {
    int i;
    QString output;
    QString hex;
    for(i=0;i<length;i++) {
        if(i%16==0) {
            if(i>0) qDebug() << output;
            output.sprintf("%04X: ",i);
        }
        hex.sprintf("%02X",buffer[i]);
        output+=hex;
    }
    if(i>0) qDebug() << output;
}
