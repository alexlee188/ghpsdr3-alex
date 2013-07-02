/* fm_demod.h */

#ifndef _fm_demod_h
#define _fm_demod_h

#include <fromsys.h>
#include <banal.h>
#include <splitfields.h>
#include <datatypes.h>
#include <bufvec.h>
#include <cxops.h>
#include <fastrig.h>
#include <update.h>
#include <lmadf.h>
#include <fftw3.h>
#include <fftw3_fix.h>
#include <ovsv.h>
#include <filter.h>
#include <oscillator.h>
#include <filt2p2z.h>

typedef struct _fm_demod
{
  int size;
  CXB ibuf, obuf, squelch_obuf;
  REAL deviation;
  REAL k_deemphasis;
  REAL deemphasis_out;
  REAL squelch_envelope;
  REAL squelch_filter;
  REAL squelch_threshold_weak;
  REAL squelch_threshold_strong;
  REAL squelch_threshold_unmute;
  int squelch_weak_timer;
  int squelch_strong_timer;
  REAL squelch_k;
  BOOLEAN squelch_muted;
  int squelch_strong_timer_hang;
  int squelch_weak_timer_hang;
  IIR_LPF_2P input_LPF1;
  IIR_LPF_2P input_LPF2;
  IIR_HPF_2P input_HPF1;
  IIR_HPF_2P input_HPF2;
  IIR_HPF_2P squelch_HPF;
  IIR_1P1Z demod_comp_filter;

  struct
  {
    REAL alpha, beta;
    struct
    {
      REAL f, l, h;
    } freq;
    REAL phs;
    struct
    {
      REAL alpha;
    } iir;
    COMPLEX delay;
  } pll;

  REAL lock, afc, cvt;
} FMDDesc, *FMD;

extern void FMDemod (FMD fm);
extern FMD newFMD (REAL samprate,
		   REAL f_initial,
		   REAL f_lobound,
		   REAL f_hibound,
		   REAL f_bandwid,
		   int size, COMPLEX * ivec, COMPLEX * ovec, char *tag);
extern void delFMD (FMD fm);

#ifndef TWOPI
#define TWOPI (2.0*M_PI)
#endif

#endif
