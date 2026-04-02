/*
   tan.h -- Prototypes for `tan.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: tan.h,v 1.1.1.1 1996/04/17 05:56:22 ksh Exp $ */

#ifndef TAN_H
#define TAN_H	1

#include "entity.h"

extern COMPLEX PROTO (tan_complex, (COMPLEX z));
extern ENTITY *PROTO (bi_tan, (ENTITY * p));
extern ENTITY *PROTO (tan_scalar, (SCALAR * p));
extern ENTITY *PROTO (tan_vector, (VECTOR * p));
extern ENTITY *PROTO (tan_matrix, (MATRIX * p));

#endif /* TAN_H */
