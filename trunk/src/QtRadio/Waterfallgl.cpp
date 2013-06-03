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

#include "Waterfallgl.h"

Waterfallgl::Waterfallgl(QScreen* screen)
    : QWindow(screen),
      ShaderProgram(0),
      m_context(0)
{
    average = -100;
    setSurfaceType(QWindow::OpenGLSurface);
    // Specify the format and create platform-specific surface
    QSurfaceFormat format;
    format.setDepthBufferSize( 24 );
    format.setMajorVersion( 2 );
    format.setMinorVersion( 1 );
    format.setSamples( 4 );
    format.setProfile( QSurfaceFormat::CoreProfile );
    setFormat(format);
    create();

    // Create an OpenGL context
    m_context = new QOpenGLContext;
    m_context->setFormat( format );
    m_context->create();

    // Make the context current on this window
    m_context->makeCurrent( this );

    // Obtain a functions object and resolve all entry points
    m_funcs = m_context->versionFunctions<QOpenGLFunctions_2_1>();
    if ( !m_funcs ) {
        qWarning( "Could not obtain OpenGL Functions object" );
        exit( 1 );
    }
    m_funcs->initializeOpenGLFunctions();
}

bool Waterfallgl::event(QEvent *event){
    switch (event->type()) {
    case QEvent::UpdateRequest:
        paintGL();
        return true;
    default:
        return QWindow::event(event);
    }
}

void Waterfallgl::exposeEvent(QExposeEvent *event){
    Q_UNUSED(event);

    if (isExposed()){
        paintGL();
    }
}

void Waterfallgl::resizeEvent(QResizeEvent *event){
    Q_UNUSED(event);

    m_context->makeCurrent(this);
    ShaderProgram->bind();

    data_width = width();
    data_height = height();
    data_height = data_height? data_height:1;
    m_funcs->glViewport( 0, 0, (GLint) data_width, (GLint) data_height );

    //Bind to tex unit 0
    m_funcs->glBindTexture(GL_TEXTURE_2D, spectrumTex);
    ShaderProgram->setUniformValue(spectrumTexture_location, 0);
    m_funcs->glActiveTexture(GL_TEXTURE0);

    GLfloat tex_width = (float) data_width / MAX_CL_WIDTH;
    ShaderProgram->setUniformValue(width_location, tex_width);

    // Ortho2D projection
    QMatrix4x4 MVPMatrix;
    MVPMatrix.ortho(0.0, data_width, 0.0, data_height, -1.0, 1.0);
    MVPMatrix.translate(0.0, data_height);
    MVPMatrix.scale(1.0, -1,0);
    ShaderProgram->setUniformValue(uMVPMatrix_location, MVPMatrix);

    const GLfloat mVertices[] =  {
        0.0f, 0.0f,  // Position 0
        (float)data_width, 0.0f,  // Position 1
        (float)data_width, MAX_CL_HEIGHT, // Position 2
        0.0f, MAX_CL_HEIGHT, // Position 3
    };

    const GLfloat mTextures[] = {
        0.0f, 0.0f,  // TexCoord 0
        tex_width, 0.0f, // TexCoord 1
        tex_width, 1.0f, // TexCoord 2
        0.0f, 1.0f // TexCoord 3
    };

    m_vao = new QOpenGLVertexArrayObject( this );
    m_vao->create();
    m_vao->bind();

    m_vbo_position.create();
    m_vbo_position.setUsagePattern(QOpenGLBuffer::StreamDraw);
    m_vbo_position.bind();
    m_vbo_position.allocate(mVertices, 4 * 2 * sizeof(float));
    ShaderProgram->enableAttributeArray(aPosition_location);
    ShaderProgram->setAttributeBuffer(aPosition_location, GL_FLOAT, 0, 2);


    m_vbo_texture.create();
    m_vbo_texture.setUsagePattern(QOpenGLBuffer::StreamDraw);
    m_vbo_texture.bind();
    m_vbo_texture.allocate(mTextures, 4 * 2 * sizeof(float));
    ShaderProgram->enableAttributeArray(textureCoord_location);
    ShaderProgram->setAttributeBuffer(textureCoord_location, GL_FLOAT, 0, 2);



}

Waterfallgl::~Waterfallgl(){
}

static char const vertexShader[] =
        "attribute highp vec4 aPosition;\n"
        "attribute highp vec2 aTextureCoord;\n"
        "uniform highp mat4 uMVPMatrix;\n"
        "varying highp vec2 vTextureCoord;\n"
        "void main()\n"
        "{\n"
        "vTextureCoord = aTextureCoord;\n"
        "gl_Position = uMVPMatrix * aPosition;\n"
        "}\n";

