#ifndef _OPENMP
#define _OPENMP
#endif

#if !defined __USRP_AUDIO_H__
#define      __USRP_AUDIO_H__

int usrp_audio_open    (int core_bandwidth) ;
void usrp_process_audio_buffer (float *outbuf, int mox);
void usrp_set_server_audio (char* setting);
int usrp_get_server_audio (void);

/*!
 * Disables the transmission of TX or RX samples
 */
void usrp_disable_path(const char *path); 


#endif
