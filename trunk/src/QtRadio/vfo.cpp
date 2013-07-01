/* File:   vfo.cpp
 * Author: Graeme Jury, ZL2APV
 *
 * Created on 21 August 2011, 20:00
 *
 * Griffin Powermate Vfo Knob support
 * added by Oliver Goldenstein, DL6KBG
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
#include "vfo.h"
#include "ui_vfo.h"
#include <QDebug>
#include <QKeyEvent>

#if defined(LINUX)
  #include "powermate.h"
#endif

//#include "Band.h"
//#include "UI.h"

vfo::vfo(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::vfo)
{
    ui->setupUi(this);
    vfohotstep = 100;
    curstep = 2;
    setStepMark();
    selectedVFO = 'A';
    ptt = false;

//  setBandButton group ID numbers;
    ui->btnGrpBand->setId(ui->bandBtn_00, 0); // 160
    ui->btnGrpBand->setId(ui->bandBtn_01, 1); // 80
    ui->btnGrpBand->setId(ui->bandBtn_02, 2);
    ui->btnGrpBand->setId(ui->bandBtn_03, 3);
    ui->btnGrpBand->setId(ui->bandBtn_04, 4);
    ui->btnGrpBand->setId(ui->bandBtn_05, 5);
    ui->btnGrpBand->setId(ui->bandBtn_06, 6); // etc.
    ui->btnGrpBand->setId(ui->bandBtn_07, 7);
    ui->btnGrpBand->setId(ui->bandBtn_08, 8);
    ui->btnGrpBand->setId(ui->bandBtn_09, 9);
    ui->btnGrpBand->setId(ui->bandBtn_10, 10); // 6
    ui->btnGrpBand->setId(ui->bandBtn_11, 11); // GEN
    ui->btnGrpBand->setId(ui->bandBtn_12, 12); // WWV
    connect(ui->btnGrpBand, SIGNAL(buttonClicked(int)),
                this, SLOT(btnGrpClicked(int)));
    connect(ui->hSlider, SIGNAL(valueChanged(int)),
                this, SLOT(processRIT(int)));

// Powermate related stuff
#if defined(LINUX)
    PmInput *input = new PmInput();
    connect(input, SIGNAL(pressed()), this, SLOT(press()));
    connect(input, SIGNAL(released()), this, SLOT(release()));
    connect(input, SIGNAL(rotated(int)), this, SLOT(increase(int)));
    input->start();
#endif

}

vfo::~vfo()
{
    delete ui;
}

void vfo::setFrequency(int freq)
{
    if (selectedVFO == 'A' || selectedVFO == 'S') {
        writeA(freq);
    } else if (!ui->pBtnSubRx->isChecked()) {
        writeB(freq);
    } else writeA(freq);
}

void vfo::processRIT(int rit)  //rit holds slider value
{
    static int prevRITfreq = 0;

    if (ui->pBtnRIT->isChecked()) {
        emit frequencyMoved(prevRITfreq - rit, 1);
    }
    prevRITfreq = rit;
}

void vfo::on_pBtnRIT_clicked()
{
    int chkd = 1;

    if (ui->pBtnRIT->isChecked()) {
        chkd = -1;
    }
    if (ui->hSlider->value() != 0) {
        emit frequencyMoved(ui->hSlider->value() * chkd, 1);
    }
}

void vfo::btnGrpClicked(int btn)
{
    emit bandBtnClicked(btn);
}

void vfo::timerEvent(QTimerEvent *event)
 {
    if (event->timerId() == timer.timerId()) {
        timer.stop();
        ui->btnGrpBand->checkedButton()->setStyleSheet("background-color: normal");
    } else {
        QWidget::timerEvent(event);
    }
}

void vfo::mousePressEvent(QMouseEvent *event)
{
    bool isVFOa = false;
    int digit, cnt;
    QString myStr = "";
    long long freq;

    if (event->button() == Qt::RightButton) {

//qDebug()<<Q_FUNC_INFO<<": event x/y = "<<event->x()<<"/"<<event->y();

        //Check to see if we have right clicked on the band button group
        if ((event->x() > 414) && (event->x() < 573) &&
            (event->y() > 6) && (event->y() < 111)) {
//            storeVFO(); //make the selected button flash yellow & call store routine
            timer.start(500,this);
            ui->btnGrpBand->checkedButton()->setStyleSheet("background-color: yellow");
            emit rightBandClick();  //connected to Band:quickMemStore via UI:quickMemStore

        }   // Check to see if we have right clicked the RIT slider
        else if ((event->x() > 189) && (event->x() < 403) &&
                 (event->y() > 89) && (event->y() < 111)) {
                ui->hSlider->setValue(0);
        }
        // We have clicked either on the display or somewhere else on the widget

        else {  // Check to see if we have clicked outside the vfo display area
            digit = getDigit(event->x(), event->y());
//qDebug()<<Q_FUNC_INFO<<": The value of digit is ..."<<digit;
            if (digit != 9) {       // getDigit returns 9 if click was outside display area.
                if (digit < 9) {    // getDigit returns 0 ... 8 if we clicked on vfoA
                    freq = readA();
                    isVFOa = true;
                    myStr = ui->lbl_Amhz->text() + ui->lbl_Akhz->text() + ui->lbl_Ahz->text();
                }
                else {                  // getDigit returns 10 ... 18 if we clicked on vfoB
                    digit = digit - 10; // so convert to 1 ... 8.
                    freq = readB();
                    myStr = ui->lbl_Bmhz->text() + ui->lbl_Bkhz->text() + ui->lbl_Bhz->text();
                }
                for (cnt = myStr.length(); cnt < 9; cnt++) {
                    myStr = "0" + myStr;
                }
                for (cnt = digit; cnt < 9; cnt++) {
                    myStr[cnt] = QChar('0');
                    ui->hSlider->setValue(0);
                }
                freq = freq - myStr.toLongLong();
                if (isVFOa) {   //We right clicked on vfoA
                    if(selectedVFO == 'A' || selectedVFO == 'S') {
                        emit frequencyMoved(freq, 1);
//qDebug()<<Q_FUNC_INFO<<": vfoA, emit frequencyChanged(myStr.toLongLong()) = "<<freq;
                    }
                    else {
                        writeA(myStr.toLongLong());
//qDebug()<<Q_FUNC_INFO<<": vfoA, writeA(myStr.toInt()) = "<<freq;
                    }
                }
//                else if(ui->pBtnSubRx->isChecked()) { //We right clicked on vfoB
//qDebug()<<Q_FUNC_INFO<<": vfoB and subRx mode";
//                }
                else if(selectedVFO == 'B') {
                        emit frequencyMoved(freq, 1);
//qDebug()<<Q_FUNC_INFO<<": Line 187 ... vfoB, emit frequencyMoved(freq, 1) = "<<freq;
                }
                else {
                        writeB(myStr.toLongLong());
//qDebug()<<Q_FUNC_INFO<<": vfoB, writeA(myStr.toInt()) = "<<freq;
                }
            }
        }
    }   //If event not right button or getDigit = 9, fall thru to here with no processing.
}

int vfo::getDigit(int x, int y)
{
    static const int idx[] = {64,87,107,137,159,178,207,226,241,257}; //These are the LHS of each digit of the frequency.
    int digit;

    if ((y < 12 || y > 84) || (x <64 || x > 257)) {
            digit = 9; //digit will = 9 if outside x or y dial display range.
    }
    else {
        for (digit = 0; idx[digit] != 257; digit++) {  // we exit this loop with digit at the wheeled digit.
            if ((x >= idx[digit])  && (x <= idx[digit + 1])) break;
        } // digit will be 0 ... 9 if inside x range or 9 if under or over range.

     //   if (y < 12 || y > 84) digit = 9; //digit will = 9 if outside x or y dial display range.
        if (y > 48 && y < 85) digit = digit + 10; //10 to 18 signifies vfoB
    }
    return digit; //0 ... 8 = vfoA; 9 = mouse pointer is outside x or y range; 10 ... 18 = vfoB.
}

void vfo::wheelEvent(QWheelEvent *event)
{
    QString str;
    long long x;
    int digit;
    int direction = 1;
    bool isVfoA = true;
    static const int mult[9] = {100000000,10000000,1000000,100000,10000,1000,100,10,1};

    digit = getDigit(event->x(), event->y());
    if (digit != 9) {  // getDigit returns 9 if click was outside display area so we just fall through.
        if (event->delta() < 0) direction = -1;  // x becomes pos or neg depending on wheel rotation.
        if (digit > 9) { // getDigit returns 10 ... 18 if we clicked on vfoB
            digit = digit - 10; // so convert to 1 ... 8.
            isVfoA = false;
        }
        x = mult[digit]*direction;
        if(isVfoA) {    //If true we scrolled on vfoA
//qDebug()<<Q_FUNC_INFO<<"The value of x = "<<", & readA() = "<<readA();
            if(selectedVFO == 'A' || selectedVFO == 'S') {
                emit frequencyMoved(x, 1);
            }
            else {
                writeA(readA() - x);
            }
        }
        else {  //We scrolled on vfoB
//qDebug()<<Q_FUNC_INFO<<"The value of x = "<< x<<", & readB() = "<<readB();
            if(selectedVFO == 'B') {
                emit frequencyMoved(x, 1);
            }
            else {
                writeB(readB() - x);
            }
        }
    }  //We fall through to here without processing the wheel if getDigit returns 9.
}



void vfo::writeA(long long freq)
{


    QString myStr;
    QString uOn;
    QString uOff;
    int cnt = 0;
    int stgChrs;

    myStr.setNum(freq);
    stgChrs = myStr.size() -1;
    ui->lbl_Ahz->setText("");  // Clear the screen for VFO A
    ui->lbl_Akhz->setText("");
    ui->lbl_Amhz->setText("");
    for (cnt = stgChrs; cnt > -1; cnt--) {
        if (stgChrs - cnt < 3) ui->lbl_Ahz->setText(uOn+myStr.at(cnt)+uOff+ui->lbl_Ahz->text());
        else if (stgChrs - cnt < 6) ui->lbl_Akhz->setText(uOn+myStr.at(cnt)+uOff+ui->lbl_Akhz->text());
        else ui->lbl_Amhz->setText(uOn+myStr.at(cnt)+uOff+ui->lbl_Amhz->text());
    }
}

void vfo::writeB(long long freq)
{
    QString myStr;
    int cnt = 0;
    int stgChrs;

    myStr.setNum(freq);
    stgChrs = myStr.size() -1;
    ui->lbl_Bhz->setText("");  // Clear the screen for VFO B
    ui->lbl_Bkhz->setText("");
    ui->lbl_Bmhz->setText("");
    for (cnt = stgChrs; cnt > -1; cnt--)
    {
        if (stgChrs - cnt < 3) ui->lbl_Bhz->setText(myStr.at(cnt)+ui->lbl_Bhz->text());
        else if (stgChrs - cnt < 6) ui->lbl_Bkhz->setText(myStr.at(cnt)+ui->lbl_Bkhz->text());
        else ui->lbl_Bmhz->setText(myStr.at(cnt)+ui->lbl_Bmhz->text());
    }
}

void vfo::checkBandBtn(int band)
{
//qDebug()<<Q_FUNC_INFO<<": Value of band button is ... "<<band;
    ui->btnGrpBand->button(band)->setChecked(true);
}

long long vfo::readA()
{
    QString myStr;

    myStr = (ui->lbl_Amhz->text() + ui->lbl_Akhz->text() + ui->lbl_Ahz->text());
    return myStr.toLongLong();
}

long long vfo::readB()
{
    QString myStr;

    myStr = ui->lbl_Bmhz->text() + ui->lbl_Bkhz->text() + ui->lbl_Bhz->text();
    return myStr.toLongLong();
}

void vfo::on_pBtnvfoA_clicked()
{
    if (selectedVFO != 'A' && ptt == false) {
        if (ui->pBtnRIT->isChecked()) {
            ui->pBtnRIT->setChecked(false);
            on_pBtnRIT_clicked();
        }
        selectedVFO = 'A';
        setVfoBtnColour();
        vfoEnabled(true, false);
        emit frequencyChanged(readA());
    }
}

void vfo::on_pBtnvfoB_clicked()
{
    if (selectedVFO != 'B' && ptt == false) {
        if (ui->pBtnRIT->isChecked()) {
            ui->pBtnRIT->setChecked(false);
            on_pBtnRIT_clicked();
        }
        selectedVFO = 'B';
        setVfoBtnColour();
        vfoEnabled(false, true);
        emit frequencyChanged(readB());
    }
}

void vfo::vfoEnabled(bool setA, bool setB)
// Set the screen info for the vfo in use to enabled.
{
    ui->lbl_Ahz->setEnabled(setA); //These are the labels used
    ui->lbl_Akhz->setEnabled(setA);//to draw the frequency display.
    ui->lbl_Amhz->setEnabled(setA);
    ui->label->setEnabled(setA);   //The MHz decimal
    ui->label_2->setEnabled(setA); //The Khz comma
    ui->lbl_Bhz->setEnabled(setB);
    ui->lbl_Bkhz->setEnabled(setB);
    ui->lbl_Bmhz->setEnabled(setB);
    ui->label_3->setEnabled(setB);
    ui->label_4->setEnabled(setB);
}

void vfo::on_pBtnSplit_clicked()
{
    if (selectedVFO != 'S' && ptt == false) {
        if (ui->pBtnRIT->isChecked()) {
            ui->pBtnRIT->setChecked(false);
            on_pBtnRIT_clicked();
        }
        selectedVFO = 'S';
        setVfoBtnColour();
        vfoEnabled(true, false);
        emit frequencyChanged(readA());
    }
}

void vfo::on_pBtnScanDn_clicked()
{
    ui->pBtnScanUp->setChecked(false);
}

void vfo::on_pBtnScanUp_clicked()
{
    ui->pBtnScanUp->setChecked(false);
}

void vfo::on_toolBtnUp_clicked()
{
    emit vfoStepBtnClicked(-1);  //Direction: 1 = down, -1 = up
}

void vfo::on_toolBtnDn_clicked()
{
    emit vfoStepBtnClicked(1);
}

void vfo::on_pBtnExch_clicked()
{
    long long exchTemp1;
    long long exchTemp2;

    exchTemp1 = readA();
    exchTemp2 = readB();

    writeA(exchTemp2);
    writeB(exchTemp1);
    if(selectedVFO == 'B') {
        emit frequencyChanged(exchTemp1);
    } else {
        emit frequencyChanged(exchTemp2);
    }
}

void vfo::on_pBtnAtoB_clicked()
{
    writeB(readA());
    emit frequencyChanged(readA()); //Doesn't matter which vfo as both now the same freq
}

void vfo::on_pBtnBtoA_clicked()
{
    writeA(readB());
    emit frequencyChanged(readB());
}

void vfo::readSettings(QSettings* settings)
{
    settings->beginGroup("vfo");
    writeB(settings->value("vfoB_f",14234567).toInt()); // vfoA initial settings from band.ccp
    settings->endGroup();
}

void vfo::writeSettings(QSettings* settings)
{
    settings->beginGroup("vfo");
    settings->setValue("vfoB_f", readB());
    settings->endGroup();
}


void vfo::on_pBtnSubRx_clicked()
{
//qDebug()<<Q_FUNC_INFO<<": Here I am!!!";
    emit subRxButtonClicked(); //Connected to "void UI::actionSubRx()"
}

//Called when subRx is checked in main menu via actionSubRx()
void vfo::checkSubRx(long long f, int samplerate)
{
    long long vfoAfreq;

    if(selectedVFO == 'B') {
        on_pBtnBtoA_clicked();
    }
    vfoAfreq = readA();
    if ((f < (vfoAfreq - (samplerate / 2))) || (f > (vfoAfreq + (samplerate / 2)))) {
        f=vfoAfreq;
    }
    writeB(f);
    emit frequencyChanged(readA());
//    on_pBtnvfoA_clicked();
    ui->pBtnvfoB->setText("subRx");
    ui->pBtnvfoA->setText("mainRx");
    selectedVFO = 'B';
    ui->pBtnvfoB->setStyleSheet("background-color: rgb(85, 255, 0)");
    ui->pBtnvfoA->setStyleSheet("background-color: normal");
    ui->pBtnSplit->setStyleSheet("background-color: normal");
    ui->pBtnvfoA->setEnabled(false);
    vfoEnabled(false, true);

//    ui->pBtnvfoA->setEnabled(false);
//qDebug()<<Q_FUNC_INFO<<": About to check pBtnSubRx";
    ui->pBtnSubRx->setChecked(true);
}

//Called when subRx is unchecked in main menu via actionSubRx()
void vfo::uncheckSubRx()
{
    ui->pBtnvfoB->setText("VFO B");
    ui->pBtnvfoA->setText("VFO A");
    ui->pBtnvfoA->setEnabled(true);
    ui->pBtnSubRx->setChecked(false);
    vfoEnabled(true, false);
    on_pBtnvfoA_clicked(); //Return to vfoA = default vfo
}

// Called by UI::frequencyMoved() in response to spectra freq move action.
void vfo::setSubRxFrequency(long long f)
{
    subRxFrequency = f;
    if(ui->pBtnSubRx->isChecked()) {
        writeB(f);
    }
}

QString vfo::rigctlGetvfo()
{
    QString vfo;
    vfo = selectedVFO;
    return "VFO" + vfo;
}

void vfo::keyPressEvent( QKeyEvent * event){
    //qDebug() << event->key();
    if(event->key() == Qt::Key_Up) {
       vfohotkey("StepUp");
       event->accept();
    }
    else if(event->key() == Qt::Key_Down) {
       vfohotkey("StepDown");
       event->accept();
    }
    else if(event->key() == Qt::Key_Left) {
       vfohotkey("FreqUp");
       event->accept();
    }
    else if(event->key() == Qt::Key_Right) {
       vfohotkey("FreqDown");
       event->accept();
    }
    else {
       event->ignore();
    }

}
// Powermate stuff begin

void vfo::decrease(int n) {
	emit frequencyMoved(vfohotstep, n);
	qDebug()<<Q_FUNC_INFO<<": Powermate rotated";
}

void vfo::increase(int n) {
        decrease(-n);
}
// Powermate press emulates arrow up key 
void vfo::press() {
	vfohotkey("StepUp");
        qDebug()<<Q_FUNC_INFO<<": Powermate pressed";

}

void vfo::release() {

	qDebug()<<Q_FUNC_INFO<<": Powermate released";
}
// Powermate stuff end

void vfo::vfohotkey(QString cmd){
    if (cmd.compare("FreqDown") == 0){
        emit frequencyMoved(vfohotstep, -1);
        //qDebug() <<"cmd=" <<cmd;
        return;
}
    if (cmd.compare("FreqUp") == 0){
        emit frequencyMoved(vfohotstep, 1);
        //qDebug() <<"cmd=" <<cmd <<"vfohotstep" <<vfohotstep;
        return;
}
    static const int mult[7] = {1,10,100,1000,10000,100000,1000000};
    if (cmd.compare("StepUp") == 0  && curstep <6){
        //qDebug() <<"Step Up old =" <<vfohotstep << " curstep" << curstep;
        curstep++;
        vfohotstep = mult[curstep];
        //qDebug() <<"new =" <<vfohotstep;
        setStepMark();
        return;
}
    if (cmd.compare("StepUp") == 0  && curstep == 6){
        //qDebug() <<"Step Up Wrap old =" <<vfohotstep << " curstep" << curstep;
        curstep = 0;
        vfohotstep = mult[curstep];
        //qDebug() <<"new =" <<vfohotstep;
        setStepMark();
        return;
}


    if (cmd.compare("StepDown") == 0  && curstep >0){
        //qDebug() <<"old =" <<vfohotstep;
        curstep--;
        vfohotstep = mult[curstep];
        //qDebug() <<"new =" <<vfohotstep;
        setStepMark();
        return;
    }
    return;

}

void vfo::setStepMark()
{
    ui->m0->hide();
    ui->m1->hide();
    ui->m2->hide();
    ui->m3->hide();
    ui->m4->hide();
    ui->m5->hide();
    ui->m6->hide();
    //qDebug() <<"setStepMark new =" <<curstep;
    switch (curstep){
      case 0:
        ui->m0->show();
        break;
      case 1:
        ui->m1->show();
        break;
      case 2:
        ui->m2->show();
        break;
      case 3:
        ui->m3->show();
        break;
      case 4:
        ui->m4->show();
        break;
      case 5:
        ui->m5->show();
        break;
      case 6:
        ui->m6->show();
        break;
    }
    return;
}

void vfo::refocus()
{
   ui->frame_2->setFocus();
}

long long vfo::getTxFrequency()
{
    if(selectedVFO == 'A') {
        return readA();
    } else {
        return readB();
    }
}

void vfo::setVfoBtnColour()
{
    switch(selectedVFO) {
        case 'A':
        if(ptt) {
            ui->pBtnvfoA->setStyleSheet("background-color: rgb(255, 0, 0)"); //Red
            ui->pBtnvfoB->setStyleSheet("background-color: normal");
            ui->pBtnSplit->setStyleSheet("background-color: normal");
        } else {
            ui->pBtnvfoA->setStyleSheet("background-color: rgb(85, 255, 0)"); //Green
            ui->pBtnvfoB->setStyleSheet("background-color: normal");
            ui->pBtnSplit->setStyleSheet("background-color: normal");
        }
            break;
        case 'B':
        if(ptt) {
            ui->pBtnvfoB->setStyleSheet("background-color: rgb(255, 0, 0)"); //Red
            ui->pBtnvfoA->setStyleSheet("background-color: normal");
            ui->pBtnSplit->setStyleSheet("background-color: normal");
        } else {
            ui->pBtnvfoB->setStyleSheet("background-color: rgb(85, 255, 0)"); //Green
            ui->pBtnvfoA->setStyleSheet("background-color: normal");
            ui->pBtnSplit->setStyleSheet("background-color: normal");
        }
            break;
        case 'S':
        if(ptt) {
            ui->pBtnvfoA->setStyleSheet("background-color: normal"); //Gray
            ui->pBtnvfoB->setStyleSheet("background-color: rgb(255, 0, 0)"); //Red
            ui->pBtnSplit->setStyleSheet("background-color: rgb(0, 170, 255)"); //Blue
        } else {
            ui->pBtnvfoA->setStyleSheet("background-color: rgb(85, 255, 0)"); //Green
            ui->pBtnvfoB->setStyleSheet("background-color: rgb(255, 155, 155)"); //Light Red
            ui->pBtnSplit->setStyleSheet("background-color: rgb(0, 170, 255)"); //Blue
        }
        break;
    }

}

void vfo::pttChange(bool pttState)
{
    long long temp;

    ptt = pttState;
    if(ptt) { // Going from Rx to Tx
        if(selectedVFO=='S') { //We are going to transmit on vfoB frequency
            temp = readA();                 //Because we are switched to vfoA we will place the
            emit frequencyChanged(readB()); //value of vfoB in it when doing the frequency change
            writeA(temp);                   //hence needing to save and restore vfoA via temp.
        }
    } else { // Going from Tx to Rx
        if(selectedVFO=='S') { //We are going to receive on vfoA frequency
            emit frequencyChanged(readA());
        }
    }
    setVfoBtnColour();
}

bool vfo::getPtt()
{
    return ptt; //returns state of ptt. false = Rx, true =Tx
}

