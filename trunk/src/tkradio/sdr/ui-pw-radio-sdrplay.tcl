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
# a hardware ui module for the SDRplay
#
package provide sdrui::pw-radio-sdrplay 1.0

package require Tcl
package require Tk
package require snit

namespace eval ::sdrui {}

snit::widgetadaptor sdrui::pw-radio-sdrplay {
    option -parent -readonly true -configuremethod Configure
    option -gain -configuremethod Configure
    option -gain-min -configuremethod Configure
    option -gain-max -configuremethod Configure
    
    component parent
    component spinbox
    component scale

    constructor {args} {
	installhull using ttk::labelframe -text {Hardware Gain}
	install spinbox using ttk::spinbox $win.l01 -width 4 -textvariable [myvar options(-gain)]
	install scale using ttk::scale $win.l03 -length 200 -variable [myvar options(-gain)]
	grid $spinbox -row 0 -column 1
	grid [ttk::label $win.l02 -text dB] -row 0 -column 2
	grid $scale -row 0 -column 3
	#don't know that I need all this
	#grid [ttk::label $win.l10 -text LNA] -row 1 -column 0
	#grid [ttk::label $win.l11 -textvar [myvar options(-lna)]] -row 1 -column 1
	#grid [ttk::label $win.l12 -text dB] -row 0 -column 2
	#grid [ttk::label $win.l20 -text Mixer] -row 1 -column 0
	#grid [ttk::label $win.l21 -textvar [myvar options(-mixer)]] -row 2 -column 1
	#grid [ttk::label $win.l22 -text dB] -row 0 -column 2
	#grid [ttk::label $win.l30 -text Baseband] -row 1 -column 0
	#grid [ttk::label $win.l31 -textvar [myvar options(-baseband)]] -row 3 -column 1
	#grid [ttk::label $win.l32 -text dB] -row 0 -column 2
	$win.l01 configure -increment 1 -command [mymethod spinboxcommand]
	$win.l03 configure -command [mymethod scalecommand]
	$self configure {*}$args
	$self rmonitor {-gain -gain-min -gain-max}
    }
    # need to listen to hardware -gain -gain-min and -gain-max
    # as they change due to frequency tuning
    # need to fix the theme colors for entry spinbox selected
    method spinboxcommand {args} {
	# verbose-puts "spinboxcommand {$args} and value [$win.l01 get]"
	$parent.hw configure -gain $options(-gain)
    }
    method scalecommand {value} {
	# verbose-puts "scalecommand $value"
	set options(-gain) [expr {int(round($value))}]
	$parent.hw configure -gain $options(-gain)
    }
    method {Configure -parent} {val} {
	set options(-parent) $val
	set parent $val
    }
    method {Configure -gain-min} {val} {
	set options(-gain-min) $val
	$spinbox configure -from $val
	$scale configure -from $val
    }
    method {Configure -gain-max} {val} {
	set options(-gain-max) $val
	$spinbox configure -to $val
	$scale configure -to $val
    }
    method {Configure -gain} {val} {
	set options(-gain) $val
    }
	
    ##
    ## monitor our configuration options
    ## 
    method monitor {opts prefix} {
	foreach opt $opts {
	    trace variable options($opt) w [list {*}[mymethod monitor-fired] $prefix]
	}
    }
    method monitor-fired {prefix name1 name2 op} {
	{*}$prefix $name2 $options($name2)
    }
    ##
    ## monitor radio hardware configuration options
    ##
    method rmonitor {opts} {
	foreach opt $opts {
	    $parent.hw monitor $opt [mymethod configure]
	    $self configure $opt [$parent.hw cget $opt]
	}
    }
}
