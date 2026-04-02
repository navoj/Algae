/*
   rand.h -- Prototypes for `rand.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: rand.h,v 1.1.1.1 1996/04/17 05:56:21 ksh Exp $ */

#ifndef RAND_H
#define RAND_H	1

#include "entity.h"

extern int PROTO (rand_init, (void));
extern ENTITY *PROTO (bi_srand, (int n, ENTITY * s));
extern ENTITY *PROTO (bi_rand, (int n, ENTITY * shape));

#endif /* RAND_H */
