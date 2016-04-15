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
    
    component parent
    component spinbox
    
    delegate option -min to spinbox as -from
    delegate option -max to spinbox as -to

    delegate option * to hull
    delegate method * to hull
    
    constructor {args} {
	installhull using ttk::labelframe -text {Hardware Gain}
	install spinbox using ttk::spinbox $win.l01 -width 4
	grid $spinbox -row 0 -column 1
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
	$win.l01 configure -increment 1 -command [mymethod spinboxcommand]
	if {[catch {$parent.hw cget -gain} gain]} { set gain 0 }
	if {[catch {$parent.hw cget -gain-min} min]} { set min 0 }
	if {[catch {$parent.hw cget -gain-max} max]} { set max 102 }
	$win.l01 set $gain
	$self configure -min $min -max $max {*}$args
    }
    # need to listen to hardware -gain -gain-min and -gain-max
    # as they change due to frequency tuning
    # need to fix the theme colors for entry spinbox selected
    method spinboxcommand {args} {
	# verbose-puts "spinboxcommand {$args} and value [$win.l01 get]"
	$parent.hw configure -gain [$win.l01 get]
    }
    method {Configure -parent} {value} {
	set options(-parent) $value
	set parent $value
    }
}
