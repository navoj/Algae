/*
   generate.h -- Prototypes for `generate.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: generate.h,v 1.1.1.1 1996/04/17 05:56:18 ksh Exp $ */

#ifndef GENERATE_H
#define GENERATE_H	1

#include "entity.h"

extern ENTITY *PROTO (generate_entity, (ENTITY * start, ENTITY * end, ENTITY * inc));
extern ENTITY *PROTO (generate_integer, (int start, int end, int inc));
extern ENTITY *PROTO (generate_real, (REAL start, REAL end, REAL inc));
extern ENTITY *PROTO (generate_complex, (COMPLEX start, COMPLEX end, COMPLEX inc));

#endif /* GENERATE_H */
