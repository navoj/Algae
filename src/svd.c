/*
   svd.c -- singular value decomposition

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: svd.c,v 1.3 2003/08/30 17:47:50 ksh Exp $";

#include "svd.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "transpose.h"
#include "lapack.h"
#include "dense.h"
#include "cast.h"

/*
 * The following character array and enum specify the options
 * accepted by the `svd' function.  Except for the NULL that
 * is required at the end of the array, the elements of the
 * array and of the enum must match one-to-one.
 */

static char *svd_options[] =
{
  "novectors",
  "full",
  NULL,
};
enum
  {
    SVD_OPT_NOVEC, SVD_OPT_FULL
  };

ENTITY *
bi_svd (int n, ENTITY *p, ENTITY *t)
{
  EASSERT (p, 0, 0);

  if (t != NULL && t->class != table)
    {
      fail ("Invalid options -- not a table.");
      delete_2_entities (p, t);
      raise_exception ();
    }

  switch (p->class)
    {
    case scalar:
    case vector:
      p = matrix_entity (p);	/* fall through */
    case matrix:
      return (svd_matrix ((MATRIX *) p, (TABLE *) t));
    default:
      fail ("Can't apply \"svd\" to a %s.",
	    class_string[p->class]);
      delete_2_entities (p, t);
      raise_exception ();
    }
}

