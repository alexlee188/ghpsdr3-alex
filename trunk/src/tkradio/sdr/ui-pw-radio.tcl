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
    option -text -default {}
    option -name -default {} -configuremethod Configure
    option -connect-status -default {}
    option -frequency -default 100000 -configuremethod Configure

    component parent

    component frequency
    component meter
    component service
    component band
    component mode
    component filter
    component name
    component conn
    component sw
    
    #delegate option -frequency -to frequency as -value
    delegate option -service to service as -value
    delegate option -service-values to service as -values
    delegate option -band to band as -value
    delegate option -band-values to band as -values
    delegate option -mode to mode as -value
    delegate option -mode-values to mode as -values
    delegate option -filter to filter as -value
    delegate option -filter-values to filter as -values
    #delegate option -name to name as -value
    delegate option -name-values to name as -values
    #delegate option -frequency -to sw
    delegate option -local-oscillator to sw
    delegate option -sample-rate to sw

    delegate method * to hull
    delegate option * to hull

    variable data -array {
	connect {connect}
    }

    constructor args {
	installhull using ttk::frame
	$self configure {*}$args

	# verbose-puts "sdrui::pw-radio $win $args "

	# frequency display
	install frequency using ui::frequency-display $win.freq \
	    -value [$parent.radio cget -frequency] \
	    -command [list $parent.radio configure -frequency]

	# s-meter
	install meter using sdrtk::meter $win.meter

	# radio service selector
	install service using ui::optionmenu $win.service \
	    -value [$parent.radio cget -service] \
	    -values [$parent.radio cget -service-values] \
	    -width [sdr::maxwidth [sdr::band-data-services]] \
	    -command [list $parent.radio configure -service]

	# band selector
	install band using ui::optionmenu $win.band \
	    -value [$parent.radio cget -band] \
	    -values [$parent.radio cget -band-values] \
	    -width [sdr::maxwidth [sdr::band-data-all-bands]] \
	    -command [list $parent.radio configure -band]

	# mode selector
	install mode using ui::optionmenu $win.mode \
		  -value [$parent.radio cget -mode] \
		  -values [$parent.radio cget -mode-values] \
		  -width [sdr::maxwidth [sdrtype::mode cget -values]] \
		  -command [list $parent.radio configure -mode]

	# filter selector
	install filter using ui::optionmenu $win.filter \
	    -value [$parent.radio cget -filter] \
	    -values [sdr::filters-get [$parent.radio cget -mode]] \
	    -width [sdr::maxwidth [sdr::filters-get-all]] \
	    -command [list $parent.radio configure -filter]

	# server selector
	install name using ui::optionmenu $win.name \
	    -value [$parent cget -name] \
	    -values [$parent cget -name-values] \
	    -width 8 \
	    -command [list $parent configure -name]

	# connect to server
	install conn using ttk::checkbutton $win.conn \
	    -textvar [myvar data(connect)] \
	    -width 10 \
	    -variable [myvar data(connect)] \
	    -onvalue {disconnect} \
	    -offvalue {connect} \
	    -command [mymethod connecttoggle]

	# spectrum and waterfall
	install sw using sdrtk::spectrum-waterfall $win.sw \
	    -command [list $parent.radio configure -frequency]

	# events
	$self rmonitor \
	    -frequency \
	    -service -service-values \
	    -band -band-values \
	    -mode -mode-values \
	    -filter -filter-values
	$self pmonitor -name -name-values -connect-status \
	    -local-oscillator -sample-rate

	$parent meter-subscribe [list $win.meter update]
	$parent spectrum-subscribe [list $win.sw update]
	$self monitor {-text -connect-status -name} [mymethod window-title]
	
	# layout
	grid $frequency -row 0 -column 0 -rowspan 3
	grid $service -row 0 -column 1 -sticky nsew
	grid $band -row 0 -column 2 -sticky nsew
	grid $mode -row 1 -column 1 -sticky nsew
	grid $filter -row 1 -column 2 -sticky nsew
	grid $name -row 2 -column 1 -sticky nsew
	grid $conn -row 2 -column 2 -sticky nsew
	grid $meter -row 3 -column 0 -sticky nsew
	grid [ttk::button $win.tb1 -text {Test} -command [mymethod test]] -row 3 -column 1
	grid $sw -row 4 -column 0 -columnspan 3 -sticky nsew

	foreach c {0 1 2 3 4 5 6}  {
	    grid columnconfigure $win $c -weight 1
	}

	grid rowconfigure $win 0 -weight 0
	grid rowconfigure $win 1 -weight 0
	grid rowconfigure $win 2 -weight 0
	grid rowconfigure $win 3 -weight 0
	grid rowconfigure $win 4 -weight 1
    }

    method {Configure -parent} {val} {
	set options(-parent) $val
	set parent $val
    }
    method {Configure -frequency} {val} {
	# puts stderr "$self configure -frequency $val"
	if {$options(-frequency) != $val} {
	    set options(-frequency) $val
	    $win.freq configure -value $val
	    $win.sw configure -frequency $val
	}
    }
    method {Configure -name} {val} { 
	if {$options(-name) ne $val} {
	    set options(-name) $val
	    $win.name configure -value $val
	}
    }

    method test {} {
	puts "$frequency configure\n\t[join [$frequency configure] \n\t]"
    }

    # rewrite the window title to reflect statusa
    method window-title {args} {
	#puts "managing window title"
	wm title . "$options(-text) -- $options(-name) $options(-connect-status)"
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
    ## monitor radio or parent configuration options
    ##
    method cmonitor {c args} {
	foreach option $args {
	    $c monitor $option [mymethod configure]
	    # copy the initial values from the component
	    $self configure $option [$c cget $option]
	}
    }
    method pmonitor {args} { $self cmonitor $parent {*}$args } 
    method rmonitor {args} { $self cmonitor $parent.radio {*}$args }
    ##
    ## delegate the connection to the radio model
    ## 
    method connecttoggle {} {
	if {[$parent connecttoggle]} {
	    set data(connect) {disconnect}
	} else {
	    set data(connect) {connect}
	}
    }
    ##
    ## map or unmap the hardware ui
    ##
    method map-hardware {} {
	grid $win.hw -row 5 -column 0 -columnspan 7
    }
    method unmap-hardware {} {
	grid forget $win.hw
    }
}
