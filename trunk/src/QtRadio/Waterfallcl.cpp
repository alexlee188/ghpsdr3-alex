
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

#include <QDebug>
#include "Waterfallcl.h"

Waterfallcl::Waterfallcl(){
    makeCurrent();
}

Waterfallcl::~Waterfallcl(){

}

void Waterfallcl::initialize(int wid, int ht){

    data_width = wid;
    data_height = ht;
    cy = MAX_CL_HEIGHT - 1;

    QImage t;
    QImage b;
    b = QImage( MAX_CL_WIDTH, MAX_CL_HEIGHT, QImage::Format_ARGB32_Premultiplied );
    t = QGLWidget::convertToGLFormat( b );;
    glGenTextures(1, &spectrumTex);
    glBindTexture(GL_TEXTURE_2D, spectrumTex);
    glTexImage2D( GL_TEXTURE_2D, 0, 3, t.width(), t.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.bits() );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

#ifdef GL_CLAMP_TO_EDGE
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#endif

    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    ShaderProgram = NULL;
    LoadShader();
    spectrumTexture_location = glGetUniformLocation(ShaderProgram->programId(), "spectrumTexture");
    cy_location =  glGetUniformLocation(ShaderProgram->programId(), "cy");
    offset_location =  glGetUniformLocation(ShaderProgram->programId(), "offset");
    waterfallLow_location =  glGetUniformLocation(ShaderProgram->programId(), "waterfallLow");
    waterfallHigh_location =  glGetUniformLocation(ShaderProgram->programId(), "waterfallHigh");
    width_location =  glGetUniformLocation(ShaderProgram->programId(), "width");
    aPosition_location = glGetAttribLocation(ShaderProgram->programId(),"aPosition");
    textureCoord_location = glGetAttribLocation(ShaderProgram->programId(), "aTextureCoord");
    glUseProgram(ShaderProgram->programId());
    //Bind to tex unit 0
    glUniform1i(spectrumTexture_location, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);

}

void Waterfallcl::setHigh(int high) {
    float wfHigh = (float) (waterfallHigh) / 256.0f;
    glUniform1f(waterfallHigh_location, wfHigh);
    waterfallHigh=high;
}

void Waterfallcl::setLow(int low) {
    waterfallLow=low;
    float wfLow = (float) (waterfallLow) / 256.0f;
    glUniform1f(waterfallLow_location, wfLow);
}

void Waterfallcl::setAutomatic(bool state) {
    waterfallAutomatic=state;
}

void Waterfallcl::setLO_offset(GLfloat offset){
    LO_offset = offset;
    glUniform1f(offset_location, LO_offset);
}


void Waterfallcl::resizeGL( int width, int height )
{
    data_width = width;
    data_height = height;

    height = height?height:1;

    glViewport( 0, 0, (GLint)width, (GLint)height );

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glScalef(1.0, -1.0, 1.0);
    glTranslatef(0, -height, 0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void Waterfallcl::updateWaterfallgl(){
    updateGL();
}

void Waterfallcl::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, spectrumTex);
    float current_line = (float) cy /  MAX_CL_HEIGHT;
    glUniform1f(cy_location, current_line);
    GLfloat tex_width = (float) data_width / MAX_CL_WIDTH;

    /*  // starting to convert opengl to opengles
        // so that the opengles code can run in low power Single Board Computers

    const GLfloat mVertices[] =  {
        0.0f, 0.0f,  // Position 0
        0.0f, 0.0f,  // TexCoord 0
        data_width, 0.0f,  // Position 1
        tex_width, 0.0f, // TexCoord 1
        data_width, MAX_CL_HEIGHT, // Position 2
        tex_width, 1.0f, // TexCoord 2
        0.0f, MAX_CL_HEIGHT, // Position 3
        0.0f, 1.0f // TexCoord 3
    };

    glVertexAttribPointer(aPosition_location, 2, GL_FLOAT, false, sizeof(GLfloat)*4, mVertices);
    glEnableVertexAttribArray(aPosition_location);
    glVertexAttribPointer(textureCoord_location, 2, GL_FLOAT, false, sizeof(GLfloat)*4, &mVertices[2]);
    glEnableVertexAttribArray(textureCoord_location);
    */

    glBegin(GL_QUADS);
    // Front Face
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0, 0.0);
    glTexCoord2f(tex_width, 0.0f); glVertex2f(data_width, 0.0);
    glTexCoord2f(tex_width, 1.0f); glVertex2f(data_width, MAX_CL_HEIGHT);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0, MAX_CL_HEIGHT);
    glEnd();

}

