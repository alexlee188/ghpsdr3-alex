
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

#define MAX_CL_WIDTH 2048
#define MAX_CL_HEIGHT 256

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

        "__kernel void waterfall(__global __read_only char *src, const int cy, const int width,\n"
                "const int height, const int offset, const int waterfallLow,\n"
                "const int waterfallHigh, __write_only image2d_t image) {\n"
        "  int id = get_global_id(0);\n"
        "  int j = id - offset;\n"
        "  if (j < 0) j += width;\n"
        "  if (j >= width) j %= width;\n"
        "  int2 pos;\n"
        "  pos = (int2)(id, cy);\n"
        "  uint unsigned_sample = src[j] & 0xff;"
        "  float sample = 0.0f - (float) unsigned_sample;\n"
        "  float percent = (sample - (float)waterfallLow)/ (float)(waterfallHigh - waterfallLow);\n"
        "  percent = (percent > 0.0f) ? percent : 0.0f;"
        "  if (percent < (2.0f/9.0f)){\n"
        "    uint value = 255.0f * percent / (2.0f/9.0f);\n"
        "    write_imageui(image, pos, (uint4)(0, 0, value, 255));\n"
        "    pos = (int2)(id, cy + height);\n"
        "    write_imageui(image, pos, (uint4)(0, 0, value, 255));\n"
        "    }\n"
        "  else if (percent < (3.0f/9.0f)){\n"
        "    uint value = 255.0f * (percent - (2.0f/9.0f)) / (1.0f/9.0f);\n"
        "    write_imageui(image, pos, (uint4)(0, value, 255, 255));\n"
        "    pos = (int2)(id, cy + height);\n"
        "    write_imageui(image, pos, (uint4)(0, value, 255, 255));\n"
        "    }\n"
        "  else if (percent < (4.0f/9.0f)){\n"
        "    uint value = 255* (1.0f - ((percent - (3.0f/9.0f)) / (1.0f/9.0f)));\n"
        "    write_imageui(image, pos, (uint4)(0, value, 255, 255));\n"
        "    pos = (int2)(id, cy + height);\n"
        "    write_imageui(image, pos, (uint4)(0, value, 255, 255));\n"
        "    }\n"
        "  else if (percent < (5.0f/9.0f)){\n"
        "    uint value = 255*((percent - (4.0f/9.0f)) / (1.0f/9.0f));\n"
        "    write_imageui(image, pos, (uint4)(value, 255, 0, 255));\n"
        "    pos = (int2)(id, cy + height);\n"
        "    write_imageui(image, pos, (uint4)(value, 255, 0, 255));\n"
        "    }\n"
        "  else if (percent < (7.0f/9.0f)){\n"
        "    uint value = 255*(1.0f - ((percent - (5.0f/9.0f)) / (2.0f/9.0f)));\n"
        "    write_imageui(image, pos, (uint4)(255, value, 0, 255));\n"
        "    pos = (int2)(id, cy + height);\n"
        "    write_imageui(image, pos, (uint4)(255, value, 0, 255));\n"
        "    }\n"
        "  else if (percent < (8.0f/9.0f)){\n"
        "    uint value = 255*((percent - (7.0f/9.0f)) / (1.0f/9.0f));\n"
        "    write_imageui(image, pos, (uint4)(255, 0, value, 255));\n"
        "    pos = (int2)(id, cy + height);\n"
        "    write_imageui(image, pos, (uint4)(255, 0, value, 255));\n"
        "    }\n"
        "  else {\n"
        "    float local_percent = (percent - (8.0f/9.0f)) / (1.0f/9.0f);\n"
        "    int R = (0.75f + 0.25f * (1.0f - local_percent))*255.0f;\n"
        "    int G = 0.5f * local_percent * 255.0f;\n"
        "    write_imageui(image, pos, (uint4)(R, G, 255, 255));\n"
        "    pos = (int2)(id, cy + height);\n"
        "    write_imageui(image, pos, (uint4)(R, G, 255, 255));\n"
        "    }\n"
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

}

ImageCLContext::~ImageCLContext()
{
    delete glContext;
}

Q_GLOBAL_STATIC(ImageCLContext, image_context)


Waterfallcl::Waterfallcl(){
    makeCurrent();
    ImageCLContext *ctx = image_context();
    ctx->init(MAX_CL_WIDTH,256);
}

Waterfallcl::~Waterfallcl(){

}

void Waterfallcl::createTBO(GLuint* tbo, GLuint* tex) {
    // create buffer object
    glGenBuffers(1, tbo);
    glBindBuffer(GL_TEXTURE_BUFFER_EXT, *tbo);

    // initialize buffer object
    unsigned int size = MAX_CL_WIDTH * MAX_CL_HEIGHT;
    glBufferData(GL_TEXTURE_BUFFER_EXT, size, 0, GL_DYNAMIC_DRAW);

    //tex
    glGenTextures(1, tex);
    glBindTexture(GL_TEXTURE_BUFFER_EXT, *tex);
    glTexBufferEXT(GL_TEXTURE_BUFFER_EXT, GL_ALPHA8, *tbo);
    glBindBuffer(GL_TEXTURE_BUFFER_EXT, 0);

}

