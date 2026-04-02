/*
   dense.h -- Prototypes for `dense.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: dense.h,v 1.1.1.1 1996/04/17 05:56:16 ksh Exp $ */

#ifndef DENSE_H
#define DENSE_H	1

#include "entity.h"

extern ENTITY *PROTO (dense_entity, (ENTITY * ip));
extern ENTITY *PROTO (dense_matrix, (MATRIX * ipm));
extern ENTITY *PROTO (dense_vector, (VECTOR * ipv));
extern ENTITY *PROTO (apt_vector, (VECTOR * v));
extern ENTITY *PROTO (apt_matrix, (MATRIX * v));

extern int PROTO (fat_vector, (int ne, int nn,
			       TYPE type, DENSITY density));
extern int PROTO (fat_matrix, (int nr, int nc, int nn,
			       TYPE type, DENSITY density));

#endif /* DENSE_H */
