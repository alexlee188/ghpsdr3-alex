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

package provide sdrtk::spectrum 1.0.0

package require Tk
package require snit
package require sdrtype::types

snit::widgetadaptor sdrtk::spectrum {
    option -pal -default 0 -type sdrtype::spec-palette
    option -max -default 0 -type sdrtype::decibel -configuremethod Revertical
    option -min -default -160 -type sdrtype::decibel -configuremethod Revertical

    option -smooth -default true -type sdrtype::smooth -configuremethod Resmooth

    option -multi -default 1 -type sdrtype::multi -configuremethod Remulti

    option -sample-rate -default 48000 -type sdrtype::sample-rate
    option -zoom -default 1 -type sdrtype::zoom
    option -pan -default 0 -type sdrtype::pan

    # from sdrkit::spectrum
    option -center-freq -default 0 -configuremethod Retune
    option -filter-low -default 0 -configuremethod Retune
    option -filter-high -default 0 -configuremethod Retune
    option -band-low -default 0 -configuremethod Retune
    option -band-high -default 0 -configuremethod Retune
    option -tuned-freq -default 0 -configuremethod Retune

    option -command -default {}

    delegate option * to hull
    delegate method * to hull

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
    }
    
    constructor {args} {
	installhull using canvas
	$self configure {*}$args
	$hull configure -bg black
	bind $win <Configure> [mymethod DrawAll]
	bind $win <ButtonPress-1> [mymethod Press %W %x %y]
	bind $win <ButtonRelease-1> [mymethod Release %W %x %y]
	bind $win <B1-Motion> [mymethod Motion %W %x %y]
    }
    
    method Press {w x y} {
	set data(freq) [expr {int(($x-double($data(xoffset)))/$data(xscale))}]
	if {$options(-command) ne {}} { {*}$options(-command) $w Press $x $y $data(freq) 0 }
    }
    method Release {w x y} {
	set df [expr {int(($x-double($data(xoffset)))/$data(xscale) - $data(freq))}]
	if {$options(-command) ne {}} { {*}$options(-command) $w Release $x $y $data(freq) $df }
    }
    method Motion {w x y} {
	set df [expr {int(($x-double($data(xoffset)))/$data(xscale) - $data(freq))}]
	incr data(freq) $df
	if {$options(-command) ne {}} { {*}$options(-command) $w Motion $x $y $data(freq) $df }
    }

    method update {xy} {
	$hull coords spectrum-$data(multi) $xy
	$hull raise spectrum-$data(multi)
	$self Scale spectrum-$data(multi)
	for {set i 0} {$i < $options(-multi)} {incr i} {
	    set j [expr {($data(multi)+$i)%$options(-multi)}]
	    $hull itemconfigure spectrum-$j -fill [lindex $data(multi-hues) $j]
	}
	set data(multi) [expr {($data(multi)-1+$options(-multi))%$options(-multi)}]
    }

    method VerticalScale {tag} {
	set data(yscale) [expr {-[winfo height $win]/double($options(-max)-$options(-min))}]
	set data(yoffset) [expr {-$options(-max)*$data(yscale)}]
	$hull scale $tag 0 0 1 $data(yscale)
	$hull move $tag 0 $data(yoffset)
    }

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

    # band limits
    method DrawBand {} {
	if {$options(-band-low) ne {} && $options(-band-high) ne {}} {
	    set low [expr {$options(-band-low)-$options(-center-freq)}]
	    set high [expr {$options(-band-high)-$options(-center-freq)}]
	    $hull create rectangle [expr {$low-$data(band-width)}] $options(-min) $low $options(-max) -fill $data(band-fill) -outline {} -tags {band band-low}
	    $hull create rectangle $high $options(-min) [expr {$high+$data(band-width)}] $options(-max) -fill $data(band-fill) -outline {} -tags {band band-high}
	    $hull lower band
	    $self Scale band
	}
    }
    method AdjustBand {} {
	if {$options(-band-low) ne {} && $options(-band-high) ne {}} {
	    if {[$hull find withtag band] eq {}} {
		$self DrawBand
	    } else {
		set low [expr {$options(-band-low)-$options(-center-freq)}]
		set high [expr {$options(-band-high)-$options(-center-freq)}]
		$hull coords band-low  [expr {$low-$data(band-width)}] $options(-min) $low $options(-max)
		$hull coords band-high $high $options(-min) [expr {$high+$data(band-width)}] $options(-max)
		$self Scale band
	    }
	}
    }
    method ScrollBand {dx} {
	if {$options(-band-low) ne {} && $options(-band-high) ne {}} {
	    if {[$hull find withtag band] eq {}} {
		$self DrawBand
	    } else {
		$self HorizontalScroll band $dx
	    }
	}
    }
	
    # filter rectangle
    method DrawFilter {} {
	$hull create rectangle $options(-filter-low) $options(-min) $options(-filter-high) $options(-max) -fill $data(darkest) -outline $data(darkest) -tags filter
	$self Scale filter
    }
    method AdjustFilter {} {
	$hull coords filter $options(-filter-low) $options(-min) $options(-filter-high) $options(-max)
	$self Scale filter
    }

    # carrier tuning line
    method DrawCarrier {} {
	$hull create line $options(-tuned-freq) $options(-min) $options(-tuned-freq) $options(-max) -fill $data(carrier) -tags carrier
	$self Scale carrier
    }

    method AdjustCarrier {} {
	$hull coords carrier $options(-tuned-freq) $options(-min) $options(-tuned-freq) $options(-max)
	$self Scale carrier
    }

    # vertical grid, horizontal dBFS markings and labels
    method DrawVgrid {} {
	set lo [expr {-$options(-sample-rate)/2.0}]
	set hi [expr {$options(-sample-rate)/2.0}]
	set xy {}
	for {set l $options(-min)} {$l <= $options(-max)} {incr l 20} {
	    # main db grid
	    lappend xy $lo $l $hi $l $lo $l
	    $hull create text 0 $l -text " $l" -font {Helvetica 8} -anchor nw -fill $data(light) -tags {vlabel label}
	    # sub grid
	    if {0} {
		for {set ll [expr {$l-10}]} {$ll > $l-20} {incr ll -10} {
		    if {$ll >= $options(-min) && $ll <= $options(-max)} {
			$hull create line $lo $ll $hi $ll -fill $data(med) -tags {vgrid grid}
		    }
		}
	    }
	}
	$hull create line $xy -fill $data(darker) -tags {vgrid grid}
	$self Scale vgrid
	$self VerticalScale vlabel
    }
    method AdjustVgrid {} {
	$hull delete vgrid
	$hull delete vlabel
	$self DrawVgrid
	$hull lower vgrid carrier
    }
    
    # horizontal grid, vertical frequency markings and labels
    method DrawHgrid {} {
	# offset of tuning from grid
	set frnd [expr {int($options(-center-freq)/10000)*10000}]
	set foff [expr {$options(-center-freq)-$frnd}]
	set fmax [expr {int(5*$options(-sample-rate)/20000+1)*10000}]
	set fmin [expr {-$fmax}]
	set xy {}
	for {set f $fmin} {$f <= $fmax} {incr f 10000} {
	    set label [format { %.2f} [expr {($f+$frnd)*1e-6}]]
	    set fo [expr {$f-$foff}]
	    lappend xy $fo $options(-min) $fo $options(-max) $fo $options(-min)
	    $hull create text $fo $options(-min) -text $label -font {Helvetica 8} -anchor sw -fill $data(light) -tags {hlabel label}
	}
	$hull create line $xy -fill $data(darker) -tags {hgrid grid}
	$self Scale hgrid
	$self Scale hlabel
	$hull lower hgrid
	set data(hgrid-scroll) 0
    }

    method ScrollHgrid {dx} {
	if {abs($data(hgrid-scroll)+$dx) < 2*$options(-sample-rate)} {
	    $self HorizontalScroll hgrid $dx
	    $self HorizontalScroll hlabel $dx
	    set data(hgrid-scroll) [expr {$data(hgrid-scroll)+$dx}]
	} else {
	    $hull delete hgrid
	    $hull delete hlabel
	    $self DrawHgrid
	}
    }

    method DrawAll {} {
	catch {$hull delete band}
	catch {$hull delete filter}
	catch {$hull delete grid}
	catch {$hull delete label}
	catch {$hull delete carrier}
	$self DrawBand
	$self DrawFilter
	$self DrawHgrid
	$self DrawVgrid
	$self DrawCarrier
    }	
    
    method {Retune -filter-low} {val} {
	set options(-filter-low) $val
	$self AdjustFilter
    }
    
    method {Retune -filter-high} {val} {
	set options(-filter-high) $val
	$self AdjustFilter
    }
    
    method {Retune -band-low} {val} {
	set options(-band-low) $val
	$self AdjustBand
    }
    
    method {Retune -band-high} {val} {
	set options(-band-high) $val
	$self AdjustBand
    }
    
    method {Retune -tuned-freq} {val} {
	set options(-tuned-freq) $val
	$self AdjustCarrier
    }
				 
    method {Retune -center-freq} {val} {
	set scroll [expr {$options(-center-freq)-$val}]
	set options(-center-freq) $val
	$self ScrollHgrid $scroll
	$self ScrollBand $scroll
    }

    method Revertical {opt value} {
	set options($opt) $value
	$self AdjustVgrid
	$self AdjustFilter
	$self AdjustCarrier
    }

    method Resmooth {opt value} {
	set options($opt) $value
	catch {$hull itemconfigure spectrum -smooth $value}
    }

    proc gray-scale {n} {
	set scale {}
	set intensity 0xFF
	for {set i 0} {$i <= $n} {incr i} {
	    lappend scale [string range [format {\#%02x%02x%02x} $intensity $intensity $intensity] 1 end]
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