ENTITY *
svd_matrix (MATRIX *p, TABLE *t)
{
  /* Compute the Singular Value Decomposition */

  ENTITY *result = NULL;
  VECTOR *s = NULL;
  MATRIX *u = NULL;
  MATRIX *vt = NULL;
  int nr, nc, ns, ijobu, ijobvt, lwork, info;
  COMPLEX dwork;
  void *work = NULL;
  REAL *rwork = NULL;

  int vectors, full;

  EASSERT (p, matrix, 0);

  WITH_HANDLING
  {

    /* Gotta have elements. */

    if (!p->nr || !p->nc)
      {
	fail ("Matrix has no elements.");
	raise_exception ();
      }

    /* Check args. */

    if (!OK_OPTIONS (t, svd_options))
      {
	raise_exception ();
      }

    /* Options for singular vectors. */

    vectors = !IN_TABLE (t, svd_options[SVD_OPT_NOVEC]);
    full = IN_TABLE (t, svd_options[SVD_OPT_FULL]);

    /* Deal with dense only. */

    if (p->density != dense)
      p = (MATRIX *) dense_matrix (EAT (p));

    ns = (p->nr > p->nc) ? p->nc : p->nr;	/* # of values */

    switch (p->type)
      {

      case integer:
	p = (MATRIX *) cast_matrix_integer_real (EAT (p));
	/* fall through */

      case real:

	if (vectors)
	  {
	    if (full)
	      {
		ijobu = 'A';	/* all left vectors  */
		ijobvt = 'A';	/* all right vectors */
		nr = p->nr;
		nc = p->nc;
	      }
	    else
	      {
		ijobu = 'S';	/* compact left vectors */
		ijobvt = 'S';	/* compact right vectors */
		nr = nc = ns;
	      }
	  }
	else
	  {
	    ijobu = 'N';	/* no left vectors */
	    ijobvt = 'N';	/* no right vectors */
	    nr = nc = 1;
	  }

	u = (MATRIX *) form_matrix (p->nr, nr, real, dense);
	vt = (MATRIX *) form_matrix (nc, p->nc, real, dense);
	s = (VECTOR *) form_vector (ns, real, dense);

	/* RGESVD overwrites the original matrix, so dup it. */

	p = (MATRIX *) dup_matrix (EAT (p));

	/* Have DGESVD determine workspace size. */

	lwork = -1;
	RGESVD (&ijobu, &ijobvt, &p->nr, &p->nc, p->a.real, &p->nr,
		s->a.real, u->a.real, &u->nr, vt->a.real, &nc,
		&dwork.real, &lwork, &info);

	/* Now allocate workspace and try it again. */

	lwork = (int) dwork.real;
	work = E_MALLOC (lwork, real);
	SIGINT_RAISE_ON ();
	RGESVD (&ijobu, &ijobvt, &p->nr, &p->nc, p->a.real, &p->nr,
		s->a.real, u->a.real, &u->nr, vt->a.real, &nc,
		work, &lwork, &info);
	SIGINT_RAISE_OFF ();
	FREE (work);
	work = NULL;

	if (info != 0)
	  {
	    if (info < 0)
	      {
		wipeout ("Bad argument to DGESVD.");
	      }
	    else
	      {
		fail ("No convergence in \"svd\".");
	      }
	  }

	result = make_table ();
	result = replace_in_table (EAT (result), EAT (s),
				   dup_char ("sigma"));
	if (vectors)
	  {
	    result = replace_in_table (EAT (result), EAT (u),
				       dup_char ("U"));
	    result = replace_in_table (EAT (result),
				       transpose_matrix (EAT (vt)),
				       dup_char ("V"));
	  }
	break;

      case complex:

	if (vectors)
	  {
	    if (full)
	      {
		ijobu = 'A';	/* all left vectors  */
		ijobvt = 'A';	/* all right vectors */
		nr = p->nr;
		nc = p->nc;
	      }
	    else
	      {
		ijobu = 'S';	/* compact left vectors */
		ijobvt = 'S';	/* compact right vectors */
		nr = nc = ns;
	      }
	  }
	else
	  {
	    ijobu = 'N';	/* no left vectors */
	    ijobvt = 'N';	/* no right vectors */
	    nr = nc = 1;
	  }

	u = (MATRIX *) form_matrix (p->nr, nr, complex, dense);
	vt = (MATRIX *) form_matrix (nc, p->nc, complex, dense);
	s = (VECTOR *) form_vector (ns, real, dense);

	/* ZGESVD overwrites the original matrix, so dup it. */

	p = (MATRIX *) dup_matrix (EAT (p));

	/* Have ZGESVD determine workspace size. */

	lwork = -1;
	XGESVD (&ijobu, &ijobvt, &p->nr, &p->nc, p->a.complex, &p->nr,
		s->a.real, u->a.complex, &u->nr, vt->a.complex,
		&nc, &dwork, &lwork, rwork, &info);

	/* Now allocate workspace and try it again. */

	lwork = (int) dwork.real;
	work = E_MALLOC (lwork, complex);
	rwork = E_MALLOC (5 * ((p->nr > p->nc) ? p->nr : p->nc), real);
	SIGINT_RAISE_ON ();
	XGESVD (&ijobu, &ijobvt, &p->nr, &p->nc, p->a.complex, &p->nr,
		s->a.real, u->a.complex, &u->nr, vt->a.complex,
		&nc, work, &lwork, rwork, &info);
	SIGINT_RAISE_OFF ();
	FREE (work);
	work = NULL;
	FREE (rwork);
	rwork = NULL;

	if (info != 0)
	  {
	    if (info < 0)
	      {
		wipeout ("Bad argument to ZGESVD.");
	      }
	    else
	      {
		fail ("No convergence in \"svd\".");
	      }
	  }

	result = make_table ();
	result = replace_in_table (EAT (result), EAT (s),
				   dup_char ("sigma"));
	if (vectors)
	  {
	    result = replace_in_table (EAT (result), EAT (u),
				       dup_char ("U"));
	    result = replace_in_table (EAT (result),
				       hermitian_transpose_matrix (EAT (vt)),
				       dup_char ("V"));
	  }
	break;

      case character:
	fail ("Can't perform \"svd\" on a character matrix.");
	raise_exception ();

      default:
	BAD_TYPE (p->type);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_table (t);
    delete_3_matrices (p, u, vt);
    delete_vector (s);
    delete_entity (result);
    TFREE (work);
    TFREE (rwork);
  }
  END_EXCEPTION;

  delete_table (t);
  delete_3_matrices (p, u, vt);
  return (ENT (result));
}
