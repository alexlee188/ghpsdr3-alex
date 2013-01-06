/********************************************************************************
** Form generated from reading UI file 'Bookmark.ui'
**
** Created: Thu Jan 3 18:54:30 2013
**      by: Qt User Interface Compiler version 5.0.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BOOKMARK_H
#define UI_BOOKMARK_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

QT_BEGIN_NAMESPACE

class Ui_BookmarkDialog
{
public:
    QDialogButtonBox *buttonBox;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;
    QLineEdit *titleLineEdit;
    QLineEdit *frequencyLineEdit;
    QLineEdit *modeLineEdit;
    QLineEdit *filterLineEdit;
    QLabel *label_3;
    QLineEdit *bandLineEdit;

    void setupUi(QDialog *BookmarkDialog)
    {
        if (BookmarkDialog->objectName().isEmpty())
            BookmarkDialog->setObjectName(QStringLiteral("BookmarkDialog"));
        BookmarkDialog->resize(514, 412);
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        BookmarkDialog->setFont(font);
        buttonBox = new QDialogButtonBox(BookmarkDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setGeometry(QRect(150, 370, 341, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        label = new QLabel(BookmarkDialog);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(30, 40, 441, 41));
        QFont font1;
        font1.setPointSize(20);
        label->setFont(font1);
        label_2 = new QLabel(BookmarkDialog);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(50, 240, 91, 17));
        QFont font2;
        font2.setBold(false);
        font2.setWeight(50);
        label_2->setFont(font2);
        label_4 = new QLabel(BookmarkDialog);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(50, 290, 67, 17));
        label_4->setFont(font2);
        label_5 = new QLabel(BookmarkDialog);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(50, 340, 67, 17));
        label_5->setFont(font2);
        label_6 = new QLabel(BookmarkDialog);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setGeometry(QRect(50, 140, 67, 17));
        label_6->setFont(font2);
        titleLineEdit = new QLineEdit(BookmarkDialog);
        titleLineEdit->setObjectName(QStringLiteral("titleLineEdit"));
        titleLineEdit->setGeometry(QRect(130, 130, 201, 31));
        frequencyLineEdit = new QLineEdit(BookmarkDialog);
        frequencyLineEdit->setObjectName(QStringLiteral("frequencyLineEdit"));
        frequencyLineEdit->setGeometry(QRect(130, 230, 141, 31));
        modeLineEdit = new QLineEdit(BookmarkDialog);
        modeLineEdit->setObjectName(QStringLiteral("modeLineEdit"));
        modeLineEdit->setGeometry(QRect(130, 280, 113, 31));
        filterLineEdit = new QLineEdit(BookmarkDialog);
        filterLineEdit->setObjectName(QStringLiteral("filterLineEdit"));
        filterLineEdit->setGeometry(QRect(130, 330, 113, 31));
        label_3 = new QLabel(BookmarkDialog);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(50, 190, 67, 17));
        label_3->setFont(font2);
        bandLineEdit = new QLineEdit(BookmarkDialog);
        bandLineEdit->setObjectName(QStringLiteral("bandLineEdit"));
        bandLineEdit->setGeometry(QRect(130, 180, 113, 27));

        retranslateUi(BookmarkDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), BookmarkDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), BookmarkDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(BookmarkDialog);
    } // setupUi

    void retranslateUi(QDialog *BookmarkDialog)
    {
        BookmarkDialog->setWindowTitle(QApplication::translate("BookmarkDialog", "Bookmark", 0));
        label->setText(QApplication::translate("BookmarkDialog", "Bookmark this frequency", 0));
        label_2->setText(QApplication::translate("BookmarkDialog", "Frequency:", 0));
        label_4->setText(QApplication::translate("BookmarkDialog", "Mode:", 0));
        label_5->setText(QApplication::translate("BookmarkDialog", "Filter:", 0));
        label_6->setText(QApplication::translate("BookmarkDialog", "Title:", 0));
        label_3->setText(QApplication::translate("BookmarkDialog", "Band:", 0));
    } // retranslateUi

};

namespace Ui {
    class BookmarkDialog: public Ui_BookmarkDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BOOKMARK_H
