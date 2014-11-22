#ifndef _correctIQ_h
#define _correctIQ_h

#include <bufvec.h>

typedef struct _iqstate
{
  REAL samplerate, phase, gain, mu, mu_scale_dB, sig2, var2sig2;
  COMPLEX w;
  BOOLEAN enable, is_enabled;
} *IQ, iqstate;

extern IQ newCorrectIQ (REAL phase, REAL gain, REAL mu_scale_dB, REAL samplerate);
extern void delCorrectIQ (IQ iq);
extern void correctIQ (CXB sigbuf, IQ iq, BOOLEAN isTX, int subchan);
#endif
