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
# plain window radio ui widget
#

package provide sdrui::pw-radio 1.0

package require Tcl
package require Tk
package require snit

package require sdr::util

package require ui::frequency-display
package require ui::optionmenu

package require sdrtk::meter
package require sdrtk::spectrum-waterfall

namespace eval ::sdrui {}

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

snit::widgetadaptor sdrui::pw-radio {
    option -parent -readonly true -configuremethod Configure
    option -radio -readonly true
    option -text -default {}
    option -name -default {}
    option -channel-status -default {}
    option -hw -default {}

    delegate method * to hull
    delegate option * to hull

    variable data -array {
	connect {connect}
	hardware {}
    }

    component parent
    
    constructor args {
	installhull using ttk::frame
	$self configure {*}$args

	verbose-puts "sdrui::pw-radio $win $args "

	# frequency display
	grid [ui::frequency-display $win.freq \
		  -value [$self rget -frequency] \
		  -command [list {*}[mymethod rconfigure] -frequency] \
		 ] -row 0 -column 0 -rowspan 3
	$self rmonitor -frequency

	# s-meter
	grid [sdrtk::meter $win.meter] -row 3 -column 0 -sticky nsew
	$parent.radio meter-subscribe [list $win.meter update]

	# radio service selector
	grid [ui::optionmenu $win.service \
		  -value [$self rget -service] \
		  -values [$self rget -service-values] \
		  -width [sdr::maxwidth [sdr::band-data-services]] \
		  -command [list {*}[mymethod rconfigure] -service] \
		 ] -row 0 -column 1 -sticky nsew
	$self rmonitor -service -service-values

	# band selector
	grid [ui::optionmenu $win.band \
		  -value [$self rget -band] \
		  -values [$self rget -band-values] \
		  -width [sdr::maxwidth [sdr::band-data-all-bands]] \
		  -command [list {*}[mymethod rconfigure] -band] \
		 ] -row 0 -column 2 -sticky nsew
	$self rmonitor -band -band-values

	# mode selector
	grid [ui::optionmenu $win.mode \
		  -value [$self rget -mode] \
		  -values [$self rget -mode-values] \
		  -width [sdr::maxwidth [sdrtype::mode cget -values]] \
		  -command [list {*}[mymethod rconfigure] -mode] \
		 ] -row 1 -column 1 -sticky nsew
	$self rmonitor -mode -mode-values

	# filter selector
	grid [ui::optionmenu $win.filter \
		  -value [$self rget -filter] \
		  -values [sdr::filters-get [$self rget -mode]] \
		  -width [sdr::maxwidth [sdr::filters-get-all]] \
		  -command [list {*}[mymethod rconfigure] -filter] \
		 ] -row 1 -column 2 -sticky nsew
	$self rmonitor -filter -filter-values

	# server selector
	grid [ui::optionmenu $win.name \
		  -value [$self rget -name] \
		  -values [$self rget -name-values] \
		  -width 8 \
		  -command [list {*}[mymethod rconfigure] -name] \
		 ] -row 2 -column 1 -sticky nsew
	$self rmonitor -name -name-values

	# connect to server
	grid [ttk::checkbutton $win.conn \
		  -textvar [myvar data(connect)] \
		  -width 10 \
		  -variable [myvar data(connect)] \
		  -onvalue {disconnect} \
		  -offvalue {connect} \
		  -command [mymethod connecttoggle] \
		 ] -row 2 -column 2 -sticky nsew
	$self rmonitor -channel-status

	# spectrum and waterfall
	grid [sdrtk::spectrum-waterfall $win.sw \
		  -command [list {*}[mymethod rconfigure] -frequency] \
		 ] -row 4 -column 0 -columnspan 3 -sticky nsew
	$self rmonitor -frequency -local-oscillator -sample-rate
	$parent.radio spectrum-subscribe [list $win.sw update]

	foreach option {-text -channel-status -name} {
	    $self monitor $option [mymethod window-title]
	}
	
	foreach c {0 1 2 3 4 5 6}  {
	    grid columnconfigure $win $c -weight 1
	}

	grid rowconfigure $win 0 -weight 0
	grid rowconfigure $win 1 -weight 0
	grid rowconfigure $win 2 -weight 1
		
	# monitor the radio -hw option for activity
	$self cmonitor $parent -hw
    }

    method {Configure -parent} {val} {
	set options(-parent) $val
	set parent $val
    }
    # rewrite the window title to reflect statusa
    method window-title {args} {
	#puts "managing window title"
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
    ## manipulations on components of the radio
    ##
    method cmonitor {c args} {
	foreach option $args {
	    $c monitor $option [list {*}[mymethod cmonitor-fired] $c]
	    $self cmonitor-fired $c $option [$c cget $option]
	}
    }
    method cmonitor-fired {c option value} {
	# verbose-puts "cmonitor-fired in .radio $c $option $value"
	switch -- $option {
	    -main-meter -
	    -subrx-meter { $win.meter configure $option $value }
	    -frequency {
		$win.freq configure -value $value
		if {[winfo exists $win.sw]} {$win.sw configure $option $value}
	    }
	    -service { $win.service configure -value $value }
	    -service-values { $win.service configure -values $value }
	    -band { $win.band configure -value $value }
	    -band-values { $win.band configure -values $value }
	    -mode { $win.mode configure -value $value }
	    -mode-values { $win.mode configure -values $value }
	    -filter { $win.filter configure -value $value }
	    -filter-values { $win.filter configure -values $value }
	    -name { 
		$win configure $option $value
		catch {$win.name configure -value $value}
	    }
	    -name-values { $win.name configure -values $value }
	    -channel-status { $win configure $option $value }
	    -spectrum -
	    -sample-rate -
	    -local-oscillator { $win.sw configure $option $value }
	    -hw {
		# $win configure $option $value
		# puts stderr "cmonitor $c -hw fired value={$value}"
		# interesting problem here, need to create and display
		# but need to undisplay and destroy, so the functions
		# happen in opposite orders
		# and the parent only knows to create when the ui triggers
		set hw [$parent cget -hw]
		set ui [$parent cget -ui]
		puts stderr "parent $parent -hw is {$hw} -ui is {$ui}"
		if {$hw ne $options(-hw)} {
		    if {$options(-hw) eq {}} {
			# load new hardware
			if {[catch {package require sdrui::$ui-radio-$hw} error]} {
			    # no such hardware ui module
			    puts stderr "package require sdrui::$ui-radio-$hw failed: $error"
			    return
			}
			if {[catch {sdrui::$ui-radio-$hw $win.hw -parent $parent} error]} {
			    puts stderr "sdrui::$ui-radio-$hw $win.hw -parent $parent failed: $error"
			    return
			}
			# choices, upper right
			# grid $win.hdw -row 0 -column 7
			# lower left
			grid $win.hw -row 5 -column 0
			set options(-hw) $hw
		    } else {
			grid forget $win.hw
			destroy $win.hw
			set options(-hw) {}
		    }
		}
	    }
	    default {
		error "unknown cmonitor-fired $c for option {$option}"
	    }
	}
    }
    ##
    ## manipulations on the radio model
    ##
    method rget {opt} { return [$parent.radio cget $opt] }
    method rmonitor {args} {
	$self cmonitor $parent.radio {*}$args
    }
    method rmonitor-fired {option value} {
	verbose-puts "rmonitor-fired in .radio $option $value"
    }
    method rconfigure {option value} {
 	$parent.radio configure $option $value
    }
    
    ##
    ## delegate the connection to the radio model
    ## 
    method connecttoggle {} {
	$parent.radio connecttoggle
	if {[$parent.radio is-connected]} {
	    set data(connect) {disconnect}
	} else {
	    set data(connect) {connect}
	}
    }
}
