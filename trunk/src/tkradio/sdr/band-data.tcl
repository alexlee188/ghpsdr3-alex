# -*- mode: Tcl; tab-width: 8; -*-
#
# Copyright (C) 2011, 2012 by Roger E Critchlow Jr, Santa Fe, NM, USA.
# Copyright (C) 2016 by Roger E Critchlow Jr, Cambridge, MA, USA.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
# 

#
# a band and channel database
# should be specialized to ITU region for amateur band limits
# should merge in personalized channel collections
# should show band plans for amateur bands
#

package provide sdr::band-data 1.0

package require sdr::util

namespace eval ::sdr {}
namespace eval ::sdr::band-data {
    ##
    ## create the dictionary
    ##
    set data [dict create]

    ##
    ## specify the spectrum ranges
    ##
    dict set data ranges [dict create {*}{
	LF {30kHz 300kHz}
	MF {300kHz 3Mhz}
	HF {1.8MHz 30MHz}
	VHF {30MHz 300MHz}
	UHF {300MHz 3GHz}
	SHF {3GHz 30GHz}
	EHF {30GHz 300GHz}
    }]

    ##
    ## initialize the services
    ##
    dict set data services {}

    ##
    ## add service to spectrum database
    ## bands are organized into services
    ##
    proc add-service {service args} {
	variable data 
	if {[lsearch [dict get $data services] $service] >= 0} {
	    error "service \"$service\" is already defined"
	}
	foreach {name value} $args {
	    # check service values
	}
	dict lappend data services $service
	dict set data $service [dict create {*}$args]
	foreach item {color bands channels} default {white {} {}} {
	    if { ! [dict exists $data $service $item]} {
		dict set data $service $item $default
	    }
	}
    }

    ##
    ## add band to service
    ##
    proc add-band {service band args} {
	variable data 
	if {[lsearch [dict get $data $service bands] $band] >= 0} {
	    error "band \"$band\" is already defined for service \"$service\""
	}
	foreach {name value} $args {
	    switch $name {
		filter - channel-step - low - high { sdr::hertz $value }
		note - name - mode {}
		default {
		    error "unknown band $name = {$value}"
		}
	    }
	}
	dict set data $service bands [concat [dict get $data $service bands] [list $band]]
	dict set data $service $band [dict create {*}$args]
    }

    ##
    ## add channel to service/band
    ##
    proc add-channel {service channel args} {
	variable data 
	if {[lsearch [dict get $data $service channels] $channel] >= 0} {
	    error "channel \"$channel\" is already defined for service \"$service\""
	}
	foreach {name value} $args {
	    switch $name {
		note - name - mode - codec {}
		filter - freq { sdr::hertz $value }
		default {
		    error "unknown channel $name = {$value}"
		}
	    }
	}
	dict set data $service channels [concat [dict get $data $service channels] [list $channel]]
	dict set data $service $channel [dict create {*}$args]
    }
    
    ##
    ## Aviation bands
    ##
    add-service Aviation color {light blue} row 1
    foreach {low high} {
	2.850 3.155
	3.400 3.500
	4.650 4.750
	5.450 5.730
	6.525 6.765
	8.815 9.040
	10.050 10.100
	11.175 11.400
	13.200 13.360
	15.010 15.100
	17.900 18.030
	21.924 22.000
	23.200 23.350
    } {
	add-band Aviation $low low ${low}MHz high ${high}MHz mode SSB channel-step 3kHz
    }
    add-band Aviation VHF-Nav low 108MHz high 117.975MHz mode AM
    add-band Aviation VHF-AM low 118MHz high 137MHz mode AM
    add-channel Aviation Distress freq 121.5MHz

