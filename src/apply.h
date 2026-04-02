/*
   apply.h -- Prototypes for `apply.c'.

   Copyright (C) 1994-96  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: apply.h,v 1.2 1996/10/23 19:08:41 ksh Exp $ */

#ifndef APPLY_H
#define APPLY_H		1

#include "entity.h"

extern ENTITY *PROTO (apply_vector_integer_integer,
		        (int (*func) (int), VECTOR * p));
extern ENTITY *PROTO (apply_vector_integer_real,
		        (int (*func) (REAL), VECTOR * p));
extern ENTITY *PROTO (apply_vector_integer_complex,
		        (int (*func) (COMPLEX), VECTOR * p));
extern ENTITY *PROTO (apply_vector_integer_character,
		        (int (*func) (char *), VECTOR * p));
extern ENTITY *PROTO (apply_vector_real_real,
		        (REAL (*func) (REAL), VECTOR * p));
extern ENTITY *PROTO (apply_vector_real_complex,
		        (REAL (*func) (COMPLEX), VECTOR * p));
extern ENTITY *PROTO (apply_vector_complex_complex,
		        (COMPLEX (*func) (COMPLEX), VECTOR * p));
extern ENTITY *PROTO (apply_vector_character_character,
		        (char * (*func) (char *), VECTOR * p));

extern ENTITY *PROTO (apply_matrix_integer_integer,
		        (int (*func) (int), MATRIX * p));
extern ENTITY *PROTO (apply_matrix_integer_real,
		        (int (*func) (REAL), MATRIX * p));
extern ENTITY *PROTO (apply_matrix_integer_complex,
		        (int (*func) (COMPLEX), MATRIX * p));
extern ENTITY *PROTO (apply_matrix_integer_character,
		        (int (*func) (char *), MATRIX * p));
extern ENTITY *PROTO (apply_matrix_real_real,
		        (REAL (*func) (REAL), MATRIX * p));
extern ENTITY *PROTO (apply_matrix_real_complex,
		        (REAL (*func) (COMPLEX), MATRIX * p));
extern ENTITY *PROTO (apply_matrix_complex_complex,
		        (COMPLEX (*func) (COMPLEX), MATRIX * p));
extern ENTITY *PROTO (apply_matrix_character_character,
		        (char * (*func) (char *), MATRIX * p));

#endif /* APPLY_H */
