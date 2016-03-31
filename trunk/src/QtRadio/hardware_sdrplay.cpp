#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QVBoxLayout>
#endif
#include "hardware_sdrplay.h"


/*!
 * Set the gain reduction for the SDRplay.
 *
 * Valid gain values are integral dB
 *
 * \param dev the device handle given by sdrplay_open()
 * \param gain reduction in dB, 115 means 115 dB.
 * \return 0 on success
 */

HardwareSdrplay :: HardwareSdrplay (Connection *pC, QWidget *pW): DlgHardware (pC, pW)
{
    // Attenuator
    QGroupBox *attGroupBox = new QGroupBox(tr("Gain Reduction"));
    QVBoxLayout *vbox = new QVBoxLayout;
    attMapper = new QSignalMapper(this);

    // defaults
    attenuatorVal = 80;

    for (int i = 0; i <= 105; i += 5) {
      char label[10];
      if (i == 105) i = 102;
      sprintf(label, "%d dB", i);
      QRadioButton *att = new QRadioButton(tr(&label[0]));
      // into the pool
      vbox->addWidget(att);
      // interconnects
      connect(att,  SIGNAL(toggled(bool)), attMapper, SLOT(map()));
      attMapper->setMapping(att,  i);
      // default value
      if (i == attenuatorVal) att->setChecked(true);
    }


    attGroupBox->setFlat ( true );
    attGroupBox->setLayout(vbox);
    vbox->addStretch(1);
    connect(attMapper, SIGNAL(mapped(int)), this, SLOT(attClicked(int)));

    // Main layout of dialog
    QHBoxLayout *grid = new QHBoxLayout;
    grid->addWidget (attGroupBox);
    setLayout(grid);

    // general features
    setWindowTitle(tr("SDRplay"));
    resize(240, 200);
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


