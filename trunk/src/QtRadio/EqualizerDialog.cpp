#include "EqualizerDialog.h"
#include "ui_EqualizerDialog.h"
#include "UI.h"

EqualizerDialog::EqualizerDialog(Connection *pConn, QWidget *parent) : QDialog(parent), ui(new Ui::EqualizerDialog)
{
    ui->setupUi(this);

    connection = pConn;

    connect(ui->rxEqPreampSlider, SIGNAL(valueChanged(int)), this, SLOT(rxSliderChanged()));
    connect(ui->rxEq32Slider, SIGNAL(valueChanged(int)), this, SLOT(rxSliderChanged()));
    connect(ui->rxEq63Slider, SIGNAL(valueChanged(int)), this, SLOT(rxSliderChanged()));
    connect(ui->rxEq125Slider, SIGNAL(valueChanged(int)), this, SLOT(rxSliderChanged()));
    connect(ui->rxEq250Slider, SIGNAL(valueChanged(int)), this, SLOT(rxSliderChanged()));
    connect(ui->rxEq500Slider, SIGNAL(valueChanged(int)), this, SLOT(rxSliderChanged()));
    connect(ui->rxEq1KSlider, SIGNAL(valueChanged(int)), this, SLOT(rxSliderChanged()));
    connect(ui->rxEq2KSlider, SIGNAL(valueChanged(int)), this, SLOT(rxSliderChanged()));
    connect(ui->rxEq4KSlider, SIGNAL(valueChanged(int)), this, SLOT(rxSliderChanged()));
    connect(ui->rxEq8KSlider, SIGNAL(valueChanged(int)), this, SLOT(rxSliderChanged()));
    connect(ui->rxEq16KSlider, SIGNAL(valueChanged(int)), this, SLOT(rxSliderChanged()));


    connect(ui->txEqPreampSlider, SIGNAL(valueChanged(int)), this, SLOT(txSliderChanged()));
    connect(ui->txEq32Slider, SIGNAL(valueChanged(int)), this, SLOT(txSliderChanged()));
    connect(ui->txEq63Slider, SIGNAL(valueChanged(int)), this, SLOT(txSliderChanged()));
    connect(ui->txEq125Slider, SIGNAL(valueChanged(int)), this, SLOT(txSliderChanged()));
    connect(ui->txEq250Slider, SIGNAL(valueChanged(int)), this, SLOT(txSliderChanged()));
    connect(ui->txEq500Slider, SIGNAL(valueChanged(int)), this, SLOT(txSliderChanged()));
    connect(ui->txEq1KSlider, SIGNAL(valueChanged(int)), this, SLOT(txSliderChanged()));
    connect(ui->txEq2KSlider, SIGNAL(valueChanged(int)), this, SLOT(txSliderChanged()));
    connect(ui->txEq4KSlider, SIGNAL(valueChanged(int)), this, SLOT(txSliderChanged()));
    connect(ui->txEq8KSlider, SIGNAL(valueChanged(int)), this, SLOT(txSliderChanged()));
    connect(ui->txEq16KSlider, SIGNAL(valueChanged(int)), this, SLOT(txSliderChanged()));

    connect(ui->rxEqResetButton, SIGNAL(clicked()), this, SLOT(resetRx()));
    connect(ui->txEqResetButton, SIGNAL(clicked()), this, SLOT(resetTx()));

    connect(ui->eq3BandButton, SIGNAL(clicked()), this, SLOT(set3BandEqualizer()));
    connect(ui->eq10BandButton, SIGNAL(clicked()), this, SLOT(set10BandEqualizer()));
}

EqualizerDialog::~EqualizerDialog()
{
    delete ui;
}

void EqualizerDialog::resetRx(void)
{
    ui->rxEqPreampSlider->setValue(0);
    ui->rxEq32Slider->setValue(0);
    ui->rxEq63Slider->setValue(0);
    ui->rxEq125Slider->setValue(0);
    ui->rxEq250Slider->setValue(0);
    ui->rxEq500Slider->setValue(0);
    ui->rxEq1KSlider->setValue(0);
    ui->rxEq2KSlider->setValue(0);
    ui->rxEq4KSlider->setValue(0);
    ui->rxEq8KSlider->setValue(0);
    ui->rxEq16KSlider->setValue(0);
} // end resetRx

void EqualizerDialog::resetTx(void)
{
    ui->txEqPreampSlider->setValue(0);
    ui->txEq32Slider->setValue(0);
    ui->txEq63Slider->setValue(0);
    ui->txEq125Slider->setValue(0);
    ui->txEq250Slider->setValue(0);
    ui->txEq500Slider->setValue(0);
    ui->txEq1KSlider->setValue(0);
    ui->txEq2KSlider->setValue(0);
    ui->txEq4KSlider->setValue(0);
    ui->txEq8KSlider->setValue(0);
    ui->txEq16KSlider->setValue(0);
} // end resetTx

