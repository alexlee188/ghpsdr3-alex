#if !defined __HARDWARE_HIQSDR_H__
#define      __HARDWARE_HIQSDR_H__

#include "hardware.h"

class QRadioButton;
class QCheckBox;

class HardwareHiqsdr: public DlgHardware
{ 
   Q_OBJECT

public:
   HardwareHiqsdr (Connection *pC, QWidget *p);
   ~HardwareHiqsdr ();

private:
   QSignalMapper *attMapper;
   int attenuatorVal;
   QSignalMapper *antMapper;
   int antennaVal;
   QSignalMapper *preselMapper;
   int preselVal;
   QRadioButton *psel[16];
   QCheckBox *preamp;
   int preampVal;
   

private slots:
   void attClicked(int state);
   void antClicked(int n);
   void preselClicked(int n);
   void preampChanged(int n);
   void processAnswer (QStringList);
};

#endif

