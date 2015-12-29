/* 
 * File:   LSBFilters.cpp
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


#include "LSBFilters.h"
#include "Filter.h"
#include "Filters.h"

LSBFilters::LSBFilters() {

    filters[0].init("5.0k",-5050,-50);
    filters[1].init("4.4k",-4450,-50);
    filters[2].init("3.8k",-3900,-100);
    filters[3].init("3.3k",-3450,-150);
    filters[4].init("2.9k",-3050,-150);
    filters[5].init("2.7k",-2900,-200);
    filters[6].init("2.4k",-2600,-200);
    filters[7].init("2.1k",-2400,-300);
    filters[8].init("1.8k",-2150,-350);
    filters[9].init("1.0k",-1400,-400);
    filters[10].init("Vari",-5050,-50);

    selectFilter(3);
}

LSBFilters::~LSBFilters() {
}

