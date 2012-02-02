/*
 * File:   Band.cpp
 * Author: John Melton, G0ORX/N6LYT
 * 
 * Created on 13 August 2010, 14:52
 */

/* Copyright (C)
* 2009 - John Melton, G0ORX/N6LYT
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

#include "Band.h"
#include "Mode.h"

Band::Band() {
//    int i;

//    for(i=0;i<BAND_LAST;i++) {
//        stack[i]=0;
//    }
    limits.clear();
}

Band::~Band() {
    // save state
    
}

void Band::loadSettings(QSettings* settings) {
    long long limitMin,limitMax;

    settings->beginGroup("Band");
    currentBand=settings->value("currentBand",BAND_40).toInt();
//    currentStack=settings->value("currentStack",0).toInt();
    // Quick memory number 1
    bandstack[BAND_160][0].setFrequency(settings->value("frequency.0.0",1810000).toLongLong());
    bandstack[BAND_160][0].setMode(settings->value("mode.0.0",MODE_CWL).toInt());
    bandstack[BAND_160][0].setFilter(settings->value("filter.0.0",4).toInt());
    bandstack[BAND_160][0].setSpectrumHigh(settings->value("spectrumHigh.0.0",-40).toInt());
    bandstack[BAND_160][0].setSpectrumLow(settings->value("spectrumLow.0.0",-160).toInt());
    bandstack[BAND_160][0].setWaterfallHigh(settings->value("waterfallHigh.0.0",-60).toInt());
    bandstack[BAND_160][0].setWaterfallLow(settings->value("waterfallLow.0.0",-125).toInt());
    bandstack[BAND_160][0].setInfo(settings->value("info.0.0",2).toInt());  //Count of Quick Memories 0 .. n
    // Quick memory number 2
    bandstack[BAND_160][1].setFrequency(settings->value("frequency.0.1",1835000).toLongLong());
    bandstack[BAND_160][1].setMode(settings->value("mode.0.1",MODE_LSB).toInt());
    bandstack[BAND_160][1].setFilter(settings->value("filter.0.1",4).toInt());
    bandstack[BAND_160][1].setSpectrumHigh(settings->value("spectrumHigh.0.1",-40).toInt());
    bandstack[BAND_160][1].setSpectrumLow(settings->value("spectrumLow.0.1",-160).toInt());
    bandstack[BAND_160][1].setWaterfallHigh(settings->value("waterfallHigh.0.1",-60).toInt());
    bandstack[BAND_160][1].setWaterfallLow(settings->value("waterfallLow.0.1",-125).toInt());
    bandstack[BAND_160][1].setInfo(settings->value("info.0.1",0).toInt());  //Pointer to stack for reading
    // Quick memory number 3
    bandstack[BAND_160][2].setFrequency(settings->value("frequency.0.2",1845000).toLongLong());
    bandstack[BAND_160][2].setMode(settings->value("mode.0.2",MODE_AM).toInt());
    bandstack[BAND_160][2].setFilter(settings->value("filter.0.2",4).toInt());
    bandstack[BAND_160][2].setSpectrumHigh(settings->value("spectrumHigh.0.2",-40).toInt());
    bandstack[BAND_160][2].setSpectrumLow(settings->value("spectrumLow.0.2",-160).toInt());
    bandstack[BAND_160][2].setWaterfallHigh(settings->value("waterfallHigh.0.2",-60).toInt());
    bandstack[BAND_160][2].setWaterfallLow(settings->value("waterfallLow.0.2",-125).toInt());
    bandstack[BAND_160][2].setInfo(settings->value("info.0.2",0).toInt());  //Pointer to stack for storing
    // Current working frequency
    bandstack[BAND_160][3].setFrequency(settings->value("frequency.0.3",1845000).toLongLong());
    bandstack[BAND_160][3].setMode(settings->value("mode.0.3",MODE_AM).toInt());
    bandstack[BAND_160][3].setFilter(settings->value("filter.0.3",4).toInt());
    bandstack[BAND_160][3].setSpectrumHigh(settings->value("spectrumHigh.0.3",-40).toInt());
    bandstack[BAND_160][3].setSpectrumLow(settings->value("spectrumLow.0.3",-160).toInt());
    bandstack[BAND_160][3].setWaterfallHigh(settings->value("waterfallHigh.0.3",-60).toInt());
    bandstack[BAND_160][3].setWaterfallLow(settings->value("waterfallLow.0.3",-125).toInt());
    bandstack[BAND_160][3].setInfo(settings->value("info.0.3",0).toInt());  //Null info. Available for whatever.

    // Quick memory number 1
    bandstack[BAND_80][0].setFrequency(settings->value("frequency.1.0",3501000).toLongLong());
    bandstack[BAND_80][0].setMode(settings->value("mode.1.0",MODE_CWL).toInt());
    bandstack[BAND_80][0].setFilter(settings->value("filter.1.0",4).toInt());
    bandstack[BAND_80][0].setSpectrumHigh(settings->value("spectrumHigh.1.0",-40).toInt());
    bandstack[BAND_80][0].setSpectrumLow(settings->value("spectrumLow.1.0",-160).toInt());
    bandstack[BAND_80][0].setWaterfallHigh(settings->value("waterfallHigh.1.0",-60).toInt());
    bandstack[BAND_80][0].setWaterfallLow(settings->value("waterfallLow.1.0",-125).toInt());
    bandstack[BAND_80][0].setInfo(settings->value("info.1.0",2).toInt());  //Count of Quick Memories 0 .. n
    // Quick memory number 2
    bandstack[BAND_80][1].setFrequency(settings->value("frequency.1.1",3751000).toLongLong());
    bandstack[BAND_80][1].setMode(settings->value("mode.1.1",MODE_LSB).toInt());
    bandstack[BAND_80][1].setFilter(settings->value("filter.1.1",5).toInt());
    bandstack[BAND_80][1].setSpectrumHigh(settings->value("spectrumHigh.1.1",-40).toInt());
    bandstack[BAND_80][1].setSpectrumLow(settings->value("spectrumLow.1.1",-160).toInt());
    bandstack[BAND_80][1].setWaterfallHigh(settings->value("waterfallHigh.1.1",-60).toInt());
    bandstack[BAND_80][1].setWaterfallLow(settings->value("waterfallLow.1.1",-125).toInt());
    bandstack[BAND_80][1].setInfo(settings->value("info.1.1",0).toInt());  //Pointer to stack for reading
    // Quick memory number 3
    bandstack[BAND_80][2].setFrequency(settings->value("frequency.1.2",3850000).toLongLong());
    bandstack[BAND_80][2].setMode(settings->value("mode.1.2",MODE_LSB).toInt());
    bandstack[BAND_80][2].setFilter(settings->value("filter.1.2",5).toInt());
    bandstack[BAND_80][2].setSpectrumHigh(settings->value("spectrumHigh.1.2",-40).toInt());
    bandstack[BAND_80][2].setSpectrumLow(settings->value("spectrumLow.1.2",-160).toInt());
    bandstack[BAND_80][2].setWaterfallHigh(settings->value("waterfallHigh.1.2",-60).toInt());
    bandstack[BAND_80][2].setWaterfallLow(settings->value("waterfallLow.1.2",-125).toInt());
    bandstack[BAND_80][2].setInfo(settings->value("info.1.2",0).toInt());  //Pointer to stack for storing
    // Current working frequency
    bandstack[BAND_80][3].setFrequency(settings->value("frequency.1.3",3850000).toLongLong());
    bandstack[BAND_80][3].setMode(settings->value("mode.1.3",MODE_LSB).toInt());
    bandstack[BAND_80][3].setFilter(settings->value("filter.1.3",5).toInt());
    bandstack[BAND_80][3].setSpectrumHigh(settings->value("spectrumHigh.1.3",-40).toInt());
    bandstack[BAND_80][3].setSpectrumLow(settings->value("spectrumLow.1.3",-160).toInt());
    bandstack[BAND_80][3].setWaterfallHigh(settings->value("waterfallHigh.1.3",-60).toInt());
    bandstack[BAND_80][3].setWaterfallLow(settings->value("waterfallLow.1.3",-125).toInt());
    bandstack[BAND_80][3].setInfo(settings->value("info.1.3",0).toInt());  //Null info. Available for whatever.

    // Quick memory number 1
    bandstack[BAND_60][0].setFrequency(settings->value("frequency.2.0",5330500).toLongLong());
    bandstack[BAND_60][0].setMode(settings->value("mode.2.0",MODE_CWL).toInt());
    bandstack[BAND_60][0].setFilter(settings->value("filter.2.0",3).toInt());
    bandstack[BAND_60][0].setSpectrumHigh(settings->value("spectrumHigh.2.0",-40).toInt());
    bandstack[BAND_60][0].setSpectrumLow(settings->value("spectrumLow.2.0",-160).toInt());
    bandstack[BAND_60][0].setWaterfallHigh(settings->value("waterfallHigh.2.0",-60).toInt());
    bandstack[BAND_60][0].setWaterfallLow(settings->value("waterfallLow.2.0",-125).toInt());
    bandstack[BAND_60][0].setInfo(settings->value("info.2.0",3).toInt());  //Count of Quick Memories 0 .. n
    // Quick memory number 2
    bandstack[BAND_60][1].setFrequency(settings->value("frequency.2.1",5346500).toLongLong());
    bandstack[BAND_60][1].setMode(settings->value("mode.2.1",MODE_LSB).toInt());
    bandstack[BAND_60][1].setFilter(settings->value("filter.2.1",3).toInt());
    bandstack[BAND_60][1].setSpectrumHigh(settings->value("spectrumHigh.2.1",-40).toInt());
    bandstack[BAND_60][1].setSpectrumLow(settings->value("spectrumLow.2.1",-160).toInt());
    bandstack[BAND_60][1].setWaterfallHigh(settings->value("waterfallHigh.2.1",-60).toInt());
    bandstack[BAND_60][1].setWaterfallLow(settings->value("waterfallLow.2.1",-125).toInt());
    bandstack[BAND_60][1].setInfo(settings->value("info.2.1",0).toInt());  //Pointer to stack for reading
    // Quick memory number 3
    bandstack[BAND_60][2].setFrequency(settings->value("frequency.2.2",5366500).toLongLong());
    bandstack[BAND_60][2].setMode(settings->value("mode.2.2",MODE_LSB).toInt());
    bandstack[BAND_60][2].setFilter(settings->value("filter.2.2",3).toInt());
    bandstack[BAND_60][2].setSpectrumHigh(settings->value("spectrumHigh.2.2",-40).toInt());
    bandstack[BAND_60][2].setSpectrumLow(settings->value("spectrumLow.2.2",-160).toInt());
    bandstack[BAND_60][2].setWaterfallHigh(settings->value("waterfallHigh.2.2",-60).toInt());
    bandstack[BAND_60][2].setWaterfallLow(settings->value("waterfallLow.2.2",-125).toInt());
    bandstack[BAND_60][2].setInfo(settings->value("info.2.2",0).toInt());  //Pointer to stack for storing
    // Quick memory number 4
    bandstack[BAND_60][3].setFrequency(settings->value("frequency.2.3",5371500).toLongLong());
    bandstack[BAND_60][3].setMode(settings->value("mode.2.3",MODE_LSB).toInt());
    bandstack[BAND_60][3].setFilter(settings->value("filter.2.3",3).toInt());
    bandstack[BAND_60][3].setSpectrumHigh(settings->value("spectrumHigh.2.3",-40).toInt());
    bandstack[BAND_60][3].setSpectrumLow(settings->value("spectrumLow.2.3",-160).toInt());
    bandstack[BAND_60][3].setWaterfallHigh(settings->value("waterfallHigh.2.3",-60).toInt());
    bandstack[BAND_60][3].setWaterfallLow(settings->value("waterfallLow.2.3",-125).toInt());
    bandstack[BAND_60][3].setInfo(settings->value("info.2.3",0).toInt());  //Reserved for total entries
    // Quick memory number 5
    bandstack[BAND_60][4].setFrequency(settings->value("frequency.2.4",5403500).toLongLong());
    bandstack[BAND_60][4].setMode(settings->value("mode.2.4",MODE_LSB).toInt());
    bandstack[BAND_60][4].setFilter(settings->value("filter.2.4",3).toInt());
    bandstack[BAND_60][4].setSpectrumHigh(settings->value("spectrumHigh.2.4",-40).toInt());
    bandstack[BAND_60][4].setSpectrumLow(settings->value("spectrumLow.2.4",-160).toInt());
    bandstack[BAND_60][4].setWaterfallHigh(settings->value("waterfallHigh.2.4",-60).toInt());
    bandstack[BAND_60][4].setWaterfallLow(settings->value("waterfallLow.2.4",-125).toInt());
    bandstack[BAND_60][4].setInfo(settings->value("info.2.4",0).toInt());  //Not used. Band terminated by being 5 entries

    // Quick memory number 1
    bandstack[BAND_40][0].setFrequency(settings->value("frequency.3.0",7001000).toLongLong());
    bandstack[BAND_40][0].setMode(settings->value("mode.3.0",MODE_CWL).toInt());
    bandstack[BAND_40][0].setFilter(settings->value("filter.3.0",4).toInt());
    bandstack[BAND_40][0].setSpectrumHigh(settings->value("spectrumHigh.3.0",-40).toInt());
    bandstack[BAND_40][0].setSpectrumLow(settings->value("spectrumLow.3.0",-160).toInt());
    bandstack[BAND_40][0].setWaterfallHigh(settings->value("waterfallHigh.3.0",-60).toInt());
    bandstack[BAND_40][0].setWaterfallLow(settings->value("waterfallLow.3.0",-125).toInt());
    bandstack[BAND_40][0].setInfo(settings->value("info.3.0",2).toInt());  //Count of Quick Memories 0 .. n
    // Quick memory number 2
    bandstack[BAND_40][1].setFrequency(settings->value("frequency.3.1",7060000).toLongLong());
    bandstack[BAND_40][1].setMode(settings->value("mode.3.1",MODE_LSB).toInt());
    bandstack[BAND_40][1].setFilter(settings->value("filter.3.1",3).toInt());
    bandstack[BAND_40][1].setSpectrumHigh(settings->value("spectrumHigh.3.1",-40).toInt());
    bandstack[BAND_40][1].setSpectrumLow(settings->value("spectrumLow.3.1",-160).toInt());
    bandstack[BAND_40][1].setWaterfallHigh(settings->value("waterfallHigh.3.1",-60).toInt());
    bandstack[BAND_40][1].setWaterfallLow(settings->value("waterfallLow.3.1",-125).toInt());
    bandstack[BAND_40][1].setInfo(settings->value("info.3.1",0).toInt());  //Pointer to stack for reading
    // Quick memory number 3
    bandstack[BAND_40][2].setFrequency(settings->value("frequency.3.2",7100000).toLongLong());
    bandstack[BAND_40][2].setMode(settings->value("mode.3.2",MODE_LSB).toInt());
    bandstack[BAND_40][2].setFilter(settings->value("filter.3.2",3).toInt());
    bandstack[BAND_40][2].setSpectrumHigh(settings->value("spectrumHigh.3.2",-40).toInt());
    bandstack[BAND_40][2].setSpectrumLow(settings->value("spectrumLow.3.2",-160).toInt());
    bandstack[BAND_40][2].setWaterfallHigh(settings->value("waterfallHigh.3.2",-60).toInt());
    bandstack[BAND_40][2].setWaterfallLow(settings->value("waterfallLow.3.2",-125).toInt());
    bandstack[BAND_40][2].setInfo(settings->value("info.3.2",0).toInt());  //Pointer to stack for storing
    // Current working frequency
    bandstack[BAND_40][3].setFrequency(settings->value("frequency.3.3",7100000).toLongLong());
    bandstack[BAND_40][3].setMode(settings->value("mode.3.3",MODE_LSB).toInt());
    bandstack[BAND_40][3].setFilter(settings->value("filter.3.3",3).toInt());
    bandstack[BAND_40][3].setSpectrumHigh(settings->value("spectrumHigh.3.3",-40).toInt());
    bandstack[BAND_40][3].setSpectrumLow(settings->value("spectrumLow.3.3",-160).toInt());
    bandstack[BAND_40][3].setWaterfallHigh(settings->value("waterfallHigh.3.3",-60).toInt());
    bandstack[BAND_40][3].setWaterfallLow(settings->value("waterfallLow.3.3",-125).toInt());
    bandstack[BAND_40][3].setInfo(settings->value("info.3.3",0).toInt());  //Null info. Available for whatever.

    // Quick memory number 1
    bandstack[BAND_30][0].setFrequency(settings->value("frequency.4.0",10120000).toLongLong());
    bandstack[BAND_30][0].setMode(settings->value("mode.4.0",MODE_CWU).toInt());
    bandstack[BAND_30][0].setFilter(settings->value("filter.4.0",4).toInt());
    bandstack[BAND_30][0].setSpectrumHigh(settings->value("spectrumHigh.4.0",-40).toInt());
    bandstack[BAND_30][0].setSpectrumLow(settings->value("spectrumLow.4.0",-160).toInt());
    bandstack[BAND_30][0].setWaterfallHigh(settings->value("waterfallHigh.4.0",-60).toInt());
    bandstack[BAND_30][0].setWaterfallLow(settings->value("waterfallLow.4.0",-125).toInt());
    bandstack[BAND_30][0].setInfo(settings->value("info.4.0",2).toInt());  //Count of Quick Memories 0 .. n
    // Quick memory number 2
    bandstack[BAND_30][1].setFrequency(settings->value("frequency.4.1",10130000).toLongLong());
    bandstack[BAND_30][1].setMode(settings->value("mode.4.1",MODE_CWU).toInt());
    bandstack[BAND_30][1].setFilter(settings->value("filter.4.1",4).toInt());
    bandstack[BAND_30][1].setSpectrumHigh(settings->value("spectrumHigh.4.1",-40).toInt());
    bandstack[BAND_30][1].setSpectrumLow(settings->value("spectrumLow.4.1",-160).toInt());
    bandstack[BAND_30][1].setWaterfallHigh(settings->value("waterfallHigh.4.1",-60).toInt());
    bandstack[BAND_30][1].setWaterfallLow(settings->value("waterfallLow.4.1",-125).toInt());
    bandstack[BAND_30][1].setInfo(settings->value("info.4.1",0).toInt());  //Pointer to stack for reading
    // Quick memory number 3
    bandstack[BAND_30][2].setFrequency(settings->value("frequency.4.2",10140000).toLongLong());
    bandstack[BAND_30][2].setMode(settings->value("mode.4.2",MODE_CWU).toInt());
    bandstack[BAND_30][2].setFilter(settings->value("filter.4.2",4).toInt());
    bandstack[BAND_30][2].setSpectrumHigh(settings->value("spectrumHigh.4.2",-40).toInt());
    bandstack[BAND_30][2].setSpectrumLow(settings->value("spectrumLow.4.2",-160).toInt());
    bandstack[BAND_30][2].setWaterfallHigh(settings->value("waterfallHigh.4.2",-60).toInt());
    bandstack[BAND_30][2].setWaterfallLow(settings->value("waterfallLow.4.2",-125).toInt());
    bandstack[BAND_30][2].setInfo(settings->value("info.4.2",0).toInt());  //Pointer to stack for storing
    // Current working frequency
    bandstack[BAND_30][3].setFrequency(settings->value("frequency.4.3",10140000).toLongLong());
    bandstack[BAND_30][3].setMode(settings->value("mode.4.3",MODE_CWU).toInt());
    bandstack[BAND_30][3].setFilter(settings->value("filter.4.3",4).toInt());
    bandstack[BAND_30][3].setSpectrumHigh(settings->value("spectrumHigh.4.3",-40).toInt());
    bandstack[BAND_30][3].setSpectrumLow(settings->value("spectrumLow.4.3",-160).toInt());
    bandstack[BAND_30][3].setWaterfallHigh(settings->value("waterfallHigh.4.3",-60).toInt());
    bandstack[BAND_30][3].setWaterfallLow(settings->value("waterfallLow.4.3",-125).toInt());
    bandstack[BAND_30][3].setInfo(settings->value("info.4.3",0).toInt());  //Null info. Available for whatever.

    // Quick memory number 1
    bandstack[BAND_20][0].setFrequency(settings->value("frequency.5.0",14010000).toLongLong());
    bandstack[BAND_20][0].setMode(settings->value("mode.5.0",MODE_CWU).toInt());
    bandstack[BAND_20][0].setFilter(settings->value("filter.5.0",4).toInt());
    bandstack[BAND_20][0].setSpectrumHigh(settings->value("spectrumHigh.5.0",-40).toInt());
    bandstack[BAND_20][0].setSpectrumLow(settings->value("spectrumLow.5.0",-160).toInt());
    bandstack[BAND_20][0].setWaterfallHigh(settings->value("waterfallHigh.5.0",-60).toInt());
    bandstack[BAND_20][0].setWaterfallLow(settings->value("waterfallLow.5.0",-125).toInt());
    bandstack[BAND_20][0].setInfo(settings->value("info.5.0",2).toInt());  //Count of Quick Memories 0 .. n
    // Quick memory number 2
    bandstack[BAND_20][1].setFrequency(settings->value("frequency.5.1",14230000).toLongLong());
    bandstack[BAND_20][1].setMode(settings->value("mode.5.1",MODE_USB).toInt());
    bandstack[BAND_20][1].setFilter(settings->value("filter.5.1",3).toInt());
    bandstack[BAND_20][1].setSpectrumHigh(settings->value("spectrumHigh.5.1",-40).toInt());
    bandstack[BAND_20][1].setSpectrumLow(settings->value("spectrumLow.5.1",-160).toInt());
    bandstack[BAND_20][1].setWaterfallHigh(settings->value("waterfallHigh.5.1",-60).toInt());
    bandstack[BAND_20][1].setWaterfallLow(settings->value("waterfallLow.5.1",-125).toInt());
    bandstack[BAND_20][1].setInfo(settings->value("info.5.1",0).toInt());  //Pointer to stack for reading
    // Quick memory number 3
    bandstack[BAND_20][2].setFrequency(settings->value("frequency.5.2",14336000).toLongLong());
    bandstack[BAND_20][2].setMode(settings->value("mode.5.2",MODE_USB).toInt());
    bandstack[BAND_20][2].setFilter(settings->value("filter.5.2",3).toInt());
    bandstack[BAND_20][2].setSpectrumHigh(settings->value("spectrumHigh.5.2",-40).toInt());
    bandstack[BAND_20][2].setSpectrumLow(settings->value("spectrumLow.5.2",-160).toInt());
    bandstack[BAND_20][2].setWaterfallHigh(settings->value("waterfallHigh.5.2",-60).toInt());
    bandstack[BAND_20][2].setWaterfallLow(settings->value("waterfallLow.5.2",-125).toInt());
    bandstack[BAND_20][2].setInfo(settings->value("info.5.2",0).toInt());  //Pointer to stack for storing
    // Current working frequency
    bandstack[BAND_20][3].setFrequency(settings->value("frequency.5.3",14336000).toLongLong());
    bandstack[BAND_20][3].setMode(settings->value("mode.5.3",MODE_USB).toInt());
    bandstack[BAND_20][3].setFilter(settings->value("filter.5.3",3).toInt());
    bandstack[BAND_20][3].setSpectrumHigh(settings->value("spectrumHigh.5.3",-40).toInt());
    bandstack[BAND_20][3].setSpectrumLow(settings->value("spectrumLow.5.3",-160).toInt());
    bandstack[BAND_20][3].setWaterfallHigh(settings->value("waterfallHigh.5.3",-60).toInt());
    bandstack[BAND_20][3].setWaterfallLow(settings->value("waterfallLow.5.3",-125).toInt());
    bandstack[BAND_20][3].setInfo(settings->value("info.5.3",0).toInt());  //Null info. Available for whatever.

    // Quick memory number 1
    bandstack[BAND_17][0].setFrequency(settings->value("frequency.6.0",18068600).toLongLong());
    bandstack[BAND_17][0].setMode(settings->value("mode.6.0",MODE_CWU).toInt());
    bandstack[BAND_17][0].setFilter(settings->value("filter.6.0",4).toInt());
    bandstack[BAND_17][0].setSpectrumHigh(settings->value("spectrumHigh.6.0",-40).toInt());
    bandstack[BAND_17][0].setSpectrumLow(settings->value("spectrumLow.6.0",-160).toInt());
    bandstack[BAND_17][0].setWaterfallHigh(settings->value("waterfallHigh.6.0",-60).toInt());
    bandstack[BAND_17][0].setWaterfallLow(settings->value("waterfallLow.6.0",-125).toInt());
    bandstack[BAND_17][0].setInfo(settings->value("info.6.0",2).toInt());  //Count of Quick Memories 0 .. n
    // Quick memory number 2
    bandstack[BAND_17][1].setFrequency(settings->value("frequency.6.1",18125000).toLongLong());
    bandstack[BAND_17][1].setMode(settings->value("mode.6.1",MODE_USB).toInt());
    bandstack[BAND_17][1].setFilter(settings->value("filter.6.1",3).toInt());
    bandstack[BAND_17][1].setSpectrumHigh(settings->value("spectrumHigh.6.1",-40).toInt());
    bandstack[BAND_17][1].setSpectrumLow(settings->value("spectrumLow.6.1",-160).toInt());
    bandstack[BAND_17][1].setWaterfallHigh(settings->value("waterfallHigh.6.1",-60).toInt());
    bandstack[BAND_17][1].setWaterfallLow(settings->value("waterfallLow.6.1",-125).toInt());
    bandstack[BAND_17][1].setInfo(settings->value("info.6.1",0).toInt());  //Pointer to stack for reading
    // Quick memory number 3
    bandstack[BAND_17][2].setFrequency(settings->value("frequency.6.2",18140000).toLongLong());
    bandstack[BAND_17][2].setMode(settings->value("mode.6.2",MODE_USB).toInt());
    bandstack[BAND_17][2].setFilter(settings->value("filter.6.2",3).toInt());
    bandstack[BAND_17][2].setSpectrumHigh(settings->value("spectrumHigh.6.2",-40).toInt());
    bandstack[BAND_17][2].setSpectrumLow(settings->value("spectrumLow.6.2",-160).toInt());
    bandstack[BAND_17][2].setWaterfallHigh(settings->value("waterfallHigh.6.2",-60).toInt());
    bandstack[BAND_17][2].setWaterfallLow(settings->value("waterfallLow.6.2",-125).toInt());
    bandstack[BAND_17][2].setInfo(settings->value("info.6.2",0).toInt());  //Pointer to stack for storing
    // Current working frequency
    bandstack[BAND_17][3].setFrequency(settings->value("frequency.6.3",18140000).toLongLong());
    bandstack[BAND_17][3].setMode(settings->value("mode.6.3",MODE_USB).toInt());
    bandstack[BAND_17][3].setFilter(settings->value("filter.6.3",3).toInt());
    bandstack[BAND_17][3].setSpectrumHigh(settings->value("spectrumHigh.6.3",-40).toInt());
    bandstack[BAND_17][3].setSpectrumLow(settings->value("spectrumLow.6.3",-160).toInt());
    bandstack[BAND_17][3].setWaterfallHigh(settings->value("waterfallHigh.6.3",-60).toInt());
    bandstack[BAND_17][3].setWaterfallLow(settings->value("waterfallLow.6.3",-125).toInt());
    bandstack[BAND_17][3].setInfo(settings->value("info.6.3",0).toInt());  //Null info. Available for whatever.

    // Quick memory number 1
    bandstack[BAND_15][0].setFrequency(settings->value("frequency.7.0",21001000).toLongLong());
    bandstack[BAND_15][0].setMode(settings->value("mode.7.0",MODE_CWU).toInt());
    bandstack[BAND_15][0].setFilter(settings->value("filter.7.0",4).toInt());
    bandstack[BAND_15][0].setSpectrumHigh(settings->value("spectrumHigh.7.0",-40).toInt());
    bandstack[BAND_15][0].setSpectrumLow(settings->value("spectrumLow.7.0",-160).toInt());
    bandstack[BAND_15][0].setWaterfallHigh(settings->value("waterfallHigh.7.0",-60).toInt());
    bandstack[BAND_15][0].setWaterfallLow(settings->value("waterfallLow.7.0",-125).toInt());
    bandstack[BAND_15][0].setInfo(settings->value("info.7.0",2).toInt());  //Count of Quick Memories 0 .. n
    // Quick memory number 2
    bandstack[BAND_15][1].setFrequency(settings->value("frequency.7.1",21255000).toLongLong());
    bandstack[BAND_15][1].setMode(settings->value("mode.7.1",MODE_USB).toInt());
    bandstack[BAND_15][1].setFilter(settings->value("filter.7.1",3).toInt());
    bandstack[BAND_15][1].setSpectrumHigh(settings->value("spectrumHigh.7.1",-40).toInt());
    bandstack[BAND_15][1].setSpectrumLow(settings->value("spectrumLow.7.1",-160).toInt());
    bandstack[BAND_15][1].setWaterfallHigh(settings->value("waterfallHigh.7.1",-60).toInt());
    bandstack[BAND_15][1].setWaterfallLow(settings->value("waterfallLow.7.1",-125).toInt());
    bandstack[BAND_15][1].setInfo(settings->value("info.7.1",0).toInt());  //Pointer to stack for reading
    // Quick memory number 3
    bandstack[BAND_15][2].setFrequency(settings->value("frequency.7.2",21300000).toLongLong());
    bandstack[BAND_15][2].setMode(settings->value("mode.7.2",MODE_USB).toInt());
    bandstack[BAND_15][2].setFilter(settings->value("filter.7.2",3).toInt());
    bandstack[BAND_15][2].setSpectrumHigh(settings->value("spectrumHigh.7.2",-40).toInt());
    bandstack[BAND_15][2].setSpectrumLow(settings->value("spectrumLow.7.2",-160).toInt());
    bandstack[BAND_15][2].setWaterfallHigh(settings->value("waterfallHigh.7.2",-60).toInt());
    bandstack[BAND_15][2].setWaterfallLow(settings->value("waterfallLow.7.2",-125).toInt());
    bandstack[BAND_15][2].setInfo(settings->value("info.7.2",0).toInt());  //Pointer to stack for storing
    // Current working frequency
    bandstack[BAND_15][3].setFrequency(settings->value("frequency.7.3",21300000).toLongLong());
    bandstack[BAND_15][3].setMode(settings->value("mode.7.3",MODE_USB).toInt());
    bandstack[BAND_15][3].setFilter(settings->value("filter.7.3",3).toInt());
    bandstack[BAND_15][3].setSpectrumHigh(settings->value("spectrumHigh.7.3",-40).toInt());
    bandstack[BAND_15][3].setSpectrumLow(settings->value("spectrumLow.7.3",-160).toInt());
    bandstack[BAND_15][3].setWaterfallHigh(settings->value("waterfallHigh.7.3",-60).toInt());
    bandstack[BAND_15][3].setWaterfallLow(settings->value("waterfallLow.7.3",-125).toInt());
    bandstack[BAND_15][3].setInfo(settings->value("info.7.3",0).toInt());  //Null info. Available for whatever.

    // Quick memory number 1
    bandstack[BAND_12][0].setFrequency(settings->value("frequency.8.0",24895000).toLongLong());
    bandstack[BAND_12][0].setMode(settings->value("mode.8.0",MODE_CWU).toInt());
    bandstack[BAND_12][0].setFilter(settings->value("filter.8.0",4).toInt());
    bandstack[BAND_12][0].setSpectrumHigh(settings->value("spectrumHigh.8.0",-40).toInt());
    bandstack[BAND_12][0].setSpectrumLow(settings->value("spectrumLow.8.0",-160).toInt());
    bandstack[BAND_12][0].setWaterfallHigh(settings->value("waterfallHigh.8.0",-60).toInt());
    bandstack[BAND_12][0].setWaterfallLow(settings->value("waterfallLow.8.0",-125).toInt());
    bandstack[BAND_12][0].setInfo(settings->value("info.8.0",2).toInt());  //Count of Quick Memories 0 .. n
    // Quick memory number 2
    bandstack[BAND_12][1].setFrequency(settings->value("frequency.8.1",24900000).toLongLong());
    bandstack[BAND_12][1].setMode(settings->value("mode.8.1",MODE_CWU).toInt());
    bandstack[BAND_12][1].setFilter(settings->value("filter.8.1",3).toInt());
    bandstack[BAND_12][1].setSpectrumHigh(settings->value("spectrumHigh.8.1",-40).toInt());
    bandstack[BAND_12][1].setSpectrumLow(settings->value("spectrumLow.8.1",-160).toInt());
    bandstack[BAND_12][1].setWaterfallHigh(settings->value("waterfallHigh.8.1",-60).toInt());
    bandstack[BAND_12][1].setWaterfallLow(settings->value("waterfallLow.8.1",-125).toInt());
    bandstack[BAND_12][1].setInfo(settings->value("info.8.1",0).toInt());  //Pointer to stack for reading
    // Quick memory number 3
    bandstack[BAND_12][2].setFrequency(settings->value("frequency.8.2",24910000).toLongLong());
    bandstack[BAND_12][2].setMode(settings->value("mode.8.2",MODE_CWU).toInt());
    bandstack[BAND_12][2].setFilter(settings->value("filter.8.2",3).toInt());
    bandstack[BAND_12][2].setSpectrumHigh(settings->value("spectrumHigh.8.2",-40).toInt());
    bandstack[BAND_12][2].setSpectrumLow(settings->value("spectrumLow.8.2",-160).toInt());
    bandstack[BAND_12][2].setWaterfallHigh(settings->value("waterfallHigh.8.2",-60).toInt());
    bandstack[BAND_12][2].setWaterfallLow(settings->value("waterfallLow.8.2",-125).toInt());
    bandstack[BAND_12][2].setInfo(settings->value("info.8.2",0).toInt());  //Pointer to stack for storing
    // Current working frequency
    bandstack[BAND_12][3].setFrequency(settings->value("frequency.8.3",24910000).toLongLong());
    bandstack[BAND_12][3].setMode(settings->value("mode.8.3",MODE_CWU).toInt());
    bandstack[BAND_12][3].setFilter(settings->value("filter.8.3",3).toInt());
    bandstack[BAND_12][3].setSpectrumHigh(settings->value("spectrumHigh.8.3",-40).toInt());
    bandstack[BAND_12][3].setSpectrumLow(settings->value("spectrumLow.8.3",-160).toInt());
    bandstack[BAND_12][3].setWaterfallHigh(settings->value("waterfallHigh.8.3",-60).toInt());
    bandstack[BAND_12][3].setWaterfallLow(settings->value("waterfallLow.8.3",-125).toInt());
    bandstack[BAND_12][3].setInfo(settings->value("info.8.3",0).toInt());  //Null info. Available for whatever.

    // Quick memory number 1
    bandstack[BAND_10][0].setFrequency(settings->value("frequency.9.0",28010000).toLongLong());
    bandstack[BAND_10][0].setMode(settings->value("mode.9.0",MODE_CWU).toInt());
    bandstack[BAND_10][0].setFilter(settings->value("filter.9.0",4).toInt());
    bandstack[BAND_10][0].setSpectrumHigh(settings->value("spectrumHigh.9.0",-40).toInt());
    bandstack[BAND_10][0].setSpectrumLow(settings->value("spectrumLow.9.0",-160).toInt());
    bandstack[BAND_10][0].setWaterfallHigh(settings->value("waterfallHigh.9.0",-60).toInt());
    bandstack[BAND_10][0].setWaterfallLow(settings->value("waterfallLow.9.0",-125).toInt());
    bandstack[BAND_10][0].setInfo(settings->value("info.9.0",2).toInt());  //Count of Quick Memories 0 .. n
    // Quick memory number 2
    bandstack[BAND_10][1].setFrequency(settings->value("frequency.9.1",28300000).toLongLong());
    bandstack[BAND_10][1].setMode(settings->value("mode.9.1",MODE_USB).toInt());
    bandstack[BAND_10][1].setFilter(settings->value("filter.9.1",3).toInt());
    bandstack[BAND_10][1].setSpectrumHigh(settings->value("spectrumHigh.9.1",-40).toInt());
    bandstack[BAND_10][1].setSpectrumLow(settings->value("spectrumLow.9.1",-160).toInt());
    bandstack[BAND_10][1].setWaterfallHigh(settings->value("waterfallHigh.9.1",-60).toInt());
    bandstack[BAND_10][1].setWaterfallLow(settings->value("waterfallLow.9.1",-125).toInt());
    bandstack[BAND_10][1].setInfo(settings->value("info.9.1",0).toInt());  //Pointer to stack for reading
    // Quick memory number 3
    bandstack[BAND_10][2].setFrequency(settings->value("frequency.9.2",28400000).toLongLong());
    bandstack[BAND_10][2].setMode(settings->value("mode.9.2",MODE_USB).toInt());
    bandstack[BAND_10][2].setFilter(settings->value("filter.9.2",3).toInt());
    bandstack[BAND_10][2].setSpectrumHigh(settings->value("spectrumHigh.9.2",-40).toInt());
    bandstack[BAND_10][2].setSpectrumLow(settings->value("spectrumLow.9.2",-160).toInt());
    bandstack[BAND_10][2].setWaterfallHigh(settings->value("waterfallHigh.9.2",-60).toInt());
    bandstack[BAND_10][2].setWaterfallLow(settings->value("waterfallLow.9.2",-125).toInt());
    bandstack[BAND_10][2].setInfo(settings->value("info.9.2",0).toInt());  //Pointer to stack for storing
    // Current working frequency
    bandstack[BAND_10][3].setFrequency(settings->value("frequency.9.3",28400000).toLongLong());
    bandstack[BAND_10][3].setMode(settings->value("mode.9.3",MODE_USB).toInt());
    bandstack[BAND_10][3].setFilter(settings->value("filter.9.3",3).toInt());
    bandstack[BAND_10][3].setSpectrumHigh(settings->value("spectrumHigh.9.3",-40).toInt());
    bandstack[BAND_10][3].setSpectrumLow(settings->value("spectrumLow.9.3",-160).toInt());
    bandstack[BAND_10][3].setWaterfallHigh(settings->value("waterfallHigh.9.3",-60).toInt());
    bandstack[BAND_10][3].setWaterfallLow(settings->value("waterfallLow.9.3",-125).toInt());
    bandstack[BAND_10][3].setInfo(settings->value("info.9.3",0).toInt());  //Null info. Available for whatever.

    // Quick memory number 1
    bandstack[BAND_6][0].setFrequency(settings->value("frequency.10.0",50010000).toLongLong());
    bandstack[BAND_6][0].setMode(settings->value("mode.10.0",MODE_CWU).toInt());
    bandstack[BAND_6][0].setFilter(settings->value("filter.10.0",4).toInt());
    bandstack[BAND_6][0].setSpectrumHigh(settings->value("spectrumHigh.10.0",-40).toInt());
    bandstack[BAND_6][0].setSpectrumLow(settings->value("spectrumLow.10.0",-160).toInt());
    bandstack[BAND_6][0].setWaterfallHigh(settings->value("waterfallHigh.10.0",-60).toInt());
    bandstack[BAND_6][0].setWaterfallLow(settings->value("waterfallLow.10.0",-125).toInt());
    bandstack[BAND_6][0].setInfo(settings->value("info.10.0",2).toInt());  //Count of Quick Memories 0 .. n
    // Quick memory number 2
    bandstack[BAND_6][1].setFrequency(settings->value("frequency.10.1",50125000).toLongLong());
    bandstack[BAND_6][1].setMode(settings->value("mode.10.1",MODE_USB).toInt());
    bandstack[BAND_6][1].setFilter(settings->value("filter.10.1",3).toInt());
    bandstack[BAND_6][1].setSpectrumHigh(settings->value("spectrumHigh.10.1",-40).toInt());
    bandstack[BAND_6][1].setSpectrumLow(settings->value("spectrumLow.10.1",-160).toInt());
    bandstack[BAND_6][1].setWaterfallHigh(settings->value("waterfallHigh.10.1",-60).toInt());
    bandstack[BAND_6][1].setWaterfallLow(settings->value("waterfallLow.10.1",-125).toInt());
    bandstack[BAND_6][1].setInfo(settings->value("info.10.1",0).toInt());  //Pointer to stack for reading
    // Quick memory number 3
    bandstack[BAND_6][2].setFrequency(settings->value("frequency.10.2",50200000).toLongLong());
    bandstack[BAND_6][2].setMode(settings->value("mode.10.2",MODE_USB).toInt());
    bandstack[BAND_6][2].setFilter(settings->value("filter.10.2",3).toInt());
    bandstack[BAND_6][2].setSpectrumHigh(settings->value("spectrumHigh.10.2",-40).toInt());
    bandstack[BAND_6][2].setSpectrumLow(settings->value("spectrumLow.10.2",-160).toInt());
    bandstack[BAND_6][2].setWaterfallHigh(settings->value("waterfallHigh.10.2",-60).toInt());
    bandstack[BAND_6][2].setWaterfallLow(settings->value("waterfallLow.10.2",-125).toInt());
    bandstack[BAND_6][2].setInfo(settings->value("info.10.2",0).toInt());  //Pointer to stack for storing
    // Current working frequency
    bandstack[BAND_6][3].setFrequency(settings->value("frequency.10.3",50200000).toLongLong());
    bandstack[BAND_6][3].setMode(settings->value("mode.10.3",MODE_USB).toInt());
    bandstack[BAND_6][3].setFilter(settings->value("filter.10.3",3).toInt());
    bandstack[BAND_6][3].setSpectrumHigh(settings->value("spectrumHigh.10.3",-40).toInt());
    bandstack[BAND_6][3].setSpectrumLow(settings->value("spectrumLow.10.3",-160).toInt());
    bandstack[BAND_6][3].setWaterfallHigh(settings->value("waterfallHigh.10.3",-60).toInt());
    bandstack[BAND_6][3].setWaterfallLow(settings->value("waterfallLow.10.3",-125).toInt());
    bandstack[BAND_6][3].setInfo(settings->value("info.10.3",0).toInt());  //Null info. Available for whatever.

    // Quick memory number 1
    bandstack[BAND_GEN][0].setFrequency(settings->value("frequency.11.0",1359000).toLongLong());
    bandstack[BAND_GEN][0].setMode(settings->value("mode.11.0",MODE_AM).toInt());
    bandstack[BAND_GEN][0].setFilter(settings->value("filter.11.0",3).toInt());
    bandstack[BAND_GEN][0].setSpectrumHigh(settings->value("spectrumHigh.11.0",-40).toInt());
    bandstack[BAND_GEN][0].setSpectrumLow(settings->value("spectrumLow.11.0",-160).toInt());
    bandstack[BAND_GEN][0].setWaterfallHigh(settings->value("waterfallHigh.11.0",-60).toInt());
    bandstack[BAND_GEN][0].setWaterfallLow(settings->value("waterfallLow.11.0",-125).toInt());
    bandstack[BAND_GEN][0].setInfo(settings->value("info.11.0",3).toInt());  //Count of Quick Memories 0 .. n
    // Quick memory number 2
    bandstack[BAND_GEN][1].setFrequency(settings->value("frequency.11.1",6145000).toLongLong());
    bandstack[BAND_GEN][1].setMode(settings->value("mode.11.1",MODE_AM).toInt());
    bandstack[BAND_GEN][1].setFilter(settings->value("filter.11.1",3).toInt());
    bandstack[BAND_GEN][1].setSpectrumHigh(settings->value("spectrumHigh.11.1",-40).toInt());
    bandstack[BAND_GEN][1].setSpectrumLow(settings->value("spectrumLow.11.1",-160).toInt());
    bandstack[BAND_GEN][1].setWaterfallHigh(settings->value("waterfallHigh.11.1",-60).toInt());
    bandstack[BAND_GEN][1].setWaterfallLow(settings->value("waterfallLow.11.1",-125).toInt());
    bandstack[BAND_GEN][1].setInfo(settings->value("info.11.1",0).toInt());  //Pointer to stack for reading
    // Quick memory number 3
    bandstack[BAND_GEN][2].setFrequency(settings->value("frequency.11.2",11765000).toLongLong());
    bandstack[BAND_GEN][2].setMode(settings->value("mode.11.2",MODE_AM).toInt());
    bandstack[BAND_GEN][2].setFilter(settings->value("filter.11.2",3).toInt());
    bandstack[BAND_GEN][2].setSpectrumHigh(settings->value("spectrumHigh.11.2",-40).toInt());
    bandstack[BAND_GEN][2].setSpectrumLow(settings->value("spectrumLow.11.2",-160).toInt());
    bandstack[BAND_GEN][2].setWaterfallHigh(settings->value("waterfallHigh.11.2",-60).toInt());
    bandstack[BAND_GEN][2].setWaterfallLow(settings->value("waterfallLow.11.2",-125).toInt());
    bandstack[BAND_GEN][2].setInfo(settings->value("info.11.2",0).toInt());  //Pointer to stack for storing
    // Quick memory number 4
    bandstack[BAND_GEN][3].setFrequency(settings->value("frequency.11.3",15400000).toLongLong());
    bandstack[BAND_GEN][3].setMode(settings->value("mode.11.3",MODE_AM).toInt());
    bandstack[BAND_GEN][3].setFilter(settings->value("filter.11.3",3).toInt());
    bandstack[BAND_GEN][3].setSpectrumHigh(settings->value("spectrumHigh.11.3",-40).toInt());
    bandstack[BAND_GEN][3].setSpectrumLow(settings->value("spectrumLow.11.3",-160).toInt());
    bandstack[BAND_GEN][3].setWaterfallHigh(settings->value("waterfallHigh.11.3",-60).toInt());
    bandstack[BAND_GEN][3].setWaterfallLow(settings->value("waterfallLow.11.3",-125).toInt());
    bandstack[BAND_GEN][3].setInfo(settings->value("info.11.3",0).toInt());  //Reserved for total entries
    // Quick memory number 5
    bandstack[BAND_GEN][4].setFrequency(settings->value("frequency.11.4",17795000).toLongLong());
    bandstack[BAND_GEN][4].setMode(settings->value("mode.11.4",MODE_AM).toInt());
    bandstack[BAND_GEN][4].setFilter(settings->value("filter.11.4",3).toInt());
    bandstack[BAND_GEN][4].setSpectrumHigh(settings->value("spectrumHigh.11.4",-40).toInt());
    bandstack[BAND_GEN][4].setSpectrumLow(settings->value("spectrumLow.11.4",-160).toInt());
    bandstack[BAND_GEN][4].setWaterfallHigh(settings->value("waterfallHigh.11.4",-60).toInt());
    bandstack[BAND_GEN][4].setWaterfallLow(settings->value("waterfallLow.11.4",-125).toInt());
    bandstack[BAND_GEN][4].setInfo(settings->value("info.11.4",0).toInt());  //Not used. Band terminated by being 5 entries

    // Quick memory number 1
    bandstack[BAND_WWV][0].setFrequency(settings->value("frequency.12.0",2500000).toLongLong());
    bandstack[BAND_WWV][0].setMode(settings->value("mode.12.0",MODE_AM).toInt());
    bandstack[BAND_WWV][0].setFilter(settings->value("filter.12.0",4).toInt());
    bandstack[BAND_WWV][0].setSpectrumHigh(settings->value("spectrumHigh.12.0",-40).toInt());
    bandstack[BAND_WWV][0].setSpectrumLow(settings->value("spectrumLow.12.0",-160).toInt());
    bandstack[BAND_WWV][0].setWaterfallHigh(settings->value("waterfallHigh.12.0",-60).toInt());
    bandstack[BAND_WWV][0].setWaterfallLow(settings->value("waterfallLow.12.0",-125).toInt());
    bandstack[BAND_WWV][0].setInfo(settings->value("info.12.0",3).toInt());  //Count of Quick Memories 0 .. n
    // Quick memory number 2
    bandstack[BAND_WWV][1].setFrequency(settings->value("frequency.12.1",5000000).toLongLong());
    bandstack[BAND_WWV][1].setMode(settings->value("mode.12.1",MODE_AM).toInt());
    bandstack[BAND_WWV][1].setFilter(settings->value("filter.12.1",4).toInt());
    bandstack[BAND_WWV][1].setSpectrumHigh(settings->value("spectrumHigh.12.1",-40).toInt());
    bandstack[BAND_WWV][1].setSpectrumLow(settings->value("spectrumLow.12.1",-160).toInt());
    bandstack[BAND_WWV][1].setWaterfallHigh(settings->value("waterfallHigh.12.1",-60).toInt());
    bandstack[BAND_WWV][1].setWaterfallLow(settings->value("waterfallLow.12.1",-125).toInt());
    bandstack[BAND_WWV][1].setInfo(settings->value("info.12.1",0).toInt());  //Pointer to stack for reading
    // Quick memory number 3
    bandstack[BAND_WWV][2].setFrequency(settings->value("frequency.12.2",10000000).toLongLong());
    bandstack[BAND_WWV][2].setMode(settings->value("mode.12.2",MODE_AM).toInt());
    bandstack[BAND_WWV][2].setFilter(settings->value("filter.12.2",4).toInt());
    bandstack[BAND_WWV][2].setSpectrumHigh(settings->value("spectrumHigh.12.2",-40).toInt());
    bandstack[BAND_WWV][2].setSpectrumLow(settings->value("spectrumLow.12.2",-160).toInt());
    bandstack[BAND_WWV][2].setWaterfallHigh(settings->value("waterfallHigh.12.2",-60).toInt());
    bandstack[BAND_WWV][2].setWaterfallLow(settings->value("waterfallLow.12.2",-125).toInt());
    bandstack[BAND_WWV][2].setInfo(settings->value("info.12.2",0).toInt());  //Pointer to stack for storing
    // Quick memory number 4
    bandstack[BAND_WWV][3].setFrequency(settings->value("frequency.12.3",15000000).toLongLong());
    bandstack[BAND_WWV][3].setMode(settings->value("mode.12.3",MODE_AM).toInt());
    bandstack[BAND_WWV][3].setFilter(settings->value("filter.12.3",4).toInt());
    bandstack[BAND_WWV][3].setSpectrumHigh(settings->value("spectrumHigh.12.3",-40).toInt());
    bandstack[BAND_WWV][3].setSpectrumLow(settings->value("spectrumLow.12.3",-160).toInt());
    bandstack[BAND_WWV][3].setWaterfallHigh(settings->value("waterfallHigh.12.3",-60).toInt());
    bandstack[BAND_WWV][3].setWaterfallLow(settings->value("waterfallLow.12.3",-125).toInt());
    bandstack[BAND_WWV][3].setInfo(settings->value("info.12.3",0).toInt());  //Reserved for total entries
    // Quick memory number 5
    bandstack[BAND_WWV][4].setFrequency(settings->value("frequency.12.4",20000000).toLongLong());
    bandstack[BAND_WWV][4].setMode(settings->value("mode.12.4",MODE_AM).toInt());
    bandstack[BAND_WWV][4].setFilter(settings->value("filter.12.4",4).toInt());
    bandstack[BAND_WWV][4].setSpectrumHigh(settings->value("spectrumHigh.12.4",-40).toInt());
    bandstack[BAND_WWV][4].setSpectrumLow(settings->value("spectrumLow.12.4",-160).toInt());
    bandstack[BAND_WWV][4].setWaterfallHigh(settings->value("waterfallHigh.12.4",-60).toInt());
    bandstack[BAND_WWV][4].setWaterfallLow(settings->value("waterfallLow.12.4",-125).toInt());
    bandstack[BAND_WWV][4].setInfo(settings->value("info.12.4",0).toInt());  //Not used.
    settings->endGroup();

    workingStack=bandstack[currentBand][0].getInfo()+1;
    currentStack=bandstack[currentBand][1].getInfo();
    if(currentBand==BAND_WWV){
        workingStack = currentStack;
//        qDebug()<<Q_FUNC_INFO<<":    in the constructor, currentBand, current stack, workingStack = "<<currentBand<<", "<<currentStack<<", "<<workingStack;
    }

    for(int x=0;x<4;x++){
        qDebug()<<"frequency.0."<<x<<" = "<<bandstack[currentBand][x].getFrequency();
        qDebug()<<"mode.0."<<x<<" = "<<bandstack[currentBand][x].getMode();
        qDebug()<<"filter.0."<<x<<" = "<<bandstack[currentBand][x].getFilter();
    }

    //Load the band limits for the eleven band buttons. GEN has no limits. WWV is hard coded.
    settings->beginGroup("bandLimits");
    limitMin=settings->value("limits.0.min",1800000).toLongLong();
    limitMax=settings->value("limits.0.max",2000000).toLongLong();
        limits << BandLimit(limitMin,limitMax);
    limitMin=settings->value("limits.1.min",3500000).toLongLong();
    limitMax=settings->value("limits.1.max",4000000).toLongLong();
        limits << BandLimit(limitMin,limitMax);
    limitMin=settings->value("limits.2.min",5330500).toLongLong();
    limitMax=settings->value("limits.2.max",5403500).toLongLong();
        limits << BandLimit(limitMin,limitMax);
    limitMin=settings->value("limits.3.min",7000000).toLongLong();
    limitMax=settings->value("limits.3.max",7300000).toLongLong();
        limits << BandLimit(limitMin,limitMax);
    limitMin=settings->value("limits.4.min",10100000).toLongLong();
    limitMax=settings->value("limits.4.max",10150000).toLongLong();
        limits << BandLimit(limitMin,limitMax);
    limitMin=settings->value("limits.5.min",14000000).toLongLong();
    limitMax=settings->value("limits.5.max",14350000).toLongLong();
        limits << BandLimit(limitMin,limitMax);
    limitMin=settings->value("limits.6.min",18068000).toLongLong();
    limitMax=settings->value("limits.6.max",18168000).toLongLong();
        limits << BandLimit(limitMin,limitMax);
    limitMin=settings->value("limits.7.min",21000000).toLongLong();
    limitMax=settings->value("limits.7.max",21450000).toLongLong();
        limits << BandLimit(limitMin,limitMax);
    limitMin=settings->value("limits.8.min",24890000).toLongLong();
    limitMax=settings->value("limits.8.max",24990000).toLongLong();
        limits << BandLimit(limitMin,limitMax);
    limitMin=settings->value("limits.9.min",28000000).toLongLong();
    limitMax=settings->value("limits.9.max",29700000).toLongLong();
        limits << BandLimit(limitMin,limitMax);
    limitMin=settings->value("limits.10.min",50000000).toLongLong();
    limitMax=settings->value("limits.10.max",54000000).toLongLong();
        limits << BandLimit(limitMin,limitMax);
    limitMin=settings->value("limits.11.min",0).toLongLong();
    limitMax=settings->value("limits.11.max",0).toLongLong();
        limits << BandLimit(limitMin,limitMax);

    settings->endGroup();
}

void Band::saveSettings(QSettings* settings) {
    int i,j;
    QString s;
    BandLimit limitItem;

    settings->beginGroup("Band");
    settings->setValue("currentBand",currentBand);
//    settings->setValue("currentStack",currentStack); //TODO this will go as current stack will always be after last memory
    for(i=0;i<BAND_LAST;i++) {
//        s.sprintf("stack.%d",i);    //TODO remove this and replace with bandstack getInfo data
//        settings->setValue(s,stack[i]);
        for(j=0;j<bandstack[i][0].getInfo()+2;j++) { //Number of memories +1 = current working stack
            s.sprintf("frequency.%d.%d",i,j);
            settings->setValue(s,bandstack[i][j].getFrequency());
            s.sprintf("filter.%d.%d",i,j);
            settings->setValue(s,bandstack[i][j].getFilter());
            s.sprintf("mode.%d.%d",i,j);
            settings->setValue(s,bandstack[i][j].getMode());
            s.sprintf("spectrumHigh.%d.%d",i,j);
            settings->setValue(s,bandstack[i][j].getSpectrumHigh());
            s.sprintf("spectrumLow.%d.%d",i,j);
            settings->setValue(s,bandstack[i][j].getSpectrumLow());
            s.sprintf("waterfallHigh.%d.%d",i,j);
            settings->setValue(s,bandstack[i][j].getWaterfallHigh());
            s.sprintf("waterfallLow.%d.%d",i,j);
            settings->setValue(s,bandstack[i][j].getWaterfallLow());
            s.sprintf("info.%d.%d",i,j);
            settings->setValue(s,bandstack[i][j].getInfo());
        }
    }
    settings->endGroup();

    settings->beginGroup("bandLimits");
    i = 0;
    limitItem = limits.at(i);
    while (limitItem.min()!=0LL) {
        s.sprintf("limits.%d.min",i);
        settings->setValue(s,limitItem.min());
        s.sprintf("limits.%d.max",i);
        settings->setValue(s,limitItem.max());
        i++;
        limitItem = limits.at(i);
    }
    //Terminate group with min = 0
    s.sprintf("limits.%d.min",i);
    settings->setValue(s,0);
    s.sprintf("limits.%d.max",i);
    settings->setValue(s,0);
    settings->endGroup();
}

void Band::initBand(int b) {
    currentBand=b;
    emit bandChanged(currentBand, currentBand);
}

int Band::getBand() {
    return currentBand;
}

QString Band::getStringBand() {
    QString b="Gen";

    b=getStringBand(currentBand);

    b.append("(");
    b.append(QString::number(workingStack));
    b.append(")");

    return b;
}

//Returns a string = Current band, (Current mem, Next mem store location)
QString Band::getStringMem()
{
    QString b="Gen";
    int nextStore;

    b=getStringBand(currentBand);
    nextStore = bandstack[currentBand][2].getInfo(); //Point to memory write position for this band
    nextStore--;         // Steps backwards to point to next store location
    if(nextStore < 0) {  //Store position for this band, 0 .. n
        nextStore = bandstack[currentBand][0].getInfo();
    }
    b.append("(");
    b.append(QString::number(currentStack));
    b.append(", ");
    b.append(QString::number(nextStore));
    b.append(")");

    return b;
}

QString Band::getStringBand(int band) {
    QString b="Gen";

    switch(band) {
        case BAND_160:
            b="160 Mtrs";
            break;
        case BAND_80:
            b="80 Mtrs";
            break;
        case BAND_60:
            b="60 Mtrs";
            break;
        case BAND_40:
            b="40 Mtrs";
            break;
        case BAND_30:
            b="30 Mtrs";
            break;
        case BAND_20:
            b="20 Mtrs";
            break;
        case BAND_17:
            b="17 Mtrs";
            break;
        case BAND_15:
            b="15 Mtrs";
            break;
        case BAND_12:
            b="12 Mtrs";
            break;
        case BAND_10:
            b="10 Mtrs";
            break;
        case BAND_6:
            b="6 Mtrs";
            break;
        case BAND_GEN:
            b="Gen";
            break;
        case BAND_WWV:
            b="WWV";
            break;
    }

    return b;
}

long long Band::getFrequency() {
    return bandstack[currentBand][workingStack].getFrequency();
}

int Band::getMode() {
    return bandstack[currentBand][workingStack].getMode();
}

int Band::getFilter() {
    return bandstack[currentBand][workingStack].getFilter();
}

int Band::getInfo() {
    return bandstack[currentBand][workingStack].getInfo();
}

int Band::getSpectrumHigh() {
    return bandstack[currentBand][workingStack].getSpectrumHigh();
}

int Band::getSpectrumLow() {
    return bandstack[currentBand][workingStack].getSpectrumLow();
}

int Band::getWaterfallHigh() {
    return bandstack[currentBand][workingStack].getWaterfallHigh();
}

int Band::getWaterfallLow() {
    return bandstack[currentBand][workingStack].getWaterfallLow();
}

BandLimit Band::getBandLimits(long long minDisplay, long long maxDisplay) {
    BandLimit result=limits.at(limits.size()-1);

    qDebug() << "Band::getBandLimits: " << minDisplay << "," << maxDisplay;
    for(int i=0;i<limits.size();i++) {
        result=limits.at(i);

        if((result.min()>=minDisplay&&result.min()<=maxDisplay) || // band min within the display
           (result.max()<=maxDisplay&&result.max()>=minDisplay) || // band max within the display
           (minDisplay>=result.min()&&maxDisplay<=result.max())) { // display within a band
            break;
        }
    }
//    qDebug() << "gvj value of result is min, max ... " << result.min() << "," << result.max();
    return result;

}

void Band::setFrequency(long long f) {  //Called by UI::frequencyChanged(long long frequency)
    BandLimit band;
    int newBand, newStack;

    for(newBand=0;newBand<limits.size();newBand++) {      //Check to see what band this frequency lies in
        band=limits.at(newBand);
        if((band.min()<=f)&&(band.max()>=f)) {; // then frequency is within this band
            break;
        }
    }
    if(newBand == limits.size()) { //frequency not found within any band so it is "GEN"
        newBand = BAND_GEN;
    }

//    qDebug() << "In setFrequency(), the value of newBand, currentBand & f = " << newBand <<", " << currentBand << ", " << f;

    if(currentBand!=newBand) {   //True if we changed band so setup new band
        newStack = bandstack[newBand][0].getInfo() + 1;
        bandstack[newBand][newStack].setFrequency(f);
        bandstack[newBand][newStack].setMode(getMode());
        bandstack[newBand][newStack].setFilter(getFilter());
        bandstack[newBand][newStack].setSpectrumHigh(getSpectrumHigh());
        bandstack[newBand][newStack].setSpectrumLow(getSpectrumLow());
        bandstack[newBand][newStack].setWaterfallHigh(getWaterfallHigh());
        bandstack[newBand][newStack].setWaterfallLow(getWaterfallLow());
        selectBand(newBand);
    } else {
        bandstack[currentBand][workingStack].setFrequency(f);
    }
}

void Band::setMode(int m) {
    bandstack[currentBand][workingStack].setMode(m);
}

void Band::setFilter(int f) {
    bandstack[currentBand][workingStack].setFilter(f);
}

void Band::setSpectrumHigh(int h) {
    bandstack[currentBand][workingStack].setSpectrumHigh(h);
}

void Band::setSpectrumLow(int l) {
    bandstack[currentBand][workingStack].setSpectrumLow(l);
}

void Band::setWaterfallHigh(int h) {
    bandstack[currentBand][workingStack].setWaterfallHigh(h);
}

void Band::setWaterfallLow(int l) {
    bandstack[currentBand][workingStack].setWaterfallLow(l);
}

//** Checks a band change signal for either a switch to a new band or traverse the quick memory. **/
//Called by VFO band button click or main menu band selection which will pass band or band button ID.
//Also called by a frequency change which takes us out of currend band. e.g Keypad or VFO frequency change.
void Band::selectBand(int b) {
    int previousBand=currentBand;
    int stackPtr = 0;
    bool vfoFlag = false;

    if(b>99){ //If band selected from main menu, 100 was added to the band number as a flag.
        b = b-100;
        vfoFlag = true;
    }
    currentBand=b;
    currentStack = bandstack[currentBand][1].getInfo();
    workingStack = bandstack[currentBand][0].getInfo()+1;

    qDebug()<<Q_FUNC_INFO<<": previousBand = "<<previousBand<<", currentBand = "<< currentBand<<", currentStack = "<<currentStack<<", stackPtr = " << stackPtr<<BAND_WWV;

    if(previousBand==currentBand) { //We are going to traverse the quick memory for the current band
        if(!vfoFlag && currentBand!=BAND_WWV) { //Only if this was called from the main menu we are going to store the current vfo display
            memCopy(currentStack, true); //copy the working memory to the current stack position and retrieve next memory
        }
        // step through band stack
        currentStack++;

        if(currentBand==BAND_WWV){
            if(currentStack > bandstack[currentBand][0].getInfo()+1) {
                currentStack = 0;
            }
            workingStack = currentStack;
            bandstack[currentBand][1].setInfo(currentStack);    //Set memory browse pointer to match stored memory position
        } else if(currentStack > bandstack[currentBand][0].getInfo()) { //Number of memories for this band, 0 .. n
            currentStack = 0;
        }

        bandstack[currentBand][1].setInfo(currentStack);
//        stack[currentBand]=currentStack;
        if(currentBand!=BAND_WWV) {
            memCopy(currentStack, false);   //Copy data from memory to working stack
        }
        qDebug()<<Q_FUNC_INFO<<":973    currentBand, currentStack, workingStack = "<<currentBand<<", "<<currentStack<<", "<<workingStack;
    } else {
        // Stepping to a new band
        if(currentBand==BAND_WWV) {
            workingStack = currentStack;
        }
        vfoFlag = false;
    }

qDebug()<<Q_FUNC_INFO<<"currentBand = "<<currentBand<<", currentStack = "<<currentStack<<", workingStack = "<<workingStack<<", f="<< bandstack[currentBand][workingStack].getFrequency();

    emit bandChanged(previousBand,currentBand);
    emit printStatusBar(" ... Using memory");
}