    ##
    ## Marine mobile bands
    ##
    add-service Marine color blue row 1
    foreach {low high} {
	2.045 2.160
	2.170 2.194
	4.000 4.438
	6.200 6.525
	8.100 8.815
	12.230 13.200
	16.360 17.410
	18.780 18.900
	19.680 19.800
	22.000 22.855
	25.070 25.210
	26.100 26.175
    } {
	add-band Marine $low low ${low}MHz high ${high}MHz mode USB
    }
    foreach freq {2.182 4.125 6.215 8.291 12.290 16.420} {
	add-channel Marine "Distress $freq" freq ${freq}MHz
    }
    foreach freq {
	4.146 4.149
	6.224 6.227 6.230
	8.294 8.297
	12.353 12.356 12.359 12.362 12.365
	16.528 16.531 16.534 16.537 16.540 16.543 16.546
	18.825 18.828 18.831 18.834 18.837 18.840 18.843
	22.159 22.162 22.165 22.168 22.171 22.174 22.177
	25.100 25.103 25.106 25.109 25.112 25.115 25.118
    } {
	add-channel Marine "Simplex $freq" freq ${freq}MHz mode USB
    }
    add-band Marine VHF-A low 156.000MHz high 157.425MHz mode NFM channel-step 25000
    add-band Marine VHF-B low 160.600MHz high 162.025MHz mode NFM channel-step 25000
    add-channel Marine 9A freq 156.450MHz mode NFM
    add-channel Marine 13A freq 156.650MHz mode NFM
    add-channel Marine 16A freq 156.800MHz mode NFM
    add-channel Marine 70A freq 156.525MHz note {Digital Selective Calling}
    add-channel Marine 87B freq 161.975MHz mode GMSK note {Automatic Identification System}
    add-channel Marine 88B freq 162.025MHz mode GMSK note {Automatic Identification System}

    ##
    ## Broadcast radio bands
    ##
    add-service Broadcast color green row 2
    add-band Broadcast {LW} low 148.5kHz high 283.5kHz mode AM channel-step 10000
    add-band Broadcast {MW} low 520kHz high 1710kHz mode AM channel-step 10000
    add-band Broadcast {120m} low 2300kHz high 2495kHz mode AM channel-step 5000
    add-band Broadcast {90m} low 3200kHz high 3400kHz mode AM channel-step 5000
    add-band Broadcast {75m} low 3900kHz high 4000kHz mode AM channel-step 5000
    add-band Broadcast {60m} low 4750kHz high 5060kHz mode AM channel-step 5000
    add-band Broadcast {49m} low 5900kHz high 6200kHz mode AM channel-step 5000
    add-band Broadcast {41m} low 7200kHz high 7450kHz mode AM channel-step 5000
    add-band Broadcast {31m} low 9400kHz high 9900kHz mode AM channel-step 5000
    add-band Broadcast {25m} low 11600kHz high 12100kHz mode AM channel-step 5000
    add-band Broadcast {22m} low 13570kHz high 13870kHz mode AM channel-step 5000
    add-band Broadcast {19m} low 15100kHz high 15800kHz mode AM channel-step 5000
    add-band Broadcast {16m} low 17480kHz high 17900kHz mode AM channel-step 5000
    add-band Broadcast {15m} low 18900kHz high 19020kHz mode AM channel-step 5000
    add-band Broadcast {13m} low 21450kHz high 21850kHz mode AM channel-step 5000
    add-band Broadcast {11m} low 25600kHz high 26100kHz mode AM channel-step 5000
    add-band Broadcast {TV VHF low} low 54MHz high 88MHz mode TV channel-step 6MHz
    add-band Broadcast {FM} low 87.5MHz high 108.0MHz mode WFM
    add-band Broadcast {TV VHF high} low 174MHz high 216MHz mode TV channel-step 6MHz
    add-band Broadcast {TV UHF} low 470MHz high 698MHz mode TV channel-step 6MHz

    ##
    ## Weather channels
    ##
    add-service Weather color grey row 3
    add-band Weather NOAA name {NOAA Weather} low 162.400MHz high 162.550MHz mode NFM channel-step 25000
    add-channel Weather WX1 freq 162.550MHz
    add-channel Weather WX2 freq 162.400MHz
    add-channel Weather WX3 freq 162.475MHz
    add-channel Weather WX4 freq 162.425MHz
    add-channel Weather WX5 freq 162.450MHz
    add-channel Weather WX6 freq 162.500MHz
    add-channel Weather WX7 freq 162.525MHz
    # alt {1=162.400, 2=162.425, 3=162.450, 4=162.475, 5=162.500, 6=162.525, 7=162.550 {in order of frequency channel numbering}}

