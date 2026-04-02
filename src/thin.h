/* src/thin.h.  Generated from thin.h.in by configure. */

/*
   thin.h -- Prototypes for `thin.f'.

   Copyright (C) 1994-2001  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: thin.h.in,v 1.3 2001/10/12 14:18:20 ksh Exp $ */

#ifndef THIN_H
#define THIN_H	1

#if !NO_FORTRAN_DEFS
#define DGSADD dgsadd_
#define DGSMUL dgsmul_
#define DGSSUB dgssub_
#define DGSTRN dgstrn_
#define IGSADD igsadd_
#define IGSMUL igsmul_
#define IGSSUB igssub_
#define IGSTRN igstrn_
#define XGSADD xgsadd_
#define XGSMUL xgsmul_
#define XGSTRN xgstrn_
#define ZGSADD zgsadd_
#define ZGSMUL zgsmul_
#define ZGSSUB zgssub_
#define ZGSTRN zgstrn_
#define DPSSOL dpssol_
#define DPSFAC dpsfac_
#define XPSFAC xpsfac_
#define ISSTRF isstrf_
#define DSSTRF dsstrf_
#define ZHSTRF zhstrf_
#endif

void PROTO (DGSADD, (int *ia, int *ja, REAL * an, int *ib,
		     int *jb, REAL * bn, int *n, int *m, int *ic, int *jc,
		     REAL * cn, REAL * x));
void PROTO (DGSMUL, (int *ia, int *ja, REAL * an, int *ib, int *jb,
		   REAL * bn, int *na, int *mb, int *ic, int *jc, REAL * cn,
		     REAL * x));
void PROTO (DGSSUB, (int *ia, int *ja, REAL * an, int *ib,
		     int *jb, REAL * bn, int *n, int *m, int *ic, int *jc,
		     REAL * cn, REAL * x));
void PROTO (DGSTRN, (int *ia, int *ja, REAL * an, int *n,
		     int *m, int *iat, int *jat, REAL * ant));
void PROTO (IGSADD, (int *ia, int *ja, int *an, int *ib,
		     int *jb, int *bn, int *n, int *m, int *ic, int *jc,
		     int *cn, int *x));
void PROTO (IGSMUL, (int *ia, int *ja, int *an, int *ib,
		     int *jb, int *bn, int *na, int *mb, int *ic, int *jc,
		     int *cn, int *x));
void PROTO (IGSSUB, (int *ia, int *ja, int *an, int *ib, int
		     *jb, int *bn, int *n, int *m, int *ic, int *jc, int *cn,
		     int *x));
void PROTO (IGSTRN, (int *ia, int *ja, int *an, int *n, int *m,
		     int *iat, int *jat, int *ant));
void PROTO (XGSADD, (int *ia, int *ja, int *ib, int *jb, int *n,
		     int *m, int *ic, int *jc, int *ix));
void PROTO (XGSMUL, (int *ia, int *ja, int *ib, int *jb, int *na,
		     int *mb, int *ic, int *jc, int *maxjc, int *ix));
void PROTO (XGSTRN, (int *ia, int *ja, int *n, int *m, int *iat,
		     int *jat));

void PROTO (ZGSADD, (int *ia, int *ja, COMPLEX * an, int *ib,
		     int *jb, COMPLEX * bn, int *n, int *m, int *ic, int *jc,
		     COMPLEX * cn, COMPLEX * x));
void PROTO (ZGSMUL, (int *ia, int *ja, COMPLEX * an, int *ib, int *jb,
		     COMPLEX * bn, int *na, int *mb, int *ic, int *jc,
		     COMPLEX * cn, COMPLEX * x));
void PROTO (ZGSSUB, (int *ia, int *ja, COMPLEX * an, int *ib,
		     int *jb, COMPLEX * bn, int *n, int *m, int *ic, int *jc,
		     COMPLEX * cn, COMPLEX * x));
void PROTO (ZGSTRN, (int *ia, int *ja, COMPLEX * an, int *n,
		     int *m, int *iat, int *jat, COMPLEX * ant));
void PROTO (DPSSOL, (int *iu, int *ju, REAL * un, REAL * di,
		     int *n, REAL * b, REAL * x));

void PROTO (DPSFAC, (int *ia, int *ja, REAL * an, REAL * ad, int *n,
		int *iu, int *ju, REAL * un, REAL * di, int *ip, int *iup));
void PROTO (XPSFAC, (int *ia, int *ja, int *n, int *nn,
		     int *iu, int *ju, int *ip));

void PROTO (ISSTRF, (int *n, int *m, int *ia, int *ja,
		     int * an, int * ad, int * p, int * c, int * w));
void PROTO (DSSTRF, (int *n, int *m, int *ia, int *ja,
		     REAL * an, REAL * ad, REAL * p, REAL * c, REAL * w));
void PROTO (ZHSTRF, (int *n, int *m, int *ia, int *ja,
	COMPLEX * an, COMPLEX * ad, COMPLEX * p, COMPLEX * c, COMPLEX * w));

#endif /* THIN_H */
