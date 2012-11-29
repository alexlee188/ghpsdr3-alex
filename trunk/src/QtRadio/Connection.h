/*
 * File:   Connection.h
 * Author: John Melton, G0ORX/N6LYT
 *
 * Created on 16 August 2010, 07:40
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

#ifndef CONNECTION_H
#define	CONNECTION_H

#include <QObject>
#include <QDebug>
#include <QTcpSocket>
#include <QTimer>
#include <QMutex>
#include <QQueue>
#include <QSemaphore>

#include "Buffer.h"

#define SPECTRUM_BUFFER     0
#define AUDIO_BUFFER        1
#define BANDSCOPE_BUFFER    2
#define RTP_REPLY_BUFFER    3
#define ANSWER_BUFFER       4

// minimum supported header version
#define HEADER_VERSION 2
#define HEADER_SUBVERSION 0

// g0orx binary header
#define HEADER_SIZE_2_0 13
#define HEADER_SIZE_2_1 15
#define AUDIO_HEADER_SIZE 5
#define AUDIO_LENGTH_POSITION 1

#define SEND_BUFFER_SIZE 64

#define READ_HEADER 0
#define READ_BUFFER 1
#define READ_HEADER_TYPE 3
#define READ_AUDIO_HEADER 4
#define READ_RTP_REPLY 5
#define READ_ANSWER 6

class Connection : public QObject {
    Q_OBJECT
public:
    Connection();
    virtual ~Connection();
    void connect(QString host,int receiver);
    void sendCommand(QString command);
    void sendAudio(int length,unsigned char* buffer);
    void freeBuffers(char* header,char* buffer);
    QSemaphore SemSpectrum;
    void setMuted(bool);
    QString getHost();
    bool getSlave();

public slots:
    void connected();
    void disconnected();
    void disconnect();
    void socketError(QAbstractSocket::SocketError socketError);
    void socketData();
    void processBuffer();

signals:
    void isConnected();
    void disconnected(QString message);
    void header(char* header);
    void audioBuffer(char* header,char* buffer);
    void spectrumBuffer(char* header,char* buffer);
    void bandscopeBuffer(char* header,char* buffer);
    void remoteRTP(char* host,int port);
    void printStatusBar(QString message);
    void slaveSetFreq(long long f);
    void slaveSetMode(int m);
    void slaveSetFilter(int l, int r);
    void slaveSetZoom(int z);
    void setdspversion(long, QString);
    void setservername( QString);
    void setRemoteRTPPort(QString,int);
    void setCanTX(bool);
    void setChkTX(bool);  // password style server
    void resetbandedges(double loffset);
    void setFPS();
    void setProtocol3(bool);
    void hardware (QString);

private:
    // really not used (and not even implemented)
    // defined as private in order to prevent unduly usage 
    Connection(const Connection& orig);

    QString host;
    int port;
    QTcpSocket* tcpSocket;
    QMutex mutex;
    int state;
    char* hdr;
    char* buffer;
    short length;   // int causes errors in converting 2 char bytes to integer
    int bytes;
    //char* ans;
    //QString answer;
    bool muted;
    bool amSlave;
    long long lastFreq;
    int lastMode;
    int lastSlave;
    long serverver;
    bool initialTxAllowedState;
    QQueue<Buffer*> queue;
};

#endif	/* CONNECTION_H */

