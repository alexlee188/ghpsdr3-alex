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
        moxPwr = ui->pwrSlider->value();
    } else if(ui->btnTune->isChecked()) {
        TunePwr = ui->pwrSlider->value();
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
