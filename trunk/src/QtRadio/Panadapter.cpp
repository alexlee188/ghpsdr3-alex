/*
 * File:   Panadapter.cpp
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

#include "Panadapter.h"

#define MAX_WIDTH 4096


/****************** Added by KD0OSS **********************************************/
lineObject::lineObject(PanadapterScene *scene, QPoint start, QPoint stop, QPen pen)
{
    itemStart = start;
    itemStop = stop;
    itemPen = pen;
    width = scene->width();
    height = scene->height();
    itemType = 0;
    setZValue(0.0);
}

void lineObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(itemPen);
    painter->drawLine(itemStart, itemStop);
}

QRectF lineObject::boundingRect() const
{
    return QRectF(itemStart, itemStop);
}

notchFilterObject::notchFilterObject(PanadapterScene *scene, int index, QPoint location, float fwidth, float fheight, QColor color)
{
    itemLocation = location;
    itemWidth = fwidth;
    itemColor = color;
    itemIndex = index;
    width = scene->width();
    height = fheight;
    itemType = 8;

    setZValue(9.0);
}

void notchFilterObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setOpacity(1.0);
    painter->setPen(itemColor);
    painter->drawRect(itemLocation.x(),itemLocation.y()-1,itemWidth,height);
    painter->fillRect(itemLocation.x(),itemLocation.y()-1,itemWidth,height,QBrush(itemColor, Qt::BDiagPattern));
}

QRectF notchFilterObject::boundingRect() const
{
    return QRectF(itemLocation, QPointF(itemLocation.x()+itemWidth, height));
}

filterObject::filterObject(PanadapterScene *scene, QPoint location, float fwidth, float fheight, QColor color)
{
    itemLocation = location;
    itemWidth = fwidth;
    itemColor = color;
    width = scene->width();
    height = fheight;
    itemType = 2;

    setZValue(8.0);
}

void filterObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setBrush(Qt::SolidPattern);
    painter->setOpacity(0.5);
    painter->fillRect(itemLocation.x(),itemLocation.y(),itemWidth,height,itemColor);
}

QRectF filterObject::boundingRect() const
{
    return QRectF(itemLocation, QPointF(itemLocation.x()+itemWidth, height));
}

textObject::textObject(PanadapterScene *scene, QString text, QPoint location, QColor color)
{
    itemText = text;
    itemLocation = location;
    itemColor = color;
    width = scene->width();
    height = scene->height();
    itemType = 3;
    setZValue(0.0);
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
    return QRectF(itemLocation, QPointF(50, 10));
}

spectrumObject::spectrumObject(int width, int height){
    plot.clear();
    plotWidth = width;
    plotHeight = height;
    itemType = 4;
    setZValue(1.0);
}

void spectrumObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    // plot Panadapter
    painter->setOpacity(0.9);
    painter->setPen(QPen(Qt::yellow, 1));
    if(plot.count() == plotWidth) {
        painter->drawPolyline(plot.constData(),plot.count());
    }
}

QRectF spectrumObject::boundingRect() const
{
    return QRectF(QPointF(0.0, plotHeight/2), QPointF(plotWidth, plotHeight));
} /** end boundingRect **/

PanadapterScene::PanadapterScene(QObject *parent) : QGraphicsScene(parent){
}
/***********************************KD0OSS******************************************/

Panadapter::Panadapter() {
}

Panadapter::Panadapter(QWidget*& widget) {
    QGraphicsView::setParent(widget);

    //qDebug() << "Panadapter::Panadapter " << width() << ":" << height();

    sampleRate=96000;
    spectrumHigh=-40;
    spectrumLow=-160;
    filterLow=-3450;
    filterHigh=-150;
    filterSelected = false; // KD0OSS
    avg = 0;
    mode="LSB";

    zoom = 0;
    sampleZoom = false;
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
    initialized = false; // KD0OSS
    notchFilterIndex = 0; // KD0OSS
    notchFilterSelected = -1; // KD0OSS
    for (int i=0;i<9;i++) // KD0OSS
    {
        notchFilterEnabled[i] = false;
        notchFilterFO[i] = 0.0;
        notchFilterBand[i] = "NA";
    }

    wsamples = (char*) malloc(MAX_WIDTH * sizeof (char)); // KD0OSS
    samples = (float*) malloc(MAX_WIDTH * sizeof (float));
    for (int i=0; i < MAX_WIDTH; i++) samples[i] = -120;

    // KD0OSS ****************************************************************************************
    panadapterScene = new PanadapterScene();
    this->setScene(panadapterScene);

    panadapterScene->clear();
    panadapterScene->sceneItems.clear();
    panadapterScene->spectrumPlot = NULL;

    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, Qt::black);
    gradient.setColorAt(1, Qt::gray);
    panadapterScene->setBackgroundBrush(gradient);

    // Context menu for notch filters
    notchFilterDeleteAction = new QAction(tr("&Delete"), this);
    notchFilterDeleteAction->setToolTip("Delete selected notch filter.");
    connect(notchFilterDeleteAction, SIGNAL(triggered()), this, SLOT(deleteNotchFilter()));

    panadapterScene->update();

    panadapterScene->spectrumPlot = new spectrumObject(width(), height()/2);
    panadapterScene->waterfallItem = new waterfallObject(width(), height()/2);
    splitViewBoundary = panadapterScene->height()/2;

    updateNfTimer = new QTimer(this);
    connect(updateNfTimer, SIGNAL(timeout()), this, SLOT(updateNotchFilter()));
    //*************************************************************************************************
}

Panadapter::~Panadapter() {
    if (samples != NULL) free(samples);
    if (wsamples != NULL) free(wsamples); // KD0OSS
}

