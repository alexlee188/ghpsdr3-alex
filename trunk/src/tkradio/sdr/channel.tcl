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

proc ::sdr::connect {radio host port} {
    set chan [socket $host $port]
    chan configure $chan -blocking 0 -encoding binary -translation binary
    fileevent $chan readable [list ::sdr::reader $radio $chan]
    # fileevent $chan writable [list ::sdr::writer $radio $chan]
    return $chan
}

proc ::sdr::disconnect {radio chan} {
    fileevent $chan readable {}
    # fileevent $chan writable {}
    close $chan
}

proc ::sdr::reader {radio chan} {
    while {1} {
	if {[catch {read $chan} buffer]} {
	    $radio channel-status "error on read $chan, $error, $::errorInfo"
	    return
	}
	while {[set len [string length $buffer]] > 0} {
	    switch [string index $buffer 0] {
		\0 {		# spectrum buffer
		    if {[binary scan $buffer cccSSSIS type version subversion samples main sub sr lo] != 8} {
			$radio channel-status "misread spectrum header"
			return
		    }
		    # puts stderr "spectrum header type $type, version $version, subversion $subversion, samples $samples, main $main, sub $sub, sr $sr, lo $lo"
		    set end [expr {15+$samples-1}]
		    if {$len < $end} {
			$radio channel-status "need longer spectrum buffer: $len < $end"
			return
		    }
		    set data [string range $buffer 15 $end]
		    set buffer [string range $buffer $end+1 end]
		    $radio process-spectrum $main $sub $sr $lo $data
		    continue
		}
		\1 {		# audio
		    if {[binary scan $buffer cccS type version subversion samples] != 4} {
			$radio channel-status "misread audio header"
			return
		    }
		    # puts "audio buffer type $type, version $version, subversion $subversion, samples $samples"
		    set end [expr {5+$samples-1}]
		    if {$len < $end} {
			# puts stderr "audio buffer type $type, version $version, subversion $subversion, samples $samples"
			$radio channel-status "need a longer audio string: $len < $end"
			return
		    }
		    set data [string range $buffer 5 $end]
		    set buffer [string range $buffer $end+1 end]
		    $radio process-audio $data
		    continue
		}
		\2 {			# bandscope
		    $radio channel-status "unhandled bandscope buffer"
		    return
		}
		\3 {			# rtp reply buffer
		    $radio channel-status "unhandled rtp-reply buffer"
		    return
		}
		4 {		# answer buffer
		    if {$len < 3} {
			$radio channel-status "need a longer answer string: $len < 3"
			return
		    }
		    set samples [scan [string range $buffer 1 2] %d]
		    set end [expr {3+$samples}]
		    if {$len < $end} {
			$radio channel-status "need a longer answer string: $len < $end"
			return
		    }
		    set data [string range $buffer 3 $end-1]
		    set buffer [string range $buffer $end end]
		    $radio process-answer $data
		    continue
		}
		default {
		    binary scan $buffer c type
		    $radio channel-status "unknown buffer type $type"
		    return
		}
	    }
	}
	if {[catch {
	    if {[eof $chan]} {
		$radio channel-status "eof on read $chan"
		return
	    }
	} error]} {
	    $radio channel-status "error testing eof $chan"
	    return
	}
	if {[catch {
	    if {[fblocked $chan]} {
		# puts "::sdr::reader $radio $chan -> fblocked is true"
		return;
	    }
	} error]} {
	    $radio channel-status "error testing fblocked $chan"
	    return
	}
    }
}

proc ::sdr::writer {radio chan} {
    puts stderr "::sdr::writer $radio $chan"
}

