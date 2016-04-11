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

package provide sdr::radio-ui-one 1.0

package require Tcl
package require Tk
package require snit

package require sdr::util

package require ui::frequency-display
package require ui::optionmenu

package require sdrtk::meter
package require sdrtk::spectrum-waterfall

namespace eval ::sdr {}

#
# a radio ui widget
#
# the ui widgets receive their values from the radio model
# via monitors placed on configuration options.
# so the digital frequency display and the spectrum-waterfall
# display listen to the radio's -frequency option.
# the monitors are defined with the rmonitor method.
#
# changes effected in the widgets are reported to the model
# with the rreport method. so the digital frequency display 
# and the spectrum-waterfall display reconfigure the radio's
# -frequency option to signal tuning events.
#
# high frequency data streams, like meter and spectrum data, are
# directly subscribed to avoid configuration option overhead.
# these call the radio model *-subscribe methods.
#
# the ui widgets do not update their primary values directly,
# they send updates to the radio model and receive the results
# back from the radio model.
#

snit::widgetadaptor sdr::radio-ui-one {
    option -radio -readonly true;	# must be set at creation
    option -text -default {}
    option -name -default {}
    option -channel-status -default {}

    delegate method * to hull
    delegate option * to hull

    variable data -array {
	connect {connect}
    }

    constructor args {
	installhull using ttk::frame
	$self configure {*}$args

	verbose-puts "sdr::ui-one $win $args "

	# s-meter
	grid [sdrtk::meter $win.meter] -row 0 -column 0 -sticky nsew
	$options(-radio) meter-subscribe [list $win.meter update]

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
		  -width [sdr::maxwidth [sdrtype::mode cget -values]] \
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
		  -textvar [myvar data(connect)] \
		  -width 10 \
		  -variable [myvar data(connect)] \
		  -onvalue {disconnect} \
		  -offvalue {connect} \
		  -command [mymethod connecttoggle] \
		 ] -row 1 -column 6
	$self rmonitor -channel-status

	# puts "after $win.conn built, data(connect) => $data(connect)"
	# puts "$win.conn cget -textvar => [$win.conn cget -textvar]"
	# puts "$win.conn cget -variable => [$win.conn cget -variable]"
	
	# spectrum and waterfall
	grid [sdrtk::spectrum-waterfall $win.sw \
		  -command [list {*}[mymethod rreport] -frequency] \
		 ] -row 2 -column 1 -columnspan 6
	$self rmonitor -sample-rate -local-oscillator
	$options(-radio) spectrum-subscribe [list $win.sw update]
	foreach option {-text -channel-status -name} {
	    $self monitor $option [mymethod window-title]
	}
    }
    # rewrite the window title to reflect statusa
    method window-title {args} {
	puts "managing window title"
	wm title . "$options(-text) -- $options(-name) $options(-channel-status)"
    }

    # monitor configuration options
    method monitor {option prefix} {
	trace variable options($option) w [list {*}[mymethod monitor-fired] $prefix]
    }
    method monitor-fired {prefix name1 name2 op} {
	{*}$prefix $name2 $options($name2)
    }
    
    ##
    ## manipulations on the radio model
    ##
    method rget {opt} {
	return [$options(-radio) cget $opt]
    }
    method rmonitor {args} {
	foreach option $args {
	    #set options($option) [$options(-radio) cget $option]
	    $options(-radio) monitor $option [mymethod rmonitor-fire]
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
	    -name { 
		$win.name configure -value $value
		$win configure $option $value
	    }
	    -name-values { $win.name configure -values $value }
	    -channel-status { $win configure $option $value }
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

    ##
    ## delegate the connection to the radio model
    ##
    method connecttoggle {} {
	$options(-radio) connecttoggle
	if {[$options(-radio) is-connected]} {
	    set data(connect) {disconnect}
	} else {
	    set data(connect) {connect}
	}
    }
}