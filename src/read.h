/*
   read.h -- Prototypes for `read.c'.

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: read.h,v 1.4 1997/06/25 07:42:06 ksh Exp $ */

#ifndef READ_H
#define READ_H	1

#include "entity.h"

#define READ_BUF_SIZE 256

extern DATUM *this_many_read;

extern ENTITY *PROTO (bi_read, (int n, ENTITY * f));
extern ENTITY *PROTO (bi_readnum, (int n, ENTITY * f, ENTITY * v));
extern ENTITY *PROTO (bi_split, (int n, ENTITY * str, ENTITY * sep));
extern ENTITY *PROTO (bi_atof, (ENTITY * p));
extern ENTITY *PROTO (atof_scalar, (SCALAR *s));
extern ENTITY *PROTO (atof_vector, (VECTOR *v));
extern ENTITY *PROTO (atof_matrix, (MATRIX *m));
extern ENTITY *PROTO (bi_substr, (int n, ENTITY *s, ENTITY *i, ENTITY *j));
extern ENTITY *PROTO (bi_dice, (ENTITY * s));
extern ENTITY *PROTO (bi_tolower, (ENTITY *e));
extern ENTITY *PROTO (bi_toupper, (ENTITY *e));
extern ENTITY *PROTO (bi_char, (ENTITY *e));
extern ENTITY *PROTO (read_vector, (FILE * stream, VECTOR * v));
extern int PROTO (read_numbers, (FILE * fp, int cnt, double *dvec));

#endif /* READ_H */
