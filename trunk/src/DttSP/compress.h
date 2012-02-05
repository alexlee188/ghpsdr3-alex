/*  wcpAGC.c

This file is part of a program that implements a Software-Defined Radio.

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

*/

#ifndef _compressor_h
#define _compressor_h

typedef struct _compressor
{
        COMPLEX *inbuff;
        COMPLEX *outbuff;
        int buffsize;
        float gain;
        char tag[4];
}compressor, *COMPRESSOR;

extern void Compressor (COMPRESSOR a);

extern COMPRESSOR newCompressor (       
                        COMPLEX *inbuff,
                        COMPLEX *outbuff,
                        int buffsize,
                        float gain,
                        char *tag
                        );

extern void delCompressor (COMPRESSOR a);


#endif