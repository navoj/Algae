/*
   mod.h -- Prototypes for `mod.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: mod.h,v 1.1.1.1 1996/04/17 05:56:20 ksh Exp $ */

#ifndef MOD_H
#define MOD_H  1

#include "entity.h"

#define OWN( p )	( ENT(p)->ref_count == 1 )

extern ENTITY *PROTO (mod_scalar_vector_integer, (SCALAR * l, VECTOR * r));
extern ENTITY *PROTO (mod_scalar_vector_real, (SCALAR * l, VECTOR * r));

extern ENTITY *PROTO (mod_scalar_matrix_integer, (SCALAR * l, MATRIX * r));
extern ENTITY *PROTO (mod_scalar_matrix_real, (SCALAR * l, MATRIX * r));

extern ENTITY *PROTO (mod_matrix_integer, (MATRIX * l, MATRIX * r));
extern ENTITY *PROTO (mod_matrix_real, (MATRIX * l, MATRIX * r));

extern ENTITY *PROTO (mod_vector_integer, (VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (mod_vector_real, (VECTOR * l, VECTOR * r));

extern ENTITY *PROTO (mod_vector_scalar_integer, (VECTOR * l, SCALAR * r));
extern ENTITY *PROTO (mod_vector_scalar_real, (VECTOR * l, SCALAR * r));

extern ENTITY *PROTO (mod_vector_matrix_integer, (VECTOR * l, MATRIX * r));
extern ENTITY *PROTO (mod_vector_matrix_real, (VECTOR * l, MATRIX * r));

extern ENTITY *PROTO (mod_matrix_scalar_integer, (MATRIX * l, SCALAR * r));
extern ENTITY *PROTO (mod_matrix_scalar_real, (MATRIX * l, SCALAR * r));

extern ENTITY *PROTO (mod_matrix_vector_integer, (MATRIX * l, VECTOR * r));
extern ENTITY *PROTO (mod_matrix_vector_real, (MATRIX * l, VECTOR * r));

#endif /* MOD_H */
