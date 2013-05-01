// Added by KD0OSS  03-30-2013


#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QTabWidget>
#endif
#include "hardware_sdr1000.h"


HardwareSDR1000 :: HardwareSDR1000 (Connection *pC, QWidget *pW): DlgHardware (pC, pW)
{
    // Attenuator
    QGroupBox *attGroupBox = new QGroupBox(tr("Attenuator"));
    attGroupBox->setFlat ( true );

    QRadioButton *att0Db  = new QRadioButton(tr("&0 dB"));
    QRadioButton *att10Db = new QRadioButton(tr("-&10 dB"));
    QVBoxLayout *vbox = new QVBoxLayout;

    vbox->addWidget(att0Db);
    vbox->addWidget(att10Db);
    attGroupBox->setLayout(vbox);

    QCheckBox *spurReduce = new QCheckBox(tr("Spur Reduction"));

    // Main layout of dialog
    QHBoxLayout *grid = new QHBoxLayout;

    // left sub pane
    QVBoxLayout *leftsp = new QVBoxLayout;
    leftsp->addWidget (attGroupBox);

    leftsp->addWidget(spurReduce);

    // add objects to main layout
    grid->addLayout (leftsp);

    // use grid obecjt as main dialog's layout
    setLayout(grid);

    // general features
    setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowTitleHint|Qt::WindowStaysOnTopHint);
    setWindowTitle(tr("Flex SDR1000"));
    resize(240, 200);

    // interconnects
    attMapper = new QSignalMapper(this);
    connect(att0Db,  SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att0Db, 0);

    connect(att10Db, SIGNAL(toggled(bool)), attMapper, SLOT(map()));
    attMapper->setMapping(att10Db, 10);

    connect(attMapper, SIGNAL(mapped(int)), this, SLOT(attClicked(int)));

    spurMapper = new QSignalMapper(this);
    connect(spurReduce,  SIGNAL(toggled(bool)), this, SLOT(spurClicked(bool)));

    att0Db->setChecked(true);              // attenuator 0 dB

    paFwd = new QTimer();
    paFwd->setInterval(2000);
    connect(paFwd, SIGNAL(timeout()), this, SLOT(getPaFwd()));
  //  paFwd->start();
}

void HardwareSDR1000 :: attClicked(int newVal)
{
   qDebug() << "Attenuator: " << newVal << "dB";
   if (attenuatorVal != newVal) {
      QString command;
      command.clear(); QTextStream(&command) << "*setattenuator " << newVal;
      pConn->sendCommand (command);
      attenuatorVal = newVal;
   }
}

void HardwareSDR1000 :: spurClicked(bool newVal)
{
   qDebug() << "Spur Reduction: " << newVal;
   QString command;
   command.clear(); QTextStream(&command) << "*setspurreduction " << newVal;
   pConn->sendCommand (command);
}

void HardwareSDR1000 :: getPaFwd(void)
{
    paFwd->stop();
    getPA_ADC(0);
}

// channel 0 = FWD, chan 1 = REV -- returns an 8 bit value
void HardwareSDR1000 :: getPA_ADC(unsigned char channel)
{
   qDebug() << "Get PA ADC: " << channel;
   QString command;
   command.clear(); QTextStream(&command) << "*getpaadc? " << channel;
   pConn->sendCommand (command);
}

void HardwareSDR1000 :: processAnswer (QStringList list)
{
    if (list[0] == "*getpaadc?") {
       // get PA ADC value
       qDebug() << Q_FUNC_INFO << list[1] << list[2];
       int x = list[2].toInt();
       paFwd->start();
    }
}

HardwareSDR1000 :: ~HardwareSDR1000 ()
{

}


namespace {
   struct RegisterHw<HardwareSDR1000> r("SDR1000");
}
