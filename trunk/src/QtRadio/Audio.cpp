/*
 * File:   Audio.cpp
 * Author: John Melton, G0ORX/N6LYT
 * 
 * Created on 16 August 2010, 11:19
 */

/* Copyright (C)
* 2009 - John Melton, G0ORX/N6LYT
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/


/* Copyright (C) 2012 - Alex Lee, 9V1Al
* modifications of the original program by John Melton
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Pl
*/

#include <ortp/rtp.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#include "Audio.h"
#include "codec2.h"


Audio_playback::Audio_playback()
    :   QIODevice()
{
    recv_ts = 0;
    audio_byte_order = QAudioFormat::LittleEndian;
    audio_encoding = 0;
    rtpSession = 0;
    rtp_connected = false;
    useRTP = false;
    pdecoded_buffer = &queue;
}

Audio_playback::~Audio_playback()
{
}

void Audio_playback::start()
{
    open(QIODevice::ReadOnly);
}

void Audio_playback::stop()
{
    close();
}

void Audio_playback::set_decoded_buffer(QHQueue<qint16> *pBuffer){
    pdecoded_buffer = pBuffer;
}

void Audio_playback::set_audio_byte_order(QAudioFormat::Endian byte_order){
    audio_byte_order = byte_order;
}

void Audio_playback::set_audio_encoding(int encoding){
    audio_encoding = encoding;
}

void Audio_playback::set_rtpSession(RtpSession *session){
    rtpSession = session;
}

void Audio_playback::set_rtp_connected(bool connected){
    rtp_connected = connected;
}

void Audio_playback::set_useRTP(bool use){
    useRTP = use;
}

qint64 Audio_playback::readData(char *data, qint64 maxlen)
 {
   qint64 bytes_read;
   qint16 v;
   qint64 bytes_to_read = maxlen > 2000 ? 2000: maxlen;
   //qint64 bytes_to_read = maxlen;
   int has_more;

   if (useRTP && rtp_connected){
       int i;
       short v;
       int length;

       uint8_t* buffer;

       // aLaw encoded from RTP, but readData is reading in bytes (each sample 2 bytes for PCM)
       buffer = (uint8_t*)malloc(bytes_to_read/2);

       length=rtp_session_recv_with_ts(rtpSession,buffer,bytes_to_read/2,recv_ts,&has_more);
       if (length>0) {
           recv_ts += length;
           if (audio_encoding == 0) {
                #pragma omp parallel for schedule(static) ordered
                for(i=0;i<length;i++) {
                    v=g711a.decode(buffer[i]);
                    #pragma omp ordered
                    pdecoded_buffer->enqueue(v);
               }
           } else {
               qDebug() << "Audio: rtp_audio only support aLaw";
           }
       }
       free(buffer);

       if (length <= 0){                  // probably not connected or late arrival.  Send silence.
           recv_ts+=bytes_to_read/2;
           memset(data, 0, bytes_to_read);
           bytes_read = bytes_to_read;
           return bytes_read;
        }

   }

   // note both TCP and RTP audio enqueue PCM data in decoded_buffer
   bytes_read = 0;

   if (pdecoded_buffer->isEmpty()) {
       // probably not connected or late arrival of packets.  Send silence.
       memset(data, 0, bytes_to_read);
       bytes_read = bytes_to_read;
   } else {
       while ((!pdecoded_buffer->isEmpty()) && (bytes_read < bytes_to_read)){
           v = pdecoded_buffer->dequeue();
            switch(audio_byte_order) {
            case QAudioFormat::LittleEndian:
                data[bytes_read++]=(char)(v&0xFF);
                data[bytes_read++]=(char)((v>>8)&0xFF);
                break;
            case QAudioFormat::BigEndian:
                data[bytes_read++]=(char)((v>>8)&0xFF);
                data[bytes_read++]=(char)(v&0xFF);
                break;
            }
        }
       while (bytes_read < bytes_to_read) data[bytes_read++] = 0;
   }

   return bytes_read;
 }

 qint64 Audio_playback::writeData(const char *data, qint64 len){
     Q_UNUSED(data);
     Q_UNUSED(len);
     return 0;
 }

