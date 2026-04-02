/*
   pile.h -- Prototypes for `pile.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: pile.h,v 1.1.1.1 1996/04/17 05:56:20 ksh Exp $ */

#ifndef PILE_H
#define PILE_H	1

#include "entity.h"

extern ENTITY *PROTO (pile_scalar_vector_integer, (SCALAR * l, VECTOR * r));
extern ENTITY *PROTO (pile_scalar_vector_real, (SCALAR * l, VECTOR * r));
extern ENTITY *PROTO (pile_scalar_vector_complex, (SCALAR * l, VECTOR * r));
extern ENTITY *PROTO (pile_scalar_vector_character, (SCALAR * l, VECTOR * r));

extern ENTITY *PROTO (pile_scalar_matrix_integer, (SCALAR * l, MATRIX * r));
extern ENTITY *PROTO (pile_scalar_matrix_real, (SCALAR * l, MATRIX * r));
extern ENTITY *PROTO (pile_scalar_matrix_complex, (SCALAR * l, MATRIX * r));
extern ENTITY *PROTO (pile_scalar_matrix_character, (SCALAR * l, MATRIX * r));

extern ENTITY *PROTO (pile_vector_integer, (VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (pile_vector_real, (VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (pile_vector_complex, (VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (pile_vector_character, (VECTOR * l, VECTOR * r));

extern ENTITY *PROTO (pile_vector_scalar_integer, (VECTOR * l, SCALAR * r));
extern ENTITY *PROTO (pile_vector_scalar_real, (VECTOR * l, SCALAR * r));
extern ENTITY *PROTO (pile_vector_scalar_complex, (VECTOR * l, SCALAR * r));
extern ENTITY *PROTO (pile_vector_scalar_character, (VECTOR * l, SCALAR * r));

extern ENTITY *PROTO (pile_vector_matrix_integer, (VECTOR * l, MATRIX * r));
extern ENTITY *PROTO (pile_vector_matrix_real, (VECTOR * l, MATRIX * r));
extern ENTITY *PROTO (pile_vector_matrix_complex, (VECTOR * l, MATRIX * r));
extern ENTITY *PROTO (pile_vector_matrix_character, (VECTOR * l, MATRIX * r));

extern ENTITY *PROTO (pile_matrix_integer, (MATRIX * l, MATRIX * r));
extern ENTITY *PROTO (pile_matrix_real, (MATRIX * l, MATRIX * r));
extern ENTITY *PROTO (pile_matrix_complex, (MATRIX * l, MATRIX * r));
extern ENTITY *PROTO (pile_matrix_character, (MATRIX * l, MATRIX * r));

extern ENTITY *PROTO (pile_matrix_scalar_integer, (MATRIX * l, SCALAR * r));
extern ENTITY *PROTO (pile_matrix_scalar_real, (MATRIX * l, SCALAR * r));
extern ENTITY *PROTO (pile_matrix_scalar_complex, (MATRIX * l, SCALAR * r));
extern ENTITY *PROTO (pile_matrix_scalar_character, (MATRIX * l, SCALAR * r));

extern ENTITY *PROTO (pile_matrix_vector_integer, (MATRIX * l, VECTOR * r));
extern ENTITY *PROTO (pile_matrix_vector_real, (MATRIX * l, VECTOR * r));
extern ENTITY *PROTO (pile_matrix_vector_complex, (MATRIX * l, VECTOR * r));
extern ENTITY *PROTO (pile_matrix_vector_character, (MATRIX * l, VECTOR * r));

#endif /* PILE_H */
