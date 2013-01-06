/********************************************************************************
** Form generated from reading UI file 'ctl.ui'
**
** Created: Thu Jan 3 18:54:30 2013
**      by: Qt User Interface Compiler version 5.0.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CTL_H
#define UI_CTL_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>

QT_BEGIN_NAMESPACE

class Ui_Ctl
{
public:
    QPushButton *btnMox;
    QPushButton *btnTune;
    QSlider *pwrSlider;
    QLabel *label;
    QProgressBar *MicProgressBar;
    QSlider *pwrSlider_2;
    QPushButton *pushButton;
    QSpinBox *spinBox;
    QCheckBox *checkBox;
    QPushButton *btnMaster;

    void setupUi(QFrame *Ctl)
    {
        if (Ctl->objectName().isEmpty())
            Ctl->setObjectName(QStringLiteral("Ctl"));
        Ctl->resize(243, 115);
        Ctl->setFrameShape(QFrame::StyledPanel);
        Ctl->setFrameShadow(QFrame::Raised);
        btnMox = new QPushButton(Ctl);
        btnMox->setObjectName(QStringLiteral("btnMox"));
        btnMox->setGeometry(QRect(10, 2, 51, 21));
        btnMox->setFocusPolicy(Qt::ClickFocus);
        btnMox->setCheckable(true);
        btnTune = new QPushButton(Ctl);
        btnTune->setObjectName(QStringLiteral("btnTune"));
        btnTune->setGeometry(QRect(10, 24, 51, 21));
        btnTune->setCheckable(true);
        pwrSlider = new QSlider(Ctl);
        pwrSlider->setObjectName(QStringLiteral("pwrSlider"));
        pwrSlider->setGeometry(QRect(70, 18, 160, 19));
        pwrSlider->setFocusPolicy(Qt::NoFocus);
        pwrSlider->setMaximum(100);
        pwrSlider->setSingleStep(5);
        pwrSlider->setValue(50);
        pwrSlider->setOrientation(Qt::Horizontal);
        pwrSlider->setInvertedAppearance(false);
        pwrSlider->setInvertedControls(false);
        pwrSlider->setTickPosition(QSlider::TicksBelow);
        pwrSlider->setTickInterval(10);
        label = new QLabel(Ctl);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(70, 0, 161, 20));
        MicProgressBar = new QProgressBar(Ctl);
        MicProgressBar->setObjectName(QStringLiteral("MicProgressBar"));
        MicProgressBar->setGeometry(QRect(70, 46, 161, 20));
        MicProgressBar->setValue(24);
        pwrSlider_2 = new QSlider(Ctl);
        pwrSlider_2->setObjectName(QStringLiteral("pwrSlider_2"));
        pwrSlider_2->setGeometry(QRect(130, 70, 101, 20));
        pwrSlider_2->setFocusPolicy(Qt::NoFocus);
        pwrSlider_2->setMaximum(100);
        pwrSlider_2->setSingleStep(5);
        pwrSlider_2->setValue(0);
        pwrSlider_2->setOrientation(Qt::Horizontal);
        pwrSlider_2->setInvertedAppearance(false);
        pwrSlider_2->setInvertedControls(false);
        pwrSlider_2->setTickPosition(QSlider::TicksBelow);
        pwrSlider_2->setTickInterval(10);
        pushButton = new QPushButton(Ctl);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(10, 71, 51, 21));
        spinBox = new QSpinBox(Ctl);
        spinBox->setObjectName(QStringLiteral("spinBox"));
        spinBox->setGeometry(QRect(68, 66, 59, 29));
        spinBox->setFocusPolicy(Qt::NoFocus);
        spinBox->setMaximum(100);
        checkBox = new QCheckBox(Ctl);
        checkBox->setObjectName(QStringLiteral("checkBox"));
        checkBox->setGeometry(QRect(10, 90, 71, 25));
        btnMaster = new QPushButton(Ctl);
        btnMaster->setObjectName(QStringLiteral("btnMaster"));
        btnMaster->setGeometry(QRect(10, 47, 51, 21));
        QFont font;
        font.setPointSize(9);
        font.setBold(false);
        font.setItalic(false);
        font.setWeight(50);
        btnMaster->setFont(font);

        retranslateUi(Ctl);

        QMetaObject::connectSlotsByName(Ctl);
    } // setupUi

    void retranslateUi(QFrame *Ctl)
    {
        Ctl->setWindowTitle(QApplication::translate("Ctl", "Frame", 0));
        btnMox->setText(QApplication::translate("Ctl", "MOX", 0));
        btnTune->setText(QApplication::translate("Ctl", "Tune", 0));
        label->setText(QApplication::translate("Ctl", "0      Power out      100", 0));
        pushButton->setText(QApplication::translate("Ctl", "Test", 0));
        checkBox->setText(QApplication::translate("Ctl", "Toggle", 0));
        btnMaster->setText(QApplication::translate("Ctl", "MASTER", 0));
    } // retranslateUi

};

namespace Ui {
    class Ctl: public Ui_Ctl {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CTL_H