Audio::Audio() {
    audio_output=NULL;
    sampleRate=8000;
    audio_encoding = 0;
    audio_channels=1;
    audio_byte_order=QAudioFormat::LittleEndian;
    rtp_connected = false;
    useRTP = false;

    qDebug() << "Audio: LittleEndian=" << QAudioFormat::LittleEndian << " BigEndian=" << QAudioFormat::BigEndian;

    audio_format.setSampleType(QAudioFormat::SignedInt);

#if QT_VERSION >= 0x050000
    audio_format.setSampleRate(sampleRate+(sampleRate==8000?SAMPLE_RATE_FUDGE:0));
    audio_format.setChannelCount(audio_channels);
#else
    audio_format.setFrequency(sampleRate+(sampleRate==8000?SAMPLE_RATE_FUDGE:0));
    audio_format.setChannels(audio_channels);
#endif

    audio_format.setSampleSize(16);
    audio_format.setCodec("audio/pcm");
    audio_format.setByteOrder(audio_byte_order);

    audio_processing = new Audio_processing;
    audio_processing->set_audio_channels(audio_channels);
    audio_processing->set_audio_encoding(audio_encoding);
    audio_processing->set_queue(&decoded_buffer);
    audio_processing_thread = new QThread;
    qDebug() << "QThread: audio_processing_thread = " << audio_processing_thread;
    audio_processing->moveToThread(audio_processing_thread);
    connect(this,SIGNAL(audio_processing_process_audio(char*,char*,int)),audio_processing,SLOT(process_audio(char*,char*,int)));
    audio_processing_thread->start(QThread::LowPriority);

    audio_output_thread = new QThread;
    qDebug() << "QThread:  audio_output_thread = " << audio_output_thread;
}

Audio::~Audio() {
    disconnect(this,SIGNAL(audio_processing_process_audio(char*,char*,int)),audio_processing,SLOT(process_audio(char*,char*,int)));
    audio_processing->deleteLater();
}

void Audio::clear_decoded_buffer(void){
    decoded_buffer.clear();
}

void Audio::get_audio_device(QAudioDeviceInfo * device){
	*device = audio_device;
}