    ##
    ## Hospital emergency room, probably US specific
    ##
    add-service Hospital color red row 3
    add-channel Hospital HERN freq 155.34MHz

    ##
    ## Amateur bands
    ##
    add-service Amateur color gold row 4
    add-band Amateur 136kHz low 1357kHz high 1378kHz
    add-band Amateur {160m} low 1.8MHz high 2MHz
    add-band Amateur {80m} low 3.5MHz high 3.6MHz
    add-band Amateur {75m} low 3.6MHz high 4MHz
    add-band Amateur {60m} low 5.2585MHz high 5.4035MHz
    add-band Amateur {40m} low 7.0MHz high 7.3MHz
    add-band Amateur {30m} low 10.1MHz high 10.15MHz
    add-band Amateur {20m} low 14MHz high 14.35MHz
    add-band Amateur {17m} low 18.068MHz high 18.168MHz
    add-band Amateur {15m} low 21MHz high 21.45MHz
    add-band Amateur {12m} low 24.89MHz high 25.99MHz
    add-band Amateur {11m} low 26MHz high 27MHz
    add-band Amateur {10m} low 28MHz high 29.7MHz
    add-band Amateur {6m} low 50MHz high 54MHz mode NFM
    add-band Amateur {4m} low 70MHz high 70.5MHz mode NFM
    add-band Amateur {2m} low 144MHz high 148MHz mode NFM
    # add-band Amateur {1.25m} low 219MHz high 220MHz mode NFM
    add-band Amateur {1.25m} low 222MHz high 225MHz mode NFM
    add-band Amateur {70cm} low 420MHz high 450MHz mode NFM
    add-band Amateur {33cm} low 902MHz high 928MHz mode NFM
    add-band Amateur {23cm} low 1.24GHz high 1.3GHz mode NFM
    add-band Amateur {13cm} low 2.3GHz high 2.31GHz mode NFM
    add-band Amateur {9cm} low 3.3GHz high 3.5GHz mode NFM
    add-band Amateur {5cm} low 5.65GHz high 5.925GHz mode NFM
    add-band Amateur {3cm} low 10GHz high 10.5GHz mode NFM
    add-band Amateur {1.2cm} low 24GHz high 24.25GHz mode NFM
    add-band Amateur {6mm} low 47GHz high 47.2GHz mode NFM
    add-band Amateur {4mm} low 75.5GHz high 81.0GHz mode NFM
    add-band Amateur {2.5mm} low 119.98GHz high 120.02GHz mode NFM
    add-band Amateur {2mm} low 142GHz high 149GHz mode NFM
    add-band Amateur {1mm} low 241GHz high 250GHz mode NFM
    
    # wspr channels, dial frequency usb
    add-channel Amateur wspr600m freq 0.5024MHz mode DIGU codec WSPR
    add-channel Amateur wspr160m freq 1.8366MHz mode DIGU codec WSPR
    add-channel Amateur wspr80m freq 3.5926MHz mode DIGU codec WSPR
    add-channel Amateur wspr60m freq 5.2872MHz mode DIGU codec WSPR
    add-channel Amateur wspr40m freq 7.0386MHz mode DIGU codec WSPR
    add-channel Amateur wspr30m freq 10.1387MHz mode DIGU codec WSPR
    add-channel Amateur wspr20m freq 14.0956MHz mode DIGU codec WSPR
    add-channel Amateur wspr17m freq 18.1046MHz mode DIGU codec WSPR
    add-channel Amateur wspr15m freq 21.0946MHz mode DIGU codec WSPR
    add-channel Amateur wspr12m freq 24.9246MHz mode DIGU codec WSPR
    add-channel Amateur wspr10m freq 28.1246MHz mode DIGU codec WSPR
    add-channel Amateur wspr6m freq 50.2930MHz mode DIGU  codec WSPR
    add-channel Amateur wspr4m freq 70.0286MHz mode DIGU codec WSPR
    add-channel Amateur wspr2m freq 144.4890MHz mode DIGU codec WSPR

