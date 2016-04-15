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
package provide sdrtk::hardware-sdrplay-ui 1.0

package require Tcl
package require Tk
package require snit

namespace eval ::sdrtk {}

snit::widgetadaptor sdrtk::hardware-sdrplay-ui {
    option -radio -readonly
    option -min -default 0 -type snit::integer -configuremethod Configure
    option -max -default 102 -type snit::integer -configuremethod Configure
    option -lna -default 0 -type snit::integer -configuremethod Configure
    option -mixer -default 0 -type snit::integer -configuremethod Configure
    option -baseband -default 0 -type snit::integer -configuremethod Configure
    
    delegate option * to hull
    delegate method * to hull
    
    constructor {args} {
	installhull using ttk::labelframe -text {Hardware Gain}
	grid [ttk::spinbox $win.l01 -width 4] -row 0 -column 1
	grid [ttk::label $win.l02 -text dB] -row 0 -column 2
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
	$win.l01 configure -from $options(-min) -to $options(-max) -increment 1 -command [mymethod spinboxcommand]
	$win.l01 set [$options(-radio) cget -hardware-gain]
	$self configure {*}$args
    }
    method spinboxcommand {args} {
	puts "spinboxcommand {$args} and value [$win.l01 get]"
    }
}
