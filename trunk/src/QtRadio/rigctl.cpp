
/*
 * Server for hamlib's TCP rigctl protocol.
 * Copyright (C) 2010 Adam Sampson <ats@offog.org>
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *
 * Modified for QtRadio by Glenn VE9GJ Sept 29 2011
 * Taken from sdr-shell project
 *
 * Applied remote set filter function over hamlib port
 * by Oliver Goldenstein, DL6KBG, Nov 16 2012
 *
 */
#include <cstdio>

#include "UI.h"
#include "rigctl.h"
#include <string>
#include <vector>
#include <QtDebug>

RigCtlSocket::RigCtlSocket(QObject *parent, UI *main, QTcpSocket *conn)
        : QObject(parent),
          main(main) {
        this->conn = conn;
}

void RigCtlSocket::disconnected() {
        deleteLater();
}


void RigCtlSocket::readyRead() {
        if (!conn->canReadLine()) {
                return;
        }

        QByteArray command(conn->readLine());
        // removed command.chop(1); because it is unable to cope with
        // with more than one character line terminations
        // i.e. telnet client uses a CR/LF sequence
        command = command.simplified();
        if (command.size() == 0) {
                command.append("?");
        }

        QString cmdstr = command.constData();
        QStringList cmdlist = cmdstr.split(QRegExp("\\s+"));
        int cmdlistcnt = cmdlist.count();
        bool output = false;
        int retcode = 0; //RIG_OK
        QTextStream out(conn);

        /* This isn't a full implementation of the rigctl protocol; it's
           essentially just enough to keep fldigi happy.  (And not very happy
           at that; you will need to up the delays to stop it sending a
           command, ignoring the reply owing to a timeout, then getting
           confused the next time it sends a command because the old reply is
           already in the buffer.)

           Not implemented but used by fldigi:
             v           get_vfo  -fixed
             F 1.234     set_freq -fixed
         */

        int space = command.indexOf(' ');
        if (command[0] == 'f') { // get_freq
            out << main->rigctlGetFreq() << "\n";
            output = true;
        } else if(cmdlist[0].compare("F") == 0 && cmdlistcnt == 2) { // set_freq
            QString newf = cmdlist[1];
#if QT_VERSION >= 0x050000
            main->rigctlSetFreq(atol(newf.toUtf8()));
#else
            main->rigctlSetFreq(atol(newf.toAscii()));
#endif
        } else if (command[0] == 'm') { // get_mode
#if QT_VERSION >= 0x050000
            out << main->rigctlGetMode().toUtf8() << "\n";
            out << main->rigctlGetFilter().toUtf8() << "\n";
#else
            out << main->rigctlGetMode().toAscii() << "\n";
            out << main->rigctlGetFilter().toAscii() << "\n";
#endif
            output = true;
        } else if (command[0] == 'v') { // get_vfo
#if QT_VERSION >= 0x050000
            out << main->rigctlGetVFO().toUtf8() << "\n";
#else
            out << main->rigctlGetVFO().toAscii() << "\n";
#endif
            output = true;
        } else if (command[0] == 'V') { // set_VFO
            QString cmd = command.constData();
            if ( cmd.contains("VFOA")){
               main->rigctlSetVFOA();
            }
            if ( cmd.contains("VFOB")){
               main->rigctlSetVFOB();
            }
        } else if (command[0] == 'j') { // get_rit
            out << "0" << "\n";
            output = true;
        } else if (command[0] == 's') { // get_split_vfo
            out << "0" << "\n";
           //out << main->rigctlGetVFO().toAscii() << "\n";
            output = true;
        } else if (command[0] == 'T') { // set_ptt  no tx yet but this keeps grig and fldigi happy
            int enabled = command.mid(space + 1).toInt();
            qDebug("Rigctl: PTT T : ->%d", enabled);
            main->rigSetPTT(enabled);
        } else if (command[0] == 'q') { // quit
            conn->close();
            return;
        } else if (cmdlist[0].compare("M") == 0 && cmdlistcnt == 3) {
            if (cmdlist[1].compare("USB") == 0 ) {
               main->rigctlSetMode(MODE_USB);
               // compare usb filters
               if (cmdlist[2].compare("5000") == 0 ) {
               main->rigctlSetFilter(0);
               }
               else if (cmdlist[2].compare("4400") == 0 ) {
               main->rigctlSetFilter(1);
               }
               else if (cmdlist[2].compare("3800") == 0 ) {
               main->rigctlSetFilter(2);
               }
               else if (cmdlist[2].compare("3300") == 0 ) {
               main->rigctlSetFilter(3);
               }
               else if (cmdlist[2].compare("2900") == 0 ) {
               main->rigctlSetFilter(4);
               }
               else if (cmdlist[2].compare("2700") == 0 ) {
               main->rigctlSetFilter(5);
               }
               else if (cmdlist[2].compare("2400") == 0 ) {
               main->rigctlSetFilter(6);
               }
               else if (cmdlist[2].compare("2100") == 0 ) {
               main->rigctlSetFilter(7);
               }
               else if (cmdlist[2].compare("1800") == 0 ) {
               main->rigctlSetFilter(8);
               }
               else if (cmdlist[2].compare("1000") == 0 ) {
               main->rigctlSetFilter(9);
               }

            }else if (cmdlist[1].compare("LSB") == 0) {
               main->rigctlSetMode(MODE_LSB);
               // compare lsb filters
               if (cmdlist[2].compare("5000") == 0 ) {
               main->rigctlSetFilter(0);
               }
               else if (cmdlist[2].compare("4400") == 0 ) {
               main->rigctlSetFilter(1);
               }
               else if (cmdlist[2].compare("3800") == 0 ) {
               main->rigctlSetFilter(2);
               }
               else if (cmdlist[2].compare("3300") == 0 ) {
               main->rigctlSetFilter(3);
               }
               else if (cmdlist[2].compare("2900") == 0 ) {
               main->rigctlSetFilter(4);
               }
               else if (cmdlist[2].compare("2700") == 0 ) {
               main->rigctlSetFilter(5);
               }
               else if (cmdlist[2].compare("2400") == 0 ) {
               main->rigctlSetFilter(6);
               }
               else if (cmdlist[2].compare("2100") == 0 ) {
               main->rigctlSetFilter(7);
               }
               else if (cmdlist[2].compare("1800") == 0 ) {
               main->rigctlSetFilter(8);
               }
               else if (cmdlist[2].compare("1000") == 0 ) {
               main->rigctlSetFilter(9);
               }

            } else if (cmdlist[1].compare("AM") == 0) {
               main->rigctlSetMode(MODE_AM);
               // compare am filters
               if (cmdlist[2].compare("16000") == 0 ) {
               main->rigctlSetFilter(0);
               }
               else if (cmdlist[2].compare("12000") == 0 ) {
               main->rigctlSetFilter(1);
               }
               else if (cmdlist[2].compare("10000") == 0 ) {
               main->rigctlSetFilter(2);
               }
               else if (cmdlist[2].compare("8000") == 0 ) {
               main->rigctlSetFilter(3);
               }
               else if (cmdlist[2].compare("6600") == 0 ) {
               main->rigctlSetFilter(4);
               }
               else if (cmdlist[2].compare("5200") == 0 ) {
               main->rigctlSetFilter(5);
               }
               else if (cmdlist[2].compare("4000") == 0 ) {
               main->rigctlSetFilter(6);
               }
               else if (cmdlist[2].compare("3100") == 0 ) {
               main->rigctlSetFilter(7);
               }
               else if (cmdlist[2].compare("2900") == 0 ) {
               main->rigctlSetFilter(8);
               }
               else if (cmdlist[2].compare("2400") == 0 ) {
               main->rigctlSetFilter(9);
               }

            } else if (cmdlist[1].compare("FM") == 0) {
               main->rigctlSetMode(MODE_FM);
               // compare fm filters
               if (cmdlist[2].compare("80000") == 0 ) {
               main->rigctlSetFilter(0);
               }
               else if (cmdlist[2].compare("12000") == 0 ) {
               main->rigctlSetFilter(1);
               }
               else if (cmdlist[2].compare("10000") == 0 ) {
               main->rigctlSetFilter(2);
               }
               else if (cmdlist[2].compare("8000") == 0 ) {
               main->rigctlSetFilter(3);
               }
               else if (cmdlist[2].compare("6600") == 0 ) {
               main->rigctlSetFilter(4);
               }
               else if (cmdlist[2].compare("5200") == 0 ) {
               main->rigctlSetFilter(5);
               }
               else if (cmdlist[2].compare("4000") == 0 ) {
               main->rigctlSetFilter(6);
               }
               else if (cmdlist[2].compare("3100") == 0 ) {
               main->rigctlSetFilter(7);
               }
               else if (cmdlist[2].compare("2900") == 0 ) {
               main->rigctlSetFilter(8);
               }
               else if (cmdlist[2].compare("2400") == 0 ) {
               main->rigctlSetFilter(9);
               }

            } else if (cmdlist[1].compare("SAM") == 0) {
               main->rigctlSetMode(MODE_SAM);
               // compare sam filters
               if (cmdlist[2].compare("16000") == 0 ) {
               main->rigctlSetFilter(0);
               }
               else if (cmdlist[2].compare("12000") == 0 ) {
               main->rigctlSetFilter(1);
               }
               else if (cmdlist[2].compare("10000") == 0 ) {
               main->rigctlSetFilter(2);
               }
               else if (cmdlist[2].compare("8000") == 0 ) {
               main->rigctlSetFilter(3);
               }
               else if (cmdlist[2].compare("6600") == 0 ) {
               main->rigctlSetFilter(4);
               }
               else if (cmdlist[2].compare("5200") == 0 ) {
               main->rigctlSetFilter(5);
               }
               else if (cmdlist[2].compare("4000") == 0 ) {
               main->rigctlSetFilter(6);
               }
               else if (cmdlist[2].compare("3100") == 0 ) {
               main->rigctlSetFilter(7);
               }
               else if (cmdlist[2].compare("2900") == 0 ) {
               main->rigctlSetFilter(8);
               }
               else if (cmdlist[2].compare("2400") == 0 ) {
               main->rigctlSetFilter(9);
               }

            } else if (cmdlist[1].compare("CW") == 0) {
               main->rigctlSetMode(MODE_CWU);
               // compare cwu filters
               if (cmdlist[2].compare("1000") == 0 ) {
               main->rigctlSetFilter(0);
               }
               else if (cmdlist[2].compare("800") == 0 ) {
               main->rigctlSetFilter(1);
               }
               else if (cmdlist[2].compare("750") == 0 ) {
               main->rigctlSetFilter(2);
               }
               else if (cmdlist[2].compare("600") == 0 ) {
               main->rigctlSetFilter(3);
               }
               else if (cmdlist[2].compare("500") == 0 ) {
               main->rigctlSetFilter(4);
               }
               else if (cmdlist[2].compare("400") == 0 ) {
               main->rigctlSetFilter(5);
               }
               else if (cmdlist[2].compare("250") == 0 ) {
               main->rigctlSetFilter(6);
               }
               else if (cmdlist[2].compare("100") == 0 ) {
               main->rigctlSetFilter(7);
               }
               else if (cmdlist[2].compare("50") == 0 ) {
               main->rigctlSetFilter(8);
               }
               else if (cmdlist[2].compare("25") == 0 ) {
               main->rigctlSetFilter(9);
               }

            } else if (cmdlist[1].compare("CWR") == 0) {
               main->rigctlSetMode(MODE_CWL);
               // compare cwl filters
               if (cmdlist[2].compare("1000") == 0 ) {
               main->rigctlSetFilter(0);
               }
               else if (cmdlist[2].compare("800") == 0 ) {
               main->rigctlSetFilter(1);
               }
               else if (cmdlist[2].compare("750") == 0 ) {
               main->rigctlSetFilter(2);
               }
               else if (cmdlist[2].compare("600") == 0 ) {
               main->rigctlSetFilter(3);
               }
               else if (cmdlist[2].compare("500") == 0 ) {
               main->rigctlSetFilter(4);
               }
               else if (cmdlist[2].compare("400") == 0 ) {
               main->rigctlSetFilter(5);
               }
               else if (cmdlist[2].compare("250") == 0 ) {
               main->rigctlSetFilter(6);
               }
               else if (cmdlist[2].compare("100") == 0 ) {
               main->rigctlSetFilter(7);
               }
               else if (cmdlist[2].compare("50") == 0 ) {
               main->rigctlSetFilter(8);
               }
               else if (cmdlist[2].compare("25") == 0 ) {
               main->rigctlSetFilter(9);
               }    
          
            }
        } else if (command == "\\dump_state" || command[0] == '1') {
            // See dump_state in rigctl_parse.c for what this means.
            out << "0\n"; // protocol version
            out << "2" << "\n"; //RIG_MODEL_NETRIGCTL
            out << "2" << "\n"; //RIG_ITU_REGION2
            // Not sure exactly what to send here but this seems to work
            out << "150000.000000 30000000.000000  0x900af -1 -1 0x10000003 0x3\n"; //("%"FREQFMT" %"FREQFMT" 0x%x %d %d 0x%x 0x%x\n",start,end,modes,low_power,high_power,vfo,ant)
            out << "0 0 0 0 0 0 0\n";
            out << "150000.000000 30000000.000000  0x900af -1 -1 0x10000003 0x3\n"; //TX
            out << "0 0 0 0 0 0 0\n";
            out << "0 0\n";
            out << "0 0\n";
            out << "0\n";
            out << "0\n";
            out << "0\n";
            out << "0\n";
            out << "\n";
            out << "\n";
            out << "0x0\n";
            out << "0x0\n";
            out << "0x0\n";
            out << "0x0\n";
            out << "0x0\n";
            out << "0\n";
            output = true;
        } else {
            fprintf(stderr, "rigctl: unknown command \"%s\"\n", command.constData());
            retcode = -11;
        }
        //fprintf(stderr, "rigctl:  command \"%s\"\n", command.constData());
        if (!output) {
                out << "RPRT " << retcode << "\n";
        }
}

RigCtlServer::RigCtlServer(QObject *parent, UI *main)
        : QObject(parent),
          main(main) {
        server = new QTcpServer(this);
        if (!server->listen(QHostAddress::Any, 19090)) { // or listen(QHostAddress::LocalHost, 19090)) {
                fprintf(stderr, "rigctl: failed to bind socket\n");
                return;
        }
        fprintf(stderr, "rigctl: Listening\n");
        connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

void RigCtlServer::newConnection() {
        QTcpSocket *conn = server->nextPendingConnection();
        RigCtlSocket *sock = new RigCtlSocket(this, main, conn);
        connect(conn, SIGNAL(disconnected()), conn, SLOT(deleteLater()));
        connect(conn, SIGNAL(disconnected()), sock, SLOT(disconnected()));
        connect(conn, SIGNAL(readyRead()), sock, SLOT(readyRead()));
}

