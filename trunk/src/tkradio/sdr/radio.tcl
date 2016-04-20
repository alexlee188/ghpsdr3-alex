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
package require sdr::filter;	# our filter syntax
package require sdr::util

package require sdr::band-data;	# service and band information
package require sdr::memory;	# memories for the radio


# define the radio widget
snit::type sdr::radio {
    # options which have a dspserver command 
    # that simply sends the option value to dspserver
    # map the option to the command, used to set a monitor
    # and to automatically send the option value when it changes
    # sdrom is called nb2 on the QtRadio menues
    #
    # ah, better idea, combine this with defaults.tcl, add in types with ranges
    #
    # still need to add bandscope and panadapter commands for notch filters
    # and the graphic equalizers
    typevariable optionmonitor [dict create {*}{
	-frequency {}
	-mode {}
	-filter {}
	-cw-pitch {}
	-agc setagc
	-spectrum-width {}
	-spectrum-fps {}

	-rx-agc-fixed-gain setfixedagc

	-rx-agc-attack setrxagcattack
	-rx-agc-decay setrxagcdecay
	-rx-agc-hang setrxagchang
	-rx-agc-hang-threshold setrxagchangthreshold
	-rx-agc-max-gain setrxagcmaxgain
	-rx-agc-slope setrxagcslope

	-tx-alc-attack settxalcattack
	-tx-alc-decay settxalcdecay
	-tx-alc-hang settxalchang
	-tx-alc-enable settxalcstate

	-anf-delay {}
	-anf-gain {}
	-anf-leak {}
	-anf-taps {}
	-anf-enable setanf

	-tx-leveler-attack settxlevelerattack
	-tx-leveler-decay settxlevelerdecay
	-tx-leveler-hang settxlevelerhang
	-tx-leveler-max-gain settxlevelermaxgain
	-tx-leveler-enable settxlevelerstate

	-nb-threshold setnbvals
	-nb-enable setnb

	-nr-delay {}
	-nr-gain {}
	-nr-leak {}
	-nr-taps {}
	-nr-enable setnr

	-rx-dc-block-enable setrxdcblock
	-rx-dc-block-gain setrxdcblockgain

	-rx-iq-gain-correct rxiqgaincorrectval
	-rx-iq-phase-correct rxiqphasecorrectval
	-rx-iq-method setiqmethod
	-rx-iq-mu rxiqmuval
	-rx-iq-enable setiqenable

	-rx-gr-eq-enable setrxgreqcmd
	
	-tx-gr-eq-enable settxgreqcmd
	
	-sdrom-threshold setsdromvals
	-sdrom-enable setsdrom

	-tx-iq-gain-correct txiqgaincorrectval
	-tx-iq-phase-correct txiqphasecorrectval
	-tx-iq-enable settxiqenable
	
	-allow-tx {}

	-tx-dc-block-gain {} 
	-tx-dc-block-enable settxdcblock

	-pws-mode setpwsmode
	-squelch-value setsquelchval
	-squelch-enable setsquelchstate
	-sub-rx-gain {}

	-rx-output-gain setrxoutputgain

	-sub-rx-frequency setsubrxfrequency
	-sub-rx subrx
	-sub-rx-gain setsubrxoutputgain
	-sub-rx-pan setsubrxpan
	
	-preamp setpreamp
	-record record
	-tx-am-carrier-level settxamcarrierlevel

	-mox mox

	-zoom zoom
	-window window
	-spectrum-polyphase setspectrumpolyphase
	
    }]
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
    option -parent -configuremethod Configure -readonly true
    option -service -configuremethod Configure
    option -service-values -configuremethod Configure2
    option -band -configuremethod Configure
    option -band-values -configuremethod Configure2
    option -frequency -default 14010000 -type sdrtype::hertz -configuremethod Configure
    option -mode -default CWL -type sdrtype::mode -configuremethod Configure
    option -mode-values -configuremethod Configure2
    option -filter -default {-250 .. 250} -configuremethod Configure
    option -filter-values -configuremethod Configure2
    option -cw-pitch -default {600} -configuremethod Configure
    option -agc -default {SLOW} -type sdrtype::agc-mode -configuremethod Configure2
    option -agc-values -configuremethod Configure2
    option -spectrum-width 1024
    option -spectrum-fps 4
    
    # more options, not implemented
    option -rx-agc-attack -default 2
    option -rx-agc-decay -default 250
    option -rx-agc-fixed-gain -default 22
    option -rx-agc-hang -default 250
    option -rx-agc-hang-threshold -default 0
    option -rx-agc-max-gain -default 75
    option -rx-agc-slope -default 0

    option -tx-alc-attack -default 2
    option -tx-alc-decay -default 10
    option -tx-alc-hang -default 500
    option -tx-alc-enable -default 0

    option -anf-delay -default 8
    option -anf-gain -default 32
    option -anf-leak -default 1
    option -anf-taps -default 64
    option -anf-enable -default 0

    option -cw-pitch -default 600

    option -tx-leveler-attack -default 2
    option -tx-leveler-decay -default 500
    option -tx-leveler-enable -default 0
    option -tx-leveler-hang -default 500
    option -tx-leveler-max-gain -default 5

    option -nb-threshold -default 20
    option -nb-enable -default 0

    option -nr-delay -default 8
    option -nr-gain -default 16
    option -nr-leak -default 10
    option -nr-taps -default 64
    option -nr-enable -default 0

    option -rx-dc-block-enable -default 0
    option -rx-dc-block-gain -default 32

    option -rx-iq-gain-correct -default 0
    option -rx-iq-phase-correct -default 0
    option -rx-iq-method -default 2
    option -rx-iq-mu -default 25
    option -rx-iq-enable -default 2

    option -sdrom-threshold -default 20
    option -sdrom-enable -default 0

    option -tx-iq-gain-correct -default 0
    option -tx-iq-phase-correct -default 0
    option -tx-iq-enable -default 0

    option -allow-tx -default 0
    option -tx-dc-block-enable -default 0

    option -gain -default 100
    option -pws-mode -default 0
    option -squelch-value -default 0
    option -squelch-enable -default 0
    option -sub-rx-gain -default 100
    
    option -mox -default 0
    option -preamp -default 0
    option -record -default 0
    option -rx-gr-eq-enable -default 0
    option -rx-output-gain -default 0
    option -spectrum-polyphase -default 0
    option -sub-rx -default 0
    option -sub-rx-frequency -default 0
    option -sub-rx-pan -default 0.5
    option -tx-am-carrier-level -default 0.5
    option -tx-dc-block-gain
    option -tx-gr-eq-enable
    option -window
    option -zoom

    component parent;		# parent

    # local data that is not options
    variable data -array {
    }
    
    # constructor
    constructor {args} {
	# puts stderr "sdr::radio {$args}"
	$self configure {*}$args
	$self configure -service-values [::sdr::band-data-services] -mode-values [sdrtype::mode cget -values]
	# we monitor changes in a set of options
	# and automatically transmit new values
	foreach opt [dict keys $optionmonitor] {
	    if {[$self info options $opt] eq {}} {
		# puts "option $opt is not defined"
		continue
	    }
	    if {[dict get $optionmonitor $opt] eq {}} {
		# puts "option $opt is skipped"
		continue
	    }
	    $self monitor $opt [mymethod option-transmit]
	    # puts [format {%24s %24s} $opt $optionmonitor($opt)]
	}
    }

    method option-transmit {opt val} {
	$parent.command [dict get $optionmonitor $opt] $val
    }

    ##
    ## one thing QtRadio did nicely was remember what you had done and
    ## bring it back, 
    ##
    ## but this kind of memory depends on there only being one radio
    ## that is remembering the last settings
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
	    puts stderr "$self configure -frequency $val"
	    set options(-frequency) $val
	    $parent.command setfrequency [$self mode-offset-frequency]
	}
    }
    method {Configure -mode} {val} {
	# a -mode implies a set of filters, but depending on the order of 
	# configuration options we may get the -filter-values before or after
	# the -mode, so the -filter-values may be wrong for the -filter.
	# in a sense the -mode should contain all the parts of the mode,
	# so demodulation, filter width and center, cw pitch, ...
	# so it should be an array of settings, and the contents of the
	# array may differ between -mode's
	# so the voice -mode's have squelch settings and agc settings may
	# vary between modes, too.
	if {$options(-mode) ne $val} {
	    set hasCW [expr {$options(-mode) in {CWU CWL} || $val in {CWU CWL}}]
	    set options(-mode) $val
	    $self configure -filter-values [sdr::filters-get $val]
	    $parent.command setmode $options(-mode)
	    # these are only necessary when changing into or out of CW modes
	    # but they can only be done when both mode, filter, and offset
	    # have been set properly
	    if {$hasCW} {
		$parent.command setfrequency [$self mode-offset-frequency]
		$parent.command setfilter {*}[$self mode-offset-filter]
	    }
	}
    }
    method {Configure -filter} {val} { 
	if {$options(-filter) ne $val} {
	    set options(-filter) $val
	    if {[lsearch $options(-filter-values) $options(-filter)] < 0} {
		$self configure -filter [random-item $options(-filter-values)]
	    }
	    $parent.command setfilter {*}[$self mode-offset-filter]
	}
    }
    method {Configure -cw-pitch} {val} {
	if {$options(-cw-pitch) != $val} {
	    set options(-cw-pitch) $val
	    if {$options(-mode) in {CWU CWL}} {
		$parent.command setfrequency [$self mode-offset-frequency]
		$parent.command setfilter {*}[$self mode-offset-filter]
	    }
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
    ## making, breaking, and testing a server connection
    ##
    method connect {} {
	# following after QtRadio/UI.cpp/UI::connected()
	$parent.command setclient tkradio
	$parent.command q-server
	# most of this should be done elsewhere, it resembles a band change
	$parent.command setfrequency [$self mode-offset-frequency]; # spectrum view follows
	$parent.command setmode $options(-mode)
	$parent.command setfilter {*}[$self mode-offset-filter]; # spectrum view follows
	# ui enable subrx select
	# ui enable mute subrx 
	## audio output encoding
	$parent.command setencoding [$parent get-audio-sample-format]
	## audio output buffer-size, sample-rate, channels, and input sample encoding
	## changing buffer size is a waste of time, dspserver is stubborn
	$parent.command startaudiostream 2000 [$parent get-audio-sample-rate] 1 ALAW
	$parent.command setpan 0.5
	$parent.command setagc $options(-agc)
	## squelch
	$parent.command setsquelchval $options(-squelch-value)
	$parent.command setsquelchstate $options(-squelch-enable)
	## spectrum type
	$parent.command setpwsmode $options(-pws-mode)
	## set these when the ANF, NR, or NB is enabled
	$parent.command setanfvals $options(-anf-taps) $options(-anf-delay) $options(-anf-gain) $options(-anf-leak)
	$parent.command setnrvals $options(-nr-taps) $options(-nr-delay) $options(-nr-gain) $options(-nr-leak)
	$parent.command setnbvals $options(-nb-threshold)
	$parent.command setanf $options(-anf-enable)
	$parent.command setnr $options(-nr-enable)
	$parent.command setnb $options(-nb-enable)
	## remote connected
	$parent.command setrxdcblock $options(-rx-dc-block-enable)
	$parent.command settxdcblock $options(-tx-dc-block-enable)
	## receive agc
	## -rx-agc-slope was commented out
	$parent.command setrxagcattack $options(-rx-agc-attack)
	$parent.command setrxagcdecay $options(-rx-agc-decay)
	$parent.command setrxagchang $options(-rx-agc-hang)
	$parent.command setrxagchangthreshold $options(-rx-agc-hang-threshold)
	$parent.command setfixedagc $options(-rx-agc-fixed-gain)
	$parent.command setrxagcslope $options(-rx-agc-slope)
	## transmit leveler
	$parent.command settxlevelerstate $options(-tx-leveler-enable)
	$parent.command settxlevelermaxgain $options(-tx-leveler-max-gain)
	$parent.command settxlevelerattack $options(-tx-leveler-attack)
	$parent.command settxlevelerdecay $options(-tx-leveler-decay)
	$parent.command settxlevelerhang $options(-tx-leveler-hang)
	## transmit alc
	$parent.command settxalcattack $options(-tx-alc-attack)
	$parent.command settxalcdecay $options(-tx-alc-decay)
	$parent.command settxalchang $options(-tx-alc-hang)
	$parent.command settxalcstate $options(-tx-alc-enable)
	## hardare query
	$parent.command *hardware?
	# start spectrum
	$parent.command setfps $options(-spectrum-width) $options(-spectrum-fps)
	# enable notch filter false
	# return 1
    }
    method disconnect {} { 
	# $parent disconnect
	# stop spectrum timer (what spectrum timer?)
	# set user none
	# set password none
	# set host {}
	$parent configure -hw {}
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
    
}
