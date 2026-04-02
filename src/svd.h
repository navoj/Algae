/*
   svd.h -- Prototypes for `svd.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: svd.h,v 1.1.1.1 1996/04/17 05:56:22 ksh Exp $ */

#ifndef SVD_H
#define SVD_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_svd, (int n, ENTITY * p, ENTITY * t));
extern ENTITY *PROTO (svd_matrix, (MATRIX * p, TABLE * t));

#endif /* SVD_H */
