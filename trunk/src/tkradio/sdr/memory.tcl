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
# a repository of channel memories
#
package provide sdr::memory 1.0

namespace eval ::sdr {}

namespace eval ::sdr::memory {
    # memories, none
    variable data [dict create]
    # last settings in 
    variable last [dict create \
		       service Amateur \
		       {Amateur settings} {-band 20m} \
		       {Amateur 160m settings} {-frequency 1810000 -mode CWL -filter {-250 .. 250} -agc SLOW} \
		       {Amateur 80m settings} {-frequency  3501000 -mode CWL -filter {-250 .. 250} -agc SLOW} \
		       {Amateur 60m settings} {-frequency  5330500 -mode CWL -filter {-300 .. 300} -agc SLOW} \
		       {Amateur 40m settings} {-frequency  7001000 -mode CWL -filter {-250 .. 250} -agc SLOW} \
		       {Amateur 30m settings} {-frequency 10120000 -mode CWU -filter {-250 .. 250} -agc SLOW} \
		       {Amateur 20m settings} {-frequency 14010000 -mode CWU -filter {-250 .. 250} -agc SLOW} \
		       {Amateur 17m settings} {-frequency 18068600 -mode CWU -filter {-250 .. 250} -agc SLOW} \
		       {Amateur 15m settings} {-frequency 21001000 -mode CWU -filter {-250 .. 250} -agc SLOW} \
		       {Amateur 12m settings} {-frequency 24895000 -mode CWU -filter {-250 .. 250} -agc SLOW} \
		       {Amateur 10m settings} {-frequency 28010000 -mode CWU -filter {-250 .. 250} -agc SLOW} \
		       {Amateur 6m settings}  {-frequency 50010000 -mode CWU -filter {-250 .. 250} -agc SLOW} \
		      ]
}

proc sdr::set-mem {service band mem freq mode filter} {
    if { ! [dict exists ::sdr::memory::data $service]} {
	dict set ::sdr::memory::data $service [dict create]
    }
    set servdict [dict get ::sdr::memory::data $service]
    if { ! [dict exists $sdict $band]} {
	dict set servdict $band [dict create]
    }
    set banddict [dict get $servdict $band]
    dict set banddict $mem [list $freq $mode $filter]
    dict set servdict $band $banddict
    dict set ::sdr::memory::data $service $servdict
}
proc sdr::is-mem {service band mem} {
    return [dict exists $::sdr::memory::data $service $band $mem]
}
proc sdr::get-mem {service band mem} {
    if {[dict exists $::sdr::memory::data $service $band $mem]} {
	return [dict get $::sdr::memory::data $service $band $mem]
    } else {
	return {}
    }
}
proc sdr::get-mem-freq {service band mem} { return [lindex [get-mem $band $mem] 0] }
proc sdr::get-mem-mode {service band mem} { return [lindex [get-mem $band $mem] 1] }
proc sdr::get-mem-filter {service band mem} { return [lindex [get-mem $band $mem] 2] }

proc sdr::set-last {type value} { 
    # puts "sdr::set-last {$type} {$value}"
    dict set ::sdr::memory::last $type $value 
}
proc sdr::get-last {type} { 
    # puts "sdr::get-last {$type}"
    if {[dict exists $::sdr::memory::last $type]} {
	# puts "sdr::get-last {$type} exists"
	return [dict get $::sdr::memory::last $type] 
    } else {
	# puts "no sdr::get-last {$type} found"
	return {}
    }
}
