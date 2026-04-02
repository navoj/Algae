/*
   assign.h -- Prototypes for `assign.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: assign.h,v 1.1.1.1 1996/04/17 05:56:15 ksh Exp $ */

#ifndef ASSIGN_H
#define ASSIGN_H	1

#include "entity.h"

extern ENTITY *PROTO (assign_submatrix,
		        (ENTITY * left, ENTITY * row,
			 ENTITY * col, ENTITY * right));
extern ENTITY *PROTO (assign_submatrix_integer,
		        (MATRIX * left, int row, int col, int e));
extern ENTITY *PROTO (assign_submatrix_real,
		        (MATRIX * left, int row, int col, REAL e));
extern ENTITY *PROTO (assign_submatrix_complex,
		        (MATRIX * left, int row, int col, COMPLEX e));
extern ENTITY *PROTO (assign_submatrix_character,
		        (MATRIX * left, int row, int col, char *e));

extern ENTITY *PROTO (assign_subvector,
		        (ENTITY * left, ENTITY * elem, ENTITY * right));
extern ENTITY *PROTO (assign_subvector_integer,
		        (VECTOR * left, int elem, int e));
extern ENTITY *PROTO (assign_subvector_real,
		        (VECTOR * left, int elem, REAL e));
extern ENTITY *PROTO (assign_subvector_complex,
		        (VECTOR * left, int elem, COMPLEX e));
extern ENTITY *PROTO (assign_subvector_character,
		        (VECTOR * left, int elem, char *e));

extern ENTITY *PROTO (assign_matrix_matrix,
		        (MATRIX * left, int *row, int *col, MATRIX * right));
extern ENTITY *PROTO (assign_matrix_scalar,
		        (MATRIX * left, int *row, int *col, SCALAR * right));
extern ENTITY *PROTO (assign_vector_vector,
		        (VECTOR * left, int *elem, VECTOR * right));

#endif /* ASSIGN_H */
