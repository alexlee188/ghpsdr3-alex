#ifndef EQUALIZERDIALOG_H
#define EQUALIZERDIALOG_H

#include <QDialog>
#include <QSettings>
#include "Connection.h"

namespace Ui {
class EqualizerDialog;
}

class EqualizerDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit EqualizerDialog(Connection *pConn, QWidget *parent = 0);
    ~EqualizerDialog();

    Connection *connection;
    
public slots:
    void resetRx(void);
    void resetTx(void);
    void rxSliderChanged(void);
    void txSliderChanged(void);
    void set3BandEqualizer(void);
    void set10BandEqualizer(void);
    void saveSettings(void);
    void saveSettings3Band(void);
    void loadSettings3Band(void);
    void saveSettings10Band(void);
    void loadSettings10Band(void);

private:
    Ui::EqualizerDialog *ui;

};

#endif // EQUALIZERDIALOG_H
