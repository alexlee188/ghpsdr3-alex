#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QtDebug>
#include <QColor>


#if QT_VERSION >= 0x050000
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QDialog>
#else
#include <QMessageBox>
#include <QDialog>
#endif


#include "servers.h"
#include "ui_servers.h"
#include "UI.h"


QNetworkAccessManager* nam;


Servers::Servers(QDialog *parent) :  QDialog(parent), ui(new Ui::Servers)


{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    nam = new QNetworkAccessManager(this);
    QObject::connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishedSlot(QNetworkReply*)));
    ui->treelist->setHeaderLabels(QString("Status;Call;Location;Band;Rig;6Antenna;Last Report;IP").split(";"));
    ui->treelist->setColumnWidth( 0,140);
    //ui->treelist->setColumnWidth( 0,125);
    ui->treelist->setColumnWidth( 1,100);
    ui->treelist->setColumnWidth( 2,160);
    ui->treelist->setColumnWidth( 3,160);
    ui->treelist->setColumnWidth( 4,160);
    ui->treelist->setColumnWidth( 5,160);
    ui->treelist->setColumnWidth( 6,150);
    ui->treelist->setColumnWidth( 7,125);
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(TimerFired()));
    timer->start(60000); // 60 seconds


}

Servers::~Servers()
{
    delete ui;
}

void Servers::closeEvent(QCloseEvent *event)
{    
    emit dialogClosed();
    event->accept();
}


void Servers::on_closebutton_clicked()
{
    qDebug() << __FUNCTION__ ;
    this->close();
}

void Servers::refreshList()
{
   ui->treelist->clear();
   QUrl url("http://qtradio.napan.ca/qtradio/qtradiolist.pl");
   QNetworkReply* reply = nam->get(QNetworkRequest(url));
}

void Servers::finishedSlot(QNetworkReply* reply )
{

    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray replyBytes = reply->readAll();  // bytes
        QString replyString(replyBytes); // string
        //fprintf(stderr, "replyString length:%i\n", replyString.length());
        //qDebug() << replyString;
         QStringList list =  replyString.split("\n", QString::SkipEmptyParts);
         populateList(list);
    }
    // Some http error received
    else
    {
         fprintf(stderr, "Servers Reply error\n");
    }
    reply->deleteLater();
}

void Servers::on_refreshButton_clicked()
{
    refreshList();

}

void Servers::populateList(QStringList list)
{
    foreach (QString line, list) {
        addLine(line);
    }
}

void Servers::addLine(QString line)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->treelist);
    int pos = 0;
    QStringList fields = line.split("~");
    foreach (QString field, fields){
       item->setText(pos ,field);
      pos++;
    }
    if (line.left(4) == "Idle"|| line.left(1) == "0"){

      item->setBackgroundColor(1,QColor(Qt::green));
    }
    ui->treelist->addTopLevelItem(item);
}

void Servers::on_QuickDisconnect_clicked()
{
     qDebug() << "DisconnectNow";
     emit disconnectNow();


}

void Servers::on_QuickConnect_clicked()
{
    QTreeWidgetItem *item;
    QString IP;
    if (ui->treelist->currentItem()){
       item = ui->treelist->currentItem();
       IP = item->text(7); //IP column
       qDebug() << "IP Selected: " << IP;
       emit connectNow(IP);

    }else{
        QMessageBox msgBox;
        msgBox.setText("Nothing Selected!\nSelect a radio server to connect to first.");
        msgBox.exec();
    }

}

void Servers::TimerFired()
{
    if (this->isVisible())
    {
      //qDebug() << " Fired Refresh";
      refreshList();
    }
}
