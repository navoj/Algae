/*
   superlu.h -- Prototypes for `superlu.c'.

   Copyright (C) 2002  K. Scott Hunziker.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: superlu.h,v 1.2 2002/07/24 04:36:28 ksh Exp $ */

#ifndef SUPERLU_H
#define SUPERLU_H

#include "entity.h"
#include "../superlu/supermatrix.h"

extern void PROTO (superlu_abort_and_exit, (char *msg));

extern void PROTO (get_perm_c, (int ispec, SuperMatrix *A, int *perm_c));
extern void PROTO (free_SuperLU, (SuperMatrix *A));
extern void PROTO (sp_preorder, (char *refact,  SuperMatrix *A, int *perm_c, 
				 int *etree, SuperMatrix *AC));
extern double PROTO (dPivotGrowth, (int ncols, SuperMatrix *A, int *perm_c, 
				    SuperMatrix *L, SuperMatrix *U));
extern double PROTO (zPivotGrowth, (int ncols, SuperMatrix *A, int *perm_c, 
				    SuperMatrix *L, SuperMatrix *U));
extern ENTITY *PROTO (SuperLU_to_table, (SuperMatrix *A));
extern SuperMatrix *PROTO (table_to_SuperLU, (TABLE *t));
extern void PROTO (dgsequ, (SuperMatrix *A, double *r, double *c,
			    double *rowcnd, double *colcnd, double *amax,
			    int *info));
extern void PROTO (zgsequ, (SuperMatrix *A, double *r, double *c,
			    double *rowcnd, double *colcnd, double *amax,
			    int *info));
extern void PROTO (dgstrf, (char *refact, SuperMatrix *A,
			    double diag_pivot_thresh, double drop_tol,
			    int relax, int panel_size, int *etree,
			    void *work, int lwork, int *perm_r, int *perm_c,
			    SuperMatrix *L, SuperMatrix *U, int *info));
extern void PROTO (zgstrf, (char *refact, SuperMatrix *A,
			    double diag_pivot_thresh, 
			    double drop_tol, int relax, int panel_size,
			    int *etree, void *work, int lwork, int *perm_r,
			    int *perm_c, SuperMatrix *L, SuperMatrix *U,
			    int *info));
extern void PROTO (dgscon, (char *norm, SuperMatrix *L, SuperMatrix *U,
			    double anorm, double *rcond, int *info));
extern void PROTO (zgscon, (char *norm, SuperMatrix *L, SuperMatrix *U,
			    double anorm, double *rcond, int *info));
extern double PROTO (dlangs, (char *norm, SuperMatrix *A));
extern double PROTO (zlangs, (char *norm, SuperMatrix *A));
extern void PROTO (StatInit, (int panel_size, int relax));
extern void StatFree();
extern void PROTO (dgstrs, (char *trans, SuperMatrix *L, SuperMatrix *U,
			    int *perm_r, int *perm_c, SuperMatrix *B,
			    int *info));
extern void PROTO (zgstrs, (char *trans, SuperMatrix *L, SuperMatrix *U,
			    int *perm_r, int *perm_c, SuperMatrix *B,
			    int *info));

#endif /* SUPERLU_H */
