/*
   cast.h -- Prototypes for `cast.h'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: cast.h,v 1.1.1.1 1996/04/17 05:56:15 ksh Exp $ */

#ifndef CAST_H
#define CAST_H	1

#include "entity.h"
#include "two_type.h"

#define AUTO_TYPE( l, r )	( ( (l) > (r) ) ? (l) : (r) )

#ifndef STUPID_CPP

#define AUTO_CAST( l, l_type, r, r_type ) \
	switch ( TWO_TYPE( l_type, r_type ) ) { \
	  case integer_integer: \
	  case real_real: \
	  case complex_complex: \
	  case character_character: \
	    break; \
	  case integer_real: \
	    (l) = (void *) cast_entity( EAT( l ), real ); \
	    break; \
	  case integer_complex: \
	  case real_complex: \
	    (l) = (void *) cast_entity( EAT( l ), complex ); \
	    break; \
	  case integer_character: \
	  case real_character: \
	  case complex_character: \
	    (l) = (void *) cast_entity( EAT( l ), character ); \
	    break; \
	  case real_integer: \
	    (r) = (void *) cast_entity( EAT( r ), real ); \
	    break; \
	  case complex_integer: \
	  case complex_real: \
	    (r) = (void *) cast_entity( EAT( r ), complex ); \
	    break; \
	  case character_integer: \
	  case character_real: \
	  case character_complex: \
	    (r) = (void *) cast_entity( EAT( r ), character ); \
	    break; \
	  default: \
	    wipeout( "Bad type." ); \
	}

#define AUTO_CAST_SCALAR( l, r ) \
    if ( ((SCALAR *)(l))->type != ((SCALAR *)(r))->type ) { \
	switch ( TWO_TYPE( ((SCALAR *)(l))->type, ((SCALAR *)(r))->type ) ) { \
	  case integer_real: \
	    (l) = (void *) cast_scalar_integer_real( (SCALAR *) EAT(l) ); \
	    break; \
	  case integer_complex: \
	    (l) = (void *) cast_scalar_integer_complex( (SCALAR *) EAT(l) ); \
	    break; \
	  case real_complex: \
	    (l) = (void *) cast_scalar_real_complex( (SCALAR *) EAT(l) ); \
	    break; \
	  case integer_character: \
	    (l) = (void *) cast_scalar_integer_character((SCALAR *) EAT(l)); \
	    break; \
	  case real_character: \
	    (l) = (void *) cast_scalar_real_character( (SCALAR *) EAT(l) ); \
	    break; \
	  case complex_character: \
	    (l) = (void *) cast_scalar_complex_character((SCALAR *) EAT(l)); \
	    break; \
	  case real_integer: \
	    (r) = (void *) cast_scalar_integer_real( (SCALAR *) EAT(r) ); \
	    break; \
	  case complex_integer: \
	    (r) = (void *) cast_scalar_integer_complex( (SCALAR *) EAT(r) ); \
	    break; \
	  case complex_real: \
	    (r) = (void *) cast_scalar_real_complex( (SCALAR *) EAT(r) ); \
	    break; \
	  case character_integer: \
	    (r) = (void *) cast_scalar_integer_character((SCALAR *) EAT(r)); \
	    break; \
	  case character_real: \
	    (r) = (void *) cast_scalar_real_character( (SCALAR *) EAT(r) ); \
	    break; \
	  case character_complex: \
	    (r) = (void *) cast_scalar_complex_character((SCALAR *) EAT(r)); \
	    break; \
	  default: \
	    wipeout( "Bad type." ); \
	} \
    } else

#define AUTO_CAST_VECTOR( l, r ) \
    if ( ((VECTOR *)(l))->type != ((VECTOR *)(r))->type ) { \
	switch ( TWO_TYPE( ((VECTOR *)(l))->type, ((VECTOR *)(r))->type ) ) { \
	  case integer_real: \
	    (l) = (void *) cast_vector_integer_real( (VECTOR *) EAT( l ) ); \
	    break; \
	  case integer_complex: \
	    (l) = (void *) cast_vector_integer_complex( (VECTOR *) EAT(l) ); \
	    break; \
	  case real_complex: \
	    (l) = (void *) cast_vector_real_complex( (VECTOR *) EAT( l ) ); \
	    break; \
	  case integer_character: \
	    (l) = (void *) cast_vector_integer_character((VECTOR *) EAT(l)); \
	    break; \
	  case real_character: \
	    (l) = (void *) cast_vector_real_character( (VECTOR *) EAT(l) ); \
	    break; \
	  case complex_character: \
	    (l) = (void *) cast_vector_complex_character((VECTOR *) EAT(l)); \
	    break; \
	  case real_integer: \
	    (r) = (void *) cast_vector_integer_real( (VECTOR *) EAT( r ) ); \
	    break; \
	  case complex_integer: \
	    (r) = (void *) cast_vector_integer_complex( (VECTOR *) EAT(r) ); \
	    break; \
	  case complex_real: \
	    (r) = (void *) cast_vector_real_complex( (VECTOR *) EAT( r ) ); \
	    break; \
	  case character_integer: \
	    (r) = (void *) cast_vector_integer_character((VECTOR *) EAT(r)); \
	    break; \
	  case character_real: \
	    (r) = (void *) cast_vector_real_character( (VECTOR *) EAT( r ) ); \
	    break; \
	  case character_complex: \
	    (r) = (void *) cast_vector_complex_character((VECTOR *) EAT(r)); \
	    break; \
	  default: \
	    wipeout( "Bad type.\n" ); \
	} \
    } else

