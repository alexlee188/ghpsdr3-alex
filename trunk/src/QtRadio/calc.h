/*
 * File:   calc.h
 * Author: Rick Schnicker, KD0OSS
 *
 * Created on 14 Apr 2013
 */

/* Copyright (C)
* 2013 Rick Schnicker, KD0OSS
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#ifndef CALC_H
#define CALC_H

#include <QObject>
#include <QDebug>
#include <math.h>

class Calc : public QObject {
    Q_OBJECT
public:
    Calc();
    virtual ~Calc();

    double SWR(int adc_fwd, int adc_rev);
    double ScaledVoltage(int adc);
    double ADCtodBm(int adc_data);
    double PAPower(int adc);
    double WattsTodBm(double watts);
    double dBmToWatts(double dBm);

private:

};
#endif // CALC_H
