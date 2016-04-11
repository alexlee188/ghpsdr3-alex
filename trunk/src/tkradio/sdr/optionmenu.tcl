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
package provide ui::optionmenu 1.0

package require Tk
package require snit

namespace eval ::ui {}

# an option menu
snit::widgetadaptor ui::optionmenu {
    option -values -default {} -configuremethod Configure
    option -value -default {} -configuremethod Configure
    option -command -default {}
    delegate method * to hull
    delegate option * to hull
    variable value
    constructor args {
	installhull using ttk::menubutton
	# puts [$self configure]
	# -font {courier 50 bold} 
	# -foreground green -background black 
	$self configure -textvar [myvar value] {*}$args
    }
    method {Configure -values} {val} {
	set options(-values) $val
	catch {destroy $win.m}
	$win configure -menu $win.m
	menu $win.m -tearoff no
	foreach v $val {
	    $win.m add radiobutton -label $v -value $v -variable [$self cget -textvar] -command [mymethod command]
	}
	# $self configure -value $options(-value)
    }
    method {Configure -value} {val} {
	set [$self cget -textvar] $val
    }
    method command {} {
	if {$options(-command) ne {}} {
	    # puts "calling $options(-command) $value"
	    {*}$options(-command) $value
	}
    }
}

