/*
   solve.h -- Prototypes for `solve.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: solve.h,v 1.1.1.1 1996/04/17 05:56:22 ksh Exp $ */

#ifndef SOLVE_H
#define SOLVE_H	1

#include "entity.h"
#include "bcs.h"

extern ENTITY *PROTO (bi_backsub, (ENTITY * a, ENTITY * b));
extern ENTITY *PROTO (backsub_table, (TABLE * a, MATRIX * b));

#endif /* SOLVE_H */
