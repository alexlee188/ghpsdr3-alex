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

    image = QImage(width()*2, height(), QImage::Format_RGB32);

    int x, y;
    #pragma omp parallel for schedule(static)
    for (x = 0; x < image.width(); x++) {
        for (y = 0; y < image.height(); y++) {
            image.setPixel(x, y, 0xFF000000);
        }
  }
    cy = image.height()/2 - 1;

    waterfallcl = new Waterfallcl;
    //waterfallcl_thread = new QThread;
    //waterfallcl->moveToThread(waterfallcl_thread);
    //waterfallcl_thread->start(QThread::LowestPriority);
    waterfallcl->setGeometry(QRect(QPoint(0,0),QPoint(511,255)));
    waterfallcl->initialize(512,256);
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
}

void Waterfall::setLow(int low) {
    waterfallLow=low;
}

void Waterfall::setAutomatic(bool state) {
    waterfallAutomatic=state;
}

bool Waterfall::getAutomatic() {
    return waterfallAutomatic;
}

void Waterfall::setObjectName(QString name) {
    QFrame::setObjectName(name);
}

void Waterfall::setGeometry(QRect rect) {
    QFrame::setGeometry(rect);

    qDebug() << "Waterfall::setGeometry: width=" << rect.width() << " height=" << rect.height();

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

    if (! move==0) emit frequencyMoved(move,100);
}

void Waterfall::mouseReleaseEvent(QMouseEvent* event) {
    int move=event->pos().x()-lastX;
    lastX=event->pos().x();
    //qDebug() << __FUNCTION__ << ": " << event->pos().x() << " move:" << move;

    if(moved) {
        emit frequencyMoved(move,100);
    } else {
//        float hzPixel = sampleRate/width();  // spectrum resolution: Hz/pixel
        float hzPixel = (float) sampleRate / width();  // spectrum resolution: Hz/pixel

        long freqOffsetPixel;
        long long f = frequency - (sampleRate/2) + (event->pos().x()*hzPixel) - LO_offset;
        if(subRx) {
            freqOffsetPixel = (subRxFrequency-f)/hzPixel;
            if (button == Qt::LeftButton) {
                if((mode!="USB")&&(mode!="LSB")){ // no adjustment needed if USB or LSB mode so we snap to the carrier frequency.
                    // set frequency to center of filter
                    if(filterLow<0 && filterHigh<0) {
                        freqOffsetPixel+=(((filterLow-filterHigh)/2)+filterHigh)/hzPixel;
                    } else if(filterLow>0 && filterHigh>0){
                        freqOffsetPixel-=(((filterHigh-filterLow)/2)-filterHigh)/hzPixel;
                    } else {
                        // no adjustment
                    }
                }
            }
        } else {
            freqOffsetPixel = (f-frequency)/hzPixel; // compute the offset from the central frequency, in pixel
            if (button == Qt::LeftButton) {
                if((mode!="USB")&&(mode!="LSB")){ // no adjustment needed if USB or LSB mode so we snap to the carrier frequency.
                    // set frequency to center of filter
                    if(filterLow<0 && filterHigh<0) {
                    freqOffsetPixel-=(((filterLow-filterHigh)/2)+filterHigh)/hzPixel;
                    } else if(filterLow>0 && filterHigh>0){
                        freqOffsetPixel+=(((filterHigh-filterLow)/2)-filterHigh)/hzPixel;
                    } else {
                    // no adjustment if filter extends each side of carrier frequency
                    }
                }
            }
        }

        emit frequencyMoved(-(long long)(freqOffsetPixel*hzPixel)/100,100);

    }
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
        emit frequencyMoved(event->delta()/8/15,50);
    } else {
        emit frequencyMoved(event->delta()/8/15,100);
    }

}


void Waterfall::paintEvent(QPaintEvent*) {
    QPainter painter(this);

    //painter.fillRect(0, 0, width(), height(), Qt::black);

    painter.drawImage(0,0,image,0,cy,image.width(),image.height()/2,Qt::AutoColor);

    if (cy <= 0) cy = image.height()/2 - 1;
    else cy--;          // "scroll"
}


void Waterfall::updateWaterfall(char*header,char* buffer,int length) {
    int i,j;
    int version,subversion;
    int offset;

    waterfallcl->updateWaterfall(header, buffer, length);

    version=header[1];
    subversion=header[2];
    sampleRate=((header[9]&0xFF)<<24)+((header[10]&0xFF)<<16)+((header[11]&0xFF)<<8)+(header[12]&0xFF);

    if(version==2 && subversion>0) {
        // only in version 2.1 and above
        LO_offset=((header[13]&0xFF)<<8)+(header[14]&0xFF);
    } else {
        LO_offset=0;
    }

    if(samples!=NULL) {
        free(samples);
    }
    samples = (float*) malloc(width() * sizeof (float));

    // rotate spectrum display if LO is not 0
    if(LO_offset==0) {
        #pragma omp parallel for schedule(static)
        for(i=0;i<width();i++) {
            samples[i] = -(buffer[i] & 0xFF);
        }
    } else {
        float step=(float)sampleRate/(float)width();
        offset=(int)((float)LO_offset/step);
        #pragma omp parallel for schedule(static) private(i,j)
        for(i=0;i<width();i++) {
            j=i-offset;
            if(j<0) j+=width();
            if(j>=width()) j%=width();
            samples[i] = -(buffer[j] & 0xFF);
        }
    }
    size = length;
    QTimer::singleShot(0,this,SLOT(updateWaterfall_2()));
}

void Waterfall::updateWaterfall_2(void){
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
    QTimer::singleShot(0,this,SLOT(updateWaterfall_3()));
}


void Waterfall::updateWaterfall_3(void){
    int x;
    int average=0;

    // draw the new line
    #pragma omp parallel for schedule(static)
    for(x=0;x<size;x++){
        uint pixel = calculatePixel(samples[x]);
        image.setPixel(x,cy,pixel);
        image.setPixel(x,cy+height(),pixel);
        #pragma omp critical
        average+=samples[x];
    }

    if(waterfallAutomatic) {
        waterfallLow=(average/size)-10;
        waterfallHigh=waterfallLow+60;
    }

    QTimer::singleShot(0,this,SLOT(updateWaterfall_4()));

}


void Waterfall::updateWaterfall_4(void){
    QTimer::singleShot(0,this,SLOT(repaint()));
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


