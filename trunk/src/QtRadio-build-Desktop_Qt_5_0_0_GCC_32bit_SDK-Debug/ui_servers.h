/********************************************************************************
** Form generated from reading UI file 'servers.ui'
**
** Created: Sun Jan 13 22:02:27 2013
**      by: Qt User Interface Compiler version 5.0.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SERVERS_H
#define UI_SERVERS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_Servers
{
public:
    QVBoxLayout *verticalLayout;
    QTreeWidget *treelist;
    QHBoxLayout *horizontalLayout;
    QPushButton *QuickConnect;
    QPushButton *QuickDisconnect;
    QPushButton *refreshButton;
    QPushButton *closebutton;
    QSpacerItem *horizontalSpacer;

    void setupUi(QDialog *Servers)
    {
        if (Servers->objectName().isEmpty())
            Servers->setObjectName(QStringLiteral("Servers"));
        Servers->resize(1197, 389);
        verticalLayout = new QVBoxLayout(Servers);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        treelist = new QTreeWidget(Servers);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(7, QStringLiteral("8"));
        __qtreewidgetitem->setText(6, QStringLiteral("7"));
        __qtreewidgetitem->setText(5, QStringLiteral("6"));
        __qtreewidgetitem->setText(4, QStringLiteral("5"));
        __qtreewidgetitem->setText(3, QStringLiteral("4"));
        __qtreewidgetitem->setText(2, QStringLiteral("3"));
        __qtreewidgetitem->setText(1, QStringLiteral("2"));
        __qtreewidgetitem->setText(0, QStringLiteral("1"));
        treelist->setHeaderItem(__qtreewidgetitem);
        treelist->setObjectName(QStringLiteral("treelist"));
        treelist->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        treelist->setAutoScroll(true);
        treelist->setAutoScrollMargin(16);
        treelist->setColumnCount(8);
        treelist->header()->setVisible(true);
        treelist->header()->setDefaultSectionSize(150);
        treelist->header()->setStretchLastSection(false);

        verticalLayout->addWidget(treelist);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        QuickConnect = new QPushButton(Servers);
        QuickConnect->setObjectName(QStringLiteral("QuickConnect"));

        horizontalLayout->addWidget(QuickConnect);

        QuickDisconnect = new QPushButton(Servers);
        QuickDisconnect->setObjectName(QStringLiteral("QuickDisconnect"));

        horizontalLayout->addWidget(QuickDisconnect);

        refreshButton = new QPushButton(Servers);
        refreshButton->setObjectName(QStringLiteral("refreshButton"));

        horizontalLayout->addWidget(refreshButton);

        closebutton = new QPushButton(Servers);
        closebutton->setObjectName(QStringLiteral("closebutton"));

        horizontalLayout->addWidget(closebutton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(Servers);

        QMetaObject::connectSlotsByName(Servers);
    } // setupUi

    void retranslateUi(QDialog *Servers)
    {
        Servers->setWindowTitle(QApplication::translate("Servers", "Active dspserver list", 0));
        QuickConnect->setText(QApplication::translate("Servers", "Connect", 0));
        QuickDisconnect->setText(QApplication::translate("Servers", "Disconnect", 0));
        refreshButton->setText(QApplication::translate("Servers", "Refresh", 0));
        closebutton->setText(QApplication::translate("Servers", "Close", 0));
    } // retranslateUi

};

namespace Ui {
    class Servers: public Ui_Servers {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SERVERS_H
