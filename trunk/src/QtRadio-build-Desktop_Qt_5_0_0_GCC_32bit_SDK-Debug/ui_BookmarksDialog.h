/********************************************************************************
** Form generated from reading UI file 'BookmarksDialog.ui'
**
** Created: Sun Jan 13 22:02:27 2013
**      by: Qt User Interface Compiler version 5.0.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BOOKMARKSDIALOG_H
#define UI_BOOKMARKSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>

QT_BEGIN_NAMESPACE

class Ui_BookmarksDialog
{
public:
    QDialogButtonBox *buttonBox;
    QListWidget *bookmarksListWidget;

    void setupUi(QDialog *BookmarksDialog)
    {
        if (BookmarksDialog->objectName().isEmpty())
            BookmarksDialog->setObjectName(QStringLiteral("BookmarksDialog"));
        BookmarksDialog->resize(508, 396);
        buttonBox = new QDialogButtonBox(BookmarksDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setGeometry(QRect(140, 350, 341, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        bookmarksListWidget = new QListWidget(BookmarksDialog);
        bookmarksListWidget->setObjectName(QStringLiteral("bookmarksListWidget"));
        bookmarksListWidget->setGeometry(QRect(10, 10, 191, 331));

        retranslateUi(BookmarksDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), BookmarksDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), BookmarksDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(BookmarksDialog);
    } // setupUi

    void retranslateUi(QDialog *BookmarksDialog)
    {
        BookmarksDialog->setWindowTitle(QApplication::translate("BookmarksDialog", "Dialog", 0));
    } // retranslateUi

};

namespace Ui {
    class BookmarksDialog: public Ui_BookmarksDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BOOKMARKSDIALOG_H
