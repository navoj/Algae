/*
   binop.h -- Macros for binary operations.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: binop.h,v 1.1.1.1 1996/04/17 05:56:15 ksh Exp $ */

#ifndef  BINOP_H
#define  BINOP_H	1

#include "entity.h"

enum
  {
    BO_ADD,
    BO_SUB,
    BO_MUL,
    BO_PROD,
    BO_DIV,
    BO_APPEND,
    BO_PILE,
    BO_MOD,
    BO_AND,
    BO_OR,
    BO_EQ,
    BO_NE,
    BO_LT,
    BO_LTE,
    BO_GT,
    BO_GTE,
    BO_POWER
  };

extern ENTITY *PROTO (binop_entity, (int op, ENTITY * l, ENTITY * r));
extern ENTITY *PROTO (binop_scalar_scalar, (int op, SCALAR * l, SCALAR * r));
extern ENTITY *PROTO (binop_scalar_vector, (int op, SCALAR * l, VECTOR * r));
extern ENTITY *PROTO (binop_scalar_matrix, (int op, SCALAR * l, MATRIX * r));
extern ENTITY *PROTO (binop_vector_scalar, (int op, VECTOR * l, SCALAR * r));
extern ENTITY *PROTO (binop_vector_vector, (int op, VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (binop_vector_matrix, (int op, VECTOR * l, MATRIX * r));
extern ENTITY *PROTO (binop_matrix_scalar, (int op, MATRIX * l, SCALAR * r));
extern ENTITY *PROTO (binop_matrix_vector, (int op, MATRIX * l, VECTOR * r));
extern ENTITY *PROTO (binop_matrix_matrix, (int op, MATRIX * l, MATRIX * r));
extern ENTITY *PROTO (binop_table_table, (int op, TABLE * l, TABLE * r));
extern ENTITY *PROTO (binop_function_function,
		        (int op, FUNCTION * l, FUNCTION * r));

extern ENTITY *PROTO (binop_scalar_integer,
		        (int op, SCALAR * l, SCALAR * r));
extern ENTITY *PROTO (binop_scalar_real,
		        (int op, SCALAR * l, SCALAR * r));
extern ENTITY *PROTO (binop_scalar_complex,
		        (int op, SCALAR * l, SCALAR * r));
extern ENTITY *PROTO (binop_scalar_character,
		        (int op, SCALAR * l, SCALAR * r));

extern ENTITY *PROTO (binop_scalar_integer_character,
		        (int op, SCALAR * l, SCALAR * r));
extern ENTITY *PROTO (binop_scalar_real_character,
		        (int op, SCALAR * l, SCALAR * r));
extern ENTITY *PROTO (binop_scalar_complex_character,
		        (int op, SCALAR * l, SCALAR * r));

extern ENTITY *PROTO (binop_scalar_vector_integer,
		        (int op, SCALAR * l, VECTOR * r));
extern ENTITY *PROTO (binop_scalar_vector_real,
		        (int op, SCALAR * l, VECTOR * r));
extern ENTITY *PROTO (binop_scalar_vector_complex,
		        (int op, SCALAR * l, VECTOR * r));
extern ENTITY *PROTO (binop_scalar_vector_character,
		        (int op, SCALAR * l, VECTOR * r));
extern ENTITY *PROTO (binop_scalar_vector_num_char,
		        (int op, SCALAR * l, VECTOR * r));

extern ENTITY *PROTO (binop_scalar_matrix_integer,
		        (int op, SCALAR * l, MATRIX * r));
extern ENTITY *PROTO (binop_scalar_matrix_real,
		        (int op, SCALAR * l, MATRIX * r));
extern ENTITY *PROTO (binop_scalar_matrix_complex,
		        (int op, SCALAR * l, MATRIX * r));
extern ENTITY *PROTO (binop_scalar_matrix_character,
		        (int op, SCALAR * l, MATRIX * r));
extern ENTITY *PROTO (binop_scalar_matrix_num_char,
		        (int op, SCALAR * l, MATRIX * r));

extern ENTITY *PROTO (binop_vector_integer,
		        (int op, VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (binop_vector_real,
		        (int op, VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (binop_vector_complex,
		        (int op, VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (binop_vector_character,
		        (int op, VECTOR * l, VECTOR * r));

extern ENTITY *PROTO (binop_vector_integer_character,
		        (int op, VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (binop_vector_real_character,
		        (int op, VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (binop_vector_complex_character,
		        (int op, VECTOR * l, VECTOR * r));

extern ENTITY *PROTO (binop_vector_scalar_integer,
		        (int op, VECTOR * l, SCALAR * r));
extern ENTITY *PROTO (binop_vector_scalar_real,
		        (int op, VECTOR * l, SCALAR * r));
extern ENTITY *PROTO (binop_vector_scalar_complex,
		        (int op, VECTOR * l, SCALAR * r));
extern ENTITY *PROTO (binop_vector_scalar_character,
		        (int op, VECTOR * l, SCALAR * r));
extern ENTITY *PROTO (binop_vector_scalar_num_char,
		        (int op, VECTOR * l, SCALAR * r));

extern ENTITY *PROTO (binop_vector_matrix_integer,
		        (int op, VECTOR * l, MATRIX * r));
extern ENTITY *PROTO (binop_vector_matrix_real,
		        (int op, VECTOR * l, MATRIX * r));
extern ENTITY *PROTO (binop_vector_matrix_complex,
		        (int op, VECTOR * l, MATRIX * r));
extern ENTITY *PROTO (binop_vector_matrix_character,
		        (int op, VECTOR * l, MATRIX * r));
extern ENTITY *PROTO (binop_vector_matrix_num_char,
		        (int op, VECTOR * l, MATRIX * r));
extern ENTITY *PROTO (binop_vector_matrix_char_num,
		        (int op, VECTOR * l, MATRIX * r));

extern ENTITY *PROTO (binop_matrix_integer,
		        (int op, MATRIX * l, MATRIX * r));
extern ENTITY *PROTO (binop_matrix_real,
		        (int op, MATRIX * l, MATRIX * r));
extern ENTITY *PROTO (binop_matrix_complex,
		        (int op, MATRIX * l, MATRIX * r));
extern ENTITY *PROTO (binop_matrix_character,
		        (int op, MATRIX * l, MATRIX * r));
extern ENTITY *PROTO (binop_matrix_num_char,
		        (int op, MATRIX * l, MATRIX * r));

extern ENTITY *PROTO (binop_matrix_scalar_integer,
		        (int op, MATRIX * l, SCALAR * r));
extern ENTITY *PROTO (binop_matrix_scalar_real,
		        (int op, MATRIX * l, SCALAR * r));
extern ENTITY *PROTO (binop_matrix_scalar_complex,
		        (int op, MATRIX * l, SCALAR * r));
extern ENTITY *PROTO (binop_matrix_scalar_character,
		        (int op, MATRIX * l, SCALAR * r));
extern ENTITY *PROTO (binop_matrix_scalar_num_char,
		        (int op, MATRIX * l, SCALAR * r));

extern ENTITY *PROTO (binop_matrix_vector_integer,
		        (int op, MATRIX * l, VECTOR * r));
extern ENTITY *PROTO (binop_matrix_vector_real,
		        (int op, MATRIX * l, VECTOR * r));
extern ENTITY *PROTO (binop_matrix_vector_complex,
		        (int op, MATRIX * l, VECTOR * r));
extern ENTITY *PROTO (binop_matrix_vector_character,
		        (int op, MATRIX * l, VECTOR * r));
extern ENTITY *PROTO (binop_matrix_vector_num_char,
		        (int op, MATRIX * l, VECTOR * r));
extern ENTITY *PROTO (binop_matrix_vector_char_num,
		        (int op, MATRIX * l, VECTOR * r));

#endif /* BINOP_H */