void Band::quickMemStore()
{
    int storePtr;

    if(currentBand!=BAND_WWV) {
        storePtr = bandstack[currentBand][2].getInfo(); //Point to memory write position for this band
        storePtr--;         // Steps backwards to always point to oldest entry
        if(storePtr < 0) {  //Store position for this band, 0 .. n
            storePtr = bandstack[currentBand][0].getInfo();
        }
        //Check to see if the frequency, mode and filter are already stored and use this if so.
        for(int x=0;x<=bandstack[currentBand][0].getInfo();x++){
            if(bandstack[currentBand][x].getFrequency()==bandstack[currentBand][workingStack].getFrequency() &&
                        bandstack[currentBand][x].getMode()==bandstack[currentBand][workingStack].getMode() &&
                        bandstack[currentBand][x].getFilter()==bandstack[currentBand][workingStack].getFilter()) {
                storePtr = x;
                qDebug()<<Q_FUNC_INFO<<":    Identical frequency, mode and filter is already stored";
            }
        }
        bandstack[currentBand][2].setInfo(storePtr);    //Save the position we stored at.
//        bandstack[currentBand][1].setInfo(storePtr);    //Set memory browse pointer to match stored memory position
        currentStack = bandstack[currentBand][1].getInfo();

//        qDebug()<<Q_FUNC_INFO<<"Got to QuickMemStore and storePtr value is ... "<<storePtr;
        memCopy(storePtr, true);    //copy the working stack data to selected memory.
        emit printStatusBar(" ... Stored at "+QString::number(storePtr));
    }
}

//direction true: copy working info to memory info and vice versa for false
//target = the memory store location
void Band::memCopy(int target, bool direction)
{
    int source;
    int dest;

//    current = bandstack[currentBand][0].getInfo() + 1;  //index to working stack
    if(direction) {
        source = workingStack;
        dest = target;  //Memory store location
    } else {
        source = target;
        dest = workingStack;
    }

//    qDebug()<<Q_FUNC_INFO<<":  target = "<<target<<", source = "<<source<<", dest = "<<dest<<", direction = "<<direction;

    bandstack[currentBand][dest].setFrequency(bandstack[currentBand][source].getFrequency());
    bandstack[currentBand][dest].setMode(bandstack[currentBand][source].getMode());
    bandstack[currentBand][dest].setFilter(bandstack[currentBand][source].getFilter());
    bandstack[currentBand][dest].setSpectrumHigh(bandstack[currentBand][source].getSpectrumHigh());
    bandstack[currentBand][dest].setSpectrumLow(bandstack[currentBand][source].getSpectrumLow());
    bandstack[currentBand][dest].setWaterfallHigh(bandstack[currentBand][source].getWaterfallHigh());
    bandstack[currentBand][dest].setWaterfallLow(bandstack[currentBand][source].getWaterfallLow());
}


