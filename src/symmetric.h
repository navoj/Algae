/*
   symmetric.h -- Prototypes for `symmetric.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: symmetric.h,v 1.1.1.1 1996/04/17 05:56:22 ksh Exp $ */

#ifndef SYMMETRIC_H
#define SYMMETRIC_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_symmetric, (ENTITY * p));
extern ENTITY *PROTO (symmetric_matrix, (MATRIX * p));

extern ENTITY *PROTO (bi_hermitian, (ENTITY * p));
extern ENTITY *PROTO (hermitian_matrix, (MATRIX * p));

#endif /* SYMMETRIC_H */
