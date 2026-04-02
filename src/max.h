/*
   max.h -- Prototypes for `max.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: max.h,v 1.2 2003/09/01 18:48:06 ksh Exp $ */

#ifndef MAX_H
#define MAX_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_max, (ENTITY * v));
extern ENTITY *PROTO (max_vector, (VECTOR * v));
extern ENTITY *PROTO (max_matrix, (MATRIX * v));
extern ENTITY *PROTO (bi_min, (ENTITY * v));
extern ENTITY *PROTO (min_vector, (VECTOR * v));
extern ENTITY *PROTO (min_matrix, (MATRIX * v));

#endif /* MAX_H */
