/*
   full.h -- Prototypes for `full.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: full.h,v 1.1.1.1 1996/04/17 05:56:18 ksh Exp $ */

#ifndef FULL_H
#define FULL_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_full, (ENTITY * p));
extern ENTITY *PROTO (full_matrix, (MATRIX * p));

#endif /* FULL_H */
