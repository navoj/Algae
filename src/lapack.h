/* src/lapack.h.  Generated from lapack.h.in by configure. */

/*
   lapack.h -- Interface for LAPACK routines.

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: lapack.h.in,v 1.6 2001/10/12 00:36:11 ksh Exp $ */

#ifndef LAPACK_H
#define LAPACK_H	1

#include "algae.h"

#if !NO_FORTRAN_DEFS
#define DGEEQU	dgeequ_
#define DPOEQU	dpoequ_
#define DGESV	dgesv_
#define RGESVD	rgesvd_
#define DGETRF	dgetrf_
#define RGETRS	rgetrs_
#define XGETRS	xgetrs_
#define RGEEV	rgeev_
#define RGGEV	rggev_
#define XGGEV	xggev_
#define RPOSV	rposv_
#define RPOTRF	rpotrf_
#define RPOTRS	rpotrs_
#define XPOTRS	xpotrs_
#define RSYEV	rsyev_
#define RSYGV	rsygv_
#define RSYSV	rsysv_
#define RSYTRF	rsytrf_
#define XSYTRF	xsytrf_
#define XHETRF	xhetrf_
#define RSYTRS	rsytrs_
#define XSYTRS	xsytrs_
#define XHETRS	xhetrs_
#define ZGEEQU	zgeequ_
#define ZPOEQU	zpoequ_
#define XGEEV	xgeev_
#define XGESVD	xgesvd_
#define ZGETRF	zgetrf_
#define XHEEV	xheev_
#define XHEGV	xhegv_
#define XHESV	xhesv_
#define XPOSV	xposv_
#define XSYCON	xsycon_
#define XGECON	xgecon_
#define XHECON	xhecon_
#define XSYSV	xsysv_
#define ZGESV	zgesv_
#define RPOCON	rpocon_
#define XPOTRF	xpotrf_
#define XPOCON	xpocon_
#define RSYCON	rsycon_
#define RGECON	rgecon_
#define RSBEV	rsbev_
#define XHBEV	xhbev_
#define RGELS	rgels_
#define XGELS	xgels_
#endif

void PROTO (DGEEQU, (int *m, int *n, REAL *a, int *lda,
		     REAL *r, REAL *c,
		     REAL *rowcnd, REAL *colcnd, REAL *amax, int *info));
void PROTO (DPOEQU, (int *n, REAL *a, int *lda,
		     REAL *s, REAL *scond, REAL *amax, int *info));
void PROTO (RGECON, (int *inorm, int *n, REAL * a, int *lda,
		     REAL * anorm, REAL * rcond,
		     REAL * work, int *iwork, int *info));
void PROTO (DGESV, (int *n, int *nrhs, REAL * a, int *lda,
		    int *ipiv, REAL * b, int *ldb, int *info));
void PROTO (RGESVD, (int *ijobu, int *ijobvt, int *m, int *n, REAL * a,
		     int *lda, REAL * s, REAL * u, int *ldu, REAL * vt,
		     int *ldvt, REAL * work, int *lwork, int *info));
void PROTO (DGETRF, (int *m, int *n, REAL * a, int *lda,
		     int *ipiv, int *info));
void PROTO (RGETRS, (int *itrans, int *n, int *nrhs, REAL * a, int *lda,
		     int *ipiv, REAL * b, int *ldb, int *info));

void PROTO (RGEEV, (int *icmpvl, int *icmpvr, int *n, REAL * a,
		    int *lda, REAL * wr, REAL * wi,
		    REAL * vl, int *ldvl, REAL * vr, int *ldvr,
		    REAL * work, int *lwork, int *info));
void PROTO (RGGEV, (int *ijobvl, int *ijobvr, int *n,
		    REAL * a, int *lda, REAL * b, int *ldb,
		    REAL * alphar, REAL * alphai, REAL * beta,
		    REAL * vl, int *ldvl, REAL * vr, int *ldvr,
		    REAL * work, int *lwork, int *info));
void PROTO (XGGEV, (int *ijobvl, int *ijobvr, int *n,
		    COMPLEX * a, int *lda, COMPLEX * b, int *ldb,
		    COMPLEX * alpha, COMPLEX * beta,
		    COMPLEX * vl, int *ldvl, COMPLEX * vr, int *ldvr,
		    COMPLEX * work, int *lwork, REAL *rwork, int *info));
void PROTO (RPOCON, (int *iuplo, int *n, REAL * a, int *lda, REAL * anorm,
		     REAL * rcond, REAL * work, int *iwork, int *info));
void PROTO (RPOSV, (int *iuplo, int *n, int *nrhs, REAL * a, int *lda,
		    REAL * b, int *ldb, int *info));
void PROTO (RPOTRF, (int *iuplo, int *n, REAL * a, int *lda, int *info));
void PROTO (RPOTRS, (int *iuplo, int *n, int *nrhs, REAL * a, int *lda,
		     REAL * b, int *ldb, int *info));
void PROTO (XPOTRS, (int *iuplo, int *n, int *nrhs, REAL * a, int *lda,
		     REAL * b, int *ldb, int *info));
void PROTO (RSYCON, (int *iuplo, int *n, REAL * a, int *lda, int *ipiv,
		     REAL * anorm, REAL * rcond, REAL * work, int *iwork,
		     int *info));
void PROTO (RSYEV, (int *icompa, int *iuplo, int *n, REAL * a, int *lda,
		    REAL * w, REAL * work, int *lwork, int *info));
void PROTO (RSYGV, (int *itype, int *ijobz, int *iuplo, int *n,
		    REAL * a, int *lda, REAL * b, int *ldb,
		    REAL * w, REAL * work, int *lwork, int *info));
