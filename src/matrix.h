/*
   matrix.h -- Prototypes for functions in `matrix.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: matrix.h,v 1.1.1.1 1996/04/17 05:56:19 ksh Exp $ */

#ifndef MATRIX_H
#define MATRIX_H	1

#include "entity.h"
#include "matrix_st.h"
#include "put.h"

#define delete_2_matrices( a, b )	do { delete_matrix( a ); \
					     delete_matrix( b ); } while(0)
#define delete_3_matrices( a, b, c )	do { delete_matrix( a ); \
					     delete_matrix( b ); \
					     delete_matrix( c ); } while(0)

#if DEBUG
#define delete_matrix( p )	DB_delete_matrix( p, __FILE__, __LINE__ )
#else
#define delete_matrix( p )	if ( (p) && --ENT(p)->ref_count == 0 ) \
                                  free_matrix( p ); else
#endif /* DEBUG */

#define copy_matrix( p )	copy_entity( ENT(p) )

extern MATRIX_MEMBER PROTO (matrix_member_search, (char *s));
extern ENTITY *PROTO (bi_matrix, (int n, ENTITY * p));
extern ENTITY *PROTO (matrix_entity, (ENTITY * ip));
extern ENTITY *PROTO (scalar_to_matrix, (SCALAR * ip));
extern ENTITY *PROTO (make_matrix, (int nr, int nc, TYPE type, DENSITY density));
extern ENTITY *PROTO (form_matrix, (int nr, int nc, TYPE type, DENSITY density));
extern ENTITY *PROTO (matrix_to_scalar, (MATRIX * ip));
extern ENTITY *PROTO (dup_matrix, (MATRIX * ip));
extern ENTITY *PROTO (matrix_to_vector, (MATRIX * ip));
extern ENTITY *PROTO (vector_to_matrix, (VECTOR * ip));
extern void PROTO (free_matrix, (MATRIX * p));
extern void PROTO (DB_delete_matrix, (MATRIX * p, char *file, int line));
extern int PROTO (put_matrix, (MATRIX * s, FILE * stream, struct ent_node *));
extern ENTITY *PROTO (get_matrix, (FILE * stream, int ver));
extern ENTITY *PROTO (gift_wrap_matrix, (int nr, int nc, TYPE type, void *data));

#endif /* MATRIX_H */
