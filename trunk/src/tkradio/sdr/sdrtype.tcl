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
## sdrtypes - snit types for option validation 
## and to supply values for selection menues
## via [sdrtype::<enum-type> cget -values]
## or [sdrtype::<range-type> cget -min/-max]
##
## some of the limits depend on the sample rate.
##
package provide sdrtype::types 1.0.0

package require snit

snit::enum	sdrtype::type		-values {none ctl dsp jack ui hw physical}

snit::enum	sdrtype::mode		-values {USB LSB DSB CWU CWL AM SAM FM DIGU DIGL}

snit::enum	sdrtype::agc-mode	-values {OFF LONG SLOW MEDIUM FAST}
snit::double	sdrtype::agc-target	-min 0.1 -max 10;	# target level, linear gain
snit::double	sdrtype::agc-attack	-min 1 -max 4;		# attack time in ms
snit::double	sdrtype::agc-decay	-min 100 -max 2000;	# decay time in ms
snit::double	sdrtype::agc-slope	-min 0.5 -max 2;	# slope, as in y = slope * x + intercept
snit::double	sdrtype::agc-hang	-min 50 -max 1000;	# hang time in ms
snit::double	sdrtype::agc-fasthang	-min 50 -max 200;	# fast hang time in ms
snit::double	sdrtype::agc-max	-min 1 -max 1e5;	# max level
snit::double	sdrtype::agc-min	-min 1e-5 -max 1;	# min level
snit::double	sdrtype::agc-threshold	-min 0.25 -max 4;	# threshold level

snit::double	sdrtype::constant-real
snit::double	sdrtype::constant-imag

# filter-overlap-save - 

snit::enum	sdrtype::leveler-mode	-values {off leveler}

snit::enum	sdrtype::iambic		-values {none ad5dz dttsp nd7pa}
snit::boolean	sdrtype::debounce
snit::double	sdrtype::debounce-period -min 0.1 -max 50;	# period for switch state sampling in ms
snit::integer	sdrtype::debounce-steps	-min 1 -max 32;		# number of consistent periods for debounced state change

snit::boolean	sdrtype::iq-swap;				# true or false, swapped or not swapped

snit::enum	sdrtype::iq-delay	-values {-1 0 1};	# delay I by one sample, no delay, or delay Q by one sample
snit::double	sdrtype::iq-correct	-min -1e6 -max 1e6;	# the learning rate for the adaptive filter
snit::double	sdrtype::sine-phase	-min -1.0 -max 1.0;	# 
snit::double	sdrtype::linear-gain	-min 0.125 -max 8.0;	# 

snit::double	sdrtype::gain		-min -200.0 -max 200.0; # gain in decibels
snit::integer	sdrtype::hertz

snit::boolean	sdrtype::mute
snit::boolean	sdrtype::spot

snit::integer	sdrtype::instance	-min 1 -max 10
snit::double	sdrtype::decibel	-min -200.0 -max 200.0
snit::enum	sdrtype::spec-size	-values {width/8 width/4 width/2 width width*2 width*4 width*8}
snit::integer	sdrtype::fftw-planbits	-min 0 -max 127
snit::enum	sdrtype::fftw-direction	-values {-1 1}
snit::integer	sdrtype::spec-polyphase	-min 1 -max 32
snit::enum	sdrtype::spec-result	-values {coeff mag mag2 dB short char}
snit::enum	sdrtype::spec-palette	-values {0 1 2 3 4 5}
snit::double	sdrtype::zoom		-min 0.5 -max 64
snit::double	sdrtype::pan		-min -200000 -max 200000
snit::enum	sdrtype::smooth		-values {true false raw bezier}
snit::integer	sdrtype::splinesteps
snit::integer	sdrtype::multi		-min 1 -max 64
snit::integer	sdrtype::sample-rate	-min 3000 -max 6144000;	# SDRplay goes higher!!!
snit::integer	sdrtype::milliseconds	-min 0 -max 30000
snit::integer	sdrtype::samples	-min 1 -max 32000
snit::double	sdrtype::decay		-min 0.0001 -max 0.9999
snit::enum	sdrtype::meter-reduce	-values {abs_real abs_imag max_abs mag2}
snit::enum	sdrtype::meter-style	-values {s-meter}
