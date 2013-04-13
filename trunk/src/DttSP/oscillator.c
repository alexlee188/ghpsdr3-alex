/* oscillator.c 

This routine implements a common fixed-frequency oscillator

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006 by Frank Brickle, AB2KT and Bob McGwier, N4HY

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

The authors can be reached by email at

ab2kt@arrl.net
or
rwmcgwier@comcast.net

or by paper mail at

The DTTS Microwave Society
6 Kathleen Place
Bridgewater, NJ 08807

Modified to implement an occasionally renormalized complex oscillator
which only uses trig at initialization 2012-02-06.

Copyright (C) 2012 by Roger E Critchlow Jr, rec@elf.org, ad5dz

Released under the same terms as the original source.

*/

#include <common.h>
#define HUGE_PHASE 1256637061.43593
/*
void
ComplexOSC (OSC p)
{
  int i;
  COMPLEX dz = OSCdz(p);
  if (--OSCclock(p) == 0) {
    OSCz(p) = Cscl(OSCz (p), 1.0/Cmag (OSCz (p)));
    OSCclock(p) = RENORMALIZATION_CLOCK;
  }
  for (i = 0; i < OSCsize (p); i++) {
    OSCz(p) = Cmul (OSCz(p), dz);
    CXBdata ((CXB) OSCbase (p), i) = OSCz(p);
  }
}

void
RealOSC (OSC p)
{
  int i;
  COMPLEX dz = OSCdz(p);
  if (--OSCclock (p) == 0) {
    OSCz (p) = Cscl(OSCz (p), 1.0/Cmag (OSCz (p)));
    OSCclock (p) = RENORMALIZATION_CLOCK;
  }
  for (i = 0; i < OSCsize (p); i++) {
    OSCz (p) = Cmul (OSCz (p), dz);
    OSCRdata (p, i) = c_im (OSCz (p));
  }
}

void
fixOSC(OSC p, double Frequency, double Phase, REAL SampleRate)
{
  setFreqOSC(p, Frequency, SampleRate);
  setPhaseOSC(p, Phase);
}

void
setFreqOSC(OSC p, double Frequency, REAL SampleRate)
{
  double radians_per_sample = 2.0 * M_PI * Frequency / SampleRate;
  COMPLEX dz = Cmplx((REAL) cos(radians_per_sample), (IMAG) sin(radians_per_sample));
  OSCdz (p) = dz;
  OSCon(p) = Frequency != 0.0;
}

void
setPhaseOSC(OSC p, double Phase)
{
  OSCz (p) = Cmplx((REAL) cos(Phase), (IMAG) sin(Phase));
}

OSC
newOSC (int size,
	OscType TypeOsc,
	double Frequency, double Phase, REAL SampleRate, char *tag)
{
  OSC p = (OSC) safealloc (1, sizeof (oscillator), tag);
  double freq;
  if ((OSCtype (p) = TypeOsc) == ComplexTone)
    OSCbase (p) = (void *) newCXB (size,
				   NULL,
				   "complex buffer for oscillator output");
  else
    OSCbase (p) = (void *) newRLB (size,
				   NULL, "real buffer for oscillator output");
  OSCsize (p) = size;
  fixOSC(p, Frequency, Phase, SampleRate);
  OSCclock (p) = 1;
  return p;
}
*/
void
ComplexOSC (OSC p)
{
  int i;
  COMPLEX z, delta_z;

  if (OSCphase (p) > TWOPI)
    OSCphase (p) -= TWOPI;
  else if (OSCphase (p) < -TWOPI)
    OSCphase (p) += TWOPI;

  z = Cmplx ((REAL) cos (OSCphase (p)), (IMAG) sin (OSCphase (p))),
    delta_z = Cmplx ((REAL) cos (OSCfreq (p)), (IMAG) sin (OSCfreq (p)));

  for (i = 0; i < OSCsize (p); i++)
    /*z = CXBdata ((CXB) OSCbase (p), i)
      = Cmul (z, delta_z), OSCphase (p) += OSCfreq (p);*/
	  CXBdata((CXB)OSCbase(p), i) = Cmplx ((REAL) cos (OSCphase (p)), (IMAG) sin (OSCphase (p))),
		OSCphase (p) += OSCfreq (p);

}

#ifdef notdef
void
ComplexOSC (OSC p)
{
  int i;
/*  if (OSCphase (p) > 1256637061.43593)
    OSCphase (p) -= 1256637061.43593; */
  for (i = 0; i < OSCsize (p); i++)
    {
      OSCreal (p, i) = cos (OSCphase (p));
      OSCimag (p, i) = sin (OSCphase (p));
      OSCphase (p) += OSCfreq (p);
    }
}
#endif

PRIVATE double
_phasemod (double angle)
{
  while (angle >= TWOPI)
    angle -= TWOPI;
  while (angle < 0.0)
    angle += TWOPI;
  return angle;
}

void
RealOSC (OSC p)
{
  int i;
  for (i = 0; i < OSCsize (p); i++)
    {
      OSCRdata (p, i) = (REAL) sin (OSCphase (p));
      OSCphase (p) = _phasemod (OSCfreq (p) + OSCphase (p));
    }
}

OSC
newOSC (int size,
	OscType TypeOsc,
	double Frequency, double Phase, REAL SampleRate, char *tag)
{
  OSC p = (OSC) safealloc (1, sizeof (oscillator), tag);
  if ((OSCtype (p) = TypeOsc) == ComplexTone)
    OSCbase (p) = (void *) newCXB (size,
				   NULL,
				   "complex buffer for oscillator output");
  else
    OSCbase (p) = (void *) newRLB (size,
				   NULL, "real buffer for oscillator output");
  OSCsize (p) = size;
  OSCfreq (p) = 2.0 * M_PI * Frequency / SampleRate;
  OSCphase (p) = Phase;
  return p;
}

void
delOSC (OSC p)
{
  if (p->OscillatorType == ComplexTone)
    delCXB ((CXB) p->signalpoints);
  else
    delRLB ((RLB) p->signalpoints);
  if (p)
    safefree ((char *) p);
}
