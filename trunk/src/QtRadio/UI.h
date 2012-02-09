/* 
 * File:   UI.h
 * Author: John Melton, G0ORX/N6LYT
 *
 * Created on 13 August 2010, 14:28
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

#ifndef _UI_H
#define	_UI_H

#include "ui_UI.h"

#include <QDebug>
#include <QSettings>
#include <QTimer>
#include <QAudioFormat>
#include <QVector>
#include <QQueue>
#include <QThread>

#include "servers.h"
#include "About.h"
#include "Configure.h"
#include "Audio.h"
#include "Audioinput.h"
#include "Connection.h"
#include "Spectrum.h"
#include "Band.h"
#include "BandLimit.h"
#include "Mode.h"
#include "Filters.h"
#include "CWLFilters.h"
#include "CWUFilters.h"
#include "LSBFilters.h"
#include "USBFilters.h"
#include "DSBFilters.h"
#include "AMFilters.h"
#include "SAMFilters.h"
#include "FMNFilters.h"
#include "DIGLFilters.h"
#include "DIGUFilters.h"
#include "Bandscope.h"
#include "BookmarkDialog.h"
#include "Bookmark.h"
#include "Bookmarks.h"
#include "BookmarksDialog.h"
#include "BookmarksEditDialog.h"
#include "Xvtr.h"
#include "XvtrEntry.h"
#include "KeypadDialog.h"
#include "vfo.h"
#include "rigctl.h"
#include "ctl.h"
#include "G711A.h"
#include "RTP.h"


#define DSPSERVER_BASE_PORT 8000

#define AGC_LONG 1
#define AGC_SLOW 2
#define AGC_MEDIUM 3
#define AGC_FAST 4

#define MIC_BUFFER_SIZE 400
#define MIC_NO_OF_FRAMES 4      // need to ensure this is the same value in dspserver
#define MIC_ENCODED_BUFFER_SIZE (BITS_SIZE*MIC_NO_OF_FRAMES)

class UI : public QMainWindow {
    Q_OBJECT
public:
    UI(const QString server = QString(""));
    virtual ~UI();
    void loadSettings();
    void saveSettings();
    void closeEvent(QCloseEvent* event);
    void sendCommand(QString command);
    long long rigctlGetFreq();
    QString rigctlGetMode();
    QString rigctlGetFilter();
    QString rigctlGetVFO();
    void rigctlSetVFOA();
    void rigctlSetVFOB();
    void rigctlSetFreq(long long f);
    void rigctlSetMode(int newmode);
    void * codec2;
    void * mic_codec2;

signals:
    void initialize_audio(int length);
    void select_audio(QAudioDeviceInfo info,int rate,int channels,QAudioFormat::Endian byteOrder);
    void process_audio(char* header,char* buffer,int length);
    void rtp_send(unsigned char* buffer, int length);
    void HideTX(bool cantx);

public slots:
    void getMeterValue(int m, int s);

    void actionConfigure();
    void actionAbout();
    void actionConnect();
    void actionConnectNow(QString IP);
    void actionDisconnectNow();
    void actionDisconnect();
    void actionQuick_Server_List();
    void actionSubRx();
    void actionBandscope();
    void actionRecord();

    void actionMuteMainRx();
    void actionMuteSubRx();

    void actionGain_10();
    void actionGain_20();
    void actionGain_30();
    void actionGain_40();
    void actionGain_50();
    void actionGain_60();
    void actionGain_70();
    void actionGain_80();
    void actionGain_90();
    void actionGain_100();

    void actionSquelch();
    void actionSquelchReset();
    void squelchValueChanged(int);

    void actionKeypad();
    void setKeypadFrequency(long long);

    void getBandBtn(int btn);
    void quickMemStore();
    void action160();
    void action80();
    void action60();
    void action40();
    void action30();
    void action20();
    void action17();
    void action15();
    void action12();
    void action10();
    void action6();
    void actionGen();
    void actionWWV();

    void actionCWL();
    void actionCWU();
    void actionLSB();
    void actionUSB();
    void actionDSB();
    void actionAM();
    void actionSAM();
    void actionFMN();
    void actionDIGL();
    void actionDIGU();

    void actionFilter0();
    void actionFilter1();
    void actionFilter2();
    void actionFilter3();
    void actionFilter4();
    void actionFilter5();
    void actionFilter6();
    void actionFilter7();
    void actionFilter8();
    void actionFilter9();

    void actionANF();
    void actionNR();
    void actionNB();
    void actionSDROM();

    void actionPolyphase();

    void actionSlow();
    void actionMedium();
    void actionFast();
    void actionLong();

    void actionPreamp();

    void connected();
    void disconnected(QString message);
    void audioBuffer(char* header,char* buffer);
    void spectrumBuffer(char* header,char* buffer);

    void bandChanged(int previousBand,int newBand);
    void modeChanged(int previousMode,int newMode);
    void filtersChanged(FiltersBase* previousFilters,FiltersBase* newFilters);
    void filterChanged(int previousFilter,int newFilter);
    void frequencyChanged(long long frequency);

    void updateSpectrum();

    void audioDeviceChanged(QAudioDeviceInfo info,int rate,int channels,QAudioFormat::Endian byteOrder);
    void encodingChanged(int choice);

    void micDeviceChanged(QAudioDeviceInfo info,int rate,int channels,QAudioFormat::Endian byteOrder);
    void micSendAudio(QQueue<qint16>*);

    void setRTP(bool state);

    void setSubRxGain(int gain);

    void frequencyMoved(int increment,int step);

    void spectrumHighChanged(int high);
    void spectrumLowChanged(int low);
    void fpsChanged(int f);
    void waterfallHighChanged(int high);
    void waterfallLowChanged(int low);
    void waterfallAutomaticChanged(bool state);

    void hostChanged(QString host);
    void receiverChanged(int rx);

    void nrValuesChanged(int,int,double,double);
    void anfValuesChanged(int,int,double,double);
    void nbThresholdChanged(double);
    void sdromThresholdChanged(double);

    void actionBookmark();
    void addBookmark();
    void selectABookmark();
    void editBookmarks();
    void bookmarkDeleted(int);
    void bookmarkUpdated(int,QString);
    void bookmarkSelected(int entry);

    void addXVTR(QString,long long,long long,long long,long long,int,int);
    void deleteXVTR(int index);
    void selectXVTR(QAction* action);
    void selectBookmark(QAction* action);
    void getBandFrequency();
    void vfoStepBtnClicked(int direction);
    void pttChange(int caller, bool ptt);
    void pwrSlider_valueChanged(double pwr);
    void printStatusBar(QString message);
 //   void setRemote(char* host,int port);
    void slaveSetMode(int newmode);
    void slaveSetSlave(int slave); // 0 = slave
    void setdspversion(long dspversion, QString dspversiontxt);
    void setChkTX(bool chk);
    void setservername(QString sname);
    void setCanTX(bool tx);
    void closeServers ();
    void RxIQcheckChanged(bool state);
    void RxIQspinChanged(double num);
    void testSliderChange(int value);
    void testButtonClick(bool state);

signals:
    void subRxStateChanged(bool state);
    void set_src_ratio(double ratio);

protected:
//    void paintEvent(QPaintEvent*);


private:
    void printWindowTitle(QString message);
    QLabel modeInfo;
    void setSubRxPan();
    void actionGain(int g);
    void setGain(bool state);
    void initRigCtl();
    RigCtlServer *rigCtl;
    QString getversionstring();
    void appendBookmark(Bookmark* bookmark);

    QString stringFrequency(long long frequency);

    Ui::UI widget;
    Audio* audio;
    int audio_device;
    int audio_sample_rate;
    int audio_channels;
    int audio_buffers;
    QAudioFormat::Endian audio_byte_order;
    QMutex audio_mutex;
    char* first_audio_buffer;
    char* first_audio_header;
    int gain;
    int subRxGain;
    bool subRx;
    AudioInput* audioinput;
    int mic_buffer_count;       // counter of mic_buffer, to encode if reaches CODEC2_SAMPLE_PER_FRAME
    int mic_frame_count;        // counter of mic_buffer, to encode enough frames before sending

#if CODEC2_SAMPLES_PER_FRAME > MIC_BUFFER_SIZE
    qint16 mic_buffer[CODEC2_SAMPLES_PER_FRAME];
#else
    qint16 mic_buffer[MIC_BUFFER_SIZE];
#endif
#if MIC_ENCODED_BUFFER_SIZE > MIC_BUFFER_SIZE
    unsigned char mic_encoded_buffer[MIC_ENCODED_BUFFER_SIZE];
#else
    unsigned char mic_encoded_buffer[MIC_BUFFER_SIZE];
#endif
    unsigned char *rtp_send_buffer;
    long long subRxFrequency;
    Connection connection;
    bool connection_valid;

    Band band;
    Mode mode;
    Filters filters;
    CWLFilters cwlFilters;
    CWUFilters cwuFilters;
    LSBFilters lsbFilters;
    USBFilters usbFilters;
    DSBFilters dsbFilters;
    AMFilters amFilters;
    SAMFilters samFilters;
    FMNFilters fmnFilters;
    DIGUFilters diguFilters;
    DIGLFilters diglFilters;

    Xvtr xvtr;

    int agc;

    int cwPitch;

    long long frequency;

    int fps;
    QTimer* spectrumTimer;

    About about;
    Configure configure;
    Servers *servers;
    int sampleRate;

    Bandscope* bandscope;

    BookmarkDialog bookmarkDialog;
    BookmarksDialog* bookmarksDialog;
    BookmarksEditDialog* bookmarksEditDialog;

    Bookmarks bookmarks;

    KeypadDialog keypad;
    Meter* sMeter;
    int meter;
//    int txPwr;
    long long txFrequency;
    bool isConnected;
    QString QuickIP;

    bool squelch;
    float squelchValue;
    bool modeFlag; //Signals mode is changed from main menu

    G711A g711a;
    RTP *rtp;
    QThread *rtp_thread;
    bool useRTP;

    int tuning;
    int slave;  // 0 = slave mode
    int infotick;
    int infotick2;
    long dspversion;
    QString dspversiontxt;
    QString lastmessage;
    QString servername;
    bool canTX;
    bool chkTX;

};

#endif	/* _UI_H */
