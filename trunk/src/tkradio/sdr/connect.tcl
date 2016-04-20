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

package provide sdr::connect 1.0

package require Tcl
package require snit

##
## channel handlers
##
namespace eval ::sdr {}

snit::type sdr::connect {
    option -parent -readonly true
    option -host
    option -port
    option -status

    variable data -array {
	channel {}
	status {}
	spectrum-listeners {}
	bandscope-listeners {}
	audio-listeners {}
	answer-listeners {}
	status-listeners {}
	snap-names {nreads nreadbytes nwrites nwritebytes npartial nspectrum naudio nanswer}
	nreads 0
	nreadbytes 0
	nwrites 0
	nwritebytes 0
	npartial 0
	nspectrum 0
	naudio 0
	nanswer 0
    }

    ##
    ## life cycle
    ##
    constructor {args} { $self configure {*}$args }
	
    destructor { $self disconnect }

    ##
    ## listener management
    ##
    # add a listener to a queue
    method add-listener {queue prefix} {
	if { ! [info exists data($queue-listeners)]} {
	    error "no connection listeners to $queue"
	}
	if {[lsearch $data($queue-listeners) $prefix] >= 0} {
	    error "$prefix is already listening to $queue"
	}
	lappend data($queue-listeners) $prefix
    }
    # subtract a listener from a queue
    method sub-listener {queue prefix} {
	if { ! [info exists data($queue-listeners)]} {
	    error "no connection listeners to $queue"
	}
	set i [lsearch $data($queue-listeners) $prefix]
	if {$i < 0} {
	    error "$prefix is not listening to $queue"
	}
	set data($queue-listeners) [lreplace $data($queue-listeners) $i $i]
    }
    # process the listeners on a queue
    method process {queue args} {
	foreach prefix $data($queue-listeners) {
	    # after 0 
	    {*}$prefix {*}$args
	}
    }
    
    ##
    ## connection management
    ##
    method connect {host port} {
	$self configure -host $host -port $port
	set data(channel) [socket $options(-host) $options(-port)]
	chan configure $data(channel) -blocking 0 -encoding binary -translation binary
	fileevent $data(channel) readable [list {*}[mymethod reader] $data(channel) {}]
	# fileevent $data(channel) writable [mymethod writer]
    }

    method disconnect {} {
	fileevent $data(channel) readable {}
	fileevent $data(channel) writable {}
	close $data(channel)
	set data(channel) {}
    }

    method is-connected {} { return [expr {$data(channel) ne {}}] }
    
    ##
    ## asynchronous reader
    ##
    method reader {fd partial} {
	while {1} {
	    if {[catch {read $fd} buffer]} {
		$self process status "error on read $fd, $error, $::errorInfo"
		return
	    }
	    incr data(nreads)
	    incr data(nreadbytes) [string length $buffer]
	    if {$partial ne {}} {
		incr data(npartial)
		set buffer [append partial $buffer]
	    }
	    while {[set len [string length $buffer]] > 0} {
		switch [string index $buffer 0] {
		    \0 {		# spectrum buffer
			if {$len < 1+1+1+2+2+2+4+2} {
			    $self process status "need longer spectrum buffer: $len < header"
			    fileevent $fd readable [list {*}[mymethod reader] $fd $buffer]
			    return
			}
			if {[binary scan $buffer cccSSSIS type version subversion samples main sub sr lo] != 8} {
			    $self process status "error in binary scan of spectrum header"
			    return
			}
			# puts stderr "spectrum t $type, v $version, subv $subversion, samples $samples, main $main, sub $sub, sr $sr, lo $lo"
			set end [expr {15+$samples-1}]
			if {$len < $end} {
			    $self process status "need longer spectrum buffer: $len < $end"
			    fileevent $fd readable [list {*}[mymethod reader] $fd $buffer]
			    return
			}
			set dat [string range $buffer 15 $end]
			set buffer [string range $buffer $end+1 end]
			$self process spectrum $main $sub $sr $lo $dat
			incr data(nspectrum)
			continue
		    }
		    \1 {		# audio
			if {$len < 1+1+1+2} {
			    $self process status "need longer audio buffer: $len < header"
			    fileevent $fd readable [list {*}[mymethod reader] $fd $buffer]
			    return
			}
			if {[binary scan $buffer cccS type version subversion samples] != 4} {
			    $self process status "error in binary scan of audio header"
			    return
			}
			# puts "audio buffer type $type, version $version, subversion $subversion, samples $samples"
			set end [expr {5+$samples-1}]
			if {$len < $end} {
			    # puts stderr "audio buffer type $type, version $version, subversion $subversion, samples $samples"
			    $self process status "need a longer audio string: $len < $end"
			    fileevent $fd readable [list {*}[mymethod reader] $fd $buffer]
			    return
			}
			set dat [string range $buffer 5 $end]
			set buffer [string range $buffer $end+1 end]
			$self process audio $dat
			incr data(naudio)
			continue
		    }
		    \2 {			# bandscope
			$self process status "error unhandled bandscope buffer"
			return
		    }
		    \3 {			# rtp reply buffer
			$self process status "error unhandled rtp-reply buffer"
			return
		    }
		    4 {		# answer buffer
			if {$len < 3} {
			    $self process status "need a longer answer string: $len < 3"
			    fileevent $fd readable [list {*}[mymethod reader] $fd $buffer]
			    return
			}
			if {[scan [string range $buffer 1 2] %d samples] != 1} {
			    $self process status "error scanning an answer size!"
			    return
			}
			set end [expr {3+$samples}]
			if {$len < $end} {
			    $self process status "need a longer answer string: $len < $end"
			    fileevent $fd readable [list {*}[mymethod reader] $fd $buffer]
			    return
			}
			set dat [string range $buffer 3 $end-1]
			set buffer [string range $buffer $end end]
			$self process answer $dat
			incr data(nanswer)
			continue
		    }
		    default {
			binary scan $buffer c type
			$self process status "unknown buffer type $type"
			return
		    }
		}
	    }
	    if {[eof $data(channel)]} {
		$self process status "eof on read $data(channel)"
		return
	    }
	    if {[fblocked $data(channel)]} {
		# $self process status "flbocked on $data(channel)"
		fileevent $fd readable [list {*}[mymethod reader] $fd $buffer]
		return;
	    }
	}
    }

    ##
    ## asynchronous writer
    ##
    method writer {} {
	puts stderr "::sdr::writer $data(channel)"
    }

    ##
    ## synchronous write
    ##
    method write {str} {
	incr data(nwrites)
	incr data(nwritebytes) [string length $str]
	puts -nonewline $data(channel) $str
    }

    method flush {} { flush $data(channel) }

    ## capture the current values
    method snap-data {} {
	foreach n $data(snap-names) {
	    lappend d $data($n)
	    set data($n) 0
	}
	return $d
    }
	
}
