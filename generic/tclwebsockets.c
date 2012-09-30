/*
 * tclwebsockets
 *
 * Freely redistributable under the BSD license.  See license.terms
 * for details.
 */

#include <tcl.h>
#include <string.h>
#include <sys/syslimits.h>
#include <libwebsockets.h>


#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT




struct websocket_session_struct {
  const char *handler_name;
  char session_array_name[64];
  char connection_cmd_name[64];
  Tcl_Interp *interp;

  Tcl_Obj *queued_data;
};



/*
 *----------------------------------------------------------------------
 *
 * tclwebsockets_connectionCmd --
 *
 *    Allows the caller to invoke various commands against a client connection.
 *
 * Results:
 *    stuff
 *
 *----------------------------------------------------------------------
 */
int
tclwebsockets_connectionCmd(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
  // cData points to websocket_session_struct?

  const char *commands[] = {
    "close",
    "write",
    NULL
  };

  enum command_enum {
    CMD_CLOSE,
    CMD_WRITE
  };

  switch (cmd) {
  case CMD_CLOSE: {
    libwebsocket_close_and_free_session(context, wsi,
					LWS_CLOSE_STATUS_NORMAL);

  }
  case CMD_WRITE: {
    n = libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT);
    if (n < 0) {
      fprintf(stderr, "ERROR writing to socket");
      return 1;
    }
    // otherwise add to queued_data
  }
  }


}


/*
 *----------------------------------------------------------------------
 *
 * tclwebsockets_contextCmd --
 *
 *    Allows the caller to invoke various commands against a server context.
 *
 * Results:
 *    stuff
 *
 *----------------------------------------------------------------------
 */
