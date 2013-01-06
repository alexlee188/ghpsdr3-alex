/********************************************************************************
** Form generated from reading UI file 'KeypadDialog.ui'
**
** Created: Thu Jan 3 18:54:30 2013
**      by: Qt User Interface Compiler version 5.0.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_KEYPADDIALOG_H
#define UI_KEYPADDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_KeypadDialog
{
public:
    QDialogButtonBox *buttonBox;
    QLabel *frequency;
    QPushButton *pushButton_1;
    QPushButton *pushButton_2;
    QPushButton *pushButton_3;
    QPushButton *pushButton_4;
    QPushButton *pushButton_5;
    QPushButton *pushButton_6;
    QPushButton *pushButton_7;
    QPushButton *pushButton_8;
    QPushButton *pushButton_9;
    QPushButton *pushButton_0;
    QPushButton *pushButton_period;

    void setupUi(QDialog *KeypadDialog)
    {
        if (KeypadDialog->objectName().isEmpty())
            KeypadDialog->setObjectName(QStringLiteral("KeypadDialog"));
        KeypadDialog->resize(279, 301);
        buttonBox = new QDialogButtonBox(KeypadDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setGeometry(QRect(10, 250, 251, 32));
        buttonBox->setFocusPolicy(Qt::TabFocus);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok|QDialogButtonBox::Reset);
        frequency = new QLabel(KeypadDialog);
        frequency->setObjectName(QStringLiteral("frequency"));
        frequency->setGeometry(QRect(10, 10, 261, 61));
        QFont font;
        font.setPointSize(32);
        font.setBold(true);
        font.setWeight(75);
        frequency->setFont(font);
        frequency->setLayoutDirection(Qt::RightToLeft);
        frequency->setFrameShape(QFrame::Box);
        frequency->setFrameShadow(QFrame::Raised);
        pushButton_1 = new QPushButton(KeypadDialog);
        pushButton_1->setObjectName(QStringLiteral("pushButton_1"));
        pushButton_1->setGeometry(QRect(50, 80, 51, 41));
        pushButton_2 = new QPushButton(KeypadDialog);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(110, 80, 51, 41));
        pushButton_3 = new QPushButton(KeypadDialog);
        pushButton_3->setObjectName(QStringLiteral("pushButton_3"));
        pushButton_3->setGeometry(QRect(170, 80, 51, 41));
        pushButton_4 = new QPushButton(KeypadDialog);
        pushButton_4->setObjectName(QStringLiteral("pushButton_4"));
        pushButton_4->setGeometry(QRect(50, 120, 51, 41));
        pushButton_5 = new QPushButton(KeypadDialog);
        pushButton_5->setObjectName(QStringLiteral("pushButton_5"));
        pushButton_5->setGeometry(QRect(110, 120, 51, 41));
        pushButton_6 = new QPushButton(KeypadDialog);
        pushButton_6->setObjectName(QStringLiteral("pushButton_6"));
        pushButton_6->setGeometry(QRect(170, 120, 51, 41));
        pushButton_7 = new QPushButton(KeypadDialog);
        pushButton_7->setObjectName(QStringLiteral("pushButton_7"));
        pushButton_7->setGeometry(QRect(50, 160, 51, 41));
        pushButton_8 = new QPushButton(KeypadDialog);
        pushButton_8->setObjectName(QStringLiteral("pushButton_8"));
        pushButton_8->setGeometry(QRect(110, 160, 51, 41));
        pushButton_9 = new QPushButton(KeypadDialog);
        pushButton_9->setObjectName(QStringLiteral("pushButton_9"));
        pushButton_9->setGeometry(QRect(170, 160, 51, 41));
        pushButton_0 = new QPushButton(KeypadDialog);
        pushButton_0->setObjectName(QStringLiteral("pushButton_0"));
        pushButton_0->setGeometry(QRect(110, 200, 51, 41));
        pushButton_period = new QPushButton(KeypadDialog);
        pushButton_period->setObjectName(QStringLiteral("pushButton_period"));
        pushButton_period->setGeometry(QRect(170, 200, 51, 41));

        retranslateUi(KeypadDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), KeypadDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), KeypadDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(KeypadDialog);
    } // setupUi

    void retranslateUi(QDialog *KeypadDialog)
    {
        KeypadDialog->setWindowTitle(QApplication::translate("KeypadDialog", "Dialog", 0));
        frequency->setText(QString());
        pushButton_1->setText(QApplication::translate("KeypadDialog", "1", 0));
        pushButton_2->setText(QApplication::translate("KeypadDialog", "2", 0));
        pushButton_3->setText(QApplication::translate("KeypadDialog", "3", 0));
        pushButton_4->setText(QApplication::translate("KeypadDialog", "4", 0));
        pushButton_5->setText(QApplication::translate("KeypadDialog", "5", 0));
        pushButton_6->setText(QApplication::translate("KeypadDialog", "6", 0));
        pushButton_7->setText(QApplication::translate("KeypadDialog", "7", 0));
        pushButton_8->setText(QApplication::translate("KeypadDialog", "8", 0));
        pushButton_9->setText(QApplication::translate("KeypadDialog", "9", 0));
        pushButton_0->setText(QApplication::translate("KeypadDialog", "0", 0));
        pushButton_period->setText(QApplication::translate("KeypadDialog", ".", 0));
    } // retranslateUi

};

namespace Ui {
    class KeypadDialog: public Ui_KeypadDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_KEYPADDIALOG_H
