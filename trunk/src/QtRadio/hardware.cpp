
/* Copyright (C) 2012 - Alex Lee, 9V1Al
* modifications of the original program by John Melton
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Pl
*/

#include <QtGui>
#include "UI.h"
#include "hardware.h"


DlgHardware :: DlgHardware (Connection *pC, QWidget *p): QWidget(p), pParent(p), pConn(pC)
{

}


DlgHardware :: ~DlgHardware ()
{

}





DlgHardware * HardwareFactory::Clone(Connection *pConn, const char *pName, QWidget * /*  p  */) 
{
   qDebug() << Q_FUNC_INFO << pName;

   CallbackMap::const_iterator i ;

   for(i = callbacks_.begin(); i != callbacks_.end(); i++) 
      qDebug() << "*************" << std::string(i->first).c_str() ;

   i = callbacks_.find (pName);
   if ( i == callbacks_.end() ) throw std::runtime_error ("Unknown Hardware ID");
   // Invoke the creation function
   return (i->second(pConn));
}

//
// http://stackoverflow.com/questions/8215303/how-do-i-remove-trailing-whitespace-in-a-qstring-in-qt-c
//
QString &trim( QString & str, char ch ) {
    
  if ( str.size() > 0 && str.at(0) == ch) str.remove( 0, 1 );
  while( str.size() > 0 && str.at( str.size() - 1 ) == ch )
    str.chop( 1 );
  return str;
}

void HardwareFactory :: processAnswer (QString a, Connection *pConn, UI *pUI )
{      
   QStringList list;

   //
   // split the answer in a list of QString's
   //
   if (a.length()) {
      
      // quoted strings are kept together
      list = a.split(QRegExp(" (?=[^\"]*(\"[^\"]*\"[^\"]*)*$)") );
      
      QList<QString>::iterator i = list.begin();
      while (i != list.end()) {
        QString t = trim(*i, '"'); // trims trailing and leading quotes
        qDebug() << Q_FUNC_INFO << ": " << qPrintable(t) << endl;
        *i = t;
        ++i;
      }
   }

   switch (list.size()) {

   case 0:
     qDebug() << Q_FUNC_INFO<< "empty answer, delete everything";
     pUI->rmHwDlg();          
     break;

   case 1:
     qDebug() << Q_FUNC_INFO << list[0];
     // not currently used
     break;

   case 2:
     qDebug() << Q_FUNC_INFO <<list[0] << list[1];
     // not currently used
     break;

   case 3:
     qDebug() << Q_FUNC_INFO << list[0] << list[1] << list[2];

     if (list[0] == "*hardware?") {
        // try to activate an hardware control panel
#if QT_VERSION >= 0x050000
        DlgHardware *pHwDlg = HardwareFactory::Clone (pConn, list[2].toUtf8(), 0);
#else
        DlgHardware *pHwDlg = HardwareFactory::Clone (pConn, list[2].toAscii(), 0);
#endif
        if (pHwDlg) {
           qDebug() << Q_FUNC_INFO<<list[2];
           pHwDlg->show();
           pUI->setHwDlg(pHwDlg);
        }
     } else {
        DlgHardware *pHwDlg = pUI->getHwDlg();
        if (pHwDlg) {
           qDebug() << Q_FUNC_INFO<<list[2];
           emit pHwDlg->processAnswer(list);
        }
     }
     break;

   case 4:
     {
         qDebug() << Q_FUNC_INFO <<list[0] << list[1] << list[2] << list[3];
         DlgHardware *pHwDlg = pUI->getHwDlg();
         if (pHwDlg) {
            //qDebug() << Q_FUNC_INFO<<list[3];
            emit pHwDlg->processAnswer(list);
         }
     }
     break;

   default:
     qDebug() << Q_FUNC_INFO<< "FATAL: more than 4: " << a;
   }

}

//
// Singleton methods
//
HardwareFactory& HardwareFactory::Instance()
{
   if (!pInstance_) {
      // Check for dead reference
      if (destroyed_)
         OnDeadReference();
      else
         Create ();
   }
   return *pInstance_;
}
void HardwareFactory::Create ()
{ 
   // initialize pInstance_
   static HardwareFactory theInstance;
   HardwareFactory::pInstance_ = &theInstance;
}

HardwareFactory* HardwareFactory::pInstance_ = 0;
bool HardwareFactory::destroyed_ = false;


