

proc websocket::handler {args} {

	if {[llength $args] < 2 || [llength $args] % 2 != 0} {
		error "Expecting even number of arguments"
	}

	set handlerName ""
	set handlerStatevars ""
	set handlerEvents ""
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
						"broadcast" -
						"receive" -
						"filter" {
							# no problem
						}
						default {
							error "Unrecognized event: $eventName"
						}
					}
				}
				set handlerEvents $value
			}
			default {
				error "Unrecognized option: $key"
			}
		}
	}

	if {$handlerName == ""} {
		error "Required option -name was not given"
	}
	if {$handlerEvents == ""} {
		error "Require option -events was not given"
	}


	set ::websockets::handlerRegistry($handlerName) [list statevars $handlerStatevars events $handlerEvents]
}
