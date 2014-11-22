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

IQ newCorrectIQ (REAL phase, REAL gain, REAL mu_scale_dB, REAL samplerate) {
  IQ iq = (IQ) safealloc (1, sizeof (iqstate), "IQ state");
  iq->enable = 0;
  iq->is_enabled = 0;
  iq->samplerate = samplerate;
  iq->phase = phase;
  iq->gain = gain;
  iq->mu_scale_dB = mu_scale_dB;
  iq->w = Cmplx(0.0,0.0);
  iq->mu = pow(10.0, iq->mu_scale_dB / 20.0) / iq->samplerate;
  iq->sig2 = 0;
  iq->var2sig2 = 0;
  return iq;
}

void delCorrectIQ (IQ iq) {
  safefree ((char *) iq);
}

void correctIQ (CXB sigbuf, IQ iq, BOOLEAN isTX, int subchan) {
  // see if enable state has changed
  if (iq->enable != iq->is_enabled) {
    // copy enable state 
    iq->is_enabled = iq->enable;
    // generate a log line
    if (iq->is_enabled) {
      // and restart filter from zero when enabled
      iq->w = Cmplx(0.0,0.0);
      iq->mu = pow(10.0, iq->mu_scale_dB / 20.0) * CXBhave(sigbuf)  / iq->samplerate;
      iq->sig2 = 0;
      iq->var2sig2 = 0;
    } else {
      fprintf(stderr, "correctIQ: enable %d, scale %10g, w (%10g,%10g), mu %10g, sig2 %10g, var2 %10g\n",
	      iq->enable, iq->mu_scale_dB, iq->w.re, iq->w.im, iq->mu, iq->sig2, iq->var2sig2);
    }
  }
  // if we are enabled
  if (iq->is_enabled) {
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
      COMPLEX y0;      /* uncorrected input sample value */
      COMPLEX y1;      /* corrected output sample value */
      REAL s2;	       /* squared magnitude */
      REAL sig2 = 0.0; /* sum squared magnitude signal level of buffer */
      REAL var2sig2 = 0.0;	/* sum variance squared */

      for (i = 0; i < CXBhave (sigbuf); i++) {
	y0 = CXBdata(sigbuf, i);			/* get incoming sample */
	y1 = Cadd(y0, Cmul(iq->w, Conjg(y0)));		/* adjust incoming sample */
	CXBdata(sigbuf, i) = y1;			/* store adjusted sample */
	iq->w = Csub(iq->w, Cscl(Cmul(y1, y1), iq->mu));/* adapt the filter */
	s2 = Csqrmag(y1);				/* compute squared magnitude */
	sig2 += s2;					/* accumulate squared signal */
	var2sig2 += sqr(s2-iq->sig2);			/* accumulate squared variance of squared signal */
	/* the Csqrmag(y1) computation could share multiplies with Cmul(y1, y1) */
      }

      // if the filter blew up
      if (isnan(y1.re) || isnan(y1.im) || Csqrmag(y1) >= 1) {
	// report blow up
	fprintf(stderr, "correctIQ: blow up, scale %10g, w (%10g,%10g), mu %10g, sig2 %10g, var2 %10g\n",
		iq->mu_scale_dB, iq->w.re, iq->w.im, iq->mu, iq->sig2, iq->var2sig2);
	// then reset iq->_w 
	iq->w = Cmplx(0.0,0.0);
	// and zero the sample buffer
	for (i = 0; i < CXBhave (sigbuf); i++)
	  CXBdata(sigbuf, i) = Cmplx(0.0,0.0);
	// and we'll start over again next buffer
      } else {
	// adjust iq->_mu
	// (sig2 / CXBhave(sigbuf)) is the actual average unscaled correction per sample
	// (1.0 / iq->samplerate) is the desired average scaled correction per sample
	// (1.0 / iq->samplerate) / (sig2 / CXBhave(sigbuf)) simplifies to
	// CXBhave(sigbuf)  / (iq->samplerate * sig2)
	// and then we scale by pow(10.0, iq->mu_scale_dB / 20.0)
	sig2 /= CXBhave(sigbuf);
	var2sig2 /= CXBhave(sigbuf);
	iq->mu = pow(10.0, iq->mu_scale_dB / 20.0) / (iq->samplerate * sig2);
	iq->sig2 += sig2;
	iq->sig2 /= 2;
	iq->var2sig2 += var2sig2;
	iq->var2sig2 /= 2;
      }
    }
  }
}
