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
# the commands used to control dspserver in ghpsdr3-alex
#
# commands are all converted to lower case in dspserver/client.c
# commands that begin with * are directed to the hardware module
# only a subset of commands are available to the slave clients
# if command starts with q it's a question
# if a hardware command ends with a ? it's a question
# 
package provide sdr::command 1.0

namespace eval ::sdr {}

namespace eval ::sdr::command {

    # modes of demodulation available
    variable modes-dict [dict create LSB 0 USB 1 DSB 2 CWL 3 CWU 4 FM 5 AM 6 DIGU 7 SPEC 8 DIGL 9 SAM 10 DRM 11 ]
    dict unset modes-dict SPEC;	# not implemented
    dict unset modes-dict DRM;	# not implemented
    proc get-modes {} { return [dict keys ${::sdr::command::modes-dict}] }
    proc fix-mode {m} {
	if {$m in [get-modes]} { 
	    return [dict get ${::sdr::command::modes-dict} $m]
	} else {
	    error "unrecognized mode token $m"
	}
    }

    # pwsmodes, spectrum types 
    variable pwsmodes-dict [dict create PostFilter 0 PreFilter 1 SemiRaw 2 PostDet 4 ]
    proc get-pwsmodes {} { return [dict keys ${::sdr::command::pwsmodes-dict}] }

    # the encodings of audio - numeric enumeration by index in list
    variable audio-encodings [dict create ALAW 0 PCM 1]

    # the encodings of mic audio - numeric enumeration by index in list
    variable mic-audio-encodings [dict create ALAW 0]

    # audiostream defaults
    variable audiostream-defaults [dict create buffer_size 2000 samplerate 8000 channels 1 encoding 0 mic-encoding 0]

    # commands which slaves are permitted to run
    proc slave-permitted {cmd} {
	switch $cmd {
	    getspectrum -
	    setclient -
	    startaudiostream -
	    startrtpstream -
	    setfps -
	    setmaster - { return 1 }
	    default { return 0 }
	}
    }

    # not commands, helpers for formatting data
    # enforce an on|off representation of a boolean
    proc on-or-off {v} {
	switch $v {
	    off - 0 - false { return off }
	    on - 1 - true { return on }
	    default { error "on-or-off invalid value {$v}" }
	}
    }
    # enforce a true|false representation of a boolean
    proc true-or-false {v} {
	switch $v {
	    false - 0 - off { return false }
	    true - 1 - on { return true }
	    default { error "true-or-false invalid value {$v}" }
	}
    }

    # not commands, helpers for formatting and sending packets
    # fill a command string with zeroes to an $n byte block
    proc fill-command {n command} {
	binary scan $command c* b
	while {[llength $b] < $n} { lappend b 0 }
	return [binary format c* $b]
    }
    # send a command
    proc send-command {command} {
	if {$::channel != -1} {
	    puts stderr "send>> $command"
	    puts -nonewline $::channel [fill-command 64 $command]
	    flush $::channel
	}
    }

    # send mic audio data
    proc mic {audio} { 
	if {$::verbose > 1} { puts stderr "send>> mic ..." }
	puts -nonewline $::channel {mic }
	puts -nonewline $::channel $audio
	flush $::channel
    }	      

    # queries - these are the only real dspserver commands with hyphens
    # query if we are master of the dspserver
    proc q-master {} { send-command q-master }
    # query the version of something
    proc q-version {} { send-command q-version }
    # query the local oscillator offset
    proc q-loffset {} { send-command q-loffset }
    # query about protocol 3
    proc q-protocol3 {} { send-command q-protocol3 }
    proc q-server {} { send-command q-server }
    proc q-info {} { send-command q-info }
    proc q-cantx {user password} { send-command "q-cantx#$user#$password" }
    proc q-rtpport {} { send-command q-rtpport }
    
