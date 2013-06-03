/*
 * File:   calc.cpp
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

#include "calc.h"

Calc::Calc()
{
}


Calc::~Calc()
{
}


double Calc::SWR(int adc_fwd, int adc_rev)
{
    if(adc_fwd == 0 && adc_rev == 0)
        return 1.0;
    else if(adc_rev > adc_fwd)
        return 50.0;

    double Ef = ScaledVoltage(adc_fwd);
    double Er = ScaledVoltage(adc_rev);

    double swr = (Ef + Er)/(Ef - Er);

    return swr;
}


double Calc::ScaledVoltage(int adc)
{
    double v_det = adc * 0.062963;			// scale factor in V/bit including pot ratio
    double v_out = v_det * 10.39853;		// scale factor in V/V for bridge output to detector voltage
    return v_out; //*PABandOffset(CurrentBand);
}


double Calc::ADCtodBm(int adc_data)
{
    if(adc_data == 0)
        return 0;

    double mult = 100000 / pow(225/1/*PABandOffset(CurrentBand)*/, 2);
    return 10*log10(mult*pow(adc_data, 2));
}


double Calc::PAPower(int adc)
{
    double v_out = ScaledVoltage(adc);
    double powr = pow(v_out, 2)/50;
    powr = qMax(powr, 0.0);
    return powr;
}


double Calc::WattsTodBm(double watts)
{
    return 10*log10(watts/0.001);
}


double Calc::dBmToWatts(double dBm)
{
    return pow(10, dBm/10)*0.001;
}

