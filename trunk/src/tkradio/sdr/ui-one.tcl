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
# radio ui widget
#

package provide sdr::ui-one 1.0

package require Tk
package require snit

package require sdr::util

package require ui::frequency-display
package require ui::optionmenu

package require sdrtk::meter
package require sdrtk::spectrum-waterfall

namespace eval ::sdr {}

# operating modes
proc sdr::get-modes {} { return [::sdr::command::get-modes] }
snit::enum sdr::modes-type -values [sdr::get-modes]
# spectrum modes
proc sdr::get-pwsmodes {} { return [::sdr::command::get-pwsmodes] }
snit::enum sdr::pwsmodes-type -values [sdr::get-pwsmodes]
# agc modes
proc sdr::get-agcmodes {} { return [::sdr::command::get-agcmodes] }
snit::enum sdr::agcmodes-type -values [sdr::get-agcmodes]

# define the radio widget
# ah, mistake, there should be a radio widget which handles
# the basic logic of maintaining state and talking to dspserver,
# and then there should be one or more ui widgets which allow
# user control of the radio model
#
snit::widgetadaptor sdr::ui-one {
    option -text -default {} -configuremethod Configure
    option -radio -readonly true;	# must be set at creation

    delegate method * to hull
    delegate option * to hull

    constructor args {
	installhull using ttk::frame
	$self configure {*}$args
	verbose-puts "sdr::ui-one $win $args "

	# s-meter
	grid [sdrtk::meter $win.meter] -row 0 -column 0 -sticky nsew
	$self rmonitor -main-meter -subrx-meter

	# frequency display
	grid [ui::frequency-display $win.freq \
		  -value [$self rget -frequency] \
		  -command [list {*}[mymethod rreport] -frequency] \
		 ] -row 0 -column 1 -columnspan 6
	$self rmonitor -frequency

	# radio service selector
	grid [ui::optionmenu $win.service \
		  -value [$self rget -service] \
		  -values [$self rget -service-values] \
		  -width [sdr::maxwidth [sdr::band-data-services]] \
		  -command [list {*}[mymethod rreport] -service] \
		 ] -row 1 -column 1
	$self rmonitor -service -service-values

	# band selector
	grid [ui::optionmenu $win.band \
		  -value [$self rget -band] \
		  -values [$self rget -band-values] \
		  -width [sdr::maxwidth [sdr::band-data-all-bands]] \
		  -command [list {*}[mymethod rreport] -band] \
		 ] -row 1 -column 2
	$self rmonitor -band -band-values

	# mode selector
	grid [ui::optionmenu $win.mode \
		  -value [$self rget -mode] \
		  -values [$self rget -mode-values] \
		  -width [sdr::maxwidth [sdr::modes-type cget -values]] \
		  -command [list {*}[mymethod rreport] -mode] \
		 ] -row 1 -column 3
	$self rmonitor -mode -mode-values

	# filter selector
	grid [ui::optionmenu $win.filter \
		  -value [$self rget -filter] \
		  -values [sdr::filters-get [$self rget -mode]] \
		  -width [sdr::maxwidth [sdr::filters-get-all]] \
		  -command [list {*}[mymethod rreport] -filter] \
		 ] -row 1 -column 4
	$self rmonitor -filter -filter-values

	# server selector
	grid [ui::optionmenu $win.name \
		  -value [$self rget -name] \
		  -values [$self rget -name-values] \
		  -width 8 \
		  -command [list {*}[mymethod rreport] -name] \
		 ] -row 1 -column 5
	$self rmonitor -name -name-values

	# connect to server
	grid [ttk::checkbutton $win.conn \
		  -width 10 \
		  -command [mymethod connecttoggle] \
		  -variable [myvar data(connect)] \
		  -onvalue {disconnect} \
		  -offvalue {connect} \
		  -textvar [myvar data(connect)] \
		 ] -row 1 -column 6
	$self rmonitor -channel -channel-status
	
	# spectrum and waterfall
	grid [sdrtk::spectrum-waterfall $win.sw \
		  -command [list {*}[mymethod rreport] -frequency] \
		 ] -row 2 -column 1 -columnspan 6
	$self rmonitor -spectrum -sample-rate -local-oscillator

    }
    method {Configure -text} {val} {
	if {$options(-text) ne $val} {
	    set options(-text) $val
	    wm title . $val
	}
    }
    method rget {opt} {
	return [$options(-radio) cget $opt]
    }
    method rmonitor {args} {
	foreach opt $args {
	    $options(-radio) monitor $opt [mymethod rmonitor-fire]
	}
    }
    method rmonitor-fire {option value} {
	switch -- $option {
	    -main-meter -
	    -subrx-meter { $win.meter configure $option $value }
	    -frequency {
		$win.freq configure -value $value
		$win.sw configure $option $value
	    }
	    -service { $win.service configure -value $value }
	    -band { $win.band configure -value $value }
	    -band-values { $win.band configure -values $value }
	    -mode { $win.mode configure -value $value }
	    -mode-values { $win.mode configure -values $value }
	    -filter { $win.filter configure -value $value }
	    -filter-values { $win.filter configure -values $value }
	    -name { $win.name configure -value $value }
	    -name-values { $win.name configure -values $value }
	    -channel -
	    -channel-status { $win.connecttoggle configure $option $value }
	    -spectrum -
	    -sample-rate -
	    -local-oscillator { $win.sw configure $option $value }
	    default {
		error "unknown rmonitor-fired for option {$option}"
	    }
	}
    }
    method rreport {option value} {
	{*}$options(-radio) configure $option $value
    }
    method connecttoggle {} {
	# the button label is already looking forward
	if {$::channel == -1} {
	    if {[catch {$self connect} error]} {
		puts $error\n$::errorInfo
		set data(connect) {connect}
	    }
	} else {
	    if {[catch {$self disconnect} error]} {
		puts $error\n$::errorInfo
		set data(connect) {disconnect}
	    }
	}
    }
}
