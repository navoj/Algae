/*
   atan.h -- Prototypes for `atan.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: atan.h,v 1.1.1.1 1996/04/17 05:56:15 ksh Exp $ */

#ifndef ATAN_H
#define ATAN_H	1

#include "entity.h"

extern COMPLEX PROTO (atan_complex, (COMPLEX z));
extern ENTITY *PROTO (bi_atan, (ENTITY * p));
extern ENTITY *PROTO (atan_scalar, (SCALAR * p));
extern ENTITY *PROTO (atan_vector, (VECTOR * p));
extern ENTITY *PROTO (atan_matrix, (MATRIX * p));

extern ENTITY *PROTO (bi_atan2, (ENTITY * y, ENTITY * x));

extern ENTITY *PROTO (atan2_scalar_scalar, (SCALAR * y, SCALAR * x));
extern ENTITY *PROTO (atan2_scalar_vector, (SCALAR * y, VECTOR * x));
extern ENTITY *PROTO (atan2_scalar_matrix, (SCALAR * y, MATRIX * x));

extern ENTITY *PROTO (atan2_vector_scalar, (VECTOR * y, SCALAR * x));
extern ENTITY *PROTO (atan2_vector_vector, (VECTOR * y, VECTOR * x));
extern ENTITY *PROTO (atan2_vector_matrix, (VECTOR * y, MATRIX * x));

extern ENTITY *PROTO (atan2_matrix_scalar, (MATRIX * y, SCALAR * x));
extern ENTITY *PROTO (atan2_matrix_vector, (MATRIX * y, VECTOR * x));
extern ENTITY *PROTO (atan2_matrix_matrix, (MATRIX * y, MATRIX * x));

#endif /* ATAN_H */
