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
# a dspserver radio model
# handle the connection to dspserver
# manage the state of the dspserver controls
# provide hooks for the ui
#

package provide sdr::radio 1.0

package require Tcl
package require snit

package require sdrtype::types;	# snit types for option checking
package require sdr::server;	# the napan.com servers listing
package require sdr::channel;	# the tcp connection to dspserver
package require sdr::audio;	# audio playback (no mike as yet)
package require sdr::command;	# the commands sent to dspserver
package require sdr::filter;	# our filter syntax
package require sdr::util

package require sdr::band-data;	# service and band information
package require sdr::memory;	# memories for the radio


# define the radio widget
snit::type sdr::radio {

    # options
    # -service is the radio service
    # -service-values lists the valid radio services
    # -band defines the band withing the radio service
    # -band-values lists the bands
    # -frequency is the tuned frequency
    # -mode is the modulation mode
    # -mode-values lists the modes
    # -filter is the filter applied
    # -filter-values lists the filters available
    # -cw-pitch defines the offset from cw carrier frequency
    # -agc is the automatic gain control mode
    # -agc-values lists the agc modes available
    # tbc ...

    # options
    option -parent -configuremethod Configure
    option -service -configuremethod Configure
    option -service-values -configuremethod Configure2
    option -band -configuremethod Configure
    option -band-values -configuremethod Configure2
    option -frequency -default 14010000 -type sdrtype::hertz -configuremethod Configure
    option -mode -default {CWU} -type sdrtype::mode -configuremethod Configure
    option -mode-values -configuremethod Configure2
    option -filter -configuremethod Configure
    option -filter-values -configuremethod Configure2
    option -cw-pitch -default {600} -configuremethod Configure
    option -agc -default {SLOW} -type sdrtype::agc-mode -configuremethod Configure
    option -agc-values -configuremethod Configure
    option -name -configuremethod Configure2
    option -name-values -configuremethod Configure2
    option -sample-rate -default 3000 -configuremethod Configure2
    option -local-oscillator -default 0 -configuremethod Configure2
    option -spectrum-freq -default 0 -configuremethod Configure
    option -channel -configuremethod Configure
    option -channel-status -configuremethod Configure2

    component parent;		# parent

    # local data that is not options
    variable data -array {
	meter-listeners {}
	spectrum-listeners {}
	spectrum-xs {}
	spectrum-sr 0
	spectrum-n 0
	hardware {}
    }
    
    # constructor
    constructor {args} {
	puts stderr "sdr::radio {$args}"
	# give the command dictionary our self
	# hacky because how do we manage two or more?
	set ::sdr::command::radio $self;	# hacky
	$self configure \
	    -service-values [::sdr::band-data-services] \
	    -mode-values [sdrtype::mode cget -values] \
	    {*}$args
	# start listing servers
	server-request [list {*}[mymethod configure] -name-values]
    }

    ##
    ## one thing QtRadio did nicely was remember what you had done and
    ## bring it back
    ##
    # given a list of -option name, get the list of -option value pairs
    # needed to restore the options
    # the [concat {*}...] idiom is called lolcat on the tcl wiki
    method get-settings {args} {
	return [concat {*}[lmap option $args {list $option [$self cget $option]}]]
    }
    # given a list of -option value pairs restore the option values
    method set-settings {settings} {
	$self configure {*}$settings
    }
    # save the last settings for a service or a band in a service
    method save-last-settings {args} {
	switch [llength $args] {
	    1 { sdr::set-last [list {*}$args settings] [$self get-settings -band] }
	    2 { sdr::set-last [list {*}$args settings] [$self get-settings -frequency -mode -filter] }
	    default { error "wrong number of prefixes" }
	}
    }
    # restore the last settings for a service
    method restore-last-settings {args} {
	set last [sdr::get-last [list {*}$args settings]]
	if {$last eq {}} {
	    switch [llength $args] {
		1 {
		    set service [sdr::band-data-service {*}$args]
		    if {[catch {dict get $service band} band]} {
			# pick a random band
			set bands [dict get $service bands]
			set band [random-item $bands]
		    }
		    set last [list -band $band]
		}
		2 {
		    set service [sdr::band-data-service [lindex $args 0]]
		    set band [sdr::band-data-band {*}$args]
		    if {[catch {dict get $band freq} freq]} {
			if {[catch {dict get $service freq} freq]} {
			    set freq "[dict get $band low]"
			}
		    }
		    if {[catch {dict get $band mode} mode]} {
			if {[catch {dict get $service mode} mode]} {
			    set mode AM
			}
		    }
		    if {[catch {dict get $band filter} filter]} {
			if {[catch {dict get $service filter} filter]} {
			    set filter {-5000 .. 5000}
			}
		    }
		    set last [list -frequency [sdr::hertz $freq] -mode $mode -filter $filter]
		}
		default {
		    error "invalid last setting key: $arg"
		}
	    }

	}
	$self set-settings $last
    }

    ##
    ## configuration handlers
    ##
    ## the basic pattern is to ignore settings with no effect
    ## some values get compared as strings with ne
    ## and others get compared as numbers with !=
    ##
    ## some settings, eg -service and -band, trigger changes in
    ## other settings and save/restore of settings.
    ##
    ## other settings trigger sdr::command's to control dspserver
    ##
    ## this could be seriously simplified by hanging sdr::command's
    ## off monitors on the responsible options.
    ##
    
    # configure methods
    method {Configure -parent} {val} {
	set options(-parent) $val
	set parent $val
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
	    set options(-service) $val
	    # remember the new service as the "last used" service
	    sdr::set-last service $val
	    # restore the settings for the new service
	    $self restore-last-settings $options(-service)
	    # set the bands for the new service
	    $self configure -band-values [sdr::band-data-bands $options(-service)]
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
	    set options(-band) $val
	    $self save-last-settings $options(-service)
	    $self restore-last-settings $options(-service) $options(-band)
	}
    }
    method {Configure -frequency} {val} {
	if {$options(-frequency) != $val} {
	    set options(-frequency) $val
	    ::sdr::command::setfrequency [$self mode-offset-frequency]
	}
    }
    method {Configure -mode} {val} {
	if {$options(-mode) ne $val} {
	    if {$options(-mode) in {CWU CWL} || $val in {CWU CWL}} { set hasCW 1 } else { set hasCW 0 }
	    set options(-mode) $val
	    $self configure -filter-values [sdr::filters-get $val]
	    if {[lsearch $options(-filter-values) $options(-filter)] < 0} {
		$self configure -filter [random-item $options(-filter-values)]
	    }
	    ::sdr::command::setmode $options(-mode)
	    # these are only necessary when changing into or out of CW modes
	    if {$hasCW} {
		::sdr::command::setfrequency [$self mode-offset-frequency]
		::sdr::command::setfilter {*}[$self mode-offset-filter]
	    }
	}
    }
    method {Configure -filter} {val} { 
	if {$options(-filter) ne $val} {
	    set options(-filter) $val
	    ::sdr::command::setfilter {*}[$self mode-offset-filter]
	}
    }
    method {Configure -agc} {val} {
	if {$options(-agc) ne $val} {
	    set options(-agc) $val
	    ::sdr::command::setagc $options(-agc)
	}
    }
    method {Configure -cw-pitch} {val} {
	if {$options(-cw-pitch) != $val} {
	    set options(-cw-pitch) $val
	    if {$options(-mode) in {CWU CWL}} {
		::sdr::command::setfrequency [$self mode-offset-frequency]
		::sdr::command::setfilter {*}[$self mode-offset-filter]
	    }
	}
    }
    method {Configure -channel} {value} {
	if {$options(-channel) ne $value} {
	    set options(-channel) $value
	}
    }
    # this is the base configuration option
    # we just filter idempotent changes to avoid triggering
    # unnecessary activity on the wire
    method Configure2 {option value} {
	if {$options($option) ne $value} {
	    set options($option) $value
	}
    }
    
    ##
    ## maintain the list of dspservers
    ##
    method update-names {names} {
	if {$names ne [$win.name cget -values]} {
	    # verbose-puts "update-names $options(-name) and $names"
	    $win.name configure -values $names
	    # set options(-name) $options(-name)
	}
    }
    # add an unshared server that won't be found via napan.com
    method add-local {val} {
	server-local-install {*}$val; # {name addr port}
    }
    
    ##
    ## making, breaking, and testing a server connection
    ##
    
    # connect to a server
    method connect {} {
	if {[$self is-connected]} { error "socket is already connected" }
	set options(-channel) [::sdr::connect $self {*}[server-address-port $options(-name)]]
	# following along with QtRadio/UI.cpp/UI::connected()
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
	::sdr::command::startaudiostream 2000 8000 1 0;	# get parameters from somewhere
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
	# ::sdr::command::setrxagcslope # this one was commented out
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
	# ::sdr::command::settxalcstate
	# ::sdr::command::settxalcattack
	# ::sdr::command::settxalcdecay
	# ::sdr::command::settxalchang
	::sdr::command::*hardware?
	# start spectrum
	sdr::command::setfps 1024 4
	# enable notch filter false
	return 1
    }
    # disconnect from a server
    method disconnect {} { 
	if { ! [$self is-connected]} { error "socket is not connected" }
	verbose-puts "disconnecting self = $self"
	::sdr::disconnect $self $options(-channel)
	$self configure -channel {}
	# stop spectrum timer (what spectrum timer?)
	# set user none
	# set passwrod none
	# set host {}
	audio-out-stop
	$parent configure -hw {}
    }
    # is the radio connected to a server
    method is-connected {} { return [expr {$options(-channel) ne {}}] }
    # toggle the connection state
    method connecttoggle {} {
	if { ! [$self is-connected]} {
	    if {[catch {$self connect} error]} {
		set options(-channel) {}
		puts stderr $error\n$::errorInfo
		switch $error {
		    {*connection refused*} { 
			$self configure -channel-status {connection refused}
		    }
		    * {
			$self configure -channel-status {unknown connect error}
		    }
		}
	    } else {
		set data(connect) {disconnect}
		$self configure -channel-status {connected}
	    }
	} else {
	    set data(connect) {disconnect}
	    if {[catch {$self disconnect} error]} {
		puts $error\n$::errorInfo
		switch $error {
		    * {
			$self configure -channel-status {unknown disconnect error}
		    }
		}
	    } else {
		set options(-channel) {}
		$self configure -channel-status {disconnected}
	    }
	}
    }
    method channel-status {status} {
	switch $status {
	    * {
		puts stderr $status
		$self configure -channel-status {unknown error}
		$self disconnect $self $options(-channel)
	    }
	}
    }
    
    ##
    ## the -frequency specifies the carrier frequency tuned
    ## in the case of CW we want to tune offset from the 
    ## carrier by -cw-pitch Hertz.  the cw filters need to
    ## be offset by the same amount.
    ##
    
    # offset the tuned -frequency to accommodate the -cw-pitch
    method mode-offset-frequency {} {
	set f $options(-frequency)
	switch $options(-mode) {
	    CWU { return [expr {$f - $options(-cw-pitch)}]}
	    CWL { return [expr {$f + $options(-cw-pitch)}]}
	    default { return $f }
	}
    }
    # offset the -filter to be centered at the -cw-pitch
    method mode-offset-filter {} {
	foreach {lo hi} [::sdr::filter-parse $options(-filter)] break
	set cw $options(-cw-pitch)
	switch $options(-mode) {
	    CWU { set lo [expr {$cw+$lo}]; set hi [expr {$cw+$hi}] }
	    CWL { set lo [expr {-$cw+$lo}]; set hi [expr {-$cw+$hi}] }
	}
	return [list $lo $hi]
    }
    
    ##
    ## incoming packets from dspserver
    ##
    
    # process spectrum records received from the server
    method process-spectrum {main sub sr lo spectrum} {
	# spectrum record
	# coming at the frame rate specified by setfps
	# with the number of samples specified by setfps
	# each sample is an unsigned byte which is the bin level
	# measured as the dB's below zero
	
	# store some values as configuration options because
	# they rarely change
	$self configure -sample-rate $sr -local-oscillator $lo
	# pass the rest into update methods that can be subscribed
	$self meter-update $main $sub
	$self spectrum-update {*}[$self xy $spectrum]
	update
    }
    # convert incoming spectrum string from unsigned -dB in bytes
    # into freq dB coordinates as floats
    method xy {ystr} {
	binary scan $ystr {c*} ys
	# last byte is always 0
	set ys [lrange $ys 0 end-1]
	set n [llength $ys]
	# recompute the x coordinates for the spectrum
	if {$n != $data(spectrum-n) || $options(-sample-rate) != $data(spectrum-sr)} {
	    set data(spectrum-n) $n
	    set sr [set data(spectrum-sr) $options(-sample-rate)]
	    set data(spectrum-xs) {}
	    set maxf [expr {$sr/2.0}]
	    set minf [expr {-$maxf}]
	    set df [expr {double($sr)/$n}]
	    set data(spectrum-xs)
	    for {set i 0} {$i < $n} {incr i} {
		lappend data(spectrum-xs) [expr {$minf+$i*$df}]
	    }
	}
	set xs $data(spectrum-xs)
	# this is how QtRadio extracts the spectrum bytes
	set ys [lmap y $ys {expr {-($y&0xfff)}}]
	# keep min, max, and average levels
	set miny [tcl::mathfunc::min {*}$ys]
	set maxy [tcl::mathfunc::max {*}$ys]
	set avgy [expr {[tcl::mathop::+ {*}$ys]/double($n)}]
	
	set xy {}
	foreach x $xs y $ys {
	    lappend xy $x $y
	}
	return [list $xy $miny $maxy $avgy]
    }
    # process audio stream from the dspserver
    method process-audio {audio} {
	# puts stderr "audio [string length $audio]"
	audio-out-data $audio
    }
    # process answers to queries to the dspserver
    method process-answer {answer} {
	switch -glob $answer {
	    {q-server:*} {
		# parse out server name and can-tx?
	    }
	    {\*hardware\? OK sdrplay} {
		# set up hardware specific model and ui
		$parent configure -hw [lindex $answer end]
	    }
	    default {
		puts stderr "answer = {$answer}"
	    }
	}
    }
    # monitor configuration options
    method monitor {option prefix} {
	trace variable options($option) w [list {*}[mymethod monitor-fired] $prefix]
    }
    method monitor-fired {prefix name1 name2 op} {
	{*}$prefix $name2 $options($name2)
    }
    # listen for meter updates
    method meter-subscribe {callback} {
	lappend data(meter-listeners) $callback
    }
    method meter-update {main subrx} {
	foreach callback $data(meter-listeners) { {*}$callback $main $subrx }
    }
    # listen for spectrum updates
    method spectrum-subscribe {callback} {
	lappend data(spectrum-listeners) $callback
    }
    method spectrum-update {xy miny maxy avgy} {
	foreach callback $data(spectrum-listeners) { {*}$callback $xy $miny $maxy $avgy }
    }
    
}