void Panadapter::resizeEvent(QResizeEvent *event)
{
    // KD0OSS **********************
    if (!initialized || splitViewBoundary > height())
    {
        splitViewBoundary = (height() / 2) - 3;
        if (!initialized) return;
    }
    drawFrequencyLines();
    drawdBmLines();
    drawBandLimits();
    drawCursor(1, false);
    drawFilter(1, false);
    drawCursor(2, !subRx);
    drawFilter(2, !subRx);
    drawUpdatedNotchFilter(1);
//    drawUpdatedNotchFilter(2);
    //******************************
}

void Panadapter::redrawItems(void)
{
    if (!initialized || splitViewBoundary > height())
    {
        splitViewBoundary = (height() / 2) - 3;
        if (!initialized) return;
    }
    drawFrequencyLines();
    drawdBmLines();
    drawBandLimits();
    drawCursor(1, false);
    drawFilter(1, false);
    drawCursor(2, !subRx);
    drawFilter(2, !subRx);
    drawUpdatedNotchFilter(1);
}

void Panadapter::setHigh(int high) {
    spectrumHigh=high;
    //repaint();
    if (initialized)   // KD0OSS
        drawdBmLines();
    update();
}

void Panadapter::setLow(int low) {
    spectrumLow=low;
//    repaint();
    if (initialized)   // KD0OSS
        drawdBmLines();
    update();
}

int Panadapter::getHigh() {
    return spectrumHigh;
}

int Panadapter::getLow() {
    return spectrumLow;
}

void Panadapter::setAvg(int value){
    avg = value;
}

void Panadapter::initialize() {
    QGraphicsView::setVisible(true);
}

void Panadapter::setSampleRate(int r) {
    sampleRate=r;
}

int Panadapter::samplerate() {
    return sampleRate;
}

void Panadapter::setBandLimits(long long min,long long max) {

    qDebug() << "Panadapter::setBandLimits: " << min << "," << max;
    band_min=min;
    band_max=max;
    if (initialized)   // KD0OSS
        drawBandLimits();
}

void Panadapter::setObjectName(QString name) {
    QGraphicsView::setObjectName(name);
}

void Panadapter::setGeometry(QRect rect) {
    QGraphicsView::setGeometry(rect);

    //qDebug() << "Panadapter:setGeometry: width=" << rect.width() << " height=" << rect.height();

    samples=(float*)malloc(rect.width()*sizeof(float));
}

void Panadapter::mousePressEvent(QMouseEvent* event) {

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
    if (items(event->pos()).size() > 0)
    {
        if (static_cast<notchFilterObject*>(itemAt(event->pos().x(), event->pos().y()))->itemType == 8 && button == 1)
            notchFilterSelected = static_cast<notchFilterObject*>(itemAt(event->pos().x(), event->pos().y()))->itemIndex;
        else
            notchFilterSelected = -1;

        if (static_cast<notchFilterObject*>(itemAt(event->pos().x(), event->pos().y()))->itemType == 8 && button == 2)
        {
            notchFilterSelected = static_cast<notchFilterObject*>(itemAt(event->pos().x(), event->pos().y()))->itemIndex;
            QMenu menu;
            menu.addAction(notchFilterDeleteAction);
            menu.exec(event->globalPos());
            this->setCursor(Qt::ArrowCursor);
  //          qDebug("Item: %d", notchFilterSelected);
            return;
        }

        if (static_cast<filterObject*>(itemAt(event->pos().x(), event->pos().y()))->itemType == 2)
        {
            this->setCursor(Qt::SizeAllCursor);
            filterSelected = true;
        }
    }
}

