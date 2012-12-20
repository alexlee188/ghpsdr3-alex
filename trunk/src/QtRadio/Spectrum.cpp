/*
 * File:   Spectrum.cpp
 * Author: John Melton, G0ORX/N6LYT
 *
 * Created on 16 August 2010, 10:03
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

#include "Spectrum.h"

#define MAX_WIDTH 4096

Spectrum::Spectrum() {
}

Spectrum::Spectrum(QWidget*& widget) {
    QFrame::setParent(widget);

    //qDebug() << "Spectrum::Spectrum " << width() << ":" << height();

    sMeterMain=new Meter("Main Rx");
    sMeterSub=new Meter("Sub Rx");
    sampleRate=96000;
    spectrumHigh=-40;
    spectrumLow=-160;
    filterLow=-3450;
    filterHigh=-150;
    avg = 0;
    mode="LSB";

    zoom = 0;
    subRxFrequency=0LL;
    subRx=false;

    band_min=0LL;
    band_max=0LL;

    samples=NULL;
    
    receiver=0;

    meter=-121;
    maxMeter=-121;
    meterCount=0;
    
    subrx_meter=-121;
    subrx_maxMeter=-121;
    subrx_meterCount=0;

    button=-1;
    showSquelchControl=false;
    settingSquelch=false;

    plot.clear();

    samples = (float*) malloc(MAX_WIDTH * sizeof (float));
    for (int i=0; i < MAX_WIDTH; i++) samples[i] = -120;
}

Spectrum::~Spectrum() {
    if (samples != NULL) free(samples);
}

void Spectrum::setHigh(int high) {
    spectrumHigh=high;
    //repaint();
    update();
}

void Spectrum::setLow(int low) {
    spectrumLow=low;
//    repaint();
    update();
}

int Spectrum::getHigh() {
    return spectrumHigh;
}

int Spectrum::getLow() {
    return spectrumLow;
}

void Spectrum::setAvg(int value){
    avg = value;
}

void Spectrum::initialize() {
    QFrame::setVisible(true);
}

void Spectrum::setSampleRate(int r) {
    sampleRate=r;
}

int Spectrum::samplerate() {
    return sampleRate;
}

void Spectrum::setBandLimits(long long min,long long max) {

    qDebug() << "Spectrum::setBandLimits: " << min << "," << max;
    band_min=min;
    band_max=max;
}

void Spectrum::setObjectName(QString name) {
    QFrame::setObjectName(name);
}

void Spectrum::setGeometry(QRect rect) {
    QFrame::setGeometry(rect);

    //qDebug() << "Spectrum:setGeometry: width=" << rect.width() << " height=" << rect.height();

    samples=(float*)malloc(rect.width()*sizeof(float));
}

void Spectrum::mousePressEvent(QMouseEvent* event) {

    //qDebug() << __FUNCTION__ << ": " << event->pos().x();

    //qDebug() << "mousePressEvent: event->button(): " << event->button();

    button=event->button();
    startX=lastX=event->pos().x();
    moved=0;

    if(squelch) {
        if(event->pos().y()>=(squelchY-1) &&
           event->pos().y()<=(squelchY+1)) {
            settingSquelch=true;
        } else {
            settingSquelch=false;
        }
    }

}

void Spectrum::mouseMoveEvent(QMouseEvent* event){
    int move=event->pos().x()-lastX;
    lastX=event->pos().x();
//    qDebug() << __FUNCTION__ << ": " << event->pos().x() << " move:" << move;

    moved=1;

    if(button==-1) {
        if(squelch &&
           event->pos().y()>=(squelchY-1) &&
           event->pos().y()<=(squelchY+1)) {
            showSquelchControl=true;
            this->setCursor(Qt::SizeVerCursor);
        } else {
            showSquelchControl=false;
            this->setCursor(Qt::ArrowCursor);
        }
    } else {
        if(settingSquelch) {
            int delta=squelchY-event->pos().y();
            delta=int((float)delta*((float)(spectrumHigh-spectrumLow)/(float)height()));
            //qDebug()<<"squelchValueChanged"<<delta<<"squelchY="<<squelchY<<" y="<<event->pos().y();
            emit squelchValueChanged(delta);
            //squelchY=event->pos().y();
        } else {
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

            if (!move==0) {
                if (subRx) emit frequencyMoved(-move,move_step);
                else emit frequencyMoved(move,move_step);
            }
        }
    }

}

void Spectrum::mouseReleaseEvent(QMouseEvent* event) {
    int move=event->pos().x()-lastX;
    lastX=event->pos().x();
    //qDebug() << __FUNCTION__ << ": " << event->pos().x() << " move:" << move;
    float zoom_factor = 1.0f + zoom/25.0f;
    float hzPixel = (float) sampleRate / width() / zoom_factor;  // spectrum resolution: Hz/pixel
    if(squelch && settingSquelch) {
        button=-1;
        settingSquelch=false;
    } else {
        float zoom_factor = 1.0f + zoom/25.0f;
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
            if (subRx) emit frequencyMoved(-move,move_ratio);
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
}

void Spectrum::wheelEvent(QWheelEvent *event) {
    //qDebug() << __FUNCTION__ << "Delta: " << event->delta() << "y: " << event->pos().y() << " heigth:" << height();

    if(event->pos().x() > 50) {
        // wheel event on the right side
        // change frequency
        float vOfs = (float)event->pos().y() / (float)height();
        //qDebug() << "wheelEvent vOfs: " << vOfs;

        if (vOfs > 0.75) {
            emit frequencyMoved(event->delta()/8/15,100);
        } else if (vOfs > 0.50) {
            emit frequencyMoved(event->delta()/8/15,50);
        } else if (vOfs > 0.25) {
            emit frequencyMoved(event->delta()/8/15,25);
        } else {
            emit frequencyMoved(event->delta()/8/15,10);
        }
    } else {
        // wheel event on the left side, change the vertical axis values
        float shift =  (float)(event->delta()/8/15 * 5)                    // phy steps of wheel * 5
                     * ((float)(spectrumHigh - spectrumLow) / height());   // dBm / pixel on vertical axis
       
        if (event->buttons() == Qt::MidButton) {
           // change the vertical axis range
           //qDebug() << __FUNCTION__ << " change vertical axis scale: " << shift;
           emit spectrumHighChanged (spectrumHigh+(int)shift);
           emit spectrumLowChanged  (spectrumLow-(int)shift);
           emit waterfallHighChanged (spectrumHigh+(int)shift);
           emit waterfallLowChanged  (spectrumLow-(int)shift);

        } else {
          // if middle mouse button pressed shift the spectrum scale
          //qDebug() << __FUNCTION__ << " shift on vertical axis scale: " << shift;
          emit spectrumHighChanged (spectrumHigh+(int)shift);
          emit spectrumLowChanged  (spectrumLow+(int)shift);
          emit waterfallHighChanged (spectrumHigh+(int)shift);
          emit waterfallLowChanged  (spectrumLow+(int)shift);

       }
    }
}

void Spectrum::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    int filterLeft;
    int filterRight;
    QString text;
    float step=(float)sampleRate/(float)width();
    int offset=(int)((float)LO_offset/step);

    QLinearGradient gradient(0, 0, 0,height());
    gradient.setColorAt(0, Qt::black);
    gradient.setColorAt(1, Qt::gray);
    painter.setBrush(gradient);
    painter.drawRect(0, 0, width(), height());

    if(sampleRate==0) {
        qDebug() << "sampleRate is 0";
        return;
    }

    float zoom_factor = 1.0f + zoom/25.0f;

    // draw cursor
    painter.setPen(QPen(Qt::red, 1));
    painter.drawLine((width()/2)+offset*zoom_factor,0,(width()/2)+offset*zoom_factor,height());

    // draw filter
    filterLeft = width()/2 + (float)(filterLow+LO_offset)* (float)width()*zoom_factor/(float)sampleRate;
    filterRight = width()/2 + (float)(filterHigh+LO_offset)*(float)width()*zoom_factor/(float)sampleRate;
    painter.setBrush(Qt::SolidPattern);
    painter.setOpacity(0.5);
    painter.fillRect(filterLeft,0,filterRight-filterLeft,height(),Qt::gray);

    // draw sub rx filter and cursor
    if(subRx) {
        //int cursor=((subRxFrequency-(frequency-(sampleRate/2))) * width() / sampleRate)+offset;
        int cursor = width()/2 + (float)(subRxFrequency-frequency+LO_offset)
                * (float)width()*zoom_factor/(float)sampleRate;
        filterLeft = width()/2 + (float)(filterLow+LO_offset+subRxFrequency-frequency)
                * (float)width()*zoom_factor/(float)sampleRate;
        filterRight = width()/2 + (float)(filterHigh+LO_offset+subRxFrequency-frequency)
                * (float)width()*zoom_factor/(float)sampleRate;
        painter.setBrush(Qt::SolidPattern);
        painter.setOpacity(0.5);
        painter.fillRect(filterLeft, 0, filterRight - filterLeft, height(), Qt::lightGray);

        painter.setPen(QPen(Qt::red, 1));
        painter.drawLine(cursor,0,cursor,height());
    }

    // plot horizontal dBm lines
    int V = spectrumHigh - spectrumLow;
    int numSteps = V / 20;
    for (int i = 1; i < numSteps; i++) {
        int num = spectrumHigh - i * 20;
        int y = (int) floor((spectrumHigh - num) * height() / V);

        painter.setOpacity(0.5);
        painter.setPen(QPen(Qt::white, 1,Qt::DotLine));
        painter.drawLine(0, y, width(), y);

        painter.setOpacity(1.0);
        painter.setPen(QPen(Qt::green, 1));
        painter.setFont(QFont("Arial", 10));
        painter.drawText(3,y,QString::number(num)+" dBm");
    }
    
    // plot the vertical frequency lines
    float hzPerPixel=(float)sampleRate/(float)width()/zoom_factor;
    long long lineStep = 10000;
    if (sampleRate > 1000000) lineStep = 100000;
    else if (sampleRate > 500000) lineStep = 50000;
    else if (sampleRate > 200000) lineStep = 20000;

    for(int i=0;i<width();i++) {
        long long f=frequency-((float)sampleRate/zoom_factor/2.0)-(float)LO_offset+(long long)(hzPerPixel*(float)i);
        if(f>0) {
            if((f % lineStep)<(long long)hzPerPixel) {
                painter.setOpacity(0.5);
                painter.setPen(QPen(Qt::white, 1,Qt::DotLine));
                painter.drawLine(i, 0, i, height());

                painter.setOpacity(1.0);
                painter.setPen(QPen(Qt::black, 1));
                painter.setFont(QFont("Arial", 10));
                text.sprintf("%lld.%02lld",f/1000000,f%1000000/10000);
                painter.drawText(i,height(),text);
            }
        }
    }

    // draw the band limits
    long long min_display=frequency-((float)sampleRate/zoom_factor/2.0);
    long long max_display=frequency+((float)sampleRate/zoom_factor/2.0);
    if(band_min!=0LL && band_max!=0LL) {
        int i;
        painter.setPen(QPen(Qt::red, 2));
        if((min_display<band_min)&&(max_display>band_min)) {
            i=(band_min-min_display)/(long long)hzPerPixel;
            painter.drawLine(i,0,i,height());
        }
        if((min_display<band_max)&&(max_display>band_max)) {
            i=(band_max-min_display)/(long long)hzPerPixel;
            painter.drawLine(i+1,0,i+1,height());
        }
    }

    // draw the squelch
    if(settingSquelch || showSquelchControl) {
        squelchY=(int) floor(((float) spectrumHigh - squelchVal)*(float) height() / (float) (spectrumHigh - spectrumLow));
        painter.setPen(QPen(Qt::red, 1,Qt::DashLine));
        painter.drawLine(0,squelchY,width(),squelchY);
        painter.setFont(QFont("Arial", 10));
        text.sprintf("%s","Squelch");
        painter.drawText(width()-48,squelchY,text);
    } else if(squelch) {
        squelchY=(int) floor(((float) spectrumHigh - squelchVal)*(float) height() / (float) (spectrumHigh - spectrumLow));
        painter.setPen(QPen(Qt::red, 1));
        painter.drawLine(0,squelchY,width(),squelchY);
        painter.setFont(QFont("Arial", 10));
        text.sprintf("%s","Squelch");
        painter.drawText(width()-48,squelchY,text);
    }

    emit meterValue(meter, subrx_meter);

    // plot Spectrum
    painter.setOpacity(0.9);
    painter.setPen(QPen(Qt::yellow, 1));
    if(plot.count()==width()) {
        painter.drawPolyline(plot.constData(),plot.count());
    }


    // show the subrx frequency
    if(subRx) {
        // show the frequency
        painter.setPen(QPen(Qt::green,1));
        painter.setFont(QFont("Verdana", 30));
    }
}

void Spectrum::setZoom(int value){
    zoom = value;
}

void Spectrum::setFrequency(long long f) {
    frequency=f;
    subRxFrequency=f;

//    gvj code

//    strFrequency.sprintf("%lld.%03lld.%03lld",f/1000000,f%1000000/1000,f%1000);
//    strSubRxFrequency.sprintf("%lld.%03lld.%03lld",f/1000000,f%1000000/1000,f%1000);
    //qDebug() << "Spectrum:setFrequency: " << f;
}

void Spectrum::setSubRxFrequency(long long f) {
    subRxFrequency=f;
    strSubRxFrequency.sprintf("%lld.%03lld.%03lld",f/1000000,f%1000000/1000,f%1000);

    //qDebug() << "Spectrum:setSubRxFrequency: " << f;
}

void Spectrum::setSubRxState(bool state) {
    subRx=state;
}

void Spectrum::setFilter(int low, int high) {
    qDebug() << "Spectrum::setFilter " << low << "," << high;
    filterLow=low;
    filterHigh=high;
}

void Spectrum::setHost(QString h) {
    host=h;
//    repaint();
//    update();
}

void Spectrum::setReceiver(int r) {
    receiver=r;
//    repaint();
    update();
}

void Spectrum::setMode(QString m) {
    mode=m;
    qDebug()<<Q_FUNC_INFO<<": Mode changed to "<<m;
    update();
}

void Spectrum::setBand(QString b) {
    band=b;
//    repaint();
    update();
}

void Spectrum::setFilter(QString f) {
    filter=f;
//    repaint();
    update();
}

void Spectrum::updateSpectrumFrame(char* header,char* buffer,int width) {
    int i;
    int version,subversion;
    int header_sampleRate;

    version=header[1];
    subversion=header[2];
    meter=((header[5]&0xFF)<<8)+(header[6]&0xFF);
    subrx_meter=((header[7]&0xFF)<<8)+(header[8]&0xFF);
    header_sampleRate=((header[9]&0xFF)<<24)+((header[10]&0xFF)<<16)+((header[11]&0xFF)<<8)+(header[12]&0xFF);

    if(version==2 && subversion>0) {
        // only in version 2.1 and above
        LO_offset=((header[13]&0xFF)<<8)+(header[14]&0xFF);
    } else {
        LO_offset=0;
    }

    sampleRate = header_sampleRate;

    // do not rotate spectrum display.  LO_offset rotation done in dspserver
    for(i=0;i<width;i++) {
        samples[i] = (float)(samples[i] * avg - (buffer[i] & 0xFF))/(float)(avg+1);
    }

    //qDebug() << "updateSpectrum: create plot points";
    plot.clear();
    for (i = 0; i < width; i++) {
        plot << QPoint(i, (int) floor(((float) spectrumHigh - samples[i])*(float) height() / (float) (spectrumHigh - spectrumLow)));
    }
    QTimer::singleShot(0,this,SLOT(update()));
}

void Spectrum::setSquelch(bool state) {
    squelch=state;
    QFrame::setMouseTracking(state);
}

void Spectrum::setSquelchVal(float val) {
    squelchVal=val;
    squelchY=(int) floor(((float) spectrumHigh - squelchVal)*(float) height() / (float) (spectrumHigh - spectrumLow));
    //qDebug()<<"Spectrum::setSquelchVal"<<val<<"squelchY="<<squelchY;
}

