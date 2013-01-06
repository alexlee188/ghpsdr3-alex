/****************************************************************************
** Meta object code from reading C++ file 'RTP.h'
**
** Created: Thu Jan 3 18:55:31 2013
**      by: The Qt Meta Object Compiler version 67 (Qt 5.0.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtRadio/RTP.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RTP.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.0.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_RTP_t {
    QByteArrayData data[11];
    char stringdata[95];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_RTP_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_RTP_t qt_meta_stringdata_RTP = {
    {
QT_MOC_LITERAL(0, 0, 3),
QT_MOC_LITERAL(1, 4, 15),
QT_MOC_LITERAL(2, 20, 0),
QT_MOC_LITERAL(3, 21, 11),
QT_MOC_LITERAL(4, 33, 7),
QT_MOC_LITERAL(5, 41, 8),
QT_MOC_LITERAL(6, 50, 9),
QT_MOC_LITERAL(7, 60, 4),
QT_MOC_LITERAL(8, 65, 14),
QT_MOC_LITERAL(9, 80, 6),
QT_MOC_LITERAL(10, 87, 6)
    },
    "RTP\0rtp_set_session\0\0RtpSession*\0"
    "session\0shutdown\0setRemote\0send\0"
    "unsigned char*\0buffer\0length\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_RTP[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x05,

 // slots: name, argc, parameters, tag, flags
       5,    0,   37,    2, 0x0a,
       6,    2,   38,    2, 0x0a,
       7,    2,   43,    2, 0x0a,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 8, QMetaType::Int,    9,   10,

       0        // eod
};

void RTP::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        RTP *_t = static_cast<RTP *>(_o);
        switch (_id) {
        case 0: _t->rtp_set_session((*reinterpret_cast< RtpSession*(*)>(_a[1]))); break;
        case 1: _t->shutdown(); break;
        case 2: _t->setRemote((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->send((*reinterpret_cast< unsigned char*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (RTP::*_t)(RtpSession * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&RTP::rtp_set_session)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject RTP::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_RTP.data,
      qt_meta_data_RTP,  qt_static_metacall, 0, 0}
};


const QMetaObject *RTP::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RTP::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RTP.stringdata))
        return static_cast<void*>(const_cast< RTP*>(this));
    return QObject::qt_metacast(_clname);
}

int RTP::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void RTP::rtp_set_session(RtpSession * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