void Panadapter::mouseMoveEvent(QMouseEvent* event){
    if (button == 2) return;   // KD0OSS
    int move=event->pos().x()-lastX;
    lastX=event->pos().x();
    int movey=event->pos().y()-lastY;   // KD0OSS
    lastY=event->pos().y();   // KD0OSS
 //   qDebug() << __FUNCTION__ << ": " << event->pos().y() << " move: " << splitViewBoundary;

    moved=1;
    emit statusMessage("");

    if(button == -1) {
        if(squelch &&
           event->pos().y()>=(squelchY-1) &&
           event->pos().y()<=(squelchY+1)) {
            showSquelchControl=true;
            this->setCursor(Qt::SizeVerCursor);
            emit statusMessage("Left click and drag to adjust squelch.");   // KD0OSS
        } else if (lastY >= (splitViewBoundary-1) && lastY <= (splitViewBoundary+1))   // KD0OSS
        {
            this->setCursor(Qt::SizeVerCursor);
            showSquelchControl=false;
            adjustSplitViewBoundary = true;
            emit statusMessage("Left click and drag to adjust panadapter ratio.");
        }
        else if (items(event->pos()).size() > 0)   // KD0OSS
        {
            showSquelchControl=false;
            adjustSplitViewBoundary = false;
            if (static_cast<notchFilterObject*>(itemAt(event->pos().x(), event->pos().y()))->itemType == 8)
            {
                this->setCursor(Qt::SizeAllCursor);
                emit statusMessage("Right click on notch filter for more actions");
            } else if (static_cast<filterObject*>(itemAt(event->pos().x(), event->pos().y()))->itemType == 2)
            {
                this->setCursor(Qt::SizeAllCursor);
                emit statusMessage("Left click and drag to adjust RX filter.");
            }
            else
                this->setCursor(Qt::ArrowCursor);
        }
        else
        {
            showSquelchControl=false;
            adjustSplitViewBoundary = false;
            this->setCursor(Qt::ArrowCursor);
        }
    } else if (button == 1 && notchFilterSelected > -1) {   // KD0OSS
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
            notchFilterFO[notchFilterSelected] += (move * move_step);
            if ((notchFilterBW[notchFilterSelected] - (movey * move_step)) < 1)
                notchFilterBW[notchFilterSelected] = 1;
            else
                notchFilterBW[notchFilterSelected] -= (movey * move_step);
            drawNotchFilter(1, notchFilterSelected, false);
    }  else if (button == 1 && filterSelected) {   // KD0OSS
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
        int bw = abs(filterLow-filterHigh);
        if ((bw - (movey * move_step)) < 100)
            setFilter(filterLow + (move * move_step), filterLow + (move * move_step) + 100);
        else
            setFilter(filterLow + (move * move_step), filterLow + (move * move_step) + bw - (movey * move_step));
     } else {
        if(settingSquelch) {
            int delta=squelchY-event->pos().y();
            delta=int((float)delta*((float)(spectrumHigh-spectrumLow)/(float)height()));
            //qDebug()<<"squelchValueChanged"<<delta<<"squelchY="<<squelchY<<" y="<<event->pos().y();
            emit squelchValueChanged(delta);
            //squelchY=event->pos().y();
        } else if (adjustSplitViewBoundary) {   // KD0OSS
            splitViewBoundary = lastY;
            drawdBmLines();
            drawFrequencyLines();
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

void Panadapter::mouseReleaseEvent(QMouseEvent* event) {
    int move=event->pos().x()-lastX;
    lastX=event->pos().x();
    //qDebug() << __FUNCTION__ << ": " << event->pos().x() << " move:" << move;

    if (notchFilterSelected > -1 && button == 1)   // KD0OSS
    {
        this->setCursor(Qt::ArrowCursor);
        button = -1;
        notchFilterSelected = -1;
    }

    if (filterSelected && button == 1)   // KD0OSS
    {
        this->setCursor(Qt::ArrowCursor);
        button = -1;
        filterSelected = false;
        emit variableFilter(filterLow, filterHigh);
    }

    if (adjustSplitViewBoundary)  // KD0OSS
    {
        button = -1;
        adjustSplitViewBoundary = false;
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
            float hzPixel = (float) sampleRate / width() / zoom_factor;  // Panadapter resolution: Hz/pixel
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

void Panadapter::wheelEvent(QWheelEvent *event) {
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
        drawUpdatedNotchFilter(1);
        updateNotchFilter(-1);
    } else {
        // wheel event on the left side, change the vertical axis values
        float shift =  (float)(event->delta()/8/15 * 5)                    // phy steps of wheel * 5
                     * ((float)(spectrumHigh - spectrumLow) / (height()/2));   // dBm / pixel on vertical axis
       
        if (event->buttons() == Qt::MidButton) {
           // change the vertical axis range
           //qDebug() << __FUNCTION__ << " change vertical axis scale: " << shift;
           emit spectrumHighChanged (spectrumHigh+(int)shift);
           emit spectrumLowChanged  (spectrumLow-(int)shift);
           emit waterfallHighChanged (spectrumHigh+(int)shift);
           emit waterfallLowChanged  (spectrumLow-(int)shift);

        } else {
          // if middle mouse button pressed shift the Panadapter scale
          //qDebug() << __FUNCTION__ << " shift on vertical axis scale: " << shift;
          emit spectrumHighChanged (spectrumHigh+(int)shift);
          emit spectrumLowChanged  (spectrumLow+(int)shift);
          emit waterfallHighChanged (spectrumHigh+(int)shift);
          emit waterfallLowChanged  (spectrumLow+(int)shift);

       }
    }
}

// KD0OSS
void Panadapter::drawCursor(int vfo, bool disable)
{
    float step=(float)sampleRate/(float)width();
    int offset=(int)((float)LO_offset/step);
    int cursorX;
    QPen pen;

    if (!panadapterScene->sceneItems.isEmpty() && panadapterScene->sceneItems.value(QString("c%1").arg(vfo), 0))
    {
        panadapterScene->removeItem((lineObject*)panadapterScene->sceneItems.find(QString("c%1").arg(vfo)).value());
        delete (lineObject*)panadapterScene->sceneItems.find(QString("c%1").arg(vfo)).value();
        panadapterScene->sceneItems.remove(QString("c%1").arg(vfo));
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

    lineObject *cursor = new lineObject(panadapterScene, QPoint(cursorX,splitViewBoundary), QPoint(cursorX,0), pen);
    panadapterScene->addItem(cursor);
    cursor->update();
    panadapterScene->sceneItems.insert(QString("c%1").arg(vfo), cursor);
}

// KD0OSS
void Panadapter::drawFilter(int vfo, bool disable)
{
    int filterLeft;
    int filterRight;
    QColor color;

    if (!panadapterScene->sceneItems.isEmpty() && panadapterScene->sceneItems.value(QString("flt%1").arg(vfo), 0))
    {
        panadapterScene->removeItem(panadapterScene->sceneItems.find(QString("flt%1").arg(vfo)).value());
        delete (filterObject*)panadapterScene->sceneItems.find(QString("flt%1").arg(vfo)).value();
        panadapterScene->sceneItems.remove(QString("flt%1").arg(vfo));
    }

    if (disable) return;

    float zoom_factor = 1.0f + zoom/25.0f;

    if (vfo == 1)
    {
        filterLeft = width()/2 + (float)(filterLow+LO_offset)* (float)width()*zoom_factor/(float)sampleRate;
        filterRight = width()/2 + (float)(filterHigh+LO_offset)*(float)width()*zoom_factor/(float)sampleRate;
        color = Qt::gray;
     //   qDebug("FL: %d  FR: %d", filterLow, filterHigh);
    }
    else
    {
        filterLeft = width()/2 + (float)(filterLow+LO_offset+subRxFrequency-frequency)
                * (float)width()*zoom_factor/(float)sampleRate;
        filterRight = width()/2 + (float)(filterHigh+LO_offset+subRxFrequency-frequency)
                * (float)width()*zoom_factor/(float)sampleRate;
        color = Qt::lightGray;
    }

    filterObject *filterItem = new filterObject(panadapterScene, QPoint(filterLeft,0), (float)(filterRight-filterLeft), (float)splitViewBoundary, color);
    filterItem->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    panadapterScene->addItem(filterItem);
    filterItem->update();
    panadapterScene->sceneItems.insert(QString("flt%1").arg(vfo), filterItem);
}

// KD0OSS
void Panadapter:: drawNotchFilter(int vfo, int index, bool disable)
{
    int filterLeft;
    int filterRight;
    QColor color;

    if (!panadapterScene->sceneItems.isEmpty() && panadapterScene->sceneItems.value(QString("nf%1%2").arg(vfo).arg(index), 0))
    {
        panadapterScene->removeItem(panadapterScene->sceneItems.find(QString("nf%1%2").arg(vfo).arg(index)).value());
        delete (notchFilterObject*)panadapterScene->sceneItems.find(QString("nf%1%2").arg(vfo).arg(index)).value();
        panadapterScene->sceneItems.remove(QString("nf%1%2").arg(vfo).arg(index));
    }

    if (disable) return;

    float zoom_factor = 1.0f + zoom/25.0f;

    if (vfo == 1)
    {
        filterLeft =  width()/2 + (float)(notchFilterFO[index]-frequency+LO_offset-(notchFilterBW[index]/2))*(float)width()*zoom_factor/(float)sampleRate;
        filterRight =  width()/2 + (float)(notchFilterFO[index]-frequency+LO_offset+(notchFilterBW[index]/2))*(float)width()*zoom_factor/(float)sampleRate;
        color = Qt::green;
 //       qDebug("NFL: %d  NFR: %d", filterLeft, filterRight);
    }
    else
    {
        filterLeft = width()/2 + (float)(notchFilterFO[index]-frequency+LO_offset+subRxFrequency-frequency-(notchFilterBW[index]/2)) * (float)width()*zoom_factor/(float)sampleRate;
        filterRight = width()/2 + (float)(notchFilterFO[index]-frequency+LO_offset+subRxFrequency-frequency+(notchFilterBW[index]/2)) * (float)width()*zoom_factor/(float)sampleRate;
        color = Qt::green;
    }

    if ((filterRight - filterLeft) < 1)
        filterRight = filterLeft + 1;

    notchFilterObject *filterItem = new notchFilterObject(panadapterScene, index, QPoint(filterLeft,0), (float)(filterRight-filterLeft), splitViewBoundary, color);
    panadapterScene->addItem(filterItem);
    filterItem->update();
    panadapterScene->sceneItems.insert(QString("nf%1%2").arg(vfo).arg(index), filterItem);
}

// KD0OSS
void Panadapter:: drawUpdatedNotchFilter(int vfo)
{
    int filterLeft;
    int filterRight;
    QColor color;

    for (int index=0;index<9;index++)
    {
        if (!panadapterScene->sceneItems.isEmpty() && panadapterScene->sceneItems.value(QString("nf%1%2").arg(vfo).arg(index), 0))
        {
            panadapterScene->removeItem(panadapterScene->sceneItems.find(QString("nf%1%2").arg(vfo).arg(index)).value());
            delete (notchFilterObject*)panadapterScene->sceneItems.find(QString("nf%1%2").arg(vfo).arg(index)).value();
            panadapterScene->sceneItems.remove(QString("nf%1%2").arg(vfo).arg(index));
        }
   //     else
       //     continue;

        if (notchFilterBand[index] != band) continue;

        float zoom_factor = 1.0f + zoom/25.0f;

        if (vfo == 1)
        {
            filterLeft =  width()/2 + (float)(notchFilterFO[index]-frequency+LO_offset-(notchFilterBW[index]/2))*(float)width()*zoom_factor/(float)sampleRate;
            filterRight =  width()/2 + (float)(notchFilterFO[index]-frequency+LO_offset+(notchFilterBW[index]/2))*(float)width()*zoom_factor/(float)sampleRate;
            color = Qt::green;
      //      qDebug("NFL: %d  NFR: %d", filterLeft, filterRight);
        }
        else
        {
            filterLeft = width()/2 + (float)(notchFilterFO[index]-frequency+LO_offset+subRxFrequency-frequency-(notchFilterBW[index]/2)) * (float)width()*zoom_factor/(float)sampleRate;
            filterRight = width()/2 + (float)(notchFilterFO[index]-frequency+LO_offset+subRxFrequency-frequency+(notchFilterBW[index]/2)) * (float)width()*zoom_factor/(float)sampleRate;
            color = Qt::green;
        }

        notchFilterObject *filterItem = new notchFilterObject(panadapterScene, index, QPoint(filterLeft,0), (float)(filterRight-filterLeft), splitViewBoundary, color);
        panadapterScene->addItem(filterItem);
        filterItem->update();
        panadapterScene->sceneItems.insert(QString("nf%1%2").arg(vfo).arg(index), filterItem);
    }
}

// KD0OSS
void Panadapter::drawdBmLines(void)
{
    static int lines;

    if (!panadapterScene->sceneItems.isEmpty())
    {
        for (int i=0;i<lines;i++)
        {
            panadapterScene->removeItem(panadapterScene->sceneItems.find(QString("dl%1").arg(i)).value());
            delete (lineObject*)panadapterScene->sceneItems.find(QString("dl%1").arg(i)).value();
            panadapterScene->sceneItems.remove(QString("dl%1").arg(i));
            panadapterScene->removeItem(panadapterScene->sceneItems.find(QString("dt%1").arg(i)).value());
            delete (textObject*)panadapterScene->sceneItems.find(QString("dt%1").arg(i)).value();
            panadapterScene->sceneItems.remove(QString("dt%1").arg(i));
        }
    }
    lines = 0;
    int V = spectrumHigh - spectrumLow;
    int numSteps = V / 20;
    for (int i = 1; i < numSteps; i++) {
        int num = spectrumHigh - i * 20;
        int y = (int) floor((spectrumHigh - num) * (splitViewBoundary) / V);

        lineObject *lineItem = new lineObject(panadapterScene, QPoint(0,y), QPoint(width(),y), QPen(QColor(255,255,255,128), 1,Qt::DotLine));
        panadapterScene->addItem(lineItem);
        lineItem->update();
        panadapterScene->sceneItems.insert(QString("dl%1").arg(lines), lineItem);

        textObject *textItem = new textObject(panadapterScene, QString::number(num)+" dBm", QPoint(3,y), Qt::green);
        panadapterScene->addItem(textItem);
        textItem->update();

        panadapterScene->sceneItems.insert(QString("dt%1").arg(lines), textItem);
        lines++;
    }
}

// KD0OSS
void Panadapter::drawFrequencyLines(void)
{
    QString text;
    static int lines;

    float zoom_factor = 1.0f + zoom/25.0f;
    float hzPerPixel=(float)sampleRate/(float)width()/zoom_factor;
    long long lineStep = 10000;
    if (sampleRate > 1000000) lineStep = 100000;
    else if (sampleRate > 500000) lineStep = 50000;
    else if (sampleRate > 200000) lineStep = 20000;

    if (!panadapterScene->sceneItems.isEmpty())
    {
        for (int i=0;i<lines;i++)
        {
            panadapterScene->removeItem(panadapterScene->sceneItems.find(QString("fl%1").arg(i)).value());
            delete (lineObject*)panadapterScene->sceneItems.find(QString("fl%1").arg(i)).value();
            panadapterScene->sceneItems.remove(QString("fl%1").arg(i));
            panadapterScene->removeItem(panadapterScene->sceneItems.find(QString("ft%1").arg(i)).value());
            delete (textObject*)panadapterScene->sceneItems.find(QString("ft%1").arg(i)).value();
            panadapterScene->sceneItems.remove(QString("ft%1").arg(i));
        }
    }
    lines = 0;
    for(int i=0;i<width();i++) {
        long long f=frequency-((float)sampleRate/zoom_factor/2.0)-(float)LO_offset+(long long)(hzPerPixel*(float)i);
        if(f>0) {
            if((f % lineStep)<(long long)hzPerPixel) {
           //     qDebug("height: %d", height());
                lineObject *lineItem = new lineObject(panadapterScene, QPoint(i,splitViewBoundary), QPoint(i,0), QPen(QColor(255,255,255,128), 1,Qt::DotLine));
                panadapterScene->addItem(lineItem);
                lineItem->update();
                panadapterScene->sceneItems.insert(QString("fl%1").arg(lines), lineItem);

                text.sprintf("%lld.%02lld",f/1000000,f%1000000/10000);
                textObject *textItem = new textObject(panadapterScene, text, QPoint(i,(splitViewBoundary)-10), Qt::lightGray);
                panadapterScene->addItem(textItem);
                textItem->update();
                panadapterScene->sceneItems.insert(QString("ft%1").arg(lines), textItem);
                lines++;
            }
        }
    }
}

// KD0OSS
void Panadapter::drawBandLimits(void)
{
    float zoom_factor = 1.0f + zoom/25.0f;
    float hzPerPixel=(float)sampleRate/(float)width()/zoom_factor;
    long long min_display=frequency-((float)sampleRate/zoom_factor/2.0);
    long long max_display=frequency+((float)sampleRate/zoom_factor/2.0);

    if (!panadapterScene->sceneItems.isEmpty())
    {
        if (panadapterScene->sceneItems.value(QString("bl0"), 0))
        {
            panadapterScene->removeItem(panadapterScene->sceneItems.find(QString("bl0")).value());
            delete (lineObject*)panadapterScene->sceneItems.find(QString("bl0")).value();
            panadapterScene->sceneItems.remove(QString("bl0"));
        }
        if (panadapterScene->sceneItems.value(QString("bl1"), 0))
        {
            panadapterScene->removeItem(panadapterScene->sceneItems.find(QString("bl1")).value());
            delete (lineObject*)panadapterScene->sceneItems.find(QString("bl1")).value();
            panadapterScene->sceneItems.remove(QString("bl1"));
        }
    }

    if(band_min!=0LL && band_max!=0LL) {
        int i;
        if((min_display<band_min)&&(max_display>band_min)) {
            i=(band_min-min_display)/(long long)hzPerPixel;
            lineObject *lineItem = new lineObject(panadapterScene, QPoint(i,splitViewBoundary), QPoint(i,0), QPen(Qt::red, 2,Qt::DotLine));
            panadapterScene->addItem(lineItem);
            lineItem->update();
            panadapterScene->sceneItems.insert(QString("bl0"), lineItem);
        }
        if((min_display<band_max)&&(max_display>band_max)) {
            i=(band_max-min_display)/(long long)hzPerPixel;
            lineObject *lineItem = new lineObject(panadapterScene, QPoint(i+1,splitViewBoundary), QPoint(i+1,0), QPen(Qt::red, 2,Qt::DotLine));
            panadapterScene->addItem(lineItem);
            lineItem->update();
            panadapterScene->sceneItems.insert(QString("bl1"), lineItem);
        }
    }
}

// KD0OSS
void Panadapter::drawSquelch(void)
{
    QString text;

    if (!panadapterScene->sceneItems.isEmpty() && panadapterScene->sceneItems.value("sl", 0))
    {
        panadapterScene->removeItem(panadapterScene->sceneItems.find("sl").value());
        delete (lineObject*)panadapterScene->sceneItems.find(QString("sl")).value();
        panadapterScene->sceneItems.remove("sl");
        panadapterScene->removeItem(panadapterScene->sceneItems.find("st").value());
        delete (textObject*)panadapterScene->sceneItems.find(QString("st")).value();
        panadapterScene->sceneItems.remove("st");
    }

    if(settingSquelch || showSquelchControl) {
        squelchY=(int) floor(((float) spectrumHigh - squelchVal)*(float)(splitViewBoundary) / (float) (spectrumHigh - spectrumLow));
        lineObject *lineItem = new lineObject(panadapterScene, QPoint(0,squelchY), QPoint(width(),squelchY), QPen(Qt::red, 1,Qt::DashLine));
        panadapterScene->addItem(lineItem);
        lineItem->update();
        text.sprintf("%s","Squelch");
        textObject *textItem = new textObject(panadapterScene, text, QPoint(width()-48,squelchY), Qt::red);
        panadapterScene->addItem(textItem);
        textItem->update();
        panadapterScene->sceneItems.insert("sl", lineItem);
        panadapterScene->sceneItems.insert("st", textItem);
    } else if(squelch) {
        squelchY=(int) floor(((float) spectrumHigh - squelchVal)*(float)(splitViewBoundary) / (float) (spectrumHigh - spectrumLow));
        lineObject *lineItem = new lineObject(panadapterScene, QPoint(0,squelchY), QPoint(width(),squelchY), QPen(Qt::red, 1,Qt::DashLine));
        panadapterScene->addItem(lineItem);
        lineItem->update();
        text.sprintf("%s","Squelch");
        textObject *textItem = new textObject(panadapterScene, text, QPoint(width()-48,squelchY), Qt::red);
        panadapterScene->addItem(textItem);
        textItem->update();
        panadapterScene->sceneItems.insert("sl", lineItem);
        panadapterScene->sceneItems.insert("st", textItem);
    }
}

void Panadapter::drawSpectrum(void)
{
    if(sampleRate==0) {
        qDebug() << "sampleRate is 0";
        return;
    }

    emit meterValue(meter, subrx_meter);

    // KD0OSS ****************************************************
    if (panadapterScene->spectrumPlot != NULL)
    {
        panadapterScene->removeItem(panadapterScene->spectrumPlot);
        delete panadapterScene->spectrumPlot;
        panadapterScene->spectrumPlot = NULL;
    }
    panadapterScene->spectrumPlot = new spectrumObject(panadapterScene->width(), splitViewBoundary);
    panadapterScene->addItem(panadapterScene->spectrumPlot);
    panadapterScene->update();
    panadapterScene->spectrumPlot->plot = plot;
 //   panadapterScene->spectrumPlot->update();
    //************************************************************

    // show the subrx frequency
    if(subRx) {
        // show the frequency
      //  painter.setPen(QPen(Qt::green,1));
      //  painter.setFont(QFont("Verdana", 30));
    }

    QTimer::singleShot(0,this,SLOT(update()));
}

void Panadapter::setZoom(int value){
    // KD0OSS ***************************
    static int vzoom;

    if (sampleZoom)
        zoom = value;
    else
    {
        setMatrix(QMatrix((value * 0.01)+1, 0.0, 0.0, 1.0, 1.0, 1.0));
        vzoom = value;
    }

    if (vzoom > 0)
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    else
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    if (!initialized)
        return;
    drawFrequencyLines();
    drawBandLimits();
    drawCursor(1, false);
    drawFilter(1, false);
    drawCursor(2, !subRx);
    drawFilter(2, !subRx);
    drawUpdatedNotchFilter(1);
    panadapterScene->removeItem(panadapterScene->waterfallItem);
    panadapterScene->addItem(panadapterScene->waterfallItem);
 //   drawUpdatedNotchFilter(2);
    //***********************************
}

void Panadapter::setFrequency(long long f) {
    frequency=f;
    subRxFrequency=f;

    // KD0OSS ***************************
    if (!initialized)
        return;
    drawFrequencyLines();
    drawBandLimits();
    drawCursor(1, false);
    drawFilter(1, false);
    drawCursor(2, !subRx);
    drawFilter(2, !subRx);
    drawUpdatedNotchFilter(1);
    updateNfTimer->start(1000);
    //***********************************

//    gvj code

//    strFrequency.sprintf("%lld.%03lld.%03lld",f/1000000,f%1000000/1000,f%1000);
//    strSubRxFrequency.sprintf("%lld.%03lld.%03lld",f/1000000,f%1000000/1000,f%1000);
    qDebug() << "Panadapter:setFrequency: " << f;
}

void Panadapter::setSubRxFrequency(long long f) {
    subRxFrequency=f;
    strSubRxFrequency.sprintf("%lld.%03lld.%03lld",f/1000000,f%1000000/1000,f%1000);

    if (!initialized)   // KD0OSS
        return;
    drawCursor(2, false);   // KD0OSS
    drawFilter(2, false);   // KD0OSS
    //qDebug() << "Panadapter:setSubRxFrequency: " << f;
}

void Panadapter::setSubRxState(bool state) {
    subRx=state;
 //   drawFilter(2, !subRx);
}

void Panadapter::setFilter(int low, int high) {
    qDebug() << "Panadapter::setFilter " << low << "," << high;
    filterLow=low;
    filterHigh=high;
    if (!initialized)   // KD0OSS
        return;
    drawFilter(1, false);   // KD0OSS
    drawFilter(2, !subRx);   // KD0OSS
}

void Panadapter::setHost(QString h) {
    host=h;
//    repaint();
//    update();
}

void Panadapter::setReceiver(int r) {
    receiver=r;
//    repaint();
    update();
}

void Panadapter::setMode(QString m) {
    mode=m;
    qDebug()<<Q_FUNC_INFO<<": Mode changed to "<< m;
    update();
}

void Panadapter::setBand(QString b) {
    band=b;
    qDebug()<<Q_FUNC_INFO<<": Band changed to "<< b;
//    repaint();
    update();
}

void Panadapter::setFilter(QString f) {
    filter=f;
//    repaint();
    if (!initialized)   // KD0OSS
        return;
    drawFilter(1, false);   // KD0OSS
    drawFilter(2, !subRx);   // KD0OSS
    update();
}

void Panadapter::updateSpectrumFrame(char* header,char* buffer,int width) {
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
//    qDebug("Meter: %d", meter);

    sampleRate = header_sampleRate;
    size = width;

    // do not rotate Panadapter display.  LO_offset rotation done in dspserver
    for(i=0;i<width;i++) {
        samples[i] = (float)(samples[i] * avg - (buffer[i] & 0xFF))/(float)(avg+1);
        wsamples[i] = buffer[i];
    }

    //qDebug() << "updateSpectrum: create plot points";
    if (width != lastWidth || height() != lastHeight)
    {
        panadapterScene->setSceneRect(0, 0, width, height());
        lastWidth = width;
        lastHeight = height();
        qDebug("Scene width: %d  ht: %d", width, height());
    }
    plot.clear();
    for (i = 0; i < width; i++) {
        plot << QPoint(i, (int) floor(((float) spectrumHigh - samples[i])*(float)(splitViewBoundary) / (float) (spectrumHigh - spectrumLow)));
    }

    if (!initialized)   // KD0OSS
    {
        initialized = true;
        panadapterScene->clear();
        panadapterScene->addItem(panadapterScene->spectrumPlot);
        panadapterScene->addItem(panadapterScene->waterfallItem);
        drawFrequencyLines();
        drawdBmLines();
        drawBandLimits();
        drawCursor(1, false);
        drawFilter(1, false);
        drawUpdatedNotchFilter(1);
//        drawUpdatedNotchFilter(2);
        QGraphicsView::setMouseTracking(true);
        splitViewBoundary = panadapterScene->height()/2;

//        QTimer::singleShot(1000,this,SLOT(redrawItems()));
    }

    QTimer::singleShot(0,this,SLOT(drawSpectrum()));
    QTimer::singleShot(0,this,SLOT(updateWaterfall()));
}

void Panadapter::setSquelch(bool state) {
    squelch=state;
    if (initialized)   // KD0OSS
        drawSquelch();
//    QGraphicsView::setMouseTracking(state);
}

void Panadapter::setSquelchVal(float val) {
    squelchVal=val;
    squelchY=(int) floor(((float) spectrumHigh - squelchVal)*(float)splitViewBoundary / (float) (spectrumHigh - spectrumLow));
    if (initialized)   // KD0OSS
        drawSquelch();
    //qDebug()<<"Panadapter::setSquelchVal"<<val<<"squelchY="<<squelchY;
}

void Panadapter::addNotchFilter(int index)   // KD0OSS
{
    QString command;

    notchFilterIndex = index;
    if (!subRx)
        notchFilterVFO[notchFilterIndex] = 1;
    else
        notchFilterVFO[notchFilterIndex] = 2;
    notchFilterFO[notchFilterIndex] = frequency+((filterLow+filterHigh)/2);
    for (int i=0;i<9;i++)
    {
        if (i == notchFilterIndex) continue;
        if (abs(notchFilterFO[notchFilterIndex] - notchFilterFO[i]) <= 100)
            return;
    }
    notchFilterBW[notchFilterIndex] = 400.0;
    notchFilterDepth[notchFilterIndex] = 1;
    notchFilterBand[notchFilterIndex] = band;
    notchFilterEnabled[notchFilterIndex] = true;
    drawNotchFilter(1, notchFilterIndex, false);

    double audio_freq = abs((notchFilterFO[notchFilterIndex] - frequency)); // Convert to audio frequency in Hz
    command.clear();
    QTextStream(&command) << "setnotchfilter " << 0 << " " << index << " " << notchFilterBW[notchFilterIndex] << " " << audio_freq;
    connection->sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;

    command.clear();
    QTextStream(&command) << "setnotchfilter " << 1 << " " << index << " " << notchFilterBW[notchFilterIndex] << " " << audio_freq;
    connection->sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;

    enableNotchFilter(notchFilterIndex, true);
}

void Panadapter::updateNotchFilter(void)
{
    updateNotchFilter(-1);
}

void Panadapter::updateNotchFilter(int index)   // KD0OSS
{
    QString command;
    double audio_freq;

    updateNfTimer->stop();
    if (index < 0) // Update all active notch filters
    {
        for (int i=0;i<9;i++)
        {
            if (notchFilterBand[i] == band)
            {
                if (notchFilterFO[i] < ((filterLow + frequency) - 200) || notchFilterFO[i] > ((filterHigh + frequency) + 200))
                {
                    enableNotchFilter(i, false);
                    continue;
                }
                else
                    enableNotchFilter(i, true);
                audio_freq = abs((notchFilterFO[i] - frequency)); // Convert to audio frequency in Hz
                command.clear();
                QTextStream(&command) << "setnotchfilter " << 0 << " " << i << " " << notchFilterBW[i] << " " << audio_freq;
                connection->sendCommand(command);
                qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;

                command.clear();
                QTextStream(&command) << "setnotchfilter " << 1 << " " << i << " " << notchFilterBW[i] << " " << audio_freq;
                connection->sendCommand(command);
                qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
            }
        }
    }
    else // Update selected notch filter
    {
        if (notchFilterFO[index] < ((filterLow + frequency) - 200) || notchFilterFO[index] > ((filterHigh + frequency) + 200))
        {
            enableNotchFilter(index, false);
            return;
        }
        else
            QTimer::singleShot(1000,this,SLOT(redrawItems()));

            enableNotchFilter(index, true);
        audio_freq = abs((notchFilterFO[index] - frequency)); // Convert to audio frequency in Hz
        command.clear();
        QTextStream(&command) << "setnotchfilter " << 0 << " " << index << " " << notchFilterBW[index] << " " << audio_freq;
        connection->sendCommand(command);
        qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;

        command.clear();
        QTextStream(&command) << "setnotchfilter " << 1 << " " << index << " " << notchFilterBW[index] << " " << audio_freq;
        connection->sendCommand(command);
        qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
    }
}

// Enable/Disable all active notch filters
void Panadapter::enableNotchFilter(bool enable)   // KD0OSS
{
    QString command;

    for (int index=0;index<9;index++)
    {
        if (notchFilterBand[index] != band && enable) continue;
        command.clear();
        QTextStream(&command) << "enablenotchfilter " << 0 << " " << index << " " << enable;
        connection->sendCommand(command);
        qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;

        command.clear();
        QTextStream(&command) << "enablenotchfilter " << 1 << " " << index << " " << enable;
        connection->sendCommand(command);
        qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
    }
    //emit enableNotchFilterSig(enable);
}

// Enable/Disable selected notch filter
void Panadapter::enableNotchFilter(int index, bool enable)   // KD0OSS
{
    QString command;

    if (notchFilterBand[index] != band && enable) return;
    command.clear();
    QTextStream(&command) << "enablenotchfilter " << 0 << " " << index << " " << enable;
    connection->sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;

    command.clear();
    QTextStream(&command) << "enablenotchfilter " << 1 << " " << index << " " << enable;
    connection->sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
    //emit enableNotchFilterSig(enable);
}

// Delete filter selected with mouse pointer
void Panadapter::deleteNotchFilter(void)   // KD0OSS
{
    drawNotchFilter(1, notchFilterSelected, true);
    enableNotchFilter(notchFilterSelected, false);
    notchFilterBand[notchFilterSelected] = "na";
    notchFilterEnabled[notchFilterSelected] = false;
    emit removeNotchFilter();
}

void Panadapter::deleteAllNotchFilters(void)   // KD0OSS
{
    for (int index=0;index<9;index++)
    {
        drawNotchFilter(1, index, true);
        enableNotchFilter(index, false);
        notchFilterEnabled[index] = false;
        notchFilterBand[index] = "na";
        emit removeNotchFilter();
    }
}

void Panadapter::updateWaterfall(void)
{
    panadapterScene->waterfallItem->updateWaterfall(wsamples, size, splitViewBoundary);
}

//*************************************************************************Waterfall**************************************************

waterfallObject::waterfallObject(int width, int height) {

    setZValue(10.0);
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

    itemWidth = width;
    itemHeight = height;
    ypos = itemHeight / 2;

    image = QImage(width, height * 2, QImage::Format_RGB32);

    int x, y;
#pragma omp parallel for schedule(static)
    for (x = 0; x < image.width(); x++) {
        for (y = 0; y < image.height(); y++) {
            image.setPixel(x, y, 0xFF000000);
        }
    }
    cy = image.height()/2 - 1;
}

void waterfallObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    // plot waterfall
    painter->drawImage(0,ypos,image,0,cy,image.width(),image.height()/2,Qt::AutoColor);
    if (cy <= 0) cy = image.height()/2 - 1;
    else cy--;          // "scroll"
}

QRectF waterfallObject::boundingRect() const
{
    return QRectF(QPointF(0.0, itemHeight), QPointF(image.width(), image.height()/2));
} /** end boundingRect **/

int waterfallObject::getHigh() {
    return waterfallHigh;
}

int waterfallObject::getLow() {
    return waterfallLow;
}

void waterfallObject::setHigh(int high) {
    waterfallHigh=high;
}

void waterfallObject::setLow(int low) {
    waterfallLow=low;
}

void waterfallObject::setAutomatic(bool state) {
    waterfallAutomatic=state;
}

bool waterfallObject::getAutomatic() {
    return waterfallAutomatic;
}

void waterfallObject::updateWaterfall(char* buffer, int length, int starty) {
    int i;
    int x,y;
    int average=0;

    if(samples!=NULL) {
        free(samples);
    }

    itemWidth = this->scene()->width();
    itemHeight = this->scene()->height() - starty;
    ypos = starty;

    samples = (float*) malloc(itemWidth * sizeof (float));

    // do not rotate spectrum display.  It is done by dspserver now
#pragma omp parallel for schedule(static)
    for(i=0;i<itemWidth;i++) {
        samples[i] = -(buffer[i] & 0xFF);
    }

    size = length;
 //   QTimer::singleShot(0,this,SLOT(updateWaterfall_2()));

    if(image.width()!=itemWidth || (image.height()/2) != itemHeight) {
        qDebug() << "Waterfall::updateWaterfall " << size << "(" << itemWidth << ")," << itemHeight;
        image = QImage(itemWidth, itemHeight*2, QImage::Format_RGB32);
        cy = image.height()/2 - 1;
        #pragma omp parallel for schedule(static)
        for (x = 0; x < itemWidth; x++) {
            for (y = 0; y < image.height(); y++) {
                image.setPixel(x, y, 0xFF000000);
            }
        }
    }

    // draw the new line
    #pragma omp parallel for schedule(static)
    for(x=0;x<size;x++){
        uint pixel = calculatePixel(samples[x]);
        image.setPixel(x,cy,pixel);
        image.setPixel(x,cy+(image.height()/2),pixel);
        #pragma omp critical
        average+=samples[x];
    }

    if (waterfallAutomatic) {
        waterfallLow=(average/size)-10;
        waterfallHigh=waterfallLow+60;
    }
}
/*
void waterfallObject::updateWaterfall_2(void){
    int x,y;

    if(image.width()!=itemWidth || (image.height()/2) != itemHeight) {
        qDebug() << "Waterfall::updateWaterfall " << size << "(" << itemWidth << ")," << itemHeight;
        image = QImage(itemWidth, itemHeight*2, QImage::Format_RGB32);
        cy = image.height()/2 - 1;
        #pragma omp parallel for schedule(static)
        for (x = 0; x < itemWidth; x++) {
            for (y = 0; y < image.height(); y++) {
                image.setPixel(x, y, 0xFF000000);
            }
        }
    }
    QTimer::singleShot(0,this,SLOT(updateWaterfall_4()));
}

void waterfallObject::updateWaterfall_4(void){
    int x;
    int average=0;

    // draw the new line
    #pragma omp parallel for schedule(static)
    for(x=0;x<size;x++){
        uint pixel = calculatePixel(samples[x]);
        image.setPixel(x,cy,pixel);
        image.setPixel(x,cy+(image.height()/2),pixel);
        #pragma omp critical
        average+=samples[x];
    }

    if (waterfallAutomatic) {
        waterfallLow=(average/size)-10;
        waterfallHigh=waterfallLow+60;
    }
}
*/
uint waterfallObject::calculatePixel(int sample) {
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
