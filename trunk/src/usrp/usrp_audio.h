#if !defined __USRP_AUDIO_H__
#define      __USRP_AUDIO_H__

int usrp_audio_open    (int core_bandwidth) ;
int usrp_audio_close   (void) ;
int usrp_audio_write   (float* left_samples,float* right_samples) ;
int usrp_audio_write_2 (float* left_samples,float* right_samples) ;
int usrp_audio_write_3 (float* left_samples,float* right_samples) ;
void usrp_process_output_buffer (float *ch1,  float *ch2, int mox);

#endif
