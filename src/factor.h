
/*
   factor.h -- Prototypes for `factor.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: factor.h,v 1.1.1.1 1996/04/17 05:56:17 ksh Exp $ */

#ifndef FACTOR_H
#define FACTOR_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_factor, (ENTITY * a));
extern ENTITY *PROTO (factor_matrix, (MATRIX * a));
extern ENTITY *PROTO (bi_chol, (ENTITY * a));
extern ENTITY *PROTO (chol_matrix, (MATRIX * a));

#if HAVE_LIBBCSEXT
ENTITY *PROTO (factor_bcs_real, (MATRIX * a, REAL pvttol));
ENTITY *PROTO (factor_bcs_complex, (MATRIX * a, REAL pvttol));
#endif

#endif /* FACTOR_H */
