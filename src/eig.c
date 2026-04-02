/*
   eig.c -- Eigenvalues.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: eig.c,v 1.7 2003/08/01 04:57:47 ksh Exp $";

#include "eig.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "lapack.h"
#include "two_type.h"
#include "cast.h"
#include "dense.h"
#include "sparse.h"

static char *eig_options[] =
{
  "no_right",			/* don't compute right eigenvectors */
  "left",			/* compute left eigenvectors */
  NULL,
};
enum
  {
    OPT_NO_RIGHT, OPT_LEFT
  };

ENTITY *
bi_eig (int n, ENTITY *a, ENTITY *b, ENTITY *c)
{
  /*
   * This function computes eigenvalues and eigenvectors for
   * the standard and generalized eigenproblems
   *          A * x = lambda * x
   * and
   *          A * x = lambda * B * x
   * The matrix "A" is given by the required first argument, `a'.
   * If `a' is the only argument, then both "lambda" and "x" are
   * computed for the standard problem.  If `b' is a matrix (or
   * can be converted to one), then it is "B" and the generalized
   * problem is solved.  Finally, a table of options may be given
   * as the last argument.
   */

  ENTITY * volatile ret;

  EASSERT (a, 0, 0);

  WITH_HANDLING
  {
    a = matrix_entity (EAT (a));

    if (c && c->class != table)
      {
	fail ("Optional third argument to \"eig\" must be a table.");
	raise_exception ();
      }

    if (b)
      {
	if (b->class == table)
	  {
	    ret = seig_matrix (EAT (a), EAT (b));
	  }
	else
	  {
	    b = matrix_entity (EAT (b));
	    ret = c ?
	      geig_matrix (EAT (a), EAT (b), EAT (c)) :
	      geig_matrix (EAT (a), EAT (b), (TABLE *) NULL);
	  }
      }
    else
      {
	ret = c ?
	  seig_matrix (EAT (a), EAT (c)) :
	  seig_matrix (EAT (a), (TABLE *) NULL);
      }

  }
  ON_EXCEPTION
  {
    delete_3_entities (a, b, c);
  }
  END_EXCEPTION;

  delete_3_entities (a, b, c);
  return ret;
}

