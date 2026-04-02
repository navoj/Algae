/*
   sparse.h -- Prototypes for `sparse.c'.

   Copyright (C) 1994-2002  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: sparse.h,v 1.3 2002/05/18 17:35:16 ksh Exp $ */

#ifndef SPARSE_H
#define SPARSE_H	1

#include "entity.h"
#include "sort.h"

extern ENTITY *PROTO (sparse_entity, (ENTITY * ip));
extern ENTITY *PROTO (sparse_vector, (VECTOR * ipv));
extern ENTITY *PROTO (sparse_matrix, (MATRIX * ipm));
extern ENTITY *PROTO (squeeze_matrix, (MATRIX * ipm));
extern ENTITY *PROTO (squeeze_vector, (VECTOR * ipv));
extern ENTITY *PROTO (sparse_to_band, (MATRIX * m));
extern ENTITY *PROTO (bi_mksparse, (int nn, ENTITY * t));
extern ENTITY *PROTO (bi_exsparse, (ENTITY * x));
extern ENTITY *PROTO (triu_entity, (int i, ENTITY *p, ENTITY *s));
extern ENTITY *PROTO (triu_matrix, (MATRIX *p, int n));
extern ENTITY *PROTO (tril_entity, (int i, ENTITY *p, ENTITY *s));
extern ENTITY *PROTO (tril_matrix, (MATRIX *p, int n));

#endif /* SPARSE_H */
