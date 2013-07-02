#if !defined __HARDWARE_HERMES_H__
#define      __HARDWARE_HERMES_H__

#include "hardware.h"

class QSlider;

class HardwareHermes: public DlgHardware
{ 
   Q_OBJECT

public:
   HardwareHermes (Connection *pC, QWidget *p);
   ~HardwareHermes ();
private:
   QSignalMapper *attMapper;
   QSignalMapper *atxrMapper;
private slots:
   void attClicked(int state); 
   void atxrClicked(int state);
   void attSlid(int state);
   void txDriveSlid(int state);
   void txLineInGainSlid(int state);
   void randomChanged(int);
   void ditherChanged(int);
   void preampChanged(int);
   void micBoostClicked(int);

   void processAnswer (QStringList);

private:
   QSlider *pAttSlider;
   QSlider *pTxSlider;
   QSlider *pTxLineInGain;
};

#endif


