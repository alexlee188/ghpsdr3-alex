/*
 * File:   Configure.h
 * Author: John Melton, G0ORX/N6LYT
 *
 * Created on 16 August 2010, 20:03
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

#ifndef _CONFIGURE_H
#define	_CONFIGURE_H

#include <QSettings>
#include <QDebug>
#include <QAudioFormat>
#include <QAudioDeviceInfo>

#include "ui_Configure.h"

#include "Audio.h"
#include "Xvtr.h"
#include "Audioinput.h"

class Configure : public QDialog {
    Q_OBJECT
public:
    Configure();
    virtual ~Configure();
    void initAudioDevices(Audio* audio);
    void initMicDevices(AudioInput* audioinput);
    void initXvtr(Xvtr* xvtr);
    void loadSettings(QSettings* settings);
    void saveSettings(QSettings* settings);

    void connected(bool state);
    void updateXvtrList(Xvtr* xvtr);

    QString getHost();
    void addHost(QString host);
    void removeHost(QString host);
    int getReceiver();
    
    int getSpectrumHigh();
    int getSpectrumLow();
    int getFps();

    void setSpectrumHigh(int high);
    void setSpectrumLow(int low);

    int getWaterfallHigh();
    int getWaterfallLow();

    void setWaterfallHigh(int high);
    void setWaterfallLow(int low);

    QAudioFormat::Endian getByteOrder();
    int getEncoding();
    int getSampleRate();
    int getChannels();

    void setSampleRate(int samplerate);
    void setChannels(int channels);

    bool getRTP();
    
    int getNrTaps();
    int getNrDelay();
    double getNrGain();
    double getNrLeak();

    int getAnfTaps();
    int getAnfDelay();
    double getAnfGain();
    double getAnfLeak();

    double getNbThreshold();
    double getSdromThreshold();
    bool getGeometryState();
    bool getTxAllowed();
    void setTxAllowed(bool newstate);
    bool setPasswd(QString ServerName);
    QString thisuser;
    QString thispass;
    bool getRxIQcheckboxState();
    double getRxIQspinBoxValue();
    int getCwPitch();

signals:
    void hostChanged(QString host);
    void receiverChanged(int receiver);
    void spectrumHighChanged(int high);
    void spectrumLowChanged(int low);
    void fpsChanged(int fps);
    void waterfallHighChanged(int high);
    void waterfallLowChanged(int low);
    void waterfallAutomaticChanged(bool state);
    void audioDeviceChanged(QAudioDeviceInfo info,int rate,int channels,QAudioFormat::Endian order);
    void micDeviceChanged(QAudioDeviceInfo info,int rate,int channels,QAudioFormat::Endian order);
    void encodingChanged(int index);
    void get_audio_devices(QComboBox* comboBox);
    void micDeviceChanged(QAudioDeviceInfo info);

    void useRTP(bool state);

    void nrValuesChanged(int taps,int delay,double gain,double leak);
    void anfValuesChanged(int taps,int delay,double gain,double leak);

    void nbThresholdChanged(double threshold);
    void sdromThresholdChanged(double threshold);

    void addXVTR(QString title,long long minFrequency,long long maxFrequency,long long ifFrequency,long long freq,int m,int filt);
    void deleteXVTR(int index);

    void RxIQcheckChanged(bool state);
    void RxIQspinChanged(double num);
    void spinBox_cwPitchChanged(int pitch);

public slots:
    void slotHostChanged(int selection);
    void slotReceiverChanged(int receiver);
    void slotSpectrumHighChanged(int high);
    void slotSpectrumLowChanged(int low);
    void slotFpsChanged(int fps);
    void slotWaterfallHighChanged(int high);
    void slotWaterfallLowChanged(int low);
    void slotWaterfallAutomaticChanged(bool state);
    void slotAudioDeviceChanged(int selection);
    void slotMicDeviceChanged(int selection);
    void slotSampleRateChanged(int rate);
    void slotChannelsChanged(int channels);
    void slotByteOrderChanged(int selection);
    void slotMicSampleRateChanged(int rate);
    void slotMicChannelsChanged(int channels);
    void slotMicOrderChanged(int selection);

    void slotUseRTP(bool state);

    void slotNrTapsChanged(int taps);
    void slotNrDelayChanged(int delay);
    void slotNrGainChanged(int gain);
    void slotNrLeakChanged(int leak);

    void slotAnfTapsChanged(int taps);
    void slotAnfDelayChanged(int delay);
    void slotAnfGainChanged(int gain);
    void slotAnfLeakChanged(int leak);

    void slotNbThresholdChanged(int threshold);
    void slotSdromThresholdChanged(int threshold);

    void slotXVTRAdd();
    void slotXVTRDelete();

private slots:
    void on_pBtnAddHost_clicked();
    void on_pBtnRemHost_clicked();
    void on_encodingComboBox_currentIndexChanged(int index);
    void on_RxIQcheckBox_toggled(bool checked);
    void on_RxIQspinBox_valueChanged(int spinValue);

    void on_userpasssave_clicked();

    void on_spinBox_cwPitch_valueChanged(int arg1);

private:
    Ui::Configure widget;
};

#endif	/* _CONFIGURE_H */
