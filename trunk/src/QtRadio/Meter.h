#ifndef METER_H
#define METER_H

#include <QDebug>
#include <QImage>
#include <QPainter>

#define SIGMETER 0
#define SWRMETER 1
#define POWMETER 2

class Meter {
public:
    Meter(QString title, short mtype);
    QImage getImage(int dbm);

private:
    QImage* image;
    int x,y;
    int type;
    int dxmin,dymin,dxmax,dymax;
    QString strDbm;

    void calculateLine(int dbm,double minRadius,double maxRadius);
    void calculateNeedle(int dbm,double minRadius,double maxRadius);
};

#endif // METER_H
