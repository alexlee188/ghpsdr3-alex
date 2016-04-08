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
# an audio output channel constructed from memchan and snack
#
package provide sdr::audio 1.0

package require pa::simple
package require tcl::chan::memchan

namespace eval ::sdr {}

namespace eval ::sdr::audio {}

proc audio-out-start {format rate} {
    switch $format {
	ALAW { set format aLaw }
	PCM { set format s16le }
	default { error "unknown audio format: $format" }
    }
    puts "pa::simple::new sdr playback sdr $format 1 $rate"
    if {[catch {pa::simple::new sdr playback sdr $format 1 $rate} error]} {
	puts "error starting audio: $error"
    }
    # set ::sdr::audio::trace [open audio.trace wb]
}
proc audio-out-stop {} {
    if {[catch {::pa::simple::drain} error]} { puts "error draining: $error" }
    if {[catch {::pa::simple::flush} error]} { puts "error flushing: $error" }
    if {[catch {::pa::simple::free} error]} { puts "error freeing: $error" }
}
proc audio-out-data {data} {
    if {[catch {pa::simple::write $data} error]} {
	puts "error writing audio: $error"
    }
    # puts -nonewline $::sdr::audio::trace $data
}
