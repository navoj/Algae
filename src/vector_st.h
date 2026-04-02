/*
   vector_st.h -- The `vector' structure.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: vector_st.h,v 1.1.1.1 1996/04/17 05:56:23 ksh Exp $ */

#ifndef  VECTOR_ST_H
#define  VECTOR_ST_H	1

#include "entity.h"

struct vector
  {
    ENTITY entity;
    TYPE type;
    ORDER order;
    DENSITY density;
    ENTITY *eid;
    int ne;			/* number of elements */
    int nn;			/* number of elements actually stored */
    int *ja;			/* integer vector of length nn */
    TABLE *stuff;
    VAL_PTR a;			/* numeric/character components */
  };


/*
 * Each member of the VECTOR structure
 * has a corresponding member in this
 * enumeration.  `END_Vector' marks
 * the end of the enumeration.
 */

typedef enum
  {
    VectorClass,
    VectorType,
    VectorOrder,
    VectorDensity,
    VectorEid,
    VectorNe,
    VectorNn,
    END_Vector
  }
VECTOR_MEMBER;

#endif /* VECTOR_ST_H  */
