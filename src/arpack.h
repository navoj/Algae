/*
   arpack.h -- Prototypes for the ARPACK package.

   Copyright (C) 2002  K. Scott Hunziker.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: arpack.h.in,v 1.2 2002/07/31 20:38:43 ksh Exp $ */

#ifndef ARPACK_H
#define ARPACK_H  1

#include "entity.h"

#if !NO_FORTRAN_DEFS
#define RSAUPD rsaupd_
#define RSEUPD rseupd_
#define RNAUPD rnaupd_
#define RNEUPD rneupd_
#define XNAUPD xnaupd_
#define XNEUPD xneupd_
#endif

extern void PROTO (RSAUPD, (int *ido, int *ibmat, int *n, int *iwhich,
			    int *nev, REAL *tol, REAL *resid, int *ncv,
			    REAL *v, int *ldv, int *iparam, int *ipntr,
			    REAL *workd, REAL *workl, int *lworkl, int *info));

extern void PROTO (RSEUPD, (int *irvec, int *ihow, int *isel, REAL *d,
			    REAL *z, int *ldz, REAL *sigma, int *ibmat, 
			    int *n, int *iwhich, int *nev, REAL *tol,
			    REAL *resid, int *ncv, REAL *v, int *ldv,
			    int *iparam, int *ipntr, REAL *workd, REAL *workl,
			    int *lworkl, int *info));

extern void PROTO (RNAUPD, (int *ido, int *ibmat, int *n, int *iwhich,
			    int *nev, REAL *tol, REAL *resid, int *ncv,
			    REAL *v, int *ldv, int *iparam, int *ipntr,
			    REAL *workd, REAL *workl, int *lworkl, int *info));

extern void PROTO (RNEUPD, (int *irvec, int *ihow, int *isel, REAL *dr,
			    REAL *di, REAL *z, int *ldz, REAL *sigmar,
			    REAL *sigmai, REAL *workev, int *ibmat, 
			    int *n, int *iwhich, int *nev, REAL *tol,
			    REAL *resid, int *ncv, REAL *v, int *ldv,
			    int *iparam, int *ipntr, REAL *workd, REAL *workl,
			    int *lworkl, int *info));

extern void PROTO (XNAUPD, (int *ido, int *ibmat, int *n, int *iwhich,
			    int *nev, REAL *tol, COMPLEX *resid, int *ncv,
			    COMPLEX *v, int *ldv, int *iparam, int *ipntr,
			    COMPLEX *workd, COMPLEX *workl, int *lworkl,
			    REAL *rwork, int *info));

extern void PROTO (XNEUPD, (int *irvec, int *ihow, int *isel, COMPLEX *d,
			    COMPLEX *z, int *ldz, COMPLEX *sigma,
			    COMPLEX *workev, int *ibmat, 
			    int *n, int *iwhich, int *nev, REAL *tol,
			    COMPLEX *resid, int *ncv, COMPLEX *v, int *ldv,
			    int *iparam, int *ipntr, COMPLEX *workd,
			    COMPLEX *workl, int *lworkl, REAL *rwork,
			    int *info));

extern ENTITY *PROTO (bi_iram, (ENTITY *dim, ENTITY *op_func,
		      ENTITY *b_func, ENTITY *params, ENTITY *options));
extern ENTITY *PROTO (iram_ds_entity, (ENTITY *dim, ENTITY *op_func,
		      ENTITY *b_func, ENTITY *params, ENTITY *options));
extern ENTITY *PROTO (iram_dn_entity, (ENTITY *dim, ENTITY *op_func,
		      ENTITY *b_func, ENTITY *params, ENTITY *options));
extern ENTITY *PROTO (iram_zn_entity, (ENTITY *dim, ENTITY *op_func,
		      ENTITY *b_func, ENTITY *params, ENTITY *options));

#endif /* ARPACK_H */
