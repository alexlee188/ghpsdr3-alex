#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QTabWidget>
#endif
#include "hardware_hermes.h"


HardwareHermes :: HardwareHermes (Connection *pC, QWidget *pW): DlgHardware (pC, pW)
{
    // tab Widwged
    QTabWidget *pTw = new QTabWidget(this);
    QWidget    *pRx = new QWidget();
    QWidget    *pTx = new QWidget();

    //
    // Receiver tab
    //

    // Attenuator
    QGroupBox *attGroupBox = new QGroupBox(tr("Attenuator"));
    attGroupBox->setFlat ( true );

    QRadioButton *att0Db  = new QRadioButton(tr("&0 dB"));
    QRadioButton *att10Db = new QRadioButton(tr("-&10 dB"));
    QRadioButton *att20Db = new QRadioButton(tr("-&20 dB"));
    QRadioButton *att30Db = new QRadioButton(tr("-&30 dB"));
    
    QVBoxLayout *rx_vbox = new QVBoxLayout;

    rx_vbox->addWidget(att0Db);
    rx_vbox->addWidget(att10Db);
    rx_vbox->addWidget(att20Db);
    rx_vbox->addWidget(att30Db);
    rx_vbox->addStretch(1);

    attGroupBox->setLayout(rx_vbox);

    // att slider
    pAttSlider = new QSlider(this);
    pAttSlider->setMaximum ( 0 );
    pAttSlider->setMinimum ( -31 );
    pAttSlider->setPageStep( 10 );

    // Dither
    QCheckBox *ditherCB = new QCheckBox(tr("Di&ther"));

    // Preamp
    QCheckBox *preampCB = new QCheckBox(tr("Pre&amplifier"));

    // Random
    QCheckBox *randomCB = new QCheckBox(tr("&Random"));

    // Main layout of rx tab
    QHBoxLayout *rx_grid = new QHBoxLayout;

    // add objects
    rx_grid->addWidget (attGroupBox);
    rx_grid->addWidget (pAttSlider);
    rx_grid->addWidget (ditherCB);
    rx_grid->addWidget (preampCB);
    rx_grid->addWidget (randomCB);


    // use grid object as main dialog's layout 
    pRx->setLayout(rx_grid);


    //
    // Transmitter tab
    //

    // Open Collector outputs
    QVBoxLayout *tx_vbox = new QVBoxLayout;

    QGroupBox *ocOutsGroupBox = new QGroupBox(tr("Open Collector outputs"));
    ocOutsGroupBox->setFlat ( true );

    QCheckBox *oc0  = new QCheckBox(tr("&0"));
    QCheckBox *oc1  = new QCheckBox(tr("&1"));
    QCheckBox *oc2  = new QCheckBox(tr("&2"));
    QCheckBox *oc3  = new QCheckBox(tr("&3"));
    QCheckBox *oc4  = new QCheckBox(tr("&4"));
    QCheckBox *oc5  = new QCheckBox(tr("&5"));
    
    tx_vbox->addWidget(oc0);
    tx_vbox->addWidget(oc1);
    tx_vbox->addWidget(oc2);
    tx_vbox->addWidget(oc3);
    tx_vbox->addWidget(oc4);
    tx_vbox->addWidget(oc5);
    tx_vbox->addStretch(1);

    ocOutsGroupBox->setLayout(tx_vbox);

    // Tx Driver slider    
    QVBoxLayout *vTxDriverbox = new QVBoxLayout;

    QGroupBox *txDriverGroupBox = new QGroupBox(tr("Tx Drive Level"));
    txDriverGroupBox->setFlat(true);

    pTxSlider = new QSlider(txDriverGroupBox);
    pTxSlider->setMaximum ( 255 );
    pTxSlider->setMinimum ( 0   );
    pTxSlider->setPageStep( 16  );

    vTxDriverbox->addWidget (pTxSlider);
    txDriverGroupBox->setLayout(vTxDriverbox);

    // TLV320 Line-in Gain
    QVBoxLayout *vTxLingBox = new QVBoxLayout;

    QGroupBox *txLineInGainGroupBox = new QGroupBox(tr("Line In Gain"));
    txLineInGainGroupBox->setFlat(true);

    pTxLineInGain = new QSlider();
    pTxLineInGain->setMaximum ( 32 );
    pTxLineInGain->setMinimum ( 0  );
    pTxLineInGain->setPageStep( 4  );

    vTxLingBox->addWidget (pTxLineInGain);
    txLineInGainGroupBox->setLayout(vTxLingBox);

    // Alex Tx relay
    QVBoxLayout *tx_atrvbox = new QVBoxLayout;

    QGroupBox *alexTxRelayGroupBox = new QGroupBox(tr("Tx outputs"));
    alexTxRelayGroupBox->setFlat ( true );

    QRadioButton *alexTx0  = new QRadioButton(tr("&0"));
    QRadioButton *alexTx1  = new QRadioButton(tr("&1"));
    QRadioButton *alexTx2  = new QRadioButton(tr("&2"));
    
    tx_atrvbox->addWidget(alexTx0);
    tx_atrvbox->addWidget(alexTx1);
    tx_atrvbox->addWidget(alexTx2);
    tx_atrvbox->addStretch(1);

    alexTxRelayGroupBox->setLayout(tx_atrvbox);

    // Hermes Microphone boosts
    QCheckBox *micBoost  = new QCheckBox(tr("&Mic Boost"));


    // Main layout of tx tab
    QHBoxLayout *tx_grid = new QHBoxLayout;

    // add objects
    tx_grid->addWidget (ocOutsGroupBox);
    tx_grid->addWidget (txDriverGroupBox);
    tx_grid->addWidget (txLineInGainGroupBox);
    tx_grid->addWidget (alexTxRelayGroupBox);
    tx_grid->addWidget (micBoost);

    // use grid object as main dialog's layout 
    pTx->setLayout(tx_grid);


    // general features
    setWindowTitle(tr("Hermes"));
    //resize(240, 160);

    //
    // add the tabs
    //
    pTw->addTab(pRx, QString("RX"));
    pTw->addTab(pTx, QString("TX"));
    pTw->show();

    // interconnects
    attMapper = new QSignalMapper(this);
    connect(att0Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att0Db, 0);

    connect(att10Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att10Db, 10);

    connect(att20Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att20Db, 20);

    connect(att30Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att30Db, 30);

    connect(attMapper, SIGNAL(mapped(int)), this, SLOT(attClicked(int)));

    connect(pAttSlider, SIGNAL(valueChanged(int)), this, SLOT(attSlid(int)));

    connect(ditherCB, SIGNAL(stateChanged(int)),  this, SLOT(ditherChanged(int)));
    connect(preampCB, SIGNAL(stateChanged(int)),  this, SLOT(preampChanged(int)));
    connect(randomCB, SIGNAL(stateChanged(int)),  this, SLOT(randomChanged(int)));

    // Tx Drive Level
    connect(pTxSlider, SIGNAL(valueChanged(int)), this, SLOT(txDriveSlid(int)));
    // Tx Line In Gain
    connect(pTxLineInGain, SIGNAL(valueChanged(int)), this, SLOT(txLineInGainSlid(int)));

    // Alex Tx / ANAN-10 Antenna Tx selector
    atxrMapper = new QSignalMapper(this);
    connect(alexTx0, SIGNAL(toggled(bool)), atxrMapper, SLOT(map()));
    atxrMapper->setMapping(alexTx0, 0);
    connect(alexTx1, SIGNAL(toggled(bool)), atxrMapper, SLOT(map()));
    atxrMapper->setMapping(alexTx1, 1);
    connect(alexTx2, SIGNAL(toggled(bool)), atxrMapper, SLOT(map()));
    atxrMapper->setMapping(alexTx2, 2);
    connect(atxrMapper, SIGNAL(mapped(int)), this, SLOT(atxrClicked(int)));

    // Mic boost
    connect(micBoost, SIGNAL(stateChanged(int)), this, SLOT(micBoostClicked(int)));

    // update the serial number in title bar
    QString command;
    command.clear(); QTextStream(&command) << "*getserial?";
    pConn->sendCommand (command);

    // defaults
    ditherCB->setCheckState(Qt::Checked);  // dither ON
    preampCB->setCheckState(Qt::Checked);  // preamp ON and disabled
    preampCB->setEnabled(false);
    randomCB->setCheckState(Qt::Checked);  // random ON
    att0Db->setChecked(true);              // attenuator 0 dB
    alexTx0->setChecked(true);             // Alex-Tx/ANAN-10 Tx antenna #0
}

