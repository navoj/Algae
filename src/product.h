/*
   product.h -- Prototypes for `product.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: product.h,v 1.1.1.1 1996/04/17 05:56:21 ksh Exp $ */

#ifndef PRODUCT_H
#define PRODUCT_H  1

#include "entity.h"

#define OWN( p )	( ENT(p)->ref_count == 1 )

extern ENTITY *PROTO (product_matrix_integer, (MATRIX * l, MATRIX * r));
extern ENTITY *PROTO (product_matrix_real, (MATRIX * l, MATRIX * r));
extern ENTITY *PROTO (product_matrix_complex, (MATRIX * l, MATRIX * r));

extern ENTITY *PROTO (product_vector_integer, (VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (product_vector_real, (VECTOR * l, VECTOR * r));
extern ENTITY *PROTO (product_vector_complex, (VECTOR * l, VECTOR * r));

#endif /* PRODUCT_H */
