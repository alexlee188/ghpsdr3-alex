/*  wcpAGC.h

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2011, 2012 Warren Pratt, NR0V

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

or by paper mail at

Warren Pratt
11303 Empire Grade
Santa Cruz, CA  95060

*/

#ifndef _wcpagc_h
#define _wcpagc_h

//defined elsewhere
//typedef enum _agcmode
//{ agcOFF, agcLONG, agcSLOW, agcMED, agcFAST} AGCMODE;


typedef struct _wcpagc
{
	int mode;
	int pmode;
	COMPLEX *buff;
	int io_buffsize;
	REAL sample_rate;

	double tau_attack;
	double tau_decay;
	int n_tau;
	double max_gain;
	double var_gain;
	double fixed_gain;
	double min_volts;
	double max_input;
	double out_targ;
	double out_target;
	double inv_max_input;
	double slope_constant;

	int out_index;
	int in_index;
	int attack_buffsize;

	double ring[19200][2];
	double abs_ring[19200];
	int ring_buffsize;
	double ring_max;

	double attack_mult;
	double decay_mult;
	double volts;
	double save_volts;
	double out_sample[2];
	double abs_out_sample;
	int state;

	double tau_fast_backaverage;
	double fast_backmult;
	double onemfast_backmult;
	double fast_backaverage;
	double tau_fast_decay;
	double fast_decay_mult;
	double pop_ratio;

	double hang_backaverage;
	double tau_hang_backmult;
	double hang_backmult;
	double onemhang_backmult;
	int hang_counter;
	double hangtime;
	double hang_thresh;
	double hang_level;

	double tau_hang_decay;
	double hang_decay_mult;
	int decay_type;

	char tag[4];

} wcpagc, *WCPAGC;

extern void loadWcpAGC (WCPAGC a);

extern void WcpAGC (WCPAGC a);

extern WCPAGC newWcpAGC (	AGCMODE mode,
	int pmode,
	COMPLEX *buff,
	int io_buffsize,
	REAL sample_rate,
	double tau_attack,
	double tau_decay,
	int n_tau,
	double max_gain,
	double var_gain,
	double fixed_gain,
	double max_input,
	double out_targ,
	double tau_fast_backaverage,
	double tau_fast_decay,
	double pop_ratio,
	double tau_hang_backmult,
	double hangtime,
	double hang_thresh,
	double tau_hang_decay,
	char *tag
);

extern void delWcpAGC (WCPAGC a);

extern void WcpAGC_flushbuf (WCPAGC a);

#endif

