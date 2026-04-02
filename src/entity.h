/*
   entity.h -- Entity enums and structures.

   Copyright (C) 1994-2002  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: entity.h,v 1.5 2002/07/30 22:04:44 ksh Exp $ */

#ifndef  ENTITY_H
#define  ENTITY_H	1

#include "algae.h"

typedef enum
  {
    scalar,
    table,
    vector,
    matrix,
    function,
    undefined_class,
    NUM_CLASS
  }
CLASS;
extern char *class_string[];
extern char bad_class[];
#define BAD_CLASS( c )		fail( bad_class, class_string[c] )

/*
 * The "skew" symmetries are no longer with us.  They need to stay
 * in this enum, though, because Algae binary files expect this to
 * remain constant.
 */

typedef enum
  {
    general,
    symmetric,
    skew_symmetric,		/* not used */
    hermitian,
    skew_hermitian,		/* not used */
    undefined_symmetry,
    NUM_SYMMETRY
  }
SYMMETRY;
extern char *symmetry_string[];
extern char bad_symmetry[];
#define BAD_SYMMETRY( s )	fail( bad_symmetry, symmetry_string[s] )

typedef enum
  {
    dense,
    sparse,
    sparse_upper,
    undefined_density,
    NUM_DENSITY
  }
DENSITY;
extern char *density_string[];
extern char bad_density[];
#define BAD_DENSITY( d )	fail( bad_density, density_string[d] )

typedef enum
  {
    ordered,
    not_ordered,
    undefined_order,
    NUM_ORDER
  }
ORDER;
extern char *order_string[];
extern char bad_order[];
#define BAD_ORDER( o )		fail( bad_order, order_string[o] )

/* Automatic type casting is done from lower to greater type value. */

typedef enum
  {
    integer,
    real,
    complex,
    character,
    undefined_type,
    NUM_TYPE
  }
TYPE;
extern char *type_string[];
extern char bad_type[];
#define BAD_TYPE( t )		fail( bad_type, type_string[t] )
extern TYPE auto_cast_table[NUM_TYPE][NUM_TYPE];
extern int type_size[];

typedef enum
  {
    builtin,
    user,
    undefined_ilk,
    NUM_ILK
  }
ILK;
extern char *ilk_string[];
extern char bad_ilk[];
#define BAD_ILK( t )		fail( bad_ilk, ilk_string[t] )

/* ************************************************************ */

typedef union
  {
    int integer;
    REAL real;
    COMPLEX complex;
    char *character;
  }
VALUE;

typedef union
  {
    int *integer;
    REAL *real;
    COMPLEX *complex;
    char **character;
    void *ptr;			/* When I don't care what type it is. */
  }
VAL_PTR;

struct entity
  {
    int ref_count;		/* Number of references to this entity. */
    CLASS class;
  };

typedef struct
  {
    char *name;			/* Each member of the scalar, matrix, etc. */
    int id;			/* structs is given a `name' and an `id'.  */
  }
MEMBER_ID;

#include "ok.h"

#define EAT( p )	eat( (void **) &(p) )

#define ENT( p )	( (ENTITY *) ( p ) )
#define delete_2_entities( a, b )	do { delete_entity( a ); \
					     delete_entity( b ); } while(0)
#define delete_3_entities( a, b, c )	do { delete_entity( a ); \
					     delete_entity( b ); \
					     delete_entity( c ); } while(0)

#define FREE_CHAR( p )		if ( p != NULL_string ) FREE( p ); else ;
#define ASSIGN_CHAR( p, q )	FREE_CHAR( p ); p = q
#define TFREE( p )		if ( p ) FREE( EAT( p ) ); else ;

#define PLURAL( i )	( ( ( i ) == 1 ) ? "" : "s" )
#define TH( i )		th( i )

#define IN_TABLE( t, s )	( (t) != NULL && exists_in_table( (TABLE *) \
						     copy_table( t ), s ) )
#define OK_OPTIONS( t, s )	( (t) == NULL || accept_members( (TABLE *) \
						     copy_table( t ), s ) )

#if DEBUG
#define delete_entity( p )	DB_delete_entity( p, __FILE__, __LINE__ )
#define copy_entity( p )	DB_copy_entity( ENT(p), __FILE__, __LINE__ )
#else
#define delete_entity( p )	if ( (p) && --ENT(p)->ref_count == 0 ) \
                                  free_entity( p ); else
#define copy_entity( p )	( ++ENT(p)->ref_count, ENT(p) )
#endif /* DEBUG */

extern void *PROTO (eat, (void **p));
extern ENTITY *PROTO (DB_copy_entity, (ENTITY * p, char *file, int line));
extern void PROTO (DB_delete_entity, (ENTITY * p, char *file, int line));
extern void PROTO (free_entity, (ENTITY * p));
extern ENTITY *PROTO (dup_entity, (ENTITY * p));
extern char *PROTO (dup_char, (char *p));
extern char *PROTO (append_char, (char *l, char *r));
extern void *PROTO (dup_mem, (void *p, int len));
extern ENTITY *PROTO (not_NULL, (ENTITY * p));
extern int PROTO (member_cmp, (const void *keyval, const void *dat));
extern ENTITY *PROTO (bi_class, (int n, ENTITY * p));
extern char *PROTO (th, (int i));
extern int PROTO (entity_to_int, (ENTITY * e));
extern REAL PROTO (entity_to_real, (ENTITY * e));
extern COMPLEX PROTO (entity_to_complex, (ENTITY * e));
extern char *PROTO (entity_to_string, (ENTITY * e));
extern void *PROTO (memmove_forward, (void *s, void *t, size_t n));

#endif /* ENTITY_H  */
