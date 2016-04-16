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

proc audio-out-start {format rate} {
    switch $format {
	ALAW { set format aLaw }
	PCM { set format s16le }
	default { error "unknown audio format: $format" }
    }
    # puts "pa::simple::new tkradio playback sdr $format 1 $rate"
    pa::simple::new sdr playback sdr $format 1 $rate
}
proc audio-out-stop {} {
    #::pa::simple::flush
    #::pa::simple::free
}
proc audio-out-data {data} {
    ::pa::simple::write $data
}