#define AUTO_CAST_MATRIX( l, r ) \
    if ( ((MATRIX *)(l))->type != ((MATRIX *)(r))->type ) { \
	switch ( TWO_TYPE( ((MATRIX *)(l))->type, ((MATRIX *)(r))->type ) ) { \
	  case integer_real: \
	    (l) = (void *) cast_matrix_integer_real( (MATRIX *) EAT( l ) ); \
	    break; \
	  case integer_complex: \
	    (l) = (void *) cast_matrix_integer_complex( (MATRIX *) EAT(l) ); \
	    break; \
	  case real_complex: \
	    (l) = (void *) cast_matrix_real_complex( (MATRIX *) EAT( l ) ); \
	    break; \
	  case integer_character: \
	    (l) = (void *) cast_matrix_integer_character((MATRIX *) EAT(l)); \
	    break; \
	  case real_character: \
	    (l) = (void *) cast_matrix_real_character( (MATRIX *) EAT( l ) ); \
	    break; \
	  case complex_character: \
	    (l) = (void *) cast_matrix_complex_character((MATRIX *) EAT(l)); \
	    break; \
	  case real_integer: \
	    (r) = (void *) cast_matrix_integer_real( (MATRIX *) EAT( r ) ); \
	    break; \
	  case complex_integer: \
	    (r) = (void *) cast_matrix_integer_complex( (MATRIX *) EAT(r) ); \
	    break; \
	  case complex_real: \
	    (r) = (void *) cast_matrix_real_complex( (MATRIX *) EAT( r ) ); \
	    break; \
	  case character_integer: \
	    (r) = (void *) cast_matrix_integer_character((MATRIX *) EAT(r)); \
	    break; \
	  case character_real: \
	    (r) = (void *) cast_matrix_real_character( (MATRIX *) EAT( r ) ); \
	    break; \
	  case character_complex: \
	    (r) = (void *) cast_matrix_complex_character( (MATRIX *) EAT(r)); \
	    break; \
	  default: \
	    wipeout( "Bad type.\n" ); \
	} \
    } else

#define AUTO_CAST_VECTOR_SCALAR( l, r ) \
    if ( ((VECTOR *)(l))->type != ((SCALAR *)(r))->type ) { \
	switch ( TWO_TYPE( ((VECTOR *)(l))->type, ((SCALAR *)(r))->type ) ) { \
	  case integer_real: \
	    (l) = (void *) cast_vector_integer_real( (VECTOR *) EAT( l ) ); \
	    break; \
	  case integer_complex: \
	    (l) = (void *) cast_vector_integer_complex( (VECTOR *) EAT(l) ); \
	    break; \
	  case real_complex: \
	    (l) = (void *) cast_vector_real_complex( (VECTOR *) EAT( l ) ); \
	    break; \
	  case integer_character: \
	    (l) = (void *) cast_vector_integer_character((VECTOR *) EAT(l)); \
	    break; \
	  case real_character: \
	    (l) = (void *) cast_vector_real_character( (VECTOR *) EAT(l) ); \
	    break; \
	  case complex_character: \
	    (l) = (void *) cast_vector_complex_character((VECTOR *) EAT(l)); \
	    break; \
	  case real_integer: \
	    (r) = (void *) cast_scalar_integer_real( (SCALAR *) EAT( r ) ); \
	    break; \
	  case complex_integer: \
	    (r) = (void *) cast_scalar_integer_complex( (SCALAR *) EAT(r) ); \
	    break; \
	  case complex_real: \
	    (r) = (void *) cast_scalar_real_complex( (SCALAR *) EAT( r ) ); \
	    break; \
	  case character_integer: \
	    (r) = (void *) cast_scalar_integer_character((SCALAR *) EAT(r)); \
	    break; \
	  case character_real: \
	    (r) = (void *) cast_scalar_real_character( (SCALAR *) EAT( r ) ); \
	    break; \
	  case character_complex: \
	    (r) = (void *) cast_scalar_complex_character((SCALAR *) EAT(r)); \
	    break; \
	  default: \
	    wipeout( "Bad type.\n" ); \
	} \
    } else

