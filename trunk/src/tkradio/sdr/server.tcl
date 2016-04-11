##
## servers from napan.com and locally
## status call loc band rig ant time addr port ? ?
##
## the problem here is that the menus may need updating, how to notify?
## and what do I mean by lservers?
##
package provide sdr::server 1.0

package require http 2.8.8

namespace eval ::sdr {
    set rservers [dict create];	# remote servers
    set lservers [dict create];	# local servers
    set listeners {}
}

proc server-request-complete {token} {
    if {[http::status $token] eq {ok}} {
	# puts "server-request-complete and ok"
	set ::sdr::rservers [dict create]
	foreach line [split [string trim [http::data $token]] \n] {
	    # puts "found $line"
	    foreach {status call loc band rig ant time addr port x y} [split $line ~] break
	    dict set ::sdr::rservers $call [dict create addr $addr port $port loc $loc band $band rig $rig ant $ant time $time status $status]
	}
	foreach cb $::sdr::listeners {
	    # puts "calling back $cb [server-names]"
	    {*}$cb [server-names]
	    # puts "called back"
	}
	http::cleanup $token
	after 30000 [list server-request {}]
    } else {
	# puts "server-request-complete and not ok"
	http::cleanup $token
	after 2000 [list server-request {}]
    }
}
proc server-request {callback} {
    # puts "server-request {$callback}"
    if {$callback ne {}} { lappend ::sdr::listeners $callback }
    # puts "info command http::* => {[info command http::*]}"
    ::http::geturl "http://napan.com/qtradio/qtradiolist.pl" -command server-request-complete -timeout 2000
}
proc server-names {} {
    return [concat [dict keys $::sdr::lservers] [dict keys $::sdr::rservers]]
}
proc server-address-port {name} {
    if {[dict exists $::sdr::rservers $name]} {
	return [list [dict get $::sdr::rservers $name addr] [dict get $::sdr::rservers $name port]]
    }
    if {[dict exists $::sdr::lservers $name]} {
	return [list [dict get $::sdr::lservers $name addr] [dict get $::sdr::lservers $name port]]
    }
    return {}
}
proc server-local-install {name addr port} {
    dict set ::sdr::lservers $name [dict create addr $addr port $port]
}
