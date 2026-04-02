/*
   sets.h -- Prototypes for `sets.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: sets.h,v 1.1.1.1 1996/04/17 05:56:22 ksh Exp $ */

#ifndef SETS_H
#define SETS_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_set, (int n, ENTITY * p));
extern ENTITY *PROTO (set_vector, (VECTOR * p));
extern ENTITY *PROTO (bi_union, (ENTITY * l, ENTITY * r));
extern ENTITY *PROTO (union_vector, (VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (bi_intersection, (ENTITY * l, ENTITY * r));
extern ENTITY *PROTO (intersection_vector, (VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (bi_complement, (ENTITY * l, ENTITY * r));
extern ENTITY *PROTO (complement_vector, (VECTOR * l, VECTOR * r));

#endif /* SETS_H */
