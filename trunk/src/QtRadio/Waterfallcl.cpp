
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

#include "Waterfallcl.h"

class ImageCLContext
{
public:
    ImageCLContext() : glContext(0) {}
    ~ImageCLContext();

    void init(int wid, int ht);

    QCLContextGL *glContext;
    QCLProgram program;
    QCLKernel waterfall;
};

void ImageCLContext::init(int wid, int ht)
{
    QByteArray source = QByteArray(
        "__kernel void waterfall(__global char *src, int cy,  __write_only image2d_t image) {\n"
        "  int id = get_global_id(0);\n"
        "  int2 pos = (int2)(id, cy);\n"
        "  image_writef(image, pos, (float4)(0,0,0, src[id]));\n"
        "}\n");

    if (glContext) {
        waterfall.setGlobalWorkSize(wid, ht);
        return;
    }

    glContext = new QCLContextGL();
    if (!glContext->create())
        return;

    program = glContext->createProgramFromSourceCode(source);
    waterfall = program.createKernel("waterfall");
    waterfall.setGlobalWorkSize(wid);
    //waterfall.setLocalWorkSize(waterfall.bestLocalWorkSizeImage2D());

}

ImageCLContext::~ImageCLContext()
{
    delete glContext;
}

Q_GLOBAL_STATIC(ImageCLContext, image_context)


Waterfallcl::Waterfallcl(){
    makeCurrent();
    ImageCLContext *ctx = image_context();
    ctx->init(100,100);
    spectrum_data = ctx->glContext->createVector<char>(1024);
}

Waterfallcl::~Waterfallcl(){

}


void Waterfallcl::initialize(int wid, int ht){

    data_width = wid;
    data_height = ht;

    rtri = 0.0f;
    rquad = 0.0f;

    makeCurrent();
    ImageCLContext *ctx = image_context();

    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    // Create the texture in the GL context.
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
#ifdef GL_CLAMP_TO_EDGE
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#endif
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data_width, data_height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // If the context supports object sharing, then this is really easy.
    if (ctx->glContext->supportsObjectSharing()) {
    waterfall_buffer = ctx->glContext->createTexture2D
            (GL_TEXTURE_2D, textureId, 0, QCLMemoryObject::ReadWrite);
    if (waterfall_buffer == 0) qFatal("Unabel to create waterfall_buffer");
    }
    else {
        qFatal("System does not support CL/GL object sharing");
    }
}

void Waterfallcl::resizeGL( int width, int height )
{
height = height?height:1;

    glViewport( 0, 0, (GLint)width, (GLint)height );

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f,(GLfloat)width/(GLfloat)height,0.5f,100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Waterfallcl::setGeometry(QRect rect){
    data_width = rect.width();
    data_height = rect.height();

    // change the width and height of textureId
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glBindTexture(GL_TEXTURE_2D, textureId);
#ifdef GL_CLAMP_TO_EDGE
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#endif
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data_width, data_height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Waterfallcl::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(-0.7f,0.0f,-6.0f);
    glRotatef(rtri,0.0f,1.0f,0.0f);

    glBegin(GL_TRIANGLES);
    glColor3f(1.0f,0.0f,0.0f);
    glVertex3f( 0.0f, 1.0f, 0.0f);
    glColor3f(0.0f,1.0f,0.0f);
    glVertex3f(-1.0f,-1.0f, 1.0f);
    glColor3f(0.0f,0.0f,1.0f);
    glVertex3f( 1.0f,-1.0f, 1.0f);
    glColor3f(1.0f,0.0f,0.0f);
    glVertex3f( 0.0f, 1.0f, 0.0f);
    glColor3f(0.0f,0.0f,1.0f);
    glVertex3f( 1.0f,-1.0f, 1.0f);
    glColor3f(0.0f,1.0f,0.0f);
    glVertex3f( 1.0f,-1.0f, -1.0f);
    glColor3f(1.0f,0.0f,0.0f);
    glVertex3f( 0.0f, 1.0f, 0.0f);
    glColor3f(0.0f,1.0f,0.0f);
    glVertex3f( 1.0f,-1.0f, -1.0f);
    glColor3f(0.0f,0.0f,1.0f);
    glVertex3f(-1.0f,-1.0f, -1.0f);
    glColor3f(1.0f,0.0f,0.0f);
    glVertex3f( 0.0f, 1.0f, 0.0f);
    glColor3f(0.0f,0.0f,1.0f);
    glVertex3f(-1.0f,-1.0f,-1.0f);
    glColor3f(0.0f,1.0f,0.0f);
    glVertex3f(-1.0f,-1.0f, 1.0f);
    glEnd();

    glLoadIdentity();
    glTranslatef(0.5f,0.0f,-5.5f);
    glScalef(0.8f, 0.8f, 0.8f);
    glRotatef(rquad,1.0f,0.0f,0.0f);

    glColor3f(0.5f,0.5f,1.0f);
    glBegin(GL_QUADS);
    glColor3f(0.0f,1.0f,0.0f);
    glVertex3f( 1.0f, 1.0f,-1.0f);
    glVertex3f(-1.0f, 1.0f,-1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f( 1.0f, 1.0f, 1.0f);
    glColor3f(1.0f,0.5f,0.0f);
    glVertex3f( 1.0f,-1.0f, 1.0f);
    glVertex3f(-1.0f,-1.0f, 1.0f);
    glVertex3f(-1.0f,-1.0f,-1.0f);
    glVertex3f( 1.0f,-1.0f,-1.0f);
    glColor3f(1.0f,0.0f,0.0f);
    glVertex3f( 1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f,-1.0f, 1.0f);
    glVertex3f( 1.0f,-1.0f, 1.0f);
    glColor3f(1.0f,1.0f,0.0f);
    glVertex3f( 1.0f,-1.0f,-1.0f);
    glVertex3f(-1.0f,-1.0f,-1.0f);
    glVertex3f(-1.0f, 1.0f,-1.0f);
    glVertex3f( 1.0f, 1.0f,-1.0f);
    glColor3f(0.0f,0.0f,1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f,-1.0f);
    glVertex3f(-1.0f,-1.0f,-1.0f);
    glVertex3f(-1.0f,-1.0f, 1.0f);
    glColor3f(1.0f,0.0f,1.0f);
    glVertex3f( 1.0f, 1.0f,-1.0f);
    glVertex3f( 1.0f, 1.0f, 1.0f);
    glVertex3f( 1.0f,-1.0f, 1.0f);
    glVertex3f( 1.0f,-1.0f,-1.0f);
    glEnd();

    rtri += 0.2f;
    rquad -= 0.5f;
}

void Waterfallcl::updateWaterfall(char *header, char *buffer, int width){
    for (int i = 0; i < width; i++) spectrum_data[i]= buffer[i];

    ImageCLContext *ctx = image_context();
    ctx->glContext->acquire(waterfall_buffer).waitForFinished();
    ctx->waterfall(spectrum_data, 0, waterfall_buffer);
    ctx->glContext->release(waterfall_buffer).waitForFinished();

}


