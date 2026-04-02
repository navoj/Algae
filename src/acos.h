/*
   acos.h -- Arc cosines.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: acos.h,v 1.1.1.1 1996/04/17 05:56:14 ksh Exp $ */

#ifndef ACOS_H
#define ACOS_H	1

#include "entity.h"

extern COMPLEX PROTO (acos_complex, (COMPLEX z));
extern ENTITY *PROTO (bi_acos, (ENTITY * p));
extern ENTITY *PROTO (acos_scalar, (SCALAR * p));
extern ENTITY *PROTO (acos_vector, (VECTOR * p));
extern ENTITY *PROTO (acos_matrix, (MATRIX * p));

#endif /* ACOS_H */
