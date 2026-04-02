/*
   test.h -- Prototypes for `test.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: test.h,v 1.1.1.1 1996/04/17 05:56:22 ksh Exp $ */

#ifndef TEST_H
#define TEST_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_test, (ENTITY * p));
extern int PROTO (test_entity, (ENTITY * p));
extern int PROTO (test_scalar, (SCALAR * p));
extern int PROTO (test_vector, (VECTOR * p));
extern int PROTO (test_matrix, (MATRIX * p));
extern int PROTO (test_table, (TABLE * p));
extern int PROTO (test_function, (FUNCTION * p));

#endif /* TEST_H */
