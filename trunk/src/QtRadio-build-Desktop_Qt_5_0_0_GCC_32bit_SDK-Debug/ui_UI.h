/********************************************************************************
** Form generated from reading UI file 'UI.ui'
**
** Created: Sun Jan 13 22:02:27 2013
**      by: Qt User Interface Compiler version 5.0.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_H
#define UI_UI_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSlider>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include <vfo.h>
#include "Spectrum.h"
#include "Waterfall.h"
#include "ctl.h"
#include "smeter.h"

QT_BEGIN_NAMESPACE

class Ui_UI
{
public:
    QAction *action160;
    QAction *action80;
    QAction *action60;
    QAction *action40;
    QAction *action30;
    QAction *action20;
    QAction *action17;
    QAction *action15;
    QAction *action12;
    QAction *action10;
    QAction *action6;
    QAction *actionGen;
    QAction *actionWWV;
    QAction *actionLSB;
    QAction *actionUSB;
    QAction *actionDSB;
    QAction *actionCWL;
    QAction *actionCWU;
    QAction *actionAM;
    QAction *actionFMN;
    QAction *actionFilter_0;
    QAction *actionFilter_1;
    QAction *actionFilter_2;
    QAction *actionFilter_3;
    QAction *actionFilter_4;
    QAction *actionFilter_5;
    QAction *actionFilter_6;
    QAction *actionFilter_7;
    QAction *actionFilter_8;
    QAction *actionFilter_9;
    QAction *actionANF;
    QAction *actionNR;
    QAction *actionNB;
    QAction *actionSlow;
    QAction *actionMedium;
    QAction *actionFast;
    QAction *actionLong;
    QAction *actionSAM;
    QAction *actionDIGL;
    QAction *actionDIGU;
    QAction *actionConfigure;
    QAction *actionQuit;
    QAction *actionRx;
    QAction *actionConnect;
    QAction *action127_0_0_1;
    QAction *actionEdit;
    QAction *actionConnectToServer;
    QAction *actionDisconnectFromServer;
    QAction *actionSubrx;
    QAction *actionConfig;
    QAction *actionMuteMainRx;
    QAction *actionMuteSubRx;
    QAction *actionPreamp;
    QAction *actionDither;
    QAction *actionRandom;
    QAction *actionBandscope;
    QAction *actionAbout;
    QAction *actionRecord;
    QAction *actionGain_10;
    QAction *actionGain_20;
    QAction *actionGain_30;
    QAction *actionGain_40;
    QAction *actionGain_50;
    QAction *actionGain_60;
    QAction *actionGain_70;
    QAction *actionGain_80;
    QAction *actionGain_90;
    QAction *actionGain_100;
    QAction *actionBookmark;
    QAction *actionEditBookmarks;
    QAction *actionBookmarkThisFrequency;
    QAction *actionXVTRConfigure;
    QAction *action123;
    QAction *actionSDROM;
    QAction *actionPolyphase;
    QAction *actionKeypad;
    QAction *actionQuick_Server_List;
    QAction *actionSquelchEnable;
    QAction *actionSquelchReset;
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    sMeter *sMeterFrame;
    vfo *vfoFrame;
    Ctl *ctlFrame;
    Spectrum *spectrumFrame;
    QSlider *zoomSpectrumSlider;
    Waterfall *waterfallFrame;
    QMenuBar *menubar;
    QMenu *menuBand;
    QMenu *menuXVTR;
    QMenu *menuMode;
    QMenu *menuFilter;
    QMenu *menuNoise_Reduction;
    QMenu *menuAGC;
    QMenu *menuReceiver;
    QMenu *menuAudio;
    QMenu *menuHardware;
    QMenu *menuHelp;
    QMenu *menuBookmarks;
    QMenu *menuView_Bookmarks;
    QMenu *menuSpectrum;
    QMenu *menuSquelch;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *UI)
    {
        if (UI->objectName().isEmpty())
            UI->setObjectName(QStringLiteral("UI"));
        UI->resize(991, 470);
        action160 = new QAction(UI);
        action160->setObjectName(QStringLiteral("action160"));
        action160->setCheckable(true);
        action80 = new QAction(UI);
        action80->setObjectName(QStringLiteral("action80"));
        action80->setCheckable(true);
        action60 = new QAction(UI);
        action60->setObjectName(QStringLiteral("action60"));
        action60->setCheckable(true);
        action40 = new QAction(UI);
        action40->setObjectName(QStringLiteral("action40"));
        action40->setCheckable(true);
        action30 = new QAction(UI);
        action30->setObjectName(QStringLiteral("action30"));
        action30->setCheckable(true);
        action20 = new QAction(UI);
        action20->setObjectName(QStringLiteral("action20"));
        action20->setCheckable(true);
        action17 = new QAction(UI);
        action17->setObjectName(QStringLiteral("action17"));
        action17->setCheckable(true);
        action15 = new QAction(UI);
        action15->setObjectName(QStringLiteral("action15"));
        action15->setCheckable(true);
        action12 = new QAction(UI);
        action12->setObjectName(QStringLiteral("action12"));
        action12->setCheckable(true);
        action10 = new QAction(UI);
        action10->setObjectName(QStringLiteral("action10"));
        action10->setCheckable(true);
        action6 = new QAction(UI);
        action6->setObjectName(QStringLiteral("action6"));
        action6->setCheckable(true);
        actionGen = new QAction(UI);
        actionGen->setObjectName(QStringLiteral("actionGen"));
        actionGen->setCheckable(true);
        actionWWV = new QAction(UI);
        actionWWV->setObjectName(QStringLiteral("actionWWV"));
        actionWWV->setCheckable(true);
        actionLSB = new QAction(UI);
        actionLSB->setObjectName(QStringLiteral("actionLSB"));
        actionLSB->setCheckable(true);
        actionUSB = new QAction(UI);
        actionUSB->setObjectName(QStringLiteral("actionUSB"));
        actionUSB->setCheckable(true);
        actionDSB = new QAction(UI);
        actionDSB->setObjectName(QStringLiteral("actionDSB"));
        actionDSB->setCheckable(true);
        actionCWL = new QAction(UI);
        actionCWL->setObjectName(QStringLiteral("actionCWL"));
        actionCWL->setCheckable(true);
        actionCWU = new QAction(UI);
        actionCWU->setObjectName(QStringLiteral("actionCWU"));
        actionCWU->setCheckable(true);
        actionAM = new QAction(UI);
        actionAM->setObjectName(QStringLiteral("actionAM"));
        actionAM->setCheckable(true);
        actionFMN = new QAction(UI);
        actionFMN->setObjectName(QStringLiteral("actionFMN"));
        actionFMN->setCheckable(true);
        actionFilter_0 = new QAction(UI);
        actionFilter_0->setObjectName(QStringLiteral("actionFilter_0"));
        actionFilter_0->setCheckable(true);
        actionFilter_1 = new QAction(UI);
        actionFilter_1->setObjectName(QStringLiteral("actionFilter_1"));
        actionFilter_1->setCheckable(true);
        actionFilter_2 = new QAction(UI);
        actionFilter_2->setObjectName(QStringLiteral("actionFilter_2"));
        actionFilter_2->setCheckable(true);
        actionFilter_3 = new QAction(UI);
        actionFilter_3->setObjectName(QStringLiteral("actionFilter_3"));
        actionFilter_3->setCheckable(true);
        actionFilter_4 = new QAction(UI);
        actionFilter_4->setObjectName(QStringLiteral("actionFilter_4"));
        actionFilter_4->setCheckable(true);
        actionFilter_5 = new QAction(UI);
        actionFilter_5->setObjectName(QStringLiteral("actionFilter_5"));
        actionFilter_5->setCheckable(true);
        actionFilter_6 = new QAction(UI);
        actionFilter_6->setObjectName(QStringLiteral("actionFilter_6"));
        actionFilter_6->setCheckable(true);
        actionFilter_7 = new QAction(UI);
        actionFilter_7->setObjectName(QStringLiteral("actionFilter_7"));
        actionFilter_7->setCheckable(true);
        actionFilter_8 = new QAction(UI);
        actionFilter_8->setObjectName(QStringLiteral("actionFilter_8"));
        actionFilter_8->setCheckable(true);
        actionFilter_9 = new QAction(UI);
        actionFilter_9->setObjectName(QStringLiteral("actionFilter_9"));
        actionFilter_9->setCheckable(true);
        actionANF = new QAction(UI);
        actionANF->setObjectName(QStringLiteral("actionANF"));
        actionANF->setCheckable(true);
        actionNR = new QAction(UI);
        actionNR->setObjectName(QStringLiteral("actionNR"));
        actionNR->setCheckable(true);
        actionNB = new QAction(UI);
        actionNB->setObjectName(QStringLiteral("actionNB"));
        actionNB->setCheckable(true);
        actionSlow = new QAction(UI);
        actionSlow->setObjectName(QStringLiteral("actionSlow"));
        actionSlow->setCheckable(true);
        actionMedium = new QAction(UI);
        actionMedium->setObjectName(QStringLiteral("actionMedium"));
        actionMedium->setCheckable(true);
        actionFast = new QAction(UI);
        actionFast->setObjectName(QStringLiteral("actionFast"));
        actionFast->setCheckable(true);
        actionLong = new QAction(UI);
        actionLong->setObjectName(QStringLiteral("actionLong"));
        actionLong->setCheckable(true);
        actionSAM = new QAction(UI);
        actionSAM->setObjectName(QStringLiteral("actionSAM"));
        actionSAM->setCheckable(true);
        actionDIGL = new QAction(UI);
        actionDIGL->setObjectName(QStringLiteral("actionDIGL"));
        actionDIGL->setCheckable(true);
        actionDIGU = new QAction(UI);
        actionDIGU->setObjectName(QStringLiteral("actionDIGU"));
        actionDIGU->setCheckable(true);
        actionConfigure = new QAction(UI);
        actionConfigure->setObjectName(QStringLiteral("actionConfigure"));
        actionQuit = new QAction(UI);
        actionQuit->setObjectName(QStringLiteral("actionQuit"));
        actionRx = new QAction(UI);
        actionRx->setObjectName(QStringLiteral("actionRx"));
        actionConnect = new QAction(UI);
        actionConnect->setObjectName(QStringLiteral("actionConnect"));
        action127_0_0_1 = new QAction(UI);
        action127_0_0_1->setObjectName(QStringLiteral("action127_0_0_1"));
        actionEdit = new QAction(UI);
        actionEdit->setObjectName(QStringLiteral("actionEdit"));
        actionConnectToServer = new QAction(UI);
        actionConnectToServer->setObjectName(QStringLiteral("actionConnectToServer"));
        actionDisconnectFromServer = new QAction(UI);
        actionDisconnectFromServer->setObjectName(QStringLiteral("actionDisconnectFromServer"));
        actionDisconnectFromServer->setEnabled(false);
        actionSubrx = new QAction(UI);
        actionSubrx->setObjectName(QStringLiteral("actionSubrx"));
        actionSubrx->setCheckable(true);
        actionConfig = new QAction(UI);
        actionConfig->setObjectName(QStringLiteral("actionConfig"));
        actionMuteMainRx = new QAction(UI);
        actionMuteMainRx->setObjectName(QStringLiteral("actionMuteMainRx"));
        actionMuteMainRx->setCheckable(true);
        actionMuteSubRx = new QAction(UI);
        actionMuteSubRx->setObjectName(QStringLiteral("actionMuteSubRx"));
        actionMuteSubRx->setCheckable(true);
        actionMuteSubRx->setEnabled(false);
        actionPreamp = new QAction(UI);
        actionPreamp->setObjectName(QStringLiteral("actionPreamp"));
        actionPreamp->setCheckable(true);
        actionDither = new QAction(UI);
        actionDither->setObjectName(QStringLiteral("actionDither"));
        actionRandom = new QAction(UI);
        actionRandom->setObjectName(QStringLiteral("actionRandom"));
        actionBandscope = new QAction(UI);
        actionBandscope->setObjectName(QStringLiteral("actionBandscope"));
        actionBandscope->setCheckable(true);
        actionAbout = new QAction(UI);
        actionAbout->setObjectName(QStringLiteral("actionAbout"));
        actionRecord = new QAction(UI);
        actionRecord->setObjectName(QStringLiteral("actionRecord"));
        actionRecord->setCheckable(true);
        actionGain_10 = new QAction(UI);
        actionGain_10->setObjectName(QStringLiteral("actionGain_10"));
        actionGain_10->setCheckable(true);
        actionGain_20 = new QAction(UI);
        actionGain_20->setObjectName(QStringLiteral("actionGain_20"));
        actionGain_20->setCheckable(true);
        actionGain_30 = new QAction(UI);
        actionGain_30->setObjectName(QStringLiteral("actionGain_30"));
        actionGain_30->setCheckable(true);
        actionGain_40 = new QAction(UI);
        actionGain_40->setObjectName(QStringLiteral("actionGain_40"));
        actionGain_40->setCheckable(true);
        actionGain_50 = new QAction(UI);
        actionGain_50->setObjectName(QStringLiteral("actionGain_50"));
        actionGain_50->setCheckable(true);
        actionGain_60 = new QAction(UI);
        actionGain_60->setObjectName(QStringLiteral("actionGain_60"));
        actionGain_60->setCheckable(true);
        actionGain_70 = new QAction(UI);
        actionGain_70->setObjectName(QStringLiteral("actionGain_70"));
        actionGain_70->setCheckable(true);
        actionGain_80 = new QAction(UI);
        actionGain_80->setObjectName(QStringLiteral("actionGain_80"));
        actionGain_80->setCheckable(true);
        actionGain_90 = new QAction(UI);
        actionGain_90->setObjectName(QStringLiteral("actionGain_90"));
        actionGain_90->setCheckable(true);
        actionGain_100 = new QAction(UI);
        actionGain_100->setObjectName(QStringLiteral("actionGain_100"));
        actionGain_100->setCheckable(true);
        actionBookmark = new QAction(UI);
        actionBookmark->setObjectName(QStringLiteral("actionBookmark"));
        actionEditBookmarks = new QAction(UI);
        actionEditBookmarks->setObjectName(QStringLiteral("actionEditBookmarks"));
        actionBookmarkThisFrequency = new QAction(UI);
        actionBookmarkThisFrequency->setObjectName(QStringLiteral("actionBookmarkThisFrequency"));
        actionXVTRConfigure = new QAction(UI);
        actionXVTRConfigure->setObjectName(QStringLiteral("actionXVTRConfigure"));
        action123 = new QAction(UI);
        action123->setObjectName(QStringLiteral("action123"));
        actionSDROM = new QAction(UI);
        actionSDROM->setObjectName(QStringLiteral("actionSDROM"));
        actionSDROM->setCheckable(true);
        actionPolyphase = new QAction(UI);
        actionPolyphase->setObjectName(QStringLiteral("actionPolyphase"));
        actionPolyphase->setCheckable(true);
        actionKeypad = new QAction(UI);
        actionKeypad->setObjectName(QStringLiteral("actionKeypad"));
        actionQuick_Server_List = new QAction(UI);
        actionQuick_Server_List->setObjectName(QStringLiteral("actionQuick_Server_List"));
        actionSquelchEnable = new QAction(UI);
        actionSquelchEnable->setObjectName(QStringLiteral("actionSquelchEnable"));
        actionSquelchEnable->setCheckable(true);
        actionSquelchReset = new QAction(UI);
        actionSquelchReset->setObjectName(QStringLiteral("actionSquelchReset"));
        centralwidget = new QWidget(UI);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        sMeterFrame = new sMeter(centralwidget);
        sMeterFrame->setObjectName(QStringLiteral("sMeterFrame"));
        sMeterFrame->setMinimumSize(QSize(150, 115));
        sMeterFrame->setMaximumSize(QSize(150, 115));
        sMeterFrame->setFrameShape(QFrame::StyledPanel);
        sMeterFrame->setFrameShadow(QFrame::Raised);

        gridLayout->addWidget(sMeterFrame, 0, 0, 1, 1);

        vfoFrame = new vfo(centralwidget);
        vfoFrame->setObjectName(QStringLiteral("vfoFrame"));
        vfoFrame->setMinimumSize(QSize(575, 115));
        vfoFrame->setMaximumSize(QSize(575, 115));
        vfoFrame->setFrameShape(QFrame::Box);
        vfoFrame->setFrameShadow(QFrame::Raised);
        vfoFrame->setLineWidth(2);

        gridLayout->addWidget(vfoFrame, 0, 1, 1, 1);

        ctlFrame = new Ctl(centralwidget);
        ctlFrame->setObjectName(QStringLiteral("ctlFrame"));
        ctlFrame->setMinimumSize(QSize(236, 115));
        ctlFrame->setMaximumSize(QSize(16777215, 115));
        ctlFrame->setFrameShape(QFrame::StyledPanel);
        ctlFrame->setFrameShadow(QFrame::Raised);

        gridLayout->addWidget(ctlFrame, 0, 2, 1, 2);

        spectrumFrame = new Spectrum(centralwidget);
        spectrumFrame->setObjectName(QStringLiteral("spectrumFrame"));
        spectrumFrame->setMinimumSize(QSize(927, 0));
        spectrumFrame->setMaximumSize(QSize(16777215, 200));
        spectrumFrame->setFrameShape(QFrame::StyledPanel);
        spectrumFrame->setFrameShadow(QFrame::Raised);

        gridLayout->addWidget(spectrumFrame, 1, 0, 1, 3);

        zoomSpectrumSlider = new QSlider(centralwidget);
        zoomSpectrumSlider->setObjectName(QStringLiteral("zoomSpectrumSlider"));
        zoomSpectrumSlider->setFocusPolicy(Qt::NoFocus);
        zoomSpectrumSlider->setMaximum(100);
        zoomSpectrumSlider->setSingleStep(5);
        zoomSpectrumSlider->setValue(0);
        zoomSpectrumSlider->setOrientation(Qt::Vertical);
        zoomSpectrumSlider->setTickPosition(QSlider::TicksBelow);

        gridLayout->addWidget(zoomSpectrumSlider, 1, 3, 2, 1);

        waterfallFrame = new Waterfall(centralwidget);
        waterfallFrame->setObjectName(QStringLiteral("waterfallFrame"));
        waterfallFrame->setMaximumSize(QSize(16777215, 16777215));
        waterfallFrame->setFrameShape(QFrame::StyledPanel);
        waterfallFrame->setFrameShadow(QFrame::Raised);

        gridLayout->addWidget(waterfallFrame, 2, 0, 1, 3);

        UI->setCentralWidget(centralwidget);
        menubar = new QMenuBar(UI);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 991, 25));
        menuBand = new QMenu(menubar);
        menuBand->setObjectName(QStringLiteral("menuBand"));
        menuXVTR = new QMenu(menuBand);
        menuXVTR->setObjectName(QStringLiteral("menuXVTR"));
        menuMode = new QMenu(menubar);
        menuMode->setObjectName(QStringLiteral("menuMode"));
        menuFilter = new QMenu(menubar);
        menuFilter->setObjectName(QStringLiteral("menuFilter"));
        menuNoise_Reduction = new QMenu(menubar);
        menuNoise_Reduction->setObjectName(QStringLiteral("menuNoise_Reduction"));
        menuAGC = new QMenu(menubar);
        menuAGC->setObjectName(QStringLiteral("menuAGC"));
        menuReceiver = new QMenu(menubar);
        menuReceiver->setObjectName(QStringLiteral("menuReceiver"));
        menuAudio = new QMenu(menubar);
        menuAudio->setObjectName(QStringLiteral("menuAudio"));
        menuHardware = new QMenu(menubar);
        menuHardware->setObjectName(QStringLiteral("menuHardware"));
        menuHelp = new QMenu(menubar);
        menuHelp->setObjectName(QStringLiteral("menuHelp"));
        menuBookmarks = new QMenu(menubar);
        menuBookmarks->setObjectName(QStringLiteral("menuBookmarks"));
        menuView_Bookmarks = new QMenu(menuBookmarks);
        menuView_Bookmarks->setObjectName(QStringLiteral("menuView_Bookmarks"));
        menuSpectrum = new QMenu(menubar);
        menuSpectrum->setObjectName(QStringLiteral("menuSpectrum"));
        menuSquelch = new QMenu(menubar);
        menuSquelch->setObjectName(QStringLiteral("menuSquelch"));
        UI->setMenuBar(menubar);
        statusbar = new QStatusBar(UI);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        statusbar->setSizeGripEnabled(false);
        UI->setStatusBar(statusbar);

        menubar->addAction(menuReceiver->menuAction());
        menubar->addAction(menuAudio->menuAction());
        menubar->addAction(menuBand->menuAction());
        menubar->addAction(menuMode->menuAction());
        menubar->addAction(menuFilter->menuAction());
        menubar->addAction(menuNoise_Reduction->menuAction());
        menubar->addAction(menuAGC->menuAction());
        menubar->addAction(menuSpectrum->menuAction());
        menubar->addAction(menuSquelch->menuAction());
        menubar->addAction(menuHardware->menuAction());
        menubar->addAction(menuBookmarks->menuAction());
        menubar->addAction(menuHelp->menuAction());
        menuBand->addAction(actionKeypad);
        menuBand->addSeparator();
        menuBand->addAction(action160);
        menuBand->addAction(action80);
        menuBand->addAction(action60);
        menuBand->addAction(action40);
        menuBand->addAction(action30);
        menuBand->addAction(action20);
        menuBand->addAction(action17);
        menuBand->addAction(action15);
        menuBand->addAction(action12);
        menuBand->addAction(action10);
        menuBand->addAction(action6);
        menuBand->addSeparator();
        menuBand->addAction(actionGen);
        menuBand->addAction(actionWWV);
        menuBand->addSeparator();
        menuBand->addAction(menuXVTR->menuAction());
        menuBand->addSeparator();
        menuMode->addAction(actionLSB);
        menuMode->addAction(actionUSB);
        menuMode->addAction(actionDSB);
        menuMode->addAction(actionCWU);
        menuMode->addAction(actionCWL);
        menuMode->addAction(actionAM);
        menuMode->addAction(actionSAM);
        menuMode->addAction(actionFMN);
        menuMode->addAction(actionDIGL);
        menuMode->addAction(actionDIGU);
        menuFilter->addAction(actionFilter_0);
        menuFilter->addAction(actionFilter_1);
        menuFilter->addAction(actionFilter_2);
        menuFilter->addAction(actionFilter_3);
        menuFilter->addAction(actionFilter_4);
        menuFilter->addAction(actionFilter_5);
        menuFilter->addAction(actionFilter_6);
        menuFilter->addAction(actionFilter_7);
        menuFilter->addAction(actionFilter_8);
        menuFilter->addAction(actionFilter_9);
        menuNoise_Reduction->addAction(actionANF);
        menuNoise_Reduction->addAction(actionNR);
        menuNoise_Reduction->addAction(actionNB);
        menuNoise_Reduction->addAction(actionSDROM);
        menuNoise_Reduction->addSeparator();
        menuAGC->addAction(actionSlow);
        menuAGC->addAction(actionMedium);
        menuAGC->addAction(actionFast);
        menuAGC->addAction(actionLong);
        menuReceiver->addAction(actionConnectToServer);
        menuReceiver->addAction(actionDisconnectFromServer);
        menuReceiver->addAction(actionQuick_Server_List);
        menuReceiver->addSeparator();
        menuReceiver->addAction(actionSubrx);
        menuReceiver->addSeparator();
        menuReceiver->addAction(actionBandscope);
        menuReceiver->addAction(actionRecord);
        menuReceiver->addAction(actionConfig);
        menuReceiver->addSeparator();
        menuAudio->addAction(actionMuteMainRx);
        menuAudio->addAction(actionMuteSubRx);
        menuAudio->addAction(actionGain_10);
        menuAudio->addAction(actionGain_20);
        menuAudio->addAction(actionGain_30);
        menuAudio->addAction(actionGain_40);
        menuAudio->addAction(actionGain_50);
        menuAudio->addAction(actionGain_60);
        menuAudio->addAction(actionGain_70);
        menuAudio->addAction(actionGain_80);
        menuAudio->addAction(actionGain_90);
        menuAudio->addAction(actionGain_100);
        menuHardware->addAction(actionPreamp);
        menuHardware->addSeparator();
        menuHardware->addAction(actionDither);
        menuHardware->addAction(actionRandom);
        menuHelp->addAction(actionAbout);
        menuBookmarks->addAction(actionBookmarkThisFrequency);
        menuBookmarks->addSeparator();
        menuBookmarks->addAction(menuView_Bookmarks->menuAction());
        menuBookmarks->addSeparator();
        menuBookmarks->addAction(actionEditBookmarks);
        menuBookmarks->addSeparator();
        menuSpectrum->addAction(actionPolyphase);
        menuSquelch->addAction(actionSquelchEnable);
        menuSquelch->addSeparator();
        menuSquelch->addAction(actionSquelchReset);

        retranslateUi(UI);

        QMetaObject::connectSlotsByName(UI);
    } // setupUi

    void retranslateUi(QMainWindow *UI)
    {
        UI->setWindowTitle(QApplication::translate("UI", "QtRadio - not connected", 0));
        action160->setText(QApplication::translate("UI", "160", 0));
        action80->setText(QApplication::translate("UI", "80", 0));
        action60->setText(QApplication::translate("UI", "60", 0));
        action40->setText(QApplication::translate("UI", "40", 0));
        action30->setText(QApplication::translate("UI", "30", 0));
        action20->setText(QApplication::translate("UI", "20", 0));
        action17->setText(QApplication::translate("UI", "17", 0));
        action15->setText(QApplication::translate("UI", "15", 0));
        action12->setText(QApplication::translate("UI", "12", 0));
        action10->setText(QApplication::translate("UI", "10", 0));
        action6->setText(QApplication::translate("UI", "6", 0));
        actionGen->setText(QApplication::translate("UI", "Gen", 0));
        actionWWV->setText(QApplication::translate("UI", "WWV", 0));
        actionLSB->setText(QApplication::translate("UI", "LSB", 0));
        actionUSB->setText(QApplication::translate("UI", "USB", 0));
        actionDSB->setText(QApplication::translate("UI", "DSB", 0));
        actionCWL->setText(QApplication::translate("UI", "CWL", 0));
        actionCWU->setText(QApplication::translate("UI", "CWU", 0));
        actionAM->setText(QApplication::translate("UI", "AM", 0));
        actionFMN->setText(QApplication::translate("UI", "FMN", 0));
        actionFilter_0->setText(QApplication::translate("UI", "Filter 0", 0));
        actionFilter_1->setText(QApplication::translate("UI", "Filter 1", 0));
        actionFilter_2->setText(QApplication::translate("UI", "Filter 2", 0));
        actionFilter_3->setText(QApplication::translate("UI", "Filter 3", 0));
        actionFilter_4->setText(QApplication::translate("UI", "Filter 4", 0));
        actionFilter_5->setText(QApplication::translate("UI", "Filter 5", 0));
        actionFilter_6->setText(QApplication::translate("UI", "Filter 6", 0));
        actionFilter_7->setText(QApplication::translate("UI", "Filter 7", 0));
        actionFilter_8->setText(QApplication::translate("UI", "Filter 8", 0));
        actionFilter_9->setText(QApplication::translate("UI", "Filter 9", 0));
        actionANF->setText(QApplication::translate("UI", "ANF", 0));
        actionNR->setText(QApplication::translate("UI", "NR", 0));
        actionNB->setText(QApplication::translate("UI", "NB", 0));
        actionSlow->setText(QApplication::translate("UI", "Slow", 0));
        actionMedium->setText(QApplication::translate("UI", "Medium", 0));
        actionFast->setText(QApplication::translate("UI", "Fast", 0));
        actionLong->setText(QApplication::translate("UI", "Long", 0));
        actionSAM->setText(QApplication::translate("UI", "SAM", 0));
        actionDIGL->setText(QApplication::translate("UI", "DIGL", 0));
        actionDIGU->setText(QApplication::translate("UI", "DIGU", 0));
        actionConfigure->setText(QApplication::translate("UI", "Configure", 0));
        actionQuit->setText(QApplication::translate("UI", "Quit", 0));
        actionRx->setText(QApplication::translate("UI", "rx 0", 0));
        actionConnect->setText(QApplication::translate("UI", "Connect", 0));
        action127_0_0_1->setText(QApplication::translate("UI", "127.0.0.1", 0));
        actionEdit->setText(QApplication::translate("UI", "Edit", 0));
        actionConnectToServer->setText(QApplication::translate("UI", "Connect", 0));
#ifndef QT_NO_TOOLTIP
        actionConnectToServer->setToolTip(QApplication::translate("UI", "Connect to a dspserver", 0));
#endif // QT_NO_TOOLTIP
        actionConnectToServer->setShortcut(QApplication::translate("UI", "C", 0));
        actionDisconnectFromServer->setText(QApplication::translate("UI", "Disconnect", 0));
        actionDisconnectFromServer->setShortcut(QApplication::translate("UI", "D", 0));
        actionSubrx->setText(QApplication::translate("UI", "Subrx", 0));
        actionSubrx->setShortcut(QApplication::translate("UI", "S", 0));
        actionConfig->setText(QApplication::translate("UI", "Configure", 0));
        actionMuteMainRx->setText(QApplication::translate("UI", "Mute Main Rx", 0));
        actionMuteSubRx->setText(QApplication::translate("UI", "Mute Sub Rx", 0));
        actionPreamp->setText(QApplication::translate("UI", "Preamp", 0));
        actionDither->setText(QApplication::translate("UI", "Dither", 0));
        actionRandom->setText(QApplication::translate("UI", "Random", 0));
        actionBandscope->setText(QApplication::translate("UI", "Bandscope", 0));
        actionAbout->setText(QApplication::translate("UI", "About", 0));
        actionRecord->setText(QApplication::translate("UI", "Record", 0));
        actionGain_10->setText(QApplication::translate("UI", "Gain 10", 0));
        actionGain_20->setText(QApplication::translate("UI", "Gain 20", 0));
        actionGain_30->setText(QApplication::translate("UI", "Gain 30", 0));
        actionGain_40->setText(QApplication::translate("UI", "Gain 40", 0));
        actionGain_50->setText(QApplication::translate("UI", "Gain 50", 0));
        actionGain_60->setText(QApplication::translate("UI", "Gain 60", 0));
        actionGain_70->setText(QApplication::translate("UI", "Gain 70", 0));
        actionGain_80->setText(QApplication::translate("UI", "Gain 80", 0));
        actionGain_90->setText(QApplication::translate("UI", "Gain 90", 0));
        actionGain_100->setText(QApplication::translate("UI", "Gain 100", 0));
        actionBookmark->setText(QApplication::translate("UI", "Bookmark this frequency", 0));
        actionEditBookmarks->setText(QApplication::translate("UI", "Edit Bookmarks", 0));
        actionBookmarkThisFrequency->setText(QApplication::translate("UI", "Bookmark This Frequency", 0));
        actionXVTRConfigure->setText(QApplication::translate("UI", "Configure", 0));
        action123->setText(QApplication::translate("UI", "123", 0));
        actionSDROM->setText(QApplication::translate("UI", "NB2", 0));
        actionPolyphase->setText(QApplication::translate("UI", "Polyphase", 0));
        actionKeypad->setText(QApplication::translate("UI", "Keypad", 0));
        actionKeypad->setShortcut(QApplication::translate("UI", "K", 0));
        actionQuick_Server_List->setText(QApplication::translate("UI", "Quick Server List  ", 0));
#ifndef QT_NO_TOOLTIP
        actionQuick_Server_List->setToolTip(QApplication::translate("UI", "Quickly select a shared SDR  server from a list of online radios", 0));
#endif // QT_NO_TOOLTIP
        actionQuick_Server_List->setShortcut(QApplication::translate("UI", "L", 0));
        actionSquelchEnable->setText(QApplication::translate("UI", "Enable", 0));
        actionSquelchReset->setText(QApplication::translate("UI", "Reset", 0));
        menuBand->setTitle(QApplication::translate("UI", "Band", 0));
        menuXVTR->setTitle(QApplication::translate("UI", "XVTR", 0));
        menuMode->setTitle(QApplication::translate("UI", "Mode", 0));
        menuFilter->setTitle(QApplication::translate("UI", "Filter", 0));
        menuNoise_Reduction->setTitle(QApplication::translate("UI", "Noise Reduction", 0));
        menuAGC->setTitle(QApplication::translate("UI", "AGC", 0));
        menuReceiver->setTitle(QApplication::translate("UI", "Receiver", 0));
        menuAudio->setTitle(QApplication::translate("UI", "Audio", 0));
        menuHardware->setTitle(QApplication::translate("UI", "Hardware", 0));
        menuHelp->setTitle(QApplication::translate("UI", "Help", 0));
        menuBookmarks->setTitle(QApplication::translate("UI", "Bookmarks", 0));
        menuView_Bookmarks->setTitle(QApplication::translate("UI", "View Bookmarks", 0));
        menuSpectrum->setTitle(QApplication::translate("UI", "Spectrum", 0));
        menuSquelch->setTitle(QApplication::translate("UI", "Squelch", 0));
    } // retranslateUi

};

namespace Ui {
    class UI: public Ui_UI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_UI_H
