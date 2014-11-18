/* correctIQ.c

This routine restores quadrature between arms of an analytic signal
possibly distorted by ADC hardware.

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
*/

#include <common.h>

IQ newCorrectIQ (REAL phase, REAL gain, REAL mu) {
  IQ iq = (IQ) safealloc (1, sizeof (iqstate), "IQ state");
  iq->samplerate = 48000;
  iq->phase = phase;
  iq->gain = gain;
  iq->mu = mu;
  iq->_w = Cmplx(0.0,0.0);
  iq->_mu = iq->mu / iq->samplerate;
  return iq;
}

void delCorrectIQ (IQ iq) {
  safefree ((char *) iq);
}

void correctIQ (CXB sigbuf, IQ iq, BOOLEAN isTX, int subchan) {
  // see if enable state has changed
  if (iq->enable != iq->_enable) {
    // copy enable state 
    iq->_enable = iq->enable;
    if (iq->_enable) {
      // and restart filter from zero when enabled
      iq->_w = Cmplx(0.0,0.0);
    }
  }
  // if we are enabled
  if (iq->_enable) {
    int i;
    // if we are a transmitter
    if (isTX) {
      // we are a transmitter
      // apply specified fixed phase and gain correction
      for (i = 0; i < CXBhave (sigbuf); i++) {
	CXBimag (sigbuf, i) += iq->phase * CXBreal (sigbuf, i);
	CXBreal (sigbuf, i) *= iq->gain;
      }
    } else {
      // we are a receiver
      REAL delta2 = 0.0;       /* sum squared magnitude signal level of buffer */
      COMPLEX y0;	       /* uncorrected input sample value */
      COMPLEX y1;	       /* corrected output sample value */
      for (i = 0; i < CXBhave (sigbuf); i++) {
	y0 = CXBdata(sigbuf, i);			/* get incoming sample */
	y1 = Cadd(y0, Cmul(iq->_w, Conjg(y0)));		/* adjust incoming sample */
	CXBdata(sigbuf, i) = y1;			/* store adjusted sample */
	iq->_w = Csub(iq->_w, Cscl(Cmul(y1, y1), iq->_mu));	/* adapt the filter */
	delta2 += Csqrmag(y1);				/* accumulate squared signal */
	/* the Csqrmag(y1) computation could share multiplies with Cmul(y1, y1) */
      }
      // if the filter blew up
      if (isnan(y1.re) || isnan(y1.im) || fabs(y1.re) + fabs(y1.im) >= 1) {
	// then reset iq->_w 
	iq->_w = Cmplx(0.0,0.0);
	// and zero the sample buffer
	for (i = 0; i < CXBhave (sigbuf); i++)
	  CXBdata(sigbuf, i) = Cmplx(0.0,0.0);
	// and we'll start over again next buffer
      } else {
	// adjust iq->_mu
	// (delta2 / CXBhave(sigbuf)) is the actual average unscaled correction per sample
	// (1.0 / iq->samplerate) is the desired average scaled correction per sample
	// but when the filter really converges, then delta2 becomes ~ equal to 0
	// and mu should not be inflated to increase delta2 when we converge.
	iq->_mu = (1.0  / iq->samplerate) / (delta2 / CXBhave(sigbuf)) * iq->mu;
      }
    }
  }
}
