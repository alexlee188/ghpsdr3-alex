/********************************************************************************
** Form generated from reading UI file 'BookmarksEditDialog.ui'
**
** Created: Sun Jan 13 22:02:27 2013
**      by: Qt User Interface Compiler version 5.0.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BOOKMARKSEDITDIALOG_H
#define UI_BOOKMARKSEDITDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_BookmarksEditDialog
{
public:
    QDialogButtonBox *buttonBox;
    QListWidget *bookmarksListWidget;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLineEdit *titleLineEdit;
    QLineEdit *bandLineEdit;
    QLineEdit *frequencyLineEdit;
    QLineEdit *modeLineEdit;
    QLineEdit *filterLineEdit;
    QPushButton *deletePushButton;
    QPushButton *updatePushButton;

    void setupUi(QDialog *BookmarksEditDialog)
    {
        if (BookmarksEditDialog->objectName().isEmpty())
            BookmarksEditDialog->setObjectName(QStringLiteral("BookmarksEditDialog"));
        BookmarksEditDialog->resize(532, 430);
        buttonBox = new QDialogButtonBox(BookmarksEditDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setGeometry(QRect(170, 390, 341, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Ok);
        bookmarksListWidget = new QListWidget(BookmarksEditDialog);
        bookmarksListWidget->setObjectName(QStringLiteral("bookmarksListWidget"));
        bookmarksListWidget->setGeometry(QRect(10, 20, 181, 371));
        label = new QLabel(BookmarksEditDialog);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(220, 30, 67, 17));
        label_2 = new QLabel(BookmarksEditDialog);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(220, 70, 67, 17));
        label_3 = new QLabel(BookmarksEditDialog);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(220, 110, 81, 17));
        label_4 = new QLabel(BookmarksEditDialog);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(220, 150, 67, 17));
        label_5 = new QLabel(BookmarksEditDialog);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(220, 190, 67, 17));
        titleLineEdit = new QLineEdit(BookmarksEditDialog);
        titleLineEdit->setObjectName(QStringLiteral("titleLineEdit"));
        titleLineEdit->setGeometry(QRect(300, 20, 171, 27));
        bandLineEdit = new QLineEdit(BookmarksEditDialog);
        bandLineEdit->setObjectName(QStringLiteral("bandLineEdit"));
        bandLineEdit->setGeometry(QRect(300, 60, 113, 27));
        frequencyLineEdit = new QLineEdit(BookmarksEditDialog);
        frequencyLineEdit->setObjectName(QStringLiteral("frequencyLineEdit"));
        frequencyLineEdit->setGeometry(QRect(300, 100, 113, 27));
        modeLineEdit = new QLineEdit(BookmarksEditDialog);
        modeLineEdit->setObjectName(QStringLiteral("modeLineEdit"));
        modeLineEdit->setGeometry(QRect(300, 140, 113, 27));
        filterLineEdit = new QLineEdit(BookmarksEditDialog);
        filterLineEdit->setObjectName(QStringLiteral("filterLineEdit"));
        filterLineEdit->setGeometry(QRect(300, 180, 113, 27));
        deletePushButton = new QPushButton(BookmarksEditDialog);
        deletePushButton->setObjectName(QStringLiteral("deletePushButton"));
        deletePushButton->setGeometry(QRect(420, 250, 98, 27));
        updatePushButton = new QPushButton(BookmarksEditDialog);
        updatePushButton->setObjectName(QStringLiteral("updatePushButton"));
        updatePushButton->setGeometry(QRect(230, 250, 98, 27));

        retranslateUi(BookmarksEditDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), BookmarksEditDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), BookmarksEditDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(BookmarksEditDialog);
    } // setupUi

    void retranslateUi(QDialog *BookmarksEditDialog)
    {
        BookmarksEditDialog->setWindowTitle(QApplication::translate("BookmarksEditDialog", "Edit Bookmarks", 0));
        label->setText(QApplication::translate("BookmarksEditDialog", "Title:", 0));
        label_2->setText(QApplication::translate("BookmarksEditDialog", "Band:", 0));
        label_3->setText(QApplication::translate("BookmarksEditDialog", "Frequency:", 0));
        label_4->setText(QApplication::translate("BookmarksEditDialog", "Mode:", 0));
        label_5->setText(QApplication::translate("BookmarksEditDialog", "Filter:", 0));
        deletePushButton->setText(QApplication::translate("BookmarksEditDialog", "Delete", 0));
        updatePushButton->setText(QApplication::translate("BookmarksEditDialog", "Update", 0));
    } // retranslateUi

};

namespace Ui {
    class BookmarksEditDialog: public Ui_BookmarksEditDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BOOKMARKSEDITDIALOG_H
