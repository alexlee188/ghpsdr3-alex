//
//
// This file has been excerpted from Quisk 3.5.11 by James C. Ahlstrom
// http://james.ahlstrom.name/quisk/
//
// In the original file there is no license.
//

#if !defined __SDRIQ_H__
#define      __SDRIQ_H__


// These are decimation factors, scale factors and filter coefficients for the SDR-IQ.
// They are used to program the AD6620 chip.

struct ad6620 {
	int Mcic2;
	int Mcic5;
	int Mrcf;
	int Scic2;
	int Scic5;
	int Sout;
	int coef[256];
} ;


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _quisk_sound_state {
    int read_error;
    int overrange;
    int data_poll_usec;
} QUISK_SOUND_STATE;


const QUISK_SOUND_STATE *get_state (void);

typedef float SAMPLE_T ;

void quisk_start_sdriq(void);

// Stop sample capture; called from the sound thread.
void quisk_stop_sdriq(void);

// Called in a loop to read samples; called from the sound thread.
//int quisk_read_sdriq (complex * cSamples);

// Called to close the sample source; called from the GUI thread.
void close_samples ();

int open_samples(const char *n, const char *clock, char* buf);


// Miscellaneous functions needed by the SDR-IQ; called from the GUI thread as
// a result of button presses.

// Set the receive frequency; called from the GUI thread.
void freq_sdriq (int f);

// Set the preamp gain; called from the GUI thread.
// gstate == 0:  Gain must be 0, -10, -20, or -30                
// gstate == 1:  Attenuator is on  and gain is 0 to 127 (7 bits) 
// gstate == 2:  Attenuator is off and gain is 0 to 127 (7 bits) 

void gain_sdriq (int gain_state, int gain);

// Set the decimation; called from the GUI thread.
void set_decimation (int decimation);

// set the bandwidth
void set_bandwidth (int nb);


typedef int (*SDRIQ_CB)(SAMPLE_T *ii, SAMPLE_T *qq, int bsize, void *data) ;

int sdriq_start_asynch_input (SDRIQ_CB cb, void *pud);
int sdriq_stop_asynch_input ();

const char *get_serial();

#ifdef __cplusplus
}
#endif


#endif