void Waterfallcl::deleteTBO(GLuint* tbo) {
    glBindBuffer(1, *tbo);
    glDeleteBuffers(1, tbo);
    *tbo = 0;
}

void Waterfallcl::setShaderUniforms() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER_EXT, spectrumTex);
}


void Waterfallcl::initialize(int wid, int ht){

    data_width = wid;
    data_height = ht;
    cy = data_height - 1;
    rquad = 0.0f;
    zoom = 4.5f * wid / 1024.0f;
    pan = -0.55f;

    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);

#ifdef GL_CLAMP_TO_EDGE
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#endif
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    ShaderProgram = NULL;
    VertexShader = NULL;
    FragmentShader = NULL;
    LoadShader("./Basic.vsh", "./Basic.fsh");
    createTBO(&tbo, &spectrumTex);
    setShaderUniforms();

}

void Waterfallcl::setHigh(int high) {
    waterfallHigh=high;
}

void Waterfallcl::setLow(int low) {
    waterfallLow=low;
}

void Waterfallcl::setAutomatic(bool state) {
    waterfallAutomatic=state;
}

void Waterfallcl::setLO_offset(GLfloat offset){
    LO_offset = offset;
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
    cy = MAX_CL_HEIGHT - 1;
}

void Waterfallcl::mouseMoveEvent(QMouseEvent* event){
    int movey=event->pos().y()-lastY;
    lastY=event->pos().y();
    zoom += (float)movey / 300.0f;
    if (zoom < 0.5f) zoom = 0.5f;
    else if (zoom > 10.0f) zoom = 10.0f;

    int movex=event->pos().x()-lastX;
    lastX=event->pos().x();
    pan += (float)movex / 100.0f;
}

void Waterfallcl::mousePressEvent(QMouseEvent *event){
    lastX = event->pos().x();
    lastY = event->pos().y();
}

void Waterfallcl::updateWaterfallgl(){
    updateGL();
}

void Waterfallcl::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    glTranslatef(pan,0.0f,-6.0f);
    glScalef(zoom, 2.0f, 2.0f);
    glRotatef(rquad,1.0f,0.0f,0.0f);

    GLfloat tex_width = (float) data_width / MAX_CL_WIDTH;

    glBegin(GL_QUADS);
    // Front Face
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
    glTexCoord2f(tex_width, 1.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
    glTexCoord2f(tex_width, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
    // Back Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
    glTexCoord2f(tex_width, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
    glTexCoord2f(tex_width, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
    // Top Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
    glTexCoord2f(tex_width, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
    glTexCoord2f(tex_width, 0.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
    // Bottom Face
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(tex_width, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
    glTexCoord2f(tex_width, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
    // Right face
    glTexCoord2f(tex_width, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
    glTexCoord2f(tex_width, 0.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
    // Left Face
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(tex_width, 1.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
    glTexCoord2f(tex_width, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
    glEnd();

    //rquad -= 0.2f;

}

void Waterfallcl::updateWaterfall(char *header, char *buffer, int width){
    data_width = (width < MAX_CL_WIDTH) ? width : MAX_CL_WIDTH;
    if (cy-- <= 0) cy = MAX_CL_HEIGHT - 1;

}

void Waterfallcl::LoadShader(QString vshader, QString fshader){
    if(ShaderProgram)
        {
        ShaderProgram->release();
        ShaderProgram->removeAllShaders();
        }
    else ShaderProgram = new QGLShaderProgram;

    if(VertexShader)
        {
        delete VertexShader;
        VertexShader = NULL;
        }

    if(FragmentShader)
        {
        delete FragmentShader;
        FragmentShader = NULL;
        }

    // load and compile vertex shader
    QFileInfo vsh(vshader);
    if(vsh.exists())
        {
        VertexShader = new QGLShader(QGLShader::Vertex);
        if(VertexShader->compileSourceFile(vshader))
            ShaderProgram->addShader(VertexShader);
        else qWarning() << "Vertex Shader Error" << VertexShader->log();
        }
    else qWarning() << "Vertex Shader source file " << vshader << " not found.";

    // load and compile fragment shader
    QFileInfo fsh(fshader);
    if(fsh.exists())
        {
        FragmentShader = new QGLShader(QGLShader::Fragment);
        if(FragmentShader->compileSourceFile(fshader))
            ShaderProgram->addShader(FragmentShader);
        else qWarning() << "Fragment Shader Error" << FragmentShader->log();
        }
    else qWarning() << "Fragment Shader source file " << fshader << " not found.";

    if(!ShaderProgram->link())
        {
        qWarning() << "Shader Program Linker Error" << ShaderProgram->log();
        }
    else ShaderProgram->bind();
}