void EqualizerDialog::set3BandEqualizer(void)
{
    ui->rxEq32Label->setText("Low");
    ui->rxEq63Label->setText("Med");
    ui->rxEq125Label->setText("High");
    ui->rxEq250Label->setEnabled(false);
    ui->rxEq500Label->setEnabled(false);
    ui->rxEq1kLabel->setEnabled(false);
    ui->rxEq2kLabel->setEnabled(false);
    ui->rxEq4kLabel->setEnabled(false);
    ui->rxEq8kLabel->setEnabled(false);
    ui->rxEq16kLabel->setEnabled(false);

    ui->rxEq250Slider->setEnabled(false);
    ui->rxEq500Slider->setEnabled(false);
    ui->rxEq1KSlider->setEnabled(false);
    ui->rxEq2KSlider->setEnabled(false);
    ui->rxEq4KSlider->setEnabled(false);
    ui->rxEq8KSlider->setEnabled(false);
    ui->rxEq16KSlider->setEnabled(false);

    ui->txEq32Label->setText("Low");
    ui->txEq63Label->setText("Med");
    ui->txEq125Label->setText("High");
    ui->txEq250Label->setEnabled(false);
    ui->txEq500Label->setEnabled(false);
    ui->txEq1kLabel->setEnabled(false);
    ui->txEq2kLabel->setEnabled(false);
    ui->txEq4kLabel->setEnabled(false);
    ui->txEq8kLabel->setEnabled(false);
    ui->txEq16kLabel->setEnabled(false);

    ui->txEq250Slider->setEnabled(false);
    ui->txEq500Slider->setEnabled(false);
    ui->txEq1KSlider->setEnabled(false);
    ui->txEq2KSlider->setEnabled(false);
    ui->txEq4KSlider->setEnabled(false);
    ui->txEq8KSlider->setEnabled(false);
    ui->txEq16KSlider->setEnabled(false);
} // end set3BandEqualizer

void EqualizerDialog::set10BandEqualizer(void)
{
    ui->rxEq32Label->setText("32");
    ui->rxEq63Label->setText("63");
    ui->rxEq125Label->setText("125");
    ui->rxEq250Label->setEnabled(true);
    ui->rxEq500Label->setEnabled(true);
    ui->rxEq1kLabel->setEnabled(true);
    ui->rxEq2kLabel->setEnabled(true);
    ui->rxEq4kLabel->setEnabled(true);
    ui->rxEq8kLabel->setEnabled(true);
    ui->rxEq16kLabel->setEnabled(true);

    ui->rxEq250Slider->setEnabled(true);
    ui->rxEq500Slider->setEnabled(true);
    ui->rxEq1KSlider->setEnabled(true);
    ui->rxEq2KSlider->setEnabled(true);
    ui->rxEq4KSlider->setEnabled(true);
    ui->rxEq8KSlider->setEnabled(true);
    ui->rxEq16KSlider->setEnabled(true);

    ui->txEq32Label->setText("32");
    ui->txEq63Label->setText("63");
    ui->txEq125Label->setText("125");
    ui->txEq250Label->setEnabled(true);
    ui->txEq500Label->setEnabled(true);
    ui->txEq1kLabel->setEnabled(true);
    ui->txEq2kLabel->setEnabled(true);
    ui->txEq4kLabel->setEnabled(true);
    ui->txEq8kLabel->setEnabled(true);
    ui->txEq16kLabel->setEnabled(true);

    ui->txEq250Slider->setEnabled(true);
    ui->txEq500Slider->setEnabled(true);
    ui->txEq1KSlider->setEnabled(true);
    ui->txEq2KSlider->setEnabled(true);
    ui->txEq4KSlider->setEnabled(true);
    ui->txEq8KSlider->setEnabled(true);
    ui->txEq16KSlider->setEnabled(true);
} // end set10BandEqualizer

void EqualizerDialog::rxSliderChanged(void)
{
    QString command;
    QString line;
    int values[11];

    values[0] = ui->rxEqPreampSlider->value();
    values[1] = ui->rxEq32Slider->value();
    values[2] = ui->rxEq63Slider->value();
    values[3] = ui->rxEq125Slider->value();
    values[4] = ui->rxEq250Slider->value();
    values[5] = ui->rxEq500Slider->value();
    values[6] = ui->rxEq1KSlider->value();
    values[7] = ui->rxEq2KSlider->value();
    values[8] = ui->rxEq4KSlider->value();
    values[9] = ui->rxEq8KSlider->value();
    values[10] = ui->rxEq16KSlider->value();

    command.clear();
    line.sprintf("setrx10bdgreq %d %d %d %d %d %d %d %d %d %d %d", values[0], values[1], values[2],
            values[3], values[4], values[5], values[6], values[7], values[8], values[9], values[10]);
    QTextStream(&command) << line;
    connection->sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is is "<< command;
} // end rxSliderChanged

void EqualizerDialog::txSliderChanged(void)
{
    QString command;
    QString line;
    int values[11];

    values[0] = ui->txEqPreampSlider->value();
    values[1] = ui->txEq32Slider->value();
    values[2] = ui->txEq63Slider->value();
    values[3] = ui->txEq125Slider->value();
    values[4] = ui->txEq250Slider->value();
    values[5] = ui->txEq500Slider->value();
    values[6] = ui->txEq1KSlider->value();
    values[7] = ui->txEq2KSlider->value();
    values[8] = ui->txEq4KSlider->value();
    values[9] = ui->txEq8KSlider->value();
    values[10] = ui->txEq16KSlider->value();

    command.clear();
    line.sprintf("settx10bdgreq %d %d %d %d %d %d %d %d %d %d %d", values[0], values[1], values[2],
            values[3], values[4], values[5], values[6], values[7], values[8], values[9], values[10]);
    QTextStream(&command) << line;
    connection->sendCommand(command);
    qDebug()<<Q_FUNC_INFO<<":   The command sent is is "<< command;
} // end txSliderChanged

