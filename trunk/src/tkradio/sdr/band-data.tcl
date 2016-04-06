# -*- mode: Tcl; tab-width: 8; -*-
#
# Copyright (C) 2011, 2012 by Roger E Critchlow Jr, Santa Fe, NM, USA.
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

package require snit
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
    add-band Amateur {4cm} low 75.5GHz high 81.0GHz mode NFM
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

snit::type sdr::bands {
    method get {args} { return [::sdr::band-data-get {*}$args] }
    method ranges {} { return [::sdr::band-data-ranges] }
    method range {range} { return [::sdr::band-data-range $range] }
    method services {} { return [::sdr::band-data-services] }
    method nrows {} { return [::sdr::band-data-nrows] }
    method row {service} { return [::sdr::band-data-row $service] }
    method service {service} { return [::sdr::band-data-service $service] }
    method bands {service} { return [::sdr::band-data-bands $service] }
    method channels {service} { return [::sdr::band-data-channels $service] }
    method color {service} { return [::sdr::band-data-color $service] }
    method band {service band} { return [::sdr::band-data-band $service $band] }
    method band-range {service band} { return [::sdr::band-data-band-range $service $band] }
    method band-range-hertz {service band} { return [::sdr::band-data-band-range-hertz $service $band] }
    method channel {service channel} { return [::sdr::band-data-channel $service $channel] }
    method channel-freq {service channel} { return [::sdr::band-data-channel-freq $service $channel] }
    method channel-freq-hertz {service channel} { return [::sdr::band-data-channel-freq-hertz $service $channel] }
}
