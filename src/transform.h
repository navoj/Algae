/*
   transform.h -- Prototypes for `transform.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: transform.h,v 1.1.1.1 1996/04/17 05:56:23 ksh Exp $ */

#ifndef TRANSFORM_H
#define TRANSFORM_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_transform, (ENTITY * a, ENTITY * p));
extern ENTITY *PROTO (transform_matrix, (MATRIX * a, MATRIX * p));

#endif /* TRANSFORM_H */
