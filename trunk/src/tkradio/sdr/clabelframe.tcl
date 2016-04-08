# -*- mode: Tcl; tab-width: 8; -*-
#
# Copyright (C) 2011, 2012 by Roger E Critchlow Jr, Santa Fe, NM, USA.
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

##
## a collapsible ttk::labelframe, much reduced from original source
## this was taken from http://wiki.tcl.tk/29126 by Julian H J Loaring
##
package provide sdrtk::clabelframe 1.0.0

package require snit
package require Tk
package require Ttk

snit::widget sdrtk::clabelframe {
    hulltype ttk::labelframe

    option -state -default open -type { snit::enum -values {open closed} } -configuremethod Configure -readonly 1
    option -label -default ""  -configuremethod Configure
    
    component label
    
    delegate option -labelfont to label as -font
    delegate option -labelfg to label as -foreground
    
    delegate method * to hull
    delegate option * to hull
    
    #
    # I'm using symbol characters within the font so that I don't
    # have to worry colour. The downside is that the developer
    # can choose a font that doesn't have these symbols...
    #
    # Because the attached label is a ttk::label, one could use a
    # compound label + text and just manage the icon, but then
    # what colour icon to use and what style for the current theme
    #
    variable data -array {
	open_symbol    "\u25bc"
	closed_symbol  "\u25ba"
	text           ""
	slaves	       {}
    }

    constructor {args} {
	#  -font {arial 11 bold}
	install label using ttk::label $win.label -textvariable [myvar data(text)]
	$win configure -labelwidget $label
	bind $label <1> [mymethod toggle]
	$self configurelist $args               
    }
    
    method {Configure -label} {value} {
	set options(-label) $value
	$self UpdateLabel
    }
    
    method {Configure -state} {value} {
	if {$value ne $options(-state)} {
	    $self toggle
	}
    }
    
    method UpdateLabel {} { set data(text) "$data($options(-state)_symbol) $options(-label)" }
    
    
    method toggle {} {
	if {$options(-state) eq "open"} {
	    set slaves [grid slaves $win]
	    if {[llength $slaves]} {
		set data(restore) [list grid {*}$slaves]
		grid remove {*}$slaves
	    } else {
		catch {unset data(restore)}
	    }
	    $hull configure -height [expr {[winfo height $label] +  2}]
	    set options(-state) closed
	} else {
	    if {[info exists data(restore)]} {
		{*}$data(restore)
	    }
	    set options(-state) open
	}
	$self UpdateLabel
    }
}

if {0} {
    set row 0
    foreach cat {cats dogs colors} subcat {{siamese manx coon} {samoyed mutt pekinese} {red green blue yellow orange brown black}} subtext {meow ruff ooh} {
	puts "making $cat with $subcat"
	grid [sdrtk::incrlabelframe .$cat -text $cat] -row $row -column 0 -sticky nsew
	#grid rowconfigure . $row -weight 1
	set subrow 0
	foreach sub $subcat {
	    puts "making $sub of $cat"
	    grid [sdrtk::labelframe .$cat.$sub -text $sub] -row $subrow -column 0 -sticky nsew
	    grid [ttk::label .$cat.$sub.text -text $subtext] -row 0 -column 0
	    #grid rowconfigure .$cat $subrow -weight 1
	    incr subrow
	}
	grid columnconfigure .$cat 0 -weight 1
	incr row
    }
    grid columnconfigure . 0 -weight 1 -minsize 100
}