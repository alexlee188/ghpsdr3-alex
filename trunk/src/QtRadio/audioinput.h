#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H

#include <QObject>
#include <QtCore>
#include <QAudioFormat>
#include <QAudioInput>
#include <QAudioDeviceInfo>
#include <QtGui/QComboBox>
#include <QMutex>

class AudioInput : public QObject
{
    Q_OBJECT
public:
    explicit AudioInput(QObject *parent = 0);
    virtual ~AudioInput();
    void get_audioinput_devices(QComboBox* comboBox);
signals:

public slots:
    void stateChanged(QAudio::State);
private:
    QAudioDeviceInfo m_device;
    QAudioFormat m_format;
    QAudioInput *m_audioInput;
    QIODevice *m_input;
    bool m_pullMode;
    QByteArray m_buffer;

    int sampleRate;
    QAudioFormat::Endian audio_byte_order;
    int audio_encoding;

    static const QString PushModeLabel;
    static const QString PullModeLabel;
    static const QString SuspendLabel;
    static const QString ResumeLabel;
};

#endif // AUDIOINPUT_H
