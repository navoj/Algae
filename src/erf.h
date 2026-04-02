/*
   erf.h -- Prototypes for `erf.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: erf.h,v 1.2 2003/12/06 00:34:01 ksh Exp $ */

#ifndef ERF_H
#define ERF_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_erf, (ENTITY * p));
extern ENTITY *PROTO (erf_scalar, (SCALAR * p));
extern ENTITY *PROTO (erf_vector, (VECTOR * p));
extern ENTITY *PROTO (erf_matrix, (MATRIX * p));
extern ENTITY *PROTO (bi_erfc, (ENTITY * p));
extern ENTITY *PROTO (erfc_scalar, (SCALAR * p));
extern ENTITY *PROTO (erfc_vector, (VECTOR * p));
extern ENTITY *PROTO (erfc_matrix, (MATRIX * p));

#endif /* ERF_H */
