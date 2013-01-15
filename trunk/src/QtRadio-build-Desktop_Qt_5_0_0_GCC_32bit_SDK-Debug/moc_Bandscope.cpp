/****************************************************************************
** Meta object code from reading C++ file 'Bandscope.h'
**
** Created: Sun Jan 13 22:03:25 2013
**      by: The Qt Meta Object Compiler version 67 (Qt 5.0.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtRadio/Bandscope.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Bandscope.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.0.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Bandscope_t {
    QByteArrayData data[10];
    char stringdata[95];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Bandscope_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Bandscope_t qt_meta_stringdata_Bandscope = {
    {
QT_MOC_LITERAL(0, 0, 9),
QT_MOC_LITERAL(1, 10, 15),
QT_MOC_LITERAL(2, 26, 0),
QT_MOC_LITERAL(3, 27, 5),
QT_MOC_LITERAL(4, 33, 6),
QT_MOC_LITERAL(5, 40, 6),
QT_MOC_LITERAL(6, 47, 9),
QT_MOC_LITERAL(7, 57, 12),
QT_MOC_LITERAL(8, 70, 7),
QT_MOC_LITERAL(9, 78, 15)
    },
    "Bandscope\0bandscopeBuffer\0\0char*\0"
    "header\0buffer\0connected\0disconnected\0"
    "message\0updateBandscope\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Bandscope[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   34,    2, 0x0a,
       6,    0,   39,    2, 0x0a,
       7,    1,   40,    2, 0x0a,
       9,    0,   43,    2, 0x0a,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    4,    5,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void,

       0        // eod
};

void Bandscope::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Bandscope *_t = static_cast<Bandscope *>(_o);
        switch (_id) {
        case 0: _t->bandscopeBuffer((*reinterpret_cast< char*(*)>(_a[1])),(*reinterpret_cast< char*(*)>(_a[2]))); break;
        case 1: _t->connected(); break;
        case 2: _t->disconnected((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->updateBandscope(); break;
        default: ;
        }
    }
}

const QMetaObject Bandscope::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_Bandscope.data,
      qt_meta_data_Bandscope,  qt_static_metacall, 0, 0}
};


const QMetaObject *Bandscope::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Bandscope::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Bandscope.stringdata))
        return static_cast<void*>(const_cast< Bandscope*>(this));
    return QFrame::qt_metacast(_clname);
}

int Bandscope::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
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
QT_END_MOC_NAMESPACE