void Audio::get_audio_devices(QComboBox* comboBox) {
    QList<QAudioDeviceInfo> devices=QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    QAudioDeviceInfo device_info;


    qDebug() << "Audio::get_audio_devices";
    for(int i=0;i<devices.length();i++) {
        device_info=devices.at(i);

        qDebug() << "Audio::get_audio_devices: " << device_info.deviceName();

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

#if QT_VERSION >= 0x050000
        QList<int> sampleRates=device_info.supportedSampleRates();
#else
        QList<int> sampleRates=device_info.supportedFrequencies();
#endif

        for(int j=0;j<sampleRates.size();j++) {
            qDebug() << "        " << sampleRates.at(j);
        }

        qDebug() << "    Sample Sizes";
        QList<int> sampleSizes=device_info.supportedSampleSizes();
        for(int j=0;j<sampleSizes.size();j++) {
            qDebug() << "        " << sampleSizes.at(j);
        }


        qDebug() << "    Channels";

#if QT_VERSION >= 0x050000
        QList<int> channels=device_info.supportedChannelCounts();
#else
         QList<int> channels=device_info.supportedChannels();
#endif

        for(int j=0;j<channels.size();j++) {
            qDebug() << "        " << channels.at(j);
        }

        comboBox->addItem(device_info.deviceName(),qVariantFromValue(device_info));
        if(i==0) {
            audio_device=device_info;
        }
        i++;
    }

    qDebug() << "Audio::get_audio_devices: default is " << audio_device.deviceName();

    audio_output = new QAudioOutput(audio_device, audio_format, this);
    connect(audio_output,SIGNAL(stateChanged(QAudio::State)),SLOT(stateChanged(QAudio::State)));

    qDebug() << "QAudioOutput: error=" << audio_output->error() << " state=" << audio_output->state();

    audio_output->setBufferSize(AUDIO_OUTPUT_BUFFER_SIZE);
    audio_out= new Audio_playback;
    audio_out->moveToThread(audio_output_thread);
    audio_output_thread->start(QThread::HighestPriority);

    audio_out->set_audio_byte_order(audio_format.byteOrder());
    audio_out->set_audio_encoding(audio_encoding);
    audio_out->set_decoded_buffer(&decoded_buffer);
    audio_out->set_rtpSession(0);
    audio_out->set_rtp_connected(false);
    audio_out->set_useRTP(false);
    audio_out->start();
    audio_output->start(audio_out);


#if QT_VERSION >= 0x050000
     audio_processing->set_audio_channels(audio_format.channelCount());
#else
    audio_processing->set_audio_channels(audio_format.channels());
#endif

    audio_processing->set_audio_encoding(audio_encoding);
    audio_processing->set_queue(&decoded_buffer);

    if(audio_output->error()!=0) {
        qDebug() << "QAudioOutput: after start error=" << audio_output->error() << " state=" << audio_output->state();

        qDebug() << "Format:";

#if QT_VERSION >= 0x050000
        qDebug() << "    sample rate: " << audio_format.sampleRate();
#else
        qDebug() << "    sample rate: " << audio_format.frequency();
#endif

        qDebug() << "    codec: " << audio_format.codec();
        qDebug() << "    byte order: " << audio_format.byteOrder();
        qDebug() << "    sample size: " << audio_format.sampleSize();
        qDebug() << "    sample type: " << audio_format.sampleType();

#if QT_VERSION >= 0x050000
        qDebug() << "    channels: " << audio_format.channelCount();
#else
        qDebug() << "    channels: " << audio_format.channels();
#endif

        audio_out->stop();
        delete audio_out;
        delete audio_output;
    }


}

void Audio::select_audio(QAudioDeviceInfo info,int rate,int channels,QAudioFormat::Endian byteOrder) {

    qDebug() << "selected audio " << info.deviceName() <<  " sampleRate:" << rate << " Channels: " << channels << " Endian:" << (byteOrder==QAudioFormat::BigEndian?"BigEndian":"LittleEndian");

    sampleRate=rate;
    audio_channels=channels;
    audio_byte_order=byteOrder;

    if(audio_output!=NULL) {
        audio_out->stop();
        delete audio_out;
        delete audio_output;
    }

    audio_device=info;

#if QT_VERSION >= 0x050000
    audio_format.setSampleRate(sampleRate+(sampleRate==8000?SAMPLE_RATE_FUDGE:0));
    audio_format.setChannelCount(audio_channels);
#else
    audio_format.setFrequency(sampleRate+(sampleRate==8000?SAMPLE_RATE_FUDGE:0));
    audio_format.setChannels(audio_channels);
#endif

    audio_format.setByteOrder(audio_byte_order);

    if (!audio_device.isFormatSupported(audio_format)) {
        qDebug()<<"Audio format not supported by device.";
    }

    audio_output = new QAudioOutput(audio_device, audio_format, this);
    connect(audio_output,SIGNAL(stateChanged(QAudio::State)),SLOT(stateChanged(QAudio::State)));

    audio_output->setBufferSize(AUDIO_OUTPUT_BUFFER_SIZE);
    audio_out= new Audio_playback;

    audio_out->moveToThread(audio_output_thread);
    audio_output_thread->start(QThread::HighestPriority);

    audio_out->set_audio_byte_order(audio_format.byteOrder());
    audio_out->set_audio_encoding(audio_encoding);
    audio_out->set_decoded_buffer(&decoded_buffer);
    audio_out->set_rtpSession(0);
    audio_out->set_rtp_connected(false);
    audio_out->set_useRTP(false);
    audio_out->start();
    audio_output->start(audio_out);


#if QT_VERSION >= 0x050000
     audio_processing->set_audio_channels(audio_format.channelCount());
#else
    audio_processing->set_audio_channels(audio_format.channels());
#endif

    audio_processing->set_audio_encoding(audio_encoding);
    audio_processing->set_queue(&decoded_buffer);

    if(audio_output->error()!=0) {
        qDebug() << "QAudioOutput: after start error=" << audio_output->error() << " state=" << audio_output->state();
        qDebug() << "Format:";

#if QT_VERSION >= 0x050000
        qDebug() << "    sample rate: " << audio_format.sampleRate();
#else
        qDebug() << "    sample rate: " << audio_format.frequency();
#endif

        qDebug() << "    codec: " << audio_format.codec();
        qDebug() << "    byte order: " << audio_format.byteOrder();
        qDebug() << "    sample size: " << audio_format.sampleSize();
        qDebug() << "    sample type: " << audio_format.sampleType();

#if QT_VERSION >= 0x050000
        qDebug() << "    channels: " << audio_format.channelCount();
#else
        qDebug() << "    channels: " << audio_format.channels();
#endif
        audio_out->stop();
        delete  audio_out;
        delete audio_output;

    }

}

