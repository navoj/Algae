/*
   band.h -- Prototypes for `band.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: band.h,v 1.2 2001/10/04 02:31:34 ksh Exp $ */

#ifndef BAND_H
#define BAND_H

#include "entity.h"

extern ENTITY *PROTO (bi_gpskca, (int n, ENTITY * p, ENTITY * b));
extern ENTITY *PROTO (gpskca_matrix, (MATRIX * p, int f));
extern ENTITY *PROTO (bi_band, (ENTITY * p));
extern ENTITY *PROTO (band_matrix, (MATRIX * p));

#endif /* BAND_H */
