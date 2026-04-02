/*
   matio.h -- Prototypes for `matio.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
*/

/* $Id: matio.h,v 1.1.1.1 1996/04/17 05:56:19 ksh Exp $ */

#ifndef MATIO_H
#define MATIO_H	1

#include "entity.h"

extern ENTITY *PROTO( bi_getmat, ( int n, ENTITY *fname ) );
extern ENTITY *PROTO( getmat_matrix, ( FILE *stream, char **name ) );
extern void PROTO( reverse_words, ( char *p, int size, int t ) );

#endif				/* GET_H */
