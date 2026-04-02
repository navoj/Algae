/*
   solve.c -- Solve linear equations.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: solve.c,v 1.7 2003/10/05 01:14:48 ksh Exp $";

#include "solve.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "lapack.h"
#include "cast.h"
#include "dense.h"
#include "thin.h"
#include "binop.h"
#include "blas.h"
#include "transpose.h"
#include "bcs.h"
#include "test.h"

#if HAVE_LIBSUPERLU
#include "superlu.h"
#endif

ENTITY *
bi_backsub (ENTITY * volatile a, ENTITY *b)
{
  volatile CLASS b_class;

  EASSERT (a, 0, 0);
  EASSERT (b, 0, 0);

  WITH_HANDLING
  {
    if (a->class != table)
      {
	fail ("First argument to \"backsub\" must be a table.");
	raise_exception ();
      }

    b_class = b->class;
    b = matrix_entity (EAT (b));
    if (b_class == vector)
      b = transpose_matrix (EAT (b));
  }
  ON_EXCEPTION
  {
    delete_2_entities (a, b);
  }
  END_EXCEPTION;

  a = backsub_table ((TABLE *) a, (MATRIX *) b);

  return (b_class == vector) ? matrix_to_vector ((MATRIX *) a) : a;
}

#define RIT( t, v, n )	t = (TABLE *) replace_in_table (EAT(t), v, dup_char(n))
#define SIT( t, n )	search_in_table ((TABLE *) copy_table (t), dup_char(n))
#define CS( c )		char_to_scalar (dup_char (c))

ENTITY *
backsub_table (TABLE *a, MATRIX *b)
{
  /*
   * This routine solves a linear system for which a triangular
   * factorization has already been performed.  The table `a'
   * contains these factors.
   */

  MATRIX *fac = NULL;
  VECTOR *hold = NULL;
  VECTOR *ipiv = NULL;
  MATRIX *x = NULL;
#if HAVE_LIBBCSEXT
  MATRIX *xr = NULL;
  MATRIX *xi = NULL;
#endif
  SCALAR *rcond = NULL;
  SCALAR *method = NULL;
  SCALAR *neq = NULL;
  int info;
  int from_superlu = 0;

#if HAVE_LIBSUPERLU
  SuperMatrix * volatile L = NULL;
  SuperMatrix * volatile U = NULL;
  SuperMatrix B;
  DNformat Store;
  TABLE *Ut = NULL;
  TABLE *Lt = NULL;
  VECTOR *perm_r = NULL;
  VECTOR *perm_c = NULL;
  MATRIX * volatile x2 = NULL;
  ENTITY * volatile rid = NULL;
  ENTITY *volatile cid = NULL;
  volatile int superlu_status_allocated = 0;

  static char *invalid_msg = "Invalid SuperLU factor.";
#define RQ( e )		if (!(e)) { fail (invalid_msg); raise_exception (); }