void Audio::stateChanged(QAudio::State State){
    switch (State) {
        case QAudio::StoppedState:
	case QAudio::SuspendedState:
            if (audio_output->error() != QAudio::NoError) {
                qDebug() << "QAudioOutput: after start error=" << audio_output->error() << " state=" << State;
            //audio_output->start(audio_out);
            break;
            }
        case QAudio::IdleState:
        case QAudio::ActiveState:
        default:
 //           qDebug() << "QAudioOutput: state changed" << " state=" << State;
	    break;
    }
    return;
}

void Audio::set_audio_encoding(int enc){
    audio_encoding = enc;
    audio_out->set_audio_encoding(enc);
    audio_processing->set_audio_encoding(enc);
}

int Audio::get_audio_encoding() {
    return audio_encoding;
}

void Audio::process_audio(char* header, char* buffer, int length){
    emit audio_processing_process_audio(header,buffer,length);
}

void Audio::set_RTP(bool use){
    useRTP = use;
    audio_out->set_useRTP(use);
}

void Audio::rtp_set_connected(void){
    qDebug() << "Audio::rtp_set_connected";
#if 0
    /*
     *  send first packet in order to help to establish session
     */
    unsigned char fake [] = "BBBBBBBBBBBBBBBB";
    rtp_session_send_with_ts(rtpSession,(uint8_t*)fake,sizeof(fake),1);
#endif
    rtp_connected = true;
    audio_out->set_rtp_connected(true);
}

void Audio::rtp_set_disconnected(void){
    rtp_connected = false;
    audio_out->set_rtp_connected(false);
}

void Audio::rtp_set_rtpSession(RtpSession* session){
    qDebug() << "Audio::rtp_set_rtpSession";
    rtpSession = session;
    audio_out->set_rtpSession(session);
}

Audio_processing::Audio_processing(){
    int sr_error;

    src_state =  src_new (
                //SRC_SINC_BEST_QUALITY,  // NOT USABLE AT ALL on Atom 300 !!!!!!!
                //SRC_SINC_MEDIUM_QUALITY,
                SRC_SINC_FASTEST,
                //SRC_ZERO_ORDER_HOLD,
                //SRC_LINEAR,
                1, &sr_error
              ) ;

    if (src_state == 0) {
        qDebug() <<  "Audio: SR INIT ERROR: " << src_strerror(sr_error);
    }
    init_decodetable();
    src_ratio = 1.0;
    codec2 = codec2_create(CODEC2_MODE_3200);
    pdecoded_buffer = &queue;
}

Audio_processing::~Audio_processing(){
     src_delete(src_state);
     codec2_destroy(codec2);
}

void Audio_processing::set_queue(QHQueue<qint16> *buffer){
    pdecoded_buffer = buffer;
}