static char const fragmentShader[] =
        "uniform highp sampler2D spectrumTexture;\n"
        "uniform highp float cy;\n"
        "uniform highp float offset;\n"
        "uniform highp float width;\n"
        "uniform highp float waterfallLow, waterfallHigh;\n"
        "varying highp vec2 vTextureCoord;\n"
        "void main()\n"
        "{\n"
            "float y_coord = vTextureCoord.t + cy;\n"
            "if (y_coord > 1.0) y_coord -= 1.0;\n"
            "float x_coord = vTextureCoord.s - offset;\n"
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

void Waterfallgl::initialize(int wid, int ht){

    data_width = wid;
    data_height = ht;
    cy = MAX_CL_HEIGHT - 1;

    m_logger = new QOpenGLDebugLogger( this );

    connect( m_logger, SIGNAL( messageLogged( QOpenGLDebugMessage ) ),
             this, SLOT( onMessageLogged( QOpenGLDebugMessage ) ),
             Qt::DirectConnection );

    if ( m_logger->initialize() ) {
        m_logger->startLogging( QOpenGLDebugLogger::SynchronousLogging );
        m_logger->enableMessages();
    }

    m_context->makeCurrent(this);
    ShaderProgram = new QOpenGLShaderProgram(m_context);
    ShaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader);
    ShaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader);
    ShaderProgram->link();
    ShaderProgram->bind();

    spectrumTexture_location = ShaderProgram->uniformLocation("spectrumTexture");
    cy_location =  ShaderProgram->uniformLocation("cy");
    offset_location =  ShaderProgram->uniformLocation("offset");
    waterfallLow_location =  ShaderProgram->uniformLocation("waterfallLow");
    waterfallHigh_location =  ShaderProgram->uniformLocation("waterfallHigh");
    width_location =  ShaderProgram->uniformLocation("width");
    aPosition_location = ShaderProgram->attributeLocation("aPosition");
    textureCoord_location = ShaderProgram->attributeLocation("aTextureCoord");
    uMVPMatrix_location = ShaderProgram->uniformLocation("uMVPMatrix");

    static unsigned char data[MAX_CL_WIDTH][MAX_CL_HEIGHT];
    for (int i = 0; i < MAX_CL_HEIGHT; i++){
        for (int j = 0; j < MAX_CL_WIDTH; j++){
            data[i][j]= (unsigned char) 0xff;
        }
    }

    m_funcs->glGenTextures(1, &spectrumTex);
    m_funcs->glBindTexture(GL_TEXTURE_2D, spectrumTex);
    m_funcs->glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, MAX_CL_WIDTH, MAX_CL_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, (GLvoid*) data);
    m_funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    m_funcs->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    m_funcs->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    m_funcs->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_funcs->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_context->doneCurrent();
}

void Waterfallgl::setHigh(int high) {
    waterfallHigh=high;
    float wfHigh = (float) (waterfallHigh) / 256.0f;
    ShaderProgram->setUniformValue(waterfallHigh_location, wfHigh);

}

void Waterfallgl::setLow(int low) {
    waterfallLow=low;
    float wfLow = (float) (waterfallLow) / 256.0f;
    ShaderProgram->setUniformValue(waterfallLow_location, wfLow);
}

void Waterfallgl::setLO_offset(GLfloat offset){
    LO_offset = offset;
    ShaderProgram->setUniformValue(offset_location, LO_offset);
}


void Waterfallgl::paintGL()
{
    m_context->makeCurrent(this);
    ShaderProgram->bind();

    m_funcs->glClear(GL_COLOR_BUFFER_BIT);


    float current_line = (float) cy /  MAX_CL_HEIGHT;

    ShaderProgram->setUniformValue(cy_location, current_line);

    const GLshort mIndices[] =
    {
        0, 1, 2, 0, 2, 3
    };

    m_vao->bind();
    m_funcs->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, mIndices );

    m_context->swapBuffers(this);
    m_context->doneCurrent();

}

void Waterfallgl::updateWaterfall(char *buffer, int width, int starty){
    Q_UNUSED(starty);

    m_context->makeCurrent(this);
    ShaderProgram->bind();
    setLO_offset(0.0);

    int sum = 0;
    for(int i=0;i<width;i++) sum += -(buffer[i] & 0xFF);
    average = average * 0.99f + (float)sum/(float)width * 0.01f; // running avera
    setLow(average - 10);
    setHigh(average + 50);

    int data_length = (width < MAX_CL_WIDTH) ? width : MAX_CL_WIDTH;
    if (cy-- <= 0) cy = MAX_CL_HEIGHT - 1;

    unsigned char data[MAX_CL_WIDTH];
    for (int i = 0; i < data_length; i++){
        data[i] = buffer[i];
    }

    // Update Texture
    m_funcs->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, cy, MAX_CL_WIDTH, 1,
                    GL_RED, GL_UNSIGNED_BYTE, (GLvoid*)data);
    paintGL();
    m_context->doneCurrent();
}

void Waterfallgl::onMessageLogged(QOpenGLDebugMessage message){
    qDebug() << message;
}

