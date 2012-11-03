
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

#define GL_GLEXT_PROTOTYPES
#include <QtCore>
#include <QPainter>
#include <QImage>
#include <QMouseEvent>
#include <QDebug>
#include <GL/gl.h>
#include <QtOpenGL>
#include <QtOpenGL/QGLBuffer>
#include <QtOpenGL/QGLWidget>
#include <QtOpenGL/qglfunctions.h>
#include <QGLShader>

#define MAX_CL_WIDTH 2048
#define MAX_CL_HEIGHT 1024

class Waterfallcl : public QGLWidget {
    Q_OBJECT

public:
    Waterfallcl();
    ~Waterfallcl();
    void initialize(int wid, int ht);
    void setLow(int low);
    void setHigh(int high);
    void setAutomatic(bool state);
    void setLO_offset(GLfloat offset);

public slots:
    void updateWaterfall(char* header,char* buffer,int width);
    void updateWaterfallgl(void);
protected:
    void resizeGL( int width, int height );
    void paintGL();
private:
    void LoadShader(void);
    QGLShaderProgram *ShaderProgram;
    GLuint spectrumTexture_location, spectrumTex;
    GLuint cy_location, waterfallLow_location, waterfallHigh_location, offset_location, width_location; 
    GLuint aPosition_location, textureCoord_location;
    GLuint uMVPMatrix_location;
    int data_width;
    int data_height;
    int waterfallHigh;
    int waterfallLow;
    bool waterfallAutomatic;
    int cy;
    GLfloat LO_offset;
};





#endif // WATERFALLCL_H
