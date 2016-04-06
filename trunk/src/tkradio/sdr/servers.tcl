##
## servers from napan.com and locally
## status call loc band rig ant time addr port ? ?
##
## the problem here is that the menus may need updating, how to notify?
## and what do I mean by lservers?
##
package provide sdr::servers 1.0

package require http

namespace eval ::sdr {
    set rservers [dict create];	# remote servers
    set lservers [dict create];	# local servers
    set listeners {}
}

proc servers-names-update {callback} {
    # puts "servers-names-update $callback"
    lappend ::sdr::listeners $callback
}
proc servers-request-complete {token} {
    if {[http::status $token] eq {ok}} {
	# puts "servers-request-complete and ok"
	set ::sdr::rservers [dict create]
	foreach line [split [string trim [http::data $token]] \n] {
	    foreach {status call loc band rig ant time addr port x y} [split $line ~] break
	    dict set ::sdr::rservers $call [dict create addr $addr port $port loc $loc band $band rig $rig ant $ant time $time status $status]
	}
	foreach cb $::sdr::listeners {
	    # puts "calling back $cb"
	    {*}$cb [servers-names]
	    # puts "called back"
	}
	http::cleanup $token
	after 30000 servers-request
    } else {
	# puts "servers-request-complete and not ok"
	http::cleanup $token
	after 2000 servers-request
    }
}
proc servers-request {} {
    # puts "servers-request"
    http::geturl "http://napan.com/qtradio/qtradiolist.pl" -command servers-request-complete -timeout 2000
}
proc servers-names {} {
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


