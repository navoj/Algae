/*
   sort.h -- Prototypes for `sort.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: sort.h,v 1.1.1.1 1996/04/17 05:56:22 ksh Exp $ */

#ifndef SORT_H
#define SORT_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_sort, (ENTITY * p));
extern ENTITY *PROTO (bi_isort, (ENTITY * p));
extern ENTITY *PROTO (sort_vector, (VECTOR * p, int ind));
extern void PROTO (isort_vector_integer, (int right, int b[], int x[]));
extern void PROTO (isort_vector_real, (int right, REAL b[], int x[]));
extern void PROTO (isort_vector_complex, (int right, COMPLEX b[], int x[]));
extern void PROTO (isort_vector_character, (int right, char *b[], int x[]));
extern int PROTO (compare_integer, (const void *a, const void *b));

#endif /* SORT_H */
