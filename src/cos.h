/*
   cos.h -- Prototypes for `cos.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: cos.h,v 1.1.1.1 1996/04/17 05:56:16 ksh Exp $ */

#ifndef COS_H
#define COS_H	1

#include "entity.h"

extern COMPLEX PROTO (cos_complex, (COMPLEX z));
extern ENTITY *PROTO (bi_cos, (ENTITY * p));
extern ENTITY *PROTO (cos_scalar, (SCALAR * p));
extern ENTITY *PROTO (cos_vector, (VECTOR * p));
extern ENTITY *PROTO (cos_matrix, (MATRIX * p));

#endif /* COS_H */
