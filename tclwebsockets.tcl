#
# tclwebsockets
#
# Freely redistributable under the BSD license.  See LICENSE
# for details.
#


proc websocket::handler {args} {

	if {[llength $args] < 2 || [llength $args] % 2 != 0} {
		error "Expecting even number of arguments"
	}

	set handlerName ""
	set handlerStatevars ""
	set handlerEvents 0
	foreach {key value} $args {
		switch -exact $key {
			-name {
				if {$handlerName != ""} {
					error "Already supplied: $key"
				}
				if {![string is wordchar -strict $value]} {
					error "Invalid name: $value"
				}
				set handlerName $value
			}
			-statevars {
				if {$handlerStatevars != ""} {
					error "Already supplied: $key"
				}
				set handlerStatevars $value
			}
			-events {
				if {$handlerEvents != ""} {
					error "Already supplied: $key"
				}
				if {[llength $value] < 3 || [llength $value] % 3 != 0} {
					error "Expecting a list argument with a multiple of 3 items to -events"
				}
				foreach {eventName eventArgs eventProc} $value {
					switch -exact $eventName {
						"established" -
						"client-connection-error" -
						"client-established" -
						"closed" -
						"receive" -
						"client-receive" -
						"client-receive-pong" -
						"client-writeable" -
						"server-writeable" -
						"http" -
						"broadcast" -
						"filter-network-connection" -
						"filter-protocol-connection" {
							# recognized eventName
							set ::websockets::handlerMethods($handlerName:$eventName) [list $eventArgs $eventProc]
						}
						default {
							error "Unrecognized event: $eventName"
						}
					}
				}
				incr handlerEvents
			}
			default {
				error "Unrecognized option: $key"
			}
		}
	}

	if {$handlerName == ""} {
		error "Required option -name was not given"
	}
	if {$handlerEvents == 0} {
		error "Require option -events was not given"
	}


	set ::websockets::handlerRegistry($handlerName) [list statevars $handlerStatevars]
}