ENTITY *
seig_matrix (MATRIX *a, TABLE *opt)
{
  int iuplo, lwork, info, ijobvl, ijobvr;
  void *work = NULL;
  void *rwork = NULL;
  MATRIX *v = NULL;
  MATRIX *vl = NULL;
  VECTOR *w = NULL;
  TABLE *t = NULL;
  int i, k;
  SYMMETRY sym;
  COMPLEX ws;

  EASSERT (a, matrix, 0);

  WITH_HANDLING
  {

    /* Check the arguments. */

    if (a->nr != a->nc)
      {
	fail ("A square matrix is required for a standard eigenvalue problem.");
	raise_exception ();
      }

    if (a->type == character)
      {
	fail ("Character type in a standard eigenvalue problem.");
	raise_exception ();
      }

    if (a->nr == 0)
      {
	fail ("Matrix has no elements.");
	raise_exception ();
      }

    /* Handle the options. */

    if (!OK_OPTIONS (opt, eig_options))
      {
	raise_exception ();
      }

    ijobvr = (IN_TABLE (opt, eig_options[OPT_NO_RIGHT])) ? 'N' : 'V';
    ijobvl = (IN_TABLE (opt, eig_options[OPT_LEFT])) ? 'V' : 'N';
    delete_table (opt);
    opt = NULL;

    /* Convert integer to real. */

    if (a->type == integer)
      a = (MATRIX *) cast_matrix_integer_real ((MATRIX *) EAT (a));

    /*
     * We have no sparse code for general symmetry or for symmetric
     * complex, so go to dense.
     */

    if (a->density != dense &&
	(a->symmetry == general ||
	 a->type == complex && a->symmetry == symmetric))
      {
	inform ("Converting to dense storage.");
	a = (MATRIX *) dense_matrix (EAT (a));
      }

    /* OK, solve it. */

    switch (a->density)
      {

      case dense:

	switch (a->type)
	  {

	  case real:

	    switch (a->symmetry)
	      {

	      case symmetric:

		ijobvl = 'N';	/* symmetric -- no left vectors */

		v = (MATRIX *) dup_matrix ((MATRIX *) EAT (a));
		delete_entity (v->rid);
		v->rid = NULL;
		if (v->cid)
		  v->rid = copy_entity (v->cid);
		v->symmetry = general;
		w = (VECTOR *) form_vector (v->nr, real, dense);

		lwork = -1;
		iuplo = 'L';

		/* Call once to get the workspace size. */

		RSYEV (&ijobvr, &iuplo, &v->nr, v->a.real,
		       &v->nr, w->a.real, (REAL *) & ws, &lwork, &info);

		lwork = (int) ws.real;
		work = E_MALLOC (lwork, real);

		SIGINT_RAISE_ON ();
		RSYEV (&ijobvr, &iuplo, &v->nr, v->a.real,
		       &v->nr, w->a.real, work, &lwork, &info);
		SIGINT_RAISE_OFF ();
		FREE (work);
		work = NULL;

		if (info)
		  {
		    if (info < 0)
		      {
			fail ("Illegal value in the %d%s argument to DSYEV.  Please report this bug.",
			      -info, TH (-info));
		      }
		    else
		      {
			fail ("Algorithm terminated before finding the %d%s eigenvalue.",
			      info, TH (info));
		      }
		    raise_exception ();
		  }
		break;

	      case general:

		a = (MATRIX *) dup_matrix ((MATRIX *) a);
		v = (MATRIX *) make_matrix (a->nr, a->nc, complex, dense);
		if (a->cid)
		  v->rid = copy_entity (a->cid);
		v->symmetry = general;
		if (ijobvl == 'V')
		  {
		    vl = (MATRIX *) make_matrix (a->nr, a->nc,
						 complex, dense);
		    if (a->cid)
		      vl->rid = copy_entity (a->cid);
		    vl->symmetry = general;
		  }
		else
		  {
		    vl = (MATRIX *) copy_matrix (v);
		  }
		w = (VECTOR *) form_vector (v->nr, complex, dense);

		/* Call once to get the workspace size. */

		lwork = -1;

		SIGINT_RAISE_ON ();
		RGEEV (&ijobvl, &ijobvr, &a->nr, a->a.real, &a->nr,
		       w->a.real, w->a.real + a->nr, vl->a.real, &vl->nr,
		       v->a.real, &v->nr, (REAL *) &ws, &lwork, &info);
		SIGINT_RAISE_OFF ();

		lwork = (int) ws.real;
		work = E_MALLOC (lwork, real);

		SIGINT_RAISE_ON ();
		RGEEV (&ijobvl, &ijobvr, &a->nr, a->a.real, &a->nr,
		       w->a.real, w->a.real + a->nr, vl->a.real, &vl->nr,
		       v->a.real, &v->nr, work, &lwork, &info);
		SIGINT_RAISE_OFF ();

		delete_matrix (a);
		a = NULL;

		/* Unpack LAPACK's weird format. */

		memcpy (work, w->a.real + v->nr, v->nr * sizeof (REAL));
		for (i = v->nr - 1; i >= 0; i--)
		  {
		    POLL_SIGINT ();
		    w->a.complex[i].real = w->a.real[i];
		    w->a.complex[i].imag = ((REAL *) work)[i];
		    if (w->a.complex[i].imag == 0.0)
		      {
			for (k = v->nr - 1; k >= 0; k--)
			  {
			    v->a.complex[i * v->nr + k].real =
			      v->a.real[i * v->nr + k];
			    v->a.complex[i * v->nr + k].imag = 0.0;
			  }
		      }
		    else if (w->a.complex[i].imag < 0.0)
		      {
			for (k = 0; k < v->nr; k++)
			  {
			    v->a.complex[i * v->nr + k].real =
			      v->a.real[(i - 1) * v->nr + k];
			    v->a.complex[i * v->nr + k].imag =
			      -v->a.real[i * v->nr + k];
			  }
		      }
		    else
		      {
			for (k = 0; k < v->nr; k++)
			  {
			    v->a.complex[i * v->nr + k].real =
			      v->a.complex[(i + 1) * v->nr + k].real;
			    v->a.complex[i * v->nr + k].imag =
			      -v->a.complex[(i + 1) * v->nr + k].imag;
			  }
		      }
		  }
		FREE (work);
		work = NULL;

		if (ijobvl == 'V')
		  {
		    for (i = vl->nr - 1; i >= 0; i--)
		      {
			POLL_SIGINT ();
			if (w->a.complex[i].imag == 0.0)
			  {
			    for (k = vl->nr - 1; k >= 0; k--)
			      {
				vl->a.complex[i * vl->nr + k].real =
				  vl->a.real[i * vl->nr + k];
				vl->a.complex[i * vl->nr + k].imag = 0.0;
			      }
			  }
			else if (w->a.complex[i].imag < 0.0)
			  {
			    for (k = 0; k < vl->nr; k++)
			      {
				vl->a.complex[i * vl->nr + k].real =
				  vl->a.real[(i - 1) * vl->nr + k];
				vl->a.complex[i * vl->nr + k].imag =
				  -vl->a.real[i * vl->nr + k];

			      }
			  }
			else
			  {
			    for (k = 0; k < vl->nr; k++)
			      {
				vl->a.complex[i * vl->nr + k].real =
				  vl->a.complex[(i + 1) * vl->nr + k].real;
				vl->a.complex[i * vl->nr + k].imag =
				  -vl->a.complex[(i + 1) * vl->nr + k].imag;
			      }
			  }
		      }
		  }

		if (info)
		  {
		    if (info < 0)
		      {
			fail ("Illegal value in the %d%s argument to DGEEV.  Please report this bug.",
			      -info, TH (-info));
		      }
		    else
		      {
			fail ("The QR algorithm in \"eig\" failed to converge.");
		      }
		    raise_exception ();
		  }
		break;

	      default:
		BAD_SYMMETRY (a->symmetry);
		raise_exception ();
	      }
	    break;

	  case complex:

	    switch (a->symmetry)
	      {

	      case hermitian:

		ijobvl = 'N';	/* hermitian -- no left vectors */

		v = (MATRIX *) dup_matrix ((MATRIX *) EAT (a));
		delete_entity (v->rid);
		v->rid = NULL;
		if (v->cid)
		  v->rid = copy_entity (v->cid);
		v->symmetry = general;
		w = (VECTOR *) form_vector (v->nr, real, dense);

		lwork = -1;
		iuplo = 'L';

		XHEEV (&ijobvr, &iuplo, &v->nr, v->a.complex,
		       &v->nr, w->a.real, &ws, &lwork,
		       rwork, &info);

		lwork = (int) ws.real;
		work = E_MALLOC (lwork, complex);
		rwork = E_MALLOC (3 * v->nr + 1, real);

		SIGINT_RAISE_ON ();
		XHEEV (&ijobvr, &iuplo, &v->nr, v->a.complex,
		       &v->nr, w->a.real, work, &lwork, rwork, &info);
		SIGINT_RAISE_OFF ();
		FREE (work);
		work = NULL;
		FREE (rwork);
		rwork = NULL;

		if (info)
		  {
		    if (info < 0)
		      {
			fail ("Illegal value in the %d%s argument to ZHEEV.  Please report this bug.",
			      -info, TH (-info));
		      }
		    else
		      {
			fail ("Algorithm terminated before finding the %d%s eigenvalue.",
			      info, TH (info));
		      }
		    raise_exception ();
		  }
		break;

	      case symmetric:
	      case general:

		a = (MATRIX *) dup_matrix ((MATRIX *) a);
		v = (MATRIX *) form_matrix (a->nr, a->nc, complex, dense);
		if (a->cid)
		  v->rid = copy_entity (a->cid);
		v->symmetry = general;

		if (ijobvl == 'V')
		  {
		    vl = (MATRIX *) form_matrix (a->nr, a->nc,
						 complex, dense);
		    if (a->cid)
		      vl->rid = copy_entity (a->cid);
		    vl->symmetry = general;
		  }
		else
		  {
		    vl = (MATRIX *) copy_matrix (v);
		  }
		w = (VECTOR *) form_vector (v->nr, complex, dense);
		rwork = E_MALLOC (2 * v->nr, real);

		/* Call once to get the workspace size. */

		lwork = -1;

		SIGINT_RAISE_ON ();
		XGEEV (&ijobvl, &ijobvr, &a->nr, a->a.complex, &a->nr,
		       w->a.complex, vl->a.complex, &vl->nr,
		       v->a.complex, &v->nr, &ws, &lwork,
		       rwork, &info);
		SIGINT_RAISE_OFF ();

		lwork = (int) ws.real;
		work = E_MALLOC (lwork, complex);

		SIGINT_RAISE_ON ();
		XGEEV (&ijobvl, &ijobvr, &a->nr, a->a.complex, &a->nr,
		       w->a.complex, vl->a.complex, &vl->nr,
		       v->a.complex, &v->nr, work, &lwork,
		       rwork, &info);
		SIGINT_RAISE_OFF ();

		if (info)
		  {
		    if (info < 0)
		      {
			fail ("The %d%s argument to ZGEEV had an illegal value.  Please report this bug.",
			      -info, TH (-info));
		      }
		    else
		      {
			fail ("The QR algorithm failed to compute all the eigenvalues.");
		      }
		    raise_exception ();
		  }

		FREE (work);
		work = NULL;
		FREE (rwork);
		rwork = NULL;

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
      case sparse:
      case sparse_upper:

	/* Convert to a "band". */

	sym = a->symmetry;
	a = (MATRIX *) sparse_to_band (EAT (a));

	/*
	 * This is `ku', but we don't have to look it up since we
	 * know `a' is symmetric.
	 */

	k = a->nr - 1;

	switch (a->type)
	  {

	  case real:

	    switch (sym)
	      {

	      case symmetric:

		if (ijobvr == 'V')
		  {
		    v = (MATRIX *) form_matrix (a->nc, a->nc, real, dense);
		    if (a->cid)
		      v->rid = copy_entity (a->cid);
		  }
		else
		  {
		    v = (MATRIX *) copy_matrix (a);
		  }
		w = (VECTOR *) form_vector (a->nc, real, dense);

		work = E_MALLOC (3 * a->nc + 1, real);

		iuplo = 'U';
		SIGINT_RAISE_ON ();
		RSBEV (&ijobvr, &iuplo, &a->nc, &k, a->a.real, &a->nr,
		       w->a.real, v->a.real, &v->nr, work, &info);
		SIGINT_RAISE_OFF ();
		FREE (work);
		work = NULL;

		if (info)
		  {
		    if (info < 0)
		      {
			fail ("Illegal value in the %d%s argument to DSBEV.  Please report this bug.",
			      -info, TH (-info));
		      }
		    else
		      {
			fail ("DSBEV terminated before finding the %d%s eigenvalue.",
			      info, TH (info));
		      }
		    raise_exception ();
		  }
		break;

	      default:

		wipeout ("Bad symmetry.");

	      }
	    break;

	  case complex:

	    switch (sym)
	      {

	      case hermitian:

		if (ijobvr == 'V')
		  {
		    v = (MATRIX *) form_matrix (a->nc, a->nc,
						complex, dense);
		    if (a->cid)
		      v->rid = copy_entity (a->cid);
		  }
		else
		  {
		    v = (MATRIX *) copy_matrix (a);
		  }
		w = (VECTOR *) form_vector (a->nc, real, dense);

		work = E_MALLOC (a->nc, complex);
		rwork = E_MALLOC (3 * a->nc + 1, real);

		iuplo = 'U';
		SIGINT_RAISE_ON ();
		XHBEV (&ijobvr, &iuplo, &a->nc, &k, a->a.complex,
		       &a->nr, w->a.real, v->a.complex, &v->nr,
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
			fail ("Illegal value in the %d%s argument to ZHBEV.  Please report this bug.",
			      -info, TH (-info));
		      }
		    else
		      {
			fail ("ZHBEV terminated before finding the %d%s eigenvalue.",
			      info, TH (info));
		      }
		    raise_exception ();
		  }
		break;

	      default:

		wipeout ("Bad symmetry.");

	      }
	    break;

	  default:

	    wipeout ("Bad type.");

	  }
	break;

      default:
	BAD_DENSITY (a->density);
	raise_exception ();
      }

    /* Now eigenvalues are in `w' and eigenvectors might be in `v'. */

    t = (TABLE *) replace_in_table ((TABLE *) make_table (),
				    copy_vector (w),
				    dup_char ("values"));

    if (ijobvr == 'V')
      {
	delete_entity (v->cid);
	v->cid = EAT (w);
	t = (TABLE *) replace_in_table ((TABLE *) EAT (t), EAT (v),
					dup_char ("vectors"));
      }

    if (ijobvl == 'V')
      {
	delete_entity (vl->cid);
	vl->cid = EAT (w);
	t = (TABLE *) replace_in_table ((TABLE *) EAT (t), EAT (vl),
					dup_char ("left_vectors"));
      }
  }
  ON_EXCEPTION
  {
    TFREE (work);
    TFREE (rwork);
    delete_3_matrices (a, v, vl);
    delete_vector (w);
    delete_2_tables (opt, t);
  }
  END_EXCEPTION;

  delete_3_matrices (a, v, vl);
  delete_vector (w);
  return (ENT (t));
}

ENTITY *
geig_matrix (MATRIX *a, MATRIX *b, TABLE * volatile opt)
{
  /* Generalized eigenvalue problem. */

  int iuplo, ijobvr, ijobvl, lwork, info, itype, i, k;
  COMPLEX ws;
  void * volatile work = NULL;
  REAL * volatile rwork = NULL;
  MATRIX *v = NULL;
  VECTOR *w = NULL;
  TABLE *t = NULL;
  TABLE *tt = NULL;
  MATRIX *vl = NULL;
  VECTOR *num = NULL;
  VECTOR *den = NULL;

  EASSERT (a, matrix, 0);
  EASSERT (b, matrix, 0);

  WITH_HANDLING
  {
    /* Check the arguments. */

    if (a->nr != a->nc || b->nr != b->nc)
      {
	fail ("Square matrices are required for the generalized eigenvalue problem.");
	raise_exception ();
      }

    if (a->nr != b->nr)
      {
        fail ("Matrices must have the same size.");
        raise_exception ();
      }

    if (a->type == character || b->type == character)
      {
	fail ("Invalid character type for the generalized eigenvalue problem.");
	raise_exception ();
      }

    if (a->nr == 0)
      {
	fail ("Matrix has no elements.");
	raise_exception ();
      }

    if (!MATCH_VECTORS (a->rid, b->rid) ||
	!MATCH_VECTORS (a->cid, b->cid))
      {
	fail ("Labels don't match.");
	raise_exception ();
      }

    /* Handle the options. */

    if (!OK_OPTIONS (opt, eig_options))
      {
	raise_exception ();
      }

    ijobvr = (IN_TABLE (opt, eig_options[OPT_NO_RIGHT])) ? 'N' : 'V';
    ijobvl = (IN_TABLE (opt, eig_options[OPT_LEFT])) ? 'V' : 'N';
    delete_table (opt);
    opt = NULL;

    /* Convert integer to real. */

    if (a->type == integer)
      a = (MATRIX *) cast_matrix_integer_real ((MATRIX *) EAT (a));
    if (b->type == integer)
      b = (MATRIX *) cast_matrix_integer_real ((MATRIX *) EAT (b));
    AUTO_CAST_MATRIX (a, b);

    /* For now, we have no sparse solvers, so convert to dense. */

    a = (MATRIX *) dense_matrix (EAT (a));

    /* OK, solve it. */

    switch (a->density)
      {
      case dense:

	b = (MATRIX *) dense_matrix ((MATRIX *) EAT (b));

	switch (a->type)
	  {
	  case real:

	    switch (TWO_SYMMETRY (a->symmetry, b->symmetry))
	      {
	      case symmetric_symmetric:

		b = (MATRIX *) dup_matrix ((MATRIX *) EAT (b));
		v = (MATRIX *) dup_matrix ((MATRIX *) EAT (a));
		delete_entity (v->rid);
		v->rid = NULL;
		if (v->cid)
		  {
		    v->rid = copy_entity (v->cid);
		  }
		else if (b->cid)
		  {
		    v->rid = copy_entity (b->cid);
		  }
		v->symmetry = general;
		w = (VECTOR *) form_vector (v->nr, real, dense);

		itype = 1;
		iuplo = 'L';

		/* call once to get the best work size */

		lwork = -1;

		RSYGV (&itype, &ijobvr, &iuplo,
		       &v->nr, v->a.real, &v->nr,
		       b->a.real, &b->nr, w->a.real,
		       (REAL *) &ws, &lwork, &info);

		lwork = (int) ws.real;
		work = E_MALLOC (lwork, real);

		SIGINT_RAISE_ON ();
		RSYGV (&itype, &ijobvr, &iuplo,
		       &v->nr, v->a.real, &v->nr,
		       b->a.real, &b->nr, w->a.real,
		       work, &lwork, &info);
		SIGINT_RAISE_OFF ();

		FREE (work);
		work = NULL;

		if (info)
		  {
		    if (info < 0)
		      {
			fail ("Illegal value in %d%s argument to DSYGV.  Please report this bug.",
			      -info, TH (-info));
		      }
		    else if (info <= v->nr)
		      {
			fail ("DSYEV failed to converge.");
		      }
		    else
		      {
			fail ("Matrix B is not positive definite.");
		      }
		    raise_exception ();
		  }

		/* Now values are in `w' and vectors might be in `v'. */

		t = (TABLE *) replace_in_table ((TABLE *) make_table (),
						copy_vector (w),
						dup_char ("values"));

		if (ijobvr == 'V')
		  {
		    delete_entity (v->cid);
		    v->cid = EAT (w);
		    t = (TABLE *) replace_in_table ((TABLE *) EAT (t),
						    EAT (v),
						    dup_char ("vectors"));
		  }

		break;

	      case symmetric_general:
	      case general_symmetric:
	      case general_general:

#if LAPACK_1_0
		detour ("Your version of Algae was compiled to use an old version of LAPACK, and doesn't support unsymmetric generalized eigenvalue problems.");
		raise_exception ();
#else
		a = (MATRIX *) dup_matrix ((MATRIX *) EAT (a));
		b = (MATRIX *) dup_matrix ((MATRIX *) EAT (b));

		if (ijobvr == 'V')
		  {

		    v = (MATRIX *) form_matrix (a->nr, a->nc,
						complex, dense);
		    if (a->cid)
		      {
			v->rid = copy_entity (a->cid);
		      }
		    else if (b->cid)
		      {
			v->rid = copy_entity (b->cid);
		      }

		  }
		else
		  {

		    /* Just need a valid pointer. */

		    v = (MATRIX *) copy_matrix (a);

		  }

		if (ijobvl == 'V')
		  {

		    vl = (MATRIX *) form_matrix (a->nr, a->nc,
						 complex, dense);
		    if (a->rid)
		      {
			vl->rid = copy_entity (a->rid);
		      }
		    else if (b->rid)
		      {
			vl->rid = copy_entity (b->rid);
		      }

		  }
		else
		  {

		    /* Just need a valid pointer. */

		    vl = (MATRIX *) copy_matrix (a);

		  }


		num = (VECTOR *) form_vector (a->nr, complex, dense);
		den = (VECTOR *) form_vector (a->nr, real, dense);

		/* Call to get workspace size. */

		lwork = -1;

		RGGEV (&ijobvl, &ijobvr,
		       &a->nr, a->a.real, &a->nr,
		       b->a.real, &b->nr,
		       num->a.real, num->a.real + a->nr, den->a.real,
		       vl->a.real, &vl->nr, v->a.real, &v->nr,
		       (REAL *) &ws, &lwork, &info);

		lwork = (int) ws.real;
		work = E_MALLOC (lwork, real);

		SIGINT_RAISE_ON ();
		RGGEV (&ijobvl, &ijobvr,
		       &a->nr, a->a.real, &a->nr,
		       b->a.real, &b->nr,
		       num->a.real, num->a.real + a->nr, den->a.real,
		       vl->a.real, &vl->nr, v->a.real, &v->nr,
		       (REAL *) work, &lwork, &info);
		SIGINT_RAISE_OFF ();
		FREE (work);
		work = NULL;

		if (info)
		  {
		    if (info < 0)
		      fail ("Illegal value in %d%s argument to DGGEV.  Please report this bug.",
			    -info, TH (-info));
		    else if (info <= a->nr)
		      fail ("The QZ iteration failed in DGGEV.");
		    else
		      fail ("Internal error in DGGEV.");

		    raise_exception ();
		  }

		/* Convert real and imag parts to complex. */

		memcpy (a->a.real, num->a.real,
			a->nr * sizeof (REAL));
		memcpy (b->a.real, num->a.real + a->nr,
			a->nr * sizeof (REAL));
		for (i = 0; i < a->nr; i++)
		  {
		    num->a.complex[i].real = a->a.real[i];
		    num->a.complex[i].imag = b->a.real[i];
		  }

		/* Unpack the eigenvectors. */

		for (i = v->nr - 1; i >= 0; i--)
		  {
		    POLL_SIGINT ();
		    if (num->a.complex[i].imag == 0.0)
		      {
			for (k = v->nr - 1; k >= 0; k--)
			  {
			    v->a.complex[i * v->nr + k].real =
			      v->a.real[i * v->nr + k];
			    v->a.complex[i * v->nr + k].imag = 0.0;
			  }
		      }
		    else if (num->a.complex[i].imag < 0.0)
		      {
			for (k = 0; k < v->nr; k++)
			  {
			    v->a.complex[i * v->nr + k].real =
			      v->a.real[(i - 1) * v->nr + k];
			    v->a.complex[i * v->nr + k].imag =
			      -v->a.real[i * v->nr + k];
			  }
		      }
		    else
		      {
			for (k = 0; k < v->nr; k++)
			  {
			    v->a.complex[i * v->nr + k].real =
			      v->a.complex[(i + 1) * v->nr + k].real;
			    v->a.complex[i * v->nr + k].imag =
			      -v->a.complex[(i + 1) * v->nr + k].imag;
			  }
		      }
		  }

		if (ijobvl == 'V')
		  {
		    for (i = vl->nr - 1; i >= 0; i--)
		      {
			POLL_SIGINT ();
			if (num->a.complex[i].imag == 0.0)
			  {
			    for (k = vl->nr - 1; k >= 0; k--)
			      {
				vl->a.complex[i * vl->nr + k].real =
				  vl->a.real[i * vl->nr + k];
				vl->a.complex[i * vl->nr + k].imag = 0.0;
			      }
			  }
			else if (num->a.complex[i].imag < 0.0)
			  {
			    for (k = 0; k < vl->nr; k++)
			      {
				vl->a.complex[i * vl->nr + k].real =
				  vl->a.real[(i - 1) * vl->nr + k];
				vl->a.complex[i * vl->nr + k].imag =
				  -vl->a.real[i * vl->nr + k];
			      }
			  }
			else
			  {
			    for (k = 0; k < vl->nr; k++)
			      {
				vl->a.complex[i * vl->nr + k].real =
				  vl->a.complex[(i + 1) * vl->nr + k].real;
				vl->a.complex[i * vl->nr + k].imag =
				  -vl->a.complex[(i + 1) * vl->nr + k].imag;
			      }
			  }
		      }
		  }

		tt = (TABLE *) replace_in_table ((TABLE *) make_table (),
						 EAT (num),
						 dup_char ("num"));
		tt = (TABLE *) replace_in_table (EAT (tt),
						 EAT (den),
						 dup_char ("denom"));

		t = (TABLE *) replace_in_table ((TABLE *) make_table (),
						EAT (tt),
						dup_char ("values"));

		if (ijobvr == 'V')
		  {
		    delete_entity (v->cid);
		    v->cid = NULL;
		    t = (TABLE *) replace_in_table (EAT (t),
						    EAT (v),
						    dup_char ("vectors"));
		  }

		if (ijobvl == 'V')
		  {
		    delete_entity (vl->cid);
		    vl->cid = NULL;
		    t = (TABLE *) replace_in_table (
						     EAT (t), EAT (vl),
						 dup_char ("left_vectors"));
		  }
#endif
		break;

	      default:
		BAD_SYMMETRY (a->symmetry);
		raise_exception ();
	      }
	    break;
	  case complex:

	    switch (TWO_SYMMETRY (a->symmetry, b->symmetry))
	      {
	      case hermitian_hermitian:

		b = (MATRIX *) dup_matrix ((MATRIX *) EAT (b));
		v = (MATRIX *) dup_matrix ((MATRIX *) EAT (a));
		delete_entity (v->rid);
		v->rid = NULL;
		if (v->cid)
		  {
		    v->rid = copy_entity (v->cid);
		  }
		else if (b->cid)
		  {
		    v->rid = copy_entity (b->cid);
		  }
		v->symmetry = general;
		w = (VECTOR *) form_vector (v->nr, real, dense);

		itype = 1;
		iuplo = 'U';

		/* call once to get the best work size */

		lwork = -1;
		rwork = E_MALLOC (3*v->nr+1, real);

		/*
		 * The current (7/22/01) version of ZHEGV has a bug
		 * which prevents us from using the "workspace query"
		 * call.  For now, I'll just pick a size.
		 */

		/*
		 *XHEGV (&itype, &ijobvr, &iuplo,
		 *	 &v->nr, v->a.complex, &v->nr,
		 *	 b->a.complex, &b->nr, w->a.real,
		 *	 (COMPLEX *) &ws, &lwork, rwork, &info);
		 *
		 *lwork = (int) ws.real;
		 *work = E_MALLOC (lwork, complex);
		 */

		lwork = 33 * v->nr;
		work = E_MALLOC (lwork, complex);

		SIGINT_RAISE_ON ();
		XHEGV (&itype, &ijobvr, &iuplo,
		       &v->nr, v->a.complex, &v->nr,
		       b->a.complex, &b->nr, w->a.real,
		       work, &lwork, rwork, &info);
		SIGINT_RAISE_OFF ();

		FREE (work); work = NULL;
		FREE (rwork); rwork = NULL;

		if (info)
		  {
		    if (info < 0)
		      {
			fail ("Illegal value in %d%s argument to ZHEGV.  Please report this bug.",
			      -info, TH (-info));
		      }
		    else if (info <= v->nr)
		      {
			fail ("ZHEEV failed to converge.");
		      }
		    else
		      {
			fail ("Matrix B is not positive definite.");
		      }
		    raise_exception ();
		  }

		/* Now values are in `w' and vectors might be in `v'. */

		t = (TABLE *) replace_in_table ((TABLE *) make_table (),
						copy_vector (w),
						dup_char ("values"));

		if (ijobvr == 'V')
		  {
		    delete_entity (v->cid);
		    v->cid = EAT (w);
		    t = (TABLE *) replace_in_table ((TABLE *) EAT (t),
						    EAT (v),
						    dup_char ("vectors"));
		  }

		break;

	      case general_general:
	      case general_symmetric:
	      case general_hermitian:
	      case symmetric_general:
	      case symmetric_symmetric:
	      case symmetric_hermitian:
	      case hermitian_general:
	      case hermitian_symmetric:

		a = (MATRIX *) dup_matrix ((MATRIX *) EAT (a));
		b = (MATRIX *) dup_matrix ((MATRIX *) EAT (b));

		if (ijobvr == 'V')
		  {
		    v = (MATRIX *) form_matrix (a->nr, a->nc,
						complex, dense);
		    if (a->cid)
		      {
			v->rid = copy_entity (a->cid);
		      }
		    else if (b->cid)
		      {
			v->rid = copy_entity (b->cid);
		      }
		  }
		else
		  {
		    /* Just need a valid pointer. */
		    v = (MATRIX *) copy_matrix (a);
		  }

		if (ijobvl == 'V')
		  {
		    vl = (MATRIX *) form_matrix (a->nr, a->nc,
						 complex, dense);
		    if (a->rid)
		      {
			vl->rid = copy_entity (a->rid);
		      }
		    else if (b->rid)
		      {
			vl->rid = copy_entity (b->rid);
		      }
		  }
		else
		  {
		    /* Just need a valid pointer. */
		    vl = (MATRIX *) copy_matrix (a);
		  }

		num = (VECTOR *) form_vector (a->nr, complex, dense);
		den = (VECTOR *) form_vector (a->nr, complex, dense);

		/* Call to get workspace size. */

		lwork = -1;
		rwork = E_MALLOC (8*a->nr, real);

		XGGEV (&ijobvl, &ijobvr,
		       &a->nr, a->a.complex, &a->nr,
		       b->a.complex, &b->nr,
		       num->a.complex, den->a.complex,
		       vl->a.complex, &vl->nr, v->a.complex, &v->nr,
		       (COMPLEX *) &ws, &lwork, rwork, &info);

		lwork = (int) ws.real;
		work = E_MALLOC (lwork, complex);

		SIGINT_RAISE_ON ();
		XGGEV (&ijobvl, &ijobvr,
		       &a->nr, a->a.complex, &a->nr,
		       b->a.complex, &b->nr,
		       num->a.complex, den->a.complex,
		       vl->a.complex, &vl->nr, v->a.complex, &v->nr,
		       (COMPLEX *) work, &lwork, rwork, &info);
		SIGINT_RAISE_OFF ();
		FREE (work); work = NULL;
		FREE (rwork); rwork = NULL;

		if (info)
		  {
		    if (info < 0)
		      fail ("Illegal value in %d%s argument to ZGGEV.  Please report this bug.",
			    -info, TH (-info));
		    else if (info <= a->nr)
		      fail ("The QZ iteration failed in ZGGEV.");
		    else
		      fail ("Internal error in ZGGEV.");

		    raise_exception ();
		  }

		tt = (TABLE *) replace_in_table ((TABLE *) make_table (),
						 EAT (num),
						 dup_char ("num"));
		tt = (TABLE *) replace_in_table (EAT (tt),
						 EAT (den),
						 dup_char ("denom"));

		t = (TABLE *) replace_in_table ((TABLE *) make_table (),
						EAT (tt),
						dup_char ("values"));

		if (ijobvr == 'V')
		  {
		    delete_entity (v->cid);
		    v->cid = NULL;
		    t = (TABLE *) replace_in_table (EAT (t),
						    EAT (v),
						    dup_char ("vectors"));
		  }

		if (ijobvl == 'V')
		  {
		    delete_entity (vl->cid);
		    vl->cid = NULL;
		    t = (TABLE *) replace_in_table (
						     EAT (t), EAT (vl),
						 dup_char ("left_vectors"));
		  }

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
      case sparse:
      case sparse_upper:
	detour ("Can't yet find eigenvalues of a %s matrix.",
		density_string[a->density]);
	raise_exception ();
      default:
	BAD_DENSITY (a->density);
	raise_exception ();
      }

  }
  ON_EXCEPTION
  {
    if (work != NULL)
      FREE (work);
    if (rwork != NULL)
      FREE (rwork);
    delete_3_matrices (a, b, v);
    delete_vector (w);
    delete_3_tables (opt, t, tt);
    delete_matrix (vl);
    delete_2_vectors (num, den);
  }
  END_EXCEPTION;


  delete_3_matrices (a, b, v);
  delete_matrix (vl);
  delete_vector (w);
  delete_table (opt);
  return (ENT (t));
}

#if LAPACK_1_0

void
DGEGV (void)
{
  /*
   * This routine SHOULD NEVER BE CALLED.  It's here so folks with old
   * versions of LAPACK won't get unresolved externals when linking.
   * It would be better to have conditional compilation in lapack.f,
   * but you know how that goes...
   */
}

#endif
