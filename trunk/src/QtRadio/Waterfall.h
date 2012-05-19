/*
 * File:   Waterfall.h
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

#ifndef WATERFALL_H
#define	WATERFALL_H

#include <QtCore>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QFrame>
#else
#include <QFrame>
#endif

#include <QPainter>
#include <QMouseEvent>

#include "Waterfallcl.h"

class Waterfall: public QFrame {
    Q_OBJECT
public:
    Waterfall();
    Waterfall(QWidget*& widget);
    virtual ~Waterfall();
    void setObjectName(QString name);
    void setGeometry(QRect rect);
    void initialize();
    void updateWaterfall(char* header,char* buffer,int width);

    void setLow(int low);
    void setHigh(int high);
    int getLow();
    int getHigh();

    void setAutomatic(bool state);
    bool getAutomatic();

    void setSampleRate(int r);
    void setFrequency(long long f);
    void setSubRxFrequency(long long f);
    void setFilter(int low,int high);
    void setSubRxState(bool state);
    void setMode(QString m);

signals:
    void frequencyMoved(int steps,int step);

private slots:
    void updateWaterfall_2(void);
    void updateWaterfall_3(void);
    void updateWaterfall_4(void);
protected:
    void paintEvent(QPaintEvent*);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void wheelEvent(QWheelEvent *event);

private:
    uint calculatePixel(int sample);

    float* samples;
    int waterfallHigh;
    int waterfallLow;
    bool waterfallAutomatic;
    int cy;         // current row
    int button;
    int startX;
    int lastX;
    int moved;

    int colorLowR;
    int colorLowG;
    int colorLowB;
    int colorMidR;
    int colorMidG;
    int colorMidB;
    int colorHighR;
    int colorHighG;
    int colorHighB;
    QImage image;

    short LO_offset;
    float average;

    int sampleRate;
    int filterLow;
    int filterHigh;
    long long frequency;
    long long subRxFrequency;
    bool subRx;
    int size;
    QString mode;

    Waterfallcl *waterfallcl;
};


#endif	/* WATERFALL_H */


