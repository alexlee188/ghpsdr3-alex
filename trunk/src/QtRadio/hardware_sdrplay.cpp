#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QVBoxLayout>
#endif
#include "hardware_sdrplay.h"


/*!
 * Set the gain for the device.
 * Manual gain mode must be enabled for this to work.
 *
 * Valid gain values (in tenths of a dB) for the E4000 tuner:
 * -10, 15, 40, 65, 90, 115, 140, 165, 190,
 * 215, 240, 290, 340, 420, 430, 450, 470, 490
 *
 * \param dev the device handle given by sdrplay_open()
 * \param gain in tenths of a dB, 115 means 11.5 dB.
 * \return 0 on success
 */

HardwareSdrplay :: HardwareSdrplay (Connection *pC, QWidget *pW): DlgHardware (pC, pW)
{
    // Attenuator
    QGroupBox *attGroupBox = new QGroupBox(tr("Attenuator"));
    attGroupBox->setFlat ( true );

    QRadioButton *att0Db  = new QRadioButton(tr("&0 dB"));
    QRadioButton *att5Db = new QRadioButton(tr("&5 dB"));
    QRadioButton *att10Db = new QRadioButton(tr("1&0 dB"));
    QRadioButton *att15Db = new QRadioButton(tr("1&5 dB"));
    QRadioButton *att20Db = new QRadioButton(tr("2&0 dB"));
    QRadioButton *att25Db = new QRadioButton(tr("2&5 dB"));
    QRadioButton *att30Db = new QRadioButton(tr("3&0 dB"));
    QRadioButton *att35Db = new QRadioButton(tr("3&5 dB"));
    QRadioButton *att40Db = new QRadioButton(tr("4&0 dB"));
    QRadioButton *att45Db = new QRadioButton(tr("4&5 dB"));
    QRadioButton *att50Db = new QRadioButton(tr("5&0 dB"));
    QRadioButton *att55Db = new QRadioButton(tr("5&5 dB"));
    QRadioButton *att60Db = new QRadioButton(tr("6&0 dB"));
    QRadioButton *att65Db = new QRadioButton(tr("6&5 dB"));
    QRadioButton *att70Db = new QRadioButton(tr("7&0 dB"));
    QRadioButton *att75Db = new QRadioButton(tr("7&5 dB"));
    QRadioButton *att80Db = new QRadioButton(tr("8&0 dB"));
    
    QVBoxLayout *vbox = new QVBoxLayout;

    vbox->addWidget(att0Db  );
    vbox->addWidget(att5Db );
    vbox->addWidget(att10Db );

    vbox->addWidget(att15Db);
    vbox->addWidget(att20Db);
    vbox->addWidget(att25Db);
    vbox->addWidget(att30Db);
    vbox->addWidget(att35Db);
    vbox->addWidget(att40Db);
    vbox->addWidget(att45Db);
    vbox->addWidget(att50Db);
    vbox->addWidget(att55Db);
    vbox->addWidget(att60Db);
    vbox->addWidget(att65Db);
    vbox->addWidget(att70Db);
    vbox->addWidget(att75Db);
    vbox->addWidget(att80Db);

    vbox->addStretch(1);

    attGroupBox->setLayout(vbox);

    // Main layout of dialog
    QHBoxLayout *grid = new QHBoxLayout;

    // add objects
    grid->addWidget (attGroupBox);

    // use grid obecjt as main dialog's layout 
    setLayout(grid);

    // general features
    setWindowTitle(tr("RTL SDR"));
    resize(240, 200);

    // interconnects
    attMapper = new QSignalMapper(this);
    connect(att0Db,  SIGNAL(toggled(bool)), attMapper, SLOT(map())); attMapper->setMapping(att0Db,  0);
    connect(att5Db,  SIGNAL(toggled(bool)), attMapper, SLOT(map())); attMapper->setMapping(att5Db,  5);
    connect(att10Db, SIGNAL(toggled(bool)), attMapper, SLOT(map())); attMapper->setMapping(att10Db, 10);
    connect(att15Db, SIGNAL(toggled(bool)), attMapper, SLOT(map())); attMapper->setMapping(att15Db, 15);
    connect(att20Db, SIGNAL(toggled(bool)), attMapper, SLOT(map())); attMapper->setMapping(att20Db, 20);
    connect(att25Db, SIGNAL(toggled(bool)), attMapper, SLOT(map())); attMapper->setMapping(att25Db, 25);
    connect(att30Db, SIGNAL(toggled(bool)), attMapper, SLOT(map())); attMapper->setMapping(att30Db, 30);
    connect(att35Db, SIGNAL(toggled(bool)), attMapper, SLOT(map())); attMapper->setMapping(att35Db, 35);
    connect(att40Db, SIGNAL(toggled(bool)), attMapper, SLOT(map())); attMapper->setMapping(att40Db, 40);
    connect(att45Db, SIGNAL(toggled(bool)), attMapper, SLOT(map())); attMapper->setMapping(att45Db, 45);
    connect(att50Db, SIGNAL(toggled(bool)), attMapper, SLOT(map())); attMapper->setMapping(att50Db, 50);
    connect(att55Db, SIGNAL(toggled(bool)), attMapper, SLOT(map())); attMapper->setMapping(att55Db, 55);
    connect(att60Db, SIGNAL(toggled(bool)), attMapper, SLOT(map())); attMapper->setMapping(att60Db, 60);
    connect(att65Db, SIGNAL(toggled(bool)), attMapper, SLOT(map())); attMapper->setMapping(att65Db, 65);
    connect(att70Db, SIGNAL(toggled(bool)), attMapper, SLOT(map())); attMapper->setMapping(att70Db, 70);
    connect(att75Db, SIGNAL(toggled(bool)), attMapper, SLOT(map())); attMapper->setMapping(att75Db, 75);
    connect(att80Db, SIGNAL(toggled(bool)), attMapper, SLOT(map())); attMapper->setMapping(att80Db, 80);

    connect(attMapper, SIGNAL(mapped(int)), this, SLOT(attClicked(int)));

    // defaults
    attenuatorVal = 40;

    att40Db->setChecked(true);              // attenuator 40 dB
}

void HardwareSdrplay :: attClicked(int newVal)
{
   qDebug() << "Attenuator: " << newVal << "dB";
   if (attenuatorVal != newVal) {
      QString command;
      command.clear(); QTextStream(&command) << "*setattenuator " << newVal;
      pConn->sendCommand (command);
      attenuatorVal = newVal;
   } 
}

void HardwareSdrplay :: processAnswer (QStringList list)
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

HardwareSdrplay :: ~HardwareSdrplay ()
{

}

namespace {
   struct RegisterHw<HardwareSdrplay> r("sdrplay");
}


