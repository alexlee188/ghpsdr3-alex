#
# Copyright (C) 2011, 2012 by Roger E Critchlow Jr, Santa Fe, NM, USA.
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
package provide sdrtk::spectrum-waterfall 1.0.0

package require Tk
package require snit
package require sdrtk::spectrum
package require sdrtk::waterfall

snit::widgetadaptor sdrtk::spectrum-waterfall {
    component spectrum
    component waterfall

    # options to waterfall
    delegate option -pal to waterfall
    delegate option -atten to waterfall
    delegate option -automatic to waterfall
    option -min-f -1e6
    option -max-f 1e6

    # options to spectrum
    delegate option -smooth to spectrum
    delegate option -multi to spectrum
    option -frequency -default 0 -configuremethod Retune
    delegate option -filter-low to spectrum
    delegate option -filter-high to spectrum
    delegate option -band-low to spectrum
    delegate option -band-high to spectrum
    option -local-oscillator -default 0 -configuremethod Retune

    # options to both
    option -sample-rate -default 48000 -type sdrtype::sample-rate -configuremethod Dispatch
    option -min -default -125.0 -type sdrtype::decibel -configuremethod Dispatch
    option -max -default -60.0 -type sdrtype::decibel -configuremethod Dispatch
    option -zoom -default 1 -type sdrtype::zoom -configuremethod Dispatch
    option -pan -default 0 -type sdrtype::pan -configuremethod Dispatch
    option -width -default 1024 -configuremethod Dispatch

    # options to here
    option -command {}

    variable data -array {
	miny 0
	maxy 0
	avgy 0
    }

    delegate option * to hull
    delegate method * to hull

    constructor {args} {
	installhull using ttk::panedwindow -orient vertical
	install spectrum using sdrtk::spectrum $win.s -command [mymethod Tune]
	install waterfall using sdrtk::waterfall $win.w -command [mymethod Tune]
	$hull add $spectrum -weight 0
	$hull add $waterfall -weight 1
	$self configure {*}$args
    }

    method Tune {w how x y f df} {
	# puts "$how $x $y $f $df at c=$options(-center-freq) t=$options(-tuned-freq)"
	switch $how {
	    Press {
		set data(start) [list $x $y $f $df] 
		set data(drag) 0
	    }
	    Release {
		if { ! $data(drag)} {
		    $self Command -frequency [expr {$options(-frequency)+($f-$options(-local-oscillator))}]
		}
	    }
	    Motion {
		lassign $data(start) x0 y0 f0 df0
		if {abs($x-$x0) > 1} {
		    incr data(drag)
		    $self Command -frequency [expr {$options(-frequency)-($f-$f0)}]
		    if {$options(-command) ne {}} {
			{*}$options(-command)
		    }
		    set data(start) [list $x $y $f $df]
		}
	    }
	    default {
		error "unanticipated spectrum/waterfall tuning event: $w $how $x $y $f $df"
	    }
	}
    }

    method Command {args} {
	if {$options(-command) ne {}} {
	    {*}$options(-command) {*}$args
	}
    }
    
    method Dispatch {opt val} {
	set options($opt) $val
	$spectrum configure $opt $val
	$waterfall configure $opt $val
    }

    method Retune {opt val} {
	set options($opt) $val
	$spectrum configure $opt $val
	$waterfall configure $opt $val
    }

    method update {xy miny maxy avgy} {
	set data(miny) [expr {($data(miny)+$miny)/2.0}]
	set data(maxy) [expr {($data(maxy)+$maxy)/2.0}]
	set data(avgy) [expr {($data(avgy)+$avgy)/2.0}]
	# puts "avg $data(avgy) min $data(miny) max $data(maxy)"
	$spectrum update $xy $data(miny) $data(maxy) $data(avgy)
	$waterfall update $xy $data(miny) $data(maxy) $data(avgy)
    }
    
}

