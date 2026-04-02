/*
   asin.h -- Prototypes for `asin.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: asin.h,v 1.1.1.1 1996/04/17 05:56:15 ksh Exp $ */

#ifndef ASIN_H
#define ASIN_H	1

#include "entity.h"

extern COMPLEX PROTO (asin_complex, (COMPLEX z));
extern ENTITY *PROTO (bi_asin, (ENTITY * p));
extern ENTITY *PROTO (asin_scalar, (SCALAR * p));
extern ENTITY *PROTO (asin_vector, (VECTOR * p));
extern ENTITY *PROTO (asin_matrix, (MATRIX * p));

#endif /* ASIN_H */
