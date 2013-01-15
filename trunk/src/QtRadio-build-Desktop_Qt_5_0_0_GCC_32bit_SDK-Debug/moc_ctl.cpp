/****************************************************************************
** Meta object code from reading C++ file 'ctl.h'
**
** Created: Sun Jan 13 22:03:35 2013
**      by: The Qt Meta Object Compiler version 67 (Qt 5.0.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtRadio/ctl.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ctl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.0.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Ctl_t {
    QByteArrayData data[28];
    char stringdata[390];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Ctl_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Ctl_t qt_meta_stringdata_Ctl = {
    {
QT_MOC_LITERAL(0, 0, 3),
QT_MOC_LITERAL(1, 4, 9),
QT_MOC_LITERAL(2, 14, 0),
QT_MOC_LITERAL(3, 15, 6),
QT_MOC_LITERAL(4, 22, 3),
QT_MOC_LITERAL(5, 26, 22),
QT_MOC_LITERAL(6, 49, 3),
QT_MOC_LITERAL(7, 53, 12),
QT_MOC_LITERAL(8, 66, 5),
QT_MOC_LITERAL(9, 72, 16),
QT_MOC_LITERAL(10, 89, 5),
QT_MOC_LITERAL(11, 95, 16),
QT_MOC_LITERAL(12, 112, 16),
QT_MOC_LITERAL(13, 129, 5),
QT_MOC_LITERAL(14, 135, 6),
QT_MOC_LITERAL(15, 142, 5),
QT_MOC_LITERAL(16, 148, 17),
QT_MOC_LITERAL(17, 166, 7),
QT_MOC_LITERAL(18, 174, 18),
QT_MOC_LITERAL(19, 193, 25),
QT_MOC_LITERAL(20, 219, 24),
QT_MOC_LITERAL(21, 244, 4),
QT_MOC_LITERAL(22, 249, 27),
QT_MOC_LITERAL(23, 277, 23),
QT_MOC_LITERAL(24, 301, 21),
QT_MOC_LITERAL(25, 323, 22),
QT_MOC_LITERAL(26, 346, 21),
QT_MOC_LITERAL(27, 368, 20)
    },
    "Ctl\0pttChange\0\0caller\0ptt\0"
    "pwrSlider_valueChanged\0pwr\0testBtnClick\0"
    "state\0testSliderChange\0value\0"
    "masterBtnClicked\0update_mic_level\0"
    "level\0HideTX\0cantx\0on_btnMox_clicked\0"
    "checked\0on_btnTune_clicked\0"
    "on_pwrSlider_valueChanged\0"
    "on_checkBox_stateChanged\0arg1\0"
    "on_pwrSlider_2_valueChanged\0"
    "on_spinBox_valueChanged\0on_pushButton_pressed\0"
    "on_pushButton_released\0on_pushButton_toggled\0"
    "on_btnMaster_clicked\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Ctl[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   99,    2, 0x05,
       5,    1,  104,    2, 0x05,
       7,    1,  107,    2, 0x05,
       9,    1,  110,    2, 0x05,
      11,    0,  113,    2, 0x05,

 // slots: name, argc, parameters, tag, flags
      12,    1,  114,    2, 0x0a,
      14,    1,  117,    2, 0x0a,
      16,    1,  120,    2, 0x08,
      18,    1,  123,    2, 0x08,
      19,    1,  126,    2, 0x08,
      20,    1,  129,    2, 0x08,
      22,    1,  132,    2, 0x08,
      23,    1,  135,    2, 0x08,
      24,    0,  138,    2, 0x08,
      25,    0,  139,    2, 0x08,
      26,    1,  140,    2, 0x08,
      27,    0,  143,    2, 0x08,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    3,    4,
    QMetaType::Void, QMetaType::Double,    6,
    QMetaType::Void, QMetaType::Bool,    8,
    QMetaType::Void, QMetaType::Int,   10,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QReal,   13,
    QMetaType::Void, QMetaType::Bool,   15,
    QMetaType::Void, QMetaType::Bool,   17,
    QMetaType::Void, QMetaType::Bool,   17,
    QMetaType::Void, QMetaType::Int,   10,
    QMetaType::Void, QMetaType::Int,   21,
    QMetaType::Void, QMetaType::Int,   10,
    QMetaType::Void, QMetaType::Int,   21,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   17,
    QMetaType::Void,

       0        // eod
};

void Ctl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Ctl *_t = static_cast<Ctl *>(_o);
        switch (_id) {
        case 0: _t->pttChange((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: _t->pwrSlider_valueChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 2: _t->testBtnClick((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->testSliderChange((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->masterBtnClicked(); break;
        case 5: _t->update_mic_level((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 6: _t->HideTX((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->on_btnMox_clicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->on_btnTune_clicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->on_pwrSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->on_checkBox_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->on_pwrSlider_2_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->on_spinBox_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->on_pushButton_pressed(); break;
        case 14: _t->on_pushButton_released(); break;
        case 15: _t->on_pushButton_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 16: _t->on_btnMaster_clicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (Ctl::*_t)(int , bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Ctl::pttChange)) {
                *result = 0;
            }
        }
        {
            typedef void (Ctl::*_t)(double );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Ctl::pwrSlider_valueChanged)) {
                *result = 1;
            }
        }
        {
            typedef void (Ctl::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Ctl::testBtnClick)) {
                *result = 2;
            }
        }
        {
            typedef void (Ctl::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Ctl::testSliderChange)) {
                *result = 3;
            }
        }
        {
            typedef void (Ctl::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Ctl::masterBtnClicked)) {
                *result = 4;
            }
        }
    }
}

const QMetaObject Ctl::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_Ctl.data,
      qt_meta_data_Ctl,  qt_static_metacall, 0, 0}
};


const QMetaObject *Ctl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Ctl::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Ctl.stringdata))
        return static_cast<void*>(const_cast< Ctl*>(this));
    return QFrame::qt_metacast(_clname);
}

int Ctl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void Ctl::pttChange(int _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Ctl::pwrSlider_valueChanged(double _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Ctl::testBtnClick(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Ctl::testSliderChange(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Ctl::masterBtnClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}
QT_END_MOC_NAMESPACE
