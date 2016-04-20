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
# a hardware module for the afedri receiver, DDC
# {100kHz .. 36MHz}
#
package provide sdr::radio-afedri 1.0

package require Tcl
package require snit

package require sdr::util

namespace eval ::sdr {}

snit::type sdr::radio-afedri {
    option -parent -readonly true -configuremethod Configure
    
    # the low and high frequencies tuned
    variable caps [dict create {*}{
	range {100kHz .. 36MHz} 
    }]

    component parent

    constructor {args} {
	$self configure {*}$args
    }
    method {Configure -parent} {value} { 
	set options(-parent) $value
	set parent $value
    }
    method get-caps {} {
	return $caps
    }
    ##
    ## monitor our configuration options
    ## 
    method monitor {opts prefix} {
	foreach option $opts {
	    trace variable options($option) w [list {*}[mymethod monitor-fired] $prefix]
	}
    }
    method monitor-fired {prefix name1 name2 op} {
	{*}$prefix $name2 $options($name2)
    }
}
# funcube dongle pro receiver
# {500kHz .. 2000MHz}
# 
