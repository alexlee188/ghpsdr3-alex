#include <QtGui>
#include "hardware_rtlsdr.h"


/*!
 * Set the gain for the device.
 * Manual gain mode must be enabled for this to work.
 *
 * Valid gain values (in tenths of a dB) for the E4000 tuner:
 * -10, 15, 40, 65, 90, 115, 140, 165, 190,
 * 215, 240, 290, 340, 420, 430, 450, 470, 490
 *
 * \param dev the device handle given by rtlsdr_open()
 * \param gain in tenths of a dB, 115 means 11.5 dB.
 * \return 0 on success
 */

HardwareRtlsdr :: HardwareRtlsdr (Connection *pC, QWidget *pW): DlgHardware (pC, pW)
{
    // Attenuator
    QGroupBox *attGroupBox = new QGroupBox(tr("Attenuator"));
    attGroupBox->setFlat ( true );

    QRadioButton *att1Db  = new QRadioButton(tr("-&1.0 dB"));
    QRadioButton *att15Db = new QRadioButton(tr("&1.5 dB"));
    QRadioButton *att40Db = new QRadioButton(tr("&4 dB"));
    QRadioButton *att65Db = new QRadioButton(tr("&6.5 dB"));
    QRadioButton *att90Db = new QRadioButton(tr("&9.0 dB"));
    QRadioButton *att115Db = new QRadioButton(tr("1&1.5 dB"));
    QRadioButton *att140Db = new QRadioButton(tr("1&4.0 dB"));
    QRadioButton *att165Db = new QRadioButton(tr("1&6.5 dB"));
    QRadioButton *att190Db = new QRadioButton(tr("1&9.0 dB"));
    QRadioButton *att215Db = new QRadioButton(tr("2&1.5 dB"));
    QRadioButton *att240Db = new QRadioButton(tr("2&4.0 dB"));
    QRadioButton *att290Db = new QRadioButton(tr("2&9.0 dB"));
    QRadioButton *att340Db = new QRadioButton(tr("3&4.0 dB"));
    QRadioButton *att420Db = new QRadioButton(tr("4&2.0 dB"));
    QRadioButton *att430Db = new QRadioButton(tr("4&3.0 dB"));
    QRadioButton *att450Db = new QRadioButton(tr("4&5.0 dB"));
    QRadioButton *att470Db = new QRadioButton(tr("4&7.0 dB"));
    QRadioButton *att490Db = new QRadioButton(tr("4&9.0 dB"));
    
    QVBoxLayout *vbox = new QVBoxLayout;

    vbox->addWidget(att1Db  );
    vbox->addWidget(att15Db );
    vbox->addWidget(att40Db );
    vbox->addWidget(att65Db );
    vbox->addWidget(att90Db );

    vbox->addWidget(att115Db);
    vbox->addWidget(att140Db);
    vbox->addWidget(att165Db);
    vbox->addWidget(att190Db);
    vbox->addWidget(att215Db);
    vbox->addWidget(att240Db);
    vbox->addWidget(att290Db);
    vbox->addWidget(att340Db);
    vbox->addWidget(att420Db);
    vbox->addWidget(att430Db);
    vbox->addWidget(att450Db);
    vbox->addWidget(att470Db);
    vbox->addWidget(att490Db);

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
    connect(att1Db,  SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att1Db, -10);

    connect(att15Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att15Db, 15);

    connect(att40Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att40Db, 40);

    connect(att65Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att65Db, 65);

    connect(att90Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att90Db, 90);

    connect(att115Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att115Db, 115);

    connect(att140Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att140Db, 140);

    connect(att165Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att165Db, 165);

    connect(att190Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att190Db, 190);

    connect(att215Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att215Db, 215);

    connect(att240Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att240Db, 240);

    connect(att340Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att340Db, 340);

    connect(att420Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att420Db, 420);

    connect(att430Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att430Db, 430);

    connect(att450Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att450Db, 470);

    connect(att490Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att490Db, 490);

    connect(attMapper, SIGNAL(mapped(int)), this, SLOT(attClicked(int)));

    // update the serial number in title bar
    QString command;
    command.clear(); QTextStream(&command) << "*getserial?";
    pConn->sendCommand (command);

    // defaults
    attenuatorVal = -1;

    att1Db->setChecked(true);              // attenuator -1 dB
}

void HardwareRtlsdr :: attClicked(int newVal)
{
   qDebug() << "Attenuator: " << newVal << "dB";
   if (attenuatorVal != newVal) {
      QString command;
      command.clear(); QTextStream(&command) << "*setattenuator " << newVal;
      pConn->sendCommand (command);
      attenuatorVal = newVal;
   } 
}

void HardwareRtlsdr :: processAnswer (QStringList list)
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

HardwareRtlsdr :: ~HardwareRtlsdr ()
{

}

namespace {
   struct RegisterHw<HardwareRtlsdr> r("rtlsdr");
}


