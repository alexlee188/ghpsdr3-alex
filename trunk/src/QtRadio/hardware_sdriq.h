#if !defined __HARDWARE_SDRIQ_H__
#define      __HARDWARE_SDRIQ_H__

#include "hardware.h"

class HardwareSdriq: public DlgHardware
{ 
   Q_OBJECT

public:
   HardwareSdriq (Connection *pC, QWidget *p);
   ~HardwareSdriq ();
private:
   QSignalMapper *attMapper;
private slots:
   void attClicked(int state);
   void processAnswer (QStringList);
};

#endif