void Waterfallcl::updateWaterfall(char *header, char *buffer, int width){
    data_width = (width < MAX_CL_WIDTH) ? width : MAX_CL_WIDTH;
    if (cy-- <= 0) cy = MAX_CL_HEIGHT - 1;

    unsigned char data[MAX_CL_WIDTH][4];
    for (int i = 0; i < data_width; i++){
        data[i][0] = buffer[i];
    }

    glUniform1f(width_location, (float)width/MAX_CL_WIDTH);
    // Update Texture
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, cy, MAX_CL_WIDTH, 1,
                    GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)data);


}
static char const vertexShader[] =
        "attribute vec4 aPosition;\n"
        "attribute vec2 aTextureCoord;\n"
        "void main()\n"
        "{\n"
        "gl_TexCoord[0] = gl_MultiTexCoord0;\n"
        "gl_Position = ftransform();\n"
        "}\n";

static char const fragmentShader[] =
        "uniform sampler2D spectrumTexture;\n"
        "uniform float cy;\n"
        "uniform float offset;\n"
        "uniform float width;\n"
        "uniform float waterfallLow, waterfallHigh;\n"
        "void main()\n"
        "{\n"
            "float y_coord = gl_TexCoord[0].t + cy;\n"
            "if (y_coord > 1.0) y_coord -= 1.0;\n"
            "float x_coord = gl_TexCoord[0].s - offset;\n"
            "if (x_coord < 0.0) x_coord += width;\n"
            "if (x_coord > width) x_coord -= width;\n"
            "vec4 value = texture2D(spectrumTexture, vec2(x_coord, y_coord));\n"
            "float sample = 0.0 - value.r;\n"
            "float percent = (sample - waterfallLow)/(waterfallHigh-waterfallLow);\n"
            "if (percent < 0.0) percent = 0.0;\n"
            "if (percent > 1.0) percent = 1.0;\n"
            "vec4 texel;\n"
            "if (percent < (2.0/9.0)) {texel = vec4(0.0, 0.0, percent/(2.0/9.0), 1.0);}\n"
                "else if (percent < (3.0/9.0)) {texel = vec4(0.0, (percent - (2.0/9.0))/(1.0/9.0), 1.0, 1.0);}\n"
                "else if (percent < (4.0/9.0)) {\n"
                "float local_percent = (percent - (3.0/9.0))/(1.0/9.0);\n"
                "texel = vec4(0.0, (1.0 - local_percent), 1.0, 1.0);\n"
                 "}\n"
            "else if (percent < (5.0/9.0)) {texel = vec4((percent - (4.0/9.0))/(1.0/9.0), 1.0, 0.0, 1.0);}\n"
            "else if (percent < (7.0/9.0)) {\n"
                "float local_percent = (percent - (5.0/9.0))/(2.0/9.0);\n"
                "texel = vec4(1.0, (1.0 - local_percent), 0.0, 1.0);\n"
                "}\n"
            "else if (percent < (8.0/9.0)){ texel = vec4(1.0, 0.0, (percent - (7.0/9.0))/(1.0/9.0), 1.0);}\n"
            "else {\n"
                "float local_percent = (percent - 8.0/9.0)/(1.0/9.0);\n"
                "texel = vec4(0.75+0.25*(1.0-local_percent), 0.5*local_percent, 1.0, 1.0);\n"
                "}\n"
            "gl_FragColor = texel;\n"
        "}\n";

void Waterfallcl::LoadShader(){
    if(ShaderProgram)
        {
        ShaderProgram->release();
        ShaderProgram->removeAllShaders();
        }
    else ShaderProgram = new QGLShaderProgram;


    if (!ShaderProgram->addShaderFromSourceCode(QGLShader::Vertex, vertexShader)){
        qWarning() << ShaderProgram->log();
        qFatal("Fatal");
    };
    if (!ShaderProgram->addShaderFromSourceCode(QGLShader::Fragment, fragmentShader)){
        qWarning() << ShaderProgram->log();
        qFatal("Fatal");
    };

    if(!ShaderProgram->link())
        {
        qWarning() << "Shader Program Linker Error" << ShaderProgram->log();
        qFatal("Fatal");
        }
    else ShaderProgram->bind();
}

