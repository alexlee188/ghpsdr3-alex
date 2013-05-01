// Added by KD0OSS  03-30-2013

#ifndef HARDWARE_SDR1000_H
#define HARDWARE_SDR1000_H

#include "hardware.h"

class QSlider;

class HardwareSDR1000: public DlgHardware
{
   Q_OBJECT

public:
   HardwareSDR1000 (Connection *pC, QWidget *p);
   ~HardwareSDR1000 ();

private:
   QSignalMapper *attMapper;
   QSignalMapper *spurMapper;
   QTimer *paFwd;
   int attenuatorVal;

private slots:
   void attClicked(int state);
   void spurClicked(bool state);
   void getPaFwd(void);
   void getPA_ADC(unsigned char channel);

   void processAnswer (QStringList);

};

#endif // HARDWARE_SDR1000_H
