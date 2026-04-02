/*
   main.h -- Prototypes for `main.c'.

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
*/

/* $Id: main.h,v 1.2 1997/02/21 09:53:59 ksh Exp $ */

#ifndef MAIN_H
#define MAIN_H

#include "algae.h"

extern RETSIGTYPE PROTO( catch_sigint, ( int i ) );
extern int PROTO( source, ( char *name ) );
extern int PROTO( parse_and_exec, ( FILE *fp, char *file_name ) );
extern void PROTO (initialize, (int argc, char ** argv));
extern void PROTO (push_parser_scanner_state, (FILE * fp, char * file_name));
extern void PROTO (pop_parser_scanner_state, (void));

#endif				/* MAIN_H */
