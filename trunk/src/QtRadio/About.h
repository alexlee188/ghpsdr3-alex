#ifndef ABOUT_H
#define ABOUT_H

#include <QtCore>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QDialog>
#else
#include <QDialog>
#endif

#define GITREV "Revision: 0.7 2013  By Rick Schnicker KD0OSS!https://github.com/alexlee188/ghpsdr3-alex!Branch: kd0oss"

namespace Ui {
    class About;
}

class About : public QDialog
{
    Q_OBJECT

public:
    explicit About(QWidget *parent = 0);
    ~About();

private:
    Ui::About *ui;
};

#endif // ABOUT_H
