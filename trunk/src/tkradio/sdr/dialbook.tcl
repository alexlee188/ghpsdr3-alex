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

#
# a rotary encoder menu multiple display widget
#
# this manages a group of tabs associated with a rotary encoder
# controller.
#
package provide sdrtk::dialbook 1.0

package require Tk
package require snit
package require sdrtk::dial

snit::widgetadaptor sdrtk::dialbook-tab {
    # Either normal, disabled or hidden.
    # If disabled, then the tab is not selectable.
    # If hidden, then the tab is not shown.
    option {-state state State} -default normal -type { snit::enum -values {normal disabled hidden} }

    # Specifies how the slave window is positioned within the pane area.
    # Value is a string containing zero or more of the characters n, s, e, or w.
    # Each letter refers to a side (north, south, east, or west) that the slave window
    # will “stick” to, as per the grid geometry manager.
    option {-sticky sticky Sticky} -default {} -type { snit::stringtype -regexp {[nsew]*} }

    # Specifies the amount of extra space to add between the notebook and this pane.
    # Syntax is the same as for the widget -padding option.
    option {-padding padding Padding} -default {} -type { snit::listtype -maxlen 4 -type snit::integer }

    delegate option -text to hull
    delegate option -image to hull
    delegate option -compound to hull
    delegate option -underline to hull

    variable data -array {}

    constructor {window args} {
	# the hull is the label displayed for tab selection
	installhull using ttk::label
	set data(window) $window
	$self configure {*}$args
    }

    destructor {
    }

    method get-text {} { return [$hull cget -text] }
    method get-window {} { return $data(window) }
    method get-label {} { return $win }
}