    ##
    ## WWV/CHU/HD2I0A/et al standard time channels
    ##
    add-service Time color {red} row 3
    add-channel Time 2.5 freq 2.5MHz mode AM
    add-channel Time 3.33 freq 3.33MHz mode AM
    add-channel Time 3.81 freq 3.81MHz mode AM
    add-channel Time 4 freq 4MHz mode AM
    add-channel Time 5 freq 5MHz mode AM
    add-channel Time 7.85 freq 7.85MHz mode AM
    add-channel Time 8 freq 8MHz mode AM
    add-channel Time 10 freq 10MHz mode AM
    add-channel Time 14.67 freq 14.67MHz mode AM
    add-channel Time 15 freq 15MHz mode AM
    add-channel Time 16 freq 16MHz mode AM
    add-channel Time 20 freq 20MHz mode AM

    ##
    ## Satellite services
    ## found the NOAA, etc listed with status as of 26/3/2012
    ## at http://www.unkebe.com/APT_Status_Report.htm
    ## this is also good http://tech.groups.yahoo.com/group/weather-satellite-reports
    ##
    add-service Satellite color {green} row 3
    add-channel Satellite GPS/L1 freq 1575.42MHz
    add-channel Satellite GPS/L2 freq 1227.60MHz
    add-channel Satellite GPS/L3 freq 1381.05MHz
    add-channel Satellite GPS/L4 freq 1379.913MHz
    add-channel Satellite GPS/L5 freq 1176.45MHz
    # polar orbiting weather satellites
    # APT is AM fax, high gain antenna
    # HRPT etc is digital, dish antenna
    add-channel Satellite NOAA/15/APT freq 137.620MHz mode APT
    add-channel Satellite NOAA/17/APT freq 137.500MHz mode APT note {No images}
    add-channel Satellite NOAA/18/APT freq 137.9125MHz mode APT
    add-channel Satellite NOAA/19/APT freq 137.10MHz mode APT
    add-channel Satellite Metop-A/LRPT freq 137.100MHz mode LRPT note off
    add-channel Satellite Meteor-M-N1/LRPT freq 137.100MHz mode LRPT note sporadic

    add-channel Satellite NOAA/15/HRPT freq 1702.5MHz mode HRPT note weak
    add-channel Satellite NOAA/16/HRPT freq 1698.0MHz mode HRPT
    add-channel Satellite NOAA/18/HRPT freq 1707.0MHz mode HRPT
    add-channel Satellite NOAA/19/HRPT freq 1698.0MHz mode HRPT
    add-channel Satellite FengYun1D freq 1700.4MHz mode CHRPT
    add-channel Satellite FengYun3A freq 1704.5MHz
    add-channel Satellite FengYun3B freq 1704.5MHz
    add-channel Satellite Metop-A freq 1701.3MHz mode AHRPT
    add-channel Satellite Meteor-M-N1 freq 1700MHz
    # geosynchronous orbiting weather satellites
    # maybe some other day
}

proc sdr::band-data-get {args} { return [dict get ${::sdr::band-data::data} {*}$args] }
proc sdr::band-data-ranges {} { return [band-data-get ranges] }
proc sdr::band-data-range {range} { return [band-data-get ranges $range] }
proc sdr::band-data-range-hertz {range} { return [map hertz [band-data-range $range]] }
proc sdr::band-data-services {} { return [band-data-get services] }
proc sdr::band-data-nrows {} { return [llength [lsort -unique [map band-data-row [band-data-services]]]] }
proc sdr::band-data-row {service} { return [band-data-get $service row] }
proc sdr::band-data-service {service} { return [band-data-get $service] }
proc sdr::band-data-bands {service} { return [band-data-get $service bands] }
proc sdr::band-data-all-bands {} { return [concat {*}[lmap s [band-data-services] {band-data-get $s bands}]] }
proc sdr::band-data-channels {service} { return [band-data-get $service channels] }
proc sdr::band-data-color {service} { return [band-data-get $service color] }
proc sdr::band-data-band {service band} { return [band-data-get $service $band] }
proc sdr::band-data-band-range {service band} { return [list [band-data-get $service $band low] [band-data-get $service $band high]] }
proc sdr::band-data-band-range-hertz {service band} { return [map hertz [band-data-band-range $service $band]] }
proc sdr::band-data-channel {service channel} { return [band-data-get $service $channel] }
proc sdr::band-data-channel-freq {service channel} { return [band-data-get $service $channel freq] }
proc sdr::band-data-channel-freq-hertz {service channel} { return [hertz [band-data-channel-freq $service $channel]] }

