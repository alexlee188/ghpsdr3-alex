
/* Copyright (C) - modifications of the original program by John Melton
* 2012 - Alex Lee, 9V1Al
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
* Foundation, Inc., 59 Temple Pl
*/

#include "Waterfallcl.h"


Waterfallcl::Waterfallcl(){
    glContext = 0;
}

Waterfallcl::~Waterfallcl(){

}

void Waterfallcl::setGeometry(QRect rect){
    data_width = rect.width();
    data_height = rect.height();
}

void Waterfallcl::initialize(int wid, int ht){
    if (glContext) {
        waterfall.setGlobalWorkSize(wid, ht);
        return;
    }

    glContext = new QCLContextGL();
    if (!glContext->create()) return;
}

void Waterfallcl::updateWaterfall(char *header, char *buffer, int width){

}


