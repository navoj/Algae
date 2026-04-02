/*
   imag.h -- Prototypes for `imag.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: imag.h,v 1.1.1.1 1996/04/17 05:56:18 ksh Exp $ */

#ifndef IMAG_H
#define IMAG_H	1

#include "entity.h"

extern REAL PROTO (imag_complex, (COMPLEX z));
extern ENTITY *PROTO (bi_imag, (ENTITY * p));
extern ENTITY *PROTO (imag_scalar, (SCALAR * p));
extern ENTITY *PROTO (imag_vector, (VECTOR * p));
extern ENTITY *PROTO (imag_matrix, (MATRIX * p));

#endif /* IMAG_H */
