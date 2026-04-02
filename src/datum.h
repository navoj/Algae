/*
   datum.h -- Macros and prototypes for `datum.c'.

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: datum.h,v 1.2 1997/02/21 09:53:53 ksh Exp $ */

#ifndef  DATUM_H
#define  DATUM_H	1

#include "algae.h"
#include "put.h"

typedef enum
  {
    D_NULL,
    D_INT,
    D_REAL,
    D_ENTITY,
    NUM_DATUM
  }
DATUM_TYPE;

struct datum
  {
    DATUM_TYPE type;
    union
      {
	int ival;		/* type == D_INT */
	double rval;		/* type == D_REAL */
	void *ptr;		/* other types */
      }
    data;
  };

#define IVAL(p)	(p)->data.ival
#define RVAL(p)	(p)->data.rval
#define E_PTR(p)  ((ENTITY*)(p)->data.ptr)
#define D_PTR(p)  ((DATUM*)(p)->data.ptr)
#define PTR(p)		(p)->data.ptr


#define TWO_DATUM_(l,r) TWO_TYPES(l,r,NUM_DATUM)
#define TEST2DATUM(p)	TWO_DATUM_((p)->type,((p)+1)->type)


#define NULL_NULL	TWO_DATUM_(D_NULL,D_NULL)
#define NULL_INT	TWO_DATUM_(D_NULL,D_INT)
#define NULL_REAL	TWO_DATUM_(D_NULL,D_REAL)
#define NULL_ENTITY	TWO_DATUM_(D_NULL,D_ENTITY)
#define INT_NULL	TWO_DATUM_(D_INT,D_NULL)
#define INT_INT		TWO_DATUM_(D_INT,D_INT)
#define INT_REAL	TWO_DATUM_(D_INT,D_REAL)
#define INT_ENTITY	TWO_DATUM_(D_INT,D_ENTITY)
#define REAL_NULL	TWO_DATUM_(D_REAL,D_NULL)
#define REAL_INT	TWO_DATUM_(D_REAL,D_INT)
#define REAL_REAL	TWO_DATUM_(D_REAL,D_REAL)
#define REAL_ENTITY	TWO_DATUM_(D_REAL,D_ENTITY)
#define ENTITY_NULL	TWO_DATUM_(D_ENTITY,D_NULL)
#define ENTITY_INT	TWO_DATUM_(D_ENTITY,D_INT)
#define ENTITY_REAL	TWO_DATUM_(D_ENTITY,D_REAL)
#define ENTITY_ENTITY	TWO_DATUM_(D_ENTITY,D_ENTITY)


#define CASE_any_NULL   case  NULL_NULL :\
			case  NULL_INT  :\
			case  NULL_REAL  :\
			case  NULL_ENTITY  :\
			case  INT_NULL  :\
			case  REAL_NULL  :\
			case  ENTITY_NULL

#if DEBUG
#define delete_datum(p)		DB_delete_datum(p)
#else
#define delete_datum(p)		do {\
				if ( (p)->type == D_ENTITY )\
				{ delete_entity(E_PTR(p)) ; }\
				(p)->type = D_NULL ; } while(0)
#endif


#define  datum_assign(to,from)  do{ delete_datum(to) ;\
				    copy_datum(to,from) ; }while(0)

#define  assign_datum(t,f)	datum_assign(t,f)

extern DATUM *vociferant;

DATUM *PROTO (new_DATUM, (void));
DATUM *PROTO (new_INT, (int));
DATUM *PROTO (new_REAL, (double));
DATUM *PROTO (copy_datum, (DATUM *, DATUM *));
void PROTO (DB_delete_datum, (DATUM *));
int PROTO (datum_test, (DATUM * dp));
int PROTO (datum_to_int, (DATUM * dp));
extern int PROTO (put_datum, (DATUM * d, FILE * stream, struct ent_node *));
extern DATUM *PROTO (get_datum, (FILE * stream));
extern void PROTO (datum_print, (DATUM * p));
extern DATUM_TYPE PROTO (cast2_datum, (DATUM * p));
extern DATUM_TYPE PROTO (cast2_to_entity, (DATUM * p));
extern int PROTO (cast1_to_entity, (DATUM * dp));

#endif /* DATUM_H */
