/*
   equil.h -- Prototypes for `equil.c'.

   Copyright (C) 2001  K. Scott Hunziker.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: equil.h,v 1.1 2001/10/12 00:32:25 ksh Exp $ */

#ifndef EQUIL_H
#define EQUIL_H

#include "entity.h"

extern ENTITY *PROTO (bi_equilibrate, (ENTITY * p));
extern ENTITY *PROTO (equilibrate_matrix, (MATRIX * p));

#endif /* EQUIL_H */
