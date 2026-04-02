/*
   not.h -- The `not' and `notnot' functions.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: not.h,v 1.1.1.1 1996/04/17 05:56:20 ksh Exp $ */

#ifndef NOT_H
#define NOT_H	1

#include "entity.h"

extern ENTITY *PROTO (not_scalar, (SCALAR * p));
extern ENTITY *PROTO (not_vector, (VECTOR * p));
extern ENTITY *PROTO (not_matrix, (MATRIX * p));
extern ENTITY *PROTO (notnot_scalar, (SCALAR * p));
extern ENTITY *PROTO (notnot_vector, (VECTOR * p));
extern ENTITY *PROTO (notnot_matrix, (MATRIX * p));

#endif /* NOT_H */
