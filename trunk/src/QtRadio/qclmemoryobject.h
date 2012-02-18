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

#ifndef QCLMEMORYOBJECT_H
#define QCLMEMORYOBJECT_H

#include "qclevent.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(CL)

class QCLContext;

class Q_CL_EXPORT QCLMemoryObject
{
// protected:


public:
    QCLMemoryObject(QCLContext *context = 0) : m_context(context), m_id(0) {}
    QCLMemoryObject(QCLContext *context, cl_mem id)
        : m_context(context), m_id(id) {}
    ~QCLMemoryObject();

    enum Access
    {
        ReadWrite = 0x0001,
        WriteOnly = 0x0002,
        ReadOnly  = 0x0004
    };

    bool isNull() const { return m_id == 0; }

    cl_mem memoryId() const { return m_id; }
    QCLContext *context() const { return m_context; }

    QCLMemoryObject::Access access() const;
    cl_mem_flags flags() const;
    void *hostPointer() const;
    size_t size() const;

    void unmap(void *ptr);
    QCLEvent unmapAsync
        (void *ptr, const QCLEventList &after = QCLEventList());

    bool operator==(const QCLMemoryObject &other) const;
    bool operator!=(const QCLMemoryObject &other) const;

protected:
    void setId(QCLContext *context, cl_mem id);

private:
    QCLContext *m_context;
    cl_mem m_id;

    Q_DISABLE_COPY(QCLMemoryObject)
};

inline QCLMemoryObject::~QCLMemoryObject()
{
    if (m_id)
        clReleaseMemObject(m_id);
}

inline bool QCLMemoryObject::operator==(const QCLMemoryObject &other) const
{
    return m_id == other.m_id;
}

inline bool QCLMemoryObject::operator!=(const QCLMemoryObject &other) const
{
    return m_id != other.m_id;
}

inline void QCLMemoryObject::setId(QCLContext *context, cl_mem id)
{
    m_context = context;
    if (id)
        clRetainMemObject(id);
    if (m_id)
        clReleaseMemObject(m_id);
    m_id = id;
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
