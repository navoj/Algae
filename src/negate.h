/*
   negate.h -- Prototypes for `negate.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: negate.h,v 1.1.1.1 1996/04/17 05:56:20 ksh Exp $ */

#ifndef NEGATE_H
#define NEGATE_H	1

#include "entity.h"

extern ENTITY *PROTO (negate_entity, (ENTITY * p));
extern ENTITY *PROTO (negate_scalar, (SCALAR * p));
extern ENTITY *PROTO (negate_vector, (VECTOR * p));
extern ENTITY *PROTO (negate_matrix, (MATRIX * p));

#endif /* NEGATE_H */
