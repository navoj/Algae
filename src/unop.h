/*
   unop.h -- Header for `unop.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: unop.h,v 1.1.1.1 1996/04/17 05:56:23 ksh Exp $ */

#ifndef UNOP_H
#define UNOP_H	1

#include "entity.h"

enum
  {
    UO_NEG,
    UO_NOT,
    UO_TRANS
  };

ENTITY *PROTO (unop_entity, (int op, ENTITY * ep));

#endif /* UNOP_H */
