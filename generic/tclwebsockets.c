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



struct context_userdata_struct {
  Tcl_Interp *interp;
  Tcl_Command cmdToken;
  struct libwebsocket_context *context;
  struct libwebsocket_protocols *protocols;
};


struct websocket_session_struct {
  const char *handler_name;             // pointer into protocol definition.
  char session_array_name[64];
  char connection_cmd_name[64];

  Tcl_Interp *interp;
  Tcl_Command cmdToken;

  //Tcl_Obj *queued_data;
  struct libwebsocket *socket;
  struct libwebsocket_context *context;
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
  struct websocket_session_struct *session_data = (struct websocket_session_struct*)cData;

  const char *commands[] = {
    "close",
    "write",
    NULL
  };

  enum command_enum {
    CMD_CLOSE,
    CMD_WRITE
  };

  int cmdIndex;


  // basic command line processing
  if (objc < 1) {
    Tcl_WrongNumArgs (interp, 1, objv, "missing argument");
    return TCL_ERROR;
  }

  if (Tcl_GetIndexFromObj(interp, objv[1], commands, "command", TCL_EXACT, &cmdIndex) != TCL_OK) {
    return TCL_ERROR;
  }

  switch (cmdIndex) {
  case CMD_CLOSE: {
    libwebsocket_close_and_free_session(session_data->context, session_data->socket, LWS_CLOSE_STATUS_NORMAL);
    
  }

  case CMD_WRITE: {
    n = libwebsocket_write(session_data->socket, p, n, LWS_WRITE_TEXT);
    if (n < 0) {
      fprintf(stderr, "ERROR writing to socket");
      return 1;
    }
    // otherwise add to queued_data
  }

  default: break;
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
  struct context_userdata_struct *userdata = (struct context_userdata_struct*) cData;

  const char *commands[] = {
    "service",
    "delete",
    NULL
  };

  enum command_enum {
    CMD_SERVICE,
    CMD_DELETE
  };

  int cmdIndex;


  // basic command line processing
  if (objc < 1) {
    Tcl_WrongNumArgs (interp, 1, objv, "missing argument");
    return TCL_ERROR;
  }

  if (Tcl_GetIndexFromObj(interp, objv[1], commands, "command", TCL_EXACT, &cmdIndex) != TCL_OK) {
    return TCL_ERROR;
  }

  switch (cmdIndex) {
  case CMD_SERVICE: {
    // process pending socket events on the listener.
    int n = libwebsocket_service(userdata->context, 50);    // block up to 50ms
    if (n != 0) {
      return TCL_ERROR;
    }
    break;
  }
  case CMD_DELETE: {
    // stop and free the listener socket.
    libwebsocket_context_destroy(userdata->context);

    // free the memory for the protocol array.
    ckfree((char*) userdata->protocols);

    // delete the Tcl command
    Tcl_DeleteCommandFromToken(userdata->interp, userdata->cmdToken);

    break;
  }
  default: break;
  } // end switch

  return 0;
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
		    void *v_session_data, void *in, size_t len)
{
  struct websocket_session_struct *session_data = (struct websocket_session_struct *)v_session_data;
  struct context_userdata_struct *context_data = (struct context_userdata_struct*)libwebsockets_get_user_data(context);
  Tcl_Obj *handlerRegistryList = NULL;


  // When a connection is first established, do some extra work to
  // intitialize the session structure.
  if (reason == LWS_CALLBACK_ESTABLISHED) {
    const struct libwebsocket_protocols *protocol;
    static unsigned long nextCmdIndex = 0;

    // initialize session_data
    protocol = libwebsockets_get_protocol(wsi);
    session_data->handler_name = protocol->name;
    session_data->interp = context_data->interp;
    session_data->socket = wsi;
    session_data->context = context;

    // generate a unique session_array_name and connection_command_name.
    snprintf(session_data->session_array_name, sizeof(session_data->session_array_name), "::websockets::session_data%lu", nextCmdIndex);
    snprintf(session_data->connection_cmd_name, sizeof(session_data->connection_cmd_name), "websocket%lu", nextCmdIndex++);

    //session_data->queued_data = NULL;

    // register a new command in the Tcl interpreter to represent this connection
    // using connection_command_name and tclwebsockets_connectionCmd
    // TODO: supply a delete handler instead of NULL
    session_data->cmdToken = Tcl_CreateObjCommand(session_data->interp, session_data->connection_cmd_name, tclwebsockets_connectionCmd, session_data, NULL);

  }



  // Look up the definition from our registry array.
  handlerRegistryList = Tcl_GetVar2Ex(session_data->interp, "::websockets::handlerRegistry", session_data->handler_name, TCL_GLOBAL_ONLY);
  if (handlerRegistryList == NULL) {
    fprintf(stderr, "callback_websocket_handler called for an unknown protocol \"%s\"\n", session_data->handler_name);
    return -1;
  }


  switch (reason) {
  case LWS_CALLBACK_ESTABLISHED: {
    //"established"
    break;
  }

  case LWS_CALLBACK_BROADCAST: {
    //"broadcast"
    break;
  }

  case LWS_CALLBACK_FILTER_NETWORK_CONNECTION: {
    //"filter-network"
    break;
  }
  case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION: {
    //"filter-protocol"
    break;
  }
  case LWS_CALLBACK_HTTP: {
    //"http"
    break;
  }
  case LWS_CALLBACK_RECEIVE: {
    //"receive"
    // do something with in, len
    break;
  }
  case LWS_CALLBACK_SERVER_WRITEABLE: {
    if (session_data->queued_data != NULL) {
      //...
    }
    break;
  }

  case LWS_CALLBACK_CLOSED: {
    // delete the Tcl command for this connection.
    // unset the session array.
    break;
  }

  default: break;
  }

    

  return 0;
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
  struct context_userdata_struct *userdata = NULL;
  
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
      port = (int) lon;
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
	char *handlerName, *handlerNameCopy;
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
	handlerNameCopy = ckalloc(handlerLen + 1);
	if (handlerNameCopy == NULL) {
	  return TCL_ERROR;
	}
	strncpy(handlerNameCopy, handlerName, handlerLen);
	handlerNameCopy[handlerLen] = '\0';
	protocols[q].name = handlerNameCopy;

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
    } // end switch


  }  // end for

  // require port
  if (port == 0) {
    Tcl_WrongNumArgs (interp, 1, objv, "-port is a required option");
    if (protocols != NULL) ckfree((char*) protocols);
    return TCL_ERROR;
  }
  
  // require handlers
  if (!protocols) {
    Tcl_WrongNumArgs (interp, 1, objv, "-handlers is a required option");
    return TCL_ERROR;
  }


  // allocate a userdata structure.
  userdata = (struct context_userdata_struct*) ckalloc(sizeof(struct context_userdata_struct));
  if (userdata == NULL) {
    Tcl_AppendResult(interp, "libwebsocket init failed", NULL);
    ckfree((char*) protocols);
    return TCL_ERROR;
  }
  userdata->interp = interp;
  userdata->protocols = protocols;


  // start listening.
  context = libwebsocket_create_context(port, interface_name, protocols,
					libwebsocket_internal_extensions,
					(use_ssl ? cert_path : NULL), (use_ssl ? key_path : NULL),
					-1, -1, 0);
  if (context == NULL) {
    Tcl_AppendResult(interp, "libwebsocket init failed", NULL);
    ckfree((char*) userdata);
    ckfree((char*) protocols);
    return TCL_ERROR;
  }

  userdata->context = context;
  libwebsockets_set_user_data(context, userdata);


  // create a Tcl command to interface with this context.
  {
    static unsigned long nextAutoCounter = 0;
    char *commandName = ckalloc(64);
    snprintf(commandName, 64, "lwscontext%lu", nextAutoCounter++);

    // TODO: supply a delete handler instead of NULL.
    userdata->cmdToken = Tcl_CreateObjCommand(interp, commandName, tclwebsockets_contextCmd, userdata, NULL);

    Tcl_SetObjResult(interp, Tcl_NewStringObj(commandName, -1));
  }

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

