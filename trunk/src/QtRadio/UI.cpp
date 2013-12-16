/*
 * File:   UI.cp
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

#include <QDebug>
#include <QSettings>
#include <QPainter>
#include <QtCore>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QMessageBox>
#else
#include <QMessageBox>
#endif

#include <QTimer>
#include <QThread>
#include <ortp/ortp.h>
#include "UI.h"
#include "About.h"
#include "Configure.h"
#include "Band.h"
#include "Mode.h"
#include "FiltersBase.h"
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
#include "XvtrEntry.h"
#include "vfo.h"
#include "Meter.h"
#include "Panadapter.h"
#include "smeter.h"
#include "codec2.h"
#include "servers.h"
#include "ctl.h"
#include "powermate.h"
#include "Frequency.h"
#include "EqualizerDialog.h"
#include "calc.h" // added by KD0OSS

UI::UI(const QString server) {

    widget.setupUi(this);
    servers = 0;
    pHwDlg = 0;
    meter = -121;
    initRigCtl();
    fprintf(stderr, "rigctl: Calling init\n");
    servername = "Unknown";
    configure.thisuser = "None";
    configure.thispass= "None";
    canTX = true;  // set to false if dspserver says so
    txNow = false; // KD0OSS
    audio = new Audio;
    loffset = 0;
    protocol3 = false;
    sampleZoomLevel = 0; // KD0OSS
    viewZoomLevel = 0; // KD0OSS

    rtp = new RTP;
    rtp_thread = new QThread(this);
    rtp->moveToThread(rtp_thread);
    rtp_thread->start(QThread::LowPriority);
    qDebug() << "QThread:  rtp_thread = " << rtp_thread;
    useRTP=configure.getRTP();
    configure.initAudioDevices(audio);

    mic_codec2 = codec2_create(CODEC2_MODE_3200);
    audioinput = new AudioInput;
    configure.initMicDevices(audioinput);

    mic_buffer_count = 0;
    mic_frame_count = 0;
    connection_valid = FALSE;

    isConnected = false;
    modeFlag = false;
    infotick = 0;
    infotick2 = 0;
    dspversion = 0;
    dspversiontxt = "Unknown";
    chkTX = false;

    // layout the screen
    widget.gridLayout->setContentsMargins(0,0,0,0);
    widget.gridLayout->setVerticalSpacing(0);
    widget.gridLayout->setHorizontalSpacing(0);

    widget.statusbar->showMessage("QtRadio branch: kd0oss 2013");

    connect(widget.vfoFrame,SIGNAL(getBandFrequency()),this,SLOT(getBandFrequency()));

    // connect up all the menus
    connect(widget.actionAbout,SIGNAL(triggered()),this,SLOT(actionAbout()));
    connect(widget.actionConnectToServer,SIGNAL(triggered()),this,SLOT(actionConnect()));
    connect(widget.actionQuick_Server_List,SIGNAL(triggered()),this,SLOT(actionQuick_Server_List()));
    connect(widget.actionDisconnectFromServer,SIGNAL(triggered()),this,SLOT(actionDisconnect()));

    connect(widget.actionSubrx,SIGNAL(triggered()),this,SLOT(actionSubRx()));
    connect(widget.actionBandscope,SIGNAL(triggered()),this,SLOT(actionBandscope()));
    connect(widget.actionRecord,SIGNAL(triggered()),this,SLOT(actionRecord()));

    connect(&connection,SIGNAL(isConnected()),this,SLOT(connected()));
    connect(&connection,SIGNAL(disconnected(QString)),this,SLOT(disconnected(QString)));
    connect(&connection,SIGNAL(audioBuffer(char*,char*)),this,SLOT(audioBuffer(char*,char*)));
    connect(&connection,SIGNAL(spectrumBuffer(char*,char*)),this,SLOT(spectrumBuffer(char*,char*)));

    connect(audioinput,SIGNAL(mic_send_audio(QQueue<qint16>*)),this,SLOT(micSendAudio(QQueue<qint16>*)));

    connect(widget.actionConfig,SIGNAL(triggered()),this,SLOT(actionConfigure()));
    connect(widget.actionEqualizer,SIGNAL(triggered()),this,SLOT(actionEqualizer())); // KD0OSS

    connect(widget.actionMuteMainRx,SIGNAL(triggered()),this,SLOT(actionMuteMainRx()));
    connect(widget.actionMuteSubRx,SIGNAL(triggered()),this,SLOT(actionMuteSubRx()));
/*
    connect(widget.actionGain_10,SIGNAL(triggered()),this,SLOT(actionGain_10()));
    connect(widget.actionGain_20,SIGNAL(triggered()),this,SLOT(actionGain_20()));
    connect(widget.actionGain_30,SIGNAL(triggered()),this,SLOT(actionGain_30()));
    connect(widget.actionGain_40,SIGNAL(triggered()),this,SLOT(actionGain_40()));
    connect(widget.actionGain_50,SIGNAL(triggered()),this,SLOT(actionGain_50()));
    connect(widget.actionGain_60,SIGNAL(triggered()),this,SLOT(actionGain_60()));
    connect(widget.actionGain_70,SIGNAL(triggered()),this,SLOT(actionGain_70()));
    connect(widget.actionGain_80,SIGNAL(triggered()),this,SLOT(actionGain_80()));
    connect(widget.actionGain_90,SIGNAL(triggered()),this,SLOT(actionGain_90()));
    connect(widget.actionGain_100,SIGNAL(triggered()),this,SLOT(actionGain_100()));
*/
    connect(widget.ctlFrame,SIGNAL(audioMuted(bool)),this,SLOT(setAudioMuted(bool)));
    connect(widget.ctlFrame,SIGNAL(audioGainChanged()),this,SLOT(audioGainChanged()));

    connect(widget.actionSquelchEnable,SIGNAL(triggered()),this,SLOT(actionSquelch()));
    connect(widget.actionSquelchReset,SIGNAL(triggered()),this,SLOT(actionSquelchReset()));

    connect(widget.actionKeypad, SIGNAL(triggered()),this,SLOT(actionKeypad()));
    connect(&keypad,SIGNAL(setKeypadFrequency(long long)),this,SLOT(setKeypadFrequency(long long)));

    connect(widget.vfoFrame,SIGNAL(bandBtnClicked(int)),this,SLOT(getBandBtn(int)));

    connect(widget.action160, SIGNAL(triggered()),this,SLOT(action160()));
    connect(widget.action80, SIGNAL(triggered()),this,SLOT(action80()));
    connect(widget.action60, SIGNAL(triggered()),this,SLOT(action60()));
    connect(widget.action40, SIGNAL(triggered()),this,SLOT(action40()));
    connect(widget.action30, SIGNAL(triggered()),this,SLOT(action30()));
    connect(widget.action20, SIGNAL(triggered()),this,SLOT(action20()));
    connect(widget.action17, SIGNAL(triggered()),this,SLOT(action17()));
    connect(widget.action15, SIGNAL(triggered()),this,SLOT(action15()));
    connect(widget.action12, SIGNAL(triggered()),this,SLOT(action12()));
    connect(widget.action10, SIGNAL(triggered()),this,SLOT(action10()));
    connect(widget.action6, SIGNAL(triggered()),this,SLOT(action6()));
    connect(widget.actionGen, SIGNAL(triggered()),this,SLOT(actionGen()));
    connect(widget.actionWWV, SIGNAL(triggered()),this,SLOT(actionWWV()));

    connect(widget.actionCWL,SIGNAL(triggered()),this,SLOT(actionCWL()));
    connect(widget.actionCWU,SIGNAL(triggered()),this,SLOT(actionCWU()));
    connect(widget.actionLSB,SIGNAL(triggered()),this,SLOT(actionLSB()));
    connect(widget.actionUSB,SIGNAL(triggered()),this,SLOT(actionUSB()));
    connect(widget.actionDSB,SIGNAL(triggered()),this,SLOT(actionDSB()));
    connect(widget.actionAM,SIGNAL(triggered()),this,SLOT(actionAM()));
    connect(widget.actionSAM,SIGNAL(triggered()),this,SLOT(actionSAM()));
    connect(widget.actionFMN,SIGNAL(triggered()),this,SLOT(actionFMN()));
    connect(widget.actionDIGL,SIGNAL(triggered()),this,SLOT(actionDIGL()));
    connect(widget.actionDIGU,SIGNAL(triggered()),this,SLOT(actionDIGU()));

    connect(widget.actionFilter_0,SIGNAL(triggered()),this,SLOT(actionFilter0()));
    connect(widget.actionFilter_1,SIGNAL(triggered()),this,SLOT(actionFilter1()));
    connect(widget.actionFilter_2,SIGNAL(triggered()),this,SLOT(actionFilter2()));
    connect(widget.actionFilter_3,SIGNAL(triggered()),this,SLOT(actionFilter3()));
    connect(widget.actionFilter_4,SIGNAL(triggered()),this,SLOT(actionFilter4()));
    connect(widget.actionFilter_5,SIGNAL(triggered()),this,SLOT(actionFilter5()));
    connect(widget.actionFilter_6,SIGNAL(triggered()),this,SLOT(actionFilter6()));
    connect(widget.actionFilter_7,SIGNAL(triggered()),this,SLOT(actionFilter7()));
    connect(widget.actionFilter_8,SIGNAL(triggered()),this,SLOT(actionFilter8()));
    connect(widget.actionFilter_9,SIGNAL(triggered()),this,SLOT(actionFilter9()));
    connect(widget.actionFilter_10,SIGNAL(triggered()),this,SLOT(actionFilter10()));

    connect(widget.actionANF,SIGNAL(triggered()),this,SLOT(actionANF()));
    connect(widget.actionNR,SIGNAL(triggered()),this,SLOT(actionNR()));
    connect(widget.actionNB,SIGNAL(triggered()),this,SLOT(actionNB()));
    connect(widget.actionSDROM,SIGNAL(triggered()),this,SLOT(actionSDROM()));

    connect(widget.actionPolyphase,SIGNAL(triggered()),this,SLOT(actionPolyphase()));

    connect(widget.actionFixed,SIGNAL(triggered()),this,SLOT(actionFixed()));  // KD0OSS
    connect(widget.actionLong,SIGNAL(triggered()),this,SLOT(actionLong()));
    connect(widget.actionSlow,SIGNAL(triggered()),this,SLOT(actionSlow()));
    connect(widget.actionMedium,SIGNAL(triggered()),this,SLOT(actionMedium()));
    connect(widget.actionFast,SIGNAL(triggered()),this,SLOT(actionFast()));


    connect(widget.actionPreamp,SIGNAL(triggered()),this,SLOT(actionPreamp()));

    connect(widget.actionPWS_Post_Filter,SIGNAL(triggered()),this,SLOT(actionPwsMode0()));  // KD0OSS
    connect(widget.actionPWS_Pre_Filter,SIGNAL(triggered()),this,SLOT(actionPwsMode1()));  // KD0OSS
    connect(widget.actionPWS_Semi_Raw,SIGNAL(triggered()),this,SLOT(actionPwsMode2()));  // KD0OSS
    connect(widget.actionPWS_Post_Det,SIGNAL(triggered()),this,SLOT(actionPwsMode3()));  // KD0OSS

    connect(widget.agcTLevelSlider,SIGNAL(valueChanged(int)),this,SLOT(AGCTLevelChanged(int)));  // KD0OSS

    connect(widget.rxEqEnableCB,SIGNAL(toggled(bool)),this,SLOT(enableRxEq(bool)));  // KD0OSS
    connect(widget.txEqEnableCB,SIGNAL(toggled(bool)),this,SLOT(enableTxEq(bool)));  // KD0OSS

    connect(widget.tnfButton, SIGNAL(clicked(bool)),widget.spectrumView,SLOT(enableNotchFilter(bool)));  // KD0OSS
    connect(widget.tnfAddButton,SIGNAL(clicked()),this,SLOT(addNotchFilter(void)));  // KD0OSS

    connect(widget.zoomSampRadio, SIGNAL(toggled(bool)), this, SLOT(setSampleZoom(bool))); // KD0OSS

    connect(widget.actionBookmarkThisFrequency,SIGNAL(triggered()),this,SLOT(actionBookmark()));
    connect(widget.actionEditBookmarks,SIGNAL(triggered()),this,SLOT(editBookmarks()));


    // connect up band and frequency changes
    connect(&band,SIGNAL(bandChanged(int,int)),this,SLOT(bandChanged(int,int)));
//    connect(&band,SIGNAL(frequencyChanged(long long)),this,SLOT(frequencyChanged(long long)));

    // connect up mode changes
    connect(&mode,SIGNAL(modeChanged(int,int)),this,SLOT(modeChanged(int,int)));

    // connect up filter changes
    connect(&filters,SIGNAL(filtersChanged(FiltersBase*,FiltersBase*)),this,SLOT(filtersChanged(FiltersBase*,FiltersBase*)));
    connect(&filters,SIGNAL(filterChanged(int,int)),this,SLOT(filterChanged(int,int)));
    connect(widget.spectrumView,SIGNAL(variableFilter(int,int)),this,SLOT(variableFilter(int,int)));

    // connect up spectrum view
    connect(widget.spectrumView, SIGNAL(frequencyMoved(int,int)),
            this, SLOT(frequencyMoved(int,int)));
//    connect(widget.spectrumView, SIGNAL(frequencyChanged(long long)),
//            this, SLOT(frequencyChanged(long long)));
    connect(widget.spectrumView, SIGNAL(spectrumHighChanged(int)),
            this,SLOT(spectrumHighChanged(int)));
    connect(widget.spectrumView, SIGNAL(spectrumLowChanged(int)),
            this,SLOT(spectrumLowChanged(int)));
    connect(widget.spectrumView, SIGNAL(waterfallHighChanged(int)),
            this,SLOT(waterfallHighChanged(int)));
    connect(widget.spectrumView, SIGNAL(waterfallLowChanged(int)),
            this,SLOT(waterfallLowChanged(int)));
    connect(widget.spectrumView, SIGNAL(meterValue(int,int)),
            this, SLOT(getMeterValue(int,int)));
    connect(widget.spectrumView, SIGNAL(squelchValueChanged(int)),
            this,SLOT(squelchValueChanged(int)));

    // connect up waterfall frame
