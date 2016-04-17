# -*- mode: Tcl; tab-width: 8; -*-
#
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
# a hardware module for the SDRplay
#
package provide sdr::radio-sdrplay 1.0

package require Tcl
package require snit

package require sdr::util

namespace eval ::sdr {}

snit::type sdr::radio-sdrplay {
    option -parent -readonly true -configuremethod Configure
    option -gain -default 51 -configuremethod Configure
    option -gain-reduction -default 51 -readonly true
    option -gain-distribution -readonly true
    option -gain-min -default 0 -readonly true
    option -gain-max -default 102 -readonly true
    
    # the low and high frequencies tuned
    # foreach band of frequencies
    #	the max gain in dB according to some library
    #	the overall range of gain reduction available
    #	the ranges of baseband, lna, and mixer gain
    #		in the standard 
    #
    # Okay, the only variable in the gain distribution
    # is when the LNA gets turned off.
    # We start at 0 gain reduction and reduce the baseband gain
    # by steps of one until the LNA turns off, at which point
    # the baseband reduction jumps up to compensate for the
    # missing LNA gain.  Then the the mixer gets
    # turned off when the baseband reduction reaches
    # its limit
    variable caps {
	range {100000 .. 2000000000} 
	bands {
	    {100kHz .. 60MHz} {
		dB-max 98 reduction {0 .. 102} baseband {0 .. 59 by 1} lna {0 .. 24 by 24 at 35} mixer {0 .. 19 by 19 at 83} 
	    }
	    {60MHz .. 120MHz} {
		dB-max 103 reduction {0 .. 102} baseband {0 .. 59 by 1} lna {0 .. 24 by 24 at 29} mixer {0 .. 19 by 19 at 83}
	    }
	    {120MHz .. 250MHz} {
		dB-max 107 reduction {0 .. 102} baseband {0 .. 59 by 1} lna {0 .. 24 by 24 at 29} mixer {0 .. 19 by 19 at 83}
	    }
	    {250MHz .. 420MHz} {
		dB-max 94 reduction {0 .. 102} baseband {0 .. 59 by 1} lna {0 .. 24 by 24 at 35} mixer {0 .. 19 by 19 at 83}
	    }
	    {420MHz .. 1GHz} {
		dB-max 94 reduction {0 .. 85} baseband {0 .. 59 by 1} lna {0 .. 7 by 7 at 12} mixer {0 .. 19 by 19 at 66}
	    }
	    {1GHz .. 2GHz} {
		dB-max 105 reduction {0 .. 85} baseband {0 .. 59 by 1} lna {0 .. 7 by 7 at 10} mixer {0 .. 19 by 19 at 66}
	    }
	}
    }

    component parent

    constructor {args} {
	set caps [dict create {*}$caps]
	dict set caps bands [dict create {*}[dict get $caps bands]]
	dict for {band params} [dict get $caps bands] {
	    dict set caps bands $band [dict create band $band band-range [sdr::parse-range-hertz $band] {*}[dict get $caps bands $band]]
	}
	$self configure {*}$args
    }
    method {Configure -parent} {value} { 
	set options(-parent) $value
	set parent $value
    }
    method {Configure -gain-reduction} {dB} {
	set options(-gain-reduction) $dB
	$self compute-from -gain-reduction
	$parent.command sdrplay*setattenuator $options(-gain-reduction)
    }
    method {Configure -gain} {dB} {
	set options(-gain) $dB
	$self compute-from -gain
	$parent.command sdrplay*setattenuator $options(-gain-reduction)
    }
    method get-caps {} {
	return $caps
    }
    method find-band {f} {
	dict for {band params} [dict get $caps bands] {
	    if {[sdr::in-range-list {*}[dict get $params band-range] $f]} {
		return $params
	    }
	}
    }
    method compute-from {option} {
	# verbose-puts "sdrplay compute-from $option calls $parent.radio cget -frequency"
	set params [$self find-band [$parent.radio cget -frequency]]
	foreach {lo hi} [sdr::parse-range [dict get $params reduction]] break
	set options(-max) [dict get $params dB-max]
	set options(-min) [expr {$options(-max)-($hi-$lo)}]
	switch -- $option {
	    -gain {
		set options(-gain-reduction) [expr {$options(-max)-$options(-gain)}]
	    }
	    -gain-reduction {
		set options(-gain) [expr {$options(-max)-$options(-gain-reduction)}]
	    }
	}
	## compute gain distribution from gain reduction
	## technically a gain reduction distribution
	set tlna [lindex [dict get $params lna] end]
	set vlna [lindex [dict get $params lna] end-2]
	set tmix [lindex [dict get $params mixer] end]
	set vmix [lindex [dict get $params mixer] end-2]
	# if less than threshold, gain reduction is 0 else value
	if {$options(-gain-reduction) < $tlna} { set grlna 0 } else { set grlna $vlna }
	if {$options(-gain-reduction) < $tmix} { set grmix 0 } else { set grmix $vmix }
	set grbas [expr {$options(-gain-reduction)-$grlna-$grmix}]
	set options(-gain-distribution) [format {%3d %3d %3d} $grbas $grlna $grmix]
    }
    ##
    ## monitor our configuration options
    ## 
    method monitor {opts prefix} {
	foreach option $opts {
	    trace variable options($option) w [list {*}[mymethod monitor-fired] $prefix]
	}
    }
    method monitor-fired {prefix name1 name2 op} {
	{*}$prefix $name2 $options($name2)
    }
}
