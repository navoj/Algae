/*
   psr.h -- definitions common to parser and scanner

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
*/

/* $Id: psr.h,v 1.3 1997/02/21 09:54:04 ksh Exp $ */

#ifndef   PSR_H
#define   PSR_H		1

#include "algae.h"
#include "ptypes.h"
#include "function.h"

#ifndef PSR_DATA_HERE
#define EXTERN  extern
#else
#define EXTERN  /* nothing */
#endif


EXTERN char *curr_line ;  /* line containing current token */
EXTERN int  curr_line_no ;
EXTERN char *curr_file ;  /* name of the current file */
EXTERN int  curr_offset  ;
EXTERN int  flush_flag ;
EXTERN int  interactive ;
EXTERN int  brace_cnt ;
EXTERN int  flow_cnt ;

extern FILE *yyin ;
extern int  da_flag ;

#define   NO_CREATE	0
#define   CREATE	1

extern void PROTO (da_function, (UF_BLOCK *ufp, FILE *fp));
extern void PROTO (var_ref_function, (UF_BLOCK *u, FILE *fp));

#endif
