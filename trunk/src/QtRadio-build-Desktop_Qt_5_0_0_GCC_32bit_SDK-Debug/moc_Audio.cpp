/****************************************************************************
** Meta object code from reading C++ file 'Audio.h'
**
** Created: Thu Jan 3 18:55:16 2013
**      by: The Qt Meta Object Compiler version 67 (Qt 5.0.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtRadio/Audio.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Audio.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.0.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Audio_playback_t {
    QByteArrayData data[17];
    char stringdata[220];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Audio_playback_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Audio_playback_t qt_meta_stringdata_Audio_playback = {
    {
QT_MOC_LITERAL(0, 0, 14),
QT_MOC_LITERAL(1, 15, 18),
QT_MOC_LITERAL(2, 34, 0),
QT_MOC_LITERAL(3, 35, 16),
QT_MOC_LITERAL(4, 52, 7),
QT_MOC_LITERAL(5, 60, 20),
QT_MOC_LITERAL(6, 81, 20),
QT_MOC_LITERAL(7, 102, 10),
QT_MOC_LITERAL(8, 113, 18),
QT_MOC_LITERAL(9, 132, 8),
QT_MOC_LITERAL(10, 141, 10),
QT_MOC_LITERAL(11, 152, 3),
QT_MOC_LITERAL(12, 156, 17),
QT_MOC_LITERAL(13, 174, 9),
QT_MOC_LITERAL(14, 184, 14),
QT_MOC_LITERAL(15, 199, 11),
QT_MOC_LITERAL(16, 211, 7)
    },
    "Audio_playback\0set_decoded_buffer\0\0"
    "QHQueue<qint16>*\0pBuffer\0set_audio_byte_order\0"
    "QAudioFormat::Endian\0byte_order\0"
    "set_audio_encoding\0encoding\0set_useRTP\0"
    "use\0set_rtp_connected\0connected\0"
    "set_rtpSession\0RtpSession*\0session\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Audio_playback[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x0a,
       5,    1,   47,    2, 0x0a,
       8,    1,   50,    2, 0x0a,
      10,    1,   53,    2, 0x0a,
      12,    1,   56,    2, 0x0a,
      14,    1,   59,    2, 0x0a,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void, QMetaType::Bool,   11,
    QMetaType::Void, QMetaType::Bool,   13,
    QMetaType::Void, 0x80000000 | 15,   16,

       0        // eod
};

void Audio_playback::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Audio_playback *_t = static_cast<Audio_playback *>(_o);
        switch (_id) {
        case 0: _t->set_decoded_buffer((*reinterpret_cast< QHQueue<qint16>*(*)>(_a[1]))); break;
        case 1: _t->set_audio_byte_order((*reinterpret_cast< QAudioFormat::Endian(*)>(_a[1]))); break;
        case 2: _t->set_audio_encoding((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->set_useRTP((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->set_rtp_connected((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->set_rtpSession((*reinterpret_cast< RtpSession*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAudioFormat::Endian >(); break;
            }
            break;
        }
    }
}

const QMetaObject Audio_playback::staticMetaObject = {
    { &QIODevice::staticMetaObject, qt_meta_stringdata_Audio_playback.data,
      qt_meta_data_Audio_playback,  qt_static_metacall, 0, 0}
};


const QMetaObject *Audio_playback::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Audio_playback::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Audio_playback.stringdata))
        return static_cast<void*>(const_cast< Audio_playback*>(this));
    return QIODevice::qt_metacast(_clname);
}

int Audio_playback::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QIODevice::qt_metacall(_c, _id, _a);
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
struct qt_meta_stringdata_Audio_processing_t {
    QByteArrayData data[13];
    char stringdata[131];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Audio_processing_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Audio_processing_t qt_meta_stringdata_Audio_processing = {
    {
QT_MOC_LITERAL(0, 0, 16),
QT_MOC_LITERAL(1, 17, 13),
QT_MOC_LITERAL(2, 31, 0),
QT_MOC_LITERAL(3, 32, 5),
QT_MOC_LITERAL(4, 38, 6),
QT_MOC_LITERAL(5, 45, 6),
QT_MOC_LITERAL(6, 52, 6),
QT_MOC_LITERAL(7, 59, 9),
QT_MOC_LITERAL(8, 69, 16),
QT_MOC_LITERAL(9, 86, 18),
QT_MOC_LITERAL(10, 105, 1),
QT_MOC_LITERAL(11, 107, 18),
QT_MOC_LITERAL(12, 126, 3)
    },
    "Audio_processing\0process_audio\0\0char*\0"
    "header\0buffer\0length\0set_queue\0"
    "QHQueue<qint16>*\0set_audio_channels\0"
    "c\0set_audio_encoding\0enc\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Audio_processing[] = {

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
       1,    3,   34,    2, 0x0a,
       7,    1,   41,    2, 0x0a,
       9,    1,   44,    2, 0x0a,
      11,    1,   47,    2, 0x0a,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3, QMetaType::Int,    4,    5,    6,
    QMetaType::Void, 0x80000000 | 8,    5,
    QMetaType::Void, QMetaType::Int,   10,
    QMetaType::Void, QMetaType::Int,   12,

       0        // eod
};

void Audio_processing::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Audio_processing *_t = static_cast<Audio_processing *>(_o);
        switch (_id) {
        case 0: _t->process_audio((*reinterpret_cast< char*(*)>(_a[1])),(*reinterpret_cast< char*(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 1: _t->set_queue((*reinterpret_cast< QHQueue<qint16>*(*)>(_a[1]))); break;
        case 2: _t->set_audio_channels((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->set_audio_encoding((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject Audio_processing::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Audio_processing.data,
      qt_meta_data_Audio_processing,  qt_static_metacall, 0, 0}
};


const QMetaObject *Audio_processing::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Audio_processing::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Audio_processing.stringdata))
        return static_cast<void*>(const_cast< Audio_processing*>(this));
    return QObject::qt_metacast(_clname);
}

int Audio_processing::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
struct qt_meta_stringdata_Audio_t {
    QByteArrayData data[32];
    char stringdata[393];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Audio_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Audio_t qt_meta_stringdata_Audio = {
    {
QT_MOC_LITERAL(0, 0, 5),
QT_MOC_LITERAL(1, 6, 30),
QT_MOC_LITERAL(2, 37, 0),
QT_MOC_LITERAL(3, 38, 5),
QT_MOC_LITERAL(4, 44, 6),
QT_MOC_LITERAL(5, 51, 6),
QT_MOC_LITERAL(6, 58, 6),
QT_MOC_LITERAL(7, 65, 12),
QT_MOC_LITERAL(8, 78, 13),
QT_MOC_LITERAL(9, 92, 12),
QT_MOC_LITERAL(10, 105, 16),
QT_MOC_LITERAL(11, 122, 4),
QT_MOC_LITERAL(12, 127, 4),
QT_MOC_LITERAL(13, 132, 8),
QT_MOC_LITERAL(14, 141, 20),
QT_MOC_LITERAL(15, 162, 9),
QT_MOC_LITERAL(16, 172, 13),
QT_MOC_LITERAL(17, 186, 17),
QT_MOC_LITERAL(18, 204, 10),
QT_MOC_LITERAL(19, 215, 8),
QT_MOC_LITERAL(20, 224, 20),
QT_MOC_LITERAL(21, 245, 16),
QT_MOC_LITERAL(22, 262, 17),
QT_MOC_LITERAL(23, 280, 6),
QT_MOC_LITERAL(24, 287, 18),
QT_MOC_LITERAL(25, 306, 3),
QT_MOC_LITERAL(26, 310, 7),
QT_MOC_LITERAL(27, 318, 3),
QT_MOC_LITERAL(28, 322, 17),
QT_MOC_LITERAL(29, 340, 20),
QT_MOC_LITERAL(30, 361, 18),
QT_MOC_LITERAL(31, 380, 11)
    },
    "Audio\0audio_processing_process_audio\0"
    "\0char*\0header\0buffer\0length\0stateChanged\0"
    "QAudio::State\0select_audio\0QAudioDeviceInfo\0"
    "info\0rate\0channels\0QAudioFormat::Endian\0"
    "byteOrder\0process_audio\0get_audio_devices\0"
    "QComboBox*\0comboBox\0clear_decoded_buffer\0"
    "get_audio_device\0QAudioDeviceInfo*\0"
    "device\0set_audio_encoding\0enc\0set_RTP\0"
    "use\0rtp_set_connected\0rtp_set_disconnected\0"
    "rtp_set_rtpSession\0RtpSession*\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Audio[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   74,    2, 0x05,

 // slots: name, argc, parameters, tag, flags
       7,    1,   81,    2, 0x0a,
       9,    4,   84,    2, 0x0a,
      16,    3,   93,    2, 0x0a,
      17,    1,  100,    2, 0x0a,
      20,    0,  103,    2, 0x0a,
      21,    1,  104,    2, 0x0a,
      24,    1,  107,    2, 0x0a,
      26,    1,  110,    2, 0x0a,
      28,    0,  113,    2, 0x0a,
      29,    0,  114,    2, 0x0a,
      30,    1,  115,    2, 0x0a,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3, QMetaType::Int,    4,    5,    6,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 8,    2,
    QMetaType::Void, 0x80000000 | 10, QMetaType::Int, QMetaType::Int, 0x80000000 | 14,   11,   12,   13,   15,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3, QMetaType::Int,    4,    5,    6,
    QMetaType::Void, 0x80000000 | 18,   19,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 22,   23,
    QMetaType::Void, QMetaType::Int,   25,
    QMetaType::Void, QMetaType::Bool,   27,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 31,    2,

       0        // eod
};

void Audio::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Audio *_t = static_cast<Audio *>(_o);
        switch (_id) {
        case 0: _t->audio_processing_process_audio((*reinterpret_cast< char*(*)>(_a[1])),(*reinterpret_cast< char*(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 1: _t->stateChanged((*reinterpret_cast< QAudio::State(*)>(_a[1]))); break;
        case 2: _t->select_audio((*reinterpret_cast< QAudioDeviceInfo(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< QAudioFormat::Endian(*)>(_a[4]))); break;
        case 3: _t->process_audio((*reinterpret_cast< char*(*)>(_a[1])),(*reinterpret_cast< char*(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 4: _t->get_audio_devices((*reinterpret_cast< QComboBox*(*)>(_a[1]))); break;
        case 5: _t->clear_decoded_buffer(); break;
        case 6: _t->get_audio_device((*reinterpret_cast< QAudioDeviceInfo*(*)>(_a[1]))); break;
        case 7: _t->set_audio_encoding((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->set_RTP((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->rtp_set_connected(); break;
        case 10: _t->rtp_set_disconnected(); break;
        case 11: _t->rtp_set_rtpSession((*reinterpret_cast< RtpSession*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAudio::State >(); break;
            }
            break;
        case 2:
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
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QComboBox* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (Audio::*_t)(char * , char * , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Audio::audio_processing_process_audio)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject Audio::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Audio.data,
      qt_meta_data_Audio,  qt_static_metacall, 0, 0}
};


const QMetaObject *Audio::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Audio::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Audio.stringdata))
        return static_cast<void*>(const_cast< Audio*>(this));
    return QObject::qt_metacast(_clname);
}

int Audio::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void Audio::audio_processing_process_audio(char * _t1, char * _t2, int _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
