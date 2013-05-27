/*
 * File:   main.cpp
 * Author: John Melton, G0ORX/N6LYT
 *
 * Created on 13 August 2010, 14:28
 */

/* Copyright (C)
* 2009 - John Melton, G0ORX/N6LYT
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
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#include <QtCore>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QApplication>
#else
#include <QtGui/QApplication>
#endif

#include "UI.h"

#include <stdio.h>
#include <stdlib.h>

int fOutputDisabled = 0;

#if QT_VERSION >= 0x050000
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
   if (fOutputDisabled) return;
    switch (type) {
    case QtDebugMsg:
         fprintf(stderr, "Debug: %s\n", message.toLocal8Bit().constData());
         break;
    case QtWarningMsg:
         fprintf(stderr, "Warning: %s\n", message.toLocal8Bit().constData());
         break;
    case QtCriticalMsg:
         fprintf(stderr, "Critical: %s\n", message.toLocal8Bit().constData());
         break;
    case QtFatalMsg:
         fprintf(stderr, "Fatal: %s\n", message.toLocal8Bit().constData());
         abort();
    }
}
#else
void myMessageOutput(QtMsgType type, const char *msg)
{
    if (fOutputDisabled) return;
    switch (type) {
    case QtDebugMsg:
         fprintf(stderr, "Debug: %s\n", msg);
         break;
    case QtWarningMsg:
         fprintf(stderr, "Warning: %s\n", msg);
         break;
    case QtCriticalMsg:
         fprintf(stderr, "Critical: %s\n", msg);
         break;
    case QtFatalMsg:
         fprintf(stderr, "Fatal: %s\n", msg);
         abort();
    }
}
#endif

int main(int argc, char *argv[]) {

     if (getenv("QT_RADIO_NO_DEBUG")) fOutputDisabled = 1;
#if QT_VERSION >= 0x050000
     qInstallMessageHandler(myMessageOutput);
#else
     qInstallMsgHandler(myMessageOutput);
#endif

    // initialize resources, if needed
    // Q_INIT_RESOURCE(resfile);

    QApplication app(argc, argv);

    qDebug() << "QThread: Main GUI thread : " << app.thread()->currentThread();

    //trying to get the arguments into a list    
    QStringList args = app.arguments();

    for (int i = 0; i < args.size(); ++i)
         fprintf (stderr, "%d: %s\n", i,  args.at(i).toLocal8Bit().constData() );
     
    QString srv("");

    if ( args.size() >1 ) srv = args.at(1) ;

    // create and show your widgets here
    UI widget(srv);
    widget.show();
    return app.exec();
}

