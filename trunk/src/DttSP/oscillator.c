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

void
ComplexOSC (OSC p)
{
  int i;
  if (--OSCclock(p) == 0)
    {
      OSCphase (p) = Cscl(OSCphase (p), 1.0/Cmag (OSCphase (p)));
      OSCclock(p) = RENORMALIZATION_CLOCK;
    }
  for (i = 0; i < OSCsize (p); i++)
    {
      OSCphase(p) = Cmul (OSCphase(p), OSCdphase(p));
      CXBdata ((CXB) OSCbase (p), i) = OSCphase(p);
    }
}

void
RealOSC (OSC p)
{
  int i;
  if (--OSCclock (p) == 0)
    {
      OSCphase (p) = Cscl(OSCphase (p), 1.0/Cmag (OSCphase (p)));
      OSCclock (p) = RENORMALIZATION_CLOCK;
    }
  for (i = 0; i < OSCsize (p); i++)
    {
      OSCphase (p) = Cmul (OSCphase (p), OSCdphase (p));
      OSCRdata (p, i) = c_im (OSCphase (p));
    }
}

void
resetOSC(OSC p, double Frequency, double Phase, REAL SampleRate)
{
  OSCfreq (p) = 2.0 * M_PI * Frequency / SampleRate;
  OSCphase (p) = Cmplx((REAL) cos(Phase), (IMAG) sin(Phase));
  OSCdphase (p) = Cmplx((REAL) cos(OSCfreq(p)), (IMAG) sin(OSCfreq(p)));;
  OSCclock (p) = 1;
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
  resetOSC(p, Frequency, Phase, SampleRate);
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
