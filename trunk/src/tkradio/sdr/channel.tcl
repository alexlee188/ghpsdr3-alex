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

package provide sdr::channel 1.0

##
## channel handlers
##
namespace eval ::sdr {}

proc ::sdr::connect {id host port} {
    set chan [socket $host $port]
    chan configure $chan -blocking 0 -encoding binary -translation binary
    fileevent $chan readable [list ::sdr::reader $id $chan]
    # fileevent $chan writable [list ::sdr::writer $id $chan]
    return $chan
}

proc ::sdr::disconnect {id chan} {
    fileevent $chan readable {}
    # fileevent $chan writable {}
    close $chan
}

proc ::sdr::reader {id chan} {
    set buffer [read $chan]
    if {[eof $chan]} {
	verbose-puts "::sdr::reader $id $chan -> eof is true"
	::sdr::disconnect $id $chan
	return
    }
    while {[set len [string length $buffer]] > 0} {
	switch [string index $buffer 0] {
	    \0 {		# spectrum buffer
		if {[binary scan $buffer cccSSSIS type version subversion samples main sub sr lo] != 8} {
		    error "misread spectrum header"
		}
		# puts stderr "spectrum header type $type, version $version, subversion $subversion, samples $samples, main $main, sub $sub, sr $sr, lo $lo"
		set end [expr {15+$samples-1}]
		if {$len < $end} {
		    error "need longer spectrum buffer: $len < $end"
		}
		set data [string range $buffer 15 $end]
		set buffer [string range $buffer $end+1 end]
		$::radio process-spectrum $main $sub $sr $lo $data
		# puts stderr "spectrum $samples bytes, main $main, sub $sub, sr $sr, lo $lo"
		# puts stderr "spectrum buffer remains [string length $buffer]"
		continue
	    }
	    \1 {		# audio
		if {[binary scan $buffer cccS type version subversion samples] != 4} {
		    error "misread audio header"
		}
		# puts "audio buffer type $type, version $version, subversion $subversion, samples $samples"
		set end [expr {5+$samples-1}]
		if {$len < $end} {
		    puts stderr "audio buffer type $type, version $version, subversion $subversion, samples $samples"
		    puts stderr "need a longer audio string: $len < $end"
		}
		set data [string range $buffer 5 $end]
		set buffer [string range $buffer $end+1 end]
		$::radio process-audio $data
		# puts stderr "audio $samples bytes"
		# puts stderr "audio buffer remains [string length $buffer]"
		continue
	    }
	    \2 {			# bandscope
		error "bandscope buffer"
	    }
	    \3 {			# rtp reply buffer
		error "rtp-reply buffer"
	    }
	    4 {		# answer buffer
		if {$len < 3} {
		    error "need a longer answer string: $len < 3"
		}
		set samples [scan [string range $buffer 1 2] %d]
		set end [expr {3+$samples}]
		if {$len < $end} {
		    error "need a longer answer string: $len < $end"
		}
		set data [string range $buffer 3 $end-1]
		set buffer [string range $buffer $end end]
		$::radio process-answer $data
		# puts stderr "answer $samples bytes -> {$data}"
		# puts stderr "answer buffer remains [string length $buffer]"
		continue
	    }
	    default {
		binary scan $buffer c type
		error "unknown buffer type $type"
	    }
	}
    }
    if {[fblocked $chan]} {
	# puts "::sdr::reader $id $chan -> fblocked is true"
	return;
    }
}

proc ::sdr::writer {id chan} {
    puts stderr "::sdr::writer $id $chan"
}

