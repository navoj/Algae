/*
   abs.h -- Prototypes for `abs.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: abs.h,v 1.1.1.1 1996/04/17 05:56:14 ksh Exp $ */

#ifndef ABS_H
#define ABS_H	1

#include "entity.h"

#if !HAVE_ABS
extern int PROTO (abs, (int i));
#endif

extern REAL PROTO (abs_complex, (COMPLEX z));
extern ENTITY *PROTO (bi_abs, (ENTITY * p));
extern ENTITY *PROTO (abs_scalar, (SCALAR * p));
extern ENTITY *PROTO (abs_vector, (VECTOR * p));
extern ENTITY *PROTO (abs_matrix, (MATRIX * p));

#endif /* ABS_H */
