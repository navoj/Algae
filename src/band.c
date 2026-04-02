/*
   band.c -- Matrix bandwidth and profile minimization.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: band.c,v 1.6 2003/08/01 04:57:46 ksh Exp $";

#include "band.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "partition.h"
#include "sparse.h"
#include "full.h"
#include "fortran.h"
#include "generate.h"
#include "transpose.h"
#include "test.h"

ENTITY *
bi_band (ENTITY *p)
{
  return band_matrix ((MATRIX *) matrix_entity (p));
}

/* Compute bandwidth and profile of a matrix. */

ENTITY *
band_matrix (MATRIX *p)
{
  VECTOR * volatile v = NULL;
  int bandwd, i, k;
  REAL profil;

  EASSERT (p, matrix, 0);

  WITH_HANDLING
  {
    if (p->nr != p->nc)
      {
	fail ("Non-square matrix given to \"band\".");
	raise_exception ();
      }

    if (p->density == dense) p = (MATRIX *) sparse_matrix (EAT (p));
    p = (MATRIX *) full_matrix (EAT (p));
    assert (p->density == sparse);

    v = (VECTOR *) make_vector (4, real, dense);

    /* Lower profile and bandwidth */

    bandwd = 0;
    profil = 0.0;
    for (i = 0; i < p->nr; i++)
      {
	k = i - p->ja[p->ia[i] - 1] + 1;
	if (k < 0) k = 0;
	if (k > bandwd) bandwd = k;
	profil += (REAL) k;
      }

    v->a.real[0] = (REAL) bandwd;
    v->a.real[1] = profil;

    p = (MATRIX *) transpose_matrix (EAT (p));

    /* Upper profile and bandwidth */

    bandwd = 0;
    profil = 0.0;
    for (i = 0; i < p->nr; i++)
      {
	k = i - p->ja[p->ia[i] - 1] + 1;
	if (k < 0) k = 0;
	if (k > bandwd) bandwd = k;
	profil += (REAL) k;
      }

    v->a.real[2] = (REAL) bandwd;
    v->a.real[3] = profil;
  }
  ON_EXCEPTION
  {
    delete_matrix (p);
    delete_vector (v);
  }
  END_EXCEPTION;

  delete_matrix (p);
  return ENT (v);
}

ENTITY *
bi_gpskca (int n, ENTITY *p, ENTITY *b)
{
  int f;
  ENTITY * volatile v = NULL;

  WITH_HANDLING
  {
    f = b ? test_entity (EAT (b)) : 0;
    v = gpskca_matrix ((MATRIX *) matrix_entity (EAT (p)), f);
  }
  ON_EXCEPTION
  {
    delete_3_entities (p, b, v);
  }
  END_EXCEPTION;

  return v;
}

ENTITY *
gpskca_matrix (MATRIX *p, int f)
{
  MATRIX *tp = NULL;
  VECTOR * volatile vp = NULL;
  int i, k, wrklen, optpro;
  int *degree = NULL;
  int *work = NULL;
  int *permut = NULL;
  int *rstart, *connec;
  int bandwd, error_flag, space, adjcnt;
  double profil;

  EASSERT (p, matrix, 0);

  WITH_HANDLING
  {
    assert (p->order == ordered);

    if (p->nr != p->nc)
      {
	fail ("Non-square matrix given to \"gpskca\".");
	raise_exception ();
      }

    if (p->symmetry != symmetric && p->symmetry != hermitian)
      {
	fail ("Matrix given to \"gpskca\" must be symmetric or hermitian.");
	raise_exception ();
      }

    if (p->density == dense) p = (MATRIX *) sparse_matrix (EAT (p));
    assert (p->density != dense);

    vp = (VECTOR *) generate_integer (1, p->nr, 1);

    /*  We can quit if the matrix has no off-diagonal elements. */

    if (p->nn)
      {

	/*
	 *  First we construct DEGREE, RSTART, and CONNEC.  DEGREE is
	 *  a vector storing the number of nonzero off-diagonal
	 *  elements in each row.  RSTART is just like "ia", except
	 *  that only off-diagonal elements are considered.  Likewise,
	 *  CONNEC is just like "ja" without the off-diagonal elements.
	 */

	tp = (MATRIX *) dup_matrix((MATRIX *) full_matrix((MATRIX *) EAT (p)));
	degree = (int *) CALLOC (tp->nr, sizeof (int));

	for (i = 0; i < tp->nr; i++)
	  {
	    for (k = tp->ia[i]; k < tp->ia[i + 1]; k++)
	      {
		if (tp->ja[k - 1] - 1 == i)
		  tp->ja[k - 1] = 0;
	      }
	  }
	tp = (MATRIX *) squeeze_matrix (EAT (tp));
	for (i = 0; i < tp->nr; i++)
	  {
	    degree[i] = tp->ia[i + 1] - tp->ia[i];
	  }
	rstart = tp->ia;
	connec = tp->ja;
	wrklen = 6 * tp->nr + 3;	/* the most it could need */
	work = (int *) CALLOC (wrklen, sizeof (int));
	permut = dup_mem (vp->a.integer, vp->ne * sizeof (int));

	optpro = !f;
	SIGINT_RAISE_ON ();
	GPSKCA (&tp->nr, degree, rstart, connec, &optpro, &wrklen,
		permut, work, &bandwd, &profil, &error_flag,
		&space, &adjcnt);
	SIGINT_RAISE_OFF ();

	FREE (EAT (work));
	FREE (EAT (degree));
	delete_matrix (tp); tp = NULL;

	if (error_flag)
	  {
	    fail ("Problem in \"gpskca\".");
	    raise_exception ();
	  }

	/*  We want to return the complement to permut. */

	assert (vp->entity.ref_count == 1);

	for (i = 0; i < vp->ne; i++)
	  vp->a.integer[permut[i] - 1] = i + 1;

	FREE (EAT (permut));
      }
  }
  ON_EXCEPTION
  {
    delete_2_matrices (p, tp);
    delete_vector (vp);
    TFREE (work);
    TFREE (permut);
    TFREE (degree);
  }
  END_EXCEPTION;

  delete_2_matrices (p, tp);
  return ENT (vp);
}
