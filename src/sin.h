/*
   sin.h -- Prototypes for `sin.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: sin.h,v 1.1.1.1 1996/04/17 05:56:22 ksh Exp $ */

#ifndef SIN_H
#define SIN_H	1

#include "entity.h"

extern COMPLEX PROTO (sin_complex, (COMPLEX z));
extern ENTITY *PROTO (bi_sin, (ENTITY * p));
extern ENTITY *PROTO (sin_scalar, (SCALAR * p));
extern ENTITY *PROTO (sin_vector, (VECTOR * p));
extern ENTITY *PROTO (sin_matrix, (MATRIX * p));

#endif /* SIN_H */
