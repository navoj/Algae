/*
   npsol.h -- Prototypes for `npsol.c'.

   Copyright (C) 1994-2002  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: npsol.h,v 1.3 2002/10/01 01:21:48 ksh Exp $ */

#ifndef NPSOL_H
#define NPSOL_H	1

#include "entity.h"

extern ENTITY *PROTO (bi_npsol,
		        (int n, ENTITY * objective, ENTITY * start,
			 ENTITY * constraints, ENTITY * options,
			 ENTITY * state));

extern ENTITY *PROTO (npsol_entity,
		        (FUNCTION * objf, FUNCTION * objgrd, VECTOR * start,
		      MATRIX * side, MATRIX * lin_coef, MATRIX * lin_bounds,
		   FUNCTION * conf, FUNCTION * congrd, MATRIX * nlin_bounds,
			 TABLE * options, VECTOR * istate, VECTOR * clamda,
		     MATRIX * r, ENTITY * obj_params, ENTITY * con_params));

extern void PROTO (set_npsol_options, (TABLE * options));

extern int PROTO (NPSOL,
		    (int *n, int *nclin, int *ncnln, int *nrowa,
		     int *nrowuj, int *nrowr, REAL * a, REAL * bl, REAL * bu,
		   int (*confun) (int *mode, int *ncnln, int *n, int *nrowj,
				int *needc, REAL * x, REAL * c, REAL * cjac,
				  int *nstate),
		     int (*objfun) (int *mode, int *n, REAL * x, REAL * objf,
				    REAL * objgrd, int *nstate),
		 int *inform, int *iter, int *istate, REAL * c, REAL * ujac,
	       REAL * clamda, REAL * objf, REAL * ugrad, REAL * r, REAL * x,
		     int *iw, int *leniw, REAL * w, int *lenw));

extern int PROTO (NPOPTN, (char *string, long int string_len));
extern int PROTO (NPOPTE, (void));

#endif /* NPSOL_H */
