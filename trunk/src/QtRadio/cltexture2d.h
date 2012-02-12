
/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtOpenCL module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef CLTEXTURE2D_H
#define CLTEXTURE2D_H

#include <qcl/qclimage.h>
#include <QtCore/qscopedpointer.h>
#include <QtOpenGL/qgl.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QCLContextGL;
class CLTexture2DPrivate;

// Note: this class probably should be in the QtOpenGL library.
class CLTexture2D : public QCLImage2D
{
public:
    CLTexture2D();
    ~CLTexture2D();

    bool create(QCLContextGL *context, const QSize &size);
    bool create(QCLContextGL *context, int width, int height);
    void destroy();

    void acquire();
    void release();

    GLuint textureId() const;

private:
    QScopedPointer<CLTexture2DPrivate> d_ptr;

    Q_DISABLE_COPY(CLTexture2D)
    Q_DECLARE_PRIVATE(CLTexture2D)
};

inline bool CLTexture2D::create(QCLContextGL *context, int width, int height)
{
    return create(context, QSize(width, height));
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
