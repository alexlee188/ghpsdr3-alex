/****************************************************************************
** Meta object code from reading C++ file 'Spectrum.h'
**
** Created: Thu Jan 3 18:55:10 2013
**      by: The Qt Meta Object Compiler version 67 (Qt 5.0.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtRadio/Spectrum.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Spectrum.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.0.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Spectrum_t {
    QByteArrayData data[17];
    char stringdata[188];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Spectrum_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Spectrum_t qt_meta_stringdata_Spectrum = {
    {
QT_MOC_LITERAL(0, 0, 8),
QT_MOC_LITERAL(1, 9, 14),
QT_MOC_LITERAL(2, 24, 0),
QT_MOC_LITERAL(3, 25, 5),
QT_MOC_LITERAL(4, 31, 4),
QT_MOC_LITERAL(5, 36, 19),
QT_MOC_LITERAL(6, 56, 4),
QT_MOC_LITERAL(7, 61, 18),
QT_MOC_LITERAL(8, 80, 3),
QT_MOC_LITERAL(9, 84, 20),
QT_MOC_LITERAL(10, 105, 19),
QT_MOC_LITERAL(11, 125, 10),
QT_MOC_LITERAL(12, 136, 5),
QT_MOC_LITERAL(13, 142, 11),
QT_MOC_LITERAL(14, 154, 19),
QT_MOC_LITERAL(15, 174, 6),
QT_MOC_LITERAL(16, 181, 5)
    },
    "Spectrum\0frequencyMoved\0\0steps\0step\0"
    "spectrumHighChanged\0high\0spectrumLowChanged\0"
    "low\0waterfallHighChanged\0waterfallLowChanged\0"
    "meterValue\0meter\0subrx_meter\0"
    "squelchValueChanged\0setAvg\0value\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Spectrum[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   54,    2, 0x05,
       5,    1,   59,    2, 0x05,
       7,    1,   62,    2, 0x05,
       9,    1,   65,    2, 0x05,
      10,    1,   68,    2, 0x05,
      11,    2,   71,    2, 0x05,
      14,    1,   76,    2, 0x05,

 // slots: name, argc, parameters, tag, flags
      15,    1,   79,    2, 0x0a,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    4,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   12,   13,
    QMetaType::Void, QMetaType::Int,    4,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,   16,

       0        // eod
};

void Spectrum::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Spectrum *_t = static_cast<Spectrum *>(_o);
        switch (_id) {
        case 0: _t->frequencyMoved((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->spectrumHighChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->spectrumLowChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->waterfallHighChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->waterfallLowChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->meterValue((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->squelchValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->setAvg((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (Spectrum::*_t)(int , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Spectrum::frequencyMoved)) {
                *result = 0;
            }
        }
        {
            typedef void (Spectrum::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Spectrum::spectrumHighChanged)) {
                *result = 1;
            }
        }
        {
            typedef void (Spectrum::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Spectrum::spectrumLowChanged)) {
                *result = 2;
            }
        }
        {
            typedef void (Spectrum::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Spectrum::waterfallHighChanged)) {
                *result = 3;
            }
        }
        {
            typedef void (Spectrum::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Spectrum::waterfallLowChanged)) {
                *result = 4;
            }
        }
        {
            typedef void (Spectrum::*_t)(int , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Spectrum::meterValue)) {
                *result = 5;
            }
        }
        {
            typedef void (Spectrum::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Spectrum::squelchValueChanged)) {
                *result = 6;
            }
        }
    }
}

const QMetaObject Spectrum::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_Spectrum.data,
      qt_meta_data_Spectrum,  qt_static_metacall, 0, 0}
};


const QMetaObject *Spectrum::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Spectrum::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Spectrum.stringdata))
        return static_cast<void*>(const_cast< Spectrum*>(this));
    return QFrame::qt_metacast(_clname);
}

int Spectrum::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void Spectrum::frequencyMoved(int _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Spectrum::spectrumHighChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Spectrum::spectrumLowChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Spectrum::waterfallHighChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Spectrum::waterfallLowChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void Spectrum::meterValue(int _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void Spectrum::squelchValueChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_END_MOC_NAMESPACE
