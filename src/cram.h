/*
   cram.h

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: cram.h,v 1.1 2001/07/09 03:50:54 ksh Exp $ */

#ifndef CRAM_H
#define CRAM_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_cram, (ENTITY * shape, ENTITY * v));
extern ENTITY *PROTO (bi_cram_scalar, (VECTOR * shape, SCALAR * v));
extern ENTITY *PROTO (bi_cram_vector, (VECTOR * shape, VECTOR * v));
extern ENTITY *PROTO (bi_cram_matrix, (VECTOR * shape, MATRIX * v));

#endif /* CRAM_H */