void PROTO (XHEGV, (int *itype, int *ijobz, int *iuplo, int *n,
		    COMPLEX * a, int *lda, COMPLEX * b, int *ldb,
		    REAL *w, COMPLEX *work, int *lwork, REAL *rwork,
		    int *info));
void PROTO (RSYSV, (int *iuplo, int *n, int *nrhs, REAL * a,
		    int *lda, int *ipiv, REAL * b,
		    int *ldb, REAL * work, int *lwork, int *info));
void PROTO (RSYTRF, (int *iuplo, int *n, REAL * a, int *lda, int *ipiv,
		     REAL * work, int *lwork, int *info));
void PROTO (RSYTRS, (int *iuplo, int *n, int *nrhs, REAL * a, int *lda,
		     int *ipiv, REAL * b, int *ldb, int *info));

void PROTO (ZGEEQU, (int *m, int *n, COMPLEX *a, int *lda,
		     REAL *r, REAL *c,
		     REAL *rowcnd, REAL *colcnd, REAL *amax, int *info));
void PROTO (ZPOEQU, (int *n, COMPLEX *a, int *lda,
		     REAL *s, REAL *scond, REAL *amax, int *info));
void PROTO (XGEEV, (int *ijobvl, int *ijobvr, int *n, COMPLEX * a, int *lda,
		    COMPLEX * w, COMPLEX * vl, int *ldvl, COMPLEX * vr,
		    int *ldvr, COMPLEX * work, int *lwork, REAL * rwork,
		    int *info));
void PROTO (XGESVD, (int *ijobu, int *ijobvt, int *m, int *n, COMPLEX * a,
		     int *lda, REAL * s, COMPLEX * u, int *ldu, COMPLEX * vt,
		     int *ldvt, COMPLEX * work, int *lwork, REAL * rwork,
		     int *info));
void PROTO (XHEEV, (int *ijobz, int *iuplo, int *n, COMPLEX * a, int *lda,
		    REAL * w, COMPLEX * work, int *lwork, REAL * rwork,
		    int *info));
void PROTO (XHESV, (int *iuplo, int *n, int *nrhs, COMPLEX * a,
		    int *lda, int *ipiv, COMPLEX * b,
		    int *ldb, COMPLEX * work, int *lwork, int *info));
void PROTO (XPOCON, (int *iuplo, int *n, COMPLEX * a, int *lda, REAL * anorm,
		     REAL * rcond, COMPLEX * work, REAL * rwork, int *info));
void PROTO (XPOSV, (int *iuplo, int *n, int *nrhs, COMPLEX * a, int *lda,
		    COMPLEX * b, int *ldb, int *info));
void PROTO (XPOTRF, (int *iuplo, int *n, COMPLEX * a, int *lda, int *info));
void PROTO (XSYSV, (int *iuplo, int *n, int *nrhs, COMPLEX * a,
		    int *lda, int *ipiv, COMPLEX * b,
		    int *ldb, COMPLEX * work, int *lwork, int *info));

void PROTO (ZGESV, (int *n, int *nrhs, COMPLEX * a, int *lda, int *ipiv,
		    COMPLEX * b, int *ldb, int *info));

void PROTO (RSBEV, (int *ijobz, int *iuplo, int *n, int *kd, REAL * ab,
		    int *ldab, REAL * w, REAL * z, int *ldz, REAL * work,
		    int *info));
void PROTO (XHBEV, (int *ijobz, int *iuplo, int *n, int *kd, COMPLEX * ab,
		 int *ldab, REAL * w, COMPLEX * z, int *ldz, COMPLEX * work,
		    REAL * rwork, int *info));

void PROTO (XGECON, (int *inorm, int *n, REAL * a, int *lda,
		     REAL * anorm, REAL * rcond,
		     REAL * work, int *rwork, int *info));

void PROTO (XGETRS, (int *itrans, int *n, int *nrhs, REAL * a, int *lda,
		     int *ipiv, REAL * b, int *ldb, int *info));

void PROTO (XSYCON, (int *iuplo, int *n, REAL * a, int *lda, int *ipiv,
		     REAL * anorm, REAL * rcond, REAL * work,
		     int *info));
void PROTO (XHECON, (int *iuplo, int *n, REAL * a, int *lda, int *ipiv,
		     REAL * anorm, REAL * rcond, REAL * work,
		     int *info));

void PROTO (XSYTRF, (int *iuplo, int *n, REAL * a, int *lda, int *ipiv,
		     REAL * work, int *lwork, int *info));
void PROTO (XHETRF, (int *iuplo, int *n, REAL * a, int *lda, int *ipiv,
		     REAL * work, int *lwork, int *info));

void PROTO (XSYTRS, (int *iuplo, int *n, int *nrhs, REAL * a, int *lda,
		     int *ipiv, REAL * b, int *ldb, int *info));
void PROTO (XHETRS, (int *iuplo, int *n, int *nrhs, REAL * a, int *lda,
		     int *ipiv, REAL * b, int *ldb, int *info));

void PROTO (ZGETRF, (int *m, int *n, REAL * a, int *lda,
		     int *ipiv, int *info));

void PROTO (RGELS, (int *itrans, int *m, int *n, int *nrhs, REAL *a,
		    int *lda, REAL *b, int *ldb, void *work,
		    int *lwork, int *info));
void PROTO (XGELS, (int *itrans, int *m, int *n, int *nrhs, COMPLEX *a,
		    int *lda, COMPLEX *b, int *ldb, void *work,
		    int *lwork, int *info));

#endif /* LAPACK_H */
