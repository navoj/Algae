/* src/blas.h.  Generated from blas.h.in by configure. */

/*
   blas.h -- Prototypes for the BLAS routines.

   Copyright (C) 1994-96  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: blas.h.in,v 1.2 1996/10/04 06:30:35 ksh Exp $ */

#ifndef BLAS_H
#define BLAS_H	1

#if !NO_FORTRAN_DEFS
#define DSCAL dscal_
#define RGEMM rgemm_
#define XGEMM xgemm_
#define ZSCAL zscal_
#endif

extern void PROTO (DSCAL, (int *n, REAL * da, REAL * dx, int *incx));
extern void PROTO (RGEMM, (int *itrnsa, int *itrnsb, int *m, int *n,
	       int *k, REAL * alpha, REAL * a, int *lda, REAL * b, int *ldb,
			   REAL * beta, REAL * c, int *ldc));
extern void PROTO (XGEMM, (int *itrnsa, int *itrnsb, int *m, int *n,
		int *k, COMPLEX * alpha, COMPLEX * a, int *lda, COMPLEX * b,
			   int *ldb, COMPLEX * beta, COMPLEX * c, int *ldc));
extern void PROTO (ZSCAL, (int *n, COMPLEX * da,
			   COMPLEX * dx, int *incx));

#endif /* BLAS_H */
