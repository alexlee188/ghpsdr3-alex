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

package provide sdrui::repl-radio

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
    
    variable input
    
    proc repl {} {
	gets stdin line
	append input $line
	if {[info complete $input]} {
	    if {[catch {eval $input} error]} {
		puts stdout $error\n$::errorInfo
	    } else {
		puts stdout $error
	    }
	    set input {}
	    set prompt {$ }
	} else {
	    set prompt {> }
	}
	flush stdout
	if {[eof stdin]} {
	    fileevent stdin readable {}
	} else {
	    puts -nonewline stdout $prompt
	    flush stdout
	}
    }
}

sdrui::repl %AUTO%
