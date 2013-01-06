/****************************************************************************
** Meta object code from reading C++ file 'Audioinput.h'
**
** Created: Thu Jan 3 18:55:28 2013
**      by: The Qt Meta Object Compiler version 67 (Qt 5.0.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtRadio/Audioinput.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QQueue>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Audioinput.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.0.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_AudioInfo_t {
    QByteArrayData data[5];
    char stringdata[41];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_AudioInfo_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_AudioInfo_t qt_meta_stringdata_AudioInfo = {
    {
QT_MOC_LITERAL(0, 0, 9),
QT_MOC_LITERAL(1, 10, 6),
QT_MOC_LITERAL(2, 17, 0),
QT_MOC_LITERAL(3, 18, 15),
QT_MOC_LITERAL(4, 34, 5)
    },
    "AudioInfo\0update\0\0QQueue<qint16>*\0"
    "queue\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AudioInfo[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x05,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

void AudioInfo::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        AudioInfo *_t = static_cast<AudioInfo *>(_o);
        switch (_id) {
        case 0: _t->update((*reinterpret_cast< QQueue<qint16>*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QQueue<qint16>* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (AudioInfo::*_t)(QQueue<qint16> * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&AudioInfo::update)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject AudioInfo::staticMetaObject = {
    { &QIODevice::staticMetaObject, qt_meta_stringdata_AudioInfo.data,
      qt_meta_data_AudioInfo,  qt_static_metacall, 0, 0}
};


const QMetaObject *AudioInfo::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AudioInfo::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AudioInfo.stringdata))
        return static_cast<void*>(const_cast< AudioInfo*>(this));
    return QIODevice::qt_metacast(_clname);
}

int AudioInfo::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QIODevice::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void AudioInfo::update(QQueue<qint16> * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_AudioInput_t {
    QByteArrayData data[19];
    char stringdata[219];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_AudioInput_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_AudioInput_t qt_meta_stringdata_AudioInput = {
    {
QT_MOC_LITERAL(0, 0, 10),
QT_MOC_LITERAL(1, 11, 16),
QT_MOC_LITERAL(2, 28, 0),
QT_MOC_LITERAL(3, 29, 5),
QT_MOC_LITERAL(4, 35, 14),
QT_MOC_LITERAL(5, 50, 15),
QT_MOC_LITERAL(6, 66, 5),
QT_MOC_LITERAL(7, 72, 12),
QT_MOC_LITERAL(8, 85, 13),
QT_MOC_LITERAL(9, 99, 12),
QT_MOC_LITERAL(10, 112, 16),
QT_MOC_LITERAL(11, 129, 4),
QT_MOC_LITERAL(12, 134, 4),
QT_MOC_LITERAL(13, 139, 8),
QT_MOC_LITERAL(14, 148, 20),
QT_MOC_LITERAL(15, 169, 9),
QT_MOC_LITERAL(16, 179, 14),
QT_MOC_LITERAL(17, 194, 14),
QT_MOC_LITERAL(18, 209, 8)
    },
    "AudioInput\0mic_update_level\0\0level\0"
    "mic_send_audio\0QQueue<qint16>*\0queue\0"
    "stateChanged\0QAudio::State\0select_audio\0"
    "QAudioDeviceInfo\0info\0rate\0channels\0"
    "QAudioFormat::Endian\0byteOrder\0"
    "slotMicUpdated\0setMicEncoding\0encoding\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AudioInput[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x05,
       4,    1,   47,    2, 0x05,

 // slots: name, argc, parameters, tag, flags
       7,    1,   50,    2, 0x0a,
       9,    4,   53,    2, 0x0a,
      16,    1,   62,    2, 0x0a,
      17,    1,   65,    2, 0x0a,

 // signals: parameters
    QMetaType::Void, QMetaType::QReal,    3,
    QMetaType::Void, 0x80000000 | 5,    6,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 8,    2,
    QMetaType::Void, 0x80000000 | 10, QMetaType::Int, QMetaType::Int, 0x80000000 | 14,   11,   12,   13,   15,
    QMetaType::Void, 0x80000000 | 5,    2,
    QMetaType::Void, QMetaType::Int,   18,

       0        // eod
};

void AudioInput::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        AudioInput *_t = static_cast<AudioInput *>(_o);
        switch (_id) {
        case 0: _t->mic_update_level((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 1: _t->mic_send_audio((*reinterpret_cast< QQueue<qint16>*(*)>(_a[1]))); break;
        case 2: _t->stateChanged((*reinterpret_cast< QAudio::State(*)>(_a[1]))); break;
        case 3: _t->select_audio((*reinterpret_cast< QAudioDeviceInfo(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< QAudioFormat::Endian(*)>(_a[4]))); break;
        case 4: _t->slotMicUpdated((*reinterpret_cast< QQueue<qint16>*(*)>(_a[1]))); break;
        case 5: _t->setMicEncoding((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QQueue<qint16>* >(); break;
            }
            break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAudio::State >(); break;
            }
            break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAudioDeviceInfo >(); break;
            case 3:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAudioFormat::Endian >(); break;
            }
            break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QQueue<qint16>* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (AudioInput::*_t)(qreal );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&AudioInput::mic_update_level)) {
                *result = 0;
            }
        }
        {
            typedef void (AudioInput::*_t)(QQueue<qint16> * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&AudioInput::mic_send_audio)) {
                *result = 1;
            }
        }
    }
}

const QMetaObject AudioInput::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_AudioInput.data,
      qt_meta_data_AudioInput,  qt_static_metacall, 0, 0}
};


const QMetaObject *AudioInput::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AudioInput::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AudioInput.stringdata))
        return static_cast<void*>(const_cast< AudioInput*>(this));
    return QObject::qt_metacast(_clname);
}

int AudioInput::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void AudioInput::mic_update_level(qreal _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void AudioInput::mic_send_audio(QQueue<qint16> * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
