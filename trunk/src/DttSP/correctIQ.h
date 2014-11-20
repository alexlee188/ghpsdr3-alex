#ifndef _correctIQ_h
#define _correctIQ_h

#include <bufvec.h>

typedef struct _iqstate
{
  REAL samplerate, phase, gain, mu, log10_mu_scale, sig2;
  COMPLEX w;
  BOOLEAN enable, is_enabled;
} *IQ, iqstate;

extern IQ newCorrectIQ (REAL phase, REAL gain, REAL log10_mu_scale);
extern void delCorrectIQ (IQ iq);
extern void correctIQ (CXB sigbuf, IQ iq, BOOLEAN isTX, int subchan);
#endif
