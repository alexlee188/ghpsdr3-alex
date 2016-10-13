/* 
 * File:   FMNFilters.cpp
 * Author: John Melton, G0ORX/N6LYT
 * 
 * Created on 14 August 2010, 10:22
 */

/* Copyright (C)
* 2009 - John Melton, G0ORX/N6LYT
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
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

#include "FMNFilters.h"
#include "Filter.h"
#include "Filters.h"

FMNFilters::FMNFilters() {
    filters[0].init("200k",-100000,100000);
    filters[1].init("160k",-80000,80000);
    filters[2].init("80k",-40000,40000);
    filters[3].init("40k",-20000,20000);
    filters[4].init("20k",-10000,10000);
    filters[5].init("10k",-5000,5000);
    filters[6].init("7k",-3500,3500);
    filters[7].init("5k",-2500,2500);
    filters[8].init("3k",-1500,1500);
    filters[9].init("2.5k",-1250,1250);
    filters[10].init("Vari",-15000,15000);

    selectFilter(6);
}

FMNFilters::~FMNFilters() {
}
