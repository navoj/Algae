/*
   conjugate.h -- Prototypes for `conjugate.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: conjugate.h,v 1.1.1.1 1996/04/17 05:56:16 ksh Exp $ */

#ifndef CONJUGATE_H
#define CONJUGATE_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_conjugate, (ENTITY * p));
extern ENTITY *PROTO (conjugate_scalar, (SCALAR * p));
extern ENTITY *PROTO (conjugate_vector, (VECTOR * p));
extern ENTITY *PROTO (conjugate_matrix, (MATRIX * p));

#endif /* CONJUGATE_H */
