/**
* @file hiqsdr.h
* @brief HiqSDR server application
* @author Andrea Montefusco, IW0HDV
* @version 1.0
* @date January 2012
*/


/* Copyright (C)
* 2012 - Andrea Montefusco, IW0HDV
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/


#if !defined __HIQSDR_H__
#define      __HIQSDR_H__

typedef int (*HIQSDR_CB)(void *buf, int bsize, void *data) ;


int hiqsdr_init (const char*hiqsdr_ip = "192.168.2.196", int hiqsdr_bw = 48000, long long hiqsdr_f = 7050000LL);
int hiqsdr_connect (void);
int hiqsdr_disconnect (void);
int hiqsdr_set_frequency (long long f);
int hiqsdr_set_bandwidth (long long b);
int hiqsdr_set_attenuator (int attDb);
int hiqsdr_set_antenna_input (int n);
int hiqsdr_set_preselector (int p);
int hiqsdr_get_preselector_desc(unsigned int p, char *pd);
int hiqsdr_set_preamp (int newstatus);
int hiqsdr_get_preamp (void);
int hiqsdr_start_asynch_input (HIQSDR_CB cb, void *pud);
int hiqsdr_stop_asynch_input ();
int hiqsdr_deinit (void);
static int createConfigFile(FILE *fc, const char *fn);

char *hiqsdr_get_ip_address ();

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#endif
