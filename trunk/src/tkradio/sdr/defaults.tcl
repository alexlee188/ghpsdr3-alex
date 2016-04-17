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

# the default values for options that might require reset
package provide sdr::defaults 1.0

namespace eval ::sdr {}

namespace eval ::sdr::defaults {
    # skipped audio equalizer
    set defaults {
	{radio -rx-agc-attack 2}
	{radio -rx-agc-decay 250}
	{radio -rx-agc-fixed-gain 22}
	{radio -rx-agc-hang 250}
	{radio -rx-agc-hang-threshold 0}
	{radio -rx-agc-max-gain 75}
	{radio -rx-agc-slope 0}

	{radio -tx-alc-attack 2}
	{radio -tx-alc-decay 10}
	{radio -tx-alc-enabled 0}
	{radio -tx-alc-hang 500}

	{radio -anf-delay 8}
	{radio -anf-gain 32}
	{radio -anf-leak 1}
	{radio -anf-taps 64}

	{radio -cw-pitch 600}

	{radio -tx-leveler-attack 2}
	{radio -tx-leveler-decay 500}
	{radio -tx-leveler-enabled 0}
	{radio -tx-leveler-hang 500}
	{radio -tx-leveler-max-gain 5}

	{radio -nb-threshold 20}

	{radio -nr-delay 8}
	{radio -nr-gain 16}
	{radio -nr-leak 10}
	{radio -nr-taps 64}

	{radio -rx-dc-block 0}
	{radio -rx-dc-block-gain 32}

	{radio -rx-iq-gain-correct 0}
	{radio -rx-iq-phase-correct 0}
	{radio -rx-iq-method 2}
	{radio -rx-iq-mu 25}
	{radio -rx-iq-on-off 2}

	{radio -sdrom-threshold 20}

	{radio -tx-iq-gain-correct 0}
	{radio -tx-iq-phase-correct 0}
	{radio -tx-iq-on-off=0}

	{radio -allow-tx 0}
	{radio -tx-dc-block 0}

	{radio -agc 0}
	{radio -gain 100}
	{radio -pws-mode 0}
	{radio -squelch-value 0}
	{radio -sub-rx-gain 100}
    }
}

proc sdr::check-defaults {parent} {
    # sweep the default parameters to see that the options match
    array set cache {}
    foreach {component option value} $::sdr::defaults::defaults {
	if { ! [info exists cache($component)]} {
	    set cache($component) [$parent.$component info options]
	}
	if {[lsearch $cache($component) $option] < 0} {
	    puts stderr "component $component has no $option option"
	}
    }
}

proc sdr::set-defaults {parent} {
    # sweep the default parameters and reset their values to the default values
    array set cache {}
    foreach {component option value} $::sdr::defaults::defaults {
	if { ! [info exists cache($component)]} {
	    set cache($component) [$parent.$component info options]
	}
	if {[lsearch $cache($component) $option] < 0} {
	    puts stderr "component $component has no $option option"
	} else {
	    $parent.$component configure $option $value
	}
    }
}
