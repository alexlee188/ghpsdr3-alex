/* File:   ctl.cpp
 * Author: Graeme Jury, ZL2APV
 *
 * Created on 16 September 2011, 17:34
 */

/* Copyright (C)
* 2011 - Graeme Jury, ZL2APV
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
#include "ctl.h"
#include "ui_ctl.h"
#include <QDebug>

Ctl::Ctl(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::Ctl)
{
    ui->setupUi(this);

    moxPwr = 100;
    TunePwr = 50;
    ui->pwrSlider->setValue(moxPwr);
<<<<<<< HEAD
    HideTX(false); // Hide buttons because we have not connected to anything yet
=======
    ui->pwrSlider_2->setValue(0);
    ui->spinBox->setMaximum(100);
>>>>>>> Connected test controls to main UI
}

Ctl::~Ctl()
{
    delete ui;
}

void Ctl::on_btnMox_clicked(bool checked)
{
    bool ptt;

    ui->btnTune->setChecked(FALSE); //Override the tune button
    if(checked) { //We are going from Rx to Tx
        ui->btnMox->setChecked(TRUE);
        ptt = TRUE;
        ui->pwrSlider->setValue(moxPwr);
    }
    else {
        ui->btnMox->setChecked(FALSE);
        ptt = FALSE;
    }
    emit pttChange(0, ptt);
}

void Ctl::on_btnTune_clicked(bool checked)
{
    bool ptt;

    ui->btnMox->setChecked(FALSE); //Override the MOX button
    if(checked) { //We are going from Rx to Tx
        ui->btnTune->setChecked(TRUE);
        ptt = TRUE;
        ui->pwrSlider->setValue(TunePwr);
    }
    else {
        ui->btnTune->setChecked(FALSE);
        ptt = FALSE;
    }
    emit pttChange(1, ptt);
}

void Ctl::on_pwrSlider_valueChanged(int value)
{
    if(ui->btnMox->isChecked()) {
        moxPwr = ui->pwrSlider->value(); //Do nothing until Tx power level adj written for DttSP
    } else if(ui->btnTune->isChecked()) {
        TunePwr = ui->pwrSlider->value();
        emit pwrSlider_valueChanged(((double)TunePwr)/100);
    }
}

int Ctl::getTxPwr()
{
    return ui->pwrSlider->value();
}

void Ctl::update_mic_level(qreal level){
    ui->MicProgressBar->setValue(100*level);
}

void Ctl::clearMoxBtn()
{
    ui->btnMox->setChecked(false);
    ui->btnTune->setChecked(false);
}

<<<<<<< HEAD
void Ctl::HideTX(bool cantx){
    if (cantx){
        ui->btnMox->setEnabled(true);
        ui->btnTune->setEnabled(true);
    }else{
        clearMoxBtn();
        ui->btnMox->setEnabled(false);
        ui->btnTune->setEnabled(false);
    }
}
=======
void Ctl::on_checkBox_stateChanged(int arg1)
{
    if(ui->checkBox->isChecked()){
        ui->pushButton->setCheckable(true);
    } else {
        ui->pushButton->setCheckable(false);
    }
}

void Ctl::on_pwrSlider_2_valueChanged(int value)
{
    if(ui->spinBox->value()!=value) ui->spinBox->setValue(value);
    emit testSliderChange(value);
    qDebug()<<Q_FUNC_INFO<<":   The value of slider = "<<value;
}

void Ctl::on_spinBox_valueChanged(int arg1)
{
    if(ui->pwrSlider_2->value()!=arg1) {
        ui->pwrSlider_2->setValue(arg1);
    } else {
        emit testSliderChange(arg1);
        qDebug()<<Q_FUNC_INFO<<":   The value of slider = "<<arg1;
    }
}

void Ctl::on_pushButton_pressed()
{
    if(!ui->pushButton->isCheckable()) {
//        qDebug()<<Q_FUNC_INFO<<":   The state of the pushbutton is "<<true;
        emit testBtnClick(true);
    }
}

void Ctl::on_pushButton_released()
{
    if(!ui->pushButton->isCheckable()) {
//        qDebug()<<Q_FUNC_INFO<<":   The state of the pushbutton is "<<false;
        emit testBtnClick(false);
    }
}

void Ctl::on_pushButton_toggled(bool checked)
{
//    qDebug()<<Q_FUNC_INFO<<":   The state of the pushbutton is "<<checked;
    emit testBtnClick(checked);
}
>>>>>>> Created test pushbutton, slider & spinbox
