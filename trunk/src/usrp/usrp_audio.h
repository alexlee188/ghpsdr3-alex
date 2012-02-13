#if !defined __USRP_AUDIO_H__
#define      __USRP_AUDIO_H__

int usrp_audio_open    (int core_bandwidth) ;
void usrp_process_audio_buffer (float *ch1,  float *ch2, int mox);
void usrp_set_server_audio (char* setting);
int usrp_get_server_audio (void);

#endif
