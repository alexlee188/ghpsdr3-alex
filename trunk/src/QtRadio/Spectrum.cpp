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


/****************** Added by KD0OSS **********************************************/
lineObject::lineObject(SpectrumScene *scene, QPoint start, QPoint stop, QPen pen)
{
    itemStart = start;
    itemStop = stop;
    itemPen = pen;
    width = scene->width();
    height = scene->height();
    itemType = 0;
}

void lineObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(itemPen);
    painter->drawLine(itemStart, itemStop);
}

QRectF lineObject::boundingRect() const
{
    return QRectF(QPointF(0.0, 0.0), QPointF(width, height));
}

notchFilterObject::notchFilterObject(SpectrumScene *scene, int index, QPoint location, float fwidth, QColor color)
{
    itemLocation = location;
    itemWidth = fwidth;
    itemColor = color;
    itemIndex = index;
    width = scene->width();
    height = scene->height();
    itemType = 1;

    setZValue(10.0);
}

void notchFilterObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setBrush(Qt::SolidPattern);
    painter->setOpacity(0.6);
    painter->fillRect(itemLocation.x(),itemLocation.y(),itemWidth,height,itemColor);
}

QRectF notchFilterObject::boundingRect() const
{
    return QRectF(QPointF(0.0, 0.0), QPointF(width, height));
}

filterObject::filterObject(SpectrumScene *scene, QPoint location, float fwidth, QColor color)
{
    itemLocation = location;
    itemWidth = fwidth;
    itemColor = color;
    width = scene->width();
    height = scene->height();
    itemType = 2;
}

void filterObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setBrush(Qt::SolidPattern);
    painter->setOpacity(0.5);
    painter->fillRect(itemLocation.x(),itemLocation.y(),itemWidth,height,itemColor);
}

QRectF filterObject::boundingRect() const
{
    return QRectF(QPointF(0.0, 0.0), QPointF(width, height));
}

textObject::textObject(SpectrumScene *scene, QString text, QPoint location, QColor color)
{
    itemText = text;
    itemLocation = location;
    itemColor = color;
    width = scene->width();
    height = scene->height();
    itemType = 3;
}

void textObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setOpacity(1.0);
    painter->setPen(QPen(itemColor, 1));
    painter->setFont(QFont("Arial", 10));
    painter->drawText(itemLocation.x(),itemLocation.y(),itemText);
}

QRectF textObject::boundingRect() const
{
    return QRectF(QPointF(0.0, 0.0), QPointF(width, height));
}

spectrumObject::spectrumObject(int width, int height){
    setZValue(0.0);
    plot.clear();
    plotWidth = width;
    plotHeight = height;
    itemType = 4;
}

void spectrumObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    // plot Spectrum
    painter->setOpacity(0.9);
    painter->setPen(QPen(Qt::yellow, 1));
    if(plot.count()==plotWidth) {
        painter->drawPolyline(plot.constData(),plot.count());
    }
}

QRectF spectrumObject::boundingRect() const
{
    return QRectF(QPointF(0.0, 0.0), QPointF(plotWidth, plotHeight));
} /** end boundingRect **/

SpectrumScene::SpectrumScene(QObject *parent) : QGraphicsScene(parent){
}

void SpectrumScene::updatePlot(void)
{
    spectrumPlot = new spectrumObject(width(), height());
    addItem(spectrumPlot);
    update();
}
/***********************************************************************************/

Spectrum::Spectrum() {
}

Spectrum::Spectrum(QWidget*& widget) {
    QGraphicsView::setParent(widget);

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
    initialized = false;
    notchFilterSelected = -1;

    samples = (float*) malloc(MAX_WIDTH * sizeof (float));
    for (int i=0; i < MAX_WIDTH; i++) samples[i] = -120;

    spectrumScene = new SpectrumScene();
    this->setScene(spectrumScene);

    spectrumScene->clear();
    spectrumScene->sceneItems.clear();
    spectrumScene->spectrumPlot = NULL;

    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, Qt::black);
    gradient.setColorAt(1, Qt::gray);
    spectrumScene->setBackgroundBrush(gradient);
    spectrumScene->update();

 //   qDebug("View Rect: %d  %d   %d   %d", sceneRect().x(), sceneRect().y(), sceneRect().width(), sceneRect().height());
 //   setSceneRect(1, 1, width()/3, height()/3);
}

Spectrum::~Spectrum() {
    if (samples != NULL) free(samples);
}

