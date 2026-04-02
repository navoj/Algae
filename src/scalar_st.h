/*
   scalar_st.h -- The `scalar' structure.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: scalar_st.h,v 1.1.1.1 1996/04/17 05:56:21 ksh Exp $ */

#ifndef  SCALAR_ST_H
#define  SCALAR_ST_H	1

#include "entity.h"

struct scalar
  {
    ENTITY entity;		/* `ref_count' and `class'  */
    TYPE type;			/* `integer', `real', etc.  */
    TABLE *stuff;		/* a table with stuff in it */
    VALUE v;			/* the value                */
  };

/*
 * Each member of the SCALAR structure
 * has a corresponding member in this
 * enumeration.  `END_Scalar' marks
 * the end of the enumeration.
 */

typedef enum
  {
    ScalarClass,
    ScalarType,
    END_Scalar
  }
SCALAR_MEMBER;

#endif /* SCALAR_ST_H  */
