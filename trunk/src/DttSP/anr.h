/*  anr.h

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
#ifndef _anr_h

#define _anr_h

#define DLINE_SIZE 1024

typedef struct _anr
{
	int buff_size;
	COMPLEX *buff;
	int dline_size;
	int mask;
	int n_taps;
	int delay;
	double two_mu;
	double gamma;
	double d [DLINE_SIZE];
	double w [DLINE_SIZE];
	int in_idx;

	double lidx;
	double lidx_min;
	double lidx_max;
	double ngamma;
	double den_mult;
	double lincr;
	double ldecr;

	char tag[4];
} anr, *ANR;

extern ANR newANR	(
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
			);

extern void del_anr (ANR a);

extern void noise_reduce (ANR a);

#endif