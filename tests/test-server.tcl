# -*- tcl -*-


websocket::handler \
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
		filter {wsi headers} {
			parray headers
			return 0
		}
		writable {wsi} {
			$wsi write $moo
		}
	}


websocket::listen -port 7681 -interface "127.0.0.1" \
	-ssl 1 -certificate "cert.pem" -privatekey "private.pem" \
	-handlers [list "dumb-increment-protocol" "lws-mirror-protocol"]



        context = libwebsocket_create_context(port, interface, protocols,
                                libwebsocket_internal_extensions,
                                cert_path, key_path, -1, -1, opts);
        if (context == NULL) {
                fprintf(stderr, "libwebsocket init failed\n");
                return -1;
        }