int
tclwebsockets_contextCmd(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
  // cData points to the context?

  const char *commands[] = {
    "service",
    "delete",
    NULL
  };

  enum command_enum {
    CMD_SERVICE,
    CMD_DELETE
  };

  switch (cmd) {
  case CMD_SERVICE: {
    int n = libwebsocket_service(context, 50);
  }
  case CMD_DELETE: {
    libwebsocket_context_destroy(context);
    // delete the Tcl command
    // ckfree the memory for the protocol array.
  }

}


/*
 *----------------------------------------------------------------------
 *
 * callback_websocket_handler --
 *
 *    Universal callback invoked by libwebsocket whenever an event
 *    occurs on a connection belonging to any of our protocols.
 *
 * Results:
 *    stuff
 *
 *----------------------------------------------------------------------
 */
static int
callback_websocket_handler(struct libwebsocket_context *context,
		    struct libwebsocket *wsi,
		    enum libwebsocket_callback_reasons reason,
		    void *user, void *in, size_t len)
{
  int n;
  struct websocket_session_struct *session_data = (struct websocket_session_struct *)user;
  Tcl_Interp *interp = NULL;
  Tcl_Obj *handlerRegistryList = NULL;


  // When a connection is first established, do some extra work to
  // intitialize the session structure.
  if (reason == LWS_CALLBACK_ESTABLISHED) {
    const struct libwebsocket_protocols *protocol;

    // initialize session_data
    protocol = libwebsockets_get_protocol(wsi);
    session_data->handler_name = protocol->name;

    // generate a unique session_array_name.
    // generate a unique connection_command_name.
    #error TODO

    // find the original Tcl interp.
    #error TODO
    session_data->queued_data = NULL;

    // register a new command in the Tcl interpreter to represent this connection
    // using connection_command_name and tclwebsockets_connectionCmd
  }


  interp = session_data->interp;


  // Look up the definition from our registry array.
  handlerRegistryList = Tcl_GetVar2Ex(interp, "::websockets::handlerRegistry", context->handler_name, TCL_GLOBAL_ONLY);
  if (handlerRegistryList == NULL) {
    fprintf(stderr, "callback_websocket_handler called for an unknown protocol \"%s\"\n", context->handler_name);
    return -1;
  }


  switch (reason) {
  case LWS_CALLBACK_ESTABLISHED: {
    "established"
  }

  case LWS_CALLBACK_BROADCAST: {
    "broadcast"
  }

  case LWS_CALLBACK_FILTER_NETWORK_CONNECTION: {
    //"filter-network"
  }
  case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION: {
    "filter-protocol"
  }
  case LWS_CALLBACK_HTTP: {
    "http"
  }
  case LWS_CALLBACK_RECEIVE: {
    "receive"
  }
  case LWS_CALLBACK_SERVER_WRITEABLE: {
    if (session_data->queued_data != NULL) {
      ...
    }
  }

  case LWS_CALLBACK_CLOSED: {
    // delete the Tcl command for this connection.
    // unset the session array.
  }

  }

    

}


/*
 *----------------------------------------------------------------------
 *
 * tclwebsockets_listenCmd --
 *
 *    Allows the user to start listening 
 *
 * Results:
 *    stuff
 *
 *----------------------------------------------------------------------
 */
int
tclwebsockets_listenCmd(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
  int i;
  int suboptIndex;
  int port = 0;
  int use_ssl = 0;
  char interface_name[128] = "";
  char cert_path[PATH_MAX] = "";
  char key_path[PATH_MAX] = "";
  struct libwebsocket_protocols *protocols = NULL;
  struct libwebsocket_context *context = NULL;
  
  static CONST char *subOptions[] = {
    "-port",
    "-interface"
    "-ssl",
    "-certificate",
    "-privatekey",
    "-handlers",
    NULL
  };

  enum suboptions {
    SUBOPT_PORT,
    SUBOPT_INTERFACE,
    SUBOPT_SSL,
    SUBOPT_CERTIFICATE,
    SUBOPT_PRIVATEKEY,
    SUBOPT_HANDLERS
  };

  // basic command line processing
  if (objc < 3 || (objc & 1) == 0) {
    Tcl_WrongNumArgs (interp, 1, objv, "listen -port integer ?-interface ipaddr? ?-ssl bool? ?-certificate filename? ?-privatekey -filename? ?-handlers list?");
    return TCL_ERROR;
  }


  for (i = 1; i < objc; i++) {
    // argument must be one of the subOptions defined above
    if (Tcl_GetIndexFromObj(interp, objv[i], subOptions, "suboption", TCL_EXACT, &suboptIndex) != TCL_OK) {
      return TCL_ERROR;
    }


    switch ((enum suboptions)suboptIndex) {
    case SUBOPT_PORT: {
      // verify integer
      long lon;

      if (i + 1 >= objc) {
	Tcl_WrongNumArgs (interp, 1, objv, "-port value");
	return TCL_ERROR;
      }

      if (Tcl_GetLongFromObj (interp, objv[++i], &lon) == TCL_ERROR) {
	return TCL_ERROR;
      }
      if (lon <= 0 || lon > 0xFFFF) {
	Tcl_AppendResult(interp, "invalid value for -port", NULL);
	return TCL_ERROR;
      }

      // save to variable
      port = (int) long;
      break;
    }

    case SUBOPT_INTERFACE: {
      // verify ipaddress
      char *value;
      int   len;

      if (i + 1 >= objc) {
	Tcl_WrongNumArgs (interp, 1, objv, "-interface value");
	return TCL_ERROR;
      }

      value = Tcl_GetStringFromObj (objv[++i], &len);
      if (len == 0 || len >= sizeof(interface_name)) {
	Tcl_AppendResult(interp, "invalid value \"", value, "\" for -interface", NULL);
	return TCL_ERROR;
      }

      // save to variable
      strncpy(interface_name, value, sizeof(interface_name));
      interface_name[sizeof(interface_name) - 1] = '\0';
      break;
    }

    case SUBOPT_SSL: {
      // verify boolean
      if (i + 1 >= objc) {
	Tcl_WrongNumArgs (interp, 1, objv, "-ssl value");
	return TCL_ERROR;
      }

      if (Tcl_GetBooleanFromObj (interp, objv[++i], &use_ssl) == TCL_ERROR) {
	return TCL_ERROR;
      }
      break;
    }
    case SUBOPT_CERTIFICATE: {
      char *value;
      int   len;

      if (i + 1 >= objc) {
	Tcl_WrongNumArgs (interp, 1, objv, "-certificate value");
	return TCL_ERROR;
      }

      value = Tcl_GetStringFromObj (objv[++i], &len);
      if (len == 0 || len >= sizeof(cert_path)) {
	Tcl_AppendResult(interp, "invalid value \"", value, "\" for -certificate", NULL);
	return TCL_ERROR;
      }

      // save to variable cert_path
      strncpy(cert_path, value, sizeof(cert_path));
      cert_path[sizeof(cert_path) - 1] = '\0';
      break;
    }
    case SUBOPT_PRIVATEKEY: {
      // save to variable key_path
      char *value;
      int   len;

      if (i + 1 >= objc) {
	Tcl_WrongNumArgs (interp, 1, objv, "-privatekey value");
	return TCL_ERROR;
      }

      value = Tcl_GetStringFromObj (objv[++i], &len);
      if (len == 0 || len >= sizeof(key_path)) {
	Tcl_AppendResult(interp, "invalid value \"", value, "\" for -privatekey", NULL);
	return TCL_ERROR;
      }

      // save to variable
      strncpy(key_path, value, sizeof(key_path));
      key_path[sizeof(key_path) - 1] = '\0';
      break;
    }
    case SUBOPT_HANDLERS: {
      // save to variable
      int num_handlers = 0;
      int q;

      if (protocols != NULL) {
	Tcl_AppendResult(interp, "-handlers may only be specified once", NULL);
	return TCL_ERROR;
      }

      // validate the length of the list of handlers.
      if (Tcl_ListObjLength(interp, objv[++i], &num_handlers) != TCL_OK) {
	return TCL_ERROR;
      }
      if (num_handlers < 1) {
	Tcl_AppendResult(interp, "-handlers must have a non-empty list argument", NULL);
	return TCL_ERROR;
      }

      // allocate memory for the native structure, plus the list terminator.
      protocols = (struct libwebsocket_protocols*) ckalloc(sizeof(struct libwebsocket_protocols) * (num_handlers + 1));
      if (protocols == NULL) {
	return TCL_ERROR;
      }
      memset(protocols, 0, sizeof(struct libwebsocket_protocols) * (num_handlers + 1));

      // populate the protocol structure array.
      for (q = 0; q < num_handlers; q++) {
	Tcl_Obj *handlerNameObj;
	const char *handlerName;
	int handlerLen;
	Tcl_Obj *handlerRegistryList;

	if (Tcl_ListObjIndex(interp, objv[i], q, &handlerNameObj) != TCL_OK) {
	  return TCL_ERROR;
	}
	handlerName = Tcl_GetStringFromObj(handlerNameObj, &handlerLen);

	// Look up the definition from our registry array.
	handlerRegistryList = Tcl_GetVar2Ex(interp, "::websockets::handlerRegistry", handlerName, TCL_GLOBAL_ONLY);
	if (handlerRegistryList == NULL) {
	  Tcl_AppendResult(interp, "-handlers specified \"", handlerName, "\" but was not defined yet", NULL);
	  return TCL_ERROR;
	}

	// Duplicate and copy the handler name.
	protocols[q].name = ckalloc(handlerLen + 1);
	if (protocols[q].name == NULL) {
	  return TCL_ERROR;
	}
	strncpy(protocols[q].name, handlerName, handlerLen);
	protocols[q].name[handlerLen] = '\0';

	// Set the callback and session structure size.
	protocols[q].callback = callback_websocket_handler;
	protocols[q].per_session_data_size = sizeof(struct websocket_session_struct);
      }

      // explicitly zero the terminating entry (even though we memset it all to zero).
      protocols[q].name = NULL;
      protocols[q].callback = NULL;
      protocols[q].per_session_data_size = 0;

      break;
    }
    default: return TCL_ERROR;
    }


  }

  // require port
  if (port == 0) {
    Tcl_WrongNumArgs (interp, 1, objv, "-port is a requirement option");
    return TCL_ERROR;
  }
  
  // require handlers
  if (!protocols) {
    Tcl_WrongNumArgs (interp, 1, objv, "-handlers is a requirement option");
    return TCL_ERROR;
  }


  // start listening.
  context = libwebsocket_create_context(port, interface, protocols,
					libwebsocket_internal_extensions,
					(use_ssl ? cert_path : NULL), (use_ssl ? key_path : NULL),
					-1, -1, opts);
  if (context == NULL) {
    Tcl_AppendResult(interp, "libwebsocket init failed", NULL);
    return TCL_ERROR;
  }


  // TODO: return "context" as a Tcl Object

  return TCL_OK;
}







/*
 *----------------------------------------------------------------------
 *
 * tclwebsockets_Init --
 *
 *	Initialize the tclwebsockets extension.  The string "tclwebsockets" 
 *      in the function name must match the PACKAGE declaration at the top of
 *	configure.in.
 *
 * Results:
 *	A standard Tcl result
 *
 * Side effects:
 *	Several new commands are added to the Tcl interpreter.
 *
 *----------------------------------------------------------------------
 */

EXTERN int
Tclwebsockets_Init(Tcl_Interp *interp)
{
    /*
     * This may work with older versions of Tcl, but I've only tested against Tcl 8.5.
     */
    if (Tcl_InitStubs(interp, "8.5", 0) == NULL) {
	return TCL_ERROR;
    }

    if (Tcl_PkgRequire(interp, "Tcl", "8.5", 0) == NULL) {
	return TCL_ERROR;
    }

    if (Tcl_PkgProvide(interp, "tclwebsockets", PACKAGE_VERSION) != TCL_OK) {
	return TCL_ERROR;
    }

    /* Create the commands */
    Tcl_CreateObjCommand(interp, "websockets::listen", (Tcl_ObjCmdProc *) tclwebsockets_listenCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

    return TCL_OK;
}

