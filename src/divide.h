/*
   divide.h -- Prototypes for `divide.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: divide.h,v 1.1.1.1 1996/04/17 05:56:16 ksh Exp $ */

#ifndef DIVIDE_H
#define DIVIDE_H	1

#include "entity.h"

/*
 * Can you believe it?  Some machines have div_t but not div, or div and
 * not div_t.  Define div_t as Div_t if necessary.
 */

typedef struct
  {
    int quot;
    int rem;
  }
Div_t;

#if !HAVE_DIV
extern div_t PROTO (div, (int num, int denom));
#endif /* !HAVE_DIV */

extern COMPLEX PROTO (divide_complex, (COMPLEX a, COMPLEX b));

extern ENTITY *PROTO (divide_scalar_vector_integer, (SCALAR * l, VECTOR * r));
extern ENTITY *PROTO (divide_scalar_vector_real, (SCALAR * l, VECTOR * r));
extern ENTITY *PROTO (divide_scalar_vector_complex, (SCALAR * l, VECTOR * r));

extern ENTITY *PROTO (divide_scalar_matrix_integer, (SCALAR * l, MATRIX * r));
extern ENTITY *PROTO (divide_scalar_matrix_real, (SCALAR * l, MATRIX * r));
extern ENTITY *PROTO (divide_scalar_matrix_complex, (SCALAR * l, MATRIX * r));

extern ENTITY *PROTO (divide_vector_integer, (VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (divide_vector_real, (VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (divide_vector_complex, (VECTOR * l, VECTOR * r));

extern ENTITY *PROTO (divide_vector_scalar_integer, (VECTOR * l, SCALAR * r));
extern ENTITY *PROTO (divide_vector_scalar_real, (VECTOR * l, SCALAR * r));
extern ENTITY *PROTO (divide_vector_scalar_complex, (VECTOR * l, SCALAR * r));

extern ENTITY *PROTO (divide_vector_matrix_integer, (VECTOR * l, MATRIX * r));
extern ENTITY *PROTO (divide_vector_matrix_real, (VECTOR * l, MATRIX * r));
extern ENTITY *PROTO (divide_vector_matrix_complex, (VECTOR * l, MATRIX * r));

extern ENTITY *PROTO (divide_matrix_integer, (MATRIX * l, MATRIX * r));
extern ENTITY *PROTO (divide_matrix_real, (MATRIX * l, MATRIX * r));
extern ENTITY *PROTO (divide_matrix_complex, (MATRIX * l, MATRIX * r));

extern ENTITY *PROTO (divide_matrix_scalar_integer, (MATRIX * l, SCALAR * r));
extern ENTITY *PROTO (divide_matrix_scalar_real, (MATRIX * l, SCALAR * r));
extern ENTITY *PROTO (divide_matrix_scalar_complex, (MATRIX * l, SCALAR * r));

extern ENTITY *PROTO (divide_matrix_vector_integer, (MATRIX * l, VECTOR * r));
extern ENTITY *PROTO (divide_matrix_vector_real, (MATRIX * l, VECTOR * r));
extern ENTITY *PROTO (divide_matrix_vector_complex, (MATRIX * l, VECTOR * r));

#endif /* DIVIDE_H */
