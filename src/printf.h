/*
   printf.h -- Prototypes for `printf.c'.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: printf.h,v 1.2 2003/09/06 17:08:28 ksh Exp $ */

#ifndef PRINTF_H
#define PRINTF_H	1

#include "entity.h"

#if HAVE_STDARG_H
void PROTO (xfprintf, (FILE * stream, char *format,...));
void PROTO (xsprintf, (char * str, char *format,...));
#else
void xfprintf ();
void xsprintf ();
#endif

void PROTO (xputc, (int c, FILE * stream));
void PROTO (xfputs, (char *s, FILE * stream));
ENTITY *PROTO (bi_fprintf, (int n, ENTITY ** args));
ENTITY *PROTO (bi_sprintf, (int n, ENTITY ** args));
ENTITY *PROTO (bi_printf, (int n, ENTITY ** args));

#endif /* PRINTF_H */