#define AUTO_CAST_MATRIX_SCALAR( l, r ) \
    if ( ((MATRIX *)(l))->type != ((SCALAR *)(r))->type ) { \
	switch ( TWO_TYPE( ((MATRIX *)(l))->type, ((SCALAR *)(r))->type ) ) { \
	  case integer_real: \
	    (l) = (void *) cast_matrix_integer_real( (MATRIX *) EAT( l ) ); \
	    break; \
	  case integer_complex: \
	    (l) = (void *) cast_matrix_integer_complex( (MATRIX *) EAT(l) ); \
	    break; \
	  case real_complex: \
	    (l) = (void *) cast_matrix_real_complex( (MATRIX *) EAT( l ) ); \
	    break; \
	  case integer_character: \
	    (l) = (void *) cast_matrix_integer_character((MATRIX *) EAT(l)); \
	    break; \
	  case real_character: \
	    (l) = (void *) cast_matrix_real_character( (MATRIX *) EAT(l) ); \
	    break; \
	  case complex_character: \
	    (l) = (void *) cast_matrix_complex_character((MATRIX *) EAT(l)); \
	    break; \
	  case real_integer: \
	    (r) = (void *) cast_scalar_integer_real( (SCALAR *) EAT( r ) ); \
	    break; \
	  case complex_integer: \
	    (r) = (void *) cast_scalar_integer_complex( (SCALAR *) EAT(r) ); \
	    break; \
	  case complex_real: \
	    (r) = (void *) cast_scalar_real_complex( (SCALAR *) EAT( r ) ); \
	    break; \
	  case character_integer: \
	    (r) = (void *) cast_scalar_integer_character((SCALAR *) EAT(r)); \
	    break; \
	  case character_real: \
	    (r) = (void *) cast_scalar_real_character( (SCALAR *) EAT( r ) ); \
	    break; \
	  case character_complex: \
	    (r) = (void *) cast_scalar_complex_character((SCALAR *) EAT(r)); \
	    break; \
	  default: \
	    wipeout( "Bad type.\n" ); \
	} \
    } else

#define AUTO_CAST_MATRIX_VECTOR( l, r ) \
    if ( ((MATRIX *)(l))->type != ((VECTOR *)(r))->type ) { \
	switch ( TWO_TYPE( ((MATRIX *)(l))->type, ((VECTOR *)(r))->type ) ) { \
	  case integer_real: \
	    (l) = (void *) cast_matrix_integer_real( (MATRIX *) EAT( l ) ); \
	    break; \
	  case integer_complex: \
	    (l) = (void *) cast_matrix_integer_complex( (MATRIX *) EAT(l) ); \
	    break; \
	  case real_complex: \
	    (l) = (void *) cast_matrix_real_complex( (MATRIX *) EAT( l ) ); \
	    break; \
	  case integer_character: \
	    (l) = (void *) cast_matrix_integer_character((MATRIX *) EAT(l)); \
	    break; \
	  case real_character: \
	    (l) = (void *) cast_matrix_real_character( (MATRIX *) EAT(l) ); \
	    break; \
	  case complex_character: \
	    (l) = (void *) cast_matrix_complex_character((MATRIX *) EAT(l)); \
	    break; \
	  case real_integer: \
	    (r) = (void *) cast_vector_integer_real( (VECTOR *) EAT( r ) ); \
	    break; \
	  case complex_integer: \
	    (r) = (void *) cast_vector_integer_complex( (VECTOR *) EAT(r) ); \
	    break; \
	  case complex_real: \
	    (r) = (void *) cast_vector_real_complex( (VECTOR *) EAT( r ) ); \
	    break; \
	  case character_integer: \
	    (r) = (void *) cast_vector_integer_character((VECTOR *) EAT(r)); \
	    break; \
	  case character_real: \
	    (r) = (void *) cast_vector_real_character( (VECTOR *) EAT( r ) ); \
	    break; \
	  case character_complex: \
	    (r) = (void *) cast_vector_complex_character((VECTOR *) EAT(r)); \
	    break; \
	  default: \
	    wipeout( "Bad type.\n" ); \
	} \
    } else

#else
#define AUTO_CAST( l, l_type, r, r_type ) \
  auto_cast( (ENTITY **) &(l), l_type, (ENTITY **) &(r), r_type )
#define AUTO_CAST_SCALAR(l,r) \
  auto_cast_scalar( (SCALAR **) &(l), (SCALAR **) &(r) )
#define AUTO_CAST_VECTOR(l,r) \
  auto_cast_vector( (VECTOR **) &(l), (VECTOR **) &(r) )
#define AUTO_CAST_MATRIX(l,r) \
  auto_cast_matrix( (MATRIX **) &(l), (MATRIX **) &(r) )
