#!/usr/local/bin/tclsh8.5

package require tclwebsockets 1.0


websockets::handler \
	-name "dumb-increment-protocol" \
	-statevars {foo moo} \
	-events {
		established wsi {
			set foo "hello"
			set moo "cow"     
		}	     
		broadcast wsi {
			$wsi write "howdy $foo"
			$wsi close
		} 
		receive {wsi data} {
			puts "got $data"
		}
		filter-network-connection {wsi - headers} {
			parray headers
			return 0
		}
		client-writeable {wsi} {
			$wsi write $moo
		}
	}


websockets::handler \
	-name "lws-mirror-protocol" \
	-events {
		established wsi {
			puts stderr "callback_lws_mirror: LWS_CALLBACK_ESTABLISHED"
		}	     
		broadcast {wsi data} {
			puts stderr "broadcast got $data"
			if {[catch { $wsi write $data } ]} {
				puts stderr "callback_lws_mirror: mirror write failed"
			}
		} 
		receive {wsi data} {
			puts stderr "got $data"
		}
		client-writeable {wsi} {
			$wsi write $moo
		}
	}



set l [websockets::listen -port 7681 -interface "127.0.0.1" \
		   -handlers [list "dumb-increment-protocol" "lws-mirror-protocol"]]

puts $l


#websockets::listen -port 7681 -interface "127.0.0.1" \
#	-ssl 1 -certificate "cert.pem" -privatekey "private.pem" \
#	-handlers [list "dumb-increment-protocol"]


proc idle_service_sock {sock} {
	$sock service
	after idle "idle_service_sock $sock"
}

puts "now servicing listener..."
idle_service_sock $l
vwait forever


#while {1} {
#	$l service
#}

