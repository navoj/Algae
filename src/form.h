/*
   form.h

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: form.h,v 1.1.1.1 1996/04/17 05:56:17 ksh Exp $ */

#ifndef FORM_H
#define FORM_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_form, (ENTITY * shape, ENTITY * v));
extern ENTITY *PROTO (bi_form_scalar, (VECTOR * shape, SCALAR * v));
extern ENTITY *PROTO (bi_form_vector, (VECTOR * shape, VECTOR * v));
extern ENTITY *PROTO (bi_form_matrix, (VECTOR * shape, MATRIX * v));

#endif /* FORM_H */
