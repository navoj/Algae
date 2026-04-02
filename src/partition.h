/*
   partition.h -- Prototypes for `partition.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: partition.h,v 1.1.1.1 1996/04/17 05:56:20 ksh Exp $ */

#ifndef PARTITION_H
#define PARTITION_H	1

#include "entity.h"

extern ENTITY *PROTO (partition_1d,
		        (ENTITY * v, ENTITY * index));
extern ENTITY *PROTO (partition_vector_label_scalar,
		        (VECTOR * v, SCALAR * index));
extern ENTITY *PROTO (partition_vector_label_vector,
		        (VECTOR * v, VECTOR * index));
extern ENTITY *PROTO (partition_vector_vector,
		        (VECTOR * v, VECTOR * index));
extern ENTITY *PROTO (partition_vector_scalar,
		        (VECTOR * v, SCALAR * index));
extern ENTITY *PROTO (partition_2d,
		        (ENTITY * m, ENTITY * rindex, ENTITY * cindex));

extern ENTITY *PROTO (partition_matrix_vector_vector,
		        (MATRIX * oldm, VECTOR * rindex, VECTOR * cindex));
extern ENTITY *PROTO (partition_matrix_scalar_vector,
		        (MATRIX * m, SCALAR * rindex, VECTOR * cindex));
extern ENTITY *PROTO (partition_matrix_vector_scalar,
		        (MATRIX * m, VECTOR * rindex, SCALAR * cindex));
extern ENTITY *PROTO (partition_matrix_scalar_scalar,
		        (MATRIX * m, SCALAR * rindex, SCALAR * cindex));
extern ENTITY *PROTO (row_partition_matrix_sparse,
		        (MATRIX * oldm, VECTOR * rindex));

extern int PROTO (get_matrix_element_integer,
		    (MATRIX * m, int r, int c));
extern REAL PROTO (get_matrix_element_real,
		     (MATRIX * m, int r, int c));
extern COMPLEX PROTO (get_matrix_element_complex,
		        (MATRIX * m, int r, int c));
extern char *PROTO (get_matrix_element_character,
		      (MATRIX * m, int r, int c));

extern int PROTO (get_vector_element_integer,
		    (VECTOR * v, int elem));
extern REAL PROTO (get_vector_element_real,
		     (VECTOR * v, int elem));
extern COMPLEX PROTO (get_vector_element_complex,
		        (VECTOR * v, int elem));
extern char *PROTO (get_vector_element_character,
		      (VECTOR * v, int elem));

#endif /* PARTITION_H */
