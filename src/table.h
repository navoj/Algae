/*
   table.h -- Prototypes for functions in `table.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: table.h,v 1.1.1.1 1996/04/17 05:56:22 ksh Exp $ */

#ifndef TABLE_H
#define TABLE_H	1

#include "entity.h"
#include "table_st.h"
#include "put.h"
#include <stdarg.h>

/* Generic callback types for table visitors/mappers.
 * Actual callbacks have varying signatures; callers cast as needed. */
typedef int (*PFI) (char *, ENTITY *, va_list);
typedef ENTITY *(*PFE) (ENTITY *);

#define delete_2_tables( a, b )		do { delete_table( a ); \
					     delete_table( b ); } while(0)
#define delete_3_tables( a, b, c )	do { delete_table( a ); \
					     delete_table( b ); \
					     delete_table( c ); } while(0)

#if DEBUG
#define delete_table( p )	DB_delete_table( p, __FILE__, __LINE__ )
#else
#define delete_table( p )	if ( (p) && --ENT(p)->ref_count == 0 ) \
                                  free_table( p ); else
#endif /* DEBUG */

#define copy_table( p )		copy_entity( ENT(p) )

extern int PROTO (accept_members, (TABLE * t, char **allowable_names));
extern ENTITY *PROTO (search_in_table, (TABLE * head, char *name));
extern int PROTO (exists_in_table, (TABLE * head, char *name));
extern ENTITY *PROTO (make_table, (void));
extern void PROTO (free_table, (TABLE * head));
extern void PROTO (print_table, (TABLE * head, FILE * stream));
extern ENTITY *PROTO (dup_table, (TABLE * old));
extern ENTITY *PROTO (delete_from_table, (TABLE * head, char *name));
extern ENTITY *PROTO (replace_in_table, (TABLE * head, ENTITY * new, char *name));
extern void PROTO (DB_delete_table, (TABLE * p, char *file, int line));
extern int PROTO (put_table, (TABLE * t, FILE * stream, struct ent_node *));
extern ENTITY *PROTO (get_table, (FILE * stream, int ver));
extern ENTITY *PROTO (add_table, (TABLE * l, TABLE * r));
extern ENTITY *PROTO (subtract_table, (TABLE * l, TABLE * r));
extern ENTITY *PROTO (lowercase_table, (TABLE * head));
extern char *PROTO (first_in_table, (TABLE * head));
extern char **PROTO (get_table_names, (TABLE_NODE * tn, char **names));

#if HAVE_STDARG_H
extern int PROTO (visit_table, (TABLE * t, PFI f,...));
#else
extern int visit_table (TABLE * t, PFI f, ...);
#endif

extern ENTITY *PROTO (binop_table_other, (int op, TABLE * l, ENTITY * r));
extern ENTITY *PROTO (binop_other_table, (int op, ENTITY * l, TABLE * r));
extern ENTITY *PROTO (unop_table, (int op, TABLE * t));
extern ENTITY *PROTO (map_table, (TABLE * t, PFE f, int n, ENTITY ** args));

#endif /* TABLE_H */
