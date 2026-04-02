/*
   norm.h -- Prototypes for `norm.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: norm.h,v 1.1.1.1 1996/04/17 05:56:20 ksh Exp $ */

#ifndef NORM_H
#define NORM_H	1

#include "entity.h"

typedef enum
  {
    infinity_norm, one_norm, two_norm, frobenius_norm
  }
NORM_TYPE;

extern ENTITY *PROTO (bi_norm, (int n, ENTITY * a, ENTITY * p));
extern ENTITY *PROTO (norm_matrix, (MATRIX * a, NORM_TYPE nt));
extern ENTITY *PROTO (norm_vector, (VECTOR * a, NORM_TYPE nt));

#endif /* NORM_H */