void Spectrum::resizeEvent(QResizeEvent *event)
{
    if (!initialized) return;
    drawFrequencyLines();
    drawdBmLines();
    drawCursor(1, false);
    drawFilter(1, false);
    drawCursor(2, !subRx);
    drawFilter(2, !subRx);
}

void Spectrum::setHigh(int high) {
    spectrumHigh=high;
    //repaint();
    if (initialized)
        drawdBmLines();
    update();
}

void Spectrum::setLow(int low) {
    spectrumLow=low;
//    repaint();
    if (initialized)
        drawdBmLines();
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
    QGraphicsView::setVisible(true);
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
    if (initialized)
        drawBandLimits();
}

void Spectrum::setObjectName(QString name) {
    QGraphicsView::setObjectName(name);
}

void Spectrum::setGeometry(QRect rect) {
    QGraphicsView::setGeometry(rect);

    //qDebug() << "Spectrum:setGeometry: width=" << rect.width() << " height=" << rect.height();

    samples=(float*)malloc(rect.width()*sizeof(float));
}

void Spectrum::mousePressEvent(QMouseEvent* event) {

    //qDebug() << __FUNCTION__ << ": " << event->pos().x();

    qDebug() << "mousePressEvent: event->button(): " << event->button();

    button=event->button();
    startX=lastX=event->pos().x();
    lastY=event->pos().y();
    moved=0;

    if(squelch) {
        if(event->pos().y()>=(squelchY-1) &&
           event->pos().y()<=(squelchY+1)) {
            settingSquelch=true;
        } else {
            settingSquelch=false;
        }
    }

    // Notch filter  KD0OSS
    if (static_cast<notchFilterObject*>(itemAt(event->pos().x(), event->pos().y()))->itemType == 1)
        notchFilterSelected = static_cast<notchFilterObject*>(itemAt(event->pos().x(), event->pos().y()))->itemIndex;
    else
        notchFilterSelected = -1;
}

