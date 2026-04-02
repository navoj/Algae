/* src/bcs.h.  Generated from bcs.h.in by configure. */

/*
   bcs.h -- Interface for BCSLIB routines.

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: bcs.h.in,v 1.3 1997/02/21 09:53:52 ksh Exp $ */

#ifndef BCS_H
#define BCS_H	1

#include "algae.h"

#if HAVE_LIBBCSEXT

#if NO_FORTRAN_DEFS

#if SINGLE_BCSLIB
#define HZSLIM HCSLIM
#define HZSLOR HCSLOR
#define HZSLSF HCSLSF
#define HZSLVM HCSLVM
#define HZSLV1 HCSLV1
#define HZSLCO HCSLCO
#define HZSLSL HCSLSL
#define HDSLIM HSSLIM
#define HDSLOR HSSLOR
#define HDSLSF HSSLSF
#define HDSLVM HSSLVM
#define HDSLV1 HSSLV1
#define HDSLCO HSSLCO
#define HDSLSL HSSLSL
#endif

#else

#define HHERRS hherrs_
#define HXSLIN hxslin_
#define HZSLIM hzslim_
#define HZSLOR hzslor_
#define HZSLSF hzslsf_
#define HZSLVM hzslvm_
#define HZSLV1 hzslv1_
#define HZSLCO hzslco_
#define HZSLSL hzslsl_
#define HRSLIN hrslin_
#define HDSLIM hdslim_
#define HDSLOR hdslor_
#define HDSLSF hdslsf_
#define HDSLVM hdslvm_
#define HDSLV1 hdslv1_
#define HDSLCO hdslco_
#define HDSLSL hdslsl_

#endif

void PROTO (HHERRS, (int *ityp, int *ival));
void PROTO (HXSLIN, (int *neqns, int *mtxtyp, int *msglvl, int *ipu,
		     int *sqfile, int *wafil1, int *wafil2, COMPLEX * hold,
		     int *nhold, int *ier));
void PROTO (HZSLIM, (int *colstr, int *rowind, COMPLEX * hold,
		     int *nhold, int *needs, int *ier));
void PROTO (HZSLOR, (int *maxzer, COMPLEX * hold,
		     int *nhold, int *needs, int *ier));
void PROTO (HZSLSF, (COMPLEX * hold, int *nhold,
		     int *needs, int *needmn, int *ier));
void PROTO (HZSLVM, (int *colstr, int *rowind, COMPLEX * values,
		     COMPLEX * hold, int *nhold, int *ier));
void PROTO (HZSLV1, (int *irow, int *jcol, COMPLEX * value,
		     COMPLEX * hold, int *nhold, int *ier));
void PROTO (HZSLCO, (REAL * pvttol, COMPLEX * hold, int *nhold,
		     REAL * cndnum, int *inrtia, int *ier));
void PROTO (HZSLSL, (int *nrhs, COMPLEX * rhs, int *ldrhs,
		     COMPLEX * hold, int *nhold, int *ier));

void PROTO (HRSLIN, (int *neqns, int *mtxtyp, int *msglvl, int *ipu,
		     int *sqfile, int *wafil1, int *wafil2, REAL * hold,
		     int *nhold, int *ier));
void PROTO (HDSLIM, (int *colstr, int *rowind, REAL * hold,
		     int *nhold, int *needs, int *ier));
void PROTO (HDSLOR, (int *maxzer, REAL * hold,
		     int *nhold, int *needs, int *ier));
void PROTO (HDSLSF, (REAL * hold, int *nhold,
		     int *needs, int *needmn, int *ier));
void PROTO (HDSLVM, (int *colstr, int *rowind, REAL * values,
		     REAL * hold, int *nhold, int *ier));
void PROTO (HDSLV1, (int *irow, int *jcol, REAL * value,
		     REAL * hold, int *nhold, int *ier));
void PROTO (HDSLCO, (REAL * pvttol, REAL * hold, int *nhold,
		     REAL * cndnum, int *inrtia, int *ier));
void PROTO (HDSLSL, (int *nrhs, REAL * rhs, int *ldrhs,
		     REAL * hold, int *nhold, int *ier));

#define CALL_BCS(f,a,b)	f( a, b )

#define BCS_PADDING 1.25	/* padding for working storage */
#define BUMP_WORK(needs,type,hold,nhold) \
	if ( needs > nhold ) \
	{ \
            nhold = needs * BCS_PADDING; \
            hold = REALLOC( hold, nhold * sizeof(type) ); \
        }

#else

#define CALL_BCS(f,a,b) \
  ( detour( "Not linked with BCSLIB.  Sparse solvers are not available." ), \
    raise_exception(), NULL )

#endif

#endif /* BCS_H */
