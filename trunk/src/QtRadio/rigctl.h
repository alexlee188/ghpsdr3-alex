#ifndef RIGCTL_H
#define RIGCTL_H

#include <QtCore/QObject>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

class UI;

class RigCtlSocket : public QObject {
        Q_OBJECT

        public:
                RigCtlSocket(QObject *parent = 0, UI *main = 0,
                             QTcpSocket *conn = 0);

        public slots:
                void disconnected(void);
                void readyRead(void);

        private:
                UI *main;
                QTcpSocket *conn;
};

class RigCtlServer : public QObject {
        Q_OBJECT

        public:
                RigCtlServer(QObject *parent = 0, UI *main = 0, unsigned short rigctl_port = RIGCTL_PORT);
                static const unsigned short RIGCTL_PORT;

        public slots:
                void newConnection(void);

        private:
                UI *main;
                QTcpServer *server;
};
#endif // RIGCTL_H