//    connect(widget.waterfallView, SIGNAL(frequencyMoved(int,int)),
  //          this, SLOT(frequencyMoved(int,int)));

    connect(widget.spectrumView, SIGNAL(statusMessage(QString)), this, SLOT(statusMessage(QString))); // KD0OSS
    connect(widget.spectrumView, SIGNAL(removeNotchFilter()), this, SLOT(removeNotchFilter()));

    // connect up configuration changes
    connect(&configure,SIGNAL(spectrumHighChanged(int)),this,SLOT(spectrumHighChanged(int)));
    connect(&configure,SIGNAL(spectrumLowChanged(int)),this,SLOT(spectrumLowChanged(int)));
    connect(&configure,SIGNAL(fpsChanged(int)),this,SLOT(fpsChanged(int)));
    connect(&configure,SIGNAL(avgSpinChanged(int)),widget.spectrumView,SLOT(setAvg(int)));
    connect(&configure,SIGNAL(waterfallHighChanged(int)),this,SLOT(waterfallHighChanged(int)));
    connect(&configure,SIGNAL(waterfallLowChanged(int)),this,SLOT(waterfallLowChanged(int)));
    connect(&configure,SIGNAL(waterfallAutomaticChanged(bool)),this,SLOT(waterfallAutomaticChanged(bool)));
    connect(&configure,SIGNAL(encodingChanged(int)),this,SLOT(encodingChanged(int)));
    connect(&configure,SIGNAL(encodingChanged(int)),audio,SLOT(set_audio_encoding(int)));
    connect(&configure,SIGNAL(micEncodingChanged(int)),audioinput,SLOT(setMicEncoding(int)));
    connect(&configure,SIGNAL(audioDeviceChanged(QAudioDeviceInfo,int,int,QAudioFormat::Endian)),this,SLOT(audioDeviceChanged(QAudioDeviceInfo,int,int,QAudioFormat::Endian)));
    connect(&configure,SIGNAL(micDeviceChanged(QAudioDeviceInfo,int,int,QAudioFormat::Endian)),this,SLOT(micDeviceChanged(QAudioDeviceInfo,int,int,QAudioFormat::Endian)));


    connect(&configure,SIGNAL(useRTP(bool)),this,SLOT(setRTP(bool)));
    connect(&configure,SIGNAL(useRTP(bool)),audio,SLOT(set_RTP(bool)));

    connect(&configure,SIGNAL(hostChanged(QString)),this,SLOT(hostChanged(QString)));
    connect(&configure,SIGNAL(receiverChanged(int)),this,SLOT(receiverChanged(int)));
    connect(&configure,SIGNAL(rxDCBlockChanged(bool)),this,SLOT(rxDCBlockChanged(bool)));  // KD0OSS
    connect(&configure,SIGNAL(rxDCBlockGainChanged(int)),this,SLOT(rxDCBlockGainChanged(int)));  // KD0OSS
    connect(&configure,SIGNAL(txDCBlockChanged(bool)),this,SLOT(txDCBlockChanged(bool)));  // KD0OSS
    connect(&configure,SIGNAL(txIQPhaseChanged(double)),this,SLOT(setTxIQPhase(double)));  // KD0OSS
    connect(&configure,SIGNAL(txIQGainChanged(double)),this,SLOT(setTxIQGain(double)));  // KD0OSS

    connect(&configure,SIGNAL(nrValuesChanged(int,int,double,double)),this,SLOT(nrValuesChanged(int,int,double,double)));
    connect(&configure,SIGNAL(anfValuesChanged(int,int,double,double)),this,SLOT(anfValuesChanged(int,int,double,double)));
    connect(&configure,SIGNAL(nbThresholdChanged(double)),this,SLOT(nbThresholdChanged(double)));
    connect(&configure,SIGNAL(sdromThresholdChanged(double)),this,SLOT(sdromThresholdChanged(double)));

    connect(&configure,SIGNAL(rxIQPhaseChanged(double)),this,SLOT(setRxIQPhase(double)));  // KD0OSS
    connect(&configure,SIGNAL(rxIQGainChanged(double)),this,SLOT(setRxIQGain(double)));  // KD0OSS
    connect(&configure,SIGNAL(windowTypeChanged(int)),this,SLOT(windowTypeChanged(int))); //KD0OSS
    connect(&configure,SIGNAL(agcAttackChanged(int)),this,SLOT(agcAttackChanged(int))); //KD0OSS
    connect(&configure,SIGNAL(agcMaxGainChanged(int)),this,SLOT(agcMaxGainChanged(int))); //KD0OSS
    connect(&configure,SIGNAL(agcSlopeChanged(int)),this,SLOT(agcSlopeChanged(int))); //KD0OSS
    connect(&configure,SIGNAL(agcDecayChanged(int)),this,SLOT(agcDecayChanged(int))); //KD0OSS
    connect(&configure,SIGNAL(agcHangChanged(int)),this,SLOT(agcHangChanged(int))); //KD0OSS
    connect(&configure,SIGNAL(agcFixedGainChanged(int)),this,SLOT(agcFixedGainChanged(int))); //KD0OSS
    connect(&configure,SIGNAL(agcHangThreshChanged(int)),this,SLOT(agcHangThreshChanged(int))); //KD0OSS
    connect(&configure,SIGNAL(levelerStateChanged(int)),this,SLOT(levelerStateChanged(int))); //KD0OSS
    connect(&configure,SIGNAL(levelerMaxGainChanged(int)),this,SLOT(levelerMaxGainChanged(int))); //KD0OSS
    connect(&configure,SIGNAL(levelerAttackChanged(int)),this,SLOT(levelerAttackChanged(int))); //KD0OSS
    connect(&configure,SIGNAL(levelerDecayChanged(int)),this,SLOT(levelerDecayChanged(int))); //KD0OSS
    connect(&configure,SIGNAL(levelerHangChanged(int)),this,SLOT(levelerHangChanged(int))); //KD0OSS
    connect(&configure,SIGNAL(alcStateChanged(int)),this,SLOT(alcStateChanged(int))); //KD0OSS
    connect(&configure,SIGNAL(alcAttackChanged(int)),this,SLOT(alcAttackChanged(int))); //KD0OSS
    connect(&configure,SIGNAL(alcDecayChanged(int)),this,SLOT(alcDecayChanged(int))); //KD0OSS
    connect(&configure,SIGNAL(alcHangChanged(int)),this,SLOT(alcHangChanged(int))); //KD0OSS
/*    connect(&configure,SIGNAL(nbTransitionChanged(int)),this,SLOT(nbTransitionChanged(int))); //KD0OSS
    connect(&configure,SIGNAL(nbLeadChanged(int)),this,SLOT(nbLeadChanged(int))); //KD0OSS
    connect(&configure,SIGNAL(nbLagChanged(int)),this,SLOT(nbLagChanged(int))); //KD0OSS
*/
    connect(&bookmarks,SIGNAL(bookmarkSelected(QAction*)),this,SLOT(selectBookmark(QAction*)));
    connect(&bookmarkDialog,SIGNAL(accepted()),this,SLOT(addBookmark()));

    connect(&configure,SIGNAL(addXVTR(QString,long long,long long,long long,long long,int,int)),this,SLOT(addXVTR(QString,long long,long long,long long,long long,int,int)));
    connect(&configure,SIGNAL(deleteXVTR(int)),this,SLOT(deleteXVTR(int)));
//    connect(&configure,SIGNAL(get_audio_devices(QComboBox*)),audio,SLOT(get_audio_devices(QComboBox*)));

    connect(&xvtr,SIGNAL(xvtrSelected(QAction*)),this,SLOT(selectXVTR(QAction*)));

    connect(widget.vfoFrame,SIGNAL(frequencyMoved(int,int)),this,SLOT(frequencyMoved(int,int)));
    connect(widget.vfoFrame,SIGNAL(frequencyChanged(long long)),this,SLOT(frequencyChanged(long long)));
    connect(widget.vfoFrame,SIGNAL(subRxButtonClicked()),this,SLOT(actionSubRx()));
    connect(widget.vfoFrame,SIGNAL(vfoStepBtnClicked(int)),this,SLOT(vfoStepBtnClicked(int)));
    connect(this,SIGNAL(process_audio(char*,char*,int)),audio,SLOT(process_audio(char*,char*,int)));
    connect(widget.ctlFrame,SIGNAL(pttChange(int,bool)),this,SLOT(pttChange(int,bool)));
    connect(widget.ctlFrame,SIGNAL(pwrSlider_valueChanged(double)),this,SLOT(pwrSlider_valueChanged(double)));
    connect(widget.vfoFrame,SIGNAL(rightBandClick()),this,SLOT(quickMemStore()));
    connect(&band,SIGNAL(printStatusBar(QString)),this,SLOT(printStatusBar(QString)));
    connect(&connection,SIGNAL(printStatusBar(QString)),this,SLOT(printStatusBar(QString)));
    connect(&connection,SIGNAL(slaveSetFreq(long long)),this,SLOT(frequencyChanged(long long)));
    connect(&connection,SIGNAL(slaveSetMode(int)),this,SLOT(slaveSetMode(int)));
    connect(&connection,SIGNAL(slaveSetFilter(int,int)),this,SLOT(slaveSetFilter(int,int)));
    connect(&connection,SIGNAL(slaveSetZoom(int)),this,SLOT(slaveSetZoom(int)));
    connect(&connection,SIGNAL(setdspversion(long, QString)),this,SLOT(setdspversion(long, QString)));
    connect(this,SIGNAL(HideTX(bool)),widget.ctlFrame,SLOT(HideTX(bool)));
    connect(&connection,SIGNAL(setservername(QString)),this,SLOT(setservername(QString)));
    connect(&connection,SIGNAL(setCanTX(bool)),this,SLOT(setCanTX(bool)));
    connect(&connection,SIGNAL(setChkTX(bool)),this,SLOT(setChkTX(bool)));
    connect(&connection,SIGNAL(resetbandedges(double)),this,SLOT(resetbandedges(double)));
    connect(&connection,SIGNAL(setRemoteRTPPort(QString,int)),rtp,SLOT(setRemote(QString,int)));
    connect(&connection,SIGNAL(setFPS()),this,SLOT(setFPS()));
    connect(&connection,SIGNAL(setProtocol3(bool)),this,SLOT(setProtocol3(bool)));
    connect(rtp,SIGNAL(rtp_set_session(RtpSession*)),audio,SLOT(rtp_set_rtpSession(RtpSession*)));
    connect(this,SIGNAL(rtp_send(unsigned char*,int)),rtp,SLOT(send(unsigned char*,int)));
    connect(&configure,SIGNAL(RxIQcheckChanged(bool)),this,SLOT(RxIQcheckChanged(bool)));
    connect(&configure,SIGNAL(RxIQspinChanged(double)),this,SLOT(RxIQspinChanged(double)));
    connect(&configure,SIGNAL(spinBox_cwPitchChanged(int)),this,SLOT(cwPitchChanged(int)));
//    connect(widget.ctlFrame,SIGNAL(testBtnClick(bool)),this,SLOT(testButtonClick(bool)));
//    connect(widget.ctlFrame,SIGNAL(testSliderChange(int)),this,SLOT(testSliderChange(int)));
    connect(&connection,SIGNAL(hardware(QString)),this,SLOT(hardware(QString)));
    connect(widget.ctlFrame,SIGNAL(masterBtnClicked()),this,SLOT(masterButtonClicked()));


    bandscope = NULL;

    fps = 15;
    gain = 100;
    subRx = FALSE;
    subRxGain = 100;
    agc = AGC_SLOW;
    cwPitch=configure.getCwPitch();
    squelchValue = -100;
    squelch = false;
    notchFilterIndex = 0;    // KD0OSS

    audio->get_audio_device(&audio_device);
    audio_sample_rate=configure.getSampleRate();
    audio_channels=configure.getChannels();
    audio_byte_order=configure.getByteOrder();

    widget.spectrumView->connection = &connection; // KD0OSS
    widget.spectrumView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    widget.spectrumView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    equalizer = new EqualizerDialog(&connection); // KD0OSS

    // load any saved settings
    loadSettings();
    switch(agc) {
        case AGC_FIXED:  // KD0OSS
            widget.actionFixed->setChecked(TRUE);
            break;
        case AGC_SLOW:
            widget.actionSlow->setChecked(TRUE);
            break;
        case AGC_MEDIUM:
            widget.actionMedium->setChecked(TRUE);
            break;
        case AGC_FAST:
            widget.actionFast->setChecked(TRUE);
            break;
        case AGC_LONG:
            widget.actionLong->setChecked(TRUE);
            break;
    }

    fps = configure.getFps();

    configure.updateXvtrList(&xvtr);
    xvtr.buildMenu(widget.menuXVTR);

    widget.spectrumView->setHost(configure.getHost());

    printWindowTitle("Remote disconnected"); //added by gvj
    //widget.spectrumView->setReceiver(configure.getReceiver()); //deleted by gvj

    //Configure statusBar
 //   modeInfo.setMinimumWidth(350);
    widget.statusbar->addPermanentWidget(&modeInfo);

    widget.actionSubrx->setDisabled(TRUE);
    widget.actionMuteSubRx->setDisabled(TRUE);

    band.initBand(band.getBand());

    // make spectrum timer
    spectrumTimer = new QTimer(this);
    connect ( spectrumTimer, SIGNAL ( timeout() ), this, SLOT ( updateSpectrum()) );

    // automatically select a server and connect to it //IW0HDV
    if (server.length()) {
       qDebug() << "Connecting to " << server;
       emit actionConnectNow(server);
    }
}

UI::~UI() {
    connection.disconnect();
    rtp->deleteLater();
    codec2_destroy(mic_codec2);
    equalizer->deleteLater();
    saveSettings();
}

void UI::actionAbout() {
    about.setVisible(TRUE);
}

