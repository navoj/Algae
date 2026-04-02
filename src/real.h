/*
   real.h -- Prototypes for `real.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: real.h,v 1.1.1.1 1996/04/17 05:56:21 ksh Exp $ */

#ifndef REAL_H
#define REAL_H	1

#include "entity.h"

#if HAVE_RINT
#define round(x) rint(x)
#else
#define round(x) floor( (x) + 0.5 )
#endif


extern ENTITY *PROTO (bi_real, (ENTITY * p));

extern COMPLEX PROTO (ceil_complex, (COMPLEX z));
extern ENTITY *PROTO (bi_ceil, (ENTITY * p));
extern ENTITY *PROTO (ceil_scalar, (SCALAR * p));
extern ENTITY *PROTO (ceil_vector, (VECTOR * p));
extern ENTITY *PROTO (ceil_matrix, (MATRIX * p));

extern COMPLEX PROTO (floor_complex, (COMPLEX z));
extern ENTITY *PROTO (bi_floor, (ENTITY * p));
extern ENTITY *PROTO (floor_scalar, (SCALAR * p));
extern ENTITY *PROTO (floor_vector, (VECTOR * p));
extern ENTITY *PROTO (floor_matrix, (MATRIX * p));

#if HAVE_RINT
#if !HAVE_RINT_DECL
extern REAL PROTO (rint, (REAL r));
#endif
#else
extern REAL PROTO (round_real, (REAL r));
#endif

extern COMPLEX PROTO (round_complex, (COMPLEX z));
extern ENTITY *PROTO (bi_round, (ENTITY * p));
extern ENTITY *PROTO (round_scalar, (SCALAR * p));
extern ENTITY *PROTO (round_vector, (VECTOR * p));
extern ENTITY *PROTO (round_matrix, (MATRIX * p));

#endif /* REAL_H */
