/*  anf.c

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2012 Warren Pratt, NR0V

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

The author can be reached by email at  

warren@wpratt.com

*/

#include <common.h>
#include <math.h>
#include <anf.h>

ANF newANF	(
				int buff_size,
				COMPLEX *buff,
				int dline_size,
				int n_taps,
				int delay,
				double two_mu,
				double gamma,

				double lidx,
				double lidx_min,
				double lidx_max,
				double ngamma,
				double den_mult,
				double lincr,
				double ldecr,
				char *tag
			)
{
	ANF a;
	a = (ANF) safealloc (1,sizeof(anf), tag);

	a->buff_size = buff_size;
	a->buff = buff;
	a->dline_size = dline_size;
	a->mask = dline_size - 1;
	a->n_taps = n_taps;
	a->delay = delay;
	a->two_mu = two_mu;
	a->gamma = gamma;
	
	a->lidx = lidx;
	a->lidx_min = lidx_min;
	a->lidx_max = lidx_max;
	a->ngamma = ngamma;
	a->den_mult = den_mult;
	a->lincr = lincr;
	a->ldecr = ldecr;
	
	memset (a->d, 0, sizeof(double) * DLINE_SIZE);
	memset (a->w, 0, sizeof(double) * DLINE_SIZE);
	
	return a;
}

void del_anf (ANF a)
{
	if (a)
		safefree((char *) a);
}

    void notch(ANF a)
    {
        int i, j, idx;
        double c0, c1;
        double y, error, sigma, inv_sigp;
		double nel, nev;
        
        for (i = 0; i < a->buff_size; i++)
        {
            a->d[a->in_idx] = (double)a->buff[i].re;

            y = 0;
            sigma = 0;

            for (j = 0; j < a->n_taps; j++)
            {
                idx = (a->in_idx + j + a->delay) & a->mask;
                y += a->w[j] * a->d[idx];
                sigma += a->d[idx] * a->d[idx];
            }
			inv_sigp = 1.0 / (sigma + 1e-10);
            error = a->d[a->in_idx] - y;

            a->buff[i].re = (REAL)error;
            a->buff[i].im = 0.0f;

	    if((nel = error * (1.0 - a->two_mu * sigma * inv_sigp)) < 0.0) nel = -nel;
	    if((nev = a->d[a->in_idx] - (1.0 - a->two_mu * a->ngamma) * y - a->two_mu * error * sigma * inv_sigp) < 0.0) nev = -nev;
            if (nev < nel)
            {
		if((a->lidx += a->lincr) > a->lidx_max) a->lidx = a->lidx_max;
            }
            else
		if((a->lidx -= a->ldecr) < a->lidx_min) a->lidx = a->lidx_min;
            a->ngamma = a->gamma * (a->lidx * a->lidx) * (a->lidx * a->lidx) * a->den_mult;

            c0 = 1.0 - a->two_mu * a->ngamma;
            c1 = a->two_mu * error * inv_sigp;

            for (j = 0; j < a->n_taps; j++)
            {
                idx = (a->in_idx + j + a->delay) & a->mask;
                a->w[j] = c0 * a->w[j] + c1 * a->d[idx];
            }
            a->in_idx = (a->in_idx + a->mask) & a->mask;
        }
    }
