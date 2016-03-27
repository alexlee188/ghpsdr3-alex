#if !defined __HARDWARE_RTLSDR_H__
#define      __HARDWARE_RTLSDR_H__

#include "hardware.h"

class HardwareSdrplay: public DlgHardware
{ 
   Q_OBJECT

public:
   HardwareSdrplay (Connection *pC, QWidget *p);
   ~HardwareSdrplay ();

private:
   QSignalMapper *attMapper;
   int attenuatorVal;

private slots:
   void attClicked(int state);
   void processAnswer (QStringList);
};
            

#endif

