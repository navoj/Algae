/*
   factor.c -- Triangular factorization.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: factor.c,v 1.6 2003/10/05 01:14:48 ksh Exp $";

#include "factor.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "cast.h"
#include "lapack.h"
#include "norm.h"
#include "bcs.h"
#include "sqrt.h"
#include "dense.h"
#include "full.h"

#if HAVE_LIBSUPERLU
#include "superlu.h"
#endif

static char ill_msg[] = "Potential for significant loss of accuracy.  The factored matrix is ill-conditioned; RCOND = %g.";

#define SMALL_RCOND 1e-8	/* RCOND smaller than this gets warning */

ENTITY *
bi_factor (ENTITY *a)
{
  /* Convert to matrix, if necessary, and call matrix_entity(). */

  return (factor_matrix ((MATRIX *) matrix_entity (a)));
}

#define RIT( t, v, n )	t = (TABLE *) replace_in_table (EAT(t), v, dup_char(n))
#define CS( c )		char_to_scalar (dup_char (c))

ENTITY *
factor_matrix (MATRIX *a)
{
  /*
   * Compute a triangular factorization of the matrix argument.
   */

  VECTOR *ipiv = NULL;
  MATRIX *fac = NULL;
  TABLE *t = NULL;
  int info, iuplo, lwork;
  void *work = NULL;
  void *rwork = NULL;
  REAL anorm, rcond;
  int *tmp_ia = NULL;
  int *tmp_ja = NULL;
  SCALAR *sn;
  SYMMETRY sym;

#if HAVE_LIBSUPERLU
  SuperMatrix * volatile A = NULL;
  SuperMatrix *AX = NULL;
  SuperMatrix * volatile AC = NULL;
  SuperMatrix *L = NULL;
  SuperMatrix *U = NULL;
  NCformat *v;
  int *perm_c = NULL;
  int *perm_r = NULL;
  int *etree = NULL;
  TABLE *Lt, *Ut;
  volatile int superlu_status_allocated = 0;
#endif

  EASSERT (a, matrix, 0);

  WITH_HANDLING
  {

    if (a->type == character)
      {
	fail ("Invalid \"character\" type for triangular factorization.");
	raise_exception ();
      }

    /* Punt if zero rows or columns. */

    if (a->nr == 0 || a->nc == 0)
      {
	fail ("Can't factor a matrix with zero %s.",
	      (a->nr == 0) ? "rows" : "columns");
	raise_exception ();
      }

    /* Convert integer to real. */

    if (a->type == integer)
      a = (MATRIX *) cast_matrix_integer_real ((MATRIX *) EAT (a));

    /* OK, do it. */

    t = (TABLE *) make_table ();

    switch (a->density)
      {

      case sparse_upper:
      case sparse:

#if HAVE_LIBSUPERLU

	{
	  int i, m, n, info, permc_spec;
	  char *refact = "N";
	  double diag_pivot_thresh, drop_tol, r_pivot_growth;
	  int panel_size, relax;

	  /* Use default parameters, for now. */

	  panel_size = 8;
	  relax = 8;
	  diag_pivot_thresh = 1.0;
	  drop_tol = 0.0;

	  StatInit (panel_size, relax);
	  superlu_status_allocated = 1;

	  /* Need full storage. */

	  if (a->density == sparse_upper)
	    a = (MATRIX *) full_matrix (EAT (a));

	  /* Can't be all zeros. */

	  if (!a->nn)
	    {
	      fail ("Matrix elements are all zero.");
	      raise_exception ();
	    }

	  A = CALLOC (1, sizeof (SuperMatrix));
	  v = CALLOC (1, sizeof (NCformat));

	  A->Stype = SLU_NC;
	  A->Mtype = SLU_GE;

	  /*
	   *  SuperLU wants compressed column storage, but we have it
	   *  in compressed row form.
	   */

	  n = A->nrow = a->nc;
	  m = A->ncol = a->nr;
	  A->Store = v;

	  v->nnz = a->nn;
	  v->nzval = dup_mem (a->a.ptr, a->nn * type_size[a->type]);

	  /* We're 1-based, SuperLU is 0-based. */

	  v->rowind = dup_mem (a->ja, a->nn * sizeof (int));
	  v->colptr = dup_mem (a->ia, (a->nr+1) * sizeof (int));
	  for (i=0; i<a->nn; i++) v->rowind[i]--;
	  for (i=0; i<a->nr+1; i++) v->colptr[i]--;

	  /*
	   * Determine column permutation.  Use MMD on A'+A if symmetric
	   * or hermitian, otherwise use COLAMD.
	   */

	  permc_spec = (a->symmetry == general) ? 3 : 2;
	  perm_c = MALLOC (A->ncol * sizeof(int));
	  get_perm_c (permc_spec, A, perm_c);

	  /* Apply the column ordering. */

	  AC = CALLOC (1, sizeof (SuperMatrix));

	  etree = MALLOC (A->ncol * sizeof(int));

	  /* sp_preorder swipes some of A's vectors! */

	  AX = CALLOC (1, sizeof (SuperMatrix));
	  AX->Stype = A->Stype;
	  AX->Dtype = A->Dtype;
	  AX->Mtype = A->Mtype;
	  AX->nrow = A->nrow;
	  AX->ncol = A->ncol;
	  AX->Store = CALLOC (1, sizeof (NCformat));
	  ((NCformat *)(AX->Store))->nnz = v->nnz;
	  ((NCformat *)(AX->Store))->nzval = 
	    dup_mem (v->nzval, v->nnz * type_size[a->type]);
	  ((NCformat *)(AX->Store))->rowind = 
	    dup_mem (v->rowind, a->nn * sizeof(int));
	  ((NCformat *)(AX->Store))->colptr = 
	    dup_mem (v->colptr, (a->nr+1) * sizeof(int));

	  sp_preorder (refact, AX, perm_c, etree, AC);

	  ((NCformat *)(AX->Store))->nzval = NULL;
	  ((NCformat *)(AX->Store))->rowind = NULL;
	  free_SuperLU (AX);

	  switch (a->type)
	    {
	    case real:

	      A->Dtype = SLU_D;

	      /* Compute LU factorization. */

	      L = CALLOC (1, sizeof (SuperMatrix));
	      U = CALLOC (1, sizeof (SuperMatrix));

	      perm_r = MALLOC (A->nrow * sizeof(int));

	      SIGINT_RAISE_ON ();
	      dgstrf (refact, AC, diag_pivot_thresh, drop_tol, relax,
		      panel_size, etree, NULL, 0, perm_r, perm_c,
		      L, U, &info);
	      SIGINT_RAISE_OFF ();

	      FREE (etree);

	      if (info)
		{
		  if (info < 0)
		    {
		      fail ("Bad argument, %s argument to dgstrf.", TH(-info));
		      raise_exception ();
		    }
		  else
		    {
		      if (info > A->ncol)
			{
			  fail ("Memory allocation failure in dgstrf.");
			  raise_exception ();
			}
		      else
			{
			  warn ("U[%d;%d] is exactly zero.  The factorization has been completed, but the factor U is singular.",
				info, info);

			  /*
			   * Compute the reciprocal pivot growth factor of the
			   * leading rank-deficient *info columns of A.
			   */

			  r_pivot_growth = dPivotGrowth(info, A, perm_c, L, U);
			  rcond = 0.0;
			}
		    }
		}
	      else
		{
		  r_pivot_growth = dPivotGrowth(A->ncol, A, perm_c, L, U);

		  /* Estimate the reciprocal of the condition number of A. */

		  anorm = dlangs ("I", A);
		  dgscon ("I", L, U, anorm, &rcond, &info);

		  if (info)
		    {
		      fail ("Bad argument, %s argument to dgscon.", TH(-info));
		      raise_exception ();
		    }

		  if (rcond < SMALL_RCOND) warn (ill_msg, rcond);
		}
	      /* Keep L and U matrices in tables. */

	      Lt = (TABLE *) SuperLU_to_table (L);
	      free_SuperLU (EAT(L));
	      RIT (t, EAT (Lt), "L");

	      Ut = (TABLE *) SuperLU_to_table (U);
	      free_SuperLU (EAT(U));
	      RIT (t, EAT (Ut), "U");

	      RIT (t, gift_wrap_vector (A->nrow, integer, EAT (perm_r)),
		   "perm_r");
	      RIT (t, gift_wrap_vector (A->ncol, integer, EAT (perm_c)),
		   "perm_c");
	      RIT (t, real_to_scalar (rcond), "RCOND");
	      RIT (t, real_to_scalar (r_pivot_growth), "RPG");

	      break;

	    case complex:

	      A->Dtype = SLU_Z;

	      /* Compute LU factorization. */

	      L = CALLOC (1, sizeof (SuperMatrix));
	      U = CALLOC (1, sizeof (SuperMatrix));

	      perm_r = MALLOC (A->nrow * sizeof(int));

	      SIGINT_RAISE_ON ();
	      zgstrf (refact, AC, diag_pivot_thresh, drop_tol, relax,
		      panel_size, etree, NULL, 0, perm_r, perm_c,
		      L, U, &info);
	      SIGINT_RAISE_OFF ();

	      FREE (etree);

	      if (info)
		{
		  if (info < 0)
		    {
		      fail ("Bad argument, %s argument to zgstrf.", TH(-info));
		      raise_exception ();
		    }
		  else
		    {
		      if (info > A->ncol)
			{
			  fail ("Memory allocation failure in zgstrf.");
			  raise_exception ();
			}
		      else
			{
			  warn ("U[%d;%d] is exactly zero.  The factorization has been completed, but the factor U is singular.",
				info, info);

			  /*
			   * Compute the reciprocal pivot growth factor of the
			   * leading rank-deficient *info columns of A.
			   */

			  r_pivot_growth = zPivotGrowth(info, A, perm_c, L, U);
			  rcond = 0.0;
			}
		    }
		}
	      else
		{
		  r_pivot_growth = zPivotGrowth(A->ncol, A, perm_c, L, U);

		  /* Estimate the reciprocal of the condition number of A. */

		  anorm = zlangs ("I", A);
		  zgscon ("I", L, U, anorm, &rcond, &info);

		  if (info)
		    {
		      fail ("Bad argument, %s argument to zgscon.", TH(-info));
		      raise_exception ();
		    }

		  if (rcond < SMALL_RCOND) warn (ill_msg, rcond);
		}
	      /* Keep L and U matrices in tables. */

	      Lt = (TABLE *) SuperLU_to_table (L);
	      free_SuperLU (EAT(L));
	      RIT (t, EAT (Lt), "L");

	      Ut = (TABLE *) SuperLU_to_table (U);
	      free_SuperLU (EAT(U));
	      RIT (t, EAT (Ut), "U");

	      RIT (t, gift_wrap_vector (A->nrow, integer, EAT (perm_r)),
		   "perm_r");
	      RIT (t, gift_wrap_vector (A->ncol, integer, EAT (perm_c)),
		   "perm_c");
	      RIT (t, real_to_scalar (rcond), "RCOND");
	      RIT (t, real_to_scalar (r_pivot_growth), "RPG");

	      break;

	    default:

	      BAD_TYPE (a->type);
	      raise_exception ();
	    }

	  /* SuperLU factors have method="SuperLU" */

	  RIT (t, CS ("SuperLU"), "method");

	  /* Hang on to labels. */

	  RIT (t, a->rid ? copy_entity (a->rid) : NULL, "rid");
	  RIT (t, a->cid ? copy_entity (a->cid) : NULL, "cid");
	}
	break;
#else
	/*
	   * If we don't have BCSLIB, we have to convert to dense.
	   * Someday it would be nice to use LAPACK's band routines
	   * in that case.
	 */

#if HAVE_LIBBCSEXT
	t = (TABLE *) ((a->type == real) ?
		       factor_bcs_real (EAT (a), 0.1) :
		       factor_bcs_complex (EAT (a), 0.1));
	break;
#else
	a = (MATRIX *) dense_matrix (EAT (a));

	/* fall through */
#endif /* HAVE_LIBBCSEXT */
#endif /* HAVE_LIBSUPERLU */

      case dense:

	switch (a->type)
	  {

	  case real:

	    switch (a->symmetry)
	      {

	      case general:

		/* Compute 1-norm for later use. */

		sn = (SCALAR *) cast_scalar ((SCALAR *)
					     norm_matrix ((MATRIX *)
							  copy_matrix (a),
							  one_norm),
					     real);
		anorm = sn->v.real;
		delete_scalar (sn);

		a = (MATRIX *) dup_matrix ((MATRIX *) EAT (a));
		a->symmetry = general;
		ipiv = (VECTOR *) form_vector ((a->nr < a->nc) ?
					       a->nr : a->nc,
					       integer, dense);

		SIGINT_RAISE_ON ();
		DGETRF (&a->nr, &a->nc, a->a.real, &a->nr,
			ipiv->a.integer, &info);
		SIGINT_RAISE_OFF ();

		if (info)
		  {
		    if (info < 0)
		      {
			wipeout ("Illegal value in the %d%s argument to DGETRF.",
				 -info, TH (-info));
		      }
		    else
		      {
			rcond = 0.0;
			warn ("Singular factor encountered in real LU decomposition.  U(%d,%d) is exactly zero.",
			      info, info);
		      }
		  }
		else
		  {

		    int inorm = '1';

		    /* Estimate the condition number. */

		    work = E_MALLOC (4 * a->nr, real);
		    rwork = E_MALLOC (a->nr, integer);

		    SIGINT_RAISE_ON ();
		    RGECON (&inorm, &a->nr, a->a.real, &a->nr,
			    &anorm, &rcond,
			    work, rwork, &info);
		    SIGINT_RAISE_OFF ();

		    FREE (work);
		    work = NULL;
		    FREE (rwork);
		    rwork = NULL;

		    if (info)
		      {
			if (info < 0)
			  {
			    wipeout ("Illegal value in the %d%s argument to DGECON.",
				     -info, TH (-info));
			  }
			else
			  {
			    wipeout ("Bad return from DGECON.");
			  }
		      }

		    if (rcond < SMALL_RCOND)
		      warn (ill_msg, rcond);

		  }

		t = (TABLE *) replace_in_table (EAT (t),
						real_to_scalar (rcond),
						dup_char ("RCOND"));
		t = (TABLE *) replace_in_table ((TABLE *) EAT (t),
						EAT (a),
						dup_char ("LU"));
		t = (TABLE *) replace_in_table ((TABLE *) EAT (t),
						EAT (ipiv),
						dup_char ("IPIV"));
		break;

	      case symmetric:

		a = (MATRIX *) dup_matrix ((MATRIX *) EAT (a));
		a->symmetry = general;

		/* Compute 1-norm for later use. */

		sn = (SCALAR *) cast_scalar ((SCALAR *)
					     norm_matrix ((MATRIX *)
							  copy_matrix (a),
							  one_norm),
					     real);
		anorm = sn->v.real;
		delete_scalar (sn);

		ipiv = (VECTOR *) form_vector (a->nr, integer, dense);
		lwork = 66 * a->nr;
		work = E_MALLOC (lwork, real);

		iuplo = 'L';
		SIGINT_RAISE_ON ();
		RSYTRF (&iuplo, &a->nr, a->a.real, &a->nr, ipiv->a.integer,
			(REAL *) work, &lwork, &info);
		SIGINT_RAISE_OFF ();

		if (info)
		  {
		    if (info < 0)
		      {
			wipeout ("Illegal value in the %d%s argument to DSYTRF.",
				 -info, TH (-info));
		      }
		    else
		      {
			rcond = 0.0;
			warn ("Singular factor encountered in real LDL' decomposition.  D(%d,%d) is exactly zero.",
			      info, info);
		      }

		  }
		else
		  {

		    /* Estimate the condition number. */

		    work = E_MALLOC (2 * a->nr, real);
		    rwork = E_MALLOC (a->nr, integer);

		    SIGINT_RAISE_ON ();
		    RSYCON (&iuplo, &a->nr, a->a.real, &a->nr,
			    ipiv->a.integer, &anorm, &rcond,
			    work, rwork, &info);
		    SIGINT_RAISE_OFF ();

		    FREE (work);
		    work = NULL;
		    FREE (rwork);
		    rwork = NULL;

		    if (info)
		      {
			if (info < 0)
			  {
			    wipeout ("Illegal value in the %d%s argument to DSYCON.",
				     -info, TH (-info));
			  }
			else
			  {
			    wipeout ("Bad return from DSYCON.");
			  }
		      }

		    if (rcond < SMALL_RCOND)
		      warn (ill_msg, rcond);

		  }

		t = (TABLE *) replace_in_table (EAT (t),
						real_to_scalar (rcond),
						dup_char ("RCOND"));
		t = (TABLE *) replace_in_table (EAT (t), EAT (ipiv),
						dup_char ("IPIV"));
		t = (TABLE *) replace_in_table (EAT (t), EAT (a),
						dup_char ("LD"));

		break;

	      default:
		BAD_SYMMETRY (a->symmetry);
		raise_exception ();

	      }
	    break;

	  case complex:

	    switch (a->symmetry)
	      {

	      case general:

		/* Compute 1-norm for later use. */

		sn = (SCALAR *) cast_scalar ((SCALAR *)
					     norm_matrix ((MATRIX *)
							  copy_matrix (a),
							  one_norm),
					     real);
		anorm = sn->v.real;
		delete_scalar (sn);

		a = (MATRIX *) dup_matrix ((MATRIX *) EAT (a));
		a->symmetry = general;
		ipiv = (VECTOR *) form_vector ((a->nr < a->nc) ?
					       a->nr : a->nc,
					       integer, dense);

		SIGINT_RAISE_ON ();
		ZGETRF (&a->nr, &a->nc, a->a.real, &a->nr,
			ipiv->a.integer, &info);
		SIGINT_RAISE_OFF ();

		if (info)
		  {
		    if (info < 0)
		      {
			wipeout ("Illegal value in the %d%s argument to ZGETRF.",
				 -info, TH (-info));
		      }
		    else
		      {
			rcond = 0.0;
			warn ("Singular factor encountered in complex LU decomposition.  U(%d,%d) is exactly zero.",
			      info, info);
		      }
		  }
		else
		  {

		    int inorm = '1';

		    /* Estimate the condition number. */

		    work = E_MALLOC (4 * a->nr, real);
		    rwork = E_MALLOC (2 * a->nr, real);

		    SIGINT_RAISE_ON ();
		    XGECON (&inorm, &a->nr, a->a.real, &a->nr,
			    &anorm, &rcond,
			    work, rwork, &info);
		    SIGINT_RAISE_OFF ();

		    FREE (work);
		    work = NULL;
		    FREE (rwork);
		    rwork = NULL;

		    if (info)
		      {
			if (info < 0)
			  {
			    wipeout ("Illegal value in the %d%s argument to ZGECON.",
				     -info, TH (-info));
			  }
			else
			  {
			    wipeout ("Bad return from ZGECON.");
			  }
		      }

		    if (rcond < SMALL_RCOND)
		      warn (ill_msg, rcond);

		  }

		t = (TABLE *) replace_in_table (EAT (t),
						real_to_scalar (rcond),
						dup_char ("RCOND"));
		t = (TABLE *) replace_in_table ((TABLE *) EAT (t),
						EAT (a),
						dup_char ("LU"));
		t = (TABLE *) replace_in_table ((TABLE *) EAT (t),
						EAT (ipiv),
						dup_char ("IPIV"));
		break;

	      case symmetric:
	      case hermitian:

		sym = a->symmetry;
		a = (MATRIX *) dup_matrix ((MATRIX *) EAT (a));
		a->symmetry = general;

		/* Compute 1-norm for later use. */

		sn = (SCALAR *) cast_scalar ((SCALAR *)
					     norm_matrix ((MATRIX *)
							  copy_matrix (a),
							  one_norm),
					     real);
		anorm = sn->v.real;
		delete_scalar (sn);

		ipiv = (VECTOR *) form_vector (a->nr, integer, dense);
		lwork = 66 * a->nr;
		work = E_MALLOC (lwork, complex);

		iuplo = 'L';
		SIGINT_RAISE_ON ();
		if (sym == symmetric)
		  XSYTRF (&iuplo, &a->nr, a->a.real, &a->nr,
			  ipiv->a.integer,
			  (REAL *) work, &lwork, &info);
		else
		  XHETRF (&iuplo, &a->nr, a->a.real, &a->nr,
			  ipiv->a.integer,
			  (REAL *) work, &lwork, &info);
		SIGINT_RAISE_OFF ();

		if (info)
		  {
		    if (info < 0)
		      {
			wipeout ("Illegal value in the %d%s argument to Z__TRF.",
				 -info, TH (-info));
		      }
		    else
		      {
			rcond = 0.0;
			warn ("Singular factor encountered in complex LDL' decomposition.  D(%d,%d) is exactly zero.",
			      info, info);
		      }

		  }
		else
		  {

		    /* Estimate the condition number. */

		    work = E_MALLOC (2 * a->nr, complex);

		    SIGINT_RAISE_ON ();
		    if (sym == symmetric)
		      XSYCON (&iuplo, &a->nr, a->a.real, &a->nr,
			      ipiv->a.integer, &anorm, &rcond,
			      work, &info);
		    else
		      XHECON (&iuplo, &a->nr, a->a.real, &a->nr,
			      ipiv->a.integer, &anorm, &rcond,
			      work, &info);
		    SIGINT_RAISE_OFF ();

		    FREE (work);
		    work = NULL;

		    if (info)
		      {
			if (info < 0)
			  {
			    wipeout ("Illegal value in the %d%s argument to Z__CON.",
				     -info, TH (-info));
			  }
			else
			  {
			    wipeout ("Bad return from Z__CON.");
			  }
		      }

		    if (rcond < SMALL_RCOND)
		      warn (ill_msg, rcond);

		  }

		t = (TABLE *) replace_in_table (EAT (t),
						real_to_scalar (rcond),
						dup_char ("RCOND"));
		t = (TABLE *) replace_in_table (EAT (t), EAT (ipiv),
						dup_char ("IPIV"));
		t = (TABLE *) replace_in_table (EAT (t), EAT (a),
						dup_char (sym == symmetric ?
							  "LD" : "LDH"));

		break;

	      default:
		BAD_SYMMETRY (a->symmetry);
		raise_exception ();

	      }
	    break;

	  default:
	    BAD_TYPE (a->type);
	    raise_exception ();
	  }
	break;

      default:
	BAD_DENSITY (a->density);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_2_matrices (a, fac);
    delete_vector (ipiv);
    delete_table (t);
    TFREE (work);
    TFREE (rwork);
    TFREE (tmp_ia);
    TFREE (tmp_ja);
#if HAVE_LIBSUPERLU
    TFREE (perm_c);
    TFREE (perm_r);
    TFREE (etree);
    free_SuperLU (A);
    free_SuperLU (AC);
    free_SuperLU (L);
    free_SuperLU (U);
    if (superlu_status_allocated) StatFree ();
#endif
  }
  END_EXCEPTION;

#if HAVE_LIBSUPERLU
  if (superlu_status_allocated) StatFree ();
  free_SuperLU (A);
  free_SuperLU (AC);
#endif
  delete_matrix (a);
  delete_vector (ipiv);
  return (ENT (t));
}

#if HAVE_LIBBCSEXT

ENTITY *
factor_bcs_real (a, pvttol)
     MATRIX *a;
     REAL pvttol;		/* pivot tolerance */
{
  int mtxtyp;			/* symmetry */
  int maxzer = 0;		/* relaxation parameter */
  int msglvl = 0;		/* message level */
  int ipu = 0;			/* Fortran output unit */
  int sqfile = -1;		/* Fortran unit */
  int wafil1 = -1;		/* Fortran unit */
  int wafil2 = -1;		/* Fortran unit */

  int i, nhold, needs, needmn, ier, inrtia[3];
  REAL rcond;
  REAL *hold = NULL;
  TABLE *t = NULL;

  /* Turn off error reporting in BCSLIB. */

  {
    int ityp = 1;
    int ival = 0;
    HHERRS (&ityp, &ival);
  }

  WITH_HANDLING
  {
    switch (a->symmetry)
      {
      case general:

	mtxtyp = 'u';
	a = (MATRIX *) transpose_matrix (EAT (a));
	assert (a->density == sparse);
	break;

      case symmetric:

	mtxtyp = 's';
	a = (MATRIX *) sparse_matrix (EAT (a));
	assert (a->density == sparse_upper);
	break;

      default:

	BAD_SYMMETRY (a->symmetry);
	raise_exception ();
      }

    /* A guess at the needed working space. */

    nhold = (200 + 2 * a->nn + 10 * a->nr) * BCS_PADDING;
    hold = MALLOC (nhold * sizeof (REAL));

    /* Initialize the sparse solver.  */

    while (HRSLIN (&a->nr, &mtxtyp, &msglvl, &ipu,
		   &sqfile, &wafil1, &wafil2,
		   hold, &nhold, &ier), ier)
      {
	if (ier != -101)
	  wipeout ("Error return (%d) from HDSLIN.", ier);

	nhold += nhold;
	hold = REALLOC (hold, nhold * sizeof (REAL));
      }

    /* Specify structure. */

    if (a->ia)
      {
	HDSLIM (a->ia, a->ja, hold, &nhold, &needs, &ier);
      }
    else
      {
	int *ia = MALLOC ((a->nr + 1) * sizeof (int));
	for (i = 0; i <= a->nr; i++)
	  ia[i] = 1;
	HDSLIM (ia, ia, hold, &nhold, &needs, &ier);
	FREE (ia);
      }
    if (ier)
      wipeout ("Error return (%d) from HDSLIM.", ier);
    BUMP_WORK (needs, REAL, hold, nhold);

    /* Reorder. */

    SIGINT_RAISE_ON ();
    HDSLOR (&maxzer, hold, &nhold, &needs, &ier);
    if (ier)
      wipeout ("Error return (%d) from HDSLOR.", ier);
    SIGINT_RAISE_OFF ();
    BUMP_WORK (needs, REAL, hold, nhold);

    /* Symbolic factorization. */

    SIGINT_RAISE_ON ();
    HDSLSF (hold, &nhold, &needs, &needmn, &ier);
    if (ier)
      wipeout ("Error return (%d) from HDSLSF.", ier);
    SIGINT_RAISE_OFF ();
    BUMP_WORK (needs, REAL, hold, nhold);

    /* Input values. */

    if (a->ia)
      {
	HDSLVM (a->ia, a->ja, a->a.real, hold, &nhold, &ier);
	if (ier)
	  wipeout ("Error return (%d) from HDSLSF.", ier);
      }

    if (a->d.real)
      {
	for (i = 1; i <= a->nr; i++)
	  {
	    HDSLV1 (&i, &i, a->d.real + i - 1, hold, &nhold, &ier);
	    if (ier)
	      wipeout ("Error return (%d) from HDSLV1.", ier);
	  }
      }

    /* Numeric factorization */

    do
      {
	SIGINT_RAISE_ON ();
	HDSLCO (&pvttol, hold, &nhold, &rcond, inrtia, &ier);
	SIGINT_RAISE_OFF ();

	switch (ier)
	  {
	  case 0:
	    break;

	  case -500:
	  case -504:
	  case -505:
	  case -506:

	    wipeout ("Error return (%d) from HDSLCO.", ier);

	  case -503:

	    fail ("Zero pivot encountered; matrix is singular.");
	    raise_exception ();

	  default:

	    nhold += nhold;
	    hold = REALLOC (hold, nhold * sizeof (REAL));

	  }
      }
    while (ier);

    rcond = 1.0 / rcond;
    if (rcond < SMALL_RCOND)
      warn (ill_msg, rcond);

    t = (TABLE *) make_table ();
    t = (TABLE *) replace_in_table (EAT (t), real_to_scalar (rcond),
				    dup_char ("RCOND"));
    t = (TABLE *) replace_in_table (EAT (t), int_to_scalar (a->nr),
				    dup_char ("N"));
    t = (TABLE *) replace_in_table (EAT (t),
				    gift_wrap_vector (nhold, real,
						      EAT (hold)),
				    dup_char ("HOLD"));
  }
  ON_EXCEPTION
  {
    delete_matrix (a);
    delete_table (t);
    TFREE (hold);
  }
  END_EXCEPTION;

  delete_matrix (a);
  TFREE (hold);

  return ENT (t);
}

ENTITY *
factor_bcs_complex (a, pvttol)
     MATRIX *a;
     REAL pvttol;		/* pivot tolerance */
{
  int mtxtyp;			/* symmetry */
  int maxzer = 0;		/* relaxation parameter */
  int msglvl = 0;		/* message level */
  int ipu = 0;			/* Fortran output unit */
  int sqfile = -1;		/* Fortran unit */
  int wafil1 = -1;		/* Fortran unit */
  int wafil2 = -1;		/* Fortran unit */

  int i, nhold, needs, needmn, ier, inrtia[3];
  REAL rcond;
  COMPLEX *hold = NULL;
  TABLE *t = NULL;

  /* Turn off error reporting in BCSLIB. */

  {
    int ityp = 1;
    int ival = 0;
    HHERRS (&ityp, &ival);
  }

  WITH_HANDLING
  {
    switch (a->symmetry)
      {
      case general:

	mtxtyp = 'u';
	a = (MATRIX *) transpose_matrix (EAT (a));
	assert (a->density == sparse);
	break;

      case symmetric:

	mtxtyp = 's';
	a = (MATRIX *) sparse_matrix (EAT (a));
	assert (a->density == sparse_upper);
	break;

      case hermitian:

	mtxtyp = 'h';
	a = (MATRIX *) sparse_matrix (EAT (a));
	a = (MATRIX *) conjugate_matrix (EAT (a));	/* for lower triangle */
	assert (a->density == sparse_upper);
	break;

      default:

	BAD_SYMMETRY (a->symmetry);
	raise_exception ();
      }

    /* A guess at the needed working space. */

    nhold = (200 + (4 * a->nn + 19 * a->nr) / 4) * BCS_PADDING;
    hold = MALLOC (nhold * sizeof (COMPLEX));

    /* Initialize the sparse solver.  */

    while (HXSLIN (&a->nr, &mtxtyp, &msglvl, &ipu,
		   &sqfile, &wafil1, &wafil2,
		   hold, &nhold, &ier), ier)
      {
	if (ier != -101)
	  wipeout ("Error return (%d) from HZSLIN.", ier);

	nhold += nhold;
	hold = REALLOC (hold, nhold * sizeof (COMPLEX));
      }

    /* Specify structure. */

    if (a->nn)
      {
	HZSLIM (a->ia, a->ja, hold, &nhold, &needs, &ier);
      }
    else
      {
	int *ia = MALLOC ((a->nr + 1) * sizeof (int));
	for (i = 0; i <= a->nr; i++)
	  ia[i] = 1;
	HZSLIM (ia, ia, hold, &nhold, &needs, &ier);
	FREE (ia);
      }
    if (ier)
      wipeout ("Error return (%d) from HZSLIM.", ier);
    BUMP_WORK (needs, COMPLEX, hold, nhold);


    /* Reorder. */

    SIGINT_RAISE_ON ();
    HZSLOR (&maxzer, hold, &nhold, &needs, &ier);
    if (ier)
      wipeout ("Error return (%d) from HZSLOR.", ier);
    SIGINT_RAISE_OFF ();
    BUMP_WORK (needs, COMPLEX, hold, nhold);

    /* Symbolic factorization. */

    SIGINT_RAISE_ON ();
    HZSLSF (hold, &nhold, &needs, &needmn, &ier);
    if (ier)
      wipeout ("Error return (%d) from HZSLSF.", ier);
    SIGINT_RAISE_OFF ();
    BUMP_WORK (needs, COMPLEX, hold, nhold);

    /* Input values. */

    if (a->ia)
      {
	HZSLVM (a->ia, a->ja, a->a.complex, hold, &nhold, &ier);
	if (ier)
	  wipeout ("Error return (%d) from HZSLSF.", ier);
      }

    if (a->d.complex)
      {
	for (i = 1; i <= a->nr; i++)
	  {
	    HZSLV1 (&i, &i, a->d.complex + i - 1, hold, &nhold, &ier);
	    if (ier)
	      wipeout ("Error return (%d) from HZSLSF.", ier);
	  }
      }

    /* Numeric factorization */

    do
      {
	SIGINT_RAISE_ON ();
	HZSLCO (&pvttol, hold, &nhold, &rcond, inrtia, &ier);
	SIGINT_RAISE_OFF ();

	switch (ier)
	  {
	  case 0:
	    break;

	  case -500:
	  case -504:
	  case -505:
	  case -506:

	    wipeout ("Error return (%d) from HZSLCO.", ier);

	  case -503:

	    fail ("Zero pivot encountered; matrix is singular.");
	    raise_exception ();

	  default:

	    nhold += nhold;
	    hold = REALLOC (hold, nhold * sizeof (COMPLEX));

	  }
      }
    while (ier);

    rcond = 1.0 / rcond;
    if (rcond < SMALL_RCOND)
      warn (ill_msg, rcond);

    t = (TABLE *) make_table ();
    t = (TABLE *) replace_in_table (EAT (t), real_to_scalar (rcond),
				    dup_char ("RCOND"));
    t = (TABLE *) replace_in_table (EAT (t), int_to_scalar (a->nr),
				    dup_char ("N"));
    t = (TABLE *) replace_in_table (EAT (t),
				    gift_wrap_vector (nhold, complex,
						      EAT (hold)),
				    dup_char ("HOLD"));
  }
  ON_EXCEPTION
  {
    delete_matrix (a);
    delete_table (t);
    TFREE (hold);
  }
  END_EXCEPTION;

  delete_matrix (a);
  TFREE (hold);

  return ENT (t);
}
#endif

ENTITY *
bi_chol (ENTITY *a)
{
  /*
   * This routine computes the Cholesky factorization for symmetric and
   * hermitian positive definite matrices.
   */

  return (chol_matrix ((MATRIX *) matrix_entity (a)));
}

ENTITY *
chol_matrix (MATRIX *a)
{
  MATRIX *fac = NULL;
  TABLE *t = NULL;
  REAL anorm, rcond;
  int i, j, info, iuplo;
  void *work = NULL;
  void *rwork = NULL;
  int *tmp_ia = NULL;
  int *tmp_ja = NULL;
  SCALAR *sn;

  EASSERT (a, matrix, 0);

  WITH_HANDLING
  {
    /* Punt if zero rows or columns. */

    if (a->nr == 0 || a->nc == 0)
      {
	fail ("Can't factor a matrix with zero %s.",
	      (a->nr == 0) ? "rows" : "columns");
	raise_exception ();
      }

    /* Check type. */

    if (a->type != real)
      {
	if (a->type == integer)
	  {
	    a = (MATRIX *) cast_matrix_integer_real (EAT (a));
	  }
	else if (a->type == character)
	  {
	    fail ("Invalid character type for Cholesky factorization.");
	    raise_exception ();
	  }
      }

    /* Check symmetry. */

    if (a->type == real && a->symmetry != symmetric ||
	a->type == complex && a->symmetry != hermitian)
      {
	fail ("Matrix not %s in \"chol\".",
	      (a->type == real) ? "symmetric" : "hermitian");
	raise_exception ();
      }

    t = (TABLE *) make_table ();

    switch (a->density)
      {
      case sparse_upper:

	/* If it's diagonal, handle it here.  Otherwise fall through. */

	if (a->nn == 0)
	  {
	    REAL tmp;

	    switch (a->type)
	      {
	      case real:

		if (a->d.real == NULL)
		  {
		    fail ("Matrix is singular.");
		    raise_exception ();
		  }

		/* Compute condition number. */

		anorm = 0.0;
		rcond = 0.0;
		for (i = 0; i < a->nr; i++)
		  {
		    tmp = a->d.real[i] * a->d.real[i];
		    anorm += tmp;
		    rcond += 1.0 / tmp;
		  }
		rcond = 1.0 / sqrt (anorm * rcond);

		/* For diagonal case, L=sqrt(A). */

		a = (MATRIX *) dup_matrix ((MATRIX *) EAT (a));
		for (i = 0; i < a->nr; i++)
		  a->d.real[i] = sqrt (a->d.real[i]);

		break;

	      case complex:

		if (a->d.complex == NULL)
		  {
		    fail ("Matrix is singular.");
		    raise_exception ();
		  }

		/* Compute condition number. */

		anorm = 0.0;
		rcond = 0.0;
		for (i = 0; i < a->nr; i++)
		  {
		    tmp = (a->d.complex[i].real * a->d.complex[i].real +
			   a->d.complex[i].imag * a->d.complex[i].imag);
		    anorm += tmp;
		    rcond += 1.0 / tmp;
		  }
		rcond = 1.0 / sqrt (anorm * rcond);

		/* For diagonal case, L=sqrt(A). */

		a = (MATRIX *) dup_matrix ((MATRIX *) EAT (a));
		for (i = 0; i < a->nr; i++)
		  a->d.complex[i] = sqrt_complex (a->d.complex[i]);

		break;

	      default:
		BAD_TYPE (a->type);
		raise_exception ();
	      }

	    if (rcond < SMALL_RCOND)
	      warn (ill_msg, rcond);

	    t = (TABLE *) replace_in_table (EAT (t), EAT (a),
					    dup_char ("L"));
	    t = (TABLE *) replace_in_table (EAT (t),
					    real_to_scalar (rcond),
					    dup_char ("RCOND"));
	    break;
	  }			/* fall through */

	a = (MATRIX *) full_matrix (EAT (a));

      case sparse:

	/* If it's empty, forget it. */

	if (a->nn == 0)
	  {
	    fail ("Matrix is singular.");
	    raise_exception ();
	  }

	/*
	 * If we don't have BCSLIB, we have to convert to dense.
	 * Someday it would be nice to use LAPACK's band routines
	 * in that case.
	 */

#if HAVE_LIBBCSEXT
	/*
	 * The only difference between this and `factor' is that
	 * the value of `pvttol' is zero.
	 */

	t = (TABLE *) ((a->type == real) ?
		       factor_bcs_real (EAT (a), 0.0) :
		       factor_bcs_complex (EAT (a), 0.0));
	break;
#else
	a = (MATRIX *) dense_matrix (EAT (a));

	/* fall through */
#endif

      case dense:

	switch (a->type)
	  {
	  case real:

	    /* Has to be symmetric for Cholesky method. */

	    if (a->symmetry != symmetric)
	      {
		fail ("Real argument to \"chol\" is not symmetric.");
		raise_exception ();
	      }

	    /* Compute 1-norm for later use. */

	    sn = (SCALAR *) cast_scalar ((SCALAR *)
					 norm_matrix ((MATRIX *)
						      copy_matrix (a),
						      one_norm),
					 real);
	    anorm = sn->v.real;
	    delete_scalar (sn);

	    /* Compute Cholesky factorization. */

	    a = (MATRIX *) dup_matrix (EAT (a));
	    a->symmetry = general;

	    iuplo = 'L';
	    SIGINT_RAISE_ON ();
	    RPOTRF (&iuplo, &a->nr, a->a.real, &a->nr, &info);
	    SIGINT_RAISE_OFF ();

	    if (info)
	      {
		if (info < 0)
		  {
		    wipeout ("Illegal value in the %d%s argument to DPOTRF.",
			     -info, TH (-info));
		  }
		else
		  {
		    fail ("The leading minor of order %d is not positive definite.", info);
		    raise_exception ();
		  }
	      }

	    /* Estimate the condition number. */

	    work = E_MALLOC (3 * a->nr, real);
	    rwork = E_MALLOC (a->nr, integer);

	    SIGINT_RAISE_ON ();
	    RPOCON (&iuplo, &a->nr, a->a.real, &a->nr, &anorm, &rcond,
		    work, rwork, &info);
	    SIGINT_RAISE_OFF ();

	    FREE (work);
	    work = NULL;
	    FREE (rwork);
	    rwork = NULL;

	    if (info)
	      {
		if (info < 0)
		  {
		    wipeout ("Illegal value in the %d%s argument to DPOCON.",
			     -info, TH (-info));
		  }
		else
		  {
		    wipeout ("Bad return from DPOCON.");
		  }
	      }

	    /*
	     * Warn if condition number is large.  (`rcond' is the
	     * reciprocal of the condition number.)
	     */

	    if (rcond < SMALL_RCOND)
	      warn (ill_msg, rcond);

	    /* Zero the (unreferenced) upper triangle. */

	    for (i = 0; i < a->nr - 1; i++)
	      {
		for (j = i + 1; j < a->nc; j++)
		  {
		    a->a.real[i + a->nr * j] = 0.0;
		  }
	      }

	    /* Return factors. */

	    t = (TABLE *) replace_in_table (EAT (t), EAT (a),
					    dup_char ("L"));
	    t = (TABLE *) replace_in_table (EAT (t),
					    real_to_scalar (rcond),
					    dup_char ("RCOND"));
	    break;

	  case complex:

	    /* Has to be Hermitian for Cholesky method. */

	    if (a->symmetry != hermitian)
	      {
		fail ("Complex argument to \"chol\" is not hermitian.");
		raise_exception ();
	      }

	    /* Compute 1-norm for later use. */

	    sn = (SCALAR *) cast_scalar ((SCALAR *)
					 norm_matrix ((MATRIX *)
						      copy_matrix (a),
						      one_norm),
					 real);
	    anorm = sn->v.real;
	    delete_scalar (sn);

	    /* Compute Cholesky factorization. */

	    a = (MATRIX *) dup_matrix (EAT (a));
	    a->symmetry = general;

	    iuplo = 'L';
	    SIGINT_RAISE_ON ();
	    XPOTRF (&iuplo, &a->nr, a->a.complex, &a->nr, &info);
	    SIGINT_RAISE_OFF ();

	    if (info)
	      {
		if (info < 0)
		  {
		    wipeout ("Illegal value in the %d%s argument to ZPOTRF.",
			     -info, TH (-info));
		  }
		else
		  {
		    fail ("The leading minor of order %d is not positive definite.", info);
		    raise_exception ();
		  }
	      }

	    /* Estimate the condition number. */

	    work = E_MALLOC (2 * a->nr, complex);
	    rwork = E_MALLOC (a->nr, real);

	    SIGINT_RAISE_ON ();
	    XPOCON (&iuplo, &a->nr, a->a.complex, &a->nr, &anorm, &rcond,
		    work, rwork, &info);
	    SIGINT_RAISE_OFF ();

	    FREE (work);
	    work = NULL;
	    FREE (rwork);
	    rwork = NULL;

	    if (info)
	      {
		if (info < 0)
		  {
		    wipeout ("Illegal value in the %d%s argument to ZPOCON.",
			     -info, TH (-info));
		  }
		else
		  {
		    wipeout ("Bad return from ZPOCON.");
		  }
	      }

	    /*
	     * Warn if condition number is large.  (`rcond' is the
	     * reciprocal of the condition number.)
	     */

	    if (rcond < SMALL_RCOND)
	      warn (ill_msg, rcond);

	    /* Zero the (unreferenced) upper triangle. */

	    for (i = 0; i < a->nr - 1; i++)
	      {
		for (j = i + 1; j < a->nc; j++)
		  {
		    a->a.complex[i + a->nr * j].real = 0.0;
		    a->a.complex[i + a->nr * j].imag = 0.0;
		  }
	      }

	    /* Return factors. */

	    t = (TABLE *) replace_in_table (EAT (t), EAT (a),
					    dup_char ("L"));
	    t = (TABLE *) replace_in_table (EAT (t),
					    real_to_scalar (rcond),
					    dup_char ("RCOND"));
	    break;

	  default:
	    BAD_TYPE (a->type);
	    raise_exception ();
	  }
	break;

      default:
	BAD_DENSITY (a->density);
	raise_exception ();

      }
  }
  ON_EXCEPTION
  {
    delete_2_matrices (a, fac);
    delete_table (t);
    TFREE (work);
    TFREE (rwork);
    TFREE (tmp_ia);
    TFREE (tmp_ja);
  }
  END_EXCEPTION;

  delete_matrix (a);

  return (ENT (t));
}
