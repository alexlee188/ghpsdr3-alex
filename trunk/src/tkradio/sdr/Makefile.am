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
CFLAGS=-std=c99 -g -O3 -DUSE_TCL_STUBS 
CPPFLAGS= -g -O3 -DUSE_TCL_STUBS 
LIBS=-ltclstub8.6 -lpulse-simple

tcltkdir = $(libdir)/tcltk/sdr
tcltk_DATA = audio.tcl
tcltk_DATA += band-data.tcl
tcltk_DATA += clabelframe.tcl
tcltk_DATA += command.tcl
tcltk_DATA += connect.tcl
tcltk_DATA += defaults.tcl
tcltk_DATA += dialbook.tcl
tcltk_DATA += dial.tcl
tcltk_DATA += filter.tcl
tcltk_DATA += frequency-display.tcl
tcltk_DATA += hotiron.tcl
tcltk_DATA += memory.tcl
tcltk_DATA += meter.tcl
tcltk_DATA += optionmenu.tcl
tcltk_DATA += radio.tcl
tcltk_DATA += radio-sdrplay.tcl
tcltk_DATA += ui-pw-radio.tcl
tcltk_DATA += ui-pw-radio-sdrplay.tcl
tcltk_DATA += ui-repl-radio.tcl
tcltk_DATA += sdrtype.tcl
tcltk_DATA += server.tcl
tcltk_DATA += spectrum.tcl
tcltk_DATA += spectrum-waterfall.tcl
tcltk_DATA += theme-alt.tcl
tcltk_DATA += theme-night.tcl
tcltk_DATA += util.tcl
tcltk_DATA += waterfall.tcl

tcltk_DATA += pa_simple.so
tcltk_DATA += pkgIndex.tcl

CLEANFILES=pa_simple.so pkgIndex.tcl

pkgIndex.tcl: $(wildcard *.so) $(filter-out pkgIndex.tcl, $(wildcard *.tcl))
	echo 'pkg_mkIndex .' | tclsh8.6

pa_simple.so: pa_simple.c
	$(CC) -fPIC -shared -o $@ $(CFLAGS) $< $(LIBS)
