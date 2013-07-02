   /*
 * File:   PANADAPTER.h
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

#ifndef PANADAPTER_H
#define	PANADAPTER_H

#include <QtCore>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsItemGroup>
#include <QtOpenGL/QGLWidget>
#else
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#endif

#include <QPainter>
#include <QAction>
#include <QMenu>
#include <QMouseEvent>

#include "Meter.h"
#include "Connection.h"

/****************** Added by KD0OSS **********************************************/

class PanadapterScene;

class waterfallObject : public QWidget, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    waterfallObject(int width, int height);
    void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF  boundingRect() const;
    int     itemType;

    void setLow(int low);
    void setHigh(int high);
    int getLow();
    int getHigh();

    void setAutomatic(bool state);
    bool getAutomatic();

    int cy;         // current row
    int ypos;
    QImage image;

public slots:
    void updateWaterfall(char*, int, int);

private:
    uint calculatePixel(int sample);

    int itemWidth;
    int itemHeight;

    float* samples;
    int waterfallHigh;
    int waterfallLow;
    bool waterfallAutomatic;
    int colorLowR;
    int colorLowG;
    int colorLowB;
    int colorMidR;
    int colorMidG;
    int colorMidB;
    int colorHighR;
    int colorHighG;
    int colorHighB;
    int size;
};


class spectrumObject : public QWidget, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    spectrumObject(int width, int height);
    QVector <QPoint> plot;
    void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF  boundingRect() const;
    int     itemType;

private:
    int plotWidth;
    int plotHeight;
};

class filterObject : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    filterObject(PanadapterScene *scene, QPoint location, float fwidth, float fheight, QColor color);
    void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF  boundingRect() const;

    QPoint  itemLocation;
    QColor  itemColor;
    float   itemWidth;
    float   width;
    float   height;
    int     itemType;
};


class textObject : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    textObject(PanadapterScene *scene, QString text, QPoint location, QColor color);
    void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF  boundingRect() const;

    QString itemText;
    QPoint  itemLocation;
    QColor  itemColor;
    float   width;
    float   height;
    int     itemType;
};


class notchFilterObject : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    notchFilterObject(PanadapterScene *scene, int index, QPoint location, float fwidth, float fheight, QColor color);
    void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF  boundingRect() const;

    QPoint  itemLocation;
    QColor  itemColor;
    float   itemWidth;
    float   width;
    float   height;
    int     itemIndex;
    int     itemType;
};

class lineObject : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    lineObject(PanadapterScene *scene, QPoint start, QPoint stop, QPen pen);
    void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF  boundingRect() const;

    QPoint  itemStart;
    QPoint  itemStop;
    QPen    itemPen;
    float   width;
    float   height;
    int     itemType;
};


class PanadapterScene : public QGraphicsScene
{
    Q_OBJECT

public:
    PanadapterScene(QObject *parent = 0);

    QMap<QString, QGraphicsItem*> sceneItems;


    spectrumObject *spectrumPlot;
    waterfallObject *waterfallItem;
    int  itemType;
};
/********************************KD0OSS*****************************************/

class Panadapter: public QGraphicsView
{
    Q_OBJECT
public:
    Panadapter();
    Panadapter(QWidget*& widget);
    virtual ~Panadapter();

    Connection *connection;  // KD0OSS
    PanadapterScene *panadapterScene;  // KD0OSS

    bool sampleZoom; // KD0OSS
    int splitViewBoundary; // KD0OSS

    void setObjectName(QString name);
    void setGeometry(QRect rect);
    void initialize();
    void setSampleRate(int r);
    void setFrequency(long long f);
    void setSubRxFrequency(long long f);
    void setFilter(int low,int high);
    void updateSpectrumFrame(char* header,char* buffer,int width);
    int samplerate();

    int getHigh();
    int getLow();
    void setHigh(int high);
    void setLow(int low);

    void setSubRxState(bool state);

    void setMode(QString m);
    void setBand(QString b);
    void setFilter(QString f);

    void setHost(QString h);
    void setReceiver(int r);

    void setBandLimits(long long min,long long max);

    void setSquelch(bool state);
    void setSquelchVal(float val);
    void setZoom(int value);


signals:
    void frequencyMoved(int steps,int step);
//    void frequencyChanged(long long frequency);
    void spectrumHighChanged(int high);
    void spectrumLowChanged(int low);
    void waterfallHighChanged(int high);
    void waterfallLowChanged(int low);
    void meterValue(int meter, int subrx_meter);
    void squelchValueChanged(int step);
    void statusMessage(QString);
    void removeNotchFilter(void);
    void variableFilter(int low, int high);

protected:
 //   void paintEvent(QPaintEvent*);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    /*
    void mouseDoubleClickEvent ( QMouseEvent * event );
    */

    void wheelEvent(QWheelEvent *event);
    void resizeEvent(QResizeEvent *event);

public slots:
    void setAvg(int value);
    void addNotchFilter(int index);    // KD0OSS
    void enableNotchFilter(bool enable);   // KD0OSS
    void enableNotchFilter(int index, bool enable);   // KD0OSS
    void updateWaterfall(void);

private slots:
    void drawCursor(int vfo, bool disable);  // KD0OSS
    void drawFilter(int vfo, bool diabale);  // KD0OSS
    void drawNotchFilter(int vfo, int index, bool disable);  // KD0OSS
    void drawdBmLines(void);  // KD0OSS
    void drawFrequencyLines(void);  // KD0OSS
    void drawBandLimits(void);  // KD0OSS
    void drawSquelch(void);  // KD0OSS
    void drawSpectrum(void);  // KD0OSS
    void drawUpdatedNotchFilter(int vfo);  // KD0OSS
    void updateNotchFilter(void);  // KD0OSS
    void updateNotchFilter(int);  // KD0OSS
    void deleteNotchFilter(void);  // KD0OSS
    void deleteAllNotchFilters(void);  // KD0OSS
    void redrawItems(void);

private:
    float* samples;
    char* wsamples; // KD0OSS
    int spectrumHigh;
    int spectrumLow;

    QString host;
    int receiver;
    QString band;
    QString mode;
    QString filter;

    int button;
    int startX;
    int lastX;
    int lastY; // KD0OSS
    int moved;
    bool adjustSplitViewBoundary; // KD0OSS

    long sampleRate;
    short meter;
    int maxMeter;
    int meterCount;
    short subrx_meter;
    int subrx_maxMeter;
    int subrx_meterCount;

    int filterLow;
    int filterHigh;
    bool filterSelected;
    int avg;
    int size;
    long long frequency;
    QString strFrequency;
    long long subRxFrequency;
    QString strSubRxFrequency;
    bool subRx;

    bool squelch;
    float squelchVal;
    int squelchY;
    bool showSquelchControl;
    bool settingSquelch;

    long long band_min;
    long long band_max;

    QVector <QPoint> plot;

    bool initialized; // KD0OSS
    QTimer *updateNfTimer;

    int   notchFilterIndex; // KD0OSS
    int   notchFilterVFO[9]; // KD0OSS
    float notchFilterBW[9]; // KD0OSS
    float notchFilterFO[9]; // KD0OSS
    int   notchFilterDepth[9]; // KD0OSS
    QString notchFilterBand[9]; // KD0OSS
    bool  notchFilterEnabled[9]; // KD0OSS
    int   notchFilterSelected; // KD0OSS

    QAction *notchFilterDeleteAction;  // KD0OSS

    short LO_offset;
    int zoom;
};


#endif	/* PANADAPTER_H */

