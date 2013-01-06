/****************************************************************************
** Meta object code from reading C++ file 'Connection.h'
**
** Created: Thu Jan 3 18:55:13 2013
**      by: The Qt Meta Object Compiler version 67 (Qt 5.0.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtRadio/Connection.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Connection.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.0.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Connection_t {
    QByteArrayData data[40];
    char stringdata[410];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Connection_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Connection_t qt_meta_stringdata_Connection = {
    {
QT_MOC_LITERAL(0, 0, 10),
QT_MOC_LITERAL(1, 11, 11),
QT_MOC_LITERAL(2, 23, 0),
QT_MOC_LITERAL(3, 24, 12),
QT_MOC_LITERAL(4, 37, 7),
QT_MOC_LITERAL(5, 45, 6),
QT_MOC_LITERAL(6, 52, 5),
QT_MOC_LITERAL(7, 58, 11),
QT_MOC_LITERAL(8, 70, 6),
QT_MOC_LITERAL(9, 77, 14),
QT_MOC_LITERAL(10, 92, 15),
QT_MOC_LITERAL(11, 108, 9),
QT_MOC_LITERAL(12, 118, 4),
QT_MOC_LITERAL(13, 123, 4),
QT_MOC_LITERAL(14, 128, 14),
QT_MOC_LITERAL(15, 143, 12),
QT_MOC_LITERAL(16, 156, 1),
QT_MOC_LITERAL(17, 158, 12),
QT_MOC_LITERAL(18, 171, 1),
QT_MOC_LITERAL(19, 173, 14),
QT_MOC_LITERAL(20, 188, 1),
QT_MOC_LITERAL(21, 190, 1),
QT_MOC_LITERAL(22, 192, 12),
QT_MOC_LITERAL(23, 205, 1),
QT_MOC_LITERAL(24, 207, 13),
QT_MOC_LITERAL(25, 221, 13),
QT_MOC_LITERAL(26, 235, 16),
QT_MOC_LITERAL(27, 252, 8),
QT_MOC_LITERAL(28, 261, 8),
QT_MOC_LITERAL(29, 270, 14),
QT_MOC_LITERAL(30, 285, 7),
QT_MOC_LITERAL(31, 293, 6),
QT_MOC_LITERAL(32, 300, 12),
QT_MOC_LITERAL(33, 313, 8),
QT_MOC_LITERAL(34, 322, 9),
QT_MOC_LITERAL(35, 332, 10),
QT_MOC_LITERAL(36, 343, 11),
QT_MOC_LITERAL(37, 355, 28),
QT_MOC_LITERAL(38, 384, 10),
QT_MOC_LITERAL(39, 395, 13)
    },
    "Connection\0isConnected\0\0disconnected\0"
    "message\0header\0char*\0audioBuffer\0"
    "buffer\0spectrumBuffer\0bandscopeBuffer\0"
    "remoteRTP\0host\0port\0printStatusBar\0"
    "slaveSetFreq\0f\0slaveSetMode\0m\0"
    "slaveSetFilter\0l\0r\0slaveSetZoom\0z\0"
    "setdspversion\0setservername\0"
    "setRemoteRTPPort\0setCanTX\0setChkTX\0"
    "resetbandedges\0loffset\0setFPS\0"
    "setProtocol3\0hardware\0connected\0"
    "disconnect\0socketError\0"
    "QAbstractSocket::SocketError\0socketData\0"
    "processBuffer\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Connection[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      27,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      21,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,  149,    2, 0x05,
       3,    1,  150,    2, 0x05,
       5,    1,  153,    2, 0x05,
       7,    2,  156,    2, 0x05,
       9,    2,  161,    2, 0x05,
      10,    2,  166,    2, 0x05,
      11,    2,  171,    2, 0x05,
      14,    1,  176,    2, 0x05,
      15,    1,  179,    2, 0x05,
      17,    1,  182,    2, 0x05,
      19,    2,  185,    2, 0x05,
      22,    1,  190,    2, 0x05,
      24,    2,  193,    2, 0x05,
      25,    1,  198,    2, 0x05,
      26,    2,  201,    2, 0x05,
      27,    1,  206,    2, 0x05,
      28,    1,  209,    2, 0x05,
      29,    1,  212,    2, 0x05,
      31,    0,  215,    2, 0x05,
      32,    1,  216,    2, 0x05,
      33,    1,  219,    2, 0x05,

 // slots: name, argc, parameters, tag, flags
      34,    0,  222,    2, 0x0a,
       3,    0,  223,    2, 0x0a,
      35,    0,  224,    2, 0x0a,
      36,    1,  225,    2, 0x0a,
      38,    0,  228,    2, 0x0a,
      39,    0,  229,    2, 0x0a,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void, 0x80000000 | 6,    5,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 6,    5,    8,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 6,    5,    8,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 6,    5,    8,
    QMetaType::Void, 0x80000000 | 6, QMetaType::Int,   12,   13,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void, QMetaType::LongLong,   16,
    QMetaType::Void, QMetaType::Int,   18,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   20,   21,
    QMetaType::Void, QMetaType::Int,   23,
    QMetaType::Void, QMetaType::Long, QMetaType::QString,    2,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    2,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Double,   30,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::QString,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 37,   36,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Connection::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Connection *_t = static_cast<Connection *>(_o);
        switch (_id) {
        case 0: _t->isConnected(); break;
        case 1: _t->disconnected((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->header((*reinterpret_cast< char*(*)>(_a[1]))); break;
        case 3: _t->audioBuffer((*reinterpret_cast< char*(*)>(_a[1])),(*reinterpret_cast< char*(*)>(_a[2]))); break;
        case 4: _t->spectrumBuffer((*reinterpret_cast< char*(*)>(_a[1])),(*reinterpret_cast< char*(*)>(_a[2]))); break;
        case 5: _t->bandscopeBuffer((*reinterpret_cast< char*(*)>(_a[1])),(*reinterpret_cast< char*(*)>(_a[2]))); break;
        case 6: _t->remoteRTP((*reinterpret_cast< char*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->printStatusBar((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 8: _t->slaveSetFreq((*reinterpret_cast< long long(*)>(_a[1]))); break;
        case 9: _t->slaveSetMode((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->slaveSetFilter((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 11: _t->slaveSetZoom((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->setdspversion((*reinterpret_cast< long(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 13: _t->setservername((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 14: _t->setRemoteRTPPort((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 15: _t->setCanTX((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 16: _t->setChkTX((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 17: _t->resetbandedges((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 18: _t->setFPS(); break;
        case 19: _t->setProtocol3((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 20: _t->hardware((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 21: _t->connected(); break;
        case 22: _t->disconnected(); break;
        case 23: _t->disconnect(); break;
        case 24: _t->socketError((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        case 25: _t->socketData(); break;
        case 26: _t->processBuffer(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 24:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAbstractSocket::SocketError >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (Connection::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::isConnected)) {
                *result = 0;
            }
        }
        {
            typedef void (Connection::*_t)(QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::disconnected)) {
                *result = 1;
            }
        }
        {
            typedef void (Connection::*_t)(char * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::header)) {
                *result = 2;
            }
        }
        {
            typedef void (Connection::*_t)(char * , char * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::audioBuffer)) {
                *result = 3;
            }
        }
        {
            typedef void (Connection::*_t)(char * , char * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::spectrumBuffer)) {
                *result = 4;
            }
        }
        {
            typedef void (Connection::*_t)(char * , char * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::bandscopeBuffer)) {
                *result = 5;
            }
        }
        {
            typedef void (Connection::*_t)(char * , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::remoteRTP)) {
                *result = 6;
            }
        }
        {
            typedef void (Connection::*_t)(QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::printStatusBar)) {
                *result = 7;
            }
        }
        {
            typedef void (Connection::*_t)(long long );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::slaveSetFreq)) {
                *result = 8;
            }
        }
        {
            typedef void (Connection::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::slaveSetMode)) {
                *result = 9;
            }
        }
        {
            typedef void (Connection::*_t)(int , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::slaveSetFilter)) {
                *result = 10;
            }
        }
        {
            typedef void (Connection::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::slaveSetZoom)) {
                *result = 11;
            }
        }
        {
            typedef void (Connection::*_t)(long , QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::setdspversion)) {
                *result = 12;
            }
        }
        {
            typedef void (Connection::*_t)(QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::setservername)) {
                *result = 13;
            }
        }
        {
            typedef void (Connection::*_t)(QString , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::setRemoteRTPPort)) {
                *result = 14;
            }
        }
        {
            typedef void (Connection::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::setCanTX)) {
                *result = 15;
            }
        }
        {
            typedef void (Connection::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::setChkTX)) {
                *result = 16;
            }
        }
        {
            typedef void (Connection::*_t)(double );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::resetbandedges)) {
                *result = 17;
            }
        }
        {
            typedef void (Connection::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::setFPS)) {
                *result = 18;
            }
        }
        {
            typedef void (Connection::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::setProtocol3)) {
                *result = 19;
            }
        }
        {
            typedef void (Connection::*_t)(QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Connection::hardware)) {
                *result = 20;
            }
        }
    }
}

const QMetaObject Connection::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Connection.data,
      qt_meta_data_Connection,  qt_static_metacall, 0, 0}
};


const QMetaObject *Connection::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Connection::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Connection.stringdata))
        return static_cast<void*>(const_cast< Connection*>(this));
    return QObject::qt_metacast(_clname);
}

int Connection::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 27)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 27;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 27)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 27;
    }
    return _id;
}

// SIGNAL 0
void Connection::isConnected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void Connection::disconnected(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Connection::header(char * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Connection::audioBuffer(char * _t1, char * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Connection::spectrumBuffer(char * _t1, char * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void Connection::bandscopeBuffer(char * _t1, char * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void Connection::remoteRTP(char * _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void Connection::printStatusBar(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void Connection::slaveSetFreq(long long _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void Connection::slaveSetMode(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void Connection::slaveSetFilter(int _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void Connection::slaveSetZoom(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void Connection::setdspversion(long _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void Connection::setservername(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}

// SIGNAL 14
void Connection::setRemoteRTPPort(QString _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}

// SIGNAL 15
void Connection::setCanTX(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 15, _a);
}

// SIGNAL 16
void Connection::setChkTX(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 16, _a);
}

// SIGNAL 17
void Connection::resetbandedges(double _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 17, _a);
}

// SIGNAL 18
void Connection::setFPS()
{
    QMetaObject::activate(this, &staticMetaObject, 18, 0);
}

// SIGNAL 19
void Connection::setProtocol3(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 19, _a);
}

// SIGNAL 20
void Connection::hardware(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 20, _a);
}
QT_END_MOC_NAMESPACE
