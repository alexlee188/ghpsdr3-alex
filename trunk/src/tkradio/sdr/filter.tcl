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
# a catalog of filters and code to manage them
#
package provide sdr::filter 1.0

package require sdr::util

namespace eval ::sdr {}

# the filters for the modes
proc ::sdr::filter-parse {filter} {
    return [sdr::parse-range $filter]
}

proc ::sdr::filter-negate {filter} {
    return [lreverse [lmap {x} [::sdr::filter-parse $filter] {expr {-$x}}]]
}

proc ::sdr::filter-bandwidth {filter} {
    foreach {lo hi} [::sdr::filter-parse $filter] break
    return [expr {$hi-$lo}]
}
    
proc ::sdr::filters-negate {filters} {
    return [lmap f $filters {::sdr::filter-negate $f}]
}

proc ::sdr::filters-get {mode} {
    return [dict get ${::sdr::filters} $mode]
}

proc sdr::filters-get-all {} {
    set all {}
    foreach mode [dict keys $::sdr::filters] {
	foreach filter [::sdr::filters-get $mode] {
	    lappend all $filter
	}
    }
    return $all
}

namespace eval ::sdr {
    # define filters
    # cw filters are centered on the cw pitch set elsewhere
    variable filters [dict create]
    dict set filters LSB {
	{-5050 .. -50}  {-4450 .. -50}  {-3900 .. -100} {-3450 .. -150} {-3050 .. -150}
	{-2900 .. -200} {-2600 .. -200} {-2400 .. -300} {-2150 .. -350} {-1400 .. -400}
    }
    dict set filters USB [::sdr::filters-negate [::sdr::filters-get LSB]]
    dict set filters DSB {
	{-8000 .. 8000} {-6000 .. 6000} {-5000 .. 5000} {-4000 .. 4000} {-3300 .. 3300}
	{-2600 .. 2600} {-2000 .. 2000} {-1550 .. 1550} {-1450 .. 1450} {-1200 .. 1200}
    }
    dict set filters CWL {
	{-500 .. 500} {-400 .. 400} {-375 .. 375} {-300 .. 300} {-250 .. 250}
	{-200 .. 200} {-125 .. 125} {-50 .. 50}   {-25 .. 25}   {-13 .. 13}
    }
    dict set filters CWU [::sdr::filters-get CWL]
    dict set filters FM { 
	{-100000 .. 100000} {-80000 .. 80000} {-40000 .. 40000} {-20000 .. 20000} {-10000 .. 10000}
	{-5000 .. 5000}     {-3500 .. 3500}   {-2500 .. 2500}   {-1500 .. 1500}   {-1250 .. 1250}
    }
    dict set filters AM [::sdr::filters-get DSB]
    dict set filters DIGU {
	{5150 .. 150} {4550 .. 150} {3950 .. 150} {3450 .. 150} {3050 .. 150}
	{2850 .. 150} {2550 .. 150} {2250 .. 150} {1950 .. 150} {1150 .. 150} 
    }
    dict set filters SPEC {}
    dict set filters DIGL [::sdr::filters-negate [::sdr::filters-get DIGU]]
    dict set filters SAM [::sdr::filters-get DSB]
    dict set filters DRM {}
}

