/*
   diag.h -- Prototypes for `diag.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: diag.h,v 1.1.1.1 1996/04/17 05:56:16 ksh Exp $ */

#ifndef DIAG_H
#define DIAG_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_diag, (ENTITY * p));
extern ENTITY *PROTO (diag_matrix, (MATRIX * p));
extern ENTITY *PROTO (diag_vector, (VECTOR * p));

#endif /* DIAG_H */
