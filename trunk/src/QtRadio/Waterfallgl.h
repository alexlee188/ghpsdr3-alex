
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

#ifndef waterfallgl_H
#define	waterfallgl_H

#include <QtGui>
#define MAX_CL_WIDTH 2048
#define MAX_CL_HEIGHT 1024

class Waterfallgl : public QWindow,  protected QOpenGLFunctions {
    Q_OBJECT
public:
    Waterfallgl();
    ~Waterfallgl();
    void initialize(int wid, int ht);
    void setLow(int low);
    void setHigh(int high);
    void setAutomatic(bool state);
    void setLO_offset(GLfloat offset);
    void render();

public slots:
    void updateWaterfall(char* buffer,int width, int starty);

protected:
    void  resizeGL(int, int);
    void  paintGL();

private:
    QOpenGLContext *m_context;
    GLuint loadShader(GLenum type, const char *source);
    QOpenGLShaderProgram *ShaderProgram;
    GLuint spectrumTexture_location, spectrumTex;
    GLuint cy_location, waterfallLow_location, waterfallHigh_location, offset_location, width_location; 
    GLuint aPosition_location, textureCoord_location;
    GLuint uMVPMatrix_location;
    int data_width;
    int data_height;
    int waterfallHigh;
    int waterfallLow;
    bool waterfallAutomatic;
    float average;
    int cy;
    GLfloat LO_offset;
};





#endif // waterfallgl_H