void HardwareHermes :: attClicked(int state)
{
   if (state < 0) state = -state;
   qDebug() << "Attenuator: " << state << "dB";
   QString command;
   command.clear(); QTextStream(&command) << "*setattenuator " << state;
   pConn->sendCommand (command);

   if (pAttSlider->value() != state) {
       pAttSlider->setValue(state);
   }
}

void HardwareHermes :: atxrClicked(int state)
{
   qDebug() << "Alex Tx antenna: " << state ;
   QString command;
   command.clear(); QTextStream(&command) << "*alextxrelay " << state;
   pConn->sendCommand (command);
}

void HardwareHermes :: micBoostClicked(int state)
{
   qDebug() << "Hermes Mic boost: " << state ;
   QString command;
   command.clear(); QTextStream(&command) << "*hermesmicboost " << state;
   pConn->sendCommand (command);
}

void HardwareHermes :: attSlid(int state)
{
   if (state < 0) state = -state;
   qDebug() << "Attenuator: " << state << "dB";
   QString command;
   command.clear(); QTextStream(&command) << "*setattenuator " << state;
   pConn->sendCommand (command);
}

void HardwareHermes :: txDriveSlid(int state)
{
   if (state < 0) return;
   qDebug() << "Tx Drive Level: " << state << "dB";
   QString command;
   command.clear(); QTextStream(&command) << "*settxdrive " << state;
   pConn->sendCommand (command);
}

