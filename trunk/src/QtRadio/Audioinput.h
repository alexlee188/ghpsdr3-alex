#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H

#include <QObject>
#include <QtCore>
#include <QAudioFormat>
#include <QAudioInput>
#include <QAudioDeviceInfo>
#include <QtGui/QComboBox>
#include <QMutex>

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

signals:
    void update();
};

class AudioInput : public QObject
{
    Q_OBJECT
public:
    explicit AudioInput(QObject *parent = 0, void * codec = 0);
    virtual ~AudioInput();
    void get_audioinput_devices(QComboBox* comboBox);
signals:
public slots:
    void stateChanged(QAudio::State);
    void select_audio(QAudioDeviceInfo info,int rate,int channels,QAudioFormat::Endian byteOrder);
    void slotMicUpdated();
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

    void *codec2;
};



#endif // AUDIOINPUT_H
