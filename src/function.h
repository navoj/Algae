/*
   function.h -- defines for functions

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: function.h,v 1.2 1997/02/21 09:53:57 ksh Exp $ */

/* function.h */

#ifndef  FUNCTION_H
#define  FUNCTION_H	1

#include "algae.h"
#include "entity.h"
#include "ptypes.h"
#include "datum.h"
#include "put.h"


typedef struct builtin BUILTIN;


struct uf_block
  {
    int cargs;			/* number of call args */
    int targs;			/* number of total args */
    INST *code;			/* execution starts here */
    DATUM *my_datum;
    int self_flag;		/* on if recursive */
  };



struct function
  {
    ENTITY entity;
    ILK ilk;
    TABLE *stuff;
    void *funct_p;		/* *BUILTIN or *UF_BLOCK */
  };

/*
 * Each member of the FUNCTION structure
 * has a corresponding member in this
 * enumeration.  `END_Function' marks
 * the end of the enumeration.
 */

typedef enum
  {
    FunctionClass,
    FunctionIlk,
    END_Function
  }
FUNCTION_MEMBER;

#define delete_2_functions( a, b )	do { delete_function( a ); \
					     delete_function( b ); } while(0)
#define delete_3_functions( a, b, c )	do { delete_function( a ); \
					     delete_function( b ); \
					     delete_function( c ); } while(0)

#if DEBUG
#define delete_function( p )	DB_delete_function( p, __FILE__, __LINE__ )
#else
#define delete_function( p )	if ( (p) && --ENT(p)->ref_count == 0 ) \
                                  free_function( p ); else
#endif /* DEBUG */

#define copy_function( p )	copy_entity( ENT(p) )

extern FUNCTION_MEMBER PROTO (function_member_search, (char *s));
extern void PROTO (free_function, (FUNCTION * p));
extern FUNCTION *PROTO (make_function, (ILK, void *));
extern ENTITY *PROTO (dup_function, (FUNCTION * p));
extern void PROTO (DB_delete_function, (FUNCTION * p, char *file, int line));
extern ENTITY *PROTO (dup_function, (FUNCTION * p));
extern ENTITY *PROTO (execute_function,
	        (FUNCTION * fp, int num_args, DATUM * argp, int arg_space));
extern ENTITY *PROTO (execute_uf, (UF_BLOCK *, int, DATUM *, int, FUNCTION *));

extern ENTITY *PROTO (bi_strip, (ENTITY * p));
extern int PROTO (code_size, (INST * start));
extern int PROTO (put_code, (INST * start, FILE * stream, struct ent_node *));
extern INST *PROTO (get_code, (FILE * stream, FUNCTION * f));
extern int PROTO (put_function, (FUNCTION * f, FILE * stream, struct ent_node *));
extern ENTITY *PROTO (get_function, (FILE * stream, int ver));
extern void PROTO (bi_init, (void));

#endif /* FUNCTION_H */
