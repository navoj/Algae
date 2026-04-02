/*
   vector.h -- Prototypes for functions in `vector.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: vector.h,v 1.1.1.1 1996/04/17 05:56:23 ksh Exp $ */

#ifndef VECTOR_H
#define VECTOR_H	1

#include "entity.h"
#include "vector_st.h"
#include "put.h"

#define MATCH_VECTORS( l, r )	( (l) == (r) || \
				  (l) == NULL || \
				  (r) == NULL || \
				  match_vectors( (VECTOR *) l, (VECTOR *) r ) )

#define delete_2_vectors( a, b )	do { delete_vector( a ); \
					     delete_vector( b ); } while(0)
#define delete_3_vectors( a, b, c )	do { delete_vector( a ); \
					     delete_vector( b ); \
					     delete_vector( c ); } while(0)

#if DEBUG
#define delete_vector( p )	DB_delete_vector( p, __FILE__, __LINE__ )
#else
#define delete_vector( p )	if ( (p) && --ENT(p)->ref_count == 0 ) \
                                  free_vector( p ); else
#endif /* DEBUG */

#define copy_vector( p )	copy_entity( ENT(p) )

extern VECTOR_MEMBER PROTO (vector_member_search, (char *s));
extern ENTITY *PROTO (bi_vector, (int n, ENTITY * p));
extern ENTITY *PROTO (vector_entity, (ENTITY * ip));
extern ENTITY *PROTO (scalar_to_vector, (SCALAR * ips));
extern ENTITY *PROTO (make_vector, (int ne, TYPE type, DENSITY density));
extern ENTITY *PROTO (form_vector, (int ne, TYPE type, DENSITY density));
extern ENTITY *PROTO (vector_to_scalar, (VECTOR * ipv));
extern ENTITY *PROTO (dup_vector, (VECTOR * ipv));
extern void PROTO (free_vector, (VECTOR * p));
extern void PROTO (DB_delete_vector, (VECTOR * p, char *file, int line));
extern int PROTO (put_vector, (VECTOR * s, FILE * stream, struct ent_node *));
extern ENTITY *PROTO (get_vector, (FILE * stream, int ver));
extern int PROTO (match_vectors, (VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (gift_wrap_vector, (int ne, TYPE type, void *data));

#endif /* VECTOR_H */