void Spectrum::mouseMoveEvent(QMouseEvent* event){
    int move=event->pos().x()-lastX;
    lastX=event->pos().x();
    int movey=event->pos().y()-lastY;
    lastY=event->pos().y();
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
    } else if (button == 1) {
        if (notchFilterSelected > -1) {
            this->setCursor(Qt::SizeHorCursor);
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
            notchFilterFO[notchFilterSelected] += move * move_step;
            notchFilterBW[notchFilterSelected] += movey * move_step;
            drawNotchFilter(subRx+1, notchFilterSelected, false);
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

    if (notchFilterSelected > -1)
    {
        emit notchFilterAdded(notchFilterSelected, notchFilterFO[notchFilterSelected], notchFilterBW[notchFilterSelected]);
        this->setCursor(Qt::ArrowCursor);
        button=-1;
        notchFilterSelected = -1;
    }

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
            float hzPixel = (float) sampleRate / width() / zoom_factor;  // spectrum resolution: Hz/pixel
            long freqOffsetPixel;
            long long f = frequency - (sampleRate/2/zoom_factor) + (event->pos().x()*hzPixel)-LO_offset;

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

// KD0OSS
void Spectrum::drawCursor(int vfo, bool disable)
{
    float step=(float)sampleRate/(float)width();
    int offset=(int)((float)LO_offset/step);
    int cursorX;
    QPen pen;

    if (!spectrumScene->sceneItems.isEmpty() && spectrumScene->sceneItems.value(QString("c%1").arg(vfo), 0))
    {
        spectrumScene->removeItem((lineObject*)spectrumScene->sceneItems.find(QString("c%1").arg(vfo)).value());
        spectrumScene->sceneItems.remove(QString("c%1").arg(vfo));
    }

    if (disable) return;

    float zoom_factor = 1.0f + zoom/25.0f;

    if (vfo == 1)
    {
        cursorX = (width()/2)+offset*zoom_factor;
        pen = QPen(QBrush(Qt::red,Qt::SolidPattern), 1);
    }
    else
    {
        cursorX = width()/2 + (float)(subRxFrequency-frequency+LO_offset) * (float)width()*zoom_factor/(float)sampleRate;
        pen = QPen(QBrush(Qt::blue,Qt::SolidPattern), 1);
    }

    lineObject *cursor = new lineObject(spectrumScene, QPoint(cursorX,0), QPoint(cursorX,height()), pen);
    spectrumScene->addItem(cursor);
    cursor->update();
    spectrumScene->sceneItems.insert(QString("c%1").arg(vfo), cursor);
}

// KD0OSS
void Spectrum::drawFilter(int vfo, bool disable)
{
    int filterLeft;
    int filterRight;
    QColor color;

    if (!spectrumScene->sceneItems.isEmpty() && spectrumScene->sceneItems.value(QString("flt%1").arg(vfo), 0))
    {
        spectrumScene->removeItem(spectrumScene->sceneItems.find(QString("flt%1").arg(vfo)).value());
        spectrumScene->sceneItems.remove(QString("flt%1").arg(vfo));
    }

    if (disable) return;

    float zoom_factor = 1.0f + zoom/25.0f;

    if (vfo == 1)
    {
        filterLeft = width()/2 + (float)(filterLow+LO_offset)* (float)width()*zoom_factor/(float)sampleRate;
        filterRight = width()/2 + (float)(filterHigh+LO_offset)*(float)width()*zoom_factor/(float)sampleRate;
        color = Qt::gray;
        qDebug("FL: %d  FR: %d", filterLow, filterHigh);
    }
    else
    {
        filterLeft = width()/2 + (float)(filterLow+LO_offset+subRxFrequency-frequency)
                * (float)width()*zoom_factor/(float)sampleRate;
        filterRight = width()/2 + (float)(filterHigh+LO_offset+subRxFrequency-frequency)
                * (float)width()*zoom_factor/(float)sampleRate;
        color = Qt::lightGray;
    }

    filterObject *filterItem = new filterObject(spectrumScene, QPoint(filterLeft,0), (float)(filterRight-filterLeft), color);
    spectrumScene->addItem(filterItem);
    filterItem->update();
    spectrumScene->sceneItems.insert(QString("flt%1").arg(vfo), filterItem);
}

// KD0OSS
void Spectrum:: drawNotchFilter(int vfo, int index, bool disable)
{
    int filterLeft;
    int filterRight;
    QColor color;

    if (!spectrumScene->sceneItems.isEmpty() && spectrumScene->sceneItems.value(QString("nf%1%2").arg(vfo).arg(index), 0))
    {
        spectrumScene->removeItem(spectrumScene->sceneItems.find(QString("nf%1%2").arg(vfo).arg(index)).value());
        spectrumScene->sceneItems.remove(QString("nf%1%2").arg(vfo).arg(index));
    }

    if (disable) return;

    float zoom_factor = 1.0f + zoom/25.0f;

    if (vfo == 1)
    {
        filterLeft =  width()/2 + (float)(notchFilterFO[index]+LO_offset)*(float)width()*zoom_factor/(float)sampleRate;
        filterRight = width()/2 + (float)(notchFilterBW[index]+notchFilterFO[index]+LO_offset)*(float)width()*zoom_factor/(float)sampleRate;
        color = Qt::darkGreen;
        qDebug("NFL: %d  NFR: %d", filterLeft, filterRight);
    }
    else
    {
        filterLeft = width()/2 + (float)(notchFilterFO[index]+LO_offset+subRxFrequency-frequency)
                * (float)width()*zoom_factor/(float)sampleRate;
        filterRight = width()/2 + (float)(notchFilterBW[index]+notchFilterFO[index]+LO_offset+subRxFrequency-frequency)
                * (float)width()*zoom_factor/(float)sampleRate;
        color = Qt::green;
    }

    notchFilterObject *filterItem = new notchFilterObject(spectrumScene, index, QPoint(filterLeft,0), (float)(filterRight-filterLeft), color);
    spectrumScene->addItem(filterItem);
    filterItem->update();
    spectrumScene->sceneItems.insert(QString("nf%1%2").arg(vfo).arg(index), filterItem);
}

// KD0OSS
void Spectrum:: updateNotchFilter(int vfo)
{
    int filterLeft;
    int filterRight;
    QColor color;

    for (int index=0;index<9;index++)
    {
        if (!spectrumScene->sceneItems.isEmpty() && spectrumScene->sceneItems.value(QString("nf%1%2").arg(vfo).arg(index), 0))
        {
            spectrumScene->removeItem(spectrumScene->sceneItems.find(QString("nf%1%2").arg(vfo).arg(index)).value());
            spectrumScene->sceneItems.remove(QString("nf%1%2").arg(vfo).arg(index));
        }
        else
            continue;

        float zoom_factor = 1.0f + zoom/25.0f;

        if (vfo == 1)
        {
            filterLeft =  width()/2 + (float)(notchFilterFO[index]+LO_offset)*(float)width()*zoom_factor/(float)sampleRate;
            filterRight = width()/2 + (float)(notchFilterBW[index]+notchFilterFO[index]+LO_offset)*(float)width()*zoom_factor/(float)sampleRate;
            color = Qt::darkGreen;
            qDebug("NFL: %d  NFR: %d", filterLeft, filterRight);
        }
        else
        {
            filterLeft = width()/2 + (float)(notchFilterFO[index]+LO_offset+subRxFrequency-frequency)
                    * (float)width()*zoom_factor/(float)sampleRate;
            filterRight = width()/2 + (float)(notchFilterBW[index]+notchFilterFO[index]+LO_offset+subRxFrequency-frequency)
                    * (float)width()*zoom_factor/(float)sampleRate;
            color = Qt::green;
        }

        notchFilterObject *filterItem = new notchFilterObject(spectrumScene, index, QPoint(filterLeft,0), (float)(filterRight-filterLeft), color);
        spectrumScene->addItem(filterItem);
        filterItem->update();
        spectrumScene->sceneItems.insert(QString("nf%1%2").arg(vfo).arg(index), filterItem);
    }
}

// KD0OSS
void Spectrum::drawdBmLines(void)
{
    static int lines;

    if (!spectrumScene->sceneItems.isEmpty())
    {
        for (int i=0;i<lines;i++)
        {
            spectrumScene->removeItem(spectrumScene->sceneItems.find(QString("dl%1").arg(i)).value());
            spectrumScene->sceneItems.remove(QString("dl%1").arg(i));
            spectrumScene->removeItem(spectrumScene->sceneItems.find(QString("dt%1").arg(i)).value());
            spectrumScene->sceneItems.remove(QString("dt%1").arg(i));
        }
    }
    lines = 0;
    int V = spectrumHigh - spectrumLow;
    int numSteps = V / 20;
    for (int i = 1; i < numSteps; i++) {
        int num = spectrumHigh - i * 20;
        int y = (int) floor((spectrumHigh - num) * height() / V);

        lineObject *lineItem = new lineObject(spectrumScene, QPoint(0,y), QPoint(width(),y), QPen(QColor(255,255,255,128), 1,Qt::DotLine));
        spectrumScene->addItem(lineItem);
        lineItem->update();
        spectrumScene->sceneItems.insert(QString("dl%1").arg(lines), lineItem);

        textObject *textItem = new textObject(spectrumScene, QString::number(num)+" dBm", QPoint(3,y), Qt::green);
        spectrumScene->addItem(textItem);
        textItem->update();

        spectrumScene->sceneItems.insert(QString("dt%1").arg(lines), textItem);
        lines++;
    }
}

// KD0OSS
void Spectrum::drawFrequencyLines(void)
{
    QString text;
    static int lines;

    float zoom_factor = 1.0f + zoom/25.0f;
    float hzPerPixel=(float)sampleRate/(float)width()/zoom_factor;
    long long lineStep = 10000;
    if (sampleRate > 1000000) lineStep = 100000;
    else if (sampleRate > 500000) lineStep = 50000;
    else if (sampleRate > 200000) lineStep = 20000;

    if (!spectrumScene->sceneItems.isEmpty())
    {
        for (int i=0;i<lines;i++)
        {
            spectrumScene->removeItem(spectrumScene->sceneItems.find(QString("fl%1").arg(i)).value());
            spectrumScene->sceneItems.remove(QString("fl%1").arg(i));
            spectrumScene->removeItem(spectrumScene->sceneItems.find(QString("ft%1").arg(i)).value());
            spectrumScene->sceneItems.remove(QString("ft%1").arg(i));
        }
    }
    lines = 0;
    for(int i=0;i<width();i++) {
        long long f=frequency-((float)sampleRate/zoom_factor/2.0)-(float)LO_offset+(long long)(hzPerPixel*(float)i);
        if(f>0) {
            if((f % lineStep)<(long long)hzPerPixel) {
                lineObject *lineItem = new lineObject(spectrumScene, QPoint(i,0), QPoint(i,height()), QPen(QColor(255,255,255,128), 1,Qt::DotLine));
                spectrumScene->addItem(lineItem);
                lineItem->update();
                spectrumScene->sceneItems.insert(QString("fl%1").arg(lines), lineItem);

                text.sprintf("%lld.%02lld",f/1000000,f%1000000/10000);
                textObject *textItem = new textObject(spectrumScene, text, QPoint(i,height()-5), Qt::gray);
                spectrumScene->addItem(textItem);
                textItem->update();
                spectrumScene->sceneItems.insert(QString("ft%1").arg(lines), textItem);
                lines++;
            }
        }
    }
}

// KD0OSS
void Spectrum::drawBandLimits(void)
{
    float zoom_factor = 1.0f + zoom/25.0f;
    float hzPerPixel=(float)sampleRate/(float)width()/zoom_factor;
    long long min_display=frequency-((float)sampleRate/zoom_factor/2.0);
    long long max_display=frequency+((float)sampleRate/zoom_factor/2.0);

    if (!spectrumScene->sceneItems.isEmpty())
    {
        if (spectrumScene->sceneItems.value(QString("bl%1").arg(0), 0))
        {
            spectrumScene->removeItem(spectrumScene->sceneItems.find(QString("bl%1").arg(0)).value());
            spectrumScene->sceneItems.remove(QString("bl%1").arg(0));
        }
        if (spectrumScene->sceneItems.value(QString("bl%1").arg(1), 0))
        {
            spectrumScene->removeItem(spectrumScene->sceneItems.find(QString("bl%1").arg(1)).value());
            spectrumScene->sceneItems.remove(QString("bl%1").arg(1));
        }
    }

    if(band_min!=0LL && band_max!=0LL) {
        int i;
        if((min_display<band_min)&&(max_display>band_min)) {
            i=(band_min-min_display)/(long long)hzPerPixel;
            lineObject *lineItem = new lineObject(spectrumScene, QPoint(i,0), QPoint(i,height()), QPen(Qt::red, 2,Qt::DotLine));
            spectrumScene->addItem(lineItem);
            lineItem->update();
            spectrumScene->sceneItems.insert(QString("bl%1").arg(0), lineItem);
        }
        if((min_display<band_max)&&(max_display>band_max)) {
            i=(band_max-min_display)/(long long)hzPerPixel;
            lineObject *lineItem = new lineObject(spectrumScene, QPoint(i+1,0), QPoint(i+1,height()), QPen(Qt::red, 2,Qt::DotLine));
            spectrumScene->addItem(lineItem);
            lineItem->update();
            spectrumScene->sceneItems.insert(QString("bl%1").arg(1), lineItem);
        }
    }
}

// KD0OSS
void Spectrum::drawSquelch(void)
{
    QString text;

    if (!spectrumScene->sceneItems.isEmpty() && spectrumScene->sceneItems.value("sl", 0))
    {
        spectrumScene->removeItem(spectrumScene->sceneItems.find("sl").value());
        spectrumScene->sceneItems.remove("sl");
        spectrumScene->removeItem(spectrumScene->sceneItems.find("st").value());
        spectrumScene->sceneItems.remove("st");
    }

    if(settingSquelch || showSquelchControl) {
        squelchY=(int) floor(((float) spectrumHigh - squelchVal)*(float) height() / (float) (spectrumHigh - spectrumLow));
        lineObject *lineItem = new lineObject(spectrumScene, QPoint(0,squelchY), QPoint(width(),squelchY), QPen(Qt::red, 1,Qt::DashLine));
        spectrumScene->addItem(lineItem);
        lineItem->update();
        text.sprintf("%s","Squelch");
        textObject *textItem = new textObject(spectrumScene, text, QPoint(width()-48,squelchY), Qt::red);
        spectrumScene->addItem(textItem);
        textItem->update();
        spectrumScene->sceneItems.insert("sl", lineItem);
        spectrumScene->sceneItems.insert("st", textItem);
    } else if(squelch) {
        squelchY=(int) floor(((float) spectrumHigh - squelchVal)*(float) height() / (float) (spectrumHigh - spectrumLow));
        lineObject *lineItem = new lineObject(spectrumScene, QPoint(0,squelchY), QPoint(width(),squelchY), QPen(Qt::red, 1,Qt::DashLine));
        spectrumScene->addItem(lineItem);
        lineItem->update();
        text.sprintf("%s","Squelch");
        textObject *textItem = new textObject(spectrumScene, text, QPoint(width()-48,squelchY), Qt::red);
        spectrumScene->addItem(textItem);
        textItem->update();
        spectrumScene->sceneItems.insert("sl", lineItem);
        spectrumScene->sceneItems.insert("st", textItem);
    }
}

void Spectrum::drawSpectrum(void)
{
    if(sampleRate==0) {
        qDebug() << "sampleRate is 0";
        return;
    }

    emit meterValue(meter, subrx_meter);

    // KD0OSS
    if (spectrumScene->spectrumPlot != NULL)
    {
        spectrumScene->removeItem(spectrumScene->spectrumPlot);
        delete spectrumScene->spectrumPlot;
        spectrumScene->spectrumPlot = NULL;
    }
    spectrumScene->updatePlot();
    spectrumScene->spectrumPlot->plot = plot;
    spectrumScene->spectrumPlot->update();

    // show the subrx frequency
    if(subRx) {
        // show the frequency
      //  painter.setPen(QPen(Qt::green,1));
      //  painter.setFont(QFont("Verdana", 30));
    }

    QTimer::singleShot(0,this,SLOT(update()));
}

void Spectrum::setZoom(int value){
    zoom = value;
    if (!initialized)
        return;
    drawFrequencyLines();
    drawBandLimits();
    drawCursor(1, false);
    drawFilter(1, false);
    drawCursor(2, !subRx);
    drawFilter(2, !subRx);
    updateNotchFilter(1);
    updateNotchFilter(2);
}

void Spectrum::setFrequency(long long f) {
    frequency=f;
    subRxFrequency=f;

    if (!initialized)
        return;
    drawFrequencyLines();
    drawBandLimits();
    drawCursor(1, false);
    drawFilter(1, false);
    drawCursor(2, !subRx);
    drawFilter(2, !subRx);

//    gvj code

//    strFrequency.sprintf("%lld.%03lld.%03lld",f/1000000,f%1000000/1000,f%1000);
//    strSubRxFrequency.sprintf("%lld.%03lld.%03lld",f/1000000,f%1000000/1000,f%1000);
    //qDebug() << "Spectrum:setFrequency: " << f;
}

void Spectrum::setSubRxFrequency(long long f) {
    subRxFrequency=f;
    strSubRxFrequency.sprintf("%lld.%03lld.%03lld",f/1000000,f%1000000/1000,f%1000);

    if (!initialized)
        return;
    drawCursor(2, false);
    drawFilter(2, false);
    //qDebug() << "Spectrum:setSubRxFrequency: " << f;
}

void Spectrum::setSubRxState(bool state) {
    subRx=state;
 //   drawFilter(2, !subRx);
}

void Spectrum::setFilter(int low, int high) {
    qDebug() << "Spectrum::setFilter " << low << "," << high;
    filterLow=low;
    filterHigh=high;
    if (!initialized)
        return;
    drawFilter(1, false);
    drawFilter(2, !subRx);
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
    if (!initialized)
        return;
    drawFilter(1, false);
    drawFilter(2, !subRx);
    update();
}

void Spectrum::updateSpectrumFrame(char* header,char* buffer,int width) {
    int i;
    int version,subversion;
    int header_sampleRate;
    static int lastWidth;
    static int lastHeight;

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
    if (width != lastWidth || height() != lastHeight)
    {
        spectrumScene->setSceneRect(0, 0, width, height());
        lastWidth = width;
        lastHeight = height();
        qDebug("Scene width: %d  ht: %d", width, height());
    }
    plot.clear();
    for (i = 0; i < width; i++) {
        plot << QPoint(i, (int) floor(((float) spectrumHigh - samples[i])*(float) height() / (float) (spectrumHigh - spectrumLow)));
    }

    if (!initialized)
    {

        initialized = true;
        spectrumScene->clear();
        drawFrequencyLines();
        drawdBmLines();
        drawCursor(1, false);
        drawFilter(1, false);
        updateNotchFilter(1);
    }

    QTimer::singleShot(0,this,SLOT(drawSpectrum()));
}

void Spectrum::setSquelch(bool state) {
    squelch=state;
    if (initialized)
        drawSquelch();
    QGraphicsView::setMouseTracking(state);
}

void Spectrum::setSquelchVal(float val) {
    squelchVal=val;
    squelchY=(int) floor(((float) spectrumHigh - squelchVal)*(float) height() / (float) (spectrumHigh - spectrumLow));
    if (initialized)
        drawSquelch();
    //qDebug()<<"Spectrum::setSquelchVal"<<val<<"squelchY="<<squelchY;
}

void Spectrum::addNotchFilter(int index){
    notchFilterIndex = index;
    if (!subRx)
        notchFilterVFO[notchFilterIndex] = 1;
    else
        notchFilterVFO[notchFilterIndex] = 2;
    notchFilterFO[notchFilterIndex] = -480.0;
    notchFilterBW[notchFilterIndex] = -1550.0;
    drawNotchFilter(notchFilterVFO[notchFilterIndex], notchFilterIndex, false);
    emit notchFilterAdded(index, -480.0, -1550.0);
}
