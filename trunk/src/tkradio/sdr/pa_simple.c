/* -*- mode: C; tab-width: 8; -*-
**
** Copyright (C) 2016 by Roger E Critchlow Jr, Cambridge, MA, USA.
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Softwaret
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

/*
** a simple pulse audio extension for Tcl so I can play back audio
** because the snack extension is broken
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#include <tcl.h>

/*
** technically, only a single stream interface
*/

static pa_simple *s;
static pa_sample_spec ss;
#define NBYTES 8*1024
static char samples[NBYTES];

/* return an TCL_ERROR with the msg in the interpreter error slot */
static int _error(Tcl_Interp *interp, char *msg) {
  Tcl_SetObjResult(interp, Tcl_ObjPrintf("%s", msg));
  return TCL_ERROR;
}
/* return the list of strings which pulse recognizes as sample formats */
/* [pa::simple::sample-formats] -> {formats} */
static int _sample_formats(ClientData clientData, Tcl_Interp *interp, int argc, Tcl_Obj* const *objv) {
  Tcl_Obj *formats = Tcl_NewListObj(0, NULL);
  Tcl_ListObjAppendElement(interp, formats, Tcl_ObjPrintf("%s", pa_sample_format_to_string(PA_SAMPLE_U8)));
  Tcl_ListObjAppendElement(interp, formats, Tcl_ObjPrintf("%s", pa_sample_format_to_string(PA_SAMPLE_S16LE)));
  Tcl_ListObjAppendElement(interp, formats, Tcl_ObjPrintf("%s", pa_sample_format_to_string(PA_SAMPLE_S16BE)));
  Tcl_ListObjAppendElement(interp, formats, Tcl_ObjPrintf("%s", pa_sample_format_to_string(PA_SAMPLE_FLOAT32LE)));
  Tcl_ListObjAppendElement(interp, formats, Tcl_ObjPrintf("%s", pa_sample_format_to_string(PA_SAMPLE_FLOAT32BE)));
  Tcl_ListObjAppendElement(interp, formats, Tcl_ObjPrintf("%s", pa_sample_format_to_string(PA_SAMPLE_ALAW)));
  Tcl_ListObjAppendElement(interp, formats, Tcl_ObjPrintf("%s", pa_sample_format_to_string(PA_SAMPLE_ULAW)));
  Tcl_ListObjAppendElement(interp, formats, Tcl_ObjPrintf("%s", pa_sample_format_to_string(PA_SAMPLE_S32LE)));
  Tcl_ListObjAppendElement(interp, formats, Tcl_ObjPrintf("%s", pa_sample_format_to_string(PA_SAMPLE_S32BE)));
  Tcl_ListObjAppendElement(interp, formats, Tcl_ObjPrintf("%s", pa_sample_format_to_string(PA_SAMPLE_S24LE)));
  Tcl_ListObjAppendElement(interp, formats, Tcl_ObjPrintf("%s", pa_sample_format_to_string(PA_SAMPLE_S24BE)));
  Tcl_ListObjAppendElement(interp, formats, Tcl_ObjPrintf("%s", pa_sample_format_to_string(PA_SAMPLE_S24_32LE)));
  Tcl_ListObjAppendElement(interp, formats, Tcl_ObjPrintf("%s", pa_sample_format_to_string(PA_SAMPLE_S24_32BE)));
  Tcl_SetObjResult(interp, formats);
  return TCL_OK;
}
static int _new(ClientData clientData, Tcl_Interp *interp, int argc, Tcl_Obj* const *objv) {
  if (argc != 7) return _error(interp, "usage: pa::simple::new name playback|record description sample-format channels rate");
  char *appname = Tcl_GetString(objv[1]);
  char *playrec = Tcl_GetString(objv[2]);
  int iplayrec;
  if (strcmp(playrec, "playback") == 0) iplayrec = PA_STREAM_PLAYBACK;
  else if (strcmp(playrec, "record") == 0) iplayrec = PA_STREAM_RECORD;
  else return _error(interp, "invalid stream direction, must be playback or record");
  char *desc = Tcl_GetString(objv[3]);
  char *format = Tcl_GetString(objv[4]);
  if ((ss.format = pa_parse_sample_format(format)) == PA_SAMPLE_INVALID)
    return _error(interp, "invalid sample format");
  int ichannels;
  if (Tcl_GetIntFromObj(interp, objv[5], &ichannels) != TCL_OK)
    return _error(interp, "invalid channels, must be an integer");
  ss.channels = ichannels;
  int irate;
  if (Tcl_GetIntFromObj(interp, objv[6], &irate) != TCL_OK)
    return _error(interp, "invalid rate, must be an integer");
  ss.rate = irate;
  int error;
  s = pa_simple_new(NULL,	/* default server */
		    appname,	/* Our application's name. */
		    iplayrec,	/* Open this stream for recording or playback? */
		    NULL,	/* Use the default device. */
		    desc,	/* Description of our stream. */
		    &ss,	/* Our sample format. */
		    NULL,	/* Use default channel map */
		    NULL,	/* Use default buffering attributes. */
		    &error	/* error code. */
		    );
  if (s == NULL)
    return _error(interp, (char *)pa_strerror(error));
  return TCL_OK;
}

