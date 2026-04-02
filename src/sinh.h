/*
   sinh.h -- Prototypes for `sinh.c'.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: sinh.h,v 1.3 2003/02/09 08:58:41 ksh Exp $ */

#ifndef SINH_H
#define SINH_H	1

#include "entity.h"

#if !HAVE_ASINH
extern double PROTO (asinh, (double z));
#endif

extern COMPLEX PROTO (sinh_complex, (COMPLEX z));
extern COMPLEX PROTO (asinh_complex, (COMPLEX z));
extern ENTITY *PROTO (bi_sinh, (ENTITY * p));
extern ENTITY *PROTO (sinh_scalar, (SCALAR * p));
extern ENTITY *PROTO (sinh_vector, (VECTOR * p));
extern ENTITY *PROTO (sinh_matrix, (MATRIX * p));
extern ENTITY *PROTO (bi_asinh, (ENTITY * p));
extern ENTITY *PROTO (asinh_scalar, (SCALAR * p));
extern ENTITY *PROTO (asinh_vector, (VECTOR * p));
extern ENTITY *PROTO (asinh_matrix, (MATRIX * p));

#endif /* SINH_H */