void UI::loadSettings() {
    QSettings settings("G0ORX", "QtRadio");
    qDebug() << "loadSettings: " << settings.fileName();

    widget.ctlFrame->loadSettings(&settings);
//    Ctl.loadSettings(&settings);
    band.loadSettings(&settings);
    xvtr.loadSettings(&settings);
    configure.loadSettings(&settings);
    configure.updateXvtrList(&xvtr);
    bookmarks.loadSettings(&settings);
    bookmarks.buildMenu(widget.menuView_Bookmarks);
    settings.beginGroup("UI");
    if(settings.contains("gain")) gain=subRxGain=settings.value("gain").toInt();
    emit widget.ctlFrame->audioGainInitalized(gain);  // KD0OSS
    if(settings.contains("agc")) agc=settings.value("agc").toInt();
    if(settings.contains("squelch")) squelchValue=settings.value("squelch").toInt();
    if(settings.contains("pwsmode")) pwsmode=settings.value("pwsmode").toInt();  // KD0OSS
    settings.endGroup();

    settings.beginGroup("mainWindow");
    if (configure.getGeometryState()) {
        restoreGeometry(settings.value("geometry").toByteArray());
    }
    settings.endGroup();

    // KD0OSS ******************************************
    settings.beginGroup("AudioEqualizer");
    if (settings.contains("eqMode"))
    {
        if (settings.value("eqMode") == 3)
            equalizer->loadSettings3Band();
        else
            equalizer->loadSettings10Band();

        if (settings.value("rxEqEnabled") == 1)
        {
            widget.rxEqEnableCB->setChecked(true);
            enableRxEq(true);
        }

        if (settings.value("txEqEnabled") == 1)
        {
            widget.txEqEnableCB->setChecked(true);
            enableTxEq(true);
        }
    }
    else
    {
        settings.setValue("eqMode", 10);
        equalizer->set10BandEqualizer();
    }
    settings.endGroup();
    // **************************************************
    widget.vfoFrame->readSettings(&settings);
    setPwsMode(pwsmode);  // KD0OSS
}

