#include <math.h>
#include "Meter.h"
#include "calc.h"

#define CENTER_X 75
#define CENTER_Y 85

Meter::Meter(QString title, short mtype) {
    image=new QImage(150,56,QImage::Format_ARGB32);
    image->fill(0xFFFFFFFF);

    type = mtype;

    QPainter painter(image);

    painter.setFont(QFont("Arial", 8));

    if (type == SIGMETER)
    {
        painter.setPen(QPen(Qt::red, 1));
        painter.drawArc(5, 15, 140, 140, -260*16, -60*16);

        painter.setPen(QPen(Qt::black, 1));
        painter.drawArc(5, 15, 140, 140, -212*16, -48*16);

        // put in the markers
        painter.setPen(Qt::black);
        calculateLine(-121, 60, 75); // 1
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.drawText(dxmax-5, dymax,"1");
        calculateLine(-115, 65, 70); // 2
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        calculateLine(-109, 60, 75); // 3
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.drawText(dxmax-5, dymax,"3");
        calculateLine(-103, 65, 70); // 4
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        calculateLine(-97, 65, 70); // 5
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        calculateLine(-91, 60, 75); // 6
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.drawText(dxmax-5, dymax,"6");
        calculateLine(-85, 65, 70); // 7
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        calculateLine(-79, 65, 70); // 8
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        calculateLine(-73, 60, 75); // 9
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.drawText(dxmax-5, dymax,"9");

        painter.setPen(Qt::red);
        calculateLine(-53, 60, 75); // +20
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.drawText(dxmax-5, dymax,"+20");
        calculateLine(-33, 60, 75); // +40
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.drawText(dxmax-5, dymax,"+40");
        calculateLine(-13, 60, 75); // +60
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.drawText(dxmax-5, dymax,"+60");
    }

    if (type == POWMETER)
    {
        Calc calc;

        painter.setPen(QPen(Qt::red, 1));
        painter.drawArc(5, 15, 140, 140, -300*16, -20*16);

        painter.setPen(QPen(Qt::black, 1));
        painter.drawArc(5, 15, 140, 140, -212*16, -88*16);


        // put in the markers
        painter.setPen(Qt::black);
        calculateLine(-121, 60, 75); // 0
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.drawText(dxmax-5, dymax,"0");
        calculateLine(calc.WattsTodBm(.025)*2.5-121, 60, 75); // 0.25
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.drawText(dxmax-5, dymax,".25");
        calculateLine(calc.WattsTodBm(.1)*2.5-121, 60, 75); // 1
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.drawText(dxmax-5, dymax,"1");
        calculateLine(calc.WattsTodBm(.2)*2.5-121, 60, 75); // 2
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.drawText(dxmax-5, dymax,"2");
        calculateLine(calc.WattsTodBm(.3)*2.5-121, 65, 70); // 3
        painter.drawLine(dxmin, dymin, dxmax, dymax);
    //    painter.drawText(dxmax-5, dymax,"3");
        calculateLine(calc.WattsTodBm(.5)*2.5-121, 60, 75); // 5
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.drawText(dxmax-5, dymax,"5");
        calculateLine(calc.WattsTodBm(1.0)*2.5-121, 60, 75); // 10
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.drawText(dxmax-5, dymax,"10");
        calculateLine(calc.WattsTodBm(2.0)*2.5-121, 60, 75); // 20
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.drawText(dxmax-5, dymax,"20");
        calculateLine(calc.WattsTodBm(3.0)*2.5-121, 65, 70); // 30
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        calculateLine(calc.WattsTodBm(4.0)*2.5-121, 65, 70); // 40
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        calculateLine(calc.WattsTodBm(5.0)*2.5-121, 60, 75); // 50
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.drawText(dxmax-5, dymax,"50");
        calculateLine(calc.WattsTodBm(6.0)*2.5-121, 65, 70); // 60
        painter.drawLine(dxmin, dymin, dxmax, dymax);

        calculateLine(calc.WattsTodBm(7.0)*2.5-121, 65, 70); // 70
        painter.drawLine(dxmin, dymin, dxmax, dymax);

        painter.setPen(Qt::red);
        calculateLine(calc.WattsTodBm(8.0)*2.5-121, 60, 75); // 80
   //     painter.drawLine(dxmin, dymin, dxmax, dymax);
   //     painter.drawText(dxmax-5, dymax,"80");
        calculateLine(calc.WattsTodBm(9.0)*2.5-121, 65, 70); // 90
   //     painter.drawLine(dxmin, dymin, dxmax, dymax);
        calculateLine(calc.WattsTodBm(10.0)*2.5-121, 60, 75); // 100
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.drawText(dxmax-5, dymax,"100");
        calculateLine(calc.WattsTodBm(18.0)*2.5-121, 60, 75); // 180
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.drawText(dxmax-5, dymax,"180");
    }

    painter.drawText(0,0,image->width(),image->height(),Qt::AlignBottom||Qt::AlignHCenter,title);
}

void Meter::calculateLine(int dbm, double minRadius, double maxRadius) {
    double degrees = -121-(dbm+121);
    double radians = degrees*(3.14159265358979323846/180.0);
    double sine   = sin(radians);
    double cosine = cos(radians);

    dxmin = CENTER_X + (int)(minRadius * sine);
    dymin = CENTER_Y + (int)(minRadius * cosine);

    dxmax = CENTER_X + (int)(maxRadius * sine);
    dymax = CENTER_Y + (int)(maxRadius * cosine);

}

void Meter::calculateNeedle(int dbm, double minRadius, double maxRadius) {
    Calc calc;
    double degrees;

    if (type == POWMETER)
        degrees = -121-(((calc.WattsTodBm((float)(dbm/10.0)))*2.5-121)+121);
    else
        degrees = -121-(dbm+121);
//    qDebug("dbm: %2.2f", ((calc.WattsTodBm((float)(dbm/10.0)))*2.5-121)+121);
    if (degrees > -121) degrees = -121;
    double radians = degrees*(3.14159265358979323846/180.0);
    double sine   = sin(radians);
    double cosine = cos(radians);

    dxmin = CENTER_X + (int)(minRadius * sine);
    dymin = CENTER_Y + (int)(minRadius * cosine);

    dxmax = CENTER_X + (int)(maxRadius * sine);
    dymax = CENTER_Y + (int)(maxRadius * cosine);

}

QImage Meter::getImage(int dbm) {
    QImage qImage(*image);
    QPainter painter(&qImage);
    Calc calc;

    painter.setPen(Qt::blue);

    if (type == POWMETER)
    {
        calculateNeedle(calc.PAPower((dbm & 0xff00) >> 8),0,75);
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.setPen(Qt::black);
        strDbm.sprintf("%1.1f SWR  %2.1f W", calc.SWR((dbm & 0xff00) >> 8, dbm & 0x00ff), calc.PAPower((dbm & 0xff00) >> 8));
    }
    else
    {
        calculateNeedle(dbm,0,75);
        painter.drawLine(dxmin, dymin, dxmax, dymax);
        painter.setPen(Qt::black);
        strDbm.sprintf("%d dBm",dbm);
    }
    QRectF r1(image->width()-120, image->height()-15, 105, 20);
    painter.drawText(r1,Qt::AlignRight,strDbm);

    return qImage;
}
