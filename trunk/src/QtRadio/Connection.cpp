/* 
 * File:   Connection.cpp
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

#include "Connection.h"

Connection::Connection() {
    qDebug() << "Connection::Connection";
    tcpSocket=NULL;
    state=READ_HEADER_TYPE;
    bytes=0;
    hdr=(char*)malloc(HEADER_SIZE_2_1);  // HEADER_SIZE is larger than AUTIO_HEADER_SIZE so it is OK
                                    // for both
    SemSpectrum.release();
    muted = false;
}

//Connection::Connection(const Connection& orig) {
//    qDebug() << "Connection::Connection: copy constructor";
//}

Connection::~Connection() {
    qDebug() << "Connection::~Connection";
}

void Connection::connect(QString h,int p) {
    host=h;
    port=p;

    // cleanup previous object, if any
    if (tcpSocket) {
        delete tcpSocket;
    }

    tcpSocket=new QTcpSocket(this);

    QObject::connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketError(QAbstractSocket::SocketError)));

    QObject::connect(tcpSocket, SIGNAL(connected()),
            this, SLOT(connected()));

    QObject::connect(tcpSocket, SIGNAL(disconnected()),
            this, SLOT(disconnected()));

    QObject::connect(tcpSocket, SIGNAL(readyRead()),
            this, SLOT(socketData()));

    // set the initial state
    state=READ_HEADER_TYPE;
    // cleanup dirty value eventually left from previous usage
    bytes=0;
    qDebug() << "Connection::connect: connectToHost: " << host << ":" << port;
    tcpSocket->connectToHost(host,port);

}

void Connection::disconnected() {
    qDebug() << "Connection::disconnected: emits: " << "Remote disconnected";
    emit disconnected("Remote disconnected");
    if(tcpSocket!=NULL) {
        QObject::disconnect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(socketError(QAbstractSocket::SocketError)));

        QObject::disconnect(tcpSocket, SIGNAL(connected()),
                this, SLOT(connected()));

        QObject::disconnect(tcpSocket, SIGNAL(disconnected()),
                this, SLOT(disconnected()));

        QObject::disconnect(tcpSocket, SIGNAL(readyRead()),
                this, SLOT(socketData()));

    }
}

void Connection::disconnect() {

    qDebug() << "Connection::disconnect Line 100";
    if(tcpSocket!=NULL) {
        tcpSocket->close();
        // object deletion moved in connect method 
        // tcpSocket=NULL;

    }
}

void Connection::socketError(QAbstractSocket::SocketError socketError) {
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
            qDebug() << "Remote closed connection";
            break;
        case QAbstractSocket::HostNotFoundError:
            qDebug() << "Host not found";
            break;
        case QAbstractSocket::ConnectionRefusedError:
            qDebug() << "Remote host refused connection";
            break;
        default:
            qDebug() << "Socket Error: " << tcpSocket->errorString();
    }

    emit disconnected(tcpSocket->errorString());
    // memory leakeage !! 
    // tcpSocket=NULL;
}

void Connection::connected() {
    qDebug() << "Connection::Connected" << tcpSocket->isValid();
    emit isConnected();
    state=READ_HEADER_TYPE;
}

void Connection::sendCommand(QString command) {
    int i;
    char buffer[SEND_BUFFER_SIZE];
    int bytesWritten;

    for (i=0; i < SEND_BUFFER_SIZE; i++) buffer[i] = 0;
    if(tcpSocket!=NULL && tcpSocket->isValid() && tcpSocket->isWritable()) {
        mutex.lock();
        strcpy(buffer,command.toUtf8().constData());
        bytesWritten = tcpSocket->write(buffer,SEND_BUFFER_SIZE);
        if (bytesWritten != SEND_BUFFER_SIZE) qDebug() << "sendCommand: write error";
        //tcpSocket->flush();
        mutex.unlock();
    }
}

void Connection::sendAudio(int length, unsigned char* data) {
    QString command;
    char buffer[SEND_BUFFER_SIZE];
    int i;
    int bytesWritten;

    for (i=0; i < SEND_BUFFER_SIZE; i++) buffer[i] = 0;
    if(tcpSocket!=NULL && tcpSocket->isValid() && tcpSocket->isWritable()) {
        QTextStream(&command) << "mic ";
        strcpy(buffer,command.toUtf8().constData());
        memcpy(&buffer[4], data,length);
        mutex.lock();
        bytesWritten = tcpSocket->write(buffer, SEND_BUFFER_SIZE);
        if (bytesWritten != SEND_BUFFER_SIZE) qDebug() << "sendCommand: write error";
        //tcpSocket->flush();
        mutex.unlock();
    }
}

void Connection::socketData() {

    int toRead;
    int bytesRead=0;
    int thisRead;
    int version;
    int subversion;
    int header_size;

    if (bytes < 0) {
        fprintf(stderr,"QtRadio: FATAL: INVALID byte counter: %d\n", bytes);
        tcpSocket->close();
        return;
    }            
    toRead=tcpSocket->bytesAvailable();
    if (toRead < 0) {
        fprintf(stderr,"QtRadio: FATAL: error in bytesAvailable: %d\n", toRead);
        tcpSocket->close();
        return;
    }
    while(bytesRead<toRead) {
        //fprintf (stderr, "%d of %d [%d]\n", bytesRead, toRead, state);
        switch(state) {
        case READ_HEADER_TYPE:
            thisRead=tcpSocket->read(&hdr[0],3);

            if (thisRead == 3) 
               bytes+=3;
            else {
                 fprintf(stderr,"QtRadio: FATAL: only %d read instead of 3\n", thisRead);
                 tcpSocket->close();
                 break;
            } 
            if (hdr[0] == AUDIO_BUFFER)
                state=READ_AUDIO_HEADER;
            else {
                version=hdr[1];
                subversion=hdr[2];
                switch(version) {
                    case 2:
                        switch(subversion) {
                            case 0:
                                header_size=HEADER_SIZE_2_0;
                                break;
                            case 1:
                                header_size=HEADER_SIZE_2_1;
                                break;
                            default:
                                fprintf(stderr,"QtRadio: Invalid subversion. Expected %d.%d got %d.%d\n",HEADER_VERSION,HEADER_SUBVERSION,version,subversion);
                                break;
                        }
                        break;
                    default:
                        fprintf(stderr,"QtRadio: Invalid version. Expected %d.%d got %d.%d\n",HEADER_VERSION,HEADER_SUBVERSION,version,subversion);
                        break;
                }
                state=READ_HEADER;
            }
            break;

        case READ_AUDIO_HEADER:
            //fprintf (stderr, "READ_AUDIO_HEADER: hdr size: %d bytes: %d\n", AUDIO_HEADER_SIZE, bytes);
            thisRead=tcpSocket->read(&hdr[bytes],AUDIO_HEADER_SIZE - bytes);
            bytes+=thisRead;
            if ((bytes == AUDIO_HEADER_SIZE)){
// g0orx binary header
                //length = atoi(&hdr[AUDIO_LENGTH_POSITION]);
                length=((hdr[3]&0xFF)<<8)+(hdr[4]&0xFF);
                if ((length < 0) || (length > 4800 * 8)){
                    fprintf(stderr,"Connection: length of audio_header out of bounds = %d\n", length);
                    state = READ_HEADER_TYPE;
                } else {
                    buffer = (char*)malloc(length);
                    bytes = 0;
                    state = READ_BUFFER;
                }
            } else {
            }
            break;

         case READ_HEADER:
            //fprintf (stderr, "READ_HEADER: hdr size: %d bytes: %d\n", header_size, bytes);
            thisRead=tcpSocket->read(&hdr[bytes],header_size - bytes);
            bytes+=thisRead;
            if(bytes==header_size) {
// g0orx binary header
                length=((hdr[3]&0xFF)<<8)+(hdr[4]&0xFF);
                if ((length < 0) || (length > 4096)){
                        state = READ_HEADER_TYPE;
                }
                else {
                    buffer=(char*)malloc(length);
                    bytes=0;
                    state=READ_BUFFER;
                }
            } else {
            }
            break;

        case READ_BUFFER:
            //fprintf (stderr, "READ_BUFFER: length: %d bytes: %d\n", length, bytes);
            thisRead=tcpSocket->read(&buffer[bytes],length-bytes);
            bytes+=thisRead;
            //qDebug() << "READ_BUFFER: read " << bytes << " of " << length;
            if(bytes==length) {
                version=hdr[1];
                subversion=hdr[2];
                queue.enqueue(new Buffer(hdr,buffer));
                QTimer::singleShot(0,this,SLOT(processBuffer()));
                hdr=(char*)malloc(HEADER_SIZE_2_1);
                bytes=0;
                state=READ_HEADER_TYPE;
            } else {
            }
            break;
        default:
            fprintf (stderr, "FATAL: WRONG STATUS !!!!!\n");         
        }
        bytesRead+=thisRead;
    }
}


void Connection::processBuffer() {
    Buffer* buffer;
    char* nextHeader;
    char* nextBuffer;

// We only want to mute the audio (actually not for full duplex)
// the spectrum display should show the microphone waveform
/*
    if(muted) { // If Rx muted, clear queue and don't process buffer - gvj
        queue.clear();
    }
*/
    while (!queue.isEmpty()){
        buffer=queue.dequeue();
        nextHeader=buffer->getHeader();
        nextBuffer=buffer->getBuffer();
        // emit a signal to show what buffer we have
        //qDebug() << "processBuffer " << nextHeader[0];
        if(nextHeader[0]==SPECTRUM_BUFFER){
            emit spectrumBuffer(nextHeader,nextBuffer);
        }
        else if(nextHeader[0]==AUDIO_BUFFER) {
            // need to add a duplex state
            if(!muted) emit audioBuffer(nextHeader,nextBuffer);
        } else if(nextHeader[0]==BANDSCOPE_BUFFER) {
            //qDebug() << "socketData: bandscope";
            emit bandscopeBuffer(nextHeader,nextBuffer);
        } else {
            qDebug() << "Connection::socketData: invalid header: " << nextHeader[0];
            queue.clear();
        }
    }
}

void Connection::freeBuffers(char* header,char* buffer) {
    if (header != NULL) free(header);
    if (buffer != NULL) free(buffer);
}

// added by gvj
void Connection::setMuted(bool muteState)
{
    muted = muteState;
}