void UI::saveSettings() {
    QString s;
    //Bookmark* bookmark;

    QSettings settings("G0ORX","QtRadio");

    qDebug() << "saveSettings: " << settings.fileName();

  //  settings.clear();
    widget.ctlFrame->saveSettings(&settings);
    configure.saveSettings(&settings);
    band.saveSettings(&settings);
    xvtr.saveSettings(&settings);
    bookmarks.saveSettings(&settings);

    settings.beginGroup("UI");
    settings.setValue("gain",gain);
    settings.setValue("subRxGain",subRxGain);
    settings.setValue("agc",agc);
    settings.setValue("squelch",squelchValue);
    settings.setValue("pwsmode",pwsmode);  // KD0OSS
    settings.endGroup();

    settings.beginGroup("mainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();

    // KD0OSS  ********************************************************
    settings.beginGroup("AudioEqualizer");
    settings.setValue("rxEqEnabled", widget.rxEqEnableCB->isChecked());
    settings.setValue("txEqEnabled", widget.txEqEnableCB->isChecked());
    settings.endGroup();
    //  ***************************************************************
    widget.vfoFrame->writeSettings(&settings);
}

void UI::hostChanged(QString host) {
    widget.spectrumView->setHost(host);
    printWindowTitle("Remote disconnected");
}

void UI::receiverChanged(int rx) {
    widget.spectrumView->setReceiver(rx);
    printWindowTitle("Remote disconnected");
}

void UI::closeServers ()
{
    if (servers) {
       delete servers;
       servers = 0;
    }
}

void UI::closeEvent(QCloseEvent* event) {
    Q_UNUSED(event);
    saveSettings();
    if (servers) {
       servers->close();   // synchronous call, triggers a closeServer signal (see above)
                           // no needs to delete the object pointed by "servers" 
    }
    rmHwDlg();
}

void UI::actionConfigure() {
    configure.show();
}

void UI::actionEqualizer() { // KD0OSS
    equalizer->show();
}

void UI::spectrumHighChanged(int high) {
    //qDebug() << __FUNCTION__ << ": " << high;

    widget.spectrumView->setHigh(high);
    configure.setSpectrumHigh(high);
    band.setSpectrumHigh(high);
}

void UI::spectrumLowChanged(int low) {
    //qDebug() << __FUNCTION__ << ": " << low;

    widget.spectrumView->setLow(low);
    configure.setSpectrumLow(low);
    band.setSpectrumLow(low);
}

void UI::fpsChanged(int f) {
    //qDebug() << "fpsChanged:" << f;
    fps=f;
}

void UI::setFPS(void){
    QString command;
    command.clear(); QTextStream(&command) << "setFPS " << widget.spectrumView->width() << " " << fps;
    connection.sendCommand(command);
}

void UI::resizeEvent(QResizeEvent *){
    if (protocol3){
        QString command;
        command.clear(); QTextStream(&command) << "setFPS " << widget.spectrumView->width() << " " << fps;
        connection.sendCommand(command);
    }
}

void UI::setProtocol3(bool p){
    protocol3 = p;
}

void UI::waterfallHighChanged(int high) {
    //qDebug() << __LINE__ << __FUNCTION__ << ": " << high;

    widget.spectrumView->panadapterScene->waterfallItem->setHigh(high);
    configure.setWaterfallHigh(high);
    band.setWaterfallHigh(high);
}

void UI::waterfallLowChanged(int low) {
    //qDebug() << __FUNCTION__ << ": " << low;

    widget.spectrumView->panadapterScene->waterfallItem->setLow(low);
    configure.setWaterfallLow(low);
    band.setWaterfallLow(low);
}

void UI::waterfallAutomaticChanged(bool state) {
    widget.spectrumView->panadapterScene->waterfallItem->setAutomatic(state);
}

void UI::audioDeviceChanged(QAudioDeviceInfo info,int rate,int channels,QAudioFormat::Endian byteOrder) {
    audio_device = info;
    audio_sample_rate = rate;
    audio_channels = channels;
    audio_byte_order = byteOrder;
    audio->select_audio(info, rate, channels, byteOrder);
}

void UI::encodingChanged(int choice){
    audio->set_audio_encoding(choice);
//    qDebug() << "UI: encodingChanged: " << choice;
    if (choice == 2){               // Codec 2
        configure.setChannels(1);
        configure.setSampleRate(8000);
    }
}

void UI::micDeviceChanged(QAudioDeviceInfo info,int rate,int channels,QAudioFormat::Endian byteOrder) {
    audioinput->select_audio(info,rate,channels,byteOrder);
}

void UI::actionConnect() {
    //qDebug() << "UI::actionConnect";
//    widget.statusbar->clearMessage(); //deleted by gvj
    connection.connect(configure.getHost(), DSPSERVER_BASE_PORT+configure.getReceiver());
    //widget.spectrumView->setHost(configure.getHost()); //deleted by gvj
    widget.spectrumView->setReceiver(configure.getReceiver());
    isConnected = true;

    // Initialise RxIQMu. Set RxIQMu to disabled, set value and then enable if checked.
    RxIQspinChanged(configure.getRxIQspinBoxValue());
//    rxDCBlockChanged(configure.getRxDCBlockValue());
//    txDCBlockChanged(configure.getTxDCBlockValue());
    widget.zoomSpectrumSlider->setValue(0);
    on_zoomSpectrumSlider_sliderMoved(0);
}


void UI::actionDisconnectNow(){

    if(isConnected == false){
       QMessageBox msgBox;
       msgBox.setText("Not connected to a server!");
       msgBox.exec();
    }else{
       actionDisconnect();

    }
}


void UI::actionDisconnect() {

    qDebug() << "actionDisconnect() QuickIP=" << QuickIP;
    if (QuickIP.length() > 6){    // Remove from saved host list or IPs will pile up forever. If empty string we did not connect via Quick Connect
      configure.removeHost(QuickIP);
      qDebug() << "actionDisconnect() removeHost(" << QuickIP <<")";
    }
    QuickIP ="";
    spectrumTimer->stop();
    protocol3 = false;
    widget.zoomSpectrumSlider->setValue(0);
    on_zoomSpectrumSlider_sliderMoved(0);

    connection.disconnect();
    widget.actionConnectToServer->setDisabled(FALSE);
    widget.actionDisconnectFromServer->setDisabled(TRUE);
    widget.actionSubrx->setDisabled(TRUE);
    widget.actionMuteSubRx->setDisabled(TRUE);

    configure.connected(FALSE);
    isConnected = false;

}

void UI::actionQuick_Server_List() {
   servers = new Servers();
   QObject::connect(servers, SIGNAL(disconnectNow()), this, SLOT(actionDisconnectNow()));
   QObject::connect(servers, SIGNAL(connectNow(QString)), this, SLOT(actionConnectNow(QString)));
   QObject::connect(servers, SIGNAL(dialogClosed()), this, SLOT(closeServers()));
   servers->show();
   servers->refreshList();
}

void UI::connected() {
    QString command;

    qDebug() << "UI::connected";
    isConnected = true;
    configure.connected(TRUE);

    // let them know who we are
    command.clear(); QTextStream(&command) << "setClient QtRadio";
    connection.sendCommand(command);

    connection.sendCommand("q-server");

    // send initial settings
    frequency=band.getFrequency();
    command.clear(); QTextStream(&command) << "setFrequency " << frequency;
    connection.sendCommand(command);
    widget.spectrumView->setFrequency(frequency);
    //widget.waterfallView->setFrequency(frequency);

//    gvj code
    widget.vfoFrame->setFrequency(frequency);

    command.clear(); QTextStream(&command) << "setMode " << band.getMode();
    connection.sendCommand(command);

    int low,high;
    if(mode.getMode()==MODE_CWL) {
        low=-cwPitch-filters.getLow();
        high=-cwPitch+filters.getHigh();
    } else if(mode.getMode()==MODE_CWU) {
        low=cwPitch-filters.getLow();
        high=cwPitch+filters.getHigh();
    } else {
        low=filters.getLow();
        high=filters.getHigh();
    }
    command.clear(); QTextStream(&command) << "setFilter " << low << " " << high;
    connection.sendCommand(command);

    // qDebug() << "connected calling widget.spectrumView.setFilter";

    widget.spectrumView->setFilter(low,high);
//    widget.waterfallView->setFilter(low,high);

    widget.actionConnectToServer->setDisabled(TRUE);
    widget.actionDisconnectFromServer->setDisabled(FALSE);
    widget.actionSubrx->setDisabled(FALSE);
    widget.actionMuteSubRx->setDisabled(TRUE);

    setPwsMode(pwsmode); // KD0OSS

    // select audio encoding
    command.clear(); QTextStream(&command) << "setEncoding " << audio->get_audio_encoding();
    connection.sendCommand(command);
    // qDebug() << "Command: " << command;

    // upon connection, (re)select the audio_device in case it was stopped because of errors
    // somehow this call stops RTP audio from working
    if (!useRTP) audio->select_audio(audio_device, audio_sample_rate, audio_channels, audio_byte_order);

    // start the audio
    audio_buffers=0;
    actionGain(gain);

    if (!getenv("QT_RADIO_NO_LOCAL_AUDIO")) {
       if(useRTP) {
           // g0orx RTP
           char host_ip[64];
           QString host = connection.getHost();
           memcpy(host_ip, host.toUtf8().constData(),host.length());
           host_ip[host.length()]=0;
           int local_port = rtp->init(host_ip, -1);

           command.clear(); QTextStream(&command) << "startRTPStream "
                 << local_port
                 << " " << audio->get_audio_encoding()
                 << " " << audio_sample_rate << " "
                 << " " << audio_channels;

           QTimer::singleShot(500,audio,SLOT(rtp_set_connected()));
       } else {
           command.clear(); QTextStream(&command) << "startAudioStream "
                << (AUDIO_BUFFER_SIZE*(audio_sample_rate/8000)) << " "
                << audio_sample_rate << " "
                << audio_channels << " " << audioinput->getMicEncoding();
       }
       connection.sendCommand(command);
       qDebug() << "command: " << command;
    }

    command.clear(); QTextStream(&command) << "SetPan 0.5"; // center
    connection.sendCommand(command);

    command.clear(); QTextStream(&command) << "SetAGC " << agc;
    connection.sendCommand(command);

    command.clear(); QTextStream(&command) << "setpwsmode " << pwsmode; // KD0OSS
    connection.sendCommand(command);

    command.clear(); QTextStream(&command) << "SetANFVals " << configure.getAnfTaps() << " " << configure.getAnfDelay() << " "
                                           << configure.getAnfGain() << " " << configure.getAnfLeak();
    connection.sendCommand(command);

    command.clear(); QTextStream(&command) << "SetNRVals " << configure.getNrTaps() << " " << configure.getNrDelay() << " "
                                           << configure.getNrGain() << " " << configure.getNrLeak();
    connection.sendCommand(command);

    command.clear(); QTextStream(&command) << "SetNBVals " << configure.getNbThreshold();
    connection.sendCommand(command);

    command.clear(); QTextStream(&command) << "SetSquelchVal " << squelchValue;
    connection.sendCommand(command);
    command.clear(); QTextStream(&command) << "SetSquelchState " << (widget.actionSquelchEnable->isChecked()?"on":"off");
    connection.sendCommand(command);

    command.clear(); QTextStream(&command) << "SetANF " << (widget.actionANF->isChecked()?"true":"false");
    connection.sendCommand(command);
    command.clear(); QTextStream(&command) << "SetNR " << (widget.actionNR->isChecked()?"true":"false");
    connection.sendCommand(command);
    command.clear(); QTextStream(&command) << "SetNB " << (widget.actionNB->isChecked()?"true":"false");
    connection.sendCommand(command);

    printWindowTitle("Remote connected");

    // Added by KD0OSS *****************************************
//    if (dspversion >= 20130609)
    {
        qDebug("Sending advanced setup commands.");
        command.clear(); QTextStream(&command) << "setrxdcblock " << configure.getRxDCBlockValue();
        connection.sendCommand(command);
        command.clear(); QTextStream(&command) << "settxdcblock " << configure.getTxDCBlockValue();
        connection.sendCommand(command);

        command.clear(); QTextStream(&command) << "setrxagcslope " << configure.getRxAGCSlopeValue();
  //      connection.sendCommand(command);
        command.clear(); QTextStream(&command) << "setrxagcmaxgain " << configure.getRxAGCMaxGainValue();
        connection.sendCommand(command);
        command.clear(); QTextStream(&command) << "setrxagcattack " << configure.getRxAGCAttackValue();
        connection.sendCommand(command);
        command.clear(); QTextStream(&command) << "setrxagcdecay " << configure.getRxAGCDecayValue();
        connection.sendCommand(command);
        command.clear(); QTextStream(&command) << "setrxagchang " << configure.getRxAGCHangValue();
        connection.sendCommand(command);
        command.clear(); QTextStream(&command) << "setfixedagc " << configure.getRxAGCFixedGainValue();
        connection.sendCommand(command);
        command.clear(); QTextStream(&command) << "setrxagchangthreshold " << configure.getRxAGCHangThreshValue();
        connection.sendCommand(command);

        command.clear(); QTextStream(&command) << "settxlevelerstate " << configure.getLevelerEnabledValue();
        connection.sendCommand(command);
        command.clear(); QTextStream(&command) << "settxlevelermaxgain " << configure.getLevelerMaxGainValue();
        connection.sendCommand(command);
        command.clear(); QTextStream(&command) << "settxlevelerattack " << configure.getLevelerAttackValue();
        connection.sendCommand(command);
        command.clear(); QTextStream(&command) << "settxlevelerdecay " << configure.getLevelerDecayValue();
        connection.sendCommand(command);
        command.clear(); QTextStream(&command) << "settxlevelerhang " << configure.getLevelerHangValue();
        connection.sendCommand(command);

        command.clear(); QTextStream(&command) << "settxalcstate " << configure.getALCEnabledValue();
        connection.sendCommand(command);
        command.clear(); QTextStream(&command) << "settxalcattack " << configure.getALCAttackValue();
        connection.sendCommand(command);
        command.clear(); QTextStream(&command) << "settxalcdecay " << configure.getALCDecayValue();
        connection.sendCommand(command);
        command.clear(); QTextStream(&command) << "settxalchang " << configure.getALCHangValue();
        connection.sendCommand(command);
    }
    // ***************************KD0OSS******************************

    //
    // hardware special command
    // queries hardware name from remote server 
    //
    command.clear(); QTextStream(&command) << "*hardware?";
    connection.sendCommand(command);

    // start the spectrum
    //qDebug() << "starting spectrum timer";
    connection.SemSpectrum.release();
    spectrumTimer->start(1000/fps);
    connection_valid = TRUE;
    if((mode.getStringMode()=="CWU")||(mode.getStringMode()=="CWL")) frequencyChanged(frequency); //gvj dummy call to set Rx offset for cw

    widget.spectrumView->enableNotchFilter(false); // KD0OSS
}

void UI::disconnected(QString message) {
    qDebug() << "UI::disconnected: " << message;
    connection_valid = FALSE;
    isConnected = false;
    spectrumTimer->stop();
    configure.thisuser = "none";
    configure.thispass ="none";
    servername = "";
    canTX = true;
    chkTX = false;
    loffset = 0;

    if(useRTP) {
        audio->rtp_set_disconnected();
        rtp->shutdown();
    }

    audio->clear_decoded_buffer();

//    widget.statusbar->showMessage(message,0); //gvj deleted code
    printWindowTitle(message);
    widget.actionConnectToServer->setDisabled(FALSE);
    widget.actionDisconnectFromServer->setDisabled(TRUE);
    widget.actionSubrx->setDisabled(TRUE);
    widget.actionMuteSubRx->setDisabled(TRUE);

    configure.connected(FALSE);
}

void UI::updateSpectrum() {

    if (!protocol3){
        QString command;
        command.clear(); QTextStream(&command) << "getSpectrum " << widget.spectrumView->width();
        connection.sendCommand(command);
    }
    if(infotick > 25){
        connection.sendCommand("q-master");
       if (connection.getSlave() == true) connection.sendCommand("q-info"); // get master freq changes
       infotick = 0;
    }
    if(infotick2 == 0){ // set to 0 wehen we first connect
       if (chkTX) connection.sendCommand("q-cantx#" + configure.thisuser); // can we tx here?
    }
    if(infotick2 > 50){
       if (chkTX) connection.sendCommand("q-cantx#" + configure.thisuser); // can we tx here?
       infotick2 = 0;
    }
    infotick++;
    infotick2++;

}

void UI::spectrumBuffer(char* header,char* buffer) {
    //qDebug()<<Q_FUNC_INFO << "spectrumBuffer";
// g0orx binary header
/*
    int length=atoi(&header[26]);
    sampleRate=atoi(&header[32]);
*/

    int length=((header[3]&0xFF)<<8)+(header[4]&0xFF);
    sampleRate=((header[9]&0xFF)<<24)+((header[10]&0xFF)<<16)+((header[11]&0xFF)<<8)+(header[12]&0xFF);

    widget.spectrumView->updateSpectrumFrame(header,buffer,length);
//    widget.spectrumView->panadapterScene->waterfallItem->updateWaterfall(header,buffer,length);
//    widget.waterfallView->updateWaterfall(header,buffer,length);
    connection.freeBuffers(header,buffer);
}

void UI::audioBuffer(char* header,char* buffer) {
    //qDebug() << "audioBuffer";
    int length;

// g0orx binary header
    //length=atoi(&header[AUDIO_LENGTH_POSITION]);
    length=((header[3]&0xFF)<<8)+(header[4]&0xFF);
    emit process_audio(header,buffer,length);
 }


void UI::micSendAudio(QQueue<qint16>* queue){
    if (!txNow)
    {
        queue->clear();
        return;
    }
    if(useRTP) {
        while(!queue->isEmpty()) {
            qint16 sample=queue->dequeue();
            if(tuning) sample=0;
            unsigned char e=g711a.encode(sample);
            mic_encoded_buffer[mic_buffer_count++]=e;
            if(mic_buffer_count >= MIC_BUFFER_SIZE) {
                // we are going to really send samples only if
                //  the connection is valid
                //  the checkbox in GUI is checked
                //  the server side has Tx capability
                if (connection_valid && configure.getTxAllowed() && (canTX == true)){
                    rtp_send_buffer = (unsigned char*) malloc(MIC_BUFFER_SIZE);
                    // rtp_send_buffer will be free'd by rtp_send()
                    memcpy(rtp_send_buffer, mic_encoded_buffer, MIC_BUFFER_SIZE);
                    emit rtp_send(rtp_send_buffer,MIC_BUFFER_SIZE);
                }
                mic_buffer_count=0;
            }
        }

    } else if (audioinput->getMicEncoding() == 1){      // Codec 2
        int samples_per_frame = codec2_samples_per_frame(mic_codec2);
        int bits_size = codec2_bits_per_frame(mic_codec2)/8;
        while(! queue->isEmpty()){
            qint16 sample = queue->dequeue();
            mic_buffer[mic_buffer_count++] = tuning ? 0: sample;
            if (mic_buffer_count >= codec2_samples_per_frame(mic_codec2)) {
                mic_buffer_count = 0;
                if (connection_valid && configure.getTxAllowed())
                    codec2_encode(mic_codec2, &mic_encoded_buffer[mic_frame_count*bits_size], mic_buffer);
                mic_frame_count++;
                if (mic_frame_count >= MIC_NO_OF_FRAMES){
                    mic_frame_count = 0;
                    // we are going to really send samples only if
                    //  the connection is valid
                    //  the checkbox in GUI is checked
                    //  the server side has Tx capability
                    if (connection_valid && configure.getTxAllowed() && (canTX == true))
                        connection.sendAudio(samples_per_frame*MIC_NO_OF_FRAMES,mic_encoded_buffer);
                }
            }
        }
    } else if (audioinput->getMicEncoding() == 0){      // aLaw
        while(!queue->isEmpty()) {
            qint16 sample=queue->dequeue();
            if(tuning) sample=0;
            unsigned char e=g711a.encode(sample);
            mic_encoded_buffer[mic_buffer_count++] = e;
            if(mic_buffer_count >= MIC_ALAW_BUFFER_SIZE) {
                // we are going to really send samples only if
                //  the connection is valid
                //  the checkbox in GUI is checked
                //  the server side has Tx capability
                if (connection_valid && configure.getTxAllowed() && (canTX == true)){
                    connection.sendAudio(MIC_ALAW_BUFFER_SIZE, mic_encoded_buffer);
                }
                mic_buffer_count=0;
            }
        }
    } else {
        qDebug() << "Error - UI: MicEncoding is not 0 nor 1 but " << audioinput->getMicEncoding();
    }

}

void UI::actionSubRx() {
    QString command;
    if(subRx) {        
        // on, so turn off
        subRx=FALSE;
        widget.spectrumView->setSubRxState(FALSE);
//        widget.waterfallView->setSubRxState(FALSE);
        widget.sMeterFrame->setSubRxState(FALSE);
        widget.actionMuteSubRx->setChecked(FALSE);
        widget.actionMuteSubRx->setDisabled(TRUE);
        widget.actionMuteMainRx->setChecked(FALSE);
        actionMuteMainRx();
        widget.vfoFrame->uncheckSubRx();
    } else {
        subRx=TRUE;
        widget.actionMuteSubRx->setChecked(FALSE);
        actionMuteSubRx();
        int samplerate = widget.spectrumView->samplerate();
//qDebug()<<Q_FUNC_INFO<<": The value of sample rate = "<<samplerate<<", The state of subRx = "<<subRx;
//qDebug()<<Q_FUNC_INFO<<": band.getFrequency = "<<band.getFrequency();
//qDebug()<<Q_FUNC_INFO<<": subRxFrequency = "<<subRxFrequency;
        widget.vfoFrame->checkSubRx(subRxFrequency, samplerate);
        // check frequency in range
        long long frequency=band.getFrequency();
        if ((subRxFrequency < (frequency - (samplerate / 2))) || (subRxFrequency > (frequency + (samplerate / 2)))) {
            subRxFrequency=band.getFrequency();
        }
        widget.spectrumView->setSubRxState(TRUE);
//        widget.waterfallView->setSubRxState(TRUE);
        widget.sMeterFrame->setSubRxState(TRUE);

        command.clear(); QTextStream(&command) << "SetSubRXFrequency " << frequency - subRxFrequency;
        connection.sendCommand(command);
        setSubRxPan();
        widget.actionMuteSubRx->setDisabled(FALSE);
        widget.actionMuteMainRx->setChecked(TRUE);
        actionMuteMainRx();
    }
    widget.actionSubrx->setChecked(subRx);
    command.clear(); QTextStream(&command) << "SetSubRX " << subRx;
    connection.sendCommand(command);
    frequencyMoved(0,1); //Dummy call to update frequency display
    widget.vfoFrame->refocus();
}

void UI::setSubRxGain(int gain) {
    QString command;
    subRxGain=gain;
    command.clear(); QTextStream(&command) << "SetSubRXOutputGain " << subRxGain;
    connection.sendCommand(command);

    qDebug() << command;
}

void UI::actionKeypad() {

    keypad.clear();
    keypad.show();
}

void UI::setKeypadFrequency(long long f) {
    frequencyChanged(f);
}

void UI::getBandBtn(int btn) {
    band.selectBand(btn+100);// +100 is used as a flag to indicate call came from vfo band buttons
}

void UI::quickMemStore()
{
    band.quickMemStore();
}


void UI::action160() {
    band.selectBand(BAND_160);
}

void UI::action80() {
    band.selectBand(BAND_80);
}

void UI::action60() {
    band.selectBand(BAND_60);
}

void UI::action40() {
    band.selectBand(BAND_40);
}

void UI::action30() {
    band.selectBand(BAND_30);
}

void UI::action20() {
    band.selectBand(BAND_20);
}

void UI::action17() {
    band.selectBand(BAND_17);
}

void UI::action15() {
    band.selectBand(BAND_15);
}

void UI::action12() {
    band.selectBand(BAND_12);
}

void UI::action10() {
    band.selectBand(BAND_10);
}

void UI::action6() {
    band.selectBand(BAND_6);
}

void UI::actionGen() {
    band.selectBand(BAND_GEN);
}

void UI::actionWWV() {
    band.selectBand(BAND_WWV);
}

void UI::bandChanged(int previousBand,int newBand) {
    qDebug()<<Q_FUNC_INFO<<":   previousBand, newBand = " << previousBand << "," << newBand;
    qDebug()<<Q_FUNC_INFO<<":   band.getFilter = "<<band.getFilter();

    // uncheck previous band
    switch(previousBand) {
        case BAND_160:
            widget.action160->setChecked(FALSE);
            break;
        case BAND_80:
            widget.action80->setChecked(FALSE);
            break;
        case BAND_60:
            widget.action60->setChecked(FALSE);
            break;
        case BAND_40:
            widget.action40->setChecked(FALSE);
            break;
        case BAND_30:
            widget.action30->setChecked(FALSE);
            break;
        case BAND_20:
            widget.action20->setChecked(FALSE);
            break;
        case BAND_17:
            widget.action17->setChecked(FALSE);
            break;
        case BAND_15:
            widget.action15->setChecked(FALSE);
            break;
        case BAND_12:
            widget.action12->setChecked(FALSE);
            break;
        case BAND_10:
            widget.action10->setChecked(FALSE);
            break;
        case BAND_6:
            widget.action6->setChecked(FALSE);
            break;
        case BAND_GEN:
            widget.actionGen->setChecked(FALSE);
            break;
        case BAND_WWV:
            widget.actionWWV->setChecked(FALSE);
            break;
    }

    // check new band
    switch(newBand) {
        case BAND_160:
            widget.action160->setChecked(TRUE);
            break;
        case BAND_80:
            widget.action80->setChecked(TRUE);
            break;
        case BAND_60:
            widget.action60->setChecked(TRUE);
            break;
        case BAND_40:
            widget.action40->setChecked(TRUE);
            break;
        case BAND_30:
            widget.action30->setChecked(TRUE);
            break;
        case BAND_20:
            widget.action20->setChecked(TRUE);
            break;
        case BAND_17:
            widget.action17->setChecked(TRUE);
            break;
        case BAND_15:
            widget.action15->setChecked(TRUE);
            break;
        case BAND_12:
            widget.action12->setChecked(TRUE);
            break;
        case BAND_10:
            widget.action10->setChecked(TRUE);
            break;
        case BAND_6:
            widget.action6->setChecked(TRUE);
            break;
        case BAND_GEN:
            widget.actionGen->setChecked(TRUE);
            break;
        case BAND_WWV:
            widget.actionWWV->setChecked(TRUE);
            break;
    }
    //Now select the correct band button in VFO
    widget.vfoFrame->checkBandBtn(newBand);

    // get the band setting
    mode.setMode(band.getMode());

    qDebug()<<Q_FUNC_INFO<<":   The value of band.getFilter is ... "<<band.getFilter();
    qDebug()<<Q_FUNC_INFO<<":   The value of filters.getFilter is  "<<filters.getFilter();


    widget.spectrumView->setBand(band.getStringBand()); // KD0OSS

    if(band.getFilter() != filters.getFilter()) {
        emit filterChanged(filters.getFilter(), band.getFilter());
    }
    frequency=band.getFrequency();
    int samplerate = widget.spectrumView->samplerate();
    if(subRx) {
        if ((subRxFrequency < (frequency - (samplerate / 2))) || (subRxFrequency > (frequency + (samplerate / 2)))) {
            subRxFrequency=frequency;
        }
    }

    QString command;
    command.clear(); QTextStream(&command) << "setFrequency " << frequency;
    connection.sendCommand(command);

    widget.spectrumView->setFrequency(frequency);

//    gvj code
    widget.vfoFrame->setFrequency(frequency);
    qDebug() << __FUNCTION__ << ": frequency, newBand = " << frequency << ", " << newBand;
    widget.spectrumView->setSubRxFrequency(subRxFrequency);
    widget.spectrumView->setHigh(band.getSpectrumHigh());
    widget.spectrumView->setLow(band.getSpectrumLow());
//    widget.waterfallView->setFrequency(frequency);
//    widget.waterfallView->setSubRxFrequency(subRxFrequency);
    widget.spectrumView->panadapterScene->waterfallItem->setHigh(band.getWaterfallHigh());
    widget.spectrumView->panadapterScene->waterfallItem->setLow(band.getWaterfallLow());
    widget.vfoFrame->setSubRxFrequency(subRxFrequency);


    BandLimit limits=band.getBandLimits(band.getFrequency()-(samplerate/2),band.getFrequency()+(samplerate/2));
    widget.spectrumView->setBandLimits(limits.min() + loffset,limits.max()+loffset);
    if((mode.getStringMode()=="CWU")||(mode.getStringMode()=="CWL")) frequencyChanged(frequency); //gvj dummy call to set Rx offset for cw
}

void UI::modeChanged(int previousMode,int newMode) {

    QString command;

    qDebug()<<Q_FUNC_INFO<< ":   previousMode, newMode" << previousMode << "," << newMode;
    qDebug()<<Q_FUNC_INFO<< ":   band.getFilter = "<<band.getFilter();

    // uncheck previous mode
    switch(previousMode) {
        case MODE_CWL:
            widget.actionCWL->setChecked(FALSE);
            break;
        case MODE_CWU:
            widget.actionCWU->setChecked(FALSE);
            break;
        case MODE_LSB:
            widget.actionLSB->setChecked(FALSE);
            break;
        case MODE_USB:
            widget.actionUSB->setChecked(FALSE);
            break;
        case MODE_DSB:
            widget.actionDSB->setChecked(FALSE);
            break;
        case MODE_AM:
            widget.actionAM->setChecked(FALSE);
            break;
        case MODE_SAM:
            widget.actionSAM->setChecked(FALSE);
            break;
        case MODE_FM:
            widget.actionFMN->setChecked(FALSE);
            break;
        case MODE_DIGL:
            widget.actionDIGL->setChecked(FALSE);
            break;
        case MODE_DIGU:
            widget.actionDIGU->setChecked(FALSE);
            break;
    }
    qDebug()<<Q_FUNC_INFO<<":  999: value of band.getFilter before filters.selectFilters has been called = "<<band.getFilter();
    // check the new mode and set the filters
    switch(newMode) {
        case MODE_CWL:
            widget.actionCWL->setChecked(TRUE);
            filters.selectFilters(&cwlFilters);
            break;
        case MODE_CWU:
            widget.actionCWU->setChecked(TRUE);
            filters.selectFilters(&cwuFilters);
            break;
        case MODE_LSB:
            widget.actionLSB->setChecked(TRUE);
            filters.selectFilters(&lsbFilters);
            break;
        case MODE_USB:
            widget.actionUSB->setChecked(TRUE);
            filters.selectFilters(&usbFilters);
            break;
        case MODE_DSB:
            widget.actionDSB->setChecked(TRUE);
            filters.selectFilters(&dsbFilters);
            break;
        case MODE_AM:
            widget.actionAM->setChecked(TRUE);
            filters.selectFilters(&amFilters);
            break;
        case MODE_SAM:
            widget.actionSAM->setChecked(TRUE);
            filters.selectFilters(&samFilters);
            break;
        case MODE_FM:
            widget.actionFMN->setChecked(TRUE);
            filters.selectFilters(&fmnFilters);
            break;
        case MODE_DIGL:
            widget.actionDIGL->setChecked(TRUE);
            filters.selectFilters(&diglFilters);
            break;
        case MODE_DIGU:
            widget.actionDIGU->setChecked(TRUE);
            filters.selectFilters(&diguFilters);
            break;
    }
    qDebug()<<Q_FUNC_INFO<<":  1043: value of band.getFilter after filters.selectFilters has been called = "<<band.getFilter();
    widget.spectrumView->setMode(mode.getStringMode());
//    widget.waterfallView->setMode(mode.getStringMode());
    command.clear(); QTextStream(&command) << "setMode " << mode.getMode();
    connection.sendCommand(command);
}

void UI::filtersChanged(FiltersBase* previousFilters,FiltersBase* newFilters) {

    qDebug()<<Q_FUNC_INFO<<":   newFilters->getText, newFilters->getSelected = " << newFilters->getText()<<", "<<newFilters->getSelected();
    qDebug()<<Q_FUNC_INFO<<":   band.getFilter = " <<band.getFilter();

    // uncheck old filter
    if(previousFilters!=NULL) {
        switch (previousFilters->getSelected()) {
            case 0:
                widget.actionFilter_0->setChecked(FALSE);
                break;
            case 1:
                widget.actionFilter_1->setChecked(FALSE);
                break;
            case 2:
                widget.actionFilter_2->setChecked(FALSE);
                break;
            case 3:
                widget.actionFilter_3->setChecked(FALSE);
                break;
            case 4:
                widget.actionFilter_4->setChecked(FALSE);
                break;
            case 5:
                widget.actionFilter_5->setChecked(FALSE);
                break;
            case 6:
                widget.actionFilter_6->setChecked(FALSE);
                break;
            case 7:
                widget.actionFilter_7->setChecked(FALSE);
                break;
            case 8:
                widget.actionFilter_8->setChecked(FALSE);
                break;
            case 9:
                widget.actionFilter_9->setChecked(FALSE);
                break;
            case 10:
                widget.actionFilter_10->setChecked(FALSE);
                break;
        }
    }

qDebug()<<Q_FUNC_INFO<<":   1092 band.getFilter = "<<band.getFilter()<<", modeFlag = "<<modeFlag;

    if(!modeFlag) {
        newFilters->selectFilter(band.getFilter()); //TODO Still not there yet
        qDebug()<<Q_FUNC_INFO<<":    Using the value from band.getFilter = "<<band.getFilter();
    }

    // set the filter menu text
    widget.actionFilter_0->setText(newFilters->getText(0));
    widget.actionFilter_1->setText(newFilters->getText(1));
    widget.actionFilter_2->setText(newFilters->getText(2));
    widget.actionFilter_3->setText(newFilters->getText(3));
    widget.actionFilter_4->setText(newFilters->getText(4));
    widget.actionFilter_5->setText(newFilters->getText(5));
    widget.actionFilter_6->setText(newFilters->getText(6));
    widget.actionFilter_7->setText(newFilters->getText(7));
    widget.actionFilter_8->setText(newFilters->getText(8));
    widget.actionFilter_9->setText(newFilters->getText(9));
    widget.actionFilter_10->setText(newFilters->getText(10));

    // check new filter
    if(newFilters!=NULL) {
        switch (newFilters->getSelected()) {
            case 0:
                widget.actionFilter_0->setChecked(TRUE);
                break;
            case 1:
                widget.actionFilter_1->setChecked(TRUE);
                break;
            case 2:
                widget.actionFilter_2->setChecked(TRUE);
                break;
            case 3:
                widget.actionFilter_3->setChecked(TRUE);
                break;
            case 4:
                widget.actionFilter_4->setChecked(TRUE);
                break;
            case 5:
                widget.actionFilter_5->setChecked(TRUE);
                break;
            case 6:
                widget.actionFilter_6->setChecked(TRUE);
                break;
            case 7:
                widget.actionFilter_7->setChecked(TRUE);
                break;
            case 8:
                widget.actionFilter_8->setChecked(TRUE);
                break;
            case 9:
                widget.actionFilter_9->setChecked(TRUE);
                break;
            case 10:
                widget.actionFilter_10->setChecked(TRUE);
                break;
        }
    }

    filters.selectFilter(filters.getFilter());
    widget.spectrumView->setFilter(filters.getText());
    printStatusBar(" .. Initial frequency. ");    //added by gvj
}

void UI::actionCWL() {
    modeFlag = true; //Signals menu selection of mode so we use the default filter
    mode.setMode(MODE_CWL);
    filters.selectFilters(&cwlFilters);
    band.setMode(MODE_CWL);
    frequencyChanged(frequency);  //force a recalculation of frequency offset for CW receive
    modeFlag = false;
}

void UI::actionCWU() {
    modeFlag = true;
    mode.setMode(MODE_CWU);
    filters.selectFilters(&cwuFilters);
    band.setMode(MODE_CWU);
    frequencyChanged(frequency);
    modeFlag = false;
}

void UI::actionLSB() {
    modeFlag = true;
    mode.setMode(MODE_LSB);
    filters.selectFilters(&lsbFilters);
    band.setMode(MODE_LSB);
    frequencyChanged(frequency);
    modeFlag = false;
}

void UI::actionUSB() {
    modeFlag = true;
    mode.setMode(MODE_USB);
    filters.selectFilters(&usbFilters);
    band.setMode(MODE_USB);
    frequencyChanged(frequency);
    modeFlag = false;
}

void UI::actionDSB() {
    modeFlag = true;
    mode.setMode(MODE_DSB);
    filters.selectFilters(&dsbFilters);
    band.setMode(MODE_DSB);
    frequencyChanged(frequency);
    modeFlag = false;
}

void UI::actionAM() {
    modeFlag=true;
    mode.setMode(MODE_AM);
    filters.selectFilters(&amFilters);
    band.setMode(MODE_AM);
    frequencyChanged(frequency);
    modeFlag = false;
}

void UI::actionSAM() {
    modeFlag = true;
    mode.setMode(MODE_SAM);
    filters.selectFilters(&samFilters);
    band.setMode(MODE_SAM);
    frequencyChanged(frequency);
    modeFlag = false;
}

void UI::actionFMN() {
    modeFlag = true;
    mode.setMode(MODE_FM);
    filters.selectFilters(&fmnFilters);
    band.setMode(MODE_FM);
    frequencyChanged(frequency);
    modeFlag = false;
}

void UI::actionDIGL() {
    modeFlag = true;
    mode.setMode(MODE_DIGL);
    filters.selectFilters(&diglFilters);
    band.setMode(MODE_DIGL);
    frequencyChanged(frequency);
    modeFlag = false;
}

void UI::actionDIGU() {
    modeFlag = true;
    mode.setMode(MODE_DIGU);
    filters.selectFilters(&diguFilters);
    band.setMode(MODE_DIGU);
    frequencyChanged(frequency);
    modeFlag = false;
}

void UI::actionFilter0() {
    filters.selectFilter(0);
}

void UI::actionFilter1() {
    filters.selectFilter(1);
}

void UI::actionFilter2() {
    filters.selectFilter(2);
}

void UI::actionFilter3() {
    filters.selectFilter(3);
}

void UI::actionFilter4() {
    filters.selectFilter(4);
}

void UI::actionFilter5() {
    filters.selectFilter(5);
}

void UI::actionFilter6() {
    filters.selectFilter(6);
}

void UI::actionFilter7() {
    filters.selectFilter(7);
}

void UI::actionFilter8() {
    filters.selectFilter(8);
}

void UI::actionFilter9() {
    filters.selectFilter(9);
}

void UI::actionFilter10() {
    filters.selectFilter(10);
}

void UI::filterChanged(int previousFilter,int newFilter) {
    QString command;

    qDebug()<<Q_FUNC_INFO<< ":   1252 previousFilter, newFilter" << previousFilter << ":" << newFilter;

    int low,high;
    switch(previousFilter) {
        case 0:
            widget.actionFilter_0->setChecked(FALSE);
            break;
        case 1:
            widget.actionFilter_1->setChecked(FALSE);
            break;
        case 2:
            widget.actionFilter_2->setChecked(FALSE);
            break;
        case 3:
            widget.actionFilter_3->setChecked(FALSE);
            break;
        case 4:
            widget.actionFilter_4->setChecked(FALSE);
            break;
        case 5:
            widget.actionFilter_5->setChecked(FALSE);
            break;
        case 6:
            widget.actionFilter_6->setChecked(FALSE);
            break;
        case 7:
            widget.actionFilter_7->setChecked(FALSE);
            break;
        case 8:
            widget.actionFilter_8->setChecked(FALSE);
            break;
        case 9:
            widget.actionFilter_9->setChecked(FALSE);
            break;
        case 10:
            widget.actionFilter_10->setChecked(FALSE);
            break;
    }

    switch(newFilter) {
        case 0:
            widget.actionFilter_0->setChecked(TRUE);
            break;
        case 1:
            widget.actionFilter_1->setChecked(TRUE);
            break;
        case 2:
            widget.actionFilter_2->setChecked(TRUE);
            break;
        case 3:
            widget.actionFilter_3->setChecked(TRUE);
            break;
        case 4:
            widget.actionFilter_4->setChecked(TRUE);
            break;
        case 5:
            widget.actionFilter_5->setChecked(TRUE);
            break;
        case 6:
            widget.actionFilter_6->setChecked(TRUE);
            break;
        case 7:
            widget.actionFilter_7->setChecked(TRUE);
            break;
        case 8:
            widget.actionFilter_8->setChecked(TRUE);
            break;
        case 9:
            widget.actionFilter_9->setChecked(TRUE);
            break;
        case 10:
            widget.actionFilter_10->setChecked(TRUE);
            break;
    }

    if (previousFilter != 10 && newFilter == 10)
        return;

    if(mode.getMode()==MODE_CWL) {
        low=-cwPitch-filters.getLow();
        high=-cwPitch+filters.getHigh();
    } else if(mode.getMode()==MODE_CWU) {
        low=cwPitch-filters.getLow();
        high=cwPitch+filters.getHigh();
    } else {
        low=filters.getLow();
        high=filters.getHigh();
    }

    command.clear(); QTextStream(&command) << "setFilter " << low << " " << high;
    connection.sendCommand(command);
    widget.spectrumView->setFilter(low,high);
    widget.spectrumView->setFilter(filters.getText());
//    widget.waterfallView->setFilter(low,high);
    band.setFilter(newFilter);
}

void UI::variableFilter(int low, int high)
{
    QString command;

    switch(filters.getFilter()) {
    case 0:
        widget.actionFilter_0->setChecked(FALSE);
        break;
    case 1:
        widget.actionFilter_1->setChecked(FALSE);
        break;
    case 2:
        widget.actionFilter_2->setChecked(FALSE);
        break;
    case 3:
        widget.actionFilter_3->setChecked(FALSE);
        break;
    case 4:
        widget.actionFilter_4->setChecked(FALSE);
        break;
    case 5:
        widget.actionFilter_5->setChecked(FALSE);
        break;
    case 6:
        widget.actionFilter_6->setChecked(FALSE);
        break;
    case 7:
        widget.actionFilter_7->setChecked(FALSE);
        break;
    case 8:
        widget.actionFilter_8->setChecked(FALSE);
        break;
    case 9:
        widget.actionFilter_9->setChecked(FALSE);
        break;
    }

    widget.actionFilter_10->setChecked(TRUE);

    command.clear(); QTextStream(&command) << "setFilter " << low << " " << high;
    connection.sendCommand(command);
    if (filters.getFilter() != 10)
    {
        band.setFilter(10);
        filters.selectFilter(10);
    }
}

void UI::frequencyChanged(long long f) {
    QString command;
    long long freqOffset = f; //Normally no offset (only for CW Rx mode)

    frequency=f;
    if((mode.getStringMode()=="CWU")&&(!widget.vfoFrame->getPtt())){
        freqOffset-=cwPitch;
    }
    if((mode.getStringMode()=="CWL")&&(!widget.vfoFrame->getPtt())){
        freqOffset+=cwPitch;
    }
    //Send command to server
    command.clear();
    QTextStream(&command) << "setFrequency " << freqOffset;
    connection.sendCommand(command);
    //Adjust all frequency displays & Check for exiting current band
    band.setFrequency(frequency);
    widget.spectrumView->setFrequency(frequency);
    widget.vfoFrame->setFrequency(frequency);
//    widget.waterfallView->setFrequency(frequency);
}

void UI::frequencyMoved(int increment,int step) {
    QString command;

    qDebug() << __FUNCTION__ << ": increment=" << increment << " step=" << step;

    long long f;

    if(subRx) {
        long long diff;
        long long frequency = band.getFrequency();
        f=subRxFrequency-(long long)(increment*step);
//        f=subRxFrequency+(long long)(increment*step);  //Original
        int samplerate = widget.spectrumView->samplerate();
        if ((f >= (frequency - (samplerate / 2))) && (f <= (frequency + (samplerate / 2)))) {
            subRxFrequency = f;
        }
        diff = frequency - subRxFrequency;
        command.clear(); QTextStream(&command) << "SetSubRXFrequency " << diff;
        connection.sendCommand(command);
        widget.spectrumView->setSubRxFrequency(subRxFrequency);
//        widget.waterfallView->setSubRxFrequency(subRxFrequency);
        widget.vfoFrame->setSubRxFrequency(subRxFrequency);// gvj subRxFrequency
        setSubRxPan();

    } else {
        frequencyChanged(band.getFrequency()-(long long)(increment*step));
    }
}

void UI::actionANF() {
    QString command;
    command.clear(); QTextStream(&command) << "SetANF " << (widget.actionANF->isChecked()?"true":"false");
    connection.sendCommand(command);
}

void UI::actionNR() {
    QString command;
    command.clear(); QTextStream(&command) << "SetNR " << (widget.actionNR->isChecked()?"true":"false");
    connection.sendCommand(command);
}

void UI::actionNB() {
    QString command;
    command.clear(); QTextStream(&command) << "SetNB " << (widget.actionNB->isChecked()?"true":"false");
    connection.sendCommand(command);
}

void UI::actionSDROM() {
    QString command;
    command.clear(); QTextStream(&command) << "SetSDROM " << (widget.actionSDROM->isChecked()?"true":"false");
    connection.sendCommand(command);
}

void UI::actionPolyphase() {
    QString command;
    command.clear(); QTextStream(&command) << "SetSpectrumPolyphase " << (widget.actionPolyphase->isChecked()?"true":"false");
    connection.sendCommand(command);
}

void UI::actionFixed() { // KD0OSS
    if (!newDspServerCheck())
    {
        widget.actionFixed->setChecked(FALSE);
        switch(agc) {
        case AGC_LONG:
            widget.actionLong->setChecked(TRUE);
            break;
        case AGC_SLOW:
            widget.actionSlow->setChecked(TRUE);
            break;
        case AGC_MEDIUM:
            widget.actionMedium->setChecked(TRUE);
            break;
        case AGC_FAST:
            widget.actionFast->setChecked(TRUE);
            break;
        }
        return;
    }

    QString command;
    // reset the current selection
    switch(agc) {
    case AGC_FIXED:
        widget.actionFixed->setChecked(FALSE);
        break;
    case AGC_LONG:
        widget.actionLong->setChecked(FALSE);
        break;
    case AGC_SLOW:
        widget.actionSlow->setChecked(FALSE);
        break;
    case AGC_MEDIUM:
        widget.actionMedium->setChecked(FALSE);
        break;
    case AGC_FAST:
        widget.actionFast->setChecked(FALSE);
        break;
    }
    agc=AGC_FIXED;

    command.clear(); QTextStream(&command) << "SetAGC " << agc;
    connection.sendCommand(command);
    AGCTLevelChanged(widget.agcTLevelSlider->value());
}

void UI::actionSlow() {
    QString command;
    // reset the current selection
    switch(agc) {
    case AGC_FIXED: // KD0OSS
        widget.actionFixed->setChecked(FALSE);
        break;
    case AGC_LONG:
        widget.actionLong->setChecked(FALSE);
        break;
    case AGC_SLOW:
        widget.actionSlow->setChecked(FALSE);
        break;
    case AGC_MEDIUM:
        widget.actionMedium->setChecked(FALSE);
        break;
    case AGC_FAST:
        widget.actionFast->setChecked(FALSE);
        break;
    }
    agc=AGC_SLOW;

    command.clear(); QTextStream(&command) << "SetAGC " << agc;
    connection.sendCommand(command);

}

void UI::actionMedium() {
    QString command;

    // reset the current selection
    switch(agc) {
    case AGC_FIXED: // KD0OSS
        widget.actionFixed->setChecked(FALSE);
        break;
    case AGC_LONG:
        widget.actionLong->setChecked(FALSE);
        break;
    case AGC_SLOW:
        widget.actionSlow->setChecked(FALSE);
        break;
    case AGC_MEDIUM:
        widget.actionMedium->setChecked(FALSE);
        break;
    case AGC_FAST:
        widget.actionFast->setChecked(FALSE);
        break;
    }
    agc=AGC_MEDIUM;

    command.clear(); QTextStream(&command) << "SetAGC " << agc;
    connection.sendCommand(command);

}

void UI::actionFast() {
    QString command;
    // reset the current selection
    switch(agc) {
    case AGC_FIXED: // KD0OSS
        widget.actionFixed->setChecked(FALSE);
        break;
    case AGC_LONG:
        widget.actionLong->setChecked(FALSE);
        break;
    case AGC_SLOW:
        widget.actionSlow->setChecked(FALSE);
        break;
    case AGC_MEDIUM:
        widget.actionMedium->setChecked(FALSE);
        break;
    case AGC_FAST:
        widget.actionFast->setChecked(FALSE);
        break;
    }
    agc=AGC_FAST;

    command.clear(); QTextStream(&command) << "SetAGC " << agc;
    connection.sendCommand(command);
}

void UI::actionLong() {
    QString command;
    // reset the current selection
    switch(agc) {
    case AGC_FIXED: // KD0OSS
        widget.actionFixed->setChecked(FALSE);
        break;
    case AGC_LONG:
        widget.actionLong->setChecked(FALSE);
        break;
    case AGC_SLOW:
        widget.actionSlow->setChecked(FALSE);
        break;
    case AGC_MEDIUM:
        widget.actionMedium->setChecked(FALSE);
        break;
    case AGC_FAST:
        widget.actionFast->setChecked(FALSE);
        break;
    }
    agc=AGC_LONG;
    command.clear(); QTextStream(&command) << "SetAGC " << agc;
    connection.sendCommand(command);
}

void UI::setSubRxPan() {
    QString command;
    float pan;

    // set the pan relative to frequency position (only works when in stereo)
    pan=(float)(subRxFrequency-(frequency-(sampleRate/2)))/(float)sampleRate;

    if(pan<0.0) pan=0.0;
    if(pan>1.0) pan=1.0;
    command.clear(); QTextStream(&command) << "SetSubRXPan " << pan;
    connection.sendCommand(command);
}

void UI::actionMuteMainRx() {
    QString command;
    int g=gain;

    if(widget.actionMuteMainRx->isChecked()) {
        g=0;
        emit widget.ctlFrame->setAudioMuted(true);
    }
    else
        emit widget.ctlFrame->setAudioMuted(false);

    command.clear(); QTextStream(&command) << "SetRXOutputGain " << g;
    connection.sendCommand(command);
}

void UI::actionMuteSubRx() {
    QString command;
    int g=subRxGain;

    if(widget.actionMuteSubRx->isChecked()) {
        g=0;
        emit widget.ctlFrame->setAudioMuted(true);
    }
    else
        emit widget.ctlFrame->setAudioMuted(false);

    command.clear(); QTextStream(&command) << "SetSubRXOutputGain " << g;
    connection.sendCommand(command);

}

void UI::actionPreamp() {
    QString command;
    command.clear(); QTextStream(&command) << "SetPreamp " << (widget.actionPreamp->isChecked()?"on":"off");
    connection.sendCommand(command);
}

void UI::actionBandscope() {
    if(widget.actionBandscope->isChecked()) {
        if(bandscope==NULL) bandscope=new Bandscope();
        bandscope->setWindowTitle("QtRadio Bandscope");
        bandscope->show();
        bandscope->connect(configure.getHost());
    } else {
        if(bandscope!=NULL) {
            bandscope->setVisible(FALSE);
            bandscope->disconnect();
        }
    }
}

void UI::actionRecord() {
    QString command;
    command.clear(); QTextStream(&command) << "record " << (widget.actionRecord->isChecked()?"on":"off");
    connection.sendCommand(command);
}
/*
void UI::actionGain_10() {
    actionGain(10);
}

void UI::actionGain_20() {
    actionGain(20);
}

void UI::actionGain_30() {
    actionGain(30);
}

void UI::actionGain_40() {
    actionGain(40);
}

void UI::actionGain_50() {
    actionGain(50);
}

void UI::actionGain_60() {
    actionGain(60);
}

void UI::actionGain_70() {
    actionGain(70);
}

void UI::actionGain_80() {
    actionGain(80);
}

void UI::actionGain_90() {
    actionGain(90);
}

void UI::actionGain_100() {
    actionGain(100);
}
*/
void UI::actionGain(int g) {
    QString command;
//    setGain(false);
    gain=g;
    subRxGain=g;
//    setGain(true);
    command.clear(); QTextStream(&command) << "SetRXOutputGain " << g;
    connection.sendCommand(command);
    command.clear(); QTextStream(&command) << "SetSubRXOutputGain " << g;
    connection.sendCommand(command);
}

void UI::setAudioMuted(bool enabled)
{
    if (enabled)
    {
        actionGain(0);
        widget.actionMuteMainRx->setChecked(true);
        widget.actionMuteSubRx->setChecked(true);
    }
    else
    {
        actionGain(widget.ctlFrame->audioGain);
        widget.actionMuteMainRx->setChecked(false);
        widget.actionMuteSubRx->setChecked(false);
    }
}

void UI::audioGainChanged(void)
{
    actionGain(widget.ctlFrame->audioGain);
}
/*
void UI::setGain(bool state) {
    switch(gain) {
    case 10:
        widget.actionGain_10->setChecked(state);
        break;
    case 20:
        widget.actionGain_20->setChecked(state);
        break;
    case 30:
        widget.actionGain_30->setChecked(state);
        break;
    case 40:
        widget.actionGain_40->setChecked(state);
        break;
    case 50:
        widget.actionGain_50->setChecked(state);
        break;
    case 60:
        widget.actionGain_60->setChecked(state);
        break;
    case 70:
        widget.actionGain_70->setChecked(state);
        break;
    case 80:
        widget.actionGain_80->setChecked(state);
        break;
    case 90:
        widget.actionGain_90->setChecked(state);
        break;
    case 100:
        widget.actionGain_100->setChecked(state);
        break;
    }
}
*/
void UI::nrValuesChanged(int taps,int delay,double gain,double leakage) {
    QString command;
    command.clear(); QTextStream(&command) << "SetNRVals " << taps << " " << delay << " "
                                           << gain << " " << leakage;
    connection.sendCommand(command);
}

void UI::anfValuesChanged(int taps,int delay,double gain,double leakage) {
    QString command;
    command.clear(); QTextStream(&command) << "SetANFVals " << taps<< " " << delay << " "
                                           << gain << " " << leakage;
    connection.sendCommand(command);
}

void UI::nbThresholdChanged(double threshold) {
    QString command;
    command.clear(); QTextStream(&command) << "SetNBVals " << threshold;
    connection.sendCommand(command);
}

void UI::sdromThresholdChanged(double threshold) {
    QString command;
    command.clear(); QTextStream(&command) << "SetSDROMVals " << threshold;
    connection.sendCommand(command);
}

void UI::actionBookmark() {
    QString strFrequency=stringFrequency(frequency);
    bookmarkDialog.setTitle(strFrequency);
    bookmarkDialog.setBand(band.getStringBand());
    bookmarkDialog.setFrequency(strFrequency);
    bookmarkDialog.setMode(mode.getStringMode());
    bookmarkDialog.setFilter(filters.getText());
    bookmarkDialog.show();
}

void UI::addBookmark() {
    qDebug() << "addBookmark";
    Bookmark* bookmark=new Bookmark();
    bookmark->setTitle(bookmarkDialog.getTitle());
    bookmark->setBand(band.getBand());
    bookmark->setFrequency(band.getFrequency());
    bookmark->setMode(mode.getMode());
    bookmark->setFilter(filters.getFilter());
    bookmarks.add(bookmark);
    bookmarks.buildMenu(widget.menuView_Bookmarks);
}

void UI::selectBookmark(QAction* action) {
    QString command;

    bookmarks.select(action);

    band.selectBand(bookmarks.getBand());

    frequency=bookmarks.getFrequency();
    band.setFrequency(frequency);
    command.clear(); QTextStream(&command) << "setFrequency " << frequency;
    connection.sendCommand(command);

    widget.spectrumView->setFrequency(frequency);
//    widget.waterfallView->setFrequency(frequency);

//    gvj code
    widget.vfoFrame->setFrequency(frequency);

    mode.setMode(bookmarks.getMode());

    filters.selectFilter(bookmarks.getFilter());
    qDebug() << "Bookmark Filter: " << bookmarks.getFilter();
}

void UI::selectABookmark() {
/*
    int entry=bookmarksDialog->getSelected();
    if(entry>=0 && entry<bookmarks.count()) {
        selectBookmark(entry);
    }
*/
}

void UI::editBookmarks() {
    bookmarksEditDialog=new BookmarksEditDialog(this,&bookmarks);
    bookmarksEditDialog->setVisible(true);
    connect(bookmarksEditDialog,SIGNAL(bookmarkDeleted(int)),this,SLOT(bookmarkDeleted(int)));
    connect(bookmarksEditDialog,SIGNAL(bookmarkUpdated(int,QString)),this,SLOT(bookmarkUpdated(int,QString)));
    connect(bookmarksEditDialog,SIGNAL(bookmarkSelected(int)),this,SLOT(bookmarkSelected(int)));
}

void UI::bookmarkDeleted(int entry) {
    //qDebug() << "UI::bookmarkDeleted: " << entry;
    bookmarks.remove(entry);
    bookmarks.buildMenu(widget.menuView_Bookmarks);
}

void UI::bookmarkUpdated(int entry,QString title) {
    if(entry>=0 && entry<bookmarks.count()) {
        Bookmark* bookmark=bookmarks.at(entry);
        bookmark->setTitle(title);
    }
}

void UI::bookmarkSelected(int entry) {

    //qDebug() << "UI::bookmarkSelected " << entry;
    if(entry>=0 && entry<bookmarks.count()) {
        Bookmark* bookmark=bookmarks.at(entry);
        FiltersBase* filters;
//TODO Get rid of message "warning: 'filters' may be used uninitialized in this function"

        bookmarksEditDialog->setTitle(bookmark->getTitle());
        bookmarksEditDialog->setBand(band.getStringBand(bookmark->getBand()));
        bookmarksEditDialog->setFrequency(stringFrequency(bookmark->getFrequency()));
        bookmarksEditDialog->setMode(mode.getStringMode(bookmark->getMode()));

        switch(bookmark->getMode()) {
        case MODE_CWL:
            filters=&cwlFilters;
            break;
        case MODE_CWU:
            filters=&cwuFilters;
            break;
        case MODE_LSB:
            filters=&lsbFilters;
            break;
        case MODE_USB:
            filters=&usbFilters;
            break;
        case MODE_DSB:
            filters=&dsbFilters;
            break;
        case MODE_AM:
            filters=&amFilters;
            break;
        case MODE_SAM:
            filters=&samFilters;
            break;
        case MODE_FM:
            filters=&fmnFilters;
            break;
        case MODE_DIGL:
            filters=&diglFilters;
            break;
        case MODE_DIGU:
            filters=&diguFilters;
            break;
        }
        bookmarksEditDialog->setFilter(filters->getText(bookmark->getFilter()));
    } else {
        bookmarksEditDialog->setTitle("");
        bookmarksEditDialog->setBand("");
        bookmarksEditDialog->setFrequency("");
        bookmarksEditDialog->setMode("");
        bookmarksEditDialog->setFilter("");
    }
}

QString UI::stringFrequency(long long frequency) {
    QString strFrequency;
    strFrequency.sprintf("%lld.%03lld.%03lld",frequency/1000000,frequency%1000000/1000,frequency%1000);
    return strFrequency;
}

void UI::addXVTR(QString title,long long minFrequency,long long maxFrequency,long long ifFrequency,long long freq,int m,int filt) {

    qDebug()<<"UI::addXVTR"<<title;
    xvtr.add(title,minFrequency,maxFrequency,ifFrequency,freq,m,filt);

    // update the menu
    xvtr.buildMenu(widget.menuXVTR);
    configure.updateXvtrList(&xvtr);
}

void UI::deleteXVTR(int index) {
    xvtr.del(index);

    // update the menu
    xvtr.buildMenu(widget.menuXVTR);
    configure.updateXvtrList(&xvtr);
}

void UI::selectXVTR(QAction* action) {
    xvtr.select(action);
}

void UI::getMeterValue(int m, int s)
{
    int dbm=0;

    dbm = m;
    widget.sMeterFrame->meter_dbm = dbm;
    widget.sMeterFrame->sub_meter_dbm = s;
    widget.sMeterFrame->update();
}

void UI::printWindowTitle(QString message)
{
    if (message.compare("Remote disconnected")==0){
        dspversion = 0;
        dspversiontxt = "";
    }
    setWindowTitle("QtRadio - Server: " + servername + " " + configure.getHost() + "(Rx "
                   + QString::number(configure.getReceiver()) +") .. "
                   + getversionstring() +  message + "  [" + QString("Qt: %1").arg(QT_VERSION, 0, 16) + "]  29 Sep 2013"); // KD0OSS  Fixed Qt version format
    lastmessage = message;
}

void UI::printStatusBar(QString message)
{
    Frequency freqInfo; // KD0OSS Added frequency description.
    static QString description;
    static long long lastFreq;

    if (lastFreq != frequency)
        description = freqInfo.getFrequencyInfo(frequency).getDescription();

    modeInfo.setText(description + "  " + band.getStringMem()+", "+mode.getStringMode()+", "+filters.getText()+message);
    lastFreq = frequency;
}

void UI::initRigCtl ()
{
    rigCtl = new RigCtlServer ( this, this );
}

long long UI::rigctlGetFreq()
{
    return(frequency);
}

QString UI::rigctlGetMode()
{
    QString  m = mode.getStringMode();
    if(m == "CWU"){
       m="CW";
    }
    if(m == "CWL"){
       m="CWR";
    }
    return m;
}

QString UI::rigctlGetFilter()
{
    QString fwidth;
    QString  m = mode.getStringMode();
    
    if (m == "CWU"){
       return fwidth.setNum(filters.getHigh() + filters.getLow());
    }
    else
    if (m == "CWL"){
       return fwidth.setNum(filters.getHigh() + filters.getLow());
    }
    else
    return fwidth.setNum(filters.getHigh() - filters.getLow());
}

QString UI::rigctlGetVFO()
{
    return widget.vfoFrame->rigctlGetvfo();
}

void UI::rigctlSetVFOA()
{
    widget.vfoFrame->on_pBtnvfoA_clicked();
}

void UI::rigctlSetVFOB()
{
    widget.vfoFrame->on_pBtnvfoB_clicked();
}

void UI::rigctlSetFreq(long long f)
{
    frequencyChanged(f);
}

void UI::rigctlSetMode(int newmode)
{
    modeChanged(mode.getMode(), newmode);
    mode.setMode(newmode);
}

void UI::rigctlSetFilter(int newfilter)

{

    qDebug() << "UI.cpp: dl6kbg: wanted filter via hamlib: " << newfilter;
    filters.selectFilter(newfilter);
}

void UI::slaveSetMode(int m)
{
    rigctlSetMode(m);
}

void UI::slaveSetFilter(int low, int high){
    widget.spectrumView->setFilter(low,high);
//    widget.waterfallView->setFilter(low,high);
}

void UI::slaveSetZoom(int position){
    widget.zoomSpectrumSlider->setValue(position);
    widget.spectrumView->setZoom(position);
 //   widget.waterfallView->setZoom(position);
}

void UI::getBandFrequency()
{
    widget.vfoFrame->setBandFrequency(band.getFrequency());
}

void UI::vfoStepBtnClicked(int direction)
{
    long long f;
    int samplerate = widget.spectrumView->samplerate();

//qDebug()<<Q_FUNC_INFO<<": vfo up or down button clicked. Direction = "<<direction<<", samplerate = "<<samplerate;
    switch ( samplerate )
    {
        case 24000 : f = 20000; break;
        case 48000 : f = 40000; break;
        case 96000 : f = 80000; break;
        case 192000 : f = 160000; break;

        default : f = (samplerate * 8) / 10;
    }
    frequencyMoved(f, direction);
}

// The ptt service has been activated. Caller values, 0 = MOX, 1 = Tune, 2 = VOX, 3 = Extern H'ware
void UI::pttChange(int caller, bool ptt)
{
    QString command;
    static int workingMode;
    static double currentPwr; // KD0OSS

    tuning = caller;
    if (configure.getTxAllowed())
    {
        if (ptt)
        {    // Going from Rx to Tx ................
            delete widget.sMeterFrame->sMeterMain;
            widget.sMeterFrame->sMeterMain = new Meter("Main Pwr", POWMETER);
            workingMode = mode.getMode(); //Save the current mode for restoration when we finish tuning
            if (caller == 1)
            { //We have clicked the tune button so switch to AM and set carrier level
                currentPwr = (double)widget.ctlFrame->getTxPwr(); // KD0OSS
                actionAM();
             //   workingMode = mode.getMode(); //Save the current mode for restoration when we finish tuning
                // Set the AM carrier level to match the tune power slider value in a scale 0 to 1.0
                if ((dspversion >= 20120201)  && canTX && chkTX)
                {
                    command.clear(); QTextStream(&command) << "setTXAMCarrierLevel " << (double)widget.ctlFrame->getTxPwr()/100 <<" "<< configure.thisuser <<" " << configure.thispass;;
                }else
                {
                    command.clear(); QTextStream(&command) << "setTXAMCarrierLevel " << (double)widget.ctlFrame->getTxPwr()/100;
                }
                connection.sendCommand(command);
                //Mute the receiver audio and freeze the spectrum and waterfall display
                connection.setMuted(true);
                //Key the radio
                if ((dspversion >= 20130901)  && canTX && chkTX)
                {
                    command.clear(); QTextStream(&command) << "Mox " << "on " << configure.thisuser << " " << configure.thispass;
                }else
                {
                    command.clear(); QTextStream(&command) << "Mox " << "on";
                }
            }
            else
            {
                //Mute the receiver audio and freeze the spectrum and waterfall display
                connection.setMuted(true);
                //Key the radio
                if ((dspversion >= 20130901)  && canTX && chkTX)
                {
                    command.clear(); QTextStream(&command) << "Mox " << "on " << configure.thisuser << " " << configure.thispass;
                }else
                {
                    command.clear(); QTextStream(&command) << "Mox " << "on";
                }
            }
            connection.sendCommand(command);
            widget.vfoFrame->pttChange(ptt); //Update the VFO to reflect that we are transmitting
            connect(audioinput,SIGNAL(mic_update_level(qreal)),widget.ctlFrame,SLOT(update_mic_level(qreal))); // KD0OSS
            txNow = true; // KD0OSS
        } else
        {    // Going from Tx to Rx .................
            delete widget.sMeterFrame->sMeterMain;
            widget.sMeterFrame->sMeterMain = new Meter("Main Rx", SIGMETER);
            if (caller == 1)
            {
                //Send signal to sdr to go to Rx
                if ((dspversion >= 20130901)  && canTX && chkTX)
                {
                    command.clear(); QTextStream(&command) << "Mox " << "off " << configure.thisuser << " " << configure.thispass;
                }else
                {
                    command.clear(); QTextStream(&command) << "Mox " << "off";
                }
                connection.sendCommand(command);

                //Restore AM carrier level to previous level. // KD0OSS
                if ((dspversion >= 20120201) && canTX && chkTX)
                {
                    command.clear(); QTextStream(&command) << "setTXAMCarrierLevel " << currentPwr/100 <<" " << configure.thisuser <<" " << configure.thispass;
                }else
                {
                    command.clear(); QTextStream(&command) << "setTXAMCarrierLevel " << currentPwr/100;
                }
                connection.sendCommand(command);
                //Restore the mode back to original before tuning
                if (workingMode != MODE_AM) // KD0OSS
                {
                    switch (workingMode)
                    {
                    case MODE_CWL: actionCWL(); break;
                    case MODE_CWU: actionCWU(); break;
                    case MODE_LSB: actionLSB(); break;
                    case MODE_USB: actionUSB(); break;
                    case MODE_DSB: actionDSB(); break;
                    case MODE_AM:  actionAM();  break;
                    case MODE_SAM: actionSAM(); break;
                    case MODE_FM:  actionFMN(); break;
                    case MODE_DIGL:actionDIGL();break;
                    case MODE_DIGU:actionDIGU();break;
                    }
                }
            }
            else
            {
                //Send signal to sdr to go to Rx
                if ((dspversion >= 20130901)  && canTX && chkTX)
                {
                    command.clear(); QTextStream(&command) << "Mox " << "off " << configure.thisuser << " " << configure.thispass;
                }else
                {
                    command.clear(); QTextStream(&command) << "Mox " << "off";
                }
                connection.sendCommand(command);
            }
            txNow = false; // KD0OSS

            //Un-mute the receiver audio
            connection.setMuted(false);
            widget.vfoFrame->pttChange(ptt); //Set band select buttons etc. to Rx state on VFO
            disconnect(audioinput,SIGNAL(mic_update_level(qreal)),widget.ctlFrame,SLOT(update_mic_level(qreal))); // KD0OSS
        }
    } else widget.ctlFrame->clearMoxBtn();
}

void UI::pwrSlider_valueChanged(double pwr)
{
    QString command;
    if (dspversion >= 20120201 && canTX && chkTX){
        command.clear(); QTextStream(&command) << "setTXAMCarrierLevel " << pwr << " " << configure.thisuser << " " << configure.thispass;;
    }else{
        command.clear(); QTextStream(&command) << "setTXAMCarrierLevel " << pwr;
    }
    connection.sendCommand(command);
}

void UI::actionConnectNow(QString IP)
{
    qDebug() << "Connect Slot:"  << IP;
    if (isConnected == false)
    {
        QuickIP = IP;
        configure.addHost(IP);
        connection.connect(IP, DSPSERVER_BASE_PORT+configure.getReceiver());
        widget.spectrumView->setReceiver(configure.getReceiver());
    }else{
        QMessageBox msgBox;
        msgBox.setText("Already Connected to a server!\nDisconnect first.");
        msgBox.exec();
    }
}

void UI::actionSquelch() {
    if(squelch) {
        squelch=false;
        QString command;
        command.clear(); QTextStream(&command) << "SetSquelchState off";
        connection.sendCommand(command);
        widget.spectrumView->setSquelch(false);
        widget.actionSquelchEnable->setChecked(false);
    } else {
        squelch=true;
        QString command;
        command.clear(); QTextStream(&command) << "SetSquelchVal " << squelchValue;
        connection.sendCommand(command);
        command.clear(); QTextStream(&command) << "SetSquelchState on";
        connection.sendCommand(command);
        widget.spectrumView->setSquelch(true);
        widget.spectrumView->setSquelchVal(squelchValue);
        widget.actionSquelchEnable->setChecked(true);
    }

}

void UI::actionSquelchReset() {
    squelchValue=-100;
    if(squelch) {
        QString command;
        command.clear(); QTextStream(&command) << "SetSquelchVal "<<squelchValue;
        connection.sendCommand(command);
        widget.spectrumView->setSquelchVal(squelchValue);
    }
}

void UI::squelchValueChanged(int val) {
    squelchValue=squelchValue+val;
    if(squelch) {
        QString command;
        command.clear(); QTextStream(&command) << "SetSquelchVal "<<squelchValue;
        connection.sendCommand(command);
        widget.spectrumView->setSquelchVal(squelchValue);
    }
}


void UI::setRTP(bool state) {
    useRTP=state;
}


QString UI::getversionstring(){
    QString str;
    if( dspversion != 0){
      str.setNum(dspversion);
      str.prepend("  (Remote = ");
      str.append("  ");
      str.append(dspversiontxt);
      str.append(")  ");
    }else{
      str ="";
    }
    return str;
}

void UI::setdspversion(long ver, QString vertxt){
    dspversion = ver;
    dspversiontxt = vertxt;
    printWindowTitle(lastmessage);

}

void UI::setservername( QString sname){
    servername = sname;
    printWindowTitle(lastmessage);
    if(!configure.setPasswd(servername)){
        configure.thisuser = "None";
        configure.thispass= "None";
    }
}

void UI::RxIQcheckChanged(bool state)
{
    QString command;

    command.clear(); QTextStream(&command) << "SetIQEnable " << (state ? "true":"false");
    connection.sendCommand(command);
}

void UI::RxIQspinChanged(double num)
{
    QString command;
    bool temp;

    temp = configure.getRxIQcheckboxState();
    // Turn off RXIQMu
    command.clear(); QTextStream(&command) << "SetIQEnable " << "false";
    connection.sendCommand(command);
    // Set the value of RxIQMu
    if(configure.getRxIQdivCheckBoxState()) num=num/100;
    command.clear(); QTextStream(&command) << "RxIQmuVal " << num;
    connection.sendCommand(command);
    //If checked to be on, then turn it back on
    if(temp) {
        command.clear(); QTextStream(&command) << "SetIQEnable " << "true";
        connection.sendCommand(command);
    }
}

void UI::cwPitchChanged(int arg1)
{
    cwPitch = arg1;
    if(isConnected){
        filters.selectFilter(filters.getFilter()); //Dummy call to centre filter on tone
        frequencyChanged(frequency); //Dummy call to set freq into correct place in filter
    }
}

void UI::setRxIQPhase(double value)  // KD0OSS
{
    QString command;

    command.clear(); QTextStream(&command) << "rxiqphasecorrectval " << value;
    connection.sendCommand(command);

    qDebug()<<Q_FUNC_INFO<<":   The value of Rx IQ Phase = "<<value;
}

void UI::setRxIQGain(double value)  // KD0OSS
{
    QString command;

    command.clear(); QTextStream(&command) << "rxiqgaincorrectval " << value;
    connection.sendCommand(command);

    qDebug()<<Q_FUNC_INFO<<":   The value of Rx IQ Gain = "<<value;
}

void UI::setCanTX(bool tx){
    canTX = tx;
    emit HideTX(tx);
}

void UI::setChkTX(bool chk){
   chkTX = true;
   infotick2 = 0;
}

void UI::setTxIQPhase(double value)  // KD0OSS
{
    QString command;

    command.clear(); QTextStream(&command) << "txiqphasecorrectval " << value;
    connection.sendCommand(command);

    qDebug()<<Q_FUNC_INFO<<":   The value of Tx IQ Phase = "<<value;
}

void UI::setTxIQGain(double value)  // KD0OSS
{
    QString command;

    command.clear(); QTextStream(&command) << "txiqgaincorrectval " << value;
    connection.sendCommand(command);

    qDebug()<<Q_FUNC_INFO<<":   The value of Tx IQ Gain = "<<value;
}

/*
void UI::testSliderChange(int value)
{
    QString command;

    command.clear(); QTextStream(&command) << "testSlider " << value;
    connection.sendCommand(command);

    qDebug()<<Q_FUNC_INFO<<":   The value of the slider = "<<value;
}

void UI::testButtonClick(bool state)
{
    QString command;

    command.clear(); QTextStream(&command) << "testbutton " << (state ? "true":"false");
    connection.sendCommand(command);

    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

//>>>>>>> Connected test controls to main UI
*/
 void UI::resetbandedges(double offset)
 {
     loffset= offset;
     BandLimit limits=band.getBandLimits(band.getFrequency()-(widget.spectrumView->samplerate()/2),band.getFrequency()+(widget.spectrumView->samplerate()/2));
     widget.spectrumView->setBandLimits(limits.min() + loffset,limits.max()+loffset);
     qDebug()<<"loffset = "<<loffset;
 }

//
// activate remote hardware control panel
// if invoked with name length == 0, close the hardware dialog deleting the object
// and resetting the pointer to zero
//

void UI :: hardware (QString answer)
{
   HardwareFactory :: Instance() .processAnswer (answer, &connection, this);
}


void UI :: setHwDlg(DlgHardware *p)
{
   if (pHwDlg) {
      pHwDlg->close(); 
      delete pHwDlg;
   }
   pHwDlg = p;
}

void UI :: rmHwDlg()
{
   if (pHwDlg) {
      pHwDlg->close(); 
      delete pHwDlg;
      pHwDlg = 0;
   }
}


void UI::on_zoomSpectrumSlider_sliderMoved(int position)
{
    QString command;

    if (widget.zoomSampRadio->isChecked()) // KD0OSS
    {
        command.clear(); QTextStream(&command) << "zoom " << position;
        connection.sendCommand(command);
        sampleZoomLevel = position;
    }
    else
        viewZoomLevel = position;
    widget.spectrumView->setZoom(position);
}

void UI::rigSetPTT(int enabled){
    if (enabled){
       widget.ctlFrame->RigCtlTX(true);
    }else{
       widget.ctlFrame->RigCtlTX(false);
    }
}

void UI::rxDCBlockChanged(bool state) {  // KD0OSS
        QString command;
        command.clear(); QTextStream(&command) << "setrxdcblock " << state;
//        qDebug("%s", command);
        connection.sendCommand(command);
        qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::rxDCBlockGainChanged(int value) {  // KD0OSS
        QString command;
        command.clear(); QTextStream(&command) << "setrxdcblockgain " << value;
//        qDebug("%s", command);
        connection.sendCommand(command);
        qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::txDCBlockChanged(bool state) {  // KD0OSS
        QString command;
        command.clear(); QTextStream(&command) << "settxdcblock " << state;
//        qDebug("%s", command);
        connection.sendCommand(command);
        qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::windowTypeChanged(int type) {
        QString command;
        command.clear(); QTextStream(&command) << "setwindow " << type;
//        qDebug("%s", command);
        connection.sendCommand(command);
        qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::setPwsMode(int mode)  // KD0OSS
{
    QString command;
    command.clear(); QTextStream(&command) << "setpwsmode " << mode;
//        qDebug("%s", command);
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;

    switch (mode)
    {
    case 0:
        widget.actionPWS_Post_Filter->setChecked(TRUE);
        break;
    case 1:
        widget.actionPWS_Pre_Filter->setChecked(TRUE);
        break;
    case 2:
        widget.actionPWS_Semi_Raw->setChecked(TRUE);
        break;
    case 3:
        widget.actionPWS_Post_Det->setChecked(TRUE);
        break;
    }
}

void UI::actionPwsMode0()   // KD0OSS
{
        QString command;
        command.clear(); QTextStream(&command) << "setpwsmode " << 0;
//        qDebug("%s", command);
        connection.sendCommand(command);
        qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;

        widget.actionPWS_Pre_Filter->setChecked(FALSE);
        widget.actionPWS_Semi_Raw->setChecked(FALSE);
        widget.actionPWS_Post_Det->setChecked(FALSE);
        pwsmode = 0;
}

void UI::actionPwsMode1()   // KD0OSS
{
        QString command;
        command.clear(); QTextStream(&command) << "setpwsmode " << 1;
//        qDebug("%s", command);
        connection.sendCommand(command);
        qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;

        widget.actionPWS_Post_Filter->setChecked(FALSE);
        widget.actionPWS_Semi_Raw->setChecked(FALSE);
        widget.actionPWS_Post_Det->setChecked(FALSE);
        pwsmode = 1;
}

void UI::actionPwsMode2()   // KD0OSS
{
        QString command;
        command.clear(); QTextStream(&command) << "setpwsmode " << 2;
//        qDebug("%s", command);
        connection.sendCommand(command);
        qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;

        widget.actionPWS_Pre_Filter->setChecked(FALSE);
        widget.actionPWS_Post_Filter->setChecked(FALSE);
        widget.actionPWS_Post_Det->setChecked(FALSE);
        pwsmode = 2;
}

void UI::actionPwsMode3()   // KD0OSS
{
        QString command;
        command.clear(); QTextStream(&command) << "setpwsmode " << 3;
//        qDebug("%s", command);
        connection.sendCommand(command);
        qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;

        widget.actionPWS_Pre_Filter->setChecked(FALSE);
        widget.actionPWS_Semi_Raw->setChecked(FALSE);
        widget.actionPWS_Post_Filter->setChecked(FALSE);
        pwsmode = 3;
}

void UI::AGCTLevelChanged(int level)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "setfixedagc " << level;
//        qDebug("%s", command);
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
    widget.agcTLevelLabel->setText(QString("%1").arg(level));
}

void UI::enableRxEq(bool enable)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "setrxgreqcmd " << enable;
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::enableTxEq(bool enable)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "settxgreqcmd " << enable;
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::addNotchFilter(void)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    if (notchFilterIndex >= 9)
    {
        QMessageBox::warning(this, "Tracking Notch Filter Error", "Maximum of 9 notch filters reached!");
        return;
    }
    widget.tnfButton->setChecked(true);
    widget.spectrumView->addNotchFilter(notchFilterIndex++);
}

void UI::removeNotchFilter(void)
{
    if (!newDspServerCheck()) return;

    notchFilterIndex--;
    if (notchFilterIndex < 0)
        notchFilterIndex = 0;
    if (notchFilterIndex == 0)
        widget.tnfButton->setChecked(false);
}

void UI::setSampleZoom(bool enable) // KD0OSS
{
    QString command;

    if (enable)
    {
        command.clear(); QTextStream(&command) << "zoom " << sampleZoomLevel;
        connection.sendCommand(command);
        widget.zoomSpectrumSlider->setValue(sampleZoomLevel);
    }
    else
        widget.zoomSpectrumSlider->setValue(viewZoomLevel);
    widget.spectrumView->sampleZoom = enable;
}

void UI::statusMessage(QString message)
{
    widget.statusbar->showMessage(message);
}

void UI::masterButtonClicked(void)
{
    connection.sendCommand("setMaster " + configure.thisuser + " " + configure.thispass);
}

bool UI::newDspServerCheck(void)
{
    if (dspversion >= 20130609 || dspversion == 0)
        return true;
    else
    {
        QMessageBox::warning(this, "Advanced Features Error", "DSP server version 20130609 or greater required.");
        return false;
    }
}

void UI::agcSlopeChanged(int value)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "setrxagcslope " << value;
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::agcMaxGainChanged(int value)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "setrxagcmaxgain " << value;
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::agcAttackChanged(int value)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "setrxagcattack " << value;
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::agcDecayChanged(int value)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "setrxagcdecay " << value;
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::agcHangChanged(int value)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "setrxagchang " << value;
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::agcHangThreshChanged(int value)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "setrxagchangthreshold " << value;
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::agcFixedGainChanged(int value)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "setfixedagc " << value;
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::levelerStateChanged(int value)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "settxlevelerstate " << value;
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::levelerMaxGainChanged(int value)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "settxlevelermaxgain " << value;
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::levelerAttackChanged(int value)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "settxlevelerattack " << value;
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::levelerDecayChanged(int value)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "settxlevelerdecay " << value;
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::levelerHangChanged(int value)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "settxlevelerhang " << value;
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::alcStateChanged(int value)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "settxalcstate " << value;
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::alcDecayChanged(int value)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "settxalcdecay " << value;
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::alcAttackChanged(int value)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "settxalcattack " << value;
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}

void UI::alcHangChanged(int value)   // KD0OSS
{
    if (!newDspServerCheck()) return;

    QString command;
    command.clear(); QTextStream(&command) << "settxalchang " << value;
    connection.sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is "<< command;
}
