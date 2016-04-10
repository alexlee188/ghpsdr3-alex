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
# a dspserver connection widget
#

package provide sdr::dspserver 1.0

package require snit

package require sdr::channel
package require sdr::audio
package require sdr::band-data
package require sdr::command
package require sdr::filter
package require sdr::memory
package require sdr::servers

# define the radio widget
snit::type sdr::dspserver {

    # options
    option -service -default {} -configuremethod Configure
    option -service-values -default {} -configuremethod Configure2
    option -band -default {} -configuremethod Configure
    option -band-values -default {} -configuremethod Configure2
    option -frequency -default {} -configuremethod Configure
    option -mode -default {} -type sdrtype::mode -configuremethod Configure
    option -mode-values -default {} -configuremethod Configure2
    option -filter -default {} -configuremethod Configure
    option -filter-values -default {} -configuremethod Configure
    option -cw-pitch -default {600} -configuremethod Configure
    option -agc -default {SLOW} -type sdrtype::agc-mode -configuremethod Configure
    option -agc-values -default {} -configuremethod Configure
    option -name -default {} -configuremethod Configure2
    option -name-values -default {} -configuremethod Configure
    option -sample-rate -default 0 -configuremethod Configure
    option -local-oscillator -default 0 -configuremethod Configure
    option -main-meter -default 0 -configuremethod Configure
    option -subrx-meter -default 0 -configuremethod Configure
    option -min-level -default 0 -configuremethod Configure
    option -max-level -default 0 -configuremethod Configure
    option -avg-level -default 0 -configuremethod Configure
    option -spectrum -default 0 -configuremethod Configure
    option -spectrum-freq -default 0 -configuremethod Configure
    option -channel -default 0 -configuremethod Configure
    option -channel-status -default 0 -configuremethod Configure2
    option -text -default {tkradio} -configuremethod Configure
    option -ui -default {} -configuremethod Configure
    option -verbose -default 0 -configuremethod Configure2

    # constructor
    constructor args {
	verbose-puts "sdr::dspserver $args "
	$self configure \
	    -service-values [::sdr::band-data-services] \
	    -mode-values [sdr::get-modes] \
	    {*}$args
	servers-names-update [list {*}[mymethod configure] -name-values]
	# start listing servers
	servers-request
    }

    # saving and restoring settings
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
	$self set-settings [sdr::get-last [list {*}$args settings]]
    }

    # configure methods
    method {Configure -ui} {val} {
	set options(-ui) $val
	pack [$val .radio -text tkradio -radio $self] -fill both -expand true
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
	# puts "Configure -frequency $val"
	if {$options(-frequency) != $val} {
	    set options(-frequency) $val
	    # if {[winfo exists $win.freq]} { $win.freq configure -frequency $val }
	    ::sdr::command::setfrequency [$self mode-offset-frequency]
	}
    }
    method {Configure -mode} {val} {
	if {$options(-mode) ne $val} {
	    set options(-mode) $val
	    ::sdr::command::setmode $options(-mode)
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
	    set ::sdr::command::channel $value
	}
    }
    method Configure2 {option value} {
	if {$options($option) ne $value} {
	    set options($option) $value
	}
    }
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
    # connect to a server
    method connect {} {
	set options(-channel) [::sdr::connect $self {*}[server-address-port $options(-name)]]
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
	    ::sdr::disconnect $self $options(-channel)
	    $self configure -channel -1
	    # stop spectrum timer (what spectrum timer?)
	    # set user none
	    # set passwrod none
	    # set host {}
	    audio-out-stop
	}
    }
    # is the radio connected to a server
    method is-connected {} { return [expr {$options(-channel) != -1}] }
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
	$self configure \
	    -meter $main \
	    -subrx-meter $sub \
	    -sample-rate $sr \
	    -local-oscillator $lo \
	    -spectrum [$self xy $spectrum]
	# $win.sw update $xy
	# $win.meter update $main
	# $win.meter update $sub
    }
    # convert incoming spectrum string unsigned -dB in bytes
    # into freq dB coordinates
    method xy {ystr} {
	binary scan $spectrum {c*} ys
	# last byte is always 0
	set ys [lrange $ys 0 end-1]
	set n [llength $ys]
	# recompute the x coordinates for the spectrum
	if {$n != [llength $options(-spectrum-freq)]} {
	    set sr $options(-sample-rate)
	    set xs {}
	    set maxf [expr {$sr/2.0}]
	    set minf [expr {-$maxf}]
	    set df [expr {double($sr)/$n}]
	    for {set i 0} {$i < $n} {incr i} {
		lappend xs [expr {$minf+$i*$df}]
	    }
	    $self configure -spectrum-freq $xs
	}
	set xs $options(-spectrum-freq)
	# this is what QtRadio does with the spectrum bytes
	set ys [lmap y $ys {expr {-($y&0xfff)}}]
	# keep min, max, and average levels
	$self configure -min-level [tcl::mathfunc::min {*}$ys]
	$self configure -max-level [tcl::mathfunc::max {*}$ys]
	$self configure -avg-level [expr {[tcl::mathop::+ {*}$ys]/double($n)}]
	# tried to do this with lmap, but it needs lolcat
	# what it actually needs is canvas arg
	set xy {}
	foreach x $xs y $ys { lappend xy $x $y }
	return $xy
    }
    method process-audio {audio} {
	# puts stderr "audio [string length $audio]"
	audio-out-data $audio
    }
    method process-answer {answer} {
	puts stderr "answer = {$answer}"
    }
	
    method monitor {option prefix} {
	trace variable options($option) w [list {*}[mymethod monitor-fired] $prefix]
    }
    method monitor-fired {prefix name1 name2 op} {
	{*}$prefix $name2 $options($name2)
    }
}
