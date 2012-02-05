/*  wcpAGC.c
 * 
 This file is part of a program that implements a Software-Defined Radio.
 * 
 Copyright (C) 2011 Warren Pratt, NR0V

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

		 This software is based upon the algorithm described by Peter Martinez, G3PLX,
	 in the January 2010 issue of RadCom magazine.

		 */

#include <common.h>
#include <compress.h>
#include <math.h>

COMPRESSOR newCompressor (
                          COMPLEX *inbuff,
                          COMPLEX *outbuff,
                          int buffsize,
                          float gain,
                          char *tag
                          )
{
	COMPRESSOR a;
	a = (COMPRESSOR) safealloc (1,sizeof(compressor), tag);
	a->inbuff = inbuff;
	a->outbuff = outbuff;
	a->buffsize = buffsize;
	a->gain = gain;

	return a;
}

void delCompressor (COMPRESSOR a)
{
	if (a)
		safefree ((char *) a);
}

void Compressor (COMPRESSOR a)
{
	int i;
	float mag;

	for (i = 0; i < a->buffsize; i++)
	{
		mag = (float)sqrt(a->inbuff[i].re * a->inbuff[i].re + a->inbuff[i].im * a->inbuff[i].im);
		if (a->gain * mag > 1.0f)
		{
			a->outbuff[i].re = a->inbuff[i].re / mag;
			a->outbuff[i].im = 0.0;
		}
		else
		{
			a->outbuff[i].re = a->gain * a->inbuff[i].re;
			a->outbuff[i].im = 0.0;
		}
	}

}
