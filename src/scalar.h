/*
   scalar.h -- Prototypes for functions in `scalar.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: scalar.h,v 1.1.1.1 1996/04/17 05:56:21 ksh Exp $ */

#ifndef SCALAR_H
#define SCALAR_H	1

#include "entity.h"
#include "scalar_st.h"
#include "put.h"

#define delete_2_scalars( a, b )	do { delete_scalar( a ); \
					     delete_scalar( b ); } while(0)
#define delete_3_scalars( a, b, c )	do { delete_scalar( a ); \
					     delete_scalar( b ); \
					     delete_scalar( c ); } while(0)

#if DEBUG
#define delete_scalar( p )	DB_delete_scalar( p, __FILE__, __LINE__ )
#else
#define delete_scalar( p )	if ( (p) && --ENT(p)->ref_count == 0 ) \
                                  free_scalar( p ); else
#endif /* DEBUG */

#define copy_scalar( p )	copy_entity( ENT(p) )

extern SCALAR_MEMBER PROTO (scalar_member_search, (char *s));
extern ENTITY *PROTO (bi_scalar, (int n, ENTITY * ip));
extern ENTITY *PROTO (scalar_entity, (ENTITY * ip));
extern ENTITY *PROTO (make_scalar, (TYPE type));
extern ENTITY *PROTO (to_scalar, (TYPE type,...));
extern ENTITY *PROTO (int_to_scalar, (int i));
extern ENTITY *PROTO (real_to_scalar, (REAL r));
extern ENTITY *PROTO (complex_to_scalar, (COMPLEX c));
extern ENTITY *PROTO (char_to_scalar, (char *p));
extern void PROTO (free_scalar, (SCALAR * p));
extern ENTITY *PROTO (dup_scalar, (SCALAR * ips));
extern void PROTO (DB_delete_scalar, (SCALAR * p, char *file, int line));
extern int PROTO (put_scalar, (SCALAR * s, FILE * stream, struct ent_node *));
extern ENTITY *PROTO (get_scalar, (FILE * stream, int ver));

#endif /* SCALAR_H */
