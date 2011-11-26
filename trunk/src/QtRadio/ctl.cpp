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
