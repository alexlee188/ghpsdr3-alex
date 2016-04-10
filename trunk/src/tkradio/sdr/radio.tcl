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
# a messed up radio widget
#

package provide ui::radio 1.0

package require Tk
package require snit

package require channel

package require ui::frequency-display
package require ui::optionmenu

package require sdrtk::meter
package require sdrtk::spectrum-waterfall

package require sdr::audio
package require sdr::band-data
package require sdr::command
package require sdr::filter
package require sdr::memory
package require sdr::servers

namespace eval ::ui {}

proc ui::maxwidth {list} {
    set widths [lmap s $list {string length $s}]
    return [tcl::mathfunc::max {*}$widths]
}

# operating modes
proc ui::get-modes {} { return [::sdr::command::get-modes] }
snit::enum ui::modes-type -values [ui::get-modes]
# spectrum modes
proc ui::get-pwsmodes {} { return [::sdr::command::get-pwsmodes] }
snit::enum ui::pwsmodes-type -values [ui::get-pwsmodes]
# agc modes
proc ui::get-agcmodes {} { return [::sdr::command::get-agcmodes] }
snit::enum ui::agcmodes-type -values [ui::get-agcmodes]

# define the radio widget
# ah, mistake, there should be a radio widget which handles
# the basic logic of maintaining state and talking to dspserver,
# and then there should be one or more ui widgets which allow
# user control of the radio model
#
snit::widgetadaptor ::ui::radio {
    variable data -array {
	connect {connect}
	sample-rate 0
	spectrum-width 0
	xs {}
	miny 1000
	maxy -1000
	sumy 0.0
	ny 0
    }
    option -text -default {} -configuremethod Configure
    option -service -default {} -configuremethod Configure
    option -band -default {} -configuremethod Configure
    option -frequency -default {} -configuremethod Configure
    option -mode -default {} -type ui::modes-type -configuremethod Configure
    option -filter -default {} -configuremethod Configure
    option -cw-pitch -default {600} -configuremethod Configure
    option -agc -default {SLOW} -type ui::agcmodes-type -configuremethod Configure
    option -name -default {} -configuremethod Configure
    option -sample-rate -default 0 -configuremethod Configure
    option -local-oscillator -default 0 -configuremethod Configure

    delegate method * to hull
    delegate option * to hull

    constructor args {
	installhull using ttk::frame
	$self configure -service [sdr::get-last service] {*}$args
	verbose-puts "ui::radio $win $args "

	grid [sdrtk::meter $win.meter] -row 0 -column 0 -sticky nsew
	foreach opt {-frequency -service -band -mode -filter -name} {
	    set data($opt) $options($opt)
	    trace variable [myvar data($opt)] w [mymethod Set]
	}
	grid [ui::frequency-display $win.freq -frequency $data(-frequency)] -row 0 -column 1 -columnspan 6
	grid [ui::optionmenu $win.service -textvar [myvar data(-service)]] -row 1 -column 1
	grid [ui::optionmenu $win.band -textvar [myvar data(-band)]] -row 1 -column 2
	grid [ui::optionmenu $win.mode -textvar [myvar data(-mode)]] -row 1 -column 3
	grid [ui::optionmenu $win.filter -textvar [myvar data(-filter)]] -row 1 -column 4
	grid [ui::optionmenu $win.name -textvar [myvar data(-name)]] -row 1 -column 5
	grid [ttk::checkbutton $win.conn -width 10 \
		  -command [mymethod connecttoggle] \
		  -variable [myvar data(connect)] \
		  -onvalue {disconnect} -offvalue {connect} \
		  -textvar [myvar data(connect)]] -row 1 -column 6
	grid [sdrtk::spectrum-waterfall $win.sw] -row 2 -column 1 -columnspan 6

	$win.service configure -values [sdr::band-data-services] -width [ui::maxwidth [sdr::band-data-services]]
	$win.band configure -values [sdr::band-data-bands $options(-service)] -width [ui::maxwidth [sdr::band-data-all-bands]]
	$win.mode configure -values [ui::modes-type cget -values] -width [ui::maxwidth [ui::modes-type cget -values]]
	$win.filter configure -values [sdr::filters-get $options(-mode)] -width [ui::maxwidth [sdr::filters-get-all]]
	$win.name configure -values [servers-names] -width 8
	servers-names-update [mymethod update-names]
	# start listing servers
	servers-request
    }
    method get-settings {args} {
	set settings {}
	foreach option $args {
	    lappend settings $option [$self cget $option]
	}
	return $settings
    }
    method set-settings {settings} {
	foreach {option value} $settings {
	    $self configure $option $value
	}
    }
    method save-last-settings {args} {
	switch [llength $args] {
	    1 { sdr::set-last [list {*}$args settings] [$self get-settings -band] }
	    2 { sdr::set-last [list {*}$args settings] [$self get-settings -frequency -mode -filter] }
	    default { error "wrong number of prefixes" }
	}
    }
    method restore-last-settings {args} {
	puts "restore-last-settings {$args} => [sdr::get-last [list {*}$args settings]]"
	$self set-settings [sdr::get-last [list {*}$args settings]]
    }
    # called from a variable trace when an option menu alters a value
    method Set {name1 name2 op} {
	$self configure $name2 $data($name2)
	# update idletasks
    }
    # called from Configure to write a new option value everywhere
    method Syncdata {option value} {
	set options($option) $value
	if {[info exists data($option)] && $data($option) ne $value} {
	    set data($option) $value
	}
    }
    method {Configure -service} {val} { 
	# if the new service is different than the current service
	if {$options(-service) ne $val} {
	    # if the old service was not blank
	    if {$options(-service) ne {}} {
		# save the settings for the old service
		$self save-last-settings $options(-service)
	    }
	    # set the new service
	    $self Syncdata -service $val
	    # remember the new service as the "last used" service
	    sdr::set-last service $val
	    # restore the settings for the new service
	    $self restore-last-settings $options(-service)
	    # set the bands for the new service
	    if {[winfo exists $win.band]} {
		$win.band configure -values [sdr::band-data-bands $options(-service)]
	    }
	    # FIX.ME set the channels for the new service
	}
    }
    method {Configure -band} {val} { 
	# if the new band is different than the current service
	if {$options(-band) ne $val} {
	    # if the old service was not blank
	    if {$options(-band) ne {}} {
		$self save-last-settings $options(-service) $options(-band)
	    }
	    $self Syncdata -band $val
	    $self save-last-settings $options(-service)
	    $self restore-last-settings $options(-service) $options(-band)
	}
    }
    method {Configure -frequency} {val} {
	# puts "Configure -frequency $val"
	if {$options(-frequency) != $val} {
	    $self Syncdata -frequency $val
	    if {[winfo exists $win.freq]} {
		$win.freq configure -frequency $val
	    }
	    ::sdr::command::setfrequency [$self mode-offset-frequency]
	}
    }
    method {Configure -mode} {val} {
	if {$options(-mode) ne $val} {
	    $self Syncdata -mode $val
	    ::sdr::command::setmode [set options(-mode) $val]
	}
    }
    method {Configure -filter} {val} { 
	if {$options(-filter) ne $val} {
	    $self Syncdata -filter $val
	    ::sdr::command::setfilter {*}[$self mode-offset-filter]
	}
    }
    method {Configure -agc} {val} {
	if {$options(-agc) ne $val} {
	    $self Syncdata -agc $val
	    ::sdr::command::setagc [set options(-agc) $val]
	}
    }
    method {Configure -cw-pitch} {val} {
	if {$options(-cw-pitch) != $val} {
	    $self Syncdata -cw-pitch $val
	    if {$options(-mode) in {CWU CWL}} {
		::sdr::command::setfrequency [$self mode-offset-frequency]
		::sdr::command::setfilter {*}[$self mode-offset-filter]
	    }
	}
    }
    method {Configure -name} {val} {
	# puts "Configure -name $val and options(-name) is $options(-name)"
	if {$options(-name) ne $val} {
	    if {[$self is-connected]} { $self disconnect }
	    # puts "$self configure -name $val, with $win.name -values {[$win.name cget -values]}"
	    $self Syncdata -name $val
	}
    }
    method {Configure -local-oscillator} {val} {
	set options(-local-oscillator) $val
	# this gets to be -center-freq -tuned-freq
	# $win.sw configure -local-oscillator $val
    }
    method {Configure -sample-rate} {val} {
	set options(-sample-rate) $val
	$win.sw configure -sample-rate $val
    }
    method {Configure -text} {val} {
	if {$options(-text) ne $val} {
	    $self Syncdata -text $val
	    wm title . [set options(-text) $val]
	}
    }
    method update-names {names} {
	if {$names ne [$win.name cget -values]} {
	    # verbose-puts "update-names $options(-name) and $names"
	    $win.name configure -values $names
	    # set options(-name) $options(-name)
	}
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
    # add an unshared server that won't be found via napan.com
    method add-local {val} {
	server-local-install {*}$val; # {name addr port}
    }
    # connect to a server
    method connect {} {
	set ::channel [::ch::connect $self {*}[server-address-port $options(-name)]]
	::sdr::command::setclient tkradio
	::sdr::command::q-server
	# most of this should be done elsewhere
	# it resembles a band change
	::sdr::command::setfrequency [$self mode-offset-frequency]; # spectrum view follows
	::sdr::command::setmode $options(-mode)
	::sdr::command::setfilter {*}[$self mode-offset-filter]; # spectrum view follows
	# ui disable connect action
	# ui enable disconnect action
	# no, no, no $self connecttoggle, already done
	# ui enable subrx select
	# ui enable mute subrx 
	# ??? ::sdr::command::setpws $options(-pwsmode)
	## do we want to do audio on this channel?
	::sdr::command::setencoding 0
	# select local audio device
	::sdr::command::startaudiostream 2000 8000 1 0
	audio-out-start ALAW 8000
	::sdr::command::setpan 0.5
	::sdr::command::setagc $options(-agc)
	# if we're going to muck with different spectra, ...
	# ::sdr::command::setpws $options(-pwsmode)
	# set these when the ANF, NR, or NB is enabled
	# ::sdr::command::setanfvals
	# ::sdr::command::setnrvals
	# ::sdr::command::setnbvals
	# ::sdr::command::setsquelchval
	# ::sdr::command::setsquelchstate
	# ::sdr::command::setanf
	# ::sdr::command::setnr
	# ::sdr::command::setnb
	# remote connected
	::sdr::command::setrxdcblock 0
	::sdr::command::settxdcblock 0
	# ::sdr::command::setrxagcslope # this one was disabled
	# ::sdr::command::setrxagcattack
	# ::sdr::command::setrxagcdecay
	# ::sdr::command::setrxagchang
	# ::sdr::command::setfixedagc
	# ::sdr::command::setrxagchangthreshold
	# ::sdr::command::settxlevelerstate
	# ::sdr::command::settxlevelermaxgain
	# ::sdr::command::settxlevelerattack
	# ::sdr::command::settxlevelerdecay
	# ::sdr::command::settxlevelerhang
	# ::sdr::command::settxalcstate # this one was misspelled
	# ::sdr::command::settxalcattack
	# ::sdr::command::settxalcdecay
	# ::sdr::command::settxalchang
	# ::sdr::command::*hardware?
	# start spectrum
	sdr::command::setfps 1024 4
	# enable notch filter false
	return 1
    }
    # disconnect from a server
    method disconnect {} { 
	if {[$self is-connected]} {
	    verbose-puts "disconnecting self = $self"
	    ::ch::disconnect $self $::channel
	    set ::channel -1
	    # stop spectrum timer (what spectrum timer?)
	    # set user none
	    # set passwrod none
	    # set host {}
	    audio-out-stop
	}
    }
    # is the radio connected to a server
    method is-connected {} { return [expr {$::channel != -1}] }
    # offset the tuned frequency to accommodate the cw pitch
    method mode-offset-frequency {} {
	set f $options(-frequency)
	switch $options(-mode) {
	    CWU { return [expr {$f - $options(-cw-pitch)}]}
	    CWL { return [expr {$f + $options(-cw-pitch)}]}
	    default { return $f }
	}
    }
    # offset the filter to the center of the cw pitch
    method mode-offset-filter {} {
	foreach {lo hi} [::sdr::filter-parse $options(-filter)] break
	set cw $options(-cw-pitch)
	switch $options(-mode) {
	    CWU { set lo [expr {$cw+$lo}]; set hi [expr {$cw+$hi}] }
	    CWL { set lo [expr {-$cw+$lo}]; set hi [expr {-$cw+$hi}] }
	}
	return [list $lo $hi]
    }
    # process spectrum records received from the server
    method process-spectrum {main sub sr lo spectrum} {
	# spectrum record
	# coming at the frame rate specified by setfps
	# with the number of samples specified by setfps
	# each sample is an unsigned byte which is the bin level
	# measured as the dB's below zero
	if {$options(-sample-rate) != $sr} {
	    $self configure -sample-rate $sr
	}
	if {$options(-local-oscillator) != $lo} {
	    $self configure -local-oscillator $lo
	}
	set xy [$self xy $spectrum]
	$win.sw update $xy
	$win.meter update $main
	# $win.meter update $sub
	# puts stderr "spectrum $main $sub $sr $lo [string length $spectrum] min .. max is $data(miny) .. $data(maxy) avgy is [expr {$data(sumy)/$data(ny)}]"
	# bring the display up to date
	after 1 {update idletasks}
    }
    # minimum y value in spectra
    method miny {} { return $data(miny) }
    # maximum y value in spectra
    method maxy {} { return $data(maxy) }
    # convert incoming spectrum string unsigned -dB
    method xy {ystr} {
	binary scan $ystr {c*} ys
	set ys [lrange $ys 0 end-1]
	# must be a dspserver bug sending a garbage byte
	# puts "last byte [lindex $ys end]"
	set n [llength $ys]
	set sr $options(-sample-rate)
	if {$data(sample-rate) != $sr || $data(spectrum-width) != $n} {
	    set data(sample-rate) $sr
	    set data(spectrum-width) $n
	    set data(xs) {}
	    set maxf [expr {$sr/2.0}]
	    set minf [expr {-$maxf}]
	    set df [expr {double($sr)/$n}]
	    for {set i 0} {$i < $n} {incr i} {
		lappend data(xs) [expr {$minf+$i*$df}]
	    }
	}
	set ys [lmap y $ys {expr {-($y&0xfff)}}]
	set data(miny) [tcl::mathfunc::min $data(miny) {*}$ys]
	set data(maxy) [tcl::mathfunc::max $data(maxy) {*}$ys]
	set data(sumy) [tcl::mathop::+ $data(sumy) {*}$ys]
	incr data(ny) $n
	# puts "min .. max is $data(miny) .. $data(maxy) avgy is [expr {$data(sumy)/$data(ny)}]"
	foreach x $data(xs) y $ys { lappend xy $x $y }
	return $xy
    }
    method process-audio {audio} {
	# puts stderr "audio [string length $audio]"
	audio-out-data $audio
    }
    method process-answer {answer} {
	puts stderr "answer = {$answer}"
    }
}