    # commands in order of appearance in dspserver/client.c/readcb()
    proc getspectrum {iwidth} { send-command "getspectrum $iwidth" }
    proc setfrequency {ihz} { send-command "setfrequency $ihz" }
    proc setpreamp {s} { send-command "setpreamp $s" }
    proc setmode {imode} { send-command "setmode [fix-mode $imode]" }
    proc setfilter {ilo ihi} { send-command "setfilter $ilo $ihi" }
    proc setagc {iagc} { send-command "setagc $iagc" }
    proc setfixedagc {iagc} { send-command "setfixedagc $iagc" }
    proc enablenotchfilter {ivfo iindex ienabled} { send-command "enablenotchfilter $ivfo $iindex $ienabled" }
    proc setnotchfilter {ivfo iindex fbandwidth ffrequency} { send-command "setnotchfilter $ivfo $iindex $fbandwidth $ffrequency" }
    proc setagctlevel {i} { send-command "setagctlevel $i" }
    proc setrxgreqcmd {i} { send-command "setrxgreqcmd $i" }
    proc setrx3bdgreq {pre b1 b2 b3} { send-command "setrx3bdgreq $pre $b1 $b2 $b3" }
    proc setrx10bdgreq {pre b1 b2 b3 b4 b5 b6 b7 b8 b9 b10} { send-command "setrx10bdgreq $pre $b1 $b2 $b3 $b4 $b5 $b6 $b7 $b8 $b9 $b10" }
    proc settxgreqcmd {i} { send-command "settxgreqcmd $i" }
    proc settx3bdgreq {pre b1 b2 b3} { send-command "settx3bdgreq $pre $b1 $b2 $b3" }
    proc settx10bdgreq {pre b1 b2 b3 b4 b5 b6 b7 b8 b9 b10} { send-command "settx10bdgreq $pre $b1 $b2 $b3 $b4 $b5 $b6 $b7 $b8 $b9 $b10" }
    proc setnr {val} { send-command "setnr [true-or-false $val]" }
    proc setnb {val} { send-command "setnb [true-or-false $val]" }
    proc setsdrom {v} { send-command "setsdrom [true-or-false $v]" }
    proc setanf {tf} { send-command "setanf [true-or-false $tf]" }
    proc setrxoutputgain {i} { send-command "setrxoutputgain $i" }
    proc setsubrxoutputgain {i} { send-command "setsubrxoutputgain $i" }
    proc startaudiostream {ibufsize irate ichannels imicencoding} { send-command "startaudiostream $ibufsize $irate $ichannels $imicencoding" }
    proc startrtpstream {iport iencoding irate ichannels} { send-command "startrtpstream $iport $iencoding $irate $ichannels" }
    proc stopaudiostream {} { send-command "stopaudiostream" };  #  does nothing
    proc setencoding {iencoding} { send-command "setencoding $iencoding" }
    proc setsubrx {int} { send-command "setsubrx $int" }
    proc setsubrxfrequency {int} { send-command "setsubrxfrequency $int" }; #  offset from setfrequency
    proc setpan {f} { send-command "setpan $f" }
    proc setsubrxpan {f} { send-command "setsubrxpan $f" }
    proc record {s} { send-command "record $s" }; #  send to ozy $s
    proc setanfvals {itaps idelay fgain fleakage} { send-command "setanfvals $itaps $idelay $fgain $fleakage" }
    proc setnrvals {itaps idelay fgain fleakage} { send-command "setnrvals $itaps $idelay $fgain $fleakage" }
    proc setrxagcattack {i} { send-command "setrxagcattack $i" }
    proc setrxagcdecay {i} { send-command "setrxagcdecay $i" }
    proc setrxagchang {i} { send-command "setrxagchang $i" }
    proc setrxagchanglevel {f} { send-command "setrxagchanglevel $f" }
    proc setrxagchangthreshold {i} { send-command "setrxagchangthreshold $i" }
    proc setrxagcthresh {f} { send-command "setrxagcthresh $f" }
    proc setrxagcmaxgain {f} { send-command "setrxagcmaxgain $f" }
    proc setrxagcslope {i} { send-command "setrxagcslope $i" };    # int? is that right?
    proc setnbvals {fthreshold} { send-command "setnbvals $fthreshold" }
    proc setsdromvals {fthreshold} { send-command "setsdromvals $fthreshold" }
    proc setpwsmode {istate} { send-command "setpwsmode $istate" }
    proc setbin {istate} { send-command "setbin $istate" }
    proc settxcompst {istate} { send-command "settxcompst $istate" }
    proc settxcompvalue {f} { send-command "settxcompvalue $f" }
    proc setoscphase {fphase} { send-command "setoscphase $fphase" }
    proc setpanamode {imode} { send-command "setpanamode $imode" }
    proc setrxmetermode {imode} { send-command "setrxmetermode $imode" }
    proc settxmetermode {imode} { send-command "settxmetermode $imode" }
    proc setrxdcblockgain {fgain} { send-command "setrxdcblockgain $fgain" }
    proc setrxdcblock {istate} { send-command "setrxdcblock $istate" }
    proc settxdcblock {istate} { send-command "settxdcblock $istate" }
    proc mox {onoff user password} { send-command "mox [on-or-off $onoff] $user $password" }
    proc settxamcarrierlevel {flevel user password} { send-command "settxamcarrierlevel $flevel $user $password" }
    proc settxamcarrierlevel {flevel} { send-command "settxamcarrierlevel $flevel" }
    proc settxalcstate {istate} { send-command "settxalcstate $istate" }
    proc settxalcattack {i} { send-command "settxalcattack $i" }
    proc settxalcdecay {i} { send-command "settxalcdecay $i" }
    proc settxalcbot {f} { send-command "settxalcbot $f" }
    proc settxalchang {i} { send-command "settxalchang $i" }
    proc settxlevelerstate {i} { send-command "settxlevelerstate $i" }
    proc settxlevelerattack {i} { send-command "settxlevelerattack $i" }
    proc settxlevelerdecay {i} { send-command "settxlevelerdecay $i" }
    proc settxlevelerhang {i} { send-command "settxlevelerhang $i" }
    proc settxlevelermaxgain {i} { send-command "settxlevelermaxgain $i" }; # it was float for rx agc max gain?
    proc settxagcff {i} { send-command "settxagcff $i" }
    proc settxagcffcompression {f} { send-command "settxagcffcompression $f" }
    proc setcorrecttxiqmu {f} { send-command "setcorrecttxiqmu $f" }
    proc setcorrecttxiqw {f1 f2} { send-command "setcorrecttxiqw $f1 $f2" }
    proc setfadelevel {i} { send-command "setfadelevel $i" }
    proc setsquelchval {f} { send-command "setsquelchval $f" }
    proc setsubrxquelchval {f} { send-command "setsubrxquelchval $f" }; # lost s in squelch
    proc setsquelchstate {onoff} { send-command "setsquelchstate [on-or-off $onoff]" }
    proc setsubrxquelchstate {onoff} { send-command "setsubrxquelchstate [on-or-off $onoff]" }; # lost s in squelch
    proc setspectrumpolyphase {tf} { send-command "setspectrumpolyphase [true-or-false $tf]" }
    proc setocoutputs {s} { send-command "setocoutputs $s" }; # to ozy?
    proc setwindow {iwindow} { send-command "setwindow $iwindow" }
    proc setclient {sclient} { send-command "setclient $sclient" }
    proc setiqenable {tf} { send-command "setiqenable [true-or-false $tf]" }
    proc setiqmethod {tf} { send-command "setiqmethod [true-or-false $tf]" }
    proc settxiqenable {tf} { send-command "settxiqenable [true-or-false $tf]" }
    proc testbutton {tf} { send-command "testbutton [true-or-false $tf]" }
    proc testslider {i} { send-command "testslider $i" }
    proc rxiqmuval {f} { send-command "rxiqmuval $f" }
    proc txiqcorrectval {f1 f2} { send-command "txiqcorrectval $f1 $f2" }
    proc txiqphasecorrectval {f} { send-command "txiqphasecorrectval $f" }
    proc txiqgaincorrectval {f} { send-command "txiqgaincorrectval $f" }
    proc rxiqphasecorrectval {f} { send-command "rxiqphasecorrectval $f" }
    proc rxiqgaincorrectval {f} { send-command "rxiqgaincorrectval $f" }
    proc rxiqcorrectwr {f1 f2} { send-command "rxiqcorrectwr $f1 $f2" }
    proc rxiqcorrectwi {f1 f2} { send-command "rxiqcorrectwi $f1 $f2" }
    proc setfps {iwidth ifps} { send-command "setfps $iwidth $ifps" }
    proc zoom {izoom} { send-command "zoom $izoom" }
    proc setmaster {suser spassword} { send-command "setmaster $suser $spassword" }

