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
#include <stdexcept>
 
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

//
// Hardware factory
//
// implemented as Singleton and Object Factory
// see "Modern C++ Design", A. Alexandrescu, Addison-Wesley 2001 
// 
class HardwareFactory {
public:
   typedef DlgHardware * (*CreateDlgHardwareCallback)(Connection *pConn);

   // Factory public methods
   DlgHardware *Clone(Connection *pConn, const char *, QWidget *p);
   void processAnswer (QString a, Connection *pConn, UI *p );

   bool Register (std::string id, CreateDlgHardwareCallback CreateFn)
   { return callbacks_.insert (CallbackMap::value_type(id, CreateFn)).second; }

   // Singleton methods
   static HardwareFactory& Instance();

private: 

   // Factory Data
   typedef std::map<std::string, CreateDlgHardwareCallback> CallbackMap;
   CallbackMap callbacks_;

   // Singleton Data
   static HardwareFactory *pInstance_;
   static bool destroyed_;

   // Creat a new singleton and store a pinter to it into pInstance_
   static void Create ();

   static void OnDeadReference () { throw std::runtime_error("Dead reference detected");}
   virtual ~HardwareFactory () { pInstance_ = 0; destroyed_ = true; }

   // 
};

template 
<
  class Base
>  
struct RegisterHw {
 static DlgHardware *cb(Connection *pConn) { return new Base(pConn, 0); }
 RegisterHw (std::string hwName) { HardwareFactory::Instance ().Register (hwName, cb); } ;
};




#endif
