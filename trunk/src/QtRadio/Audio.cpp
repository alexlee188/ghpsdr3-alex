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

#include "Audio.h"
#include "codec2.h"

Audio::Audio(void * codec) {
    int sr_error;
    audio_output=NULL;
    sampleRate=8000;
    audio_encoding = 0;
    audio_channels=1;
    audio_byte_order=QAudioFormat::LittleEndian;

    qDebug() << "Audio: LittleEndian=" << QAudioFormat::LittleEndian << " BigEndian=" << QAudioFormat::BigEndian;

    audio_format.setSampleType(QAudioFormat::SignedInt);
    audio_format.setFrequency(sampleRate+(sampleRate==8000?SAMPLE_RATE_FUDGE:0));
    audio_format.setChannels(audio_channels);
    audio_format.setSampleSize(16);
    audio_format.setCodec("audio/pcm");
    audio_format.setByteOrder(audio_byte_order);
    codec2 = codec;

    src_ratio = 0.99;
    sr_state = src_new (
                         //SRC_SINC_BEST_QUALITY,  // NOT USABLE AT ALL on Atom 300 !!!!!!!
                         SRC_SINC_MEDIUM_QUALITY,
                         //SRC_SINC_FASTEST,
                         //SRC_ZERO_ORDER_HOLD,
                         //SRC_LINEAR,
                         audio_channels, &sr_error
                       ) ;

    if (sr_state == 0) {
        qDebug() <<  "Audio: SR INIT ERROR: " << src_strerror(sr_error);
    } else {
        qDebug() <<  "Audio::audio sample rate init successfully at ratio:" << src_ratio;
    }
}

Audio::~Audio() {
    src_delete(sr_state);
    codec2_destroy(codec2);
}


void Audio::initialize_audio(int buffer_size) {
    qDebug() << "initialize_audio " << buffer_size;

    if ( (buffer_size*6*4) < CODEC2_SAMPLES_PER_FRAME*8)
        decoded_buffer.resize(CODEC2_SAMPLES_PER_FRAME*8); // To cater to 8 frames of codec2
    else decoded_buffer.resize(buffer_size*6*4);  // To cater to 2 channels and 16 bits and 48khz rate

    init_decodetable();
}

void Audio::get_audio_devices(QComboBox* comboBox) {
    int sr_error;
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
    audio_out = audio_output->start();

    if(audio_output->error()!=0) {
        qDebug() << "QAudioOutput: after start error=" << audio_output->error() << " state=" << audio_output->state();

        qDebug() << "Format:";
        qDebug() << "    sample rate: " << audio_format.frequency();
        qDebug() << "    codec: " << audio_format.codec();
        qDebug() << "    byte order: " << audio_format.byteOrder();
        qDebug() << "    sample size: " << audio_format.sampleSize();
        qDebug() << "    sample type: " << audio_format.sampleType();
        qDebug() << "    channels: " << audio_format.channels();
        audio_out = NULL;
        delete audio_output;
    }
    sr_state = src_new (
                         //SRC_SINC_BEST_QUALITY,  // NOT USABLE AT ALL on Atom 300 !!!!!!!
                         SRC_SINC_MEDIUM_QUALITY,
                         //SRC_SINC_FASTEST,
                         //SRC_ZERO_ORDER_HOLD,
                         //SRC_LINEAR,
                         audio_channels, &sr_error
                       ) ;

    if (sr_state == 0) {
        qDebug() <<  "Audio: SR INIT ERROR: " << src_strerror(sr_error);
    } else {
        qDebug() <<  "Audio::get_audio_devices: sample rate init successfully at ratio:" << src_ratio;
    }
}

void Audio::select_audio(QAudioDeviceInfo info,int rate,int channels,QAudioFormat::Endian byteOrder) {
    int sr_error;

    qDebug() << "selected audio " << info.deviceName() <<  " sampleRate:" << rate << " Channels: " << channels << " Endian:" << (byteOrder==QAudioFormat::BigEndian?"BigEndian":"LittleEndian");

    sampleRate=rate;
    audio_channels=channels;
    audio_byte_order=byteOrder;

    if(audio_output!=NULL) {
        audio_output->stop();
        audio_output->disconnect(this);
        delete audio_output;
    }

    if(sr_state != NULL) src_delete(sr_state);

    audio_device=info;
    audio_format.setFrequency(sampleRate+(sampleRate==8000?SAMPLE_RATE_FUDGE:0));
    audio_format.setChannels(audio_channels);
    audio_format.setByteOrder(audio_byte_order);

    if (!audio_device.isFormatSupported(audio_format)) {
        qDebug()<<"Audio format not supported by device.";
    }

    audio_output = new QAudioOutput(audio_device, audio_format, this);
    connect(audio_output,SIGNAL(stateChanged(QAudio::State)),SLOT(stateChanged(QAudio::State)));

    audio_output->setBufferSize(AUDIO_OUTPUT_BUFFER_SIZE);

    audio_out = audio_output->start();

    if(audio_output->error()!=0) {
        qDebug() << "QAudioOutput: after start error=" << audio_output->error() << " state=" << audio_output->state();

        qDebug() << "Format:";
        qDebug() << "    sample rate: " << audio_format.frequency();
        qDebug() << "    codec: " << audio_format.codec();
        qDebug() << "    byte order: " << audio_format.byteOrder();
        qDebug() << "    sample size: " << audio_format.sampleSize();
        qDebug() << "    sample type: " << audio_format.sampleType();
        qDebug() << "    channels: " << audio_format.channels();
        audio_out = NULL;
    }

    sr_state = src_new (
                         //SRC_SINC_BEST_QUALITY,  // NOT USABLE AT ALL on Atom 300 !!!!!!!
                         SRC_SINC_MEDIUM_QUALITY,
                         //SRC_SINC_FASTEST,
                         //SRC_ZERO_ORDER_HOLD,
                         //SRC_LINEAR,
                         audio_channels, &sr_error
                       ) ;

    if (sr_state == 0) {
        qDebug() <<  "Audio: SR INIT ERROR: " << src_strerror(sr_error);
    } else {
        qDebug() <<  "Audio:select_audio: sample rate init successfully at ratio:" << src_ratio;
    }
}

