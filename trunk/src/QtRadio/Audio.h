/* 
 * File:   Audio.h
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

#ifndef AUDIO_H
#define	AUDIO_H

#include <QtCore>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QtGui/QComboBox>
#include <QMutex>
#include <samplerate.h>
#include "G711A.h"

#define AUDIO_BUFFER_SIZE 800
#define AUDIO_OUTPUT_BUFFER_SIZE 2048
#define RESAMPLING_BUFFER_SIZE (10600*2)    // 2 channels of 9600 + 10%

#define BIGENDIAN
// There are problems running at 8000 samples per second on Mac OS X
// The resolution is to run at 8011 samples persecond.
#define SAMPLE_RATE_FUDGE 11

class Audio;

class Audio_playback : public QIODevice
{
    Q_OBJECT
public:
    Audio_playback(QObject *parent);
    ~Audio_playback();

    void start();
    void stop();

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private:
    Audio* p;
    int has_more;
signals:
};


class Audio : public QObject {
    Q_OBJECT
public:
    Audio(void * codec2);
//    Audio(const Audio& orig);
    virtual ~Audio();
    int get_audio_encoding();
    QQueue <char> decoded_buffer;
signals:
    void bufferProcessed(void);
public slots:
    void stateChanged(QAudio::State);
    void initialize_audio(int buffer_size);
    void select_audio(QAudioDeviceInfo info,int rate,int channels,QAudioFormat::Endian byteOrder);
    void process_audio(char* header,char* buffer,int length);
    void process_rtp_audio(char* buffer,int length);
    void get_audio_devices(QComboBox* comboBox);
    void set_audio_encoding(int enc);
    void set_src_ratio(double ratio);

private:
    void * codec2;
    void aLawDecode(char* buffer,int length);
    void pcmDecode(char * buffer,int length);
    void codec2Decode(char* buffer, int length);
    void resample(int no_of_samples);
    void init_decodetable();
    QAudioFormat     audio_format;
    QAudioOutput*    audio_output;
    QAudioDeviceInfo audio_device;
    Audio_playback*  audio_out;
    float buffer_in[RESAMPLING_BUFFER_SIZE];
    float buffer_out[RESAMPLING_BUFFER_SIZE];
    short decodetable[256];
    SRC_STATE *sr_state;
    SRC_DATA  sr_data;
    double src_ratio;
    int sampleRate;
    int audio_channels;
    QAudioFormat::Endian audio_byte_order;
    int audio_encoding;
    G711A g711a;
};


#endif	/* AUDIO_H */
