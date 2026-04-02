/*
   exp.h -- Prototypes for `exp.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: exp.h,v 1.1.1.1 1996/04/17 05:56:17 ksh Exp $ */

#ifndef EXP_H
#define EXP_H	1

#include "entity.h"

#if HAVE_MATHERR
#define CHECK_EXP_RESULTS(r)
#else
#define CHECK_EXP_RESULTS(r) \
	if (errno) {if (r) math_error (); errno = 0;} else;
#endif

extern COMPLEX PROTO (exp_complex, (COMPLEX z));
extern ENTITY *PROTO (bi_exp, (ENTITY * p));
extern ENTITY *PROTO (exp_scalar, (SCALAR * p));
extern ENTITY *PROTO (exp_vector, (VECTOR * p));
extern ENTITY *PROTO (exp_matrix, (MATRIX * p));

#endif /* EXP_H */