snit::widget sdrtk::dialbook {
    component tab
    component dial
    option -class undefined
    option {-cursor cursor Cursor} {}
    option {-style style Style} {}
    option {-takefocus takeFocus TakeFocus} 1
    # If present and greater than zero, specifies the desired height of the pane area
    # (not including internal padding or the dial).
    # Otherwise, the maximum height of all panes is used.
    option {-height height Height} {}
    # Specifies the amount of extra space to add around the outside of the notebook.
    # The padding is a list of up to four length specifications left top right bottom.
    # If fewer than four elements are specified, bottom defaults to top,
    # right defaults to left, and top defaults to left.
    option {-padding padding Padding} {}
    # If present and greater than zero, specifies the desired width of the pane area
    # (not including internal padding). Otherwise, the maximum width of all panes is used.
    option {-width width Width} {}
    
    variable data -array {
	tabs {}
	current {}
	displayed {}
	menu false
	counter 0
    }

    constructor {args} {
	install tab using ttk::frame $win.tab
	install dial using sdrtk::dial $win.dial
	$self configure {*}$args
	grid $win.tab -row 0
	grid $win.dial -row 1 -sticky nsew
	bind $win.dial <<DialCW>> [mymethod Adjust 1]
	bind $win.dial <<DialCCW>> [mymethod Adjust -1]
	bind $win.dial <<DialPress>> [mymethod Press]
    }

    #
    # The tabid argument to the following commands may take any of the following forms:
    # •  An integer between zero and the number of tabs;
    # •  The name of a slave window;
    # •  A positional specification of the form “@x,y”, which identifies the tab
    # •  The literal string “current”, which identifies the currently-selected tab; or:
    # •  The literal string “end”, which returns the number of tabs (only valid for “pathname index”).
    #

    # Adds a new tab to the dialbook.
    # See TAB OPTIONS for the list of available options.
    # If window is currently managed by the notebook but hidden,
    # it is restored to its previous position.
    method add {window args} {
	set tab [$self FindWindow $window]
	if {$tab ne {}} {
	    # remember hidden window
	    $tab configure -state normal {*}$args
	    return
	}
	lappend data(tabs) [$self NewTab $window {*}$args]
	$self UpdateLists
    }

    # Removes the tab specified by tabid, unmaps and unmanages the associated window.
    method forget {tabid} {
	puts "$self forget $tabid"
    }

    # Hides the tab specified by tabid.
    # The tab will not be displayed, but the associated window remains managed by the notebook
    # and its configuration remembered.
    # Hidden tabs may be restored with the add command.
    method hide {tabid} {
	puts "$self hide $tabid"
    }

    # Returns the name of the element under the point given by x and y,
    # or the empty string if no component is present at that location.
    # Returns the name of the element at the specified location.
    method {identify element} {x y} {
	puts "$self identify element $x $y"
    }

    # Returns the index of the tab at the specified location.
    method {identify tab} {x y} {
	puts "$self identify tab $x $y"
    }

    # Returns the numeric index of the tab specified by tabid, or the total number of tabs if tabid is the string “end”.
    method index {tabid} { return [$self FindIndex $tabid] }

    # Inserts a pane at the specified position.
    # pos is either the string end, an integer index, or the name of a managed subwindow.
    # If subwindow is already managed by the notebook, moves it to the specified position.
    # See TAB OPTIONS for the list of available options.
    method insert {pos subwindow args} {
	set tab [$self FindWindow $subwindow]
	if {$tab ne {}} {
	    set i [lsearch $data(tabs) $tab]
	    set data(tabs) [lreplace $data(tabs) $i $i]
	} else {
	    set tab [$self NewTab $window {*}$args]
	}
	set i [$self FindIndex $pos]
	if {$i eq {}} {
	    lappend data(tabs) $tab
	} else {
	    set data(tabs) [linsert $data(tabs) $i $tab]
	}
    }

    # See ttk::widget(n).
    method instate {statespec args} {
    }

    # Selects the specified tab.
    # The associated slave window will be displayed,
    # and the previously-selected window (if different) is unmapped.
    # If tabid is omitted, returns the widget name of the currently selected pane.
    method select {{tabid {}}} {
	if {$tabid eq {}} {
	    if {$data(current) ne {}} {
		return [$data(current) get-window]
	    }
	    return {}
	}
	set current [$self Tab $tabid]
	if {$current eq {}} {
	    return
	}
	set data(current) $current
	$self UpdateCurrent
    }

    # See ttk::widget(n).
    method state {args} {
    }

    # Query or modify the options of the specific tab.
    # If no -option is specified, returns a dictionary of the tab option values.
    # If one -option is specified, returns the value of that option.
    # Otherwise, sets the -options to the corresponding values.
    # See TAB OPTIONS for the available options.
    method tab {tabid args} {
    }

    # Returns the list of windows managed by the notebook.
    method tabs {} {
	set tabs {}
	foreach tab $data(tabs) { lappend tabs [$tab get-window] }
	return $tabs
    }
    
    ##
    ##
    ##
    method Adjust {step} {
	#puts "$self Adjust $step"
	$dial Rotate $step
	if {$data(menu)} {
	    
	} else {
	    if {$data(current) ne {}} { [$data(current) get-window] adjust $step }
	}
    }
    method Press {} {
	# puts "$self Press"
	if {$data(menu)} {
	    # select currently addressed tab
	    # end menu
	} else {
	    # start menu - done as a plain popup, but needs to be
	    # redone in a way that uses the rotational input
	    set data(menu) true
	    if { ! [winfo exists $win.menu]} {
		menu $win.menu -tearoff no
	    } else {
		$win.menu delete 0 end
	    }
	    set data(menu-select) $data(current)
	    foreach atab $data(tabs) {
		set text [$atab get-text]
		$win.menu add radiobutton -label $text -value $atab -variable [myvar data(menu-select)] -command [mymethod MenuInvoke $atab]
	    }
	    bind $win.menu <<MenuSelect>> [mymethod MenuSelect]
	    bind $win.menu <Unmap> [mymethod MenuUnmap]
	    tk_popup $win.menu [winfo pointerx $win] [winfo pointery $win] [$win.menu index [$data(current) cget -text]]
	}
    }

    method MenuSelect {} {
	set atab [$win.menu entrycget active -value]
	if {$atab ne {}} { $self DisplayTab $atab }
    }

    method MenuInvoke {atab} {
	set data(menu) false
	set data(current) $atab
	$self UpdateCurrent
    }

    method MenuUnmap {} {
	set data(menu) false
	$self UpdateCurrent
    }

    method NewTab {window args} {
	return [sdrtk::dialbook-tab $win.tab[incr data(counter)] $window {*}$args]
    }

    method FindWindow {window} {
	foreach tab $data(tabs) { if {[$tab get-window] eq $window} { return $tab } }
    }
	
    method FindIndex {tabid} {
	switch -regexp $tabid {
	    {^end$} { return [llength $data(tabs)] }
	    {^current$} {
		if {$data(current) eq {}} { return {} }
		return [lsearch $data(tabs) $data(current)]
	    }
	    {^@\d+} {
		# angular? o'clock?
	    }
	    {^\d+$} {
		if {$tabid >= 0 && $tabid < [llength $data(tabs)]} {
		    return $tabid
		}
	    }
	    {^\..*$} {
		set tab [$self FindWindow $tabid]
		if {$tab ne {}} {
		    return [lsearch $data(tabs) $tab]
		}
	    }
	}
	return {}
    }
	
    method FindTab {tabid} {
	set i [$self FindIndex $tabid]
	if {$i ne {} && $i >= 0 && $i < [llength $data(tabs)]} { return [lindex $data(tabs) $i] }
	return {}
    }

    method IsDisplayedTab {atab} { return $tab eq $data(displayed) }
    method DisplayTab {atab} {
	if {$data(displayed) ne {} && [info commands $data(displayed)] ne {}} {
	    grid forget [$data(displayed) get-window]
	}
	set data(displayed) $atab
	grid [$atab get-window] -in $win.tab -sticky [$atab cget -sticky] -row 0 -column 0
	grid columnconfigure $win.tab 0 -minsize $data(wd)
	grid rowconfigure $win.tab 0 -minsize $data(ht)
    }

    method SubwindowMapped {w} {
	bind $w <Map> {}
	$self UpdateLists
    }

    method UpdateLists {} {
	set data(wd) 0
	set data(ht) 0
	foreach atab $data(tabs) {
	    set w [$atab get-window]
	    if { ! [winfo ismapped $w]} {
		#update idletasks
		bind $w <Map> [mymethod SubwindowMapped %W]
	    }
	    lappend data(wd) [winfo width $w]
	    lappend data(ht) [winfo height $w]
	}
	set data(wd) [tcl::mathfunc::max {*}$data(wd)]
	set data(ht) [tcl::mathfunc::max {*}$data(ht)]
	# puts "$self UpdateLists tabs $data(tabs) wd $data(wd) ht $data(ht)"
	if { ! $data(menu)} {
	    $self UpdateCurrent
	}
    }

    method UpdateCurrent {} {
	# puts "$self UpdateCurrent: $data(current) $data(tabs)"
	if {$data(current) eq {} || [lsearch $data(tabs) $data(current)] < 0} {
	    if {[llength $data(tabs)] > 0} {
		set data(current) [lindex $data(tabs) 0]
		$self DisplayTab $data(current)
	    }
	} else {
	    $self DisplayTab $data(current)
	}
    }
}    
