/*
   print.h -- Prototypes for `print.c'.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: print.h,v 1.2 2003/08/07 04:48:19 ksh Exp $ */

#ifndef PRINT_H
#define PRINT_H	1

#include "entity.h"

extern DATUM *term_width;
extern DATUM *prompt_string;
extern DATUM *num_digits;

extern ENTITY *PROTO (bi_print, (int n, ENTITY * p, ENTITY * fname));
extern void PROTO (print_entity, (ENTITY * p, FILE * stream));
extern void PROTO (print_scalar, (SCALAR * p, FILE * stream));
extern void PROTO (print_matrix, (MATRIX * p, FILE * stream));
extern void PROTO (print_matrix_character_dense, (MATRIX * p, FILE * stream));
extern void PROTO (print_matrix_character_sparse, (MATRIX * p, FILE * stream));
extern void PROTO (print_matrix_integer_dense, (MATRIX * p, FILE * stream));
extern void PROTO (print_matrix_integer_sparse, (MATRIX * p, FILE * stream));
extern void PROTO (print_matrix_real_dense, (MATRIX * p, FILE * stream));
extern void PROTO (print_matrix_real_sparse, (MATRIX * p, FILE * stream));
extern void PROTO (print_matrix_complex_dense, (MATRIX * p, FILE * stream));
extern void PROTO (print_matrix_complex_sparse, (MATRIX * p, FILE * stream));
extern void PROTO (print_vector, (VECTOR * p, FILE * stream));
extern void PROTO (print_vector_dense, (VECTOR * p, FILE * stream));
extern void PROTO (print_vector_sparse, (VECTOR * p, FILE * stream));
extern void PROTO (print_function, (FUNCTION * p, FILE * stream));
extern char *PROTO (get_prompt, (int i));

#endif /* PRINT_H */
