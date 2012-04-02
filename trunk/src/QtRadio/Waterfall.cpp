/*
 * File:   Waterfall.cpp
 * Author: John Melton, G0ORX/N6LYT
 * 
 * Created on 16 August 2010, 10:35
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

#ifdef _OPENMP
#include <omp.h>
#endif
#include "Waterfall.h"
#include "Waterfallcl.h"

Waterfall::Waterfall() {
}

Waterfall::Waterfall(QWidget*& widget) {
    QFrame::setParent(widget);
    
    sampleRate=96000;

    subRxFrequency=0LL;
    subRx=FALSE;

    waterfallHigh=-60;
    waterfallLow=-125;

    colorLowR=0;
    colorLowG=0;
    colorLowB=0;
    colorMidR=255;
    colorMidG=0;
    colorMidB=0;
    colorHighR=255;
    colorHighG=255;
    colorHighB=0;

    samples=NULL;

    waterfallcl = new Waterfallcl;
    waterfallcl->setParent(this);
    waterfallcl->setGeometry(QRect(QPoint(0,0),QPoint(width()*2-1,255)));
    waterfallcl->initialize(width()*2,256);
    waterfallcl->resize(width()*2,height());
}

Waterfall::~Waterfall() {
    delete waterfallcl;
}

void Waterfall::initialize() {
    QFrame::setVisible(true);
}

int Waterfall::getHigh() {
    return waterfallHigh;
}

int Waterfall::getLow() {
    return waterfallLow;
}

void Waterfall::setHigh(int high) {
    waterfallHigh=high;
    waterfallcl->setHigh(high);
}

void Waterfall::setLow(int low) {
    waterfallLow=low;
    waterfallcl->setLow(low);
}

void Waterfall::setAutomatic(bool state) {
    waterfallAutomatic=state;
    waterfallcl->setAutomatic(state);
}

bool Waterfall::getAutomatic() {
    return waterfallAutomatic;
}

void Waterfall::setObjectName(QString name) {
    QFrame::setObjectName(name);
}

void Waterfall::setGeometry(QRect rect) {
    QFrame::setGeometry(rect);
}


void Waterfall::mousePressEvent(QMouseEvent* event) {
}

void Waterfall::mouseMoveEvent(QMouseEvent* event){
}

void Waterfall::mouseReleaseEvent(QMouseEvent* event) {
}

void Waterfall::wheelEvent(QWheelEvent *event) {
}


void Waterfall::paintEvent(QPaintEvent*) {
}


void Waterfall::updateWaterfall(char*header,char* buffer,int length) {
    int i,j;
    int version,subversion;
    int offset;

    version=header[1];
    subversion=header[2];
    sampleRate=((header[9]&0xFF)<<24)+((header[10]&0xFF)<<16)+((header[11]&0xFF)<<8)+(header[12]&0xFF);

    if(version==2 && subversion>0) {
        // only in version 2.1 and above
        LO_offset=((header[13]&0xFF)<<8)+(header[14]&0xFF);
    } else {
        LO_offset=0;
    }
    samples = (float*) malloc(width() * sizeof (float));

    // rotate spectrum display if LO is not 0
    if(LO_offset==0) {
        waterfallcl->setLO_offset(0);
    } else {
        float step=(float)sampleRate/(float)width();
        offset=(int)((float)LO_offset/step);
        waterfallcl->setLO_offset(offset);
    }
    size = length;
    waterfallcl->updateWaterfall(header, buffer, length);

    for(i=0;i<width();i++) average += -(buffer[j] & 0xFF);
    average = average/width();

    QTimer::singleShot(0,this,SLOT(updateWaterfall_4()));
}

void Waterfall::updateWaterfall_2(void){
}


void Waterfall::updateWaterfall_3(void){
}


void Waterfall::updateWaterfall_4(void){

    if(waterfallAutomatic) {
        waterfallLow=average-10;
        waterfallHigh=waterfallLow+60;
        waterfallcl->setLow(waterfallLow);
        waterfallcl->setHigh(waterfallHigh);
    }

    QTimer::singleShot(0, waterfallcl, SLOT(updateWaterfallgl()));
}




uint Waterfall::calculatePixel(int sample) {
        // simple gray scale
//        int v=((int)sample-waterfallLow)*255/(waterfallHigh-waterfallLow);
//
//        if(v<0) v=0;
//        if(v>255) v=255;
//
//        int pixel=(255<<24)+(v<<16)+(v<<8)+v;
//        return pixel;

    int R,G,B;
    if(sample<waterfallLow) {
        R=colorLowR;
        G=colorLowG;
        B=colorLowB;
    } else if(sample>waterfallHigh) {
        R=colorHighR;
        G=colorHighG;
        B=colorHighB;
    } else {
        float range=waterfallHigh-waterfallLow;
        float offset=sample-waterfallLow;
        float percent=offset/range;
        if(percent<(2.0f/9.0f)) {
            float local_percent = percent / (2.0f/9.0f);
            R = (int)((1.0f-local_percent)*colorLowR);
            G = (int)((1.0f-local_percent)*colorLowG);
            B = (int)(colorLowB + local_percent*(255-colorLowB));
        } else if(percent<(3.0f/9.0f)) {
            float local_percent = (percent - 2.0f/9.0f) / (1.0f/9.0f);
            R = 0;
            G = (int)(local_percent*255);
            B = 255;
        } else if(percent<(4.0f/9.0f)) {
             float local_percent = (percent - 3.0f/9.0f) / (1.0f/9.0f);
             R = 0;
             G = 255;
             B = (int)((1.0f-local_percent)*255);
        } else if(percent<(5.0f/9.0f)) {
             float local_percent = (percent - 4.0f/9.0f) / (1.0f/9.0f);
             R = (int)(local_percent*255);
             G = 255;
             B = 0;
        } else if(percent<(7.0f/9.0f)) {
             float local_percent = (percent - 5.0f/9.0f) / (2.0f/9.0f);
             R = 255;
             G = (int)((1.0f-local_percent)*255);
             B = 0;
        } else if(percent<(8.0f/9.0f)) {
             float local_percent = (percent - 7.0f/9.0f) / (1.0f/9.0f);
             R = 255;
             G = 0;
             B = (int)(local_percent*255);
        } else {
             float local_percent = (percent - 8.0f/9.0f) / (1.0f/9.0f);
             R = (int)((0.75f + 0.25f*(1.0f-local_percent))*255.0f);
             G = (int)(local_percent*255.0f*0.5f);
             B = 255;
        }
    }

    int pixel = (255 << 24)+(R << 16)+(G << 8) + B;
    return pixel;

}

void Waterfall::setSampleRate(int r) {
    sampleRate=r;
}

void Waterfall::setFrequency(long long f) {
    frequency=f;
    subRxFrequency=f;

    //qDebug() << "Spectrum:setFrequency: " << f;
}

void Waterfall::setSubRxFrequency(long long f) {
    subRxFrequency=f;
    //qDebug() << "Spectrum:setSubRxFrequency: " << f;
}

void Waterfall::setSubRxState(bool state) {
    subRx=state;
}

void Waterfall::setMode(QString m)
{
    mode=m;
}

void Waterfall::setFilter(int low, int high) {
    filterLow=low;
    filterHigh=high;
}


