/*
   lose.h -- Prototypes for `lose.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: lose.h,v 1.1.1.1 1996/04/17 05:56:19 ksh Exp $ */

#ifndef LOSE_H
#define LOSE_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_lose, (ENTITY * ap, ENTITY * bp));
extern ENTITY *PROTO (lose_vector, (VECTOR * ap, VECTOR * bp));

#endif /* LOSE_H */
