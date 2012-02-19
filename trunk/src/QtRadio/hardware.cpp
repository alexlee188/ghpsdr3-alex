#include <QWidget> 
#include <QtGui>
#include <QDebug>

#include "hardware.h"
#include "UI.h"


DlgHardware :: DlgHardware (Connection *pC, QWidget *p): QWidget(p), pParent(p), pConn(pC)
{

}


DlgHardware :: ~DlgHardware ()
{

}


HardwarePerseus :: HardwarePerseus (Connection *pC, QWidget *pW): DlgHardware (pC, pW)
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

    // Dither
    QCheckBox *ditherCB = new QCheckBox(tr("Di&ther"));

    // Preamp
    QCheckBox *preampCB = new QCheckBox(tr("Pre&amplifier"));

    // Main layout of dialog
    QHBoxLayout *grid = new QHBoxLayout;

    // add objects
    grid->addWidget (attGroupBox);
    grid->addWidget (ditherCB);
    grid->addWidget (preampCB);


    // use grid obecjt as main dialog's layout 
    setLayout(grid);

    // general features
    setWindowTitle(tr("Perseus"));
    //resize(240, 160);

    // interconnects
    attMapper = new QSignalMapper(this);
    connect(att0Db, SIGNAL(clicked(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att0Db, 0);

    connect(att10Db, SIGNAL(clicked(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att10Db, 10);

    connect(att20Db, SIGNAL(clicked(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att20Db, 20);

    connect(att30Db, SIGNAL(clicked(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att30Db, 30);

    connect(attMapper, SIGNAL(mapped(int)), this, SLOT(attClicked(int)));

    connect(ditherCB, SIGNAL(clicked(bool)), this, SLOT(ditherClicked(bool)));
    connect(preampCB, SIGNAL(clicked(bool)), this, SLOT(preampClicked(bool)));
}

void HardwarePerseus :: attClicked(int state)
{
   qDebug() << "Attenuator: " << state << "dB";
   QString command;
   command.clear(); QTextStream(&command) << "*setattenuator " << state;
   pConn->sendCommand (command);
}

void HardwarePerseus :: ditherClicked(bool state)
{
   qDebug() << "Dither: " << state;
   QString command;
   command.clear(); QTextStream(&command) << "*dither " << ((state==true) ? "on" : "off") ;
   pConn->sendCommand (command);
}
void HardwarePerseus :: preampClicked(bool state)
{   
   qDebug() << "Preamp: " << state;
   QString command;
   command.clear(); QTextStream(&command) << "*preamp " << ((state==true) ? "on" : "off") ;
   pConn->sendCommand (command);
}

HardwarePerseus :: ~HardwarePerseus ()
{

}


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
    connect(att0Db, SIGNAL(clicked(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att0Db, 0);

    connect(att10Db, SIGNAL(clicked(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att10Db, 10);

    connect(att20Db, SIGNAL(clicked(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att20Db, 20);

    connect(att30Db, SIGNAL(clicked(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att30Db, 30);

    connect(attMapper, SIGNAL(mapped(int)), this, SLOT(attClicked(int)));
}

void HardwareSdriq :: attClicked(int state)
{
   qDebug() << "Attenuator: " << state << "dB";
   QString command;
   command.clear(); QTextStream(&command) << "*setattenuator " << state;
   pConn->sendCommand (command);
}


HardwareSdriq :: ~HardwareSdriq ()
{

}




DlgHardware * HardwareFactory::Clone(Connection *pConn, const char *pName, QWidget * /*  p  */) 
{
   qDebug() << Q_FUNC_INFO << pName;

   if (strcmp(pName, "Perseus")==0) {
      return new HardwarePerseus(pConn, 0);
   }
   if (strcmp(pName, "SDR-IQ")==0) {
      return new HardwareSdriq(pConn, 0);
   }
   return 0;
}

void HardwareFactory :: processAnswer (QString a, Connection *pConn, UI *pUI )
{      
   QStringList list;

   if (a.length()) list = a.split(QRegExp(" "));

   switch (list.size()) {

   case 0:
     qDebug() << Q_FUNC_INFO<< "empty answer, delete everything";
     pUI->rmHwDlg();          
     break;

   case 1:
     qDebug() << Q_FUNC_INFO << list[0];
//     pUI->rmHwDlg();          
     break;

   case 2:
     qDebug() << Q_FUNC_INFO <<list[0] << list[1];
     break;

   case 3:
     qDebug() << Q_FUNC_INFO << list[0] << list[1] << list[2];

     if (list[0] == "*hardware?") {
        // try to activate an hardware control panel
        DlgHardware *pHwDlg = HardwareFactory::Clone (pConn, list[2].toAscii(), 0);
        if (pHwDlg) {
           qDebug() << Q_FUNC_INFO<<list[2];
           pHwDlg->show();
           pUI->setHwDlg(pHwDlg);
        }
     }
     break;

   default:
     qDebug() << Q_FUNC_INFO<< "FATAL: more than 3: " << a;
   }

}

HardwareFactory hwFactory;