if {0} {
    ## okay, min Hz, max Hz, description, band, can transmit
    ## from the QtRadio Frequency.cpp source
    ## dropped the band names and can transmit
    foreach {low high description} {
    60000 60000 "MSF Time Signal"
    75000 75000 "HGB Time Signal"
    77500 77500 "DCF77 Time Signal"
    153000 279000 "Broadcast AM Long Wave"
    530000 1710000 "Broadcast AM Med Wave"
    1800000 1809999 "160M CW/Digital Modes"
    1810000 1810000 "160M CW QRP"
    1810001 1842999 "160M CW"
    1843000 1909999 "160M SSB/SSTV/Wide band"
    1910000 1910000 "160M SSB QRP"
    1910001 1994999 "160M SSB/SSTV/Wide band"
    1995000 1999999 "160M Experimental"

    2300000 2495000 "120M Short Wave"

    2500000 2500000 "WWV"

    3200000 3400000 "90M Short Wave"

    3500000 3524999 "80M Extra CW"
    3525000 3579999 "80M CW"
    3580000 3589999 "80M RTTY"
    3590000 3590000 "80M RTTY DX"
    3590001 3599999 "80M RTTY"
    3600000 3699999 "75M Extra SSB"
    3700000 3789999 "75M Ext/Adv SSB"
    3790000 3799999 "75M Ext/Adv DX Window"
    3800000 3844999 "75M SSB"
    3845000 3845000 "75M SSTV"
    3845001 3884999 "75M SSB"
    3885000 3885000 "75M AM Calling Frequency"
    3885001 3999999 "75M SSB"
    4750000 4999999 "60M Short Wave"

    5000000 5000000 "WWV"

    5330500 5330500 "60M Channel 1"
    5346500 5346500 "60M Channel 2"
    5366500 5366500 "60M Channel 3"
    5371500 5371500 "60M Channel 4"
    5403500 5403500 "60M Channel 5"

    5900000 6200000 "49M Short Wave"

    7000000 7024999 "40M Extra CW"
    7025000 7039999 "40M CW"
    7040000 7040000 "40M RTTY DX"
    7040001 7099999 "40M RTTY"
    7100000 7124999 "40M CW"
    7125000 7170999 "40M Ext/Adv SSB"
    7171000 7171000 "40M SSTV"
    7171001 7174999 "40M Ext/Adv SSB"
    7175000 7289999 "40M SSB"
    7290000 7290000 "40M AM Calling Frequency"
    7290001 7299999 "40M SSB"

    7300000 7350000 "41M Short Wave"
    9400000 9900000 "31M Short Wave"

    10000000 10000000 "WWV"

    10100000 10129999 "30M CW"
    10130000 10139999 "30M RTTY"
    10140000 10149999 "30M Packet"

    11600000 12100000 "25M Short Wave"
    13570000 13870000 "22M Short Wave"

    14000000 14024999 "20M Extra CW"
    14025000 14069999 "20M CW"
    14070000 14094999 "20M RTTY"
    14095000 14099499 "20M Packet"
    14099500 14099999 "20M CW"
    14100000 14100000 "20M NCDXF Beacons"
    14100001 14100499 "20M CW"
    14100500 14111999 "20M Packet"
    14112000 14149999 "20M CW"
    14150000 14174999 "20M Extra SSB"
    14175000 14224999 "20M Ext/Adv SSB"
    14225000 14229999 "20M SSB"
    14230000 14230000 "20M SSTV"
    14230000 14285999 "20M SSB"
    14286000 14286000 "20M AM Calling Frequency"
    14286001 14349999 "20M SSB"

    15000000 15000000 "WWV"

    15100000 15800000 "19M Short Wave"
    17480000 17900000 "16M Short Wave"

    18068000 18099999 "17M CW"
    18100000 18104999 "17M RTTY"
    18105000 18109999 "17M Packet"
    18110000 18110000 "17M NCDXF Beacons"
    18110001 18167999 "17M SSB"

    18900000 19020000 "15M Short Wave"

    20000000 20000000 "WWV"

    21000000 21024999 "15M Extra CW"
    21025000 21069999 "15M CW"
    21070000 21099999 "15M RTTY"
    21100000 21109999 "15M Packet"
    21110000 21149999 "15M CW"
    21150000 21150000 "15M NCDXF Beacons"
    21150001 21199999 "15M CW"
    21200000 21224999 "15M Extra SSB"
    21225000 21274999 "15M Ext/Adv SSB"
    21275000 21339999 "15M SSB"
    21340000 21340000 "15M SSTV"
    21340001 21449999 "15M SSB"

    21450000 21850000 "13M Short Wave"

    24890000 24919999 "12M CW"
    24920000 24924999 "12M RTTY"
    24925000 24929999 "12M Packet"
    24930000 24930000 "12M NCDXF Beacons"

    25600000 26100000 "11M Short Wave"

    28000000 28069999 "10M CW"
    28070000 28149999 "10M RTTY"
    28150000 28199999 "10M CW"
    28200000 28200000 "10M NCDXF Beacons"
    28200001 28299999 "10M Beacons"
    28300000 28679999 "10M SSB"
    28680000 28680000 "10M SSTV"
    28680001 28999999 "10M SSB"
    29000000 29199999 "10M AM"
    29200000 29299999 "10M SSB"
    29300000 29509999 "10M Satellite Downlinks"
    29510000 29519999 "10M DeadBAND_"
    29520000 29589999 "10M Repeater Inputs"
    29590000 29599999 "10M DeadBAND_"
    29600000 29600000 "10M FM Simplex"
    29600001 29609999 "10M DeadBAND_"
    29610000 29699999 "10M Repeater Outputs"

    50000000 50059999 "6M CW"
    50060000 50079999 "6M Beacon Sub-BAND_"
    50080000 50099999 "6M CW"
    50100000 50124999 "6M DX Window"
    50125000 50125000 "6M Calling Frequency"
    50125001 50299999 "6M SSB"
    50300000 50599999 "6M All Modes"
    50600000 50619999 "6M Non Voice"
    50620000 50620000 "6M Digital Packet Calling"
    50620001 50799999 "6M Non Voice"
    50800000 50999999 "6M RC"
    51000000 51099999 "6M Pacific DX Window"
    51100000 51119999 "6M DeadBAND_"
    51120000 51179999 "6M Digital Repeater Inputs"
    51180000 51479999 "6M Repeater Inputs"
    51480000 51619999 "6M DeadBAND_"
    51620000 51679999 "6M Digital Repeater Outputs"
    51680000 51979999 "6M Repeater Outputs"
    51980000 51999999 "6M DeadBAND_"
    52000000 52019999 "6M Repeater Inputs"
    52020000 52020000 "6M FM Simplex"
    52020001 52039999 "6M Repeater Inputs"
    52040000 52040000 "6M FM Simplex"
    52040001 52479999 "6M Repeater Inputs"
    52480000 52499999 "6M DeadBAND_"
    52500000 52524999 "6M Repeater Outputs"
    52525000 52525000 "6M Primary FM Simplex"
    52525001 52539999 "6M DeadBAND_"
    52540000 52540000 "6M Secondary FM Simplex"
    52540001 52979999 "6M Repeater Outputs"
    52980000 52999999 "6M DeadBAND_s"
    53000000 53000000 "6M Remote Base FM Spx"
    53000001 53019999 "6M Repeater Inputs"
    53020000 53020000 "6M FM Simplex"
    53020001 53479999 "6M Repeater Inputs"
    53480000 53499999 "6M DeadBAND_"
    53500000 53519999 "6M Repeater Outputs"
    53520000 53520000 "6M FM Simplex"
    53520001 53899999 "6M Repeater Outputs"
    53900000 53900000 "6M FM Simplex"
    53900010 53979999 "6M Repeater Outputs"
    53980000 53999999 "6M DeadBAND_"

    144000000 144099999 "2M CW"
    144100000 144199999 "2M CW/SSB"
    144200000 144200000 "2M Calling"
    144200001 144274999 "2M CW/SSB"
    144275000 144299999 "2M Beacon Sub-BAND_"
    144300000 144499999 "2M Satellite"
    144500000 144599999 "2M Linear Translator Inputs"
    144600000 144899999 "2M FM Repeater"
    144900000 145199999 "2M FM Simplex"
    145200000 145499999 "2M FM Repeater"
    145500000 145799999 "2M FM Simplex"
    145800000 145999999 "2M Satellite"
    146000000 146399999 "2M FM Repeater"
    146400000 146609999 "2M FM Simplex"
    146610000 147389999 "2M FM Repeater"
    147390000 147599999 "2M FM Simplex"
    147600000 147999999 "2M FM Repeater"

    222000000 222024999 "125M EME/Weak Signal"
    222025000 222049999 "125M Weak Signal"
    222050000 222059999 "125M Propagation Beacons"
    222060000 222099999 "125M Weak Signal"
    222100000 222100000 "125M SSB/CW Calling"
    222100001 222149999 "125M Weak Signal CW/SSB"
    222150000 222249999 "125M Local Option"
    222250000 223380000 "125M FM Repeater Inputs"
    222380001 223399999 "125M General"
    223400000 223519999 "125M FM Simplex"
    223520000 223639999 "125M Digital/Packet"
    223640000 223700000 "125M Links/Control"
    223700001 223709999 "125M General"
    223710000 223849999 "125M Local Option"
    223850000 224980000 "125M Repeater Outputs"

    420000000 425999999 "70CM ATV Repeater"
    426000000 431999999 "70CM ATV Simplex"
    432000000 432069999 "70CM EME"
    432070000 432099999 "70CM Weak Signal CW"
    432100000 432100000 "70CM Calling Frequency"
    432100001 432299999 "70CM Mixed Mode Weak Signal"
    432300000 432399999 "70CM Propagation Beacons"
    432400000 432999999 "70CM Mixed Mode Weak Signal"
    433000000 434999999 "70CM Auxillary/Repeater Links"
    435000000 437999999 "70CM Satellite Only"
    438000000 441999999 "70CM ATV Repeater"
    442000000 444999999 "70CM Local Repeaters"
    445000000 445999999 "70CM Local Option"
    446000000 446000000 "70CM Simplex"
    446000001 446999999 "70CM Local Option"
    447000000 450000000 "70CM Local Repeaters"


    902000000 902099999 "33CM Weak Signal SSTV/FAX/ACSSB"
    902100000 902100000 "33CM Weak Signal Calling"
    902100001 902799999 "33CM Weak Signal SSTV/FAX/ACSSB"
    902800000 902999999 "33CM Weak Signal EME/CW"
    903000000 903099999 "33CM Digital Modes"
    903100000 903100000 "33CM Alternate Calling"
    903100001 905999999 "33CM Digital Modes"
    906000000 908999999 "33CM FM Repeater Inputs"
    909000000 914999999 "33CM ATV"
    915000000 917999999 "33CM Digital Modes"
    918000000 920999999 "33CM FM Repeater Outputs"
    921000000 926999999 "33CM ATV"
    927000000 928000000 "33CM FM Simplex/Links"

    1240000000 1245999999 "23CM ATV #1"
    1246000000 1251999999 "23CM FMN Point/Links"
    1252000000 1257999999 "23CM ATV #2, Digital Modes"
    1258000000 1259999999 "23CM FMN Point/Links"
    1260000000 1269999999 "23CM Sat Uplinks/WideBAND_ Exp"
    1270000000 1275999999 "23CM Repeater Inputs"
    1276000000 1281999999 "23CM ATV #3"
    1282000000 1287999999 "23CM Repeater Outputs"
    1288000000 1293999999 "23CM Simplex ATV/WideBAND_ Exp"
    1294000000 1294499999 "23CM Simplex FMN"
    1294500000 1294500000 "23CM FM Simplex Calling"
    1294500001 1294999999 "23CM Simplex FMN"
    1295000000 1295799999 "23CM SSTV/FAX/ACSSB/Exp"
    1295800000 1295999999 "23CM EME/CW Expansion"
    1296000000 1296049999 "23CM EME Exclusive"
    1296050000 1296069999 "23CM Weak Signal"
    1296070000 1296079999 "23CM CW Beacons"
    1296080000 1296099999 "23CM Weak Signal"
    1296100000 1296100000 "23CM CW/SSB Calling"
    1296100001 1296399999 "23CM Weak Signal"
    1296400000 1296599999 "23CM X-BAND_ Translator Input"
    1296600000 1296799999 "23CM X-BAND_ Translator Output"
    1296800000 1296999999 "23CM Experimental Beacons"
    1297000000 1300000000 "23CM Digital Modes"

    2300000000 2302999999 "23GHz High Data Rate"
    2303000000 2303499999 "23GHz Packet"
    2303500000 2303800000 "23GHz TTY Packet"
    2303800001 2303899999 "23GHz General"
    2303900000 2303900000 "23GHz Packet/TTY/CW/EME"
    2303900001 2304099999 "23GHz CW/EME"
    2304100000 2304100000 "23GHz Calling Frequency"
    2304100001 2304199999 "23GHz CW/EME/SSB"
    2304200000 2304299999 "23GHz SSB/SSTV/FAX/Packet AM/Amtor"
    2304300000 2304319999 "23GHz Propagation Beacon Network"
    2304320000 2304399999 "23GHz General Propagation Beacons"
    2304400000 2304499999 "23GHz SSB/SSTV/ACSSB/FAX/Packet AM"
    2304500000 2304699999 "23GHz X-BAND_ Translator Input"
    2304700000 2304899999 "23GHz X-BAND_ Translator Output"
    2304900000 2304999999 "23GHz Experimental Beacons"
    2305000000 2305199999 "23GHz FM Simplex"
    2305200000 2305200000 "23GHz FM Simplex Calling"
    2305200001 2305999999 "23GHz FM Simplex"
    2306000000 2308999999 "23GHz FM Repeaters"
    2309000000 2310000000 "23GHz Control/Aux Links"
    2390000000 2395999999 "23GHz Fast-Scan TV"
    2396000000 2398999999 "23GHz High Rate Data"
    2399000000 2399499999 "23GHz Packet"
    2399500000 2399999999 "23GHz Control/Aux Links"
    2400000000 2402999999 "24GHz Satellite"
    2403000000 2407999999 "24GHz Satellite High-Rate Data"
    2408000000 2409999999 "24GHz Satellite"
    2410000000 2412999999 "24GHz FM Repeaters"
    2413000000 2417999999 "24GHz High-Rate Data"
    2418000000 2429999999 "24GHz Fast-Scan TV"
    2430000000 2432999999 "24GHz Satellite"
    2433000000 2437999999 "24GHz Sat High-Rate Data"
    2438000000 2450000000 "24GHz WideBAND_ FM/FSTV/FMTV"

    3456000000 3456099999 "3.4GHz General"
    3456100000 3456100000 "3.4GHz Calling Frequency"
    3456100001 3456299999 "3.4GHz General"
    3456300000 3456400000 "3.4GHz Propagation Beacons"

    5760000000 5760099999 "5.7GHz General"
    5760100000 5760100000 "5.7GHz Calling Frequency"
    5760100001 5760299999 "5.7GHz General"
    5760300000 5760400000 "5.7GHz Propagation Beacons"

    10368000000 10368099999 "10GHz General"
    10368100000 10368100000 "10GHz Calling Frequency"
    10368100001 10368400000 "10GHz General"

    24192000000 24192099999 "24GHz General"
    24192100000 24192100000 "24GHz Calling Frequency"
    24192100001 24192400000 "24GHz General"

    47088000000 47088099999 "47GHz General"
    47088100000 47088100000 "47GHz Calling Frequency"
    47088100001 47088400000 "47GHz General"
    } {
	
    }
}
