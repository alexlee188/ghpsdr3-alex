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

package provide sdr::util 1.0

namespace eval ::sdr::util {}

##
## map a function over a list
##
proc ::sdr::map {command items} { set map {}; foreach i $items { lappend map [{*}$command $i] }; return $map }

##
## get the maximum string length in a list of items
##
proc sdr::maxwidth {list} {
    return [tcl::mathfunc::max {*}[lmap s $list {string length $s}]]
}

##
## convert variously formatted frequencies to Hertz
##
proc ::sdr::hertz {string} {
    # match a number followed by an optional frequency unit
    # allow any case spellings of frequency units
    # allow spaces before, after, or between
    # allow exponent specification
    if {[regexp -nocase {^\s*(-?\d+|\d+\.\d+|\.\d+|\d+\.)([eE][-+]\d+)?\s*([kMG]?Hz)?\s*$} $string all number exponent unit]} {
	set f $number$exponent
	switch -nocase $unit {
	    {} - Hz  { return [expr {int($f*1.0)}] }
	    kHz { return [expr {int($f*1000.0)}] }
	    MHz { return [expr {int($f*1000.0*1000.0)}] }
	    GHz { return [expr {int($f*1000.0*1000.0*1000.0)}] }
	}
    }
    error "badly formatted frequency: $string"
}

##
## choose a random item from a list
##
proc random-item {list} { return [lindex $list [expr {int(rand()*[llength $list])}]] }

##
## parse a {<low> .. <high>} range string into a list
##
proc ::sdr::parse-range {string} {
    if {[set sep [string first { .. } $string]] < 0} { error "unmatched range string: $string" }
    return [list [string range $string 0 $sep-1] [string range $string $sep+4 end]]
}
##
## ditto, and apply hertz conversions to low and high
##
proc ::sdr::parse-range-hertz {string} {
    return [lmap {f} [sdr::parse-range $string] {sdr::hertz $f}]
}

proc ::sdr::in-range {range value} {
    return [::sdr::in-range-list {*}[sdr::parse-range $range] $value]
}

proc ::sdr::in-range-list {lo hi value} {
    return [expr {$lo <= $value && $value < $hi}]
}

	
