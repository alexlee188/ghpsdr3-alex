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

package require Tcl
package require snit

namespace eval ::sdr {}

snit::type ::sdr::command {
    ##
    ## procs and variables which are independent of instance
    ##

    # use a dictionary to translate symbolic values and accept valid numeric values
    proc fix-dict {m dict msg} {
	if {$m in [dict values $dict]} {
	    return $m
	} elseif {$m in [dict keys $dict]} { 
	    return [dict get $dict $m]
	} else {
	    error $msg
	}
    }

    # modes of demodulation available
    # should be part of sdrtypes
    # leaving out SPEC 8 and DRM 11, not implemented
    typevariable modes-dict [dict create LSB 0 USB 1 DSB 2 CWL 3 CWU 4 FM 5 AM 6 DIGU 7 DIGL 9 SAM 10 ]
    proc get-modes {} { return [dict keys ${modes-dict}] }
    proc fix-mode {m} { return [fix-dict $m ${modes-dict} "unrecognized mode token $m"] }

    # pwsmodes, spectrum types 
    typevariable pwsmodes-dict [dict create PostFilter 0 PreFilter 1 SemiRaw 2 PostDet 4 ]
    proc get-pwsmodes {} { return [dict keys ${pwsmodes-dict}] }
    proc fix-pwsmode {m} { return [fix-dict $m ${pwsmodes-dict} "unrecognized pws mode token $m"] }

    # the encodings of audio - numeric enumeration by index in list
    typevariable audio-encodings [dict create ALAW 0 PCM 1]
    proc get-audio-encodings {} { return [dict keys ${audio-encodings}] }
    proc fix-encoding {e} { return [fix-dict $e ${audio-encodings} "unrecognized audio encoding token $e"] }

    # the encodings of mic audio - numeric enumeration by index in list
    # oh, can't do PCM on the microphone side?
    typevariable mic-audio-encodings [dict create ALAW 0]

    # audiostream defaults
    # cannot seem to reduce the size of the audio buffer, which is a waste, that's a 1/4 second buffer!
    typevariable audiostream-defaults [dict create buffer_size 2000 samplerate 8000 channels 1 encoding ALAW mic-encoding ALAW]

    # agc values
    typevariable agcmodes-dict [dict create FIXED 0 LONG 1 SLOW 2 MEDIUM 3 FAST 4]
    proc get-agcmodes {} { return [dict keys ${agcmodes-dict}] }
    proc fix-agcmode {agc} { return [fix-dict $agc ${agcmodes-dict} "unrecognized agc token $agc"] }
		    
    # is cmd a command which slaves are permitted to run
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

    # enforce an on|off representation of a boolean
    typevariable on-or-off-dict [dict create off off 0 off false off on on 1 on true on]
    proc on-or-off {v} { return [fix-dict $v ${on-or-off-dict} "on-or-off invalid value {$v}"] }

    # enforce a true|false representation of a boolean
    typevariable true-or-false-dict [dict create off false 0 false false false on true 1 true true true]
    proc true-or-false {v} { return [fix-dict $v ${true-or-false-dict} "true-or-false invalid value {$v}"] }

    # fill a command string with zeroes to an $n byte block
    proc fill {n command} {
	binary scan $command c* b
	while {[llength $b] < $n} { lappend b 0 }
	return [binary format c* $b]
    }

    ##
    ## begin instance code
    ##
    option -parent -readonly true -configuremethod Configure

    component parent
    
    method {Configure -parent} {value} { set parent $value }

    # send a command
    method send {command} {
	if {[$parent.connect is-connected]} {
	    verbose-puts "send>> $command"
	    $parent.connect write [fill 64 $command]
	    $parent.connect flush
	}
    }

    # send mic audio data
    method mic {audio} { 
	if {[$parent.connect is-connected]} {
	    verbose-puts "send>> mic ..."
	    $parent.connect write {mic }
	    $parent.connect write $audio
	    $parent.connect flush
	}
    }	      

    # queries - these are the only real dspserver commands with hyphens
    # query if we are master of the dspserver
    method q-master {} { $self send q-master }
    # query the version of something
    method q-version {} { $self send q-version }
    # query the local oscillator offset
    method q-loffset {} { $self send q-loffset }
    # query about protocol 3
    method q-protocol3 {} { $self send q-protocol3 }
    method q-server {} { $self send q-server }
    method q-info {} { $self send q-info }
    method q-cantx {user password} { $self send "q-cantx#$user#$password" }
    method q-rtpport {} { $self send q-rtpport }
    
    # commands in order of appearance in dspserver/client.c/readcb()
    method getspectrum {iwidth} { $self send "getspectrum $iwidth" }
    method setfrequency {ihz} { $self send "setfrequency $ihz" }
    method setpreamp {s} { $self send "setpreamp $s" }
    method setmode {imode} { $self send "setmode [fix-mode $imode]" }
    method setfilter {ilo ihi} { $self send "setfilter $ilo $ihi" }
    method setagc {iagc} { $self send "setagc [fix-agcmode $iagc]" }
    method setfixedagc {iagc} { $self send "setfixedagc $iagc" }
    method enablenotchfilter {ivfo iindex ienabled} { $self send "enablenotchfilter $ivfo $iindex $ienabled" }
    method setnotchfilter {ivfo iindex fbandwidth ffrequency} { $self send "setnotchfilter $ivfo $iindex $fbandwidth $ffrequency" }
    method setagctlevel {i} { $self send "setagctlevel $i" }
    method setrxgreqcmd {i} { $self send "setrxgreqcmd $i" }
    method setrx3bdgreq {pre b1 b2 b3} { $self send "setrx3bdgreq $pre $b1 $b2 $b3" }
    method setrx10bdgreq {pre b1 b2 b3 b4 b5 b6 b7 b8 b9 b10} { $self send "setrx10bdgreq $pre $b1 $b2 $b3 $b4 $b5 $b6 $b7 $b8 $b9 $b10" }
    method settxgreqcmd {i} { $self send "settxgreqcmd $i" }
    method settx3bdgreq {pre b1 b2 b3} { $self send "settx3bdgreq $pre $b1 $b2 $b3" }
    method settx10bdgreq {pre b1 b2 b3 b4 b5 b6 b7 b8 b9 b10} { $self send "settx10bdgreq $pre $b1 $b2 $b3 $b4 $b5 $b6 $b7 $b8 $b9 $b10" }
    method setnr {val} { $self send "setnr [true-or-false $val]" }
    method setnb {val} { $self send "setnb [true-or-false $val]" }
    method setsdrom {v} { $self send "setsdrom [true-or-false $v]" }
    method setanf {tf} { $self send "setanf [true-or-false $tf]" }
    method setrxoutputgain {i} { $self send "setrxoutputgain $i" }
    method setsubrxoutputgain {i} { $self send "setsubrxoutputgain $i" }
    method startaudiostream {ibufsize irate ichannels imicencoding} { $self send "startaudiostream $ibufsize $irate $ichannels [fix-encoding $imicencoding]" }
    method startrtpstream {iport iencoding irate ichannels} { $self send "startrtpstream $iport $iencoding $irate $ichannels" }
    method stopaudiostream {} { $self send "stopaudiostream" };  #  does nothing
    method setencoding {iencoding} { $self send "setencoding [fix-encoding $iencoding]" }
    method setsubrx {int} { $self send "setsubrx $int" }
    method setsubrxfrequency {int} { $self send "setsubrxfrequency $int" }; #  offset from setfrequency
    method setpan {f} { $self send "setpan $f" }
    method setsubrxpan {f} { $self send "setsubrxpan $f" }
    method record {s} { $self send "record $s" }; #  send to ozy $s
    method setanfvals {itaps idelay fgain fleakage} { $self send "setanfvals $itaps $idelay $fgain $fleakage" }
    method setnrvals {itaps idelay fgain fleakage} { $self send "setnrvals $itaps $idelay $fgain $fleakage" }
    method setrxagcattack {i} { $self send "setrxagcattack $i" }
    method setrxagcdecay {i} { $self send "setrxagcdecay $i" }
    method setrxagchang {i} { $self send "setrxagchang $i" }
    method setrxagchanglevel {f} { $self send "setrxagchanglevel $f" }
    method setrxagchangthreshold {i} { $self send "setrxagchangthreshold $i" }
    method setrxagcthresh {f} { $self send "setrxagcthresh $f" }
    method setrxagcmaxgain {f} { $self send "setrxagcmaxgain $f" }
    method setrxagcslope {i} { $self send "setrxagcslope $i" };    # int? is that right?
    method setnbvals {fthreshold} { $self send "setnbvals $fthreshold" }
    method setsdromvals {fthreshold} { $self send "setsdromvals $fthreshold" }
    method setpwsmode {istate} { $self send "setpwsmode $istate" }
    method setbin {istate} { $self send "setbin $istate" }
    method settxcompst {istate} { $self send "settxcompst $istate" }
    method settxcompvalue {f} { $self send "settxcompvalue $f" }
    method setoscphase {fphase} { $self send "setoscphase $fphase" }
    method setpanamode {imode} { $self send "setpanamode $imode" }
    method setrxmetermode {imode} { $self send "setrxmetermode $imode" }
    method settxmetermode {imode} { $self send "settxmetermode $imode" }
    method setrxdcblockgain {fgain} { $self send "setrxdcblockgain $fgain" }
    method setrxdcblock {istate} { $self send "setrxdcblock $istate" }
    method settxdcblock {istate} { $self send "settxdcblock $istate" }
    method mox {onoff user password} { $self send "mox [on-or-off $onoff] $user $password" }
    method settxamcarrierlevel {flevel user password} { $self send "settxamcarrierlevel $flevel $user $password" }
    method settxamcarrierlevel {flevel} { $self send "settxamcarrierlevel $flevel" }
    method settxalcstate {istate} { $self send "settxalcstate $istate" }
    method settxalcattack {i} { $self send "settxalcattack $i" }
    method settxalcdecay {i} { $self send "settxalcdecay $i" }
    method settxalcbot {f} { $self send "settxalcbot $f" }
    method settxalchang {i} { $self send "settxalchang $i" }
    method settxlevelerstate {i} { $self send "settxlevelerstate $i" }
    method settxlevelerattack {i} { $self send "settxlevelerattack $i" }
    method settxlevelerdecay {i} { $self send "settxlevelerdecay $i" }
    method settxlevelerhang {i} { $self send "settxlevelerhang $i" }
    method settxlevelermaxgain {i} { $self send "settxlevelermaxgain $i" }; # it was float for rx agc max gain?
    method settxagcff {i} { $self send "settxagcff $i" }
    method settxagcffcompression {f} { $self send "settxagcffcompression $f" }
    method setcorrecttxiqmu {f} { $self send "setcorrecttxiqmu $f" }
    method setcorrecttxiqw {f1 f2} { $self send "setcorrecttxiqw $f1 $f2" }
    method setfadelevel {i} { $self send "setfadelevel $i" }
    method setsquelchval {f} { $self send "setsquelchval $f" }
    method setsubrxquelchval {f} { $self send "setsubrxquelchval $f" }; # lost s in squelch
    method setsquelchstate {onoff} { $self send "setsquelchstate [on-or-off $onoff]" }
    method setsubrxquelchstate {onoff} { $self send "setsubrxquelchstate [on-or-off $onoff]" }; # lost s in squelch
    method setspectrumpolyphase {tf} { $self send "setspectrumpolyphase [true-or-false $tf]" }
    method setocoutputs {s} { $self send "setocoutputs $s" }; # to ozy?
    method setwindow {iwindow} { $self send "setwindow $iwindow" }
    method setclient {sclient} { $self send "setclient $sclient" }
    method setiqenable {tf} { $self send "setiqenable [true-or-false $tf]" }
    method setiqmethod {tf} { $self send "setiqmethod [true-or-false $tf]" }
    method settxiqenable {tf} { $self send "settxiqenable [true-or-false $tf]" }
    method testbutton {tf} { $self send "testbutton [true-or-false $tf]" }
    method testslider {i} { $self send "testslider $i" }
    method rxiqmuval {f} { $self send "rxiqmuval $f" }
    method txiqcorrectval {f1 f2} { $self send "txiqcorrectval $f1 $f2" }
    method txiqphasecorrectval {f} { $self send "txiqphasecorrectval $f" }
    method txiqgaincorrectval {f} { $self send "txiqgaincorrectval $f" }
    method rxiqphasecorrectval {f} { $self send "rxiqphasecorrectval $f" }
    method rxiqgaincorrectval {f} { $self send "rxiqgaincorrectval $f" }
    method rxiqcorrectwr {f1 f2} { $self send "rxiqcorrectwr $f1 $f2" }
    method rxiqcorrectwi {f1 f2} { $self send "rxiqcorrectwi $f1 $f2" }
    method setfps {iwidth ifps} { $self send "setfps $iwidth $ifps" }
    method zoom {izoom} { $self send "zoom $izoom" }
    method setmaster {suser spassword} { $self send "setmaster $suser $spassword" }

    ########################################################################
    # no such commands in dspserver
    # proc getbandscope {iwidth} { send-command "getbandscope $iwidth" }

    ########################################################################
    #
    # hardware
    #
    method *hardware? {} { $self send "*hardware?" }

    # from hardware_hermes
    method hermes*getserial? {} { $self send "*getserial?" }
    method hermes*setattenuator {i} { $self send "*setattenuator $i" }
    method hermes*alextxrelay {i}  { $self send "*alextxrelay $i" }
    method hermes*hermesmicboost {i} { $self send "*hermesmicboost $i" }
    method hermes*settxdrive {i} { $self send "*settxdrive $i" }
    method hermes*settxlineingain {i} { $self send "*settxlineingain $i" }
    method hermes*dither {onoff} { $self send "*dither [on-or-off $onoff]" }
    method hermes*preamp {onoff} { $self send "*preamp [on-or-off $onoff]" }
    method hermes*random {onoff} { $self send "*random [on-or-off $onoff]" }
    # from hardware_hiqsdr
    method hiqsdr*getserial? {} { $self send "*getserial?" }
    method hiqsdr*getpreselector? {i} { $self send "*getpreselector? $i" }
    method hiqsdr*getpreampstatus? {} { $self send "*getpreampstatus?" }
    method hiqsdr*setattenuator {i} { $self send "*setattenuator $i" }
    method hiqsdr*selectantenna {i} { $self send "*selectantenna $i" }
    method hiqsdr*selectpresel {i} { $self send "*selectpresel $i" }
    method hiqsdr*activatepreamp {i} { $self send "*activatepreamp $i" }
    # from hardware_perseus
    method perseus*getserial? {} { $self send "*getserial?" }
    method perseus*setattenuator {i} { $self send "*setattenuator $i" }
    method perseus*dither {i} { $self send "*dither $i" }
    method perseus*preamp {i} { $self send "*preamp $i" }
    # from hardware_rtlsdr
    method rtlsdr*getserial? {} { $self send "*getserial?" }
    method rtlsdr*setattenuator {i} { $self send "*setattenuator $i" }
    # from hardware_sdr1000
    method sdr1000*setattenuator {i} { $self send "*setattenuator $i" }
    method sdr1000*setspurreduction {i} { $self send "*setspurreduction $i" }
    method sdr1000*getpaadc? {i} { $self send "*getpaadc? $i" }
    # from hardware_sdriq
    method sdriq*getserial? {} { $self send "*getserial?" }
    method sdriq*setattenuator {i} { $self send "*setattenuator $i" }
    # from hardware_sdrplay
    method sdrplay*setattenuator {i} { $self send "*setattenuator $i" }
}
