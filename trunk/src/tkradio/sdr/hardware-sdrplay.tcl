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
package provide sdr::hardware::sdrplay 1.0

package require Tcl
package require snit

package require sdr::util
package require sdr::command

namespace eval ::sdr {}

snit::type sdr::hardware-sdrplay {
    option -radio -readonly
    option -gain-reduction -configuremethod Configure
    option -frequency -configuremethod Configure
    option -gain -configuremethod Configure
    option -gain-distribution
    
    # the low and high frequencies tuned
    # foreach band of frequencies
    #	the max gain in dB according to some library
    #	the overall range of gain reduction available
    #	the ranges of baseband, lna, and mixer gain
    #		in the standard 
    variable caps {
	range {100000 .. 2000000000} 
	bands {
	    {100000 .. 59999999} {
		dB-max 98 reduction {0 .. 102} baseband {0 .. 59 by 1} lna {0 .. 24 by 24 at 35} mixer {0 .. 19 by 19 at 83} 
	    }
	    {60000000 .. 119999999} {
		dB-max 103 reduction {0 .. 102} baseband {0 .. 59 by 1} lna {0 .. 24 by 24 at 29} mixer {0 .. 19 by 19 at 83}
	    }
	    {120000000 .. 249999999} {
		dB-max 107 reduction {0 .. 102} baseband {0 .. 59 by 1} lna {0 .. 24 by 24 at 29} mixer {0 .. 19 by 19 at 83}
	    }
	    {250000000 .. 419999999} {
		dB-max 94 reduction {0 .. 102} baseband {0 .. 59 by 1} lna {0 .. 24 by 24 at 35} mixer {0 .. 19 by 19 at 83}
	    }
	    {420000000 .. 999999999} {
		dB-max 94 reduction {0 .. 85} baseband {0 .. 59 by 1} lna {0 .. 7 by 7 at 12} mixer {0 .. 19 by 19 at 66}
	    }
	    {1000000000 .. 1999999999} {
		dB-max 105 reduction {0 .. 85} baseband {0 .. 59 by 1} lna {0 .. 7 by 7 at 10} mixer {0 .. 19 by 19 at 66}
	    }
	}
    }
    constructor {args} {
	set caps [dict create {*}$caps]
	dict set caps bands [dict create {*}[dict get $caps bands]]
	$self configure {*}$args
    }
    method {Configure -gain-reduction} {dB} {
	set options(-gain-reduction) $dB
	set options(-gain) [$self compute-from -gain-reduction]
	sdr::command::hardware::*setattenuator $dB
    }
    method {Configure -gain} {dB} {
	set options(-gain) $dB
	set options(-gain-reduction) [$self compute-from -gain]
	sdr::command::hardware::*setattenuator $dB
    }
    method find-band {f} {
	dict foreach {band params} [dict get $caps bands] {
	    if {[sdr::in-range $band $f]} {
		return $band $params
	    }
    }
    method get-range {} {
	return [sdr::parse-range [dict get caps range]]
    }
}


