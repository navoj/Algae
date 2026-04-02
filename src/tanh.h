/*
   tanh.h -- Prototypes for `tanh.c'.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: tanh.h,v 1.3 2003/02/09 08:58:41 ksh Exp $ */

#ifndef TANH_H
#define TANH_H	1

#include "entity.h"

#if !HAVE_ATANH
extern double PROTO (atanh, (double z));
#endif

extern COMPLEX PROTO (tanh_complex, (COMPLEX z));
extern ENTITY *PROTO (bi_tanh, (ENTITY * p));
extern ENTITY *PROTO (tanh_scalar, (SCALAR * p));
extern ENTITY *PROTO (tanh_vector, (VECTOR * p));
extern ENTITY *PROTO (tanh_matrix, (MATRIX * p));
extern COMPLEX PROTO (atanh_complex, (COMPLEX z));
extern ENTITY *PROTO (bi_atanh, (ENTITY * p));
extern ENTITY *PROTO (atanh_scalar, (SCALAR * p));
extern ENTITY *PROTO (atanh_vector, (VECTOR * p));
extern ENTITY *PROTO (atanh_matrix, (MATRIX * p));

#endif /* TANH_H */
