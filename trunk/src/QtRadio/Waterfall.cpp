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


/* Copyright (C) 2012 - Alex Lee, 9V1Al
* modifications of the original program by John Melton
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

#ifdef _OPENMP
#include <omp.h>
#endif
#include "Waterfall.h"

Waterfall::Waterfall() {
}

Waterfall::Waterfall(QWidget*& widget) {
    QFrame::setParent(widget);
    
    sampleRate=96000;

    subRxFrequency=0LL;
    subRx=false;

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
    zoom = 0;

#ifdef WATERFALL_2D
    image = QImage(width()*2, height(), QImage::Format_RGB32);

    int x, y;
    #pragma omp parallel for schedule(static)
    for (x = 0; x < image.width(); x++) {
        for (y = 0; y < image.height(); y++) {
            image.setPixel(x, y, 0xFF000000);
        }
    }
    cy = image.height()/2 - 1;
#endif

    waterfallcl = new Waterfallcl;

    waterfallcl->initialize(width()*2,256);
    waterfallcl->resize(width()*2,height()*2);
    waterfallcl->setParent(this);
    waterfallcl->show();
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

void Waterfall::setZoom(int value){
    zoom = value;
}

void Waterfall::setGeometry(QRect rect) {
    QFrame::setGeometry(rect);
    //qDebug() << "Waterfall::setGeometry: width=" << rect.width() << " height=" << rect.height();
#ifdef WATERFALL_2D
    samples = (float*) malloc(rect.width() * sizeof (float));

    // no scroll algorithm needs 2 copies of waterfaull
    image = QImage(rect.width(), rect.height()*2, QImage::Format_RGB32);
    cy = rect.height()-1;
    qDebug() << "Waterfall::Waterfall " << rect.width() << ":" << rect.height();

    int x, y;
    #pragma omp parallel for schedule(static)
    for (x = 0; x < rect.width(); x++) {
        for (y = 0; y < rect.height()*2; y++) {
            image.setPixel(x, y, 0xFF000000);
        }
    }
#endif

}


void Waterfall::mousePressEvent(QMouseEvent* event) {

    //qDebug() << __FUNCTION__ << ": " << event->pos().x();

    //qDebug() << "mousePressEvent: event->button(): " << event->button();

    button=event->button();
    startX=lastX=event->pos().x();
    moved=0;
}

void Waterfall::mouseMoveEvent(QMouseEvent* event){
    int move=event->pos().x()-lastX;
    lastX=event->pos().x();
//    qDebug() << __FUNCTION__ << ": " << event->pos().x() << " move:" << move;

    moved=1;
    float zoom_factor = 1.0f + zoom/25.0f;
    float move_ratio = (float)sampleRate/48000.0f/zoom_factor;
    int move_step;
    if (move_ratio > 10.0f) move_step = 500;
    else if (move_ratio > 5.0f) move_step = 200;
    else if (move_ratio > 2.5f) move_step = 100;
    else if (move_ratio > 1.0f) move_step = 50;
    else if (move_ratio > 0.5f) move_step = 10;
    else if (move_ratio > 0.25f) move_step = 5;
    else move_step = 1;
    if (! move==0) {
        if (subRx) emit frequencyMoved(-move,move_step);
        else emit frequencyMoved(move,move_step);
    }
}

void Waterfall::mouseReleaseEvent(QMouseEvent* event) {
    int move=event->pos().x()-lastX;
    lastX=event->pos().x();
    //qDebug() << __FUNCTION__ << ": " << event->pos().x() << " move:" << move;
    float zoom_factor = 1.0f + zoom/25.0f;
    float hzPixel = (float) sampleRate / width() / zoom_factor;  // spectrum resolution: Hz/pixel
    if(moved) {
        float move_ratio = (float)sampleRate/48000.0f/zoom_factor;
        int move_step;
        if (move_ratio > 10.0f) move_step = 500;
        else if (move_ratio > 5.0f) move_step = 200;
        else if (move_ratio > 2.5f) move_step = 100;
        else if (move_ratio > 1.0f) move_step = 50;
        else if (move_ratio > 0.5f) move_step = 10;
        else if (move_ratio > 0.25f) move_step = 5;
        else move_step = 1;
        if (subRx) emit frequencyMoved(-move,move_step);
        else emit frequencyMoved(move,move_step);
    } else {
        long freqOffsetPixel;
        long long f = frequency - (sampleRate/2/zoom_factor) + (event->pos().x()*hzPixel)
                -LO_offset;
        if(subRx) {
            freqOffsetPixel = (subRxFrequency-f)/hzPixel;
            if (button == Qt::LeftButton) {
                if((mode!="USB")&&(mode!="LSB")){
                    // set frequency to center of filter
                    if(filterLow<0 && filterHigh<0) {
                        freqOffsetPixel+=(((filterLow-filterHigh)/2)+filterHigh)/hzPixel;
                    } else if(filterLow>0 && filterHigh>0){
                        freqOffsetPixel-=(((filterHigh-filterLow)/2)-filterHigh)/hzPixel;
                    } else {
                    // no adjustment
                    }
                } // no adjustment needed if USB or LSB mode so we snap to the carrier frequency.
            }
            emit frequencyMoved((long long)(freqOffsetPixel*hzPixel)/100,100);
        } else {
            freqOffsetPixel = (f-frequency)/hzPixel; // compute the offset from the central frequency, in pixel
            if (button == Qt::LeftButton) {
                if((mode!="USB")&&(mode!="LSB")){
                    // set frequency to center of filter
                    if(filterLow<0 && filterHigh<0) {
                    freqOffsetPixel-=(((filterLow-filterHigh)/2)+filterHigh)/hzPixel;
                    } else if(filterLow>0 && filterHigh>0){
                        freqOffsetPixel+=(((filterHigh-filterLow)/2)-filterHigh)/hzPixel;
                    } else {
                    // no adjustment if filter extends each side of carrier frequency
                    }
                } // no adjustment needed if USB or LSB mode so we snap to the carrier frequency.
            }
            emit frequencyMoved(-(long long)(freqOffsetPixel*hzPixel)/100,100);
        }
    }
    button = -1;
}

void Waterfall::wheelEvent(QWheelEvent *event) {
    //qDebug() << __FUNCTION__ << "Delta: " << event->delta() << "y: " << event->pos().y() << " heigth:" << height();

    // change frequency
    float vOfs = (float)event->pos().y() / (float)height();
    //qDebug() << "wheelEvent vOfs: " << vOfs;

    if (vOfs > 0.75) {
        emit frequencyMoved(event->delta()/8/15,10);
    } else if (vOfs > 0.50) {
        emit frequencyMoved(event->delta()/8/15,25);
    } else if (vOfs > 0.25) {
        emit (event->delta()/8/15,50);
    } else {
        emit frequencyMoved(event->delta()/8/15,100);
    }

}


void Waterfall::paintEvent(QPaintEvent* event) {
#ifdef WATERFALL_2D
    QPainter painter(this);

    painter.drawImage(0,0,image,0,cy,image.width(),image.height()/2,Qt::AutoColor);
    if (cy <= 0) cy = image.height()/2 - 1;
    else cy--;          // "scroll"
#endif
}


void Waterfall::updateWaterfall(char*header,char* buffer,int length) {
    int i;
    int version,subversion;
#ifdef WATERFALL_2D
    int offset;
#endif
    //qDebug() << "updateWaterfall: " << width() << ":" << height();

    version=header[1];
    subversion=header[2];
    sampleRate=((header[9]&0xFF)<<24)+((header[10]&0xFF)<<16)+((header[11]&0xFF)<<8)+(header[12]&0xFF);

    if(version==2 && subversion>0) {
        // only in version 2.1 and above
        LO_offset=((header[13]&0xFF)<<8)+(header[14]&0xFF);
    } else {
        LO_offset=0;
    }
#ifdef WATERFALL_2D
    if(samples!=NULL) {
        free(samples);
    }
    samples = (float*) malloc(width() * sizeof (float));
#endif

#ifdef WATERFALL_2D
    // do not rotate spectrum display.  It is done by dspserver now
        #pragma omp parallel for schedule(static)
        for(i=0;i<width();i++) {
            samples[i] = -(buffer[i] & 0xFF);
        }
#endif

    waterfallcl->setLO_offset(0.0);

    size = length;

    int sum = 0;
    for(i=0;i<length;i++) sum += -(buffer[i] & 0xFF);
    average = average * 0.99f + (float)(sum/length) * 0.01f; // running average

    waterfallcl->updateWaterfall(header, buffer, length);

    QTimer::singleShot(0,this,SLOT(updateWaterfall_2()));
}

void Waterfall::updateWaterfall_2(void){


#ifdef WATERFALL_2D
    int x,y;
    if(image.width()!=width() ||
       (image.height()/2) != (height())) { 
        qDebug() << "Waterfall::updateWaterfall " << size << "(" << width() << ")," << height();
        image = QImage(width(), height()*2, QImage::Format_RGB32);
        cy = image.height()/2 - 1;
        #pragma omp parallel for schedule(static)
        for (x = 0; x < width(); x++) {
            for (y = 0; y < image.height(); y++) {
                image.setPixel(x, y, 0xFF000000);
            }
        }
    }
#endif
    QTimer::singleShot(0,this,SLOT(updateWaterfall_4()));
}

void Waterfall::updateWaterfall_3(void){

}



void Waterfall::updateWaterfall_4(void){
#ifdef WATERFALL_2D
    int x;
    int local_average=0;

    // draw the new line
    #pragma omp parallel for schedule(static)
    for(x=0;x<size;x++){
        uint pixel = calculatePixel(samples[x]);
        image.setPixel(x,cy,pixel);
        image.setPixel(x,cy+height(),pixel);
        #pragma omp critical
        local_average+=samples[x];
    }
#endif
    if(waterfallAutomatic) {
#ifdef WATERFALL_2D
        waterfallLow=(local_average/size)-10;
        waterfallHigh=waterfallLow+60;
#endif
        waterfallcl->setLow(average - 10);
        waterfallcl->setHigh(average + 50);
    }

    waterfallcl->updateWaterfallgl();

#ifdef WATERFALL_2D
    QTimer::singleShot(0,this,SLOT(repaint()));
#endif

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


