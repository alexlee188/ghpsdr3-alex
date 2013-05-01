#if !defined __HARDWARE_PERSEUS_H__
#define      __HARDWARE_PERSEUS_H__

#include "hardware.h"

class HardwarePerseus: public DlgHardware
{ 
   Q_OBJECT

public:
   HardwarePerseus (Connection *pC, QWidget *p);
   ~HardwarePerseus ();
private:
   QSignalMapper *attMapper;
private slots:
   void attClicked(int state);
   void ditherChanged(int);
   void preampChanged(int);

   void processAnswer (QStringList);
};

#endif

