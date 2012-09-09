#if !defined __HARDWARE_H__
#define __HARDWARE_H__

#include <QDebug>
#include <QtCore>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QWidget>
#include <QtWidgets/QRadioButton>
#else
#include <QWidget>
#include <QSignalMapper>
#endif
#include "Connection.h"

class HardwareCommProxy 
{
public:
    HardwareCommProxy ();
    ~HardwareCommProxy ();

    virtual void send (const char *) {}
};

class DlgHardware: public QWidget {

Q_OBJECT

public:
    DlgHardware (Connection *pC, QWidget *parent);
    virtual ~DlgHardware ();

public slots:
    virtual void processAnswer (QStringList) {}

private:
    QWidget    *pParent;
protected:
    Connection *pConn;
};


class UI;

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


class QRadioButton;

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

private slots:
   void attClicked(int state);
   void antClicked(int n);
   void preselClicked(int n);
   void processAnswer (QStringList);
};


class HardwareRtlsdr: public DlgHardware
{ 
   Q_OBJECT

public:
   HardwareRtlsdr (Connection *pC, QWidget *p);
   ~HardwareRtlsdr ();

private:
   QSignalMapper *attMapper;
   int attenuatorVal;

private slots:
   void attClicked(int state);
   void processAnswer (QStringList);
};



class HardwareFactory {
public:

   static DlgHardware *Clone(Connection *pConn, const char *, QWidget *p);

   static void processAnswer (QString a, Connection *pConn, UI *p );

   void get() {}


};

extern HardwareFactory hwFactory;

#endif
