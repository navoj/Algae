/*
   eig.h -- Prototypes for `eig.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: eig.h,v 1.1.1.1 1996/04/17 05:56:17 ksh Exp $ */

#ifndef EIG_H
#define EIG_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_eig, (int n, ENTITY * a, ENTITY * b, ENTITY * c));
extern ENTITY *PROTO (seig_matrix, (MATRIX * a, TABLE * opt));
extern ENTITY *PROTO (geig_matrix, (MATRIX * a, MATRIX * b, TABLE * opt));

#endif /* EIG_H */
