/*
   matrix_st.h -- The `matrix' structure.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: matrix_st.h,v 1.1.1.1 1996/04/17 05:56:19 ksh Exp $ */

#ifndef  MATRIX_ST_H
#define  MATRIX_ST_H	1

#include "entity.h"

struct matrix
  {
    ENTITY entity;
    SYMMETRY symmetry;
    TYPE type;
    ORDER order;
    DENSITY density;
    ENTITY *rid;
    ENTITY *cid;
    int nr;			/* number of rows */
    int nc;			/* number of columns */
    int nn;			/* number of nonzero elements */
    int *ia;			/* integer vector of length nr+1 */
    int *ja;			/* integer vector of length nn */
    TABLE *stuff;
    VAL_PTR a;			/* numeric/character components */
    VAL_PTR d;			/* diagonal values (sparse_upper) */
  };

/*
 * Each member of the MATRIX structure
 * has a corresponding member in this
 * enumeration.  `END_Matrix' marks
 * the end of the enumeration.
 */

typedef enum
  {
    MatrixClass,
    MatrixType,
    MatrixSymmetry,
    MatrixOrder,
    MatrixDensity,
    MatrixRid,
    MatrixCid,
    MatrixNr,
    MatrixNc,
    MatrixNn,
    END_Matrix
  }
MATRIX_MEMBER;

#endif /* MATRIX_ST_H  */
