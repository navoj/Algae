/*
   fill.h

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: fill.h,v 1.1.1.1 1996/04/17 05:56:17 ksh Exp $ */

#ifndef FILL_H
#define FILL_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_fill, (ENTITY * shape, ENTITY * v));
extern ENTITY *PROTO (fill_scalar, (VECTOR * shape, SCALAR * v));
extern ENTITY *PROTO (fill_vector, (VECTOR * shape, VECTOR * v));
extern ENTITY *PROTO (fill_matrix, (VECTOR * shape, MATRIX * v));

#endif /* FILL_H */
