#include "Audioinput.h"

AudioInput::AudioInput(QObject *parent, void *codec) :
    QObject(parent)
{
    m_audioInput=NULL;
    m_sampleRate=8000;
    m_audio_encoding = 0;
    m_byte_order=QAudioFormat::LittleEndian;

    m_format.setSampleType(QAudioFormat::SignedInt);
    m_format.setFrequency(m_sampleRate);
    m_format.setChannels(1);
    m_format.setSampleSize(16);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(m_byte_order);
    codec2 = codec;                         // pointer to codec2 state
                                            // codec2 lib is probably NOT re-entrant
                                            // so make sure only one thread is actively using it
}

AudioInput::~AudioInput()
{

}

void AudioInput::get_audioinput_devices(QComboBox* comboBox) {

    QList<QAudioDeviceInfo> devices=QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    QAudioDeviceInfo device_info;

    qDebug() << "Audio::get_audioinput_devices";
    for(int i=0;i<devices.length();i++) {
        device_info=devices.at(i);
        qDebug() << "Audio::get_audioinput_devices: " << device_info.deviceName();

        qDebug() << "    Codecs:";
        QStringList codecs=device_info.supportedCodecs();
        for(int j=0;j<codecs.size();j++) {
            qDebug() << "        " << codecs.at(j).toLocal8Bit().constData();
        }

        qDebug() << "    Byte Orders";
        QList<QAudioFormat::Endian> byteOrders=device_info.supportedByteOrders();
        for(int j=0;j<byteOrders.size();j++) {
            qDebug() << "        " << (byteOrders.at(j)==QAudioFormat::BigEndian?"BigEndian":"LittleEndian");
        }

        qDebug() << "    Sample Type";
        QList<QAudioFormat::SampleType> sampleTypes=device_info.supportedSampleTypes();
        for(int j=0;j<sampleTypes.size();j++) {
            if(sampleTypes.at(j)==QAudioFormat::Unknown) {
                qDebug() << "        Unknown";
            } else if(sampleTypes.at(j)==QAudioFormat::SignedInt) {
                qDebug() << "        SignedInt";
            } else if(sampleTypes.at(j)==QAudioFormat::UnSignedInt) {
                qDebug() << "        UnSignedInt";
            } else if(sampleTypes.at(j)==QAudioFormat::Float) {
                qDebug() << "        Float";
            }
        }

        qDebug() << "    Sample Rates";
        QList<int> sampleRates=device_info.supportedFrequencies();
        for(int j=0;j<sampleRates.size();j++) {
            qDebug() << "        " << sampleRates.at(j);
        }

        qDebug() << "    Sample Sizes";
        QList<int> sampleSizes=device_info.supportedSampleSizes();
        for(int j=0;j<sampleSizes.size();j++) {
            qDebug() << "        " << sampleSizes.at(j);
        }

        qDebug() << "    Channels";
        QList<int> channels=device_info.supportedChannels();
        for(int j=0;j<channels.size();j++) {
            qDebug() << "        " << channels.at(j);
        }

        if (comboBox != NULL) comboBox->addItem(device_info.deviceName(),qVariantFromValue(device_info));
        if(i==0) {
            m_device=device_info;
        }
        i++;
    }

    qDebug() << "Audioinput::get_audio_devices: default is " << m_device.deviceName();

    m_audioInput = new QAudioInput(m_device, m_format, this);
    connect(m_audioInput,SIGNAL(stateChanged(QAudio::State)),SLOT(stateChanged(QAudio::State)));

    qDebug() << "QAudioOutput: error=" << m_audioInput->error() << " state=" << m_audioInput->state();

    m_input = m_audioInput->start();

    if(m_audioInput->error()!=0) {
        qDebug() << "QAudioInput: after start error=" << m_audioInput->error() << " state=" << m_audioInput->state();

        qDebug() << "Format:";
        qDebug() << "    sample rate: " << m_format.frequency();
        qDebug() << "    codec: " << m_format.codec();
        qDebug() << "    byte order: " << m_format.byteOrder();
        qDebug() << "    sample size: " << m_format.sampleSize();
        qDebug() << "    sample type: " << m_format.sampleType();
        qDebug() << "    channels: " << m_format.channels();
        m_input = NULL;
        delete m_audioInput;
    }
}

void AudioInput::select_audio(QAudioDeviceInfo info, int rate, int channels, QAudioFormat::Endian byteOrder){
    m_device = info;
    m_sampleRate = rate;
    m_channels = channels;
    m_byte_order = byteOrder;

    if(m_audioInput!=NULL) {
        m_audioInput->stop();
        m_audioInput->disconnect(this);
        delete m_audioInput;
    }

    m_format.setFrequency(m_sampleRate);
    m_format.setChannels(m_channels);
    m_format.setByteOrder(m_byte_order);

    if (!m_device.isFormatSupported(m_format)) {
        qDebug()<<"Audio format not supported by Mic input device.";
    }

    m_audioInput = new QAudioInput(m_device, m_format, this);
    connect(m_audioInput,SIGNAL(stateChanged(QAudio::State)),SLOT(stateChanged(QAudio::State)));

    m_input = m_audioInput->start();

    if(m_audioInput->error()!=0) {
        qDebug() << "QAudioInput: after start error=" << m_audioInput->error() << " state=" << m_audioInput->state();

        qDebug() << "Format:";
        qDebug() << "    sample rate: " << m_format.frequency();
        qDebug() << "    codec: " << m_format.codec();
        qDebug() << "    byte order: " << m_format.byteOrder();
        qDebug() << "    sample size: " << m_format.sampleSize();
        qDebug() << "    sample type: " << m_format.sampleType();
        qDebug() << "    channels: " << m_format.channels();
        m_input = NULL;
        delete m_audioInput;
    }
}

void AudioInput::stateChanged(QAudio::State State){
    switch (State) {
        case QAudio::StoppedState:
            if (m_audioInput->error() != QAudio::NoError) {
                qDebug() << "QAudioOutput: after start error=" << m_audioInput->error() << " state=" << State;
            break;
            }
        case QAudio::IdleState:
        case QAudio::SuspendedState:
        case QAudio::ActiveState:
        default:
 //           qDebug() << "QAudioOutput: state changed" << " state=" << State;
        return;
    }
}