void Audio_processing::set_audio_channels(int c){
    audio_channels = c;
    int sr_error;

    src_delete(src_state);
    src_state =  src_new (
                //SRC_SINC_BEST_QUALITY,  // NOT USABLE AT ALL on Atom 300 !!!!!!!
                //SRC_SINC_MEDIUM_QUALITY,
                SRC_SINC_FASTEST,
                //SRC_ZERO_ORDER_HOLD,
                //SRC_LINEAR,
                c, &sr_error
              ) ;

    if (src_state == 0) {
        qDebug() <<  "Audio: SR INIT ERROR: " << src_strerror(sr_error);
    }
}

void Audio_processing::set_audio_encoding(int enc){
    audio_encoding = enc;
}

void Audio_processing::process_audio(char* header,char* buffer,int length) {

    if (pdecoded_buffer->count() < 4000){
        if (audio_encoding == 0) aLawDecode(buffer,length);
        else if (audio_encoding == 1) pcmDecode(buffer,length);
        else if (audio_encoding == 2) codec2Decode(buffer,length);
        else {
            qDebug() << "Error: Audio::process_audio:  audio_encoding = " << audio_encoding;
        }
    }
    if (header != NULL) free(header);
    if (buffer != NULL) free(buffer);
}

void Audio_processing::resample(int no_of_samples){
    int i;
    qint16 v;
    int rc;

    sr_data.data_in = buffer_in;
    sr_data.data_out = buffer_out;
    sr_data.input_frames = no_of_samples;
    sr_data.src_ratio = src_ratio;
    sr_data.output_frames = RESAMPLING_BUFFER_SIZE/2;
    sr_data.end_of_input = 0;

    rc = src_process(src_state, &sr_data);
    if (rc) qDebug() << "SRATE: error: " << src_strerror (rc) << rc;
    else {
            #pragma omp parallel for schedule(static) ordered
            for (i = 0; i < sr_data.output_frames_gen; i++){
                v = buffer_out[i]*32767.0;
                #pragma omp ordered
                pdecoded_buffer->enqueue(v);
            }
    }
}

void Audio_processing::aLawDecode(char* buffer,int length) {
    int i;
    qint16 v;

    for (i=0; i < length; i++) {
        v=decodetable[buffer[i]&0xFF];
        //buffer_in[i] = (float)v/32767.0f;
        pdecoded_buffer->enqueue(v);
    }
    //resample(length);
}

void Audio_processing::pcmDecode(char* buffer,int length) {
    int i;
    short v;

    for (i=0; i < length; i+=2) {
        v = (buffer[i] & 0xff) | ((buffer[i+1] & 0xff) << 8);
        //buffer_in[i/2] = (float)v/32767.0f;
        pdecoded_buffer->enqueue(v);
        }
    //resample(length/2);
}

void Audio_processing::codec2Decode(char* buffer,int length) {
    int i,j,k;
    int samples_per_frame = codec2_samples_per_frame(codec2);
    short v[samples_per_frame];
    int bits_size = codec2_bits_per_frame(codec2)/8;
    unsigned char bits[bits_size];

    j = 0;
    k = 0;
    while (j < length) {
        memcpy(bits,&buffer[j],bits_size);
        codec2_decode(codec2, v, bits);
        for (i=0; i < samples_per_frame; i++){
            //buffer_in[i] = (float)v[i]/32767.0f;
            pdecoded_buffer->enqueue(v[i]);
        }
        j += bits_size;
        k++;
    }
    //resample(samples_per_frame);
}

void Audio_processing::init_decodetable() {
    qDebug() << "init_decodetable";
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < 256; i++) {
        int input = i ^ 85;
        int mantissa = (input & 15) << 4;
        int segment = (input & 112) >> 4;
        int value = mantissa + 8;
        if (segment >= 1) {
            value += 256;
        }
        if (segment > 1) {
            value <<= (segment - 1);
        }
        if ((input & 128) == 0) {
            value = -value;
        }
        decodetable[i] = (short) value;
    }
}
