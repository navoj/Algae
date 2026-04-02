/*
   cosh.h -- Prototypes for `cosh.c'.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: cosh.h,v 1.3 2003/02/09 08:58:41 ksh Exp $ */

#ifndef COSH_H
#define COSH_H	1

#include "entity.h"

#if !HAVE_ACOSH
extern double PROTO (acosh, (double z));
#endif

extern COMPLEX PROTO (cosh_complex, (COMPLEX z));
extern ENTITY *PROTO (bi_cosh, (ENTITY * p));
extern ENTITY *PROTO (cosh_scalar, (SCALAR * p));
extern ENTITY *PROTO (cosh_vector, (VECTOR * p));
extern ENTITY *PROTO (cosh_matrix, (MATRIX * p));
extern COMPLEX PROTO (acosh_complex, (COMPLEX z));
extern ENTITY *PROTO (bi_acosh, (ENTITY * p));
extern ENTITY *PROTO (acosh_scalar, (SCALAR * p));
extern ENTITY *PROTO (acosh_vector, (VECTOR * p));
extern ENTITY *PROTO (acosh_matrix, (MATRIX * p));

#endif /* COSH_H */