#define AUTO_CAST_VECTOR_SCALAR(l,r) \
  auto_cast_vector_scalar( (VECTOR **) &(l), (SCALAR **) &(r) )
#define AUTO_CAST_MATRIX_SCALAR(l,r) \
  auto_cast_matrix_scalar( (MATRIX **) &(l), (SCALAR **) &(r) )
#define AUTO_CAST_MATRIX_VECTOR(l,r) \
  auto_cast_matrix_vector( (MATRIX **) &(l), (VECTOR **) &(r) )
#endif /* STUPID_CPP */

extern ENTITY *PROTO (bi_integer, (ENTITY * p));
extern ENTITY *PROTO (cast_entity, (ENTITY * ip, TYPE type));
extern ENTITY *PROTO (cast_scalar, (SCALAR * ips, TYPE type));
extern ENTITY *PROTO (cast_vector, (VECTOR * ipv, TYPE type));
extern ENTITY *PROTO (cast_matrix, (MATRIX * ipm, TYPE type));

extern ENTITY *PROTO (cast_scalar_integer_real, (SCALAR * ips));
extern ENTITY *PROTO (cast_scalar_integer_complex, (SCALAR * ips));
extern ENTITY *PROTO (cast_scalar_integer_character, (SCALAR * ips));
extern ENTITY *PROTO (cast_scalar_real_integer, (SCALAR * ips));
extern ENTITY *PROTO (cast_scalar_real_complex, (SCALAR * ips));
extern ENTITY *PROTO (cast_scalar_real_character, (SCALAR * ips));
extern ENTITY *PROTO (cast_scalar_complex_integer, (SCALAR * ips));
extern ENTITY *PROTO (cast_scalar_complex_real, (SCALAR * ips));
extern ENTITY *PROTO (cast_scalar_complex_character, (SCALAR * ips));
extern ENTITY *PROTO (cast_scalar_character_integer, (SCALAR * ips));
extern ENTITY *PROTO (cast_scalar_character_real, (SCALAR * ips));
extern ENTITY *PROTO (cast_scalar_character_complex, (SCALAR * ips));

extern ENTITY *PROTO (cast_vector_integer_real, (VECTOR * ipm));
extern ENTITY *PROTO (cast_vector_integer_complex, (VECTOR * ipm));
extern ENTITY *PROTO (cast_vector_integer_character, (VECTOR * ipm));
extern ENTITY *PROTO (cast_vector_real_integer, (VECTOR * ipm));
extern ENTITY *PROTO (cast_vector_real_complex, (VECTOR * ipm));
extern ENTITY *PROTO (cast_vector_real_character, (VECTOR * ipm));
extern ENTITY *PROTO (cast_vector_complex_integer, (VECTOR * ipm));
extern ENTITY *PROTO (cast_vector_complex_real, (VECTOR * ipm));
extern ENTITY *PROTO (cast_vector_complex_character, (VECTOR * ipm));
extern ENTITY *PROTO (cast_vector_character_integer, (VECTOR * ipm));

extern ENTITY *PROTO (cast_matrix_integer_real, (MATRIX * ipm));
extern ENTITY *PROTO (cast_matrix_integer_complex, (MATRIX * ipm));
extern ENTITY *PROTO (cast_matrix_integer_character, (MATRIX * ipm));
extern ENTITY *PROTO (cast_matrix_real_integer, (MATRIX * ipm));
extern ENTITY *PROTO (cast_matrix_real_complex, (MATRIX * ipm));
extern ENTITY *PROTO (cast_matrix_real_character, (MATRIX * ipm));
extern ENTITY *PROTO (cast_matrix_complex_integer, (MATRIX * ipm));
extern ENTITY *PROTO (cast_matrix_complex_real, (MATRIX * ipm));
extern ENTITY *PROTO (cast_matrix_complex_character, (MATRIX * ipm));
extern ENTITY *PROTO (cast_matrix_character_integer, (MATRIX * ipm));

#ifdef STUPID_CPP

void PROTO (auto_cast, (ENTITY ** l, TYPE l_type, ENTITY ** r, TYPE r_type));
extern void PROTO (auto_cast_scalar, (SCALAR ** l, SCALAR ** r));
extern void PROTO (auto_cast_vector, (VECTOR ** l, VECTOR ** r));
extern void PROTO (auto_cast_matrix, (MATRIX ** l, MATRIX ** r));
extern void PROTO (auto_cast_vector_scalar, (VECTOR ** l, SCALAR ** r));
extern void PROTO (auto_cast_matrix_scalar, (MATRIX ** l, SCALAR ** r));
extern void PROTO (auto_cast_matrix_vector, (MATRIX ** l, VECTOR ** r));

#endif /* STUPID_CPP */

#endif /* CAST_H */
