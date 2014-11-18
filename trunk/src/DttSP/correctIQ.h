#ifndef _correctIQ_h
#define _correctIQ_h

#include <bufvec.h>

typedef struct _iqstate
{
  REAL samplerate, phase, gain, mu, _mu;
  COMPLEX _w;
  BOOLEAN enable, _enable;
} *IQ, iqstate;

extern IQ newCorrectIQ (REAL phase, REAL gain, REAL mu);
extern void delCorrectIQ (IQ iq);
extern void correctIQ (CXB sigbuf, IQ iq, BOOLEAN isTX, int subchan);
#endif
