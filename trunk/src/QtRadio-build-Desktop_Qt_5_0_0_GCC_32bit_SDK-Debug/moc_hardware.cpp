/****************************************************************************
** Meta object code from reading C++ file 'hardware.h'
**
** Created: Sun Jan 13 22:03:40 2013
**      by: The Qt Meta Object Compiler version 67 (Qt 5.0.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtRadio/hardware.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'hardware.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.0.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_DlgHardware_t {
    QByteArrayData data[3];
    char stringdata[28];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_DlgHardware_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_DlgHardware_t qt_meta_stringdata_DlgHardware = {
    {
QT_MOC_LITERAL(0, 0, 11),
QT_MOC_LITERAL(1, 12, 13),
QT_MOC_LITERAL(2, 26, 0)
    },
    "DlgHardware\0processAnswer\0\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DlgHardware[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x0a,

 // slots: parameters
    QMetaType::Void, QMetaType::QStringList,    2,

       0        // eod
};

void DlgHardware::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DlgHardware *_t = static_cast<DlgHardware *>(_o);
        switch (_id) {
        case 0: _t->processAnswer((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject DlgHardware::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_DlgHardware.data,
      qt_meta_data_DlgHardware,  qt_static_metacall, 0, 0}
};


const QMetaObject *DlgHardware::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DlgHardware::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DlgHardware.stringdata))
        return static_cast<void*>(const_cast< DlgHardware*>(this));
    return QWidget::qt_metacast(_clname);
}

int DlgHardware::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_HardwarePerseus_t {
    QByteArrayData data[7];
    char stringdata[77];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_HardwarePerseus_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_HardwarePerseus_t qt_meta_stringdata_HardwarePerseus = {
    {
QT_MOC_LITERAL(0, 0, 15),
QT_MOC_LITERAL(1, 16, 10),
QT_MOC_LITERAL(2, 27, 0),
QT_MOC_LITERAL(3, 28, 5),
QT_MOC_LITERAL(4, 34, 13),
QT_MOC_LITERAL(5, 48, 13),
QT_MOC_LITERAL(6, 62, 13)
    },
    "HardwarePerseus\0attClicked\0\0state\0"
    "ditherChanged\0preampChanged\0processAnswer\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_HardwarePerseus[] = {

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
       1,    1,   34,    2, 0x08,
       4,    1,   37,    2, 0x08,
       5,    1,   40,    2, 0x08,
       6,    1,   43,    2, 0x08,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::QStringList,    2,

       0        // eod
};

void HardwarePerseus::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        HardwarePerseus *_t = static_cast<HardwarePerseus *>(_o);
        switch (_id) {
        case 0: _t->attClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->ditherChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->preampChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->processAnswer((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject HardwarePerseus::staticMetaObject = {
    { &DlgHardware::staticMetaObject, qt_meta_stringdata_HardwarePerseus.data,
      qt_meta_data_HardwarePerseus,  qt_static_metacall, 0, 0}
};


const QMetaObject *HardwarePerseus::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *HardwarePerseus::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_HardwarePerseus.stringdata))
        return static_cast<void*>(const_cast< HardwarePerseus*>(this));
    return DlgHardware::qt_metacast(_clname);
}

int HardwarePerseus::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DlgHardware::qt_metacall(_c, _id, _a);
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
struct qt_meta_stringdata_HardwareSdriq_t {
    QByteArrayData data[5];
    char stringdata[47];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_HardwareSdriq_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_HardwareSdriq_t qt_meta_stringdata_HardwareSdriq = {
    {
QT_MOC_LITERAL(0, 0, 13),
QT_MOC_LITERAL(1, 14, 10),
QT_MOC_LITERAL(2, 25, 0),
QT_MOC_LITERAL(3, 26, 5),
QT_MOC_LITERAL(4, 32, 13)
    },
    "HardwareSdriq\0attClicked\0\0state\0"
    "processAnswer\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_HardwareSdriq[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x08,
       4,    1,   27,    2, 0x08,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::QStringList,    2,

       0        // eod
};

void HardwareSdriq::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        HardwareSdriq *_t = static_cast<HardwareSdriq *>(_o);
        switch (_id) {
        case 0: _t->attClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->processAnswer((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject HardwareSdriq::staticMetaObject = {
    { &DlgHardware::staticMetaObject, qt_meta_stringdata_HardwareSdriq.data,
      qt_meta_data_HardwareSdriq,  qt_static_metacall, 0, 0}
};


const QMetaObject *HardwareSdriq::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *HardwareSdriq::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_HardwareSdriq.stringdata))
        return static_cast<void*>(const_cast< HardwareSdriq*>(this));
    return DlgHardware::qt_metacast(_clname);
}

int HardwareSdriq::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DlgHardware::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
struct qt_meta_stringdata_HardwareHiqsdr_t {
    QByteArrayData data[9];
    char stringdata[89];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_HardwareHiqsdr_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_HardwareHiqsdr_t qt_meta_stringdata_HardwareHiqsdr = {
    {
QT_MOC_LITERAL(0, 0, 14),
QT_MOC_LITERAL(1, 15, 10),
QT_MOC_LITERAL(2, 26, 0),
QT_MOC_LITERAL(3, 27, 5),
QT_MOC_LITERAL(4, 33, 10),
QT_MOC_LITERAL(5, 44, 1),
QT_MOC_LITERAL(6, 46, 13),
QT_MOC_LITERAL(7, 60, 13),
QT_MOC_LITERAL(8, 74, 13)
    },
    "HardwareHiqsdr\0attClicked\0\0state\0"
    "antClicked\0n\0preselClicked\0preampChanged\0"
    "processAnswer\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_HardwareHiqsdr[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x08,
       4,    1,   42,    2, 0x08,
       6,    1,   45,    2, 0x08,
       7,    1,   48,    2, 0x08,
       8,    1,   51,    2, 0x08,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::QStringList,    2,

       0        // eod
};

void HardwareHiqsdr::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        HardwareHiqsdr *_t = static_cast<HardwareHiqsdr *>(_o);
        switch (_id) {
        case 0: _t->attClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->antClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->preselClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->preampChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->processAnswer((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject HardwareHiqsdr::staticMetaObject = {
    { &DlgHardware::staticMetaObject, qt_meta_stringdata_HardwareHiqsdr.data,
      qt_meta_data_HardwareHiqsdr,  qt_static_metacall, 0, 0}
};


const QMetaObject *HardwareHiqsdr::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *HardwareHiqsdr::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_HardwareHiqsdr.stringdata))
        return static_cast<void*>(const_cast< HardwareHiqsdr*>(this));
    return DlgHardware::qt_metacast(_clname);
}

int HardwareHiqsdr::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DlgHardware::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
struct qt_meta_stringdata_HardwareRtlsdr_t {
    QByteArrayData data[5];
    char stringdata[48];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_HardwareRtlsdr_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_HardwareRtlsdr_t qt_meta_stringdata_HardwareRtlsdr = {
    {
QT_MOC_LITERAL(0, 0, 14),
QT_MOC_LITERAL(1, 15, 10),
QT_MOC_LITERAL(2, 26, 0),
QT_MOC_LITERAL(3, 27, 5),
QT_MOC_LITERAL(4, 33, 13)
    },
    "HardwareRtlsdr\0attClicked\0\0state\0"
    "processAnswer\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_HardwareRtlsdr[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x08,
       4,    1,   27,    2, 0x08,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::QStringList,    2,

       0        // eod
};

void HardwareRtlsdr::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        HardwareRtlsdr *_t = static_cast<HardwareRtlsdr *>(_o);
        switch (_id) {
        case 0: _t->attClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->processAnswer((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject HardwareRtlsdr::staticMetaObject = {
    { &DlgHardware::staticMetaObject, qt_meta_stringdata_HardwareRtlsdr.data,
      qt_meta_data_HardwareRtlsdr,  qt_static_metacall, 0, 0}
};


const QMetaObject *HardwareRtlsdr::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *HardwareRtlsdr::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_HardwareRtlsdr.stringdata))
        return static_cast<void*>(const_cast< HardwareRtlsdr*>(this));
    return DlgHardware::qt_metacast(_clname);
}

int HardwareRtlsdr::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DlgHardware::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
