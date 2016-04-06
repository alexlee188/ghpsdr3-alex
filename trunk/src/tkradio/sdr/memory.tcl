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
    ## these are the default memories for QtRadio, reformatted
    ## freq mode filter
    ## spectrumLow spectrumHigh waterfallLow waterfallHigh were omitted
    ## because they're too hard and too confusing to change
    set data [dict create \
		  160m [dict create \
			    0 {1810000 CWL {-250 .. 250}} \
			    1 {1835000 LSB {-3050 .. -150}} \
			    2 {1845000 AM  {-3300 .. 3300}} \
			    3 {1845000 AM  {-3300 .. 3300}} ] \
		  80m  [dict create \
			    0 {3501000 CWL {-250 .. 250}} \
			    1 {3751000 LSB {-2900 .. -200}} \
			    2 {3850000 LSB {-2900 .. -200}} \
			    3 {3850000 LSB {-2900 .. -200}} ] \
		  60m  [dict create \
			    0 {5330500 CWL {-300 .. 300}} \
			    1 {5346500 LSB {-3450 .. -150}} \
			    2 {5366500 LSB {-3450 .. -150}} \
			    3 {5371500 LSB {-3450 .. -150}} \
			    4 {5403500 LSB {-3450 .. -150}} ]\
		  40m [dict create \
			   0 {7001000 CWL {-250 .. 250}} \
			   1 {7060000 LSB {-3450 .. -150}} \
			   2 {7100000 LSB {-3450 .. -150}} \
			   3 {7100000 LSB {-3450 .. -150}} ] \
		  30m [dict create \
			   0 {10120000 CWU {-250 .. 250}} \
			   1 {10130000 CWU {-250 .. 250}} \
			   2 {10140000 CWU {-250 .. 250}} \
			   3 {10140000 CWU {-250 .. 250}} ] \
		  20m [dict create \
			   0 {14010000 CWU {-250 .. 250}} \
			   1 {14230000 USB {150 .. 3450}} \
			   2 {14336000 USB {150 .. 3450}} \
			   3 {14336000 USB {150 .. 3450}} ] \
		  17m [dict create \
			   0 {18068600 CWU {-250 .. 250}} \
			   1 {18125000 USB {150 .. 3450}} \
			   2 {18140000 USB {150 .. 3450}} \
			   3 {18140000 USB {150 .. 3450}} ] \
		  15m [dict create \
			   0 {21001000 CWU {-250 .. 250}} \
			   1 {21255000 USB {150 .. 3450}} \
			   2 {21300000 USB {150 .. 3450}} \
			   3 {21300000 USB {150 .. 3450}} ] \
		  12m [dict create \
			   0 {24895000 CWU {-250 .. 250}} \
			   1 {24900000 USB {150 .. 3450}} \
			   2 {24910000 USB {150 .. 3450}} \
			   3 {24910000 USB {150 .. 3450}} ] \
		  10m [dict create \
			   0 {28010000 CWU {-250 .. 250}} \
			   1 {28300000 USB {150 .. 3450}} \
			   2 {28400000 USB {150 .. 3450}} \
			   3 {28400000 USB {150 .. 3450}} ] \
		  6m [dict create \
			  0 {50010000 CWU {-250 .. 250}} \
			  1 {50125000 USB {150 .. 3450}} \
			  2 {50200000 USB {150 .. 3450}} \
			  3 {50200000 USB {150 .. 3450}} ] \
		  ]
}

proc sdr::memory-save {band mem freq mode filter} {
    dict set ::sdr::memory::data $band $mem [list $freq $mode $filter]
}

proc sdr::memory-get {band mem} { return [dict get $::sdr::memory::data $band $mem] }
proc sdr::memory-freq {band mem} { return [lindex [memory-get $band $mem] 0] }
proc sdr::memory-mode {band mem} { return [lindex [memory-get $band $mem] 1] }
proc sdr::memory-filter {band mem} { return [lindex [memory-get $band $mem] 2] }

