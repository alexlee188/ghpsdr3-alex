/* diversity.c */

/*
This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2009 by Frank Brickle, AB2KT and Bob McGwier, N4HY

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
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
#if 0
#include <diversity.h>
#include <sdrexport.h>


static int arrival_osc=0;
static int arrival_sum=0;
static int arrival_out=0;
static int arrival_trx=0;

void barrier_osc()
{
	pthread_mutex_lock(&diversity.diversity_osc_mutex);
	arrival_osc++;
	if (arrival_osc < 2) {
		pthread_cond_wait(&diversity.dv_osc_cond,&diversity.diversity_osc_mutex);
	} else {
		arrival_osc =  0;
		pthread_cond_broadcast(&diversity.dv_osc_cond);
	}
	pthread_mutex_unlock(&diversity.diversity_osc_mutex);
}

void barrier_sum()
{
	pthread_mutex_lock(&diversity.diversity_sum_mutex);
	arrival_sum++;
	if (arrival_sum < 2) {
		pthread_cond_wait(&diversity.dv_sum_cond,&diversity.diversity_sum_mutex);
	} else {
		arrival_sum =  0;
		pthread_cond_broadcast(&diversity.dv_sum_cond);
	}
	pthread_mutex_unlock(&diversity.diversity_sum_mutex);
}


void barrier_out()
{
	pthread_mutex_lock(&diversity.diversity_out_mutex);
	arrival_out++;
	if (arrival_out < 2) {
		pthread_cond_wait(&diversity.dv_out_cond,&diversity.diversity_out_mutex);
	} else {
		arrival_out =  0;
		pthread_cond_broadcast(&diversity.dv_out_cond);
	}
	pthread_mutex_unlock(&diversity.diversity_out_mutex);
}

void barrier_TRX()
{
	pthread_mutex_lock(&diversity.diversity_trx_mutex);
	arrival_trx++;
	if (arrival_trx < 2) {
		pthread_cond_wait(&diversity.dv_trx_cond,&diversity.diversity_trx_mutex);
	} else {
		arrival_trx =  0;
		pthread_cond_broadcast(&diversity.dv_trx_cond);
	}
	pthread_mutex_unlock(&diversity.diversity_trx_mutex);
}
#endif