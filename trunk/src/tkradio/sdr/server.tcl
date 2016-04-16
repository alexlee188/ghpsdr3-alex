##
## servers from napan.com and locally
## status call loc band rig ant time addr port ? ?
##
## the problem here is that the menus may need updating, how to notify?
## and what do I mean by lservers?
##
package provide sdr::server 1.0

package require Tcl
package require snit
package require http 2.8.8

snit::type sdr::server {
    option -parent
    
    variable rservers [dict create];	# remote servers
    variable lservers [dict create];	# local servers
    variable servers [dict create]
    variable listeners {}
    
    method request-complete {token} {
	if {[http::status $token] eq {ok}} {
	    # puts "request-complete and ok"
	    set rservers [dict create]
	    foreach line [split [string trim [http::data $token]] \n] {
		# puts "found $line"
		foreach {status call loc band rig ant time addr port x y} [split $line ~] break
		dict set rservers $call [dict create addr $addr port $port loc $loc band $band rig $rig ant $ant time $time status $status]
	    }
	    set servers [dict merge $lservers $rservers]
	    foreach cb $listeners {
		# puts "calling back $cb [$self names]"
		{*}$cb [$self names]
		# puts "called back"
	    }
	    http::cleanup $token
	    after 30000 [list {*}[mymethod request] {}]
	} else {
	    # puts "server-request-complete and not ok"
	    http::cleanup $token
	    after 2000 [list {*}[mymethod request] {}]
	}
    }

    method request {callback} {
	# puts "server-request {$callback}"
	if {$callback ne {} && [lsearch $listeners $callback] < 0} {
	    lappend listeners $callback
	}
	# puts "info command http::* => {[info command http::*]}"
	::http::geturl "http://napan.com/qtradio/qtradiolist.pl" -command [mymethod request-complete] -timeout 2000
    }

    method names {} { return [dict keys $servers] }

    method exists {name} { return [dict exists $servers $name] }
    method get {name key} { return [dict get $servers $name $key] }
    method address-port {name} {
	if {[$self exists $name]} {
	    return [list [$self get $name addr] [$self get $name port]]
	}
	return {}
    }
    method add-local {name addr port} {
	set dict [dict create addr $addr port $port]
	dict set lservers $name $dict
	dict set servers $name $dict
    }
}
