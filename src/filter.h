/*
   filter.h -- Prototypes for `filter.c'.

   Copyright (C) 1995  K. Scott Hunziker.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: filter.h,v 1.1.1.1 1996/04/17 05:56:17 ksh Exp $ */

#ifndef FILTER_H
#define FILTER_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_filter, (int n, ENTITY * b, ENTITY * a,
				  ENTITY * x, ENTITY * z));
extern ENTITY *PROTO (filter_vector, (VECTOR * b, VECTOR * a,
				      VECTOR * x, VECTOR * z));

#endif /* FILTER_H */
