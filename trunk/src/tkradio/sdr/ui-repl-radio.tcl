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

package provide sdrui::repl-radio 1.0

package require Tcl
package require snit

snit::type sdrui::repl-radio {
    ##
    ## implement a repl -- a read, eval, print loop -- as the default
    ## user interface to tkradio, without any tk if you like, though
    ## this could run alongside a window interface, too.
    ##
    constructor {args} {
	fileevent stdin readable [myproc repl]
    }
    
    typevariable input {}

    proc repl {} {
	if {[gets stdin line] < 0} {
	    # eof on stdin
	    fileevent stdin readable {}
	    set ::forever 0
	    return
	}
	append input $line\n
	if { ! [info complete $input]} {
	    set prompt {> }
	} else {
	    if {$input ne "\n"} {
		catch {eval uplevel #0 [list $input]} result
		if {[string length $result]} {
		    puts stdout $result
		}
	    }
	    set input {}
	    set prompt {$ }
	}
	puts -nonewline stdout $prompt
	flush stdout
    }
}
