#include <QtGui>
#include "hardware_sdriq.h"

HardwareSdriq :: HardwareSdriq (Connection *pC, QWidget *pW): DlgHardware (pC, pW)
{

    // Attenuator
    QGroupBox *attGroupBox = new QGroupBox(tr("Attenuator"));
    attGroupBox->setFlat ( true );

    QRadioButton *att0Db  = new QRadioButton(tr("&0 dB"));
    QRadioButton *att10Db = new QRadioButton(tr("-&10 dB"));
    QRadioButton *att20Db = new QRadioButton(tr("-&20 dB"));
    QRadioButton *att30Db = new QRadioButton(tr("-&30 dB"));
    
    QVBoxLayout *vbox = new QVBoxLayout;

    vbox->addWidget(att0Db);
    vbox->addWidget(att10Db);
    vbox->addWidget(att20Db);
    vbox->addWidget(att30Db);
    vbox->addStretch(1);

    attGroupBox->setLayout(vbox);

    // Main layout of dialog
    QHBoxLayout *grid = new QHBoxLayout;

    // add objects
    grid->addWidget (attGroupBox);

    // use grid obecjt as main dialog's layout 
    setLayout(grid);

    // general features
    setWindowTitle(tr("SDR-IQ"));
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

    connect(attMapper, SIGNAL(mapped(int)), this, SLOT(attClicked(int)));

    // update the serial number in title bar
    QString command;
    command.clear(); QTextStream(&command) << "*getserial?";
    pConn->sendCommand (command);

    // defaults
    att0Db->setChecked(true);              // attenuator 0 dB
}

void HardwareSdriq :: attClicked(int state)
{
   qDebug() << "Attenuator: " << state << "dB";
   QString command;
   command.clear(); QTextStream(&command) << "*setattenuator " << state;
   pConn->sendCommand (command);
}

void HardwareSdriq :: processAnswer (QStringList list)
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

HardwareSdriq :: ~HardwareSdriq ()
{

}

namespace {
   struct RegisterHw<HardwareSdriq> r("SDR-IQ");
}


