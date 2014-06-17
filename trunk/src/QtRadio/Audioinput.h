#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H



#include <QObject>
#include <QtCore>
#include <QAudioFormat>
#include <QAudioInput>
#include <QAudioDeviceInfo>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QComboBox>
#else
#include <QComboBox>
#endif

#include <QMutex>
#include <QQueue>

Q_DECLARE_METATYPE(QQueue<qint16>);
Q_DECLARE_METATYPE(QQueue<qint16>*);

class AudioInfo : public QIODevice
{
    Q_OBJECT
public:
    AudioInfo(const QAudioFormat &format, QObject *parent);
    ~AudioInfo();

    void start();
    void stop();

    qreal level() const { return m_level; }
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private:
    const QAudioFormat m_format;
    quint16 m_maxAmplitude;
    qreal m_level; // 0.0 <= m_level <= 1.0
    QQueue<qint16> m_queue;

signals:
    void update(QQueue<qint16>* queue);
};

class AudioInput : public QObject
{
    Q_OBJECT
public:
    AudioInput();
    ~AudioInput();

    void get_audioinput_devices(QComboBox* comboBox);
    int getMicEncoding(void);

signals:
    void mic_update_level(qreal level);
    void mic_send_audio(QQueue<qint16>* queue);

public slots:
    void stateChanged(QAudio::State);
    void select_audio(QAudioDeviceInfo info,int rate,int channels,QAudioFormat::Endian byteOrder);
    void slotMicUpdated(QQueue<qint16>*);
    void setMicEncoding(int encoding);

private:
    QAudioDeviceInfo m_device;
    QAudioFormat m_format;
    QAudioInput *m_audioInput;
    QIODevice *m_input;
    QByteArray m_buffer;

    AudioInfo *m_audioInfo;

    int m_sampleRate;
    int m_channels;
    QAudioFormat::Endian m_byte_order;
    int m_audio_encoding;
};



#endif // AUDIOINPUT_H