void Audio::stateChanged(QAudio::State State){
    switch (State) {
        case QAudio::StoppedState:
            if (audio_output->error() != QAudio::NoError) {
                qDebug() << "QAudioOutput: after start error=" << audio_output->error() << " state=" << State;
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

void Audio::set_audio_encoding(int enc){
    audio_encoding = enc;
}

int Audio::get_audio_encoding() {
    return audio_encoding;
}

void Audio::process_audio(char* header,char* buffer,int length) {
    //qDebug() << "process audio";
    int written=0;
    int length_to_write, total_to_write;

    if (audio_encoding == 0) aLawDecode(buffer,length);
    else if (audio_encoding == 1) pcmDecode(buffer,length);
    else if (audio_encoding == 2) codec2Decode(buffer,length);
    else {
        qDebug() << "Error: Audio::process_audio:  audio_encoding = " << audio_encoding;
    }

    if(audio_out!=NULL) {
        //qDebug() << "writing audio data length=: " <<  decoded_buffer.length();
        total_to_write = decoded_buffer.length();
        while( written< total_to_write) {
            if (audio_output->bytesFree() < 4) usleep(1000);
            // writing in periodsize is recommended
            length_to_write = (audio_output->periodSize() > (decoded_buffer.length()-written)) ?
                        (decoded_buffer.length()-written) : audio_output->periodSize();
            written+=audio_out->write(&decoded_buffer.data()[written],length_to_write);
        }
    }

    if (header != NULL) free(header);
    if (buffer != NULL) free(buffer);
}

void Audio::resample(int no_of_samples){
    int i;
    short v;
    int rc;

    decoded_buffer.clear();

    sr_data.data_in = buffer_in;
    sr_data.data_out = buffer_out;
    sr_data.input_frames = no_of_samples;
    sr_data.src_ratio = src_ratio;
    sr_data.output_frames = 1600*6;
    sr_data.end_of_input = 0;

    rc = src_process(sr_state, &sr_data);
    if (rc) qDebug() << "SRATE: error: " << src_strerror (rc) << rc;
    else {
        for (i = 0; i < sr_data.output_frames_gen; i++){
            v = buffer_out[i]*32767.0;
            switch(audio_byte_order) {
            case QAudioFormat::LittleEndian:
                decoded_buffer.append((char)(v&0xFF));
                decoded_buffer.append((char)((v>>8)&0xFF));
                break;
            case QAudioFormat::BigEndian:
                decoded_buffer.append((char)((v>>8)&0xFF));
                decoded_buffer.append((char)(v&0xFF));
                break;
            }
        }
    }

}

void Audio::aLawDecode(char* buffer,int length) {
    int i;
    short v;

    for (i=0; i < length; i++) {
        v=decodetable[buffer[i]&0xFF];
        buffer_in[i] = (float)v / 32767.0;
    }

    resample(length);

}

void Audio::pcmDecode(char* buffer,int length) {
    int i;
    short v;

    for (i=0; i < length; i+=2) {
        v = (buffer[i] & 0xff) | ((buffer[i+1] & 0xff) << 8);
        buffer_in[i/2] = v / 32767.0;
        }
    resample(length/2);

}

void Audio::codec2Decode(char* buffer,int length) {
    int i,j,k;
    short v[CODEC2_SAMPLES_PER_FRAME];
    unsigned char bits[BITS_SIZE];

    j = 0;
    k = 0;
    while (j < length) {
        memcpy(bits,&buffer[j],BITS_SIZE);
        codec2_decode(codec2, v, bits);

        for (i=0; i < CODEC2_SAMPLES_PER_FRAME; i++){
            buffer_in[i+k*CODEC2_SAMPLES_PER_FRAME]= v[i]/ 32767.0;
        }
        j += BITS_SIZE;
        k++;
    }
    resample(k*CODEC2_SAMPLES_PER_FRAME);
}

void Audio::init_decodetable() {
    qDebug() << "init_decodetable";
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
