/****************************************************************************
** Meta object code from reading C++ file 'BookmarksEditDialog.h'
**
** Created: Sun Jan 13 22:03:29 2013
**      by: The Qt Meta Object Compiler version 67 (Qt 5.0.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtRadio/BookmarksEditDialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'BookmarksEditDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.0.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_BookmarksEditDialog_t {
    QByteArrayData data[10];
    char stringdata[132];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_BookmarksEditDialog_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_BookmarksEditDialog_t qt_meta_stringdata_BookmarksEditDialog = {
    {
QT_MOC_LITERAL(0, 0, 19),
QT_MOC_LITERAL(1, 20, 15),
QT_MOC_LITERAL(2, 36, 0),
QT_MOC_LITERAL(3, 37, 5),
QT_MOC_LITERAL(4, 43, 16),
QT_MOC_LITERAL(5, 60, 15),
QT_MOC_LITERAL(6, 76, 5),
QT_MOC_LITERAL(7, 82, 14),
QT_MOC_LITERAL(8, 97, 18),
QT_MOC_LITERAL(9, 116, 14)
    },
    "BookmarksEditDialog\0bookmarkDeleted\0"
    "\0entry\0bookmarkSelected\0bookmarkUpdated\0"
    "title\0deleteBookmark\0bookmarkRowChanged\0"
    "updateBookmark\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_BookmarksEditDialog[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x05,
       4,    1,   47,    2, 0x05,
       5,    2,   50,    2, 0x05,

 // slots: name, argc, parameters, tag, flags
       7,    0,   55,    2, 0x0a,
       8,    1,   56,    2, 0x0a,
       9,    0,   59,    2, 0x0a,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,    3,    6,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,

       0        // eod
};

void BookmarksEditDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        BookmarksEditDialog *_t = static_cast<BookmarksEditDialog *>(_o);
        switch (_id) {
        case 0: _t->bookmarkDeleted((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->bookmarkSelected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->bookmarkUpdated((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 3: _t->deleteBookmark(); break;
        case 4: _t->bookmarkRowChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->updateBookmark(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (BookmarksEditDialog::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&BookmarksEditDialog::bookmarkDeleted)) {
                *result = 0;
            }
        }
        {
            typedef void (BookmarksEditDialog::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&BookmarksEditDialog::bookmarkSelected)) {
                *result = 1;
            }
        }
        {
            typedef void (BookmarksEditDialog::*_t)(int , QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&BookmarksEditDialog::bookmarkUpdated)) {
                *result = 2;
            }
        }
    }
}

const QMetaObject BookmarksEditDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_BookmarksEditDialog.data,
      qt_meta_data_BookmarksEditDialog,  qt_static_metacall, 0, 0}
};


const QMetaObject *BookmarksEditDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BookmarksEditDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BookmarksEditDialog.stringdata))
        return static_cast<void*>(const_cast< BookmarksEditDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int BookmarksEditDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void BookmarksEditDialog::bookmarkDeleted(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void BookmarksEditDialog::bookmarkSelected(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void BookmarksEditDialog::bookmarkUpdated(int _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
