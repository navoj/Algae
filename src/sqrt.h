/*
   sqrt.h -- Prototypes for `sqrt.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: sqrt.h,v 1.1.1.1 1996/04/17 05:56:22 ksh Exp $ */

#ifndef SQRT_H
#define SQRT_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_sqrt, (ENTITY * p));
extern ENTITY *PROTO (sqrt_scalar, (SCALAR * p));
extern ENTITY *PROTO (sqrt_vector, (VECTOR * p));
extern ENTITY *PROTO (sqrt_matrix, (MATRIX * p));
extern COMPLEX PROTO (sqrt_complex, (COMPLEX z));

#endif /* SQRT_H */
