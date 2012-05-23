#include "Audioinput.h"

AudioInput::AudioInput()
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

    m_audioInfo  = new AudioInfo(m_format, this);
    connect(m_audioInfo,SIGNAL(update(QQueue<qint16>*)),this,SLOT(slotMicUpdated(QQueue<qint16>*)));
    m_audioInfo->start();
    m_audioInput->start(m_audioInfo);

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

        if (m_audioInfo != NULL){
        m_audioInfo->stop();
        delete m_audioInfo;
        }
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

    if (m_audioInfo!= NULL){
        m_audioInfo->stop();
        delete m_audioInfo;
    }

    m_format.setFrequency(m_sampleRate);
    m_format.setChannels(m_channels);
    m_format.setByteOrder(m_byte_order);

    if (!m_device.isFormatSupported(m_format)) {
        qDebug()<<"Audio format not supported by Mic input device.";
    }

    m_audioInput = new QAudioInput(m_device, m_format, this);
    connect(m_audioInput,SIGNAL(stateChanged(QAudio::State)),SLOT(stateChanged(QAudio::State)));


    m_audioInfo  = new AudioInfo(m_format, this);
    connect(m_audioInfo,SIGNAL(update(QQueue<qint16>*)),this,SLOT(slotMicUpdated(QQueue<qint16>*)));
    m_audioInfo->start();
    m_audioInput->start(m_audioInfo);

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
        if (m_audioInfo != NULL){
            m_audioInfo->stop();
            delete m_audioInfo;
        }
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

void AudioInput::slotMicUpdated(QQueue<qint16>* queue){
    emit mic_update_level(m_audioInfo->level());
    emit mic_send_audio(queue);
}


int AudioInput::getMicEncoding(){
    return m_audio_encoding;
}

void AudioInput::setMicEncoding(int encoding){
    m_audio_encoding = encoding;
    qDebug() << "Mic encoding changed to " << m_audio_encoding;
}


AudioInfo::AudioInfo(const QAudioFormat &format, QObject *parent)
    :   QIODevice(parent)
    ,   m_format(format)
    ,   m_maxAmplitude(0)
    ,   m_level(0.0)
{
    switch (m_format.sampleSize()) {
    case 8:
        switch (m_format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            m_maxAmplitude = 255;
            break;
        case QAudioFormat::SignedInt:
            m_maxAmplitude = 127;
            break;
        default: ;
        }
        break;
    case 16:
        switch (m_format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            m_maxAmplitude = 65535;
            break;
        case QAudioFormat::SignedInt:
            m_maxAmplitude = 32767;
            break;
        default: ;
        }
        break;
    }
}

AudioInfo::~AudioInfo()
{
}

void AudioInfo::start()
{
    open(QIODevice::WriteOnly);
}

void AudioInfo::stop()
{
    close();
}

qint64 AudioInfo::readData(char *data, qint64 maxlen)
 {
     Q_UNUSED(data)
     Q_UNUSED(maxlen)

     return 0;
 }

 qint64 AudioInfo::writeData(const char *data, qint64 len)
 {
     qint16 v;
     if (m_maxAmplitude) {
         Q_ASSERT(m_format.sampleSize() % 8 == 0);
         const int channelBytes = m_format.sampleSize() / 8;
         const int sampleBytes = m_format.channels() * channelBytes;
         Q_ASSERT(len % sampleBytes == 0);
         const int numSamples = len / sampleBytes;

         quint16 maxValue = 0;
         const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data);

         for (int i = 0; i < numSamples; ++i) {
             for(int j = 0; j < m_format.channels(); ++j) {
                 quint16 value = 0;

                 v = 0;
                 if (m_format.sampleSize() == 8 && m_format.sampleType() == QAudioFormat::UnSignedInt) {
                     value = *reinterpret_cast<const quint8*>(ptr);
                 } else if (m_format.sampleSize() == 8 && m_format.sampleType() == QAudioFormat::SignedInt) {
                     value = qAbs(*reinterpret_cast<const qint8*>(ptr));
                 } else if (m_format.sampleSize() == 16 && m_format.sampleType() == QAudioFormat::UnSignedInt) {
                     if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                         value = qFromLittleEndian<quint16>(ptr);
                     else
                         value = qFromBigEndian<quint16>(ptr);
                 } else if (m_format.sampleSize() == 16 && m_format.sampleType() == QAudioFormat::SignedInt) {
                     if (m_format.byteOrder() == QAudioFormat::LittleEndian){
                        v = qFromLittleEndian<qint16>(ptr);
                        value = qAbs(v);
                     }
                     else {
                        v = qFromBigEndian<qint16>(ptr);
                        value = qAbs(v);
                     }
                 }
                 if (j == 0) m_queue.enqueue(v);            // use only 1st channel
                 maxValue = qMax(value, maxValue);
                 ptr += channelBytes;
             }
         }

         maxValue = qMin(maxValue, m_maxAmplitude);
         m_level = qreal(maxValue) / m_maxAmplitude;
     }
     emit update(&m_queue);
     return len;
 }
