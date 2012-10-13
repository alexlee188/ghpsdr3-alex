
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

#include "KeypadDialog.h"
#include "ui_KeypadDialog.h"

#include <QDebug>


KeypadDialog::KeypadDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KeypadDialog)
{
    ui->setupUi(this);

    ui->frequency->setStyleSheet("QLabel { background-color : black; color : green; }");


    connect(ui->pushButton_0,SIGNAL(clicked()),this,SLOT(key_0()));
    connect(ui->pushButton_1,SIGNAL(clicked()),this,SLOT(key_1()));
    connect(ui->pushButton_2,SIGNAL(clicked()),this,SLOT(key_2()));
    connect(ui->pushButton_3,SIGNAL(clicked()),this,SLOT(key_3()));
    connect(ui->pushButton_4,SIGNAL(clicked()),this,SLOT(key_4()));
    connect(ui->pushButton_5,SIGNAL(clicked()),this,SLOT(key_5()));
    connect(ui->pushButton_6,SIGNAL(clicked()),this,SLOT(key_6()));
    connect(ui->pushButton_7,SIGNAL(clicked()),this,SLOT(key_7()));
    connect(ui->pushButton_8,SIGNAL(clicked()),this,SLOT(key_8()));
    connect(ui->pushButton_9,SIGNAL(clicked()),this,SLOT(key_9()));
    connect(ui->pushButton_period,SIGNAL(clicked()),this,SLOT(key_period()));

    connect(ui->buttonBox,SIGNAL(clicked(QAbstractButton*)),this,SLOT(clicked(QAbstractButton*)));

    ui->buttonBox->button(QDialogButtonBox::Ok)->setFocus();

    frequency="";
    showFrequency();
}

KeypadDialog::~KeypadDialog()
{
    delete ui;
}

long long KeypadDialog::getFrequency() {
    return (long long)(frequency.toDouble()*1000000.0);
}

void KeypadDialog::clear() {
    frequency="";
    showFrequency();
}

void KeypadDialog::commitFrequency() {
    if ((long long)(frequency.toDouble() * 1000000.0)!=0)
        emit setKeypadFrequency((long long)(frequency.toDouble() * 1000000.0));
}

void KeypadDialog::clicked(QAbstractButton* button) {
    qDebug()<<"KeypadDialog::clicked "<<button->text();
    if(button->text()=="&OK") {
        commitFrequency();
    } else if(button->text()=="Reset") {
        frequency="";
        showFrequency();
    } else {
    }
}

void KeypadDialog::key_0() {
    //frequency=frequency*10;
    frequency.append("0");
    showFrequency();
}

void KeypadDialog::key_1() {
    //frequency=(frequency*10)+1;
    frequency.append("1");
    showFrequency();
}

void KeypadDialog::key_2() {
    //frequency=(frequency*10)+2;
    frequency.append("2");
    showFrequency();
}

void KeypadDialog::key_3() {
    //frequency=(frequency*10)+3;
    frequency.append("3");
    showFrequency();
}

void KeypadDialog::key_4() {
    //frequency=(frequency*10)+4;
    frequency.append("4");
    showFrequency();
}

void KeypadDialog::key_5() {
    //frequency=(frequency*10)+5;
    frequency.append("5");
    showFrequency();
}

void KeypadDialog::key_6() {
    //frequency=(frequency*10)+6;
    frequency.append("6");
    showFrequency();
}

void KeypadDialog::key_7() {
    //frequency=(frequency*10)+7;
    frequency.append("7");
    showFrequency();
}

void KeypadDialog::key_8() {
    //frequency=(frequency*10)+8;
    frequency.append("8");
    showFrequency();
}

void KeypadDialog::key_9() {
    //frequency=(frequency*10)+9;
    frequency.append("9");
    showFrequency();
}

void KeypadDialog::key_period() {
    //frequency=frequency*1000;
    if(frequency.count(".")==0) {
        frequency.append(".");
        showFrequency();
    }
}

void KeypadDialog::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_0:
            frequency.append("0");
            showFrequency();
            break;
        case Qt::Key_1:
            frequency.append("1");
            showFrequency();
            break;
        case Qt::Key_2:
            frequency.append("2");
            showFrequency();
            break;
        case Qt::Key_3:
            frequency.append("3");
            showFrequency();
            break;
        case Qt::Key_4:
            frequency.append("4");
            showFrequency();
            break;
        case Qt::Key_5:
            frequency.append("5");
            showFrequency();
            break;
        case Qt::Key_6:
            frequency.append("6");
            showFrequency();
            break;
        case Qt::Key_7:
            frequency.append("7");
            showFrequency();
            break;
        case Qt::Key_8:
            frequency.append("8");
            showFrequency();
            break;
        case Qt::Key_9:
            frequency.append("9");
            showFrequency();
            break;
        case Qt::Key_Period:
            if (frequency.count(".") == 0) {
                frequency.append(".");
                showFrequency();
            }
            break;

        case Qt::Key_Backspace:
            if (frequency.length() > 0) {
                frequency.remove(frequency.length()-1, 1);
                showFrequency();
            }
            break;

        default:
            QDialog::keyPressEvent(event);
    }
}

void KeypadDialog::showFrequency() {
    //QString f;
    //f.sprintf("%lld.%03lld.%03lld",frequency/1000000,frequency%1000000/1000,frequency%1000);
    ui->frequency->setText(frequency);
}