void HardwareHermes :: txLineInGainSlid(int state)
{
   if (state < 0) return;
   qDebug() << "Tx Line In Gain:: " << state << "dB";
   QString command;
   command.clear(); QTextStream(&command) << "*settxlineingain " << state;
   pConn->sendCommand (command);
}



void HardwareHermes :: ditherChanged(int state)
{
   qDebug() << "Dither: " << state;
   QString command;
   command.clear(); QTextStream(&command) << "*dither " << ((state==Qt::Checked) ? "on" : "off") ;
   pConn->sendCommand (command);
}

void HardwareHermes :: preampChanged(int state)
{   
   qDebug() << "Preamp: " << state;
   QString command;
   command.clear(); QTextStream(&command) << "*preamp " << ((state==Qt::Checked) ? "on" : "off") ;
   pConn->sendCommand (command);
}

void HardwareHermes :: randomChanged(int state)
{   
   qDebug() << "Random: " << state;
   QString command;
   command.clear(); QTextStream(&command) << "*random " << ((state==Qt::Checked) ? "on" : "off") ;
   pConn->sendCommand (command);
}


void HardwareHermes :: processAnswer (QStringList list)
{
    if (list[0] == "*getserial?") {
       // try to set the serial
       qDebug() << Q_FUNC_INFO<<list[2];
       // change the title bar
       QString x;
       x.clear(); 
       QTextStream(&x) << windowTitle() << " - SN: " << list[2];

       setWindowTitle(x) ;
    }

}

HardwareHermes :: ~HardwareHermes ()
{

}

namespace {
   struct RegisterHw<HardwareHermes> r("Hermes");
}
