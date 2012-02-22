
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

        "__kernel void waterfall(__global __read_only char *src, const int cy, const int height,  __write_only image2d_t image) {\n"
        "  int id = get_global_id(0);\n"
        "  int2 pos;\n"
        "  pos = (int2)(id, cy);\n"
                "  write_imagef(image, pos, (float4)(0.5, 0.25, 1.0, (float)src[id]/256.0));\n"
        "  pos = (int2)(id, cy + height);\n"
                "  write_imagef(image, pos, (float4)(0.5, 0.25, 1.0, (float)src[id]/256.0));\n"
        "}\n");

    if (glContext) {
        waterfall.setGlobalWorkSize(wid);
        return;
    }

    glContext = new QCLContextGL();
    if (!glContext->create())
        return;

    program = glContext->buildProgramFromSourceCode(source);
    qDebug() << program.sourceCode();
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

    QImage t;
    QImage b;

    b = QImage( 1024, 512*2, QImage::Format_ARGB32_Premultiplied);
    b.fill( Qt::green);

    t = QGLWidget::convertToGLFormat( b );

    makeCurrent();
    ImageCLContext *ctx = image_context();

    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    // Create the textures in the GL context.
    glGenTextures(2, textureId);

    glBindTexture(GL_TEXTURE_2D, textureId[0]);
#ifdef GL_CLAMP_TO_EDGE
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#endif
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 512*2, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, t.bits());


    loadGLTextures(textureId[1]);

    glBindTexture(GL_TEXTURE_2D, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // If the context supports object sharing, then this is really easy.
    if (ctx->glContext->supportsObjectSharing()) {
    waterfall_buffer = ctx->glContext->createTexture2D
            (GL_TEXTURE_2D, textureId[0], 0, QCLMemoryObject::WriteOnly);
    if (waterfall_buffer == 0) qFatal("Unabel to create waterfall_buffer");
    }
    else {
        qDebug() << "System does not support CL/GL object sharing";
        waterfall_buffer = ctx->glContext->createImage2DDevice
            (QCLImageFormat(QCLImageFormat::Order_RGBA,
                        QCLImageFormat::Type_Normalized_UInt8),
                        QSize(1024, 512*2), QCLMemoryObject::WriteOnly);

    }

}

void Waterfallcl::loadGLTextures(GLuint textureId)
{
    QImage t;
    QImage b;

    if ( !b.load( "./crate.bmp" ) )
    {
        b = QImage( 16, 16, QImage::Format_ARGB32_Premultiplied);
        b.fill( Qt::green);
    }

    t = QGLWidget::convertToGLFormat( b );

    glBindTexture( GL_TEXTURE_2D, textureId );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D( GL_TEXTURE_2D, 0, 3, t.width(), t.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.bits() );
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
    cy = data_height - 1;
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

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureId[1]);

    glBegin(GL_QUADS);
    // Front Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
    // Back Face
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
    // Top Face
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
    // Bottom Face
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
    // Right face
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
    // Left Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    rtri += 0.2f;
    rquad -= 0.5f;
    if (cy-- <= 0) cy = data_height - 1;
}

void Waterfallcl::updateWaterfall(char *header, char *buffer, int width){

    for (int i = 0; i < width; i++) spectrum_data[i] = buffer[i];

    ImageCLContext *ctx = image_context();
    QCLKernel waterfall = ctx->waterfall;

    //ctx->glContext->acquire(waterfall_buffer).waitForFinished();
    waterfall(spectrum_data, cy, data_height, waterfall_buffer);
    //ctx->glContext->release(waterfall_buffer).waitForFinished();

}


