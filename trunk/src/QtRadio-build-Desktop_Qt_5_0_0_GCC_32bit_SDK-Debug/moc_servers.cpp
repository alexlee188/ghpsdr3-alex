/****************************************************************************
** Meta object code from reading C++ file 'servers.h'
**
** Created: Sun Jan 13 22:03:38 2013
**      by: The Qt Meta Object Compiler version 67 (Qt 5.0.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtRadio/servers.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'servers.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.0.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Servers_t {
    QByteArrayData data[17];
    char stringdata[225];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Servers_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Servers_t qt_meta_stringdata_Servers = {
    {
QT_MOC_LITERAL(0, 0, 7),
QT_MOC_LITERAL(1, 8, 13),
QT_MOC_LITERAL(2, 22, 0),
QT_MOC_LITERAL(3, 23, 10),
QT_MOC_LITERAL(4, 34, 2),
QT_MOC_LITERAL(5, 37, 12),
QT_MOC_LITERAL(6, 50, 22),
QT_MOC_LITERAL(7, 73, 12),
QT_MOC_LITERAL(8, 86, 14),
QT_MOC_LITERAL(9, 101, 5),
QT_MOC_LITERAL(10, 107, 24),
QT_MOC_LITERAL(11, 132, 26),
QT_MOC_LITERAL(12, 159, 23),
QT_MOC_LITERAL(13, 183, 10),
QT_MOC_LITERAL(14, 194, 10),
QT_MOC_LITERAL(15, 205, 12),
QT_MOC_LITERAL(16, 218, 5)
    },
    "Servers\0disconnectNow\0\0connectNow\0IP\0"
    "dialogClosed\0on_closebutton_clicked\0"
    "finishedSlot\0QNetworkReply*\0reply\0"
    "on_refreshButton_clicked\0"
    "on_QuickDisconnect_clicked\0"
    "on_QuickConnect_clicked\0TimerFired\0"
    "closeEvent\0QCloseEvent*\0event\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Servers[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x05,
       3,    1,   65,    2, 0x05,
       5,    0,   68,    2, 0x05,

 // slots: name, argc, parameters, tag, flags
       6,    0,   69,    2, 0x08,
       7,    1,   70,    2, 0x08,
      10,    0,   73,    2, 0x08,
      11,    0,   74,    2, 0x08,
      12,    0,   75,    2, 0x08,
      13,    0,   76,    2, 0x08,
      14,    1,   77,    2, 0x08,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 15,   16,

       0        // eod
};

void Servers::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Servers *_t = static_cast<Servers *>(_o);
        switch (_id) {
        case 0: _t->disconnectNow(); break;
        case 1: _t->connectNow((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->dialogClosed(); break;
        case 3: _t->on_closebutton_clicked(); break;
        case 4: _t->finishedSlot((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 5: _t->on_refreshButton_clicked(); break;
        case 6: _t->on_QuickDisconnect_clicked(); break;
        case 7: _t->on_QuickConnect_clicked(); break;
        case 8: _t->TimerFired(); break;
        case 9: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QNetworkReply* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (Servers::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Servers::disconnectNow)) {
                *result = 0;
            }
        }
        {
            typedef void (Servers::*_t)(QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Servers::connectNow)) {
                *result = 1;
            }
        }
        {
            typedef void (Servers::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Servers::dialogClosed)) {
                *result = 2;
            }
        }
    }
}

const QMetaObject Servers::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_Servers.data,
      qt_meta_data_Servers,  qt_static_metacall, 0, 0}
};


const QMetaObject *Servers::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Servers::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Servers.stringdata))
        return static_cast<void*>(const_cast< Servers*>(this));
    return QDialog::qt_metacast(_clname);
}

int Servers::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void Servers::disconnectNow()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void Servers::connectNow(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Servers::dialogClosed()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}
QT_END_MOC_NAMESPACE
