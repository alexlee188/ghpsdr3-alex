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
package provide hotiron 1.0.0

##
## hotiron - compute a pixel for a hue in 0..1 from palette in 0 .. 5
##
## palette 0 simulates the spectrum of blackbody radiation with temperature
## starting from black=cold through red, orange, and yellow to white=hot,
## which is what happens to iron when you heat it, hence the name.
##
## palettes 1 through five simply permute the red, green and blue values to
## give some other cheap pixel level map computations.
##
## originally from Eric Grosse's rainbow.c and hotiron.c available from
## http://www.netlib.org/graphics/, though he gives Cleve Moler credit for
## the hotiron palette.  Thanks, Cleve.
##
proc ::hotiron {hue pal} {
    switch $pal {
	0 { lassign [list [expr {3*($hue+0.03)}] [expr {3*($hue-.333333)}] [expr {3*($hue-.666667)}]] r g b }
	1 { lassign [list [expr {3*($hue+0.03)}] [expr {3*($hue-.666667)}] [expr {3*($hue-.333333)}]] r g b }
	2 { lassign [list [expr {3*($hue-.666667)}] [expr {3*($hue+0.03)}] [expr {3*($hue-.333333)}]] r g b }
	3 { lassign [list [expr {3*($hue-.333333)}] [expr {3*($hue+0.03)}] [expr {3*($hue-.666667)}]] r g b }
	4 { lassign [list [expr {3*($hue-.333333)}] [expr {3*($hue-.666667)}] [expr {3*($hue+0.03)}]] r g b }
	5 { lassign [list [expr {3*($hue-.666667)}] [expr {3*($hue-.333333)}] [expr {3*($hue+0.03)}]] r g b }
	6 { # waterfallPalette from quisk-3.6.1
	    # 8 bit color table lut values and r, g, b lut values
	    lassign [::hotiron-interpolate $hue {
		{  0   0   0   0}
		{ 36  85   0 255}
		{ 73 153   0 255}
		{109 255   0 128}
		{146 255 119   0}
		{182  85 255 100}
		{219 255 255   0}
		{255 255 255 255}
	    }] r g b
	}
	7 { # digipanWaterfallPalette from quisk-3.6.1
	    lassign [::hotiron-interpolate $hue {
		{  0   0   0   0}
		{ 32   0   0  62}
		{ 64   0   0 126}
		{ 96 145 142  96}
		{128 181 184  48}
		{160 223 226 105}
		{192 254 254   4}
		{255 255  58   0}
	    }] r g b
	}
	8 { # greyscale
	    lassign [list $hue $hue $hue] r g b
	}
	9 { # QtRadio palette
	    lassign [::hotiron-interpolate $hue {
		{  0   0   0   0}
		{ 57   0   0 255}
		{ 85   0 255 255}
		{113   0 255   0}
		{142 255 255   0}
		{198 255   0   0}
		{227 255   0 255}
		{255 191 127 255}
	    }] r g b
	}
    }
    return \#[format {%04x%04x%04x} [expr {int(65535*min(1,max($r,0)))}] [expr {int(65535*min(1,max($g,0)))}] [expr {int(65535*min(1,max($b,0)))}]]
}

proc ::hotiron-interpolate {hue lut} {
    foreach lo [lrange $lut 0 end-1] hi [lrange $lut 1 end] {
	lassign $lo ilo rlo glo blo
	lassign $hi ihi rhi ghi bhi
	set llo [expr {$ilo/255.0}]
	set lhi [expr {$ihi/255.0}]
	if {$hue >= $llo && $hue <= $lhi} {
	    set p1 [expr {$hue-$llo}]
	    set p2 [expr {$lhi-$hue}]
	    set phi [expr {$p1/($p1+$p2)}]
	    set plo [expr {$p2/($p1+$p2)}]
	    return [list [expr {($plo*$rlo+$phi*$rhi)/255.0}] [expr {($plo*$glo+$phi*$ghi)/255.0}] [expr {($plo*$blo+$phi*$bhi)/255.0}]]
	}
    }
    return {0 0 0}
}