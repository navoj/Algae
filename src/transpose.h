/*
   transpose.h -- Prototypes for `transpose.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: transpose.h,v 1.1.1.1 1996/04/17 05:56:23 ksh Exp $ */

#ifndef TRANSPOSE_H
#define TRANSPOSE_H

#include "entity.h"

extern ENTITY *PROTO (transpose_entity, (ENTITY * p));
extern ENTITY *PROTO (transpose_matrix, (MATRIX * m));
extern ENTITY *PROTO (hermitian_transpose_entity, (ENTITY * p));
extern ENTITY *PROTO (hermitian_transpose_matrix, (MATRIX * m));

#endif /* TRANSPOSE_H */
