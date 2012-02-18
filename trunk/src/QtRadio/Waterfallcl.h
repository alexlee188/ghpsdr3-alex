
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

#ifndef WATERFALLCL_H
#define	WATERFALLCL_H

#include <QtCore>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <GL/glu.h>
#include <QtOpenGL/QGLWidget>
#include <QtOpenGL/qglfunctions.h>
#include <qcl/qclbuffer.h>
#include <qcl/qclcommandqueue.h>
#include <qcl/qclcontext.h>
#include <qcl/qcldevice.h>
#include <qcl/qclevent.h>
#include <qcl/qclimage.h>
#include <qcl/qclimageformat.h>
#include <qcl/qclprogram.h>
#include <qcl/qclvector.h>
#include <qcl/qclcontextgl.h>
#include "qclmemoryobject.h"


class Waterfallcl : public QGLWidget {
    Q_OBJECT

public:
    Waterfallcl();
    ~Waterfallcl();
    void setGeometry(QRect rect);
    void initialize(int wid, int ht);
    void resizeGL( int width, int height );
    void paintGL();
public slots:
    void updateWaterfall(char* header,char* buffer,int width);
private:
    QCLVector<char> spectrum_data;
    QCLImage2D waterfall_buffer;
    GLuint textureId;
    int data_width;
    int data_height;
    int cy;
    GLfloat rtri, rquad;
};





#endif // WATERFALLCL_H
