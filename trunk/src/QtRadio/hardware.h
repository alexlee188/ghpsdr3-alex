#if !defined __HARDWARE_H__
#define __HARDWARE_H__

#include <QWidget>
#include <QSignalMapper>
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
};



class HardwareFactory {
public:

   static DlgHardware *Clone(Connection *pConn, const char *, QWidget *p);

   static void processAnswer (QString a, Connection *pConn, UI *p );

   void get() {}


};

extern HardwareFactory hwFactory;

#endif