    ########################################################################
    # no such commands in dspserver
    # proc getbandscope {iwidth} { send-command "getbandscope $iwidth" }
    # proc setalcstate {i} { send-command "setalcstate $i" }

    ########################################################################
    #
    # hardware
    #
    proc *hardware? {} { send-command "*hardware?" }

    # from hardware_hermes
    namespace eval hermes {
	proc *getserial? {} { send-command "*getserial?" }
	proc *setattenuator {i} { send-command "*setattenuator $i" }
	proc *alextxrelay {i}  { send-command "*alextxrelay $i" }
	proc *hermesmicboost {i} { send-command "*hermesmicboost $i" }
	proc *settxdrive {i} { send-command "*settxdrive $i" }
	proc *settxlineingain {i} { send-command "*settxlineingain $i" }
	proc *dither {onoff} { send-command "*dither [on-or-off $onoff]" }
	proc *preamp {onoff} { send-command "*preamp [on-or-off $onoff]" }
	proc *random {onoff} { send-command "*random [on-or-off $onoff]" }
    }
    # from hardware_hiqsdr
    namespace eval hiqsdr {
	proc *getserial? {} { send-command "*getserial?" }
	proc *getpreselector? {i} { send-command "*getpreselector? $i" }
	proc *getpreampstatus? {} { send-command "*getpreampstatus?" }
	proc *setattenuator {i} { send-command "*setattenuator $i" }
	proc *selectantenna {i} { send-command "*selectantenna $i" }
	proc *selectpresel {i} { send-command "*selectpresel $i" }
	proc *activatepreamp {i} { send-command "*activatepreamp $i" }
    }
    namespace eval perseus {
	proc *getserial? {} { send-command "*getserial?" }
	proc *setattenuator {i} { send-command "*setattenuator $i" }
	proc *dither {i} { send-command "*dither $i" }
	proc *preamp {i} { send-command "*preamp $i" }
    }
    namespace eval rtlsdr {
	proc *getserial? {} { send-command "*getserial?" }
	proc *setattenuator {i} { send-command "*setattenuator $i" }
    }
    namespace eval sdr1000 {
	proc *setattenuator {i} { send-command "*setattenuator $i" }
	proc *setspurreduction {i} { send-command "*setspurreduction $i" }
	proc *getpaadc? {i} { send-command "*getpaadc? $i" }
    }
    namespace eval sdriq {
	proc *getserial? {} { send-command "*getserial?" }
	proc *setattenuator {i} { send-command "*setattenuator $i" }
    }
    namespace eval sdrplay {
	proc *setattenuator {i} { send-command "*setattenuator $i" }
    }
}
