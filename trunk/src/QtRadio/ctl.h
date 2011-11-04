#ifndef CTL_H
#define CTL_H

#include <QFrame>

namespace Ui {
    class Ctl;
}

class Ctl : public QFrame
{
    Q_OBJECT

public:
    explicit Ctl(QWidget *parent = 0);
    ~Ctl();
    int getTxPwr();
    void clearMoxBtn();

signals:
    void pttChange(int caller, bool ptt);//0 = MOX, 1 = Tune, 2 = VOX, 3 = Extern H'ware

public slots:
    void update_mic_level(qreal level);

private slots:

    void on_btnMox_clicked(bool checked);

    void on_btnTune_clicked(bool checked);

    void on_pwrSlider_valueChanged(int value);


private:
    Ui::Ctl *ui;
    int moxPwr;
    int TunePwr;
};

#endif // CTL_H