static int _get_latency(ClientData clientData, Tcl_Interp *interp, int argc, Tcl_Obj* const *objv) {
  if (argc != 1) return _error(interp, "usage pa::simple::get-latency");
  Tcl_SetObjResult(interp, Tcl_ObjPrintf("%u", (unsigned)pa_simple_get_latency(s, NULL)));
  return TCL_OK;
}
static int _read(ClientData clientData, Tcl_Interp *interp, int argc, Tcl_Obj* const *objv) {
  int error;
  if (argc != 2) return _error(interp, "usage: pa::simple::read nbytes");
  int nbytes;
  if (Tcl_GetIntFromObj(interp, objv[1], &nbytes) != TCL_OK)
    return _error(interp, "invalid nbytes, must be an integer");
  Tcl_Obj *result = Tcl_NewByteArrayObj(NULL, nbytes);
  if (result == NULL) return _error(interp, "urk");
  char *data = Tcl_GetByteArrayFromObj(result, NULL);
  if (data == NULL) return _error(interp, "urk2");
  int nread = pa_simple_read(s, data, nbytes, &error);
  if (nread < 0) return _error(interp, "read error");
  Tcl_SetObjResult(interp, result);
  return TCL_OK;
}
static int _write(ClientData clientData, Tcl_Interp *interp, int argc, Tcl_Obj* const *objv) {
  int error;
  if (argc != 2) return _error(interp, "usage: pa::simple::write data");
  int ninput;
  char *data = Tcl_GetByteArrayFromObj(objv[1], &ninput);
  if (pa_simple_write(s, data, ninput, &error) < 0)
    return _error(interp, (char *)pa_strerror(error));
  return TCL_OK;
}
static int _flush(ClientData clientData, Tcl_Interp *interp, int argc, Tcl_Obj* const *objv) {
  if (argc != 1) return _error(interp, "usage pa::simple::flush");
  pa_simple_flush(s, NULL);
  return TCL_OK;
}
static int _drain(ClientData clientData, Tcl_Interp *interp, int argc, Tcl_Obj* const *objv) {
  if (argc != 1) return _error(interp, "usage pa::simple::drain");
  pa_simple_drain(s, NULL);
  return TCL_OK;
}
static int _free(ClientData clientData, Tcl_Interp *interp, int argc, Tcl_Obj* const *objv) {
  if (argc != 1) return _error(interp, "usage pa::simple::free");
  pa_simple_free(s);
  return TCL_OK;
}

int DLLEXPORT Pa_simple_Init(Tcl_Interp *interp) {
  // tcl stubs and tk stubs are needed for dynamic loading,
  // you must have this set as a compiler option
#ifdef USE_TCL_STUBS
  if (Tcl_InitStubs(interp, TCL_VERSION, 1) == NULL) {
    Tcl_SetResult(interp, "Tcl_InitStubs failed", TCL_STATIC);
    return TCL_ERROR;
  }
#endif
#ifdef USE_TK_STUBS
  if (Tk_InitStubs(interp, TCL_VERSION, 1) == NULL) {
    Tcl_SetResult(interp, "Tk_InitStubs failed", TCL_STATIC);
    return TCL_ERROR;
  }
#endif
  Tcl_PkgProvide(interp, "pa::simple", "0.0.1");
  Tcl_CreateObjCommand(interp, "pa::simple::sample-formats", _sample_formats, NULL, NULL);
  Tcl_CreateObjCommand(interp, "pa::simple::new", _new, NULL, NULL);
  Tcl_CreateObjCommand(interp, "pa::simple::get-latency", _get_latency, NULL, NULL);
  Tcl_CreateObjCommand(interp, "pa::simple::read", _read, NULL, NULL);
  Tcl_CreateObjCommand(interp, "pa::simple::write", _write, NULL, NULL);
  Tcl_CreateObjCommand(interp, "pa::simple::flush", _flush, NULL, NULL);
  Tcl_CreateObjCommand(interp, "pa::simple::drain", _drain, NULL, NULL);
  Tcl_CreateObjCommand(interp, "pa::simple::free", _free, NULL, NULL);
  return TCL_OK;
}
