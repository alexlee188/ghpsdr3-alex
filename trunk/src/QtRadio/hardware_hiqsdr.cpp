#include <QtGui>
#include "hardware.h"
#include "hardware_hiqsdr.h"
#include "UI.h"

HardwareHiqsdr :: HardwareHiqsdr (Connection *pC, QWidget *pW): DlgHardware (pC, pW)
{
    // Attenuator
    QGroupBox *attGroupBox = new QGroupBox(tr("Attenuator"));
    attGroupBox->setFlat ( true );

    QRadioButton *att0Db  = new QRadioButton(tr("&0 dB"));
    QRadioButton *att10Db = new QRadioButton(tr("-&10 dB"));
    QRadioButton *att20Db = new QRadioButton(tr("-&20 dB"));
    QRadioButton *att30Db = new QRadioButton(tr("-&30 dB"));
    QRadioButton *att40Db = new QRadioButton(tr("-&40 dB"));
    
    QVBoxLayout *vbox = new QVBoxLayout;

    vbox->addWidget(att0Db);
    vbox->addWidget(att10Db);
    vbox->addWidget(att20Db);
    vbox->addWidget(att30Db);
    vbox->addWidget(att40Db);
    vbox->addStretch(1);

    attGroupBox->setLayout(vbox);

    // Antenna selector
    QGroupBox *antGroupBox = new QGroupBox(tr("Antenna"));

    QRadioButton *ant0  = new QRadioButton(tr("Antenna input # &0"));
    QRadioButton *ant1  = new QRadioButton(tr("Antenna input # &1"));

    QVBoxLayout *vabox = new QVBoxLayout;

    vabox->addWidget(ant0);
    vabox->addWidget(ant1);
    antGroupBox->setLayout(vabox);

    // preselector
    QGroupBox *preselGroupBox = new QGroupBox(tr("Preselector"));
    QVBoxLayout *vpbox = new QVBoxLayout;

    for (int i = 0; i < 16; ++i) {
        char s[16];
        sprintf (s, "%d", i);
        psel[i] = new QRadioButton(tr(s));
        vpbox->addWidget(psel[i]);
    }
    preselGroupBox->setLayout(vpbox);

   // preamplifier
    QGroupBox *preampGroupBox = new QGroupBox(tr("Preamplifier"));
    QVBoxLayout *vpabox = new QVBoxLayout;
    preamp = new QCheckBox(tr("Pre&amplifier"));
    vpabox->addWidget(preamp);
    preampGroupBox->setLayout(vpabox);
    
    
    // Main layout of dialog
    QHBoxLayout *grid = new QHBoxLayout;
    
    // left sub pane
    QVBoxLayout *leftsp = new QVBoxLayout;
    leftsp->addWidget (attGroupBox);
    leftsp->addWidget (preampGroupBox);
    leftsp->addWidget (antGroupBox);
    
    // add objects to main layout
    grid->addLayout (leftsp);
    
    // add objects to main layout 
    grid->addWidget (preselGroupBox);

    // use grid obecjt as main dialog's layout 
    setLayout(grid);

    // general features
    setWindowTitle(tr("HiQSDR"));
    resize(240, 200);

    // interconnects
    attMapper = new QSignalMapper(this);
    connect(att0Db,  SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att0Db, 0);

    connect(att10Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att10Db, 10);

    connect(att20Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att20Db, 20);

    connect(att30Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att30Db, 30);

    connect(att40Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att40Db, 40);

    connect(attMapper, SIGNAL(mapped(int)), this, SLOT(attClicked(int)));

    // antenna mapper
    antMapper = new QSignalMapper(this);
    connect(ant0,  SIGNAL(toggled(bool)), antMapper, SLOT(map()));
    antMapper->setMapping(ant0, 0);

    connect(ant1, SIGNAL(toggled(bool)), antMapper, SLOT(map()));
    antMapper->setMapping(ant1, 1);

    connect(antMapper, SIGNAL(mapped(int)), this, SLOT(antClicked(int)));

    // preselector mapper
    preselMapper = new QSignalMapper(this);
    for (int i = 0; i < 16; ++i) {
        connect(psel[i],  SIGNAL(toggled(bool)), preselMapper, SLOT(map()));
        preselMapper->setMapping(psel[i], i);
    }
    connect(preselMapper, SIGNAL(mapped(int)), this, SLOT(preselClicked(int)));

    // preamplifier event connection
    connect(preamp, SIGNAL(stateChanged(int)),  this, SLOT(preampChanged(int)));
 
    // update the serial number in title bar
    QString command;
    command.clear(); QTextStream(&command) << "*getserial?";
    pConn->sendCommand (command);

    // defaults
    attenuatorVal = -1;
    antennaVal = -1;
    preampVal = 0;

    att0Db->setChecked(true);              // attenuator 0 dB
    ant0->setChecked(true);                // antenna #0
    psel[0]->setChecked(true);
    preamp->setChecked(false);             // preamplifier off

    // update local preselector labels querying the remote server
    for (int n=0; n < 16; ++n) {
        QString command;
        command.clear(); QTextStream(&command) << "*getpreselector? " << n;
        pConn->sendCommand (command);
    }
    // update local preaplifier status querying the remote server
    {
        QString command;
        command.clear(); QTextStream(&command) << "*getpreampstatus?";
        pConn->sendCommand (command);
    }
}

void HardwareHiqsdr :: attClicked(int newVal)
{
   qDebug() << "Attenuator: " << newVal << "dB";
   if (attenuatorVal != newVal) {
      QString command;
      command.clear(); QTextStream(&command) << "*setattenuator " << newVal;
      pConn->sendCommand (command);
      attenuatorVal = newVal;
   } 
}

void HardwareHiqsdr :: antClicked(int n)
{
   qDebug() << "Antenna: " << n ;
   if (antennaVal != n) {
      QString command;
      command.clear(); QTextStream(&command) << "*selectantenna " << n;
      pConn->sendCommand (command);
      antennaVal = n;
   }
}

void HardwareHiqsdr :: preselClicked(int n)
{
   qDebug() << "Preselector: " << n ;
   if (preselVal != n) {
      QString command;
      command.clear(); QTextStream(&command) << "*selectpresel " << n;
      pConn->sendCommand (command);
      preselVal = n;
   }
}

void HardwareHiqsdr :: preampChanged(int state)
{
   qDebug() << "Preamplifier: " << state ;

   if (preampVal != state) {
      QString command;
      command.clear(); QTextStream(&command) << "*activatepreamp " << ((state==Qt::Checked) ? 1 : 0);
      pConn->sendCommand (command);
      preampVal = state;
   }
}

void HardwareHiqsdr :: processAnswer (QStringList list)
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

    if (list[0] == "*getpreselector?") {
       // try to set the serial
       qDebug() << Q_FUNC_INFO << list [1] << list[2] << list[3] ;
       // change the preselector buttons
       int x = list[1].toInt() ;
       
       if (x >= 0 && x < 16) {
           psel[x]->setText(list[3]);
       }
    }
    
    if (list[0] == "getpreampstatus?") {
       // try to set the serial
       qDebug() << Q_FUNC_INFO << list [1] << list[2] << list[3] ;
       // change the preamp button
       int x = list[1].toInt() ;
       
       if (x >= 0 && x <= 1) {
           preampVal = x;
           preamp->setChecked((preampVal == 1) ? true : false);   
       }
    }
}

HardwareHiqsdr :: ~HardwareHiqsdr ()
{

}

namespace {
   struct RegisterHw<HardwareHiqsdr> r("HiQSDR");
}
