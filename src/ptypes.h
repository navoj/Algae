/*
   ptypes.h -- parser types

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: ptypes.h,v 1.1.1.1 1996/04/17 05:56:21 ksh Exp $ */

#ifndef  PTYPES_H
#define  PTYPES_H	1

#include "datum.h"


/* global  symbol table structure */

typedef struct
  {
    char *name;
    int scope;
    union
      {
	DATUM *datum;
	int offset;		/* for locals -- offset in local frame */
      }
    stval;
  }
SYMTAB;

#define  NO_SCOPE	(-1)

extern int scope_level;

SYMTAB *PROTO (insert, (char *));
SYMTAB *PROTO (find, (char *, int));
ENTITY *PROTO (symbols_to_table, (void));
char *PROTO (address_to_id, (DATUM * dp));


/* a stack machine instruction */
typedef union
  {
    int op;
    void *ptr;
  }
INST;


/* used to recognize the type of two DATUMs */
#define  TWO_TYPES(l,r,n) (((int)(n))*((int)(l))+((int)(r)))


typedef struct uf_block UF_BLOCK;

void PROTO (push_scope, (UF_BLOCK *));
UF_BLOCK *PROTO (pop_scope, (void));

#endif /* PTYPES_H */
