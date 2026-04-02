/*
   multiply.h -- Prototypes for `multiply.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: multiply.h,v 1.1.1.1 1996/04/17 05:56:20 ksh Exp $ */

#ifndef MULTIPLY_H
#define MULTIPLY_H	1

#include "entity.h"

extern ENTITY *PROTO (multiply_scalar_integer, (SCALAR * l, SCALAR * r));
extern ENTITY *PROTO (multiply_scalar_real, (SCALAR * l, SCALAR * r));
extern ENTITY *PROTO (multiply_scalar_complex, (SCALAR * l, SCALAR * r));
extern ENTITY *PROTO (multiply_scalar_matrix_integer, (SCALAR * l, MATRIX * r));
extern ENTITY *PROTO (multiply_scalar_matrix_real, (SCALAR * l, MATRIX * r));
extern ENTITY *PROTO (multiply_scalar_matrix_complex, (SCALAR * l, MATRIX * r));
extern ENTITY *PROTO (multiply_scalar_vector_integer, (SCALAR * l, VECTOR * r));
extern ENTITY *PROTO (multiply_scalar_vector_real, (SCALAR * l, VECTOR * r));
extern ENTITY *PROTO (multiply_scalar_vector_complex, (SCALAR * l, VECTOR * r));
extern ENTITY *PROTO (multiply_matrix_integer, (MATRIX * l, MATRIX * r));
extern ENTITY *PROTO (multiply_matrix_real, (MATRIX * l, MATRIX * r));
extern ENTITY *PROTO (multiply_matrix_complex, (MATRIX * l, MATRIX * r));
extern ENTITY *PROTO (multiply_vector_integer, (VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (multiply_vector_real, (VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (multiply_vector_complex, (VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (multiply_vector_matrix_integer, (VECTOR * l, MATRIX * r));
extern ENTITY *PROTO (multiply_vector_matrix_real, (VECTOR * l, MATRIX * r));
extern ENTITY *PROTO (multiply_vector_matrix_complex, (VECTOR * l, MATRIX * r));
extern ENTITY *PROTO (multiply_matrix_vector_integer, (MATRIX * l, VECTOR * r));
extern ENTITY *PROTO (multiply_matrix_vector_real, (MATRIX * l, VECTOR * r));
extern ENTITY *PROTO (multiply_matrix_vector_complex, (MATRIX * l, VECTOR * r));

#endif /* MULTIPLY_H */
