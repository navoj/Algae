/*
   find.h -- Prototypes for `find.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: find.h,v 1.1.1.1 1996/04/17 05:56:17 ksh Exp $ */

#ifndef FIND_H
#define FIND_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_find, (ENTITY * ap, ENTITY * bp));
extern ENTITY *PROTO (find_vector, (VECTOR * ap, VECTOR * bp));

#endif /* FIND_H */
