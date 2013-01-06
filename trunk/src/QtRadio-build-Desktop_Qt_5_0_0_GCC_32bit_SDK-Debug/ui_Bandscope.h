/********************************************************************************
** Form generated from reading UI file 'Bandscope.ui'
**
** Created: Thu Jan 3 18:54:30 2013
**      by: Qt User Interface Compiler version 5.0.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BANDSCOPE_H
#define UI_BANDSCOPE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>

QT_BEGIN_NAMESPACE

class Ui_Bandscope
{
public:

    void setupUi(QFrame *Bandscope)
    {
        if (Bandscope->objectName().isEmpty())
            Bandscope->setObjectName(QStringLiteral("Bandscope"));
        Bandscope->resize(600, 92);
        Bandscope->setFrameShape(QFrame::StyledPanel);
        Bandscope->setFrameShadow(QFrame::Raised);

        retranslateUi(Bandscope);

        QMetaObject::connectSlotsByName(Bandscope);
    } // setupUi

    void retranslateUi(QFrame *Bandscope)
    {
        Bandscope->setWindowTitle(QApplication::translate("Bandscope", "Frame", 0));
    } // retranslateUi

};

namespace Ui {
    class Bandscope: public Ui_Bandscope {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BANDSCOPE_H
