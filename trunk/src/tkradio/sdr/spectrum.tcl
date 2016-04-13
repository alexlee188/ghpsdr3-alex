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
## spectrum
##
## thoughts since making this 
##  the spectrum display should just center itself in the available window.
##  scrolling off the top or bottom because the user specified unrealistic
##  max and min is silly. showing decibel calibrations is silly because they
##  are almost never calibrated.  and they don't respond to arbitrary 
##  amplification and attenuation.
##  on the other hand, showing the exact frequency and level under the cursor
##  can be helpful, as would making a temporary magnification when the user
##  is trying to pick a frequency.
##  most of the decorations, band limits, filters, carriers, etc should move
##  somewhere where there isn't so much data being displayed, a separate info
##  window on the same x scale.
##
## implementation details from 5 years into the future
##  the spectrum widget is built on a tk canvas
##  the bandwidth of spectrum is configured with the -sample-rate option
##  the width of the window as given by window management
##  spectra are drawn as poly line objects in the tk canvas
##  spectra lines are smoothed using the -smooth and -splinesteps options
##  multiple spectra can be drawn with the -multi option
##  

package provide sdrtk::spectrum 1.0.0

package require Tk
package require snit
package require sdrtype::types

snit::widgetadaptor sdrtk::spectrum {
    # -pal is not used
    option -pal -default 0 -type sdrtype::spec-palette

    # -max and -min specify the upper and lower limits in dB
    option -max -default 0 -type sdrtype::decibel -configuremethod Revertical
    option -min -default -160 -type sdrtype::decibel -configuremethod Revertical

    # smooth the spectrum display using the canvas smoothed line drawing
    option -smooth -default false -type sdrtype::smooth -configuremethod Resmooth
    option -splinesteps -default 12 -type sdrtype::splinesteps -configuremethod Resmooth

    # show multiple spectra with decaying intensity
    option -multi -default 1 -type sdrtype::multi -configuremethod Remulti

    # the -sample-rate is used to determine the spectrum width
    # and the frequency units of the xscale of the spectrum window
    option -sample-rate -default 48000 -type sdrtype::sample-rate
    option -zoom -default 1 -type sdrtype::zoom
    option -pan -default 0 -type sdrtype::pan

    # from sdrkit::spectrum
    option -frequency -default 0 -configuremethod Retune
    option -local-oscillator -default 0 -configuremethod Retune

    option -command -default {}

    delegate option * to hull
    delegate method * to hull

    # multi - the number of consecutive spectra to display
    variable data -array {
	multi 0
	darkest \#333
	darker \#666
	dark \#888
	med \#AAA
	light \#CCC
	lighter \#EEE
	lightest \#FFF
	carrier red
	hgrid-scroll 0
	freq 0
	band-fill darkred
	band-width 1000
	xoffset 0
	xscale 1
    }
    
    constructor {args} {
	installhull using canvas
	$self configure -multi 1 {*}$args
	$hull configure -bg black -height 100
	bind $win <Configure> [mymethod DrawAll]
	bind $win <ButtonPress-1> [mymethod Press %W %x %y]
	bind $win <ButtonRelease-1> [mymethod Release %W %x %y]
	bind $win <B1-Motion> [mymethod Motion %W %x %y]
    }
    
    method DrawAll {} {
    }
    method Press {w x y} {
	set f [expr {int(($x-double($data(xoffset)))/$data(xscale))}]
	set data(freq) $f
	$self Command $w Press $x $y $data(freq) 0
    }
    method Release {w x y} {
	set f [expr {int(($x-double($data(xoffset)))/$data(xscale))}]
	set df [expr {$f - $data(freq)}]
	$self Command $w Release $x $y $data(freq) $df
    }
    method Motion {w x y} {
	set f [expr {int(($x-double($data(xoffset)))/$data(xscale))}]
	set df [expr {$f - $data(freq)}]
	incr data(freq) $df
	$self Command $w Motion $x $y $data(freq) $df
    }
    method Command args {
	if {$options(-command) ne {}} { {*}$options(-command) {*}$args }
    }
    method update {xy minx miny avgy} {
	$hull coords spectrum-$data(multi) $xy
	$hull raise spectrum-$data(multi)
	$self Scale spectrum-$data(multi)
	for {set i 0} {$i < $options(-multi)} {incr i} {
	    set j [expr {($data(multi)+$i)%$options(-multi)}]
	    $hull itemconfigure spectrum-$j -fill [lindex $data(multi-hues) $j]
	}
	set data(multi) [expr {($data(multi)-1+$options(-multi))%$options(-multi)}]
    }

    # vertical scale the spectrum traces
    method VerticalScale {tag} {
	set data(yscale) [expr {-[winfo height $win]/double($options(-max)-$options(-min))}]
	set data(yoffset) [expr {-$options(-max)*$data(yscale)}]
	$hull scale $tag 0 0 1 $data(yscale)
	$hull move $tag 0 $data(yoffset)
    }

    # horizontal scale the spectrum traces
    method HorizontalScale {tag} {
	set data(xscale) [expr {[winfo width $win]/double($options(-sample-rate))}]
	set data(xoffset) [expr {[winfo width $win]/2.0}]
	$hull scale $tag 0 0 $data(xscale) 1
	$hull move $tag $data(xoffset) 0
    }
    
    method HorizontalScroll {tag dx} {
	$hull move $tag [expr {$dx*$data(xscale)}] 0
    }

    method Scale {tag} {
	$self VerticalScale $tag
	$self HorizontalScale $tag
    }

    method {Retune -frequency} {val} {
	set options(-frequency) $val
    }
				 
    method {Retune -local-oscillator} {val} {
	set options(-local-oscillator) $val
    }

    method Revertical {opt value} {
	set options($opt) $value
    }

    # change the spectrum smoothing, 
    # cf. the tk canvas line item -smooth and -splinesteps options
    # 
    method Resmooth {opt value} {
	set options($opt) $value
	catch {$hull itemconfigure spectrum -smooth $options(-smooth) -splinesteps $options(-splinesteps)}
    }

    proc gray-scale {n} {
	set scale {}
	set intensity 0xFF
	for {set i 0} {$i <= $n} {incr i} {
	    lappend scale [string range [format {\#%02x%02x%02x} $intensity 0 0] 1 end]
	    incr intensity [expr {-(0xFF/($n+1))}]
	}
	return $scale
    }

    method Remulti {opt value} {
	set options($opt) $value
	set data(multi) 0
	set data(multi-hues) [gray-scale $options(-multi)]
	catch {$hull delete spectrum}
	for {set i 0} {$i < $options(-multi)} {incr i} {
	    $hull create line 0 0 0 0 -width 0 -tags [list spectrum spectrum-$i]
	}
    }

}