#endif

  EASSERT (a, table, 0);
  EASSERT (b, matrix, 0);

  WITH_HANDLING
  {
    /* Check arguments. */

    b = (MATRIX *) dense_matrix ((MATRIX *) EAT (b));

    /* If it came from SuperLU, "method" member will say so. */

    method = (SCALAR *) SIT (a, "method");
    if (method)
      {
	method = (SCALAR *) bi_scalar (1, EAT (method));
	method = (SCALAR *) cast_scalar (EAT (method), character);
	from_superlu = !strcmp (method->v.character, "SuperLU");
	delete_scalar (method);
	method = NULL;
      }

#if HAVE_LIBSUPERLU
    if (from_superlu)
      {
	/* Use default parameters, for now. */

	StatInit (8, 8);
	superlu_status_allocated = 1;

	/* The factor is actually of the transpose of the matrix. */

	rcond = (SCALAR *) SIT (a, "RCOND");
	if (rcond)
	  {
	    rcond = (SCALAR *) bi_scalar (1, EAT (rcond));
	    if (!test_scalar (EAT (rcond)))
	      {
		fail ("Singular matrix indicated by RCOND.");
		raise_exception ();
	      }
	  }

	Ut = (TABLE *) SIT (a, "U");
	if (Ut)
	  {
	    RQ (((ENTITY *)Ut)->class == table);
	    U = table_to_SuperLU (EAT (Ut));
	    RQ (U->Stype == SLU_NC);
	    RQ (U->Mtype == SLU_TRU);
	  }
	else
	  {
	    fail ("Invalid SuperLU table; no U table.");
	    raise_exception ();
	  }

	Lt = (TABLE *) SIT (a, "L");
	if (Lt)
	  {
	    RQ (((ENTITY *)Lt)->class == table)
	    L = table_to_SuperLU (EAT(Lt));
	    RQ (L->Stype == SLU_SC);
	    RQ (L->Mtype == SLU_TRLU);
	    RQ (L->Dtype == U->Dtype);
	    RQ (L->nrow == U->nrow && L->ncol == U->ncol);
	  }
	else
	  {
	    fail ("Invalid SuperLU table; no L table.");
	    raise_exception ();
	  }

	perm_r = (VECTOR *) SIT (a, "perm_r");
	if (perm_r)
	  {
	    perm_r = (VECTOR *) vector_entity (EAT (perm_r));
	    RQ (perm_r->type == integer);
	    RQ (perm_r->density == dense);
	    RQ (perm_r->ne == L->nrow);
	  }
	else
	  {
	    fail ("Invalid SuperLU table; no perm_r table.");
	    raise_exception ();
	  }

	perm_c = (VECTOR *) SIT (a, "perm_c");
	if (perm_c)
	  {
	    perm_c = (VECTOR *) vector_entity (EAT (perm_c));
	    RQ (perm_c->type == integer);
	    RQ (perm_c->density == dense);
	    RQ (perm_c->ne == L->ncol);
	  }
	else
	  {
	    fail ("Invalid SuperLU table; no perm_c table.");
	    raise_exception ();
	  }

	if (L->nrow != b->nr)
	  {
	    fail ("Coefficient matrix has %d column%s, RHS has %d row%s.",
		  L->ncol, PLURAL (L->ncol), b->nr, PLURAL (b->nr));
	    raise_exception ();
	  }

	rid = SIT (a, "rid");
	cid = SIT (a, "cid");

	if (!MATCH_VECTORS (rid, b->rid))
	  {
	    fail ("Labels don't match.");
	    raise_exception ();
	  }

	if (L->Dtype == SLU_Z)
	  {
	    x = (MATRIX *) cast_matrix (EAT (b), complex);
	    x = (MATRIX *) dup_matrix (EAT (x));
	    assert (x->density == dense);
	    x->symmetry = general;

	    if (x->nc)
	      {
		B.Stype = SLU_DN;
		B.Dtype = SLU_Z;
		B.Mtype = SLU_GE;
		B.nrow = x->nr;
		B.ncol = x->nc;
		B.Store = &Store;
		Store.lda = x->nr;
		Store.nzval = x->a.complex;

		SIGINT_RAISE_ON ();
		zgstrs ("T", L, U, perm_r->a.integer, perm_c->a.integer,
			&B, &info);
		SIGINT_RAISE_OFF ();

		if (info)
		  {
		    fail ("Invalid argument to zgstrs.");
		    raise_exception ();
		  }
	      }
	  }
	else
	  {
	    if (b->type == complex)
	      {
		int i;

		/* We have a complex RHS, but real factor. */

		x = (MATRIX *) dup_matrix (EAT (b));
		assert (x->density == dense);
		x->symmetry = general;

		if (x->nc)
		  {
		    x2 = (MATRIX *) cast_matrix_complex_real (
					(MATRIX *) copy_matrix (x));

		    B.Stype = SLU_DN;
		    B.Dtype = SLU_D;
		    B.Mtype = SLU_GE;
		    B.nrow = x2->nr;
		    B.ncol = x2->nc;
		    B.Store = &Store;
		    Store.lda = x2->nr;
		    Store.nzval = x2->a.real;

		    SIGINT_RAISE_ON ();
		    dgstrs ("T", L, U, perm_r->a.integer, perm_c->a.integer,
			    &B, &info);
		    SIGINT_RAISE_OFF ();

		    if (info)
		      {
			fail ("Invalid argument to dgstrs.");
			raise_exception ();
		      }

		    for (i=0; i<x->nn; i++)
		      {
			x->a.complex[i].real = x2->a.real[i];
			x2->a.real[i] = x->a.complex[i].imag;
		      }

		    B.Stype = SLU_DN;
		    B.Dtype = SLU_D;
		    B.Mtype = SLU_GE;
		    B.nrow = x2->nr;
		    B.ncol = x2->nc;
		    B.Store = &Store;
		    Store.lda = x2->nr;
		    Store.nzval = x2->a.real;

		    SIGINT_RAISE_ON ();
		    dgstrs ("T", L, U, perm_r->a.integer, perm_c->a.integer,
			    &B, &info);
		    SIGINT_RAISE_OFF ();

		    if (info)
		      {
			fail ("Invalid argument to dgstrs.");
			raise_exception ();
		      }

		    for (i=0; i<x->nn; i++)
		      x->a.complex[i].imag = x2->a.real[i];

		    delete_matrix (x2);
		    x2 = NULL;
		  }
	      }
	    else
	      {
		x = (MATRIX *) cast_matrix (EAT (b), real);
		x = (MATRIX *) dup_matrix (EAT (x));
		assert (x->density == dense);
		x->symmetry = general;

		if (x->nc)
		  {
		    B.Stype = SLU_DN;
		    B.Dtype = SLU_D;
		    B.Mtype = SLU_GE;
		    B.nrow = x->nr;
		    B.ncol = x->nc;
		    B.Store = &Store;
		    Store.lda = x->nr;
		    Store.nzval = x->a.real;

		    SIGINT_RAISE_ON ();
		    dgstrs ("T", L, U, perm_r->a.integer, perm_c->a.integer,
			    &B, &info);
		    SIGINT_RAISE_OFF ();

		    if (info)
		      {
			fail ("Invalid argument to dgstrs.");
			raise_exception ();
		      }
		  }
	      }
	  }

	if (x->rid) delete_entity (x->rid);
	if (cid) x->rid = copy_entity (cid);
      }

    else
#endif
      if (IN_TABLE (a, "LU"))	/* Came from _GETRF */
      {
	int itrans = 'N';

	rcond = (SCALAR *) search_in_table ((TABLE *) copy_table (a),
					    dup_char ("RCOND"));
	if (rcond)
	  {
	    rcond = (SCALAR *) bi_scalar (1, EAT (rcond));
	    if (!test_scalar (EAT (rcond)))
	      {
		fail ("Singular matrix indicated by RCOND.");
		raise_exception ();
	      }
	  }

	fac = (MATRIX *) search_in_table ((TABLE *) copy_table (a),
					  dup_char ("LU"));
	fac = (MATRIX *) matrix_entity (EAT (fac));
	fac = (MATRIX *) dense_matrix ((MATRIX *) EAT (fac));

	if (fac->nr != fac->nc)
	  {
	    fail ("LU matrix is not square.");
	    raise_exception ();
	  }

	if (fac->nc != b->nr)
	  {
	    fail ("Coefficient matrix has order %d, but right hand side has %d row%s.",
		  fac->nc, b->nr, PLURAL (b->nr));
	    raise_exception ();
	  }

	if (!MATCH_VECTORS (fac->rid, b->rid))
	  {
	    fail ("Labels don't match.");
	    raise_exception ();
	  }

	ipiv = (VECTOR *) search_in_table ((TABLE *) copy_table (a),
					   dup_char ("IPIV"));
	ipiv = (VECTOR *) vector_entity (EAT (ipiv));
	ipiv = (VECTOR *) dense_vector ((VECTOR *) EAT (ipiv));

	if (ipiv->type != integer)
	  {
	    fail ("IPIV has type %s---should be integer.");
	    raise_exception ();
	  }

	if (ipiv->ne != fac->nr)
	  {
	    fail ("LU has %d row%s, but IPIV has %d element%s.",
		  fac->nr, PLURAL (fac->nr), ipiv->ne, PLURAL (ipiv->ne));
	    raise_exception ();
	  }

	AUTO_CAST_MATRIX (fac, b);
	x = (MATRIX *) dup_matrix (EAT (b));
	assert (x->density == dense);
	x->symmetry = general;

	if (x->rid) delete_entity (x->rid);
	if (fac->cid) x->rid = copy_entity (fac->cid);

	switch (fac->type)
	  {
	  case real:
	    SIGINT_RAISE_ON ();
	    RGETRS (&itrans, &fac->nr, &x->nc, fac->a.real,
		    &fac->nr, ipiv->a.integer,
		    x->a.real, &x->nr, &info);
	    SIGINT_RAISE_OFF ();
	    if (info)
	      {
		wipeout ("Illegal value in the %d%s argument to DGETRS.",
			 -info, TH (-info));
	      }
	    break;
	  case complex:
	    SIGINT_RAISE_ON ();
	    XGETRS (&itrans, &fac->nr, &x->nc, fac->a.real,
		    &fac->nr, ipiv->a.integer,
		    x->a.real, &x->nr, &info);
	    SIGINT_RAISE_OFF ();
	    if (info)
	      {
		wipeout ("Illegal value in the %d%s argument to ZGETRS.",
			 -info, TH (-info));
	      }
	    break;
	  default:
	    fail ("LU has invalid %s type.", type_string[fac->type]);
	    raise_exception ();
	  }
      }
    else if (IN_TABLE (a, "L"))	/* Came from _POTRF */
      {
	int iuplo = 'L';

	fac = (MATRIX *) search_in_table ((TABLE *) copy_table (a),
					  dup_char ("L"));
	fac = (MATRIX *) matrix_entity (EAT (fac));
	fac = (MATRIX *) dense_matrix ((MATRIX *) EAT (fac));

	if (fac->nr != fac->nc)
	  {
	    fail ("L matrix is not square.");
	    raise_exception ();
	  }

	if (fac->nc != b->nr)
	  {
	    fail ("Coefficient matrix has order %d, but right hand side has %d row%s.",
		  fac->nc, b->nr, PLURAL (b->nr));
	    raise_exception ();
	  }

	if (!MATCH_VECTORS (fac->rid, b->rid))
	  {
	    fail ("Labels don't match.");
	    raise_exception ();
	  }

	AUTO_CAST_MATRIX (fac, b);
	x = (MATRIX *) dup_matrix (EAT (b));
	x->symmetry = general;

	if (x->rid) delete_entity (x->rid);
	if (fac->cid) x->rid = copy_entity (fac->cid);

	switch (fac->type)
	  {
	  case real:

	    SIGINT_RAISE_ON ();
	    RPOTRS (&iuplo, &fac->nr, &x->nc, fac->a.real, &fac->nr,
		    x->a.real, &x->nr, &info);
	    SIGINT_RAISE_OFF ();
	    if (info)
	      {
		wipeout ("Illegal value in the %d%s argument to DPOTRS.",
			 -info, TH (-info));
	      }
	    break;

	  case complex:

	    SIGINT_RAISE_ON ();
	    XPOTRS (&iuplo, &fac->nr, &x->nc, fac->a.real, &fac->nr,
		    x->a.real, &x->nr, &info);
	    SIGINT_RAISE_OFF ();
	    if (info)
	      wipeout ("Illegal value in the %d%s argument to ZPOTRS.",
		       -info, TH (-info));
	    break;

	  default:

	    fail ("L has invalid %s type.", type_string[fac->type]);
	    raise_exception ();
	  }

      }
    else if (IN_TABLE (a, "LD") || IN_TABLE (a, "LDH"))
      {
	/* Came from _SYTRF or ZHETRF */

	SYMMETRY sym = IN_TABLE (a, "LD") ? symmetric : hermitian;
	int iuplo = 'L';

	rcond = (SCALAR *) search_in_table ((TABLE *) copy_table (a),
					    dup_char ("RCOND"));
	if (rcond)
	  {
	    rcond = (SCALAR *) bi_scalar (1, EAT (rcond));
	    if (!test_scalar (EAT (rcond)))
	      {
		fail ("Singular matrix indicated by RCOND.");
		raise_exception ();
	      }
	  }

	fac = (MATRIX *)
	  search_in_table ((TABLE *) copy_table (a),
			   dup_char (sym == symmetric ? "LD" : "LDH"));
	fac = (MATRIX *) matrix_entity (EAT (fac));
	fac = (MATRIX *) dense_matrix ((MATRIX *) EAT (fac));

	if (fac->nr != fac->nc)
	  {
	    fail ("LD or LDH matrix is not square.");
	    raise_exception ();
	  }

	if (fac->nc != b->nr)
	  {
	    fail ("Coefficient matrix has order %d, but right hand side has %d row%s.",
		  fac->nc, b->nr, PLURAL (b->nr));
	    raise_exception ();
	  }

	if (!MATCH_VECTORS (fac->rid, b->rid))
	  {
	    fail ("Labels don't match.");
	    raise_exception ();
	  }

	ipiv = (VECTOR *) search_in_table ((TABLE *) copy_table (a),
					   dup_char ("IPIV"));
	ipiv = (VECTOR *) vector_entity (EAT (ipiv));
	ipiv = (VECTOR *) dense_vector ((VECTOR *) EAT (ipiv));

	if (ipiv->type != integer)
	  {
	    fail ("IPIV has type %s---should be integer.");
	    raise_exception ();
	  }

	if (ipiv->ne != fac->nr)
	  {
	    fail ("LU has %d row%s, but IPIV has %d element%s.",
		  fac->nr, PLURAL (fac->nr), ipiv->ne, PLURAL (ipiv->ne));
	    raise_exception ();
	  }

	AUTO_CAST_MATRIX (fac, b);
	x = (MATRIX *) dup_matrix (EAT (b));
	x->symmetry = general;

	if (x->rid) delete_entity (x->rid);
	if (fac->cid) x->rid = copy_entity (fac->cid);

	switch (fac->type)
	  {
	  case real:
	    SIGINT_RAISE_ON ();
	    RSYTRS (&iuplo, &fac->nr, &x->nc, fac->a.real, &fac->nr,
		    ipiv->a.integer, x->a.real, &x->nr, &info);
	    SIGINT_RAISE_OFF ();
	    if (info)
	      {
		wipeout ("Illegal value in the %d%s argument to DSYTRS.",
			 -info, TH (-info));
	      }
	    break;

	  case complex:

	    SIGINT_RAISE_ON ();
	    if (sym == symmetric)
	      XSYTRS (&iuplo, &fac->nr, &x->nc, fac->a.real, &fac->nr,
		      ipiv->a.integer, x->a.real, &x->nr, &info);
	    else
	      XHETRS (&iuplo, &fac->nr, &x->nc, fac->a.real, &fac->nr,
		      ipiv->a.integer, x->a.real, &x->nr, &info);
	    SIGINT_RAISE_OFF ();

	    if (info)
	      {
		wipeout ("Illegal value in the %d%s argument to ZSYTRS.",
			 -info, TH (-info));
	      }
	    break;
	  default:
	    fail ("LD or LDH has invalid %s type.",
		  type_string[fac->type]);
	    raise_exception ();
	  }
      }
    else if (IN_TABLE (a, "HOLD"))	/* Came from BCSLIB */
      {
#if HAVE_LIBBCSEXT

	int nhold;

	rcond = (SCALAR *) search_in_table ((TABLE *) copy_table (a),
					    dup_char ("RCOND"));
	if (rcond)
	  {
	    rcond = (SCALAR *) scalar_entity (EAT (rcond));
	    if (!test_scalar (EAT (rcond)))
	      {
		fail ("Singular matrix indicated by RCOND.");
		raise_exception ();
	      }
	  }

	neq = (SCALAR *) search_in_table ((TABLE *) copy_table (a),
					  dup_char ("N"));
	if (!neq)
	  {
	    fail ("Required member \"N\" is missing.");
	    raise_exception ();
	  }
	neq = (SCALAR *) scalar_entity (EAT (neq));
	neq = (SCALAR *) cast_scalar (EAT (neq), integer);

	hold = (VECTOR *) search_in_table ((TABLE *) copy_table (a),
					   dup_char ("HOLD"));
	hold = (VECTOR *) vector_entity (EAT (hold));
	hold = (VECTOR *) dense_vector ((VECTOR *) EAT (hold));
	nhold = hold->ne;

	if (neq->v.integer != b->nr)
	  {
	    fail ("Coefficient matrix has order %d, but right hand side has %d row%s.",
		  neq->v.integer, b->nr, PLURAL (b->nr));
	    raise_exception ();
	  }

	/* Turn off error reporting in BCSLIB. */

	{
	  int ityp = 1;
	  int ival = 0;
	  HHERRS (&ityp, &ival);
	}

	switch (hold->type)
	  {
	  case real:

	    if (b->type == complex)
	      {
		assert (b->density == dense);
		xr = (MATRIX *) cast_matrix_complex_real (
						(MATRIX *) copy_matrix (b));
		xi = (MATRIX *) imag_matrix (EAT (b));

		SIGINT_RAISE_ON ();
		HDSLSL (&xr->nc, xr->a.real, &xr->nr,
			hold->a.real, &nhold, &info);
		SIGINT_RAISE_OFF ();
		if (info)
		  wipeout ("Error return (%d) from HDSLSL.", info);

		SIGINT_RAISE_ON ();
		HDSLSL (&xi->nc, xi->a.real, &xi->nr,
			hold->a.real, &nhold, &info);
		SIGINT_RAISE_OFF ();
		if (info)
		  wipeout ("Error return (%d) from HDSLSL.", info);

		x = (MATRIX *) form_matrix (xr->nr, xr->nc, complex, dense);

		{
		  int i;
		  COMPLEX *xp = x->a.complex;
		  REAL *xrp = xr->a.real;
		  REAL *xip = xi->a.real;

		  for (i = 0; i < x->nn; i++)
		    {
		      xp->real = *xrp++;
		      xp->imag = *xip++;
		      xp++;
		    }
		}
	      }
	    else
	      {
		x = (MATRIX *) cast_matrix ((MATRIX *) EAT (b), real);
		x = (MATRIX *) dup_matrix ((MATRIX *) EAT (x));
		assert (x->density == dense);
		x->symmetry = general;

		SIGINT_RAISE_ON ();
		HDSLSL (&x->nc, x->a.real, &x->nr,
			hold->a.real, &nhold, &info);
		SIGINT_RAISE_OFF ();
		if (info)
		  wipeout ("Error return (%d) from HDSLSL.", info);
	      }

	    break;

	  case complex:

	    x = (MATRIX *) cast_matrix ((MATRIX *) EAT (b), complex);
	    x = (MATRIX *) dup_matrix ((MATRIX *) EAT (x));
	    assert (x->density == dense);
	    x->symmetry = general;

	    SIGINT_RAISE_ON ();
	    HZSLSL (&x->nc, x->a.complex, &x->nr,
		    hold->a.complex, &nhold, &info);
	    SIGINT_RAISE_OFF ();
	    if (info)
	      wipeout ("Error return (%d) from HZSLSL.", info);
	    break;

	  default:
	    fail ("HOLD has invalid %s type.", type_string[hold->type]);
	    raise_exception ();
	  }
#else
	fail ("BCSLIB required for this factor.");
	raise_exception ();
#endif
      }
    else
      {
	fail ("Don't understand this kind of factor.");
	raise_exception ();
      }
  }

  ON_EXCEPTION
  {
#if HAVE_LIBSUPERLU
    delete_2_entities (rid, cid);
    delete_2_tables (Lt, Ut);
    delete_2_vectors (perm_r, perm_c);
    delete_matrix (x2);
    free_SuperLU (L);
    free_SuperLU (U);
    if (superlu_status_allocated) StatFree ();
#endif
#if HAVE_LIBBCSEXT
    delete_2_matrices (xr, xi);
#endif
    delete_3_matrices (b, fac, x);
    delete_table (a);
    delete_3_scalars (method, rcond, neq);
    delete_2_vectors (ipiv, hold);
  }
  END_EXCEPTION;

#if HAVE_LIBBCSEXT
  delete_2_matrices (xr, xi);
#endif
#if HAVE_LIBSUPERLU
  delete_2_entities (rid, cid);
  delete_2_vectors (perm_r, perm_c);
  free_SuperLU (L);
  free_SuperLU (U);
  if (superlu_status_allocated) StatFree ();
#endif
  delete_2_matrices (b, fac);
  delete_table (a);
  delete_2_scalars (rcond, neq);
  delete_2_vectors (hold, ipiv);
  return (ENT (x));
}
