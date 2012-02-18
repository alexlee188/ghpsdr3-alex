
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
        "__kernel void waterfall(__global float *src, __global float *dest) {\n"
        "  int id = get_global_id(0);\n"
        "  dest[id] = sin(src[id]);\n"
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
    waterfall.setGlobalWorkSize(wid, ht);
    waterfall.setLocalWorkSize(waterfall.bestLocalWorkSizeImage2D());
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
}

Waterfallcl::~Waterfallcl(){

}

void Waterfallcl::setGeometry(QRect rect){
    data_width = rect.width();
    data_height = rect.height() * 2;

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

void Waterfallcl::initialize(int wid, int ht){

    data_width = wid;
    data_height = ht*2;     // for fast waterfall algorithm without scrolling

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


void Waterfallcl::updateWaterfall(char *header, char *buffer, int width){

}


