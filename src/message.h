/*
   message.h -- Prototypes for `message.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: message.h,v 1.1.1.1 1996/04/17 05:56:19 ksh Exp $ */

#ifndef MESSAGE_H
#define MESSAGE_H	1

#if HAVE_STDARG_H
extern void PROTO (p_error, (char *fmt,...));
extern void PROTO (wipeout, (char *fmt,...));
extern void PROTO (detour, (char *fmt,...));
extern void PROTO (fail, (char *fmt,...));
extern void PROTO (warn, (char *fmt,...));
extern void PROTO (inform, (char *fmt,...));
extern void PROTO (math_error, (void));
#else
extern void p_error ();
extern void wipeout ();
extern void detour ();
extern void fail ();
extern void warn ();
extern void inform ();
extern void math_error ();
#endif

#endif /* MESSAGE_H */
