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
# an option menu
#
package provide ui::frequency-display 1.0

package require Tk
package require snit

namespace eval ::ui {}

# define a frequency display widget
# G MMM kkk HHH UUU radix point replacing space
# depending on the Unit displayed
snit::widgetadaptor ui::frequency-display {
    variable data -array {
	formatted {}
	width 16
    }
    option -command -default {}
    option -value -default 14010000 -type snit::integer -configuremethod Configure
    option -min -default 100000 -configuremethod Configure
    option -max -default 2000000000 -configuremethod Configure
    delegate method * to hull
    delegate option * to hull
    constructor args {
	installhull using ttk::label
	$self configure -width 16 -textvar [myvar data(formatted)] -font {courier 50 bold} -foreground red -background black {*}$args
	$self redraw
	bind $win <ButtonPress> [mymethod button press %x %y %b]
	bind $win <ButtonRelease> [mymethod button release %x %y %b]
	bind $win <MouseWheel> [mymethod button wheel %x %y %D]
    }
    method clamp-frequency {val} {
	return [expr {max($options(-min),min($val,$options(-max)))}]
    }
    method {Configure -value} {val} {
	set val [$self clamp-frequency $val]
	if {$options(-value) != $val} {
	    puts "$self configure -value $val"
	    set options(-value) $val
	    $self redraw
	}
    }
    # if the top or bottom of a digit is tapped, increase or lower the value of the digit.
    # but if the release is outside the window then cancel the change.
    # if the blank between digits is tapped, zero the digits to the right.
    method button {t x y etc} {
	switch $t-$etc {
	    press-1 {}
	    release-1 {
		set cy [expr {$y/([winfo height $win]/2)}]
		if {$cy != 0 && $cy != 1} return
		set cx [expr {$x/([winfo width $win]/$data(width))}]
		switch $cx {
		    0 { $self delta 1000000000 $cy }
		    1 { $self zero  1000000000 $cy }
		    2 { $self delta  100000000 $cy }
		    3 { $self delta   10000000 $cy }
		    4 { $self delta    1000000 $cy }
		    5 { $self zero     1000000 $cy }
		    6 { $self delta     100000 $cy }
		    7 { $self delta      10000 $cy }
		    8 { $self delta       1000 $cy }
		    9 { $self zero        1000 $cy }
		    10 { $self delta       100 $cy }
		    11 { $self delta        10 $cy }
		    12 { $self delta         1 $cy }
		    default return
		}
	    }
	    default {
		set cx [expr {$x/([winfo width $win]/17)}]
		set cy [expr {$y/([winfo height $win]/2)}]
		verbose-puts "button $t $cx $cy $etc"
	    }
	}
    }
    # change frequency by incrementing or decrementing by $delta
    method delta {delta cy} {
	set f $options(-value)
	if {$cy == 1} { set delta [expr {-$delta}] }
	incr f $delta
	if {$options(-command) ne {}} {
	    {*}$options(-command) $f
	} else {
	    $self configure -value $f
	}
    }
    # change frequency by zeroing remainder by $zero
    method zero {zero cy} {
	set f $options(-value)
	set f [expr {$f - ($f % $zero)}]
	if {$options(-command) ne {}} {
	    {*}$options(-command) $f
	} else {
	    $self configure -value $f
	}
    }
    method redraw {} {
	set f $options(-value)
	set G [expr {$f / 1000000000}]
	set f [expr {$f % 1000000000}]
	set M [expr {$f / 1000000}]
	set f [expr {$f % 1000000}]
	set k [expr {$f / 1000}]
	set H [expr {$f % 1000}]
	set data(formatted) [format {%1d %03d %03d %03d Hz} $G $M $k $H]
	for {set i 0} {$i < [string length $data(formatted)]} {incr i} {
	    switch [string index $data(formatted) $i] {
		0 {
		    # replace leading zeroes with spaces
		    set data(formatted) [string replace $data(formatted) $i $i { }]
		    continue
		}
		{ } continue
		default break
	    }
	}
    }
}


