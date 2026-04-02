/*
   power.c --- The binary "power" operation.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: power.c,v 1.5 2003/08/01 04:57:48 ksh Exp $";

#include "power.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "function.h"
#include "table.h"
#include "cast.h"
#include "exp.h"
#include "log.h"
#include "transpose.h"
#include "real.h"
#include "abs.h"
#include "dense.h"
#include "full.h"

static char bad_labels[] = "Labels don't match for power.";
static char bad_vector_dims[] = "Inconsistent dimensions for vector power operation.  First vector has %d element%s; second vector has %d element%s.";
static char bad_matrix_dims[] = "Inconsistent dimensions for matrix power operation.  First matrix has %d row%s and %d column%s; second matrix has %d row%s and %d column%s.";

static char undefined_op[] = "Undefined power operation (0^0).";
static char divide_by_zero[] = "Undefined power operation (1/0).";
static char undefined_cmplx_op[] = "Undefined complex power operation (0^z).";

COMPLEX
power_complex (COMPLEX a, COMPLEX b)
{
  /*
   * Complex exponentiation.  The arg `a' must not be zero.
   * This function was adapted from the f2c "pow_zz" function.
   * Copyright 1990 by AT&T Bell Laboratories and Bellcore.
   */

  COMPLEX r;
  REAL logr, logi, x, y;

  errno = 0;
  logr = xlog (abs_complex (a));
  logi = atan2 (a.imag, a.real);
  CHECK_MATH ();

  x = exp (logr * b.real - logi * b.imag);
  y = logr * b.imag + logi * b.real;

  r.real = x * cos (y);
  r.imag = x * sin (y);

  return r;
}

ENTITY *
power_scalar_vector_integer (SCALAR *l, VECTOR *r)
{
  /* l^r[] --> ( l^r[1], l^r[2], ... ) */

  VECTOR *v = NULL;
  int i;

  EASSERT (l, scalar, integer);
  EASSERT (r, vector, integer);

  WITH_HANDLING
  {
    /* Check for 0^r. */

    if (l->v.integer == 0)
      {
	/* Check for 0^0 or 1/0 */

	for (i = 0; i < r->nn; i++)
	  {
	    if (r->a.integer[i] <= 0)
	      {
		fail (r->a.integer[i] ? divide_by_zero : undefined_op);
		raise_exception ();
	      }
	  }

	v = (VECTOR *) make_vector (r->ne, integer, sparse);
	v = (VECTOR *) apt_vector (EAT (v));
      }
    else
      {
	int neg = 0;

	r = (VECTOR *) dense_vector (EAT (r));

	/* See if there are any negative elements of `r'. */

	for (i = 0; i < r->nn; i++)
	  {
	    if (r->a.integer[i] < 0)
	      {
		neg = 1;
		break;
	      }
	  }

	/* Now do it, converting to real if necessary. */

	if (neg)
	  {
	    v = (VECTOR *) form_vector (r->ne, real, dense);
	    if (r->eid)
	      v->eid = copy_entity (r->eid);
	    for (i = 0; i < v->nn; i++)
	      {
		v->a.real[i] = pow ((REAL) l->v.integer,
				    (REAL) r->a.integer[i]);
	      }
	  }
	else
	  {
	    v = (VECTOR *) dup_vector (EAT (r));
	    for (i = 0; i < v->nn; i++)
	      {
		v->a.integer[i] =
		  (int) round (pow ((REAL) l->v.integer,
				    (REAL) v->a.integer[i]));
	      }
	  }
      }
  }
  ON_EXCEPTION
  {
    delete_scalar (l);
    delete_2_vectors (r, v);
  }
  END_EXCEPTION;

  delete_scalar (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
power_scalar_vector_real (SCALAR *l, VECTOR *r)
{
  /* l^r[] --> ( l^r[1], l^r[2], ... ) */

  VECTOR *v = NULL;
  int i;

  EASSERT (l, scalar, real);
  EASSERT (r, vector, real);

  WITH_HANDLING
  {
    /* Check for 0^r. */

    if (l->v.real == 0.0)
      {
	/* Check for 0^0 or 1/0. */

	for (i = 0; i < r->nn; i++)
	  {
	    if (r->a.real[i] <= 0.0)
	      {
		fail (r->a.real[i] ? divide_by_zero : undefined_op);
		raise_exception ();
	      }
	  }

	v = (VECTOR *) make_vector (r->ne, real, sparse);
	v = (VECTOR *) apt_vector (EAT (v));
      }
    else
      {
	int cmplx = 0;

	r = (VECTOR *) dense_vector ((VECTOR *) EAT (r));

	/* If `l' is negative and `r' not integer, then result complex. */

	if (l->v.real < 0.0)
	  {
	    for (i = 0; i < r->nn; i++)
	      {
		if ((int) r->a.real[i] != r->a.real[i])
		  {
		    cmplx = 1;
		    break;
		  }
	      }
	  }

	/* Do it, converting to complex if necessary. */

	if (cmplx)
	  {
	    COMPLEX a, b;
	    v = (VECTOR *) form_vector (r->ne, complex, dense);
	    if (r->eid)
	      v->eid = copy_entity (r->eid);
	    a.real = l->v.real;
	    a.imag = b.imag = 0.0;
	    for (i = 0; i < v->nn; i++)
	      {
		b.real = r->a.real[i];
		v->a.complex[i] = power_complex (a, b);
	      }
	  }
	else
	  {
	    v = (VECTOR *) dup_vector ((VECTOR *) EAT (r));
	    for (i = 0; i < v->nn; i++)
	      v->a.real[i] = pow (l->v.real, v->a.real[i]);
	  }
      }
  }
  ON_EXCEPTION
  {
    delete_scalar (l);
    delete_2_vectors (r, v);
  }
  END_EXCEPTION;

  delete_scalar (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
power_scalar_vector_complex (SCALAR *l, VECTOR *r)
{
  /* l^r[] --> ( l^r[1], l^r[2], ... ) */

  VECTOR * volatile v = NULL;
  int i;

  EASSERT (l, scalar, complex);
  EASSERT (r, vector, complex);

  WITH_HANDLING
  {
    v = (VECTOR *) dense_vector ((VECTOR *) EAT (r));

    /* check for 0^z */

    if (!(l->v.complex.real || l->v.complex.imag))
      {
	fail (undefined_cmplx_op);
	raise_exception ();
      }
    else
      {
	COMPLEX a;
	a = l->v.complex;
	for (i = 0; i < v->nn; i++)
	  v->a.complex[i] = power_complex (a, v->a.complex[i]);
      }
  }
  ON_EXCEPTION
  {
    delete_scalar (l);
    delete_2_vectors (r, v);
  }
  END_EXCEPTION;

  delete_scalar (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
power_vector_scalar_integer (VECTOR *l, SCALAR *r)
{
  /* l[]^r --> ( l[1]^r, l[2]^r, ... ) */

  VECTOR * volatile v = NULL;
  int i;

  EASSERT (l, vector, integer);
  EASSERT (r, scalar, integer);

  WITH_HANDLING
  {
    /* Check for l^0 */

    if (r->v.integer == 0)
      {
	/* Result must be dense. */

	v = (VECTOR *) dup_vector ((VECTOR *) dense_vector (EAT (l)));

	for (i = 0; i < v->nn; i++)
	  {
	    /* Check for 0^0. */

	    if (v->a.integer[i] == 0)
	      {
		fail (undefined_op);
		raise_exception ();
	      }

	    v->a.integer[i] = 1;
	  }
      }
    else
      {
	if (r->v.integer < 0)
	  {
	    /*
	     * Check for 1/0.  The `pow' function will do this
	     * for us, but we still have to check for the case of
	     * a sparse vector.
	     */

	    if (l->density == sparse && l->nn < l->ne)
	      {
		fail (divide_by_zero);
		raise_exception ();
	      }

	    /* convert to real and do it */

	    v = (VECTOR *) cast_vector_integer_real (EAT (l));
	    for (i = 0; i < v->nn; i++)
	      {
		v->a.real[i] = pow (v->a.real[i], (REAL) r->v.integer);
	      }
	  }
	else
	  {
	    v = (VECTOR *) dup_vector (EAT (l));
	    for (i = 0; i < v->nn; i++)
	      {
		v->a.integer[i] = (int) round (pow ((REAL) v->a.integer[i],
						    (REAL) r->v.integer));
	      }
	  }
      }
  }
  ON_EXCEPTION
  {
    delete_scalar (r);
    delete_2_vectors (l, v);
  }
  END_EXCEPTION;

  delete_vector (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
power_vector_scalar_real (VECTOR *l, SCALAR *r)
{
  /* l[]^r --> ( l[1]^r, l[2]^r, ... ) */

  VECTOR * volatile v = NULL;
  int i;

  EASSERT (l, vector, real);
  EASSERT (r, scalar, real);

  WITH_HANDLING
  {
    if (r->v.real == 0.0)
      {
	/* Result must be dense. */

	v = (VECTOR *) dup_vector ((VECTOR *) dense_vector (EAT (l)));

	for (i = 0; i < v->nn; i++)
	  {
	    /* Check for 0^0. */

	    if (v->a.real[i] == 0.0)
	      {
		fail (undefined_op);
		raise_exception ();
	      }

	    v->a.real[i] = 1.0;
	  }
      }
    else
      {
	int cmplx = 0;

	if (r->v.real < 0.0)
	  {
	    /*
	     * Check for 1/0.  The `pow' function will do this
	     * for us, but we still have to check for the case of
	     * a sparse vector.
	     */

	    if (l->density == sparse && l->nn < l->ne)
	      {
		fail (divide_by_zero);
		raise_exception ();
	      }
	  }

	/*
	 * If `l' has a negative term and `r' is not integer, then
	 * result is complex.
	 */

	if ((int) r->v.real != r->v.real)
	  {
	    for (i = 0; i < l->nn; i++)
	      {
		if (l->a.real[i] < 0.0)
		  {
		    cmplx = 1;
		    break;
		  }
	      }
	  }

	if (cmplx)
	  {
	    COMPLEX b;

	    b.real = r->v.real;
	    b.imag = 0.0;

	    v = (VECTOR *) cast_vector_real_complex (EAT (l));
	    for (i = 0; i < v->nn; i++)
	      {
		if (v->a.complex[i].real < 0.0)
		  {
		    v->a.complex[i] = power_complex (v->a.complex[i], b);
		  }
		else
		  {
		    v->a.complex[i].real =
		      pow (v->a.complex[i].real, r->v.real);
		    v->a.complex[i].imag = 0.0;
		  }
	      }
	  }
	else
	  {
	    v = (VECTOR *) dup_vector (EAT (l));
	    for (i = 0; i < v->nn; i++)
	      {
		v->a.real[i] = pow (v->a.real[i], r->v.real);
	      }
	  }
      }
  }
  ON_EXCEPTION
  {
    delete_scalar (r);
    delete_2_vectors (l, v);
  }
  END_EXCEPTION;

  delete_vector (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
power_vector_scalar_complex (VECTOR *l, SCALAR *r)
{
  /* l[]^r --> ( l[1]^r, l[2]^r, ... ) */

  VECTOR * volatile v = NULL;
  int i;

  EASSERT (l, vector, complex);
  EASSERT (r, scalar, complex);

  WITH_HANDLING
  {
    /* check for 0^z */

    for (i = 0; i < l->nn; i++)
      {
	if (!(l->a.complex[i].real || l->a.complex[i].imag))
	  {
	    fail (undefined_cmplx_op);
	    raise_exception ();
	  }
      }

    if (l->density == sparse && l->nn < l->ne)
      {
	fail (undefined_cmplx_op);
	raise_exception ();
      }

    /* check for z^0 */

    if (!(r->v.complex.real || r->v.complex.imag))
      {
	/* No zeros in `l', so return l^0. */

	v = (VECTOR *) form_vector (l->ne, complex, dense);
	if (l->eid)
	  v->eid = copy_entity (l->eid);
	for (i = 0; i < v->nn; i++)
	  {
	    v->a.complex[i].real = 1.0;
	    v->a.complex[i].imag = 0.0;
	  }
      }
    else
      {
	v = (VECTOR *) dup_vector (EAT (l));

	for (i = 0; i < v->nn; i++)
	  {
	    v->a.complex[i] =
	      power_complex (v->a.complex[i], r->v.complex);
	  }

      }
  }
  ON_EXCEPTION
  {
    delete_scalar (r);
    delete_2_vectors (l, v);
  }
  END_EXCEPTION;

  delete_vector (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
power_vector_integer (VECTOR *l, VECTOR *r)
{
  /* l[]^r[] --> ( l[1]^r[1], l[2]^r[2], ... ) */

  ENTITY * volatile eid = NULL;
  VECTOR * volatile v = NULL;
  int neg = 0;
  int i, j, k;

  EASSERT (l, vector, integer);
  EASSERT (r, vector, integer);

  WITH_HANDLING
  {
    if (l->ne != r->ne)
      {
	fail (bad_vector_dims,
	      l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->eid, r->eid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    /* Save copy of labels, to put on later. */

    if (l->eid != NULL)
      eid = copy_entity (l->eid);
    else if (r->eid != NULL)
      eid = copy_entity (r->eid);

    /* Check for negative elements in `r'. */

    for (i = 0; i < r->nn; i++)
      {
	if (r->a.integer[i] < 0)
	  {
	    neg = 1;
	    break;
	  }
      }

    r = (VECTOR *) dense_vector (EAT (r));

    switch (l->density)
      {
      case dense:

	if (neg)
	  {
	    v = (VECTOR *) form_vector (r->ne, real, dense);
	    for (i = 0; i < v->nn; i++)
	      {
		if (!(l->a.integer[i] || r->a.integer[i]))
		  {
		    fail (undefined_op);
		    raise_exception ();
		  }
		v->a.real[i] = pow ((REAL) l->a.integer[i],
				    (REAL) r->a.integer[i]);
	      }
	  }
	else
	  {
	    v = (VECTOR *) dup_vector (EAT (r));
	    for (i = 0; i < v->nn; i++)
	      {
		if (!(l->a.integer[i] || v->a.integer[i]))
		  {
		    fail (undefined_op);
		    raise_exception ();
		  }
		v->a.integer[i] =
		  (int) round (pow ((REAL) l->a.integer[i],
				    (REAL) v->a.integer[i]));
	      }
	  }
	break;

      case sparse:

	/* Look for 0^0 and 1/0. */

	assert (l->order == ordered);
	k = 1;
	for (i = 0; i < l->nn; i++)
	  {
	    for (j = k; j < l->ja[i]; j++)
	      {
		if (r->a.integer[j - 1] <= 0)
		  {
		    fail (r->a.integer[j - 1] ?
			  divide_by_zero : undefined_op);
		    raise_exception ();
		  }
	      }
	    k = l->ja[i] + 1;
	  }
	for (j = k; j <= l->ne; j++)
	  {
	    if (r->a.integer[j - 1] <= 0)
	      {
		fail (r->a.integer[j - 1] ? divide_by_zero : undefined_op);
		raise_exception ();
	      }
	  }

	if (neg)
	  {
	    v = (VECTOR *) cast_vector_integer_real (EAT (l));
	    for (i = 0; i < v->nn; i++)
	      {
		if (!(v->a.integer[i] || r->a.integer[v->ja[i] - 1]))
		  {
		    fail (undefined_op);
		    raise_exception ();
		  }
		v->a.real[i] = pow (v->a.real[i],
				    (REAL) r->a.integer[v->ja[i] - 1]);
	      }
	  }
	else
	  {
	    v = (VECTOR *) dup_vector (EAT (l));
	    for (i = 0; i < v->nn; i++)
	      {
		if (!(v->a.integer[i] || r->a.integer[v->ja[i] - 1]))
		  {
		    fail (undefined_op);
		    raise_exception ();
		  }
		v->a.integer[i] =
		  (int) round (pow ((REAL) v->a.integer[i],
				    (REAL) r->a.integer[v->ja[i] - 1]));
	      }
	  }
	break;

      default:
	BAD_DENSITY (l->density);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_entity (eid);
    delete_3_vectors (l, r, v);
  }
  END_EXCEPTION;

  delete_entity (v->eid);
  v->eid = eid;

  delete_2_vectors (l, r);
  return (ENT (v));
}

ENTITY *
power_vector_real (VECTOR *l, VECTOR *r)
{
  /* l[]^r[] --> ( l[1]^r[1], l[2]^r[2], ... ) */

  ENTITY * volatile eid = NULL;
  VECTOR * volatile v = NULL;
  int cmplx = 0;
  int i, j, k;

  EASSERT (l, vector, real);
  EASSERT (r, vector, real);

  WITH_HANDLING
  {
    if (l->ne != r->ne)
      {
	fail (bad_vector_dims,
	      l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->eid, r->eid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    if (l->eid != NULL)
      {
	eid = copy_entity (l->eid);
      }
    else if (r->eid != NULL)
      {
	eid = copy_entity (r->eid);
      }

    r = (VECTOR *) dense_vector (EAT (r));

    switch (l->density)
      {
      case dense:

	/* See if result is complex. */

	for (i = 0; i < l->nn; i++)
	  {
	    if (l->a.real[i] < 0.0 &&
		(int) r->a.real[i] != r->a.real[i])
	      {
		cmplx = 1;
		break;
	      }
	  }

	if (cmplx)
	  {
	    COMPLEX a, b;
	    a.imag = b.imag = 0.0;

	    v = (VECTOR *) form_vector (r->ne, complex, dense);
	    for (i = 0; i < v->nn; i++)
	      {
		if (l->a.real[i] == 0.0 && r->a.real[i] <= 0.0)
		  {
		    fail (r->a.real[i] ? divide_by_zero : undefined_op);
		    raise_exception ();
		  }
		if (l->a.real[i] < 0.0)
		  {
		    a.real = l->a.real[i];
		    b.real = r->a.real[i];

		    v->a.complex[i] = power_complex (a, b);
		  }
		else
		  {
		    if (!(l->a.real[i] || r->a.real[i]))
		      {
			fail (undefined_op);
			raise_exception ();
		      }
		    v->a.complex[i].real =
		      pow (l->a.real[i], r->a.real[i]);
		    v->a.complex[i].imag = 0.0;
		  }
	      }
	  }
	else
	  {
	    v = (VECTOR *) dup_vector (EAT (r));
	    for (i = 0; i < v->nn; i++)
	      {
		if (!(l->a.real[i] || v->a.real[i]))
		  {
		    fail (undefined_op);
		    raise_exception ();
		  }
		v->a.real[i] = pow (l->a.real[i], v->a.real[i]);
	      }
	  }
	break;

      case sparse:

	/* Look for 0^0 and 1/0. */

	assert (l->order == ordered);
	k = 1;
	for (i = 0; i < l->nn; i++)
	  {
	    for (j = k; j < l->ja[i]; j++)
	      {
		if (r->a.real[j - 1] <= 0.0)
		  {
		    fail (r->a.integer[j - 1] ?
			  divide_by_zero : undefined_op);
		    raise_exception ();
		  }
	      }
	    k = l->ja[i] + 1;
	  }
	for (j = k; j <= l->ne; j++)
	  {
	    if (r->a.real[j - 1] <= 0.0)
	      {
		fail (r->a.integer[j - 1] ? divide_by_zero : undefined_op);
		raise_exception ();
	      }
	  }

	/* See if result is complex. */

	for (i = 0; i < l->nn; i++)
	  {
	    if (l->a.real[i] < 0.0 &&
		(int) r->a.real[l->ja[i] - 1] != r->a.real[l->ja[i] - 1])
	      {
		cmplx = 1;
		break;
	      }
	  }

	if (cmplx)
	  {
	    COMPLEX b;
	    b.imag = 0.0;

	    v = (VECTOR *) cast_vector_real_complex (EAT (l));
	    for (i = 0; i < v->nn; i++)
	      {
		if (!(v->a.complex[i].real || r->a.real[v->ja[i] - 1]))
		  {
		    fail (undefined_op);
		    raise_exception ();
		  }
		if (v->a.complex[i].real < 0.0)
		  {
		    b.real = r->a.real[v->ja[i] - 1];
		    v->a.complex[i] = power_complex (v->a.complex[i], b);
		  }
		else
		  {
		    v->a.complex[i].real =
		      pow (v->a.complex[i].real, r->a.real[v->ja[i] - 1]);
		    v->a.complex[i].imag = 0.0;
		  }
	      }
	  }
	else
	  {
	    v = (VECTOR *) dup_vector (EAT (l));
	    for (i = 0; i < v->nn; i++)
	      {
		if (!(v->a.real[i] || r->a.real[v->ja[i] - 1]))
		  {
		    fail (undefined_op);
		    raise_exception ();
		  }
		v->a.real[i] = pow (v->a.real[i],
				    r->a.real[v->ja[i] - 1]);
	      }
	  }
	break;

      default:
	BAD_DENSITY (l->density);
	raise_exception ();
      }

  }
  ON_EXCEPTION
  {
    delete_entity (eid);
    delete_3_vectors (l, r, v);
  }
  END_EXCEPTION;

  delete_entity (v->eid);
  v->eid = eid;

  delete_2_vectors (l, r);
  return (ENT (v));
}

ENTITY *
power_vector_complex (VECTOR *l, VECTOR *r)
{
  /* l[]^r[] --> ( l[1]^r[1], l[2]^r[2], ... ) */

  ENTITY * volatile eid = NULL;
  VECTOR * volatile v = NULL;
  int i;
  COMPLEX a, b;

  EASSERT (l, vector, complex);
  EASSERT (r, vector, complex);

  WITH_HANDLING
  {
    if (l->ne != r->ne)
      {
	fail (bad_vector_dims,
	      l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->eid, r->eid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    if (l->eid != NULL)
      {
	eid = copy_entity (l->eid);
      }
    else if (r->eid != NULL)
      {
	eid = copy_entity (r->eid);
      }

    l = (VECTOR *) dense_vector (EAT (l));
    r = (VECTOR *) dense_vector (EAT (r));

    v = (VECTOR *) dup_vector (EAT (r));
    for (i = 0; i < v->nn; i++)
      {
	a = l->a.complex[i];
	b = v->a.complex[i];
	if (!(a.real || a.imag))
	  {
	    fail (undefined_cmplx_op);
	    raise_exception ();
	  }
	v->a.complex[i] = power_complex (a, b);
      }
  }
  ON_EXCEPTION
  {
    delete_entity (eid);
    delete_3_vectors (l, r, v);
  }
  END_EXCEPTION;

  delete_entity (v->eid);
  v->eid = eid;

  delete_2_vectors (l, r);
  return (ENT (v));
}

ENTITY *
power_scalar_matrix_integer (SCALAR *l, MATRIX *r)
{
  /* l^r[;] --> [ l^r[1;1], l^r[1;2], ... ; l^r[2;1] , ... ] */

  MATRIX *v = NULL;
  int i;

  EASSERT (l, scalar, integer);
  EASSERT (r, matrix, integer);

  WITH_HANDLING
  {
    if (l->v.integer == 0)
      {
	/* Check for 0^0 or 1/0 */

	for (i = 0; i < r->nn; i++)
	  {
	    if (r->a.integer[i] <= 0)
	      {
		fail (r->a.integer[i] ? divide_by_zero : undefined_op);
		raise_exception ();
	      }
	  }

	v = (MATRIX *) make_matrix (r->nr, r->nc, integer, sparse);
	v = (MATRIX *) apt_matrix (EAT (v));
      }
    else
      {
	int neg = 0;

	r = (MATRIX *) dense_matrix (EAT (r));

	/* See if there are any negative elements of `r'. */

	for (i = 0; i < r->nn; i++)
	  {
	    if (r->a.integer[i] < 0)
	      {
		neg = 1;
		break;
	      }
	  }

	/* Now do it, converting to real if necessary. */

	if (neg)
	  {
	    v = (MATRIX *) form_matrix (r->nr, r->nc, real, dense);
	    v->symmetry = r->symmetry;
	    if (r->rid)
	      v->rid = copy_entity (r->rid);
	    if (r->cid)
	      v->cid = copy_entity (r->cid);
	    for (i = 0; i < v->nn; i++)
	      {
		v->a.real[i] = pow ((REAL) l->v.integer,
				    (REAL) r->a.integer[i]);
	      }
	  }
	else
	  {
	    v = (MATRIX *) dup_matrix (EAT (r));
	    for (i = 0; i < v->nn; i++)
	      {
		v->a.integer[i] =
		  (int) round (pow ((REAL) l->v.integer,
				    (REAL) v->a.integer[i]));
	      }
	  }
      }
  }
  ON_EXCEPTION
  {
    delete_scalar (l);
    delete_2_matrices (r, v);
  }
  END_EXCEPTION;

  delete_scalar (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
power_scalar_matrix_real (SCALAR *l, MATRIX *r)
{
  /* l^r[;] --> [ l^r[1;1], l^r[1;2], ... ; l^r[2;1] , ... ] */

  MATRIX *v = NULL;
  int i;

  EASSERT (l, scalar, real);
  EASSERT (r, matrix, real);

  WITH_HANDLING
  {
    if (l->v.real == 0)
      {
	/* Check for 0^0 or 1/0 */

	for (i = 0; i < r->nn; i++)
	  {
	    if (r->a.real[i] <= 0.0)
	      {
		fail (r->a.real[i] ? divide_by_zero : undefined_op);
		raise_exception ();
	      }
	  }

	v = (MATRIX *) make_matrix (r->nr, r->nc, real, sparse);
	v = (MATRIX *) apt_matrix (EAT (v));
      }
    else
      {
	int cmplx = 0;

	r = (MATRIX *) dense_matrix ((MATRIX *) EAT (r));

	/* If `l' is negative and `r' not integer, then result complex. */

	if (l->v.real < 0.0)
	  {
	    for (i = 0; i < r->nn; i++)
	      {
		if ((int) r->a.real[i] != r->a.real[i])
		  {
		    cmplx = 1;
		    break;
		  }
	      }
	  }

	/* Do it, converting to complex if necessary. */

	if (cmplx)
	  {
	    COMPLEX a, b;
	    v = (MATRIX *) form_matrix (r->nr, r->nc, complex, dense);
	    v->symmetry = r->symmetry;
	    if (r->rid)
	      v->rid = copy_entity (r->rid);
	    if (r->cid)
	      v->cid = copy_entity (r->cid);
	    a.real = l->v.real;
	    a.imag = b.imag = 0.0;
	    for (i = 0; i < v->nn; i++)
	      {
		b.real = r->a.real[i];
		v->a.complex[i] = power_complex (a, b);
	      }
	  }
	else
	  {
	    v = (MATRIX *) dup_matrix ((MATRIX *) EAT (r));
	    for (i = 0; i < v->nn; i++)
	      {
		v->a.real[i] = pow (l->v.real, v->a.real[i]);
	      }
	  }
	CHECK_MATH ();
      }
  }
  ON_EXCEPTION
  {
    delete_scalar (l);
    delete_2_matrices (r, v);
  }
  END_EXCEPTION;

  delete_scalar (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
power_scalar_matrix_complex (SCALAR *l, MATRIX *r)
{
  /* l^r[;] --> [ l^r[1;1], l^r[1;2], ... ; l^r[2;1] , ... ] */

  MATRIX * volatile v = NULL;
  int i;

  EASSERT (l, scalar, complex);
  EASSERT (r, matrix, complex);

  WITH_HANDLING
  {
    int dgnl = r->nn == 0 &&
      (r->symmetry == symmetric || r->symmetry == hermitian);

    v = (MATRIX *) dense_matrix ((MATRIX *) EAT (r));

    /* check for 0^z */

    if (!(l->v.complex.real || l->v.complex.imag))
      {
	fail (undefined_cmplx_op);
	raise_exception ();
      }
    else
      {
	COMPLEX a;

	/*
	 * If it's hermitian, then it goes to symmetric if it's diagonal
	 * or general otherwise.
	 */

	if (v->symmetry == hermitian && l->v.complex.imag != 0.0)
	  v->symmetry = dgnl ? symmetric : general;

	a = l->v.complex;
	for (i = 0; i < v->nn; i++)
	  v->a.complex[i] = power_complex (a, v->a.complex[i]);
      }
  }
  ON_EXCEPTION
  {
    delete_scalar (l);
    delete_2_matrices (r, v);
  }
  END_EXCEPTION;

  delete_scalar (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
power_matrix_scalar_integer (MATRIX *l, SCALAR *r)
{
  /* l[;]^r --> [ l[1;1]^r, l[1;2]^r, ... ; l[2;1]^r, ... ] */

  MATRIX * volatile v = NULL;
  int i;

  EASSERT (l, matrix, integer);
  EASSERT (r, scalar, integer);

  WITH_HANDLING
  {
    /* Check for l^0 */

    if (r->v.integer == 0)
      {
	/* Result must be dense. */

	v = (MATRIX *) dup_matrix ((MATRIX *) dense_matrix (EAT (l)));

	/* Check for 0^0. */

	for (i = 0; i < v->nn; i++)
	  {
	    if (v->a.integer[i] == 0)
	      {
		fail (undefined_op);
		raise_exception ();
	      }

	    v->a.integer[i] = 1;
	  }

	/* Matrix is all ones -- if it's square then it's symmetric. */

	if (v->nr == v->nc)
	  v->symmetry = symmetric;
      }
    else
      {
	if (r->v.integer < 0)
	  {
	    /*
	     * Check for 1/0.  The `pow' function will do this
	     * for us, but we still have to check for the case of
	     * a sparse vector.
	     */

	    if (l->density == sparse &&
                l->nn < (double) l->nr * l->nc ||
		l->density == sparse_upper &&
		(l->symmetry == general ||
		 2 * l->nn + l->nr < (double) l->nr * l->nc))
	      {
		fail (divide_by_zero);
		raise_exception ();
	      }

	    /* convert to real and do it */

	    v = (MATRIX *) cast_matrix_integer_real (EAT (l));
	    for (i = 0; i < v->nn; i++)
	      {
		v->a.real[i] = pow (v->a.real[i], (REAL) r->v.integer);
	      }
	    if (v->d.integer)
	      {
		for (i = 0; i < v->nr; i++)
		  {
		    v->d.real[i] = pow (v->d.real[i], (REAL) r->v.integer);
		  }
	      }
	  }
	else
	  {
	    v = (MATRIX *) dup_matrix (EAT (l));
	    for (i = 0; i < v->nn; i++)
	      {
		v->a.integer[i] = (int) round (pow ((REAL) v->a.integer[i],
						    (REAL) r->v.integer));
	      }
	    if (v->d.integer)
	      {
		for (i = 0; i < v->nr; i++)
		  {
		    v->d.integer[i] =
		      (int) round (pow ((REAL) v->d.integer[i],
					(REAL) r->v.integer));
		  }
	      }

	  }
      }
  }
  ON_EXCEPTION
  {
    delete_scalar (r);
    delete_2_matrices (l, v);
  }
  END_EXCEPTION;

  delete_matrix (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
power_matrix_scalar_real (MATRIX *l, SCALAR *r)
{
  /* l[;]^r --> [ l[1;1]^r, l[1;2]^r, ... ; l[2;1]^r, ... ] */

  MATRIX * volatile v = NULL;
  int i;

  EASSERT (l, matrix, real);
  EASSERT (r, scalar, real);

  WITH_HANDLING
  {
    if (r->v.real == 0.0)
      {
	/* Result must be dense. */

	v = (MATRIX *) dup_matrix ((MATRIX *) dense_matrix (EAT (l)));

	for (i = 0; i < v->nn; i++)
	  {
	    /* Check for 0^0. */

	    if (v->a.real[i] == 0.0)
	      {
		fail (undefined_op);
		raise_exception ();
	      }

	    v->a.real[i] = 1.0;
	  }

	/* Matrix is all ones -- if it's square then it's symmetric. */

	if (v->nr == v->nc)
	  v->symmetry = symmetric;
      }
    else
      {
	int cmplx = 0;

	if (r->v.real < 0.0)
	  {
	    /*
	     * Check for 1/0.  The `pow' function will do this
	     * for us, but we still have to check for the case of
	     * a sparse vector.
	     */

	    if (l->density == sparse &&
                l->nn < (double) l->nr * l->nc ||
		l->density == sparse_upper &&
		(l->symmetry == general || !l->d.real ||
		 2 * l->nn + l->nr < (double) l->nr * l->nc))
	      {
		fail (divide_by_zero);
		raise_exception ();
	      }
	  }

	/*
	 * If `l' has a negative term and `r' is not integer, then
	 * result is complex.
	 */

	if ((int) r->v.real != r->v.real)
	  {
	    for (i = 0; i < l->nn; i++)
	      {
		if (l->a.real[i] < 0.0)
		  {
		    cmplx = 1;
		    break;
		  }
	      }
	    if (!cmplx && l->d.real)
	      {
		for (i = 0; i < l->nr; i++)
		  {
		    if (l->d.real[i] < 0.0)
		      {
			cmplx = 1;
			break;
		      }
		  }
	      }
	  }

	if (cmplx)
	  {
	    COMPLEX b;

	    b.real = r->v.real;
	    b.imag = 0.0;

	    v = (MATRIX *) cast_matrix_real_complex (EAT (l));
	    for (i = 0; i < v->nn; i++)
	      {
		if (v->a.complex[i].real < 0.0)
		  {
		    v->a.complex[i] = power_complex (v->a.complex[i], b);
		  }
		else
		  {
		    v->a.complex[i].real =
		      pow (v->a.complex[i].real, r->v.real);
		    v->a.complex[i].imag = 0.0;
		  }
	      }
	    if (v->d.complex)
	      {
		for (i = 0; i < v->nr; i++)
		  {
		    if (v->d.complex[i].real < 0.0)
		      {
			v->d.complex[i] =
			  power_complex (v->d.complex[i], b);
		      }
		    else
		      {
			v->d.complex[i].real =
			  pow (v->d.complex[i].real, r->v.real);
			v->d.complex[i].imag = 0.0;
		      }
		  }
	      }
	  }
	else
	  {
	    v = (MATRIX *) dup_matrix (EAT (l));
	    for (i = 0; i < v->nn; i++)
	      {
		v->a.real[i] = pow (v->a.real[i], r->v.real);
	      }
	    if (v->d.real)
	      {
		for (i = 0; i < v->nr; i++)
		  {
		    v->d.real[i] = pow (v->d.real[i], r->v.real);
		  }
	      }
	  }
      }
  }
  ON_EXCEPTION
  {
    delete_scalar (r);
    delete_2_matrices (l, v);
  }
  END_EXCEPTION;

  delete_matrix (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
power_matrix_scalar_complex (MATRIX *l, SCALAR *r)
{
  /* l[;]^r --> [ l[1;1]^r, l[1;2]^r, ... ; l[2;1]^r, ... ] */

  MATRIX * volatile v = NULL;
  int i;

  EASSERT (l, matrix, complex);
  EASSERT (r, scalar, complex);

  WITH_HANDLING
  {
    /* Result must be dense, since 0^z is not allowed. */

    v = (MATRIX *) dup_matrix ((MATRIX *) dense_matrix (EAT (l)));

    /* check for 0^z */

    for (i = 0; i < v->nn; i++)
      {
	if (!(v->a.complex[i].real || v->a.complex[i].imag))
	  {
	    fail (undefined_cmplx_op);
	    raise_exception ();
	  }
      }

    /* If it's hermitian, then it goes to general. */

    if (v->symmetry == hermitian && r->v.complex.imag != 0.0)
      v->symmetry = general;

    /* check for z^0 */

    if (!(r->v.complex.real || r->v.complex.imag))
      {
	/* No zeros in `l', so return l^0. */

	for (i = 0; i < v->nn; i++)
	  {
	    v->a.complex[i].real = 1.0;
	    v->a.complex[i].imag = 0.0;
	  }

	/* Matrix is all ones -- if it's square then it's hermitian. */

	if (v->nr == v->nc)
	  v->symmetry = hermitian;
      }
    else
      {
	for (i = 0; i < v->nn; i++)
	  {
	    v->a.complex[i] =
	      power_complex (v->a.complex[i], r->v.complex);
	  }
	if (v->d.complex)
	  {
	    for (i = 0; i < v->nr; i++)
	      {
		v->d.complex[i] =
		  power_complex (v->d.complex[i], r->v.complex);
	      }
	  }
      }
  }
  ON_EXCEPTION
  {
    delete_scalar (r);
    delete_2_matrices (l, v);
  }
  END_EXCEPTION;

  delete_matrix (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
power_vector_matrix_integer (VECTOR *l, MATRIX *r)
{
  MATRIX *m = NULL;

  WITH_HANDLING
  {
    m = (MATRIX *) vector_to_matrix (EAT (l));
    m = (MATRIX *) power_matrix_integer (EAT (m), EAT (r));
  }
  ON_EXCEPTION
  {
    delete_vector (l);
    delete_2_matrices (m, r);
  }
  END_EXCEPTION;

  return (ENT (m));
}

ENTITY *
power_vector_matrix_real (VECTOR *l, MATRIX *r)
{
  MATRIX *m = NULL;

  WITH_HANDLING
  {
    m = (MATRIX *) vector_to_matrix (EAT (l));
    m = (MATRIX *) power_matrix_real (EAT (m), EAT (r));
  }
  ON_EXCEPTION
  {
    delete_vector (l);
    delete_2_matrices (m, r);
  }
  END_EXCEPTION;

  return (ENT (m));
}

ENTITY *
power_vector_matrix_complex (VECTOR *l, MATRIX *r)
{
  MATRIX *m = NULL;

  WITH_HANDLING
  {
    m = (MATRIX *) vector_to_matrix (EAT (l));
    m = (MATRIX *) power_matrix_complex (EAT (m), EAT (r));
  }
  ON_EXCEPTION
  {
    delete_vector (l);
    delete_2_matrices (m, r);
  }
  END_EXCEPTION;

  return (ENT (m));
}

ENTITY *
power_matrix_vector_integer (MATRIX *l, VECTOR *r)
{
  MATRIX *m = NULL;

  WITH_HANDLING
  {
    m = (MATRIX *) vector_to_matrix (EAT (r));
    m = (MATRIX *) power_matrix_integer (EAT (l), EAT (m));
  }
  ON_EXCEPTION
  {
    delete_vector (r);
    delete_2_matrices (m, l);
  }
  END_EXCEPTION;

  return (ENT (m));
}

ENTITY *
power_matrix_vector_real (MATRIX *l, VECTOR *r)
{
  MATRIX *m = NULL;

  WITH_HANDLING
  {
    m = (MATRIX *) vector_to_matrix (EAT (r));
    m = (MATRIX *) power_matrix_real (EAT (l), EAT (m));
  }
  ON_EXCEPTION
  {
    delete_vector (r);
    delete_2_matrices (m, l);
  }
  END_EXCEPTION;

  return (ENT (m));
}

ENTITY *
power_matrix_vector_complex (MATRIX *l, VECTOR *r)
{
  MATRIX *m = NULL;

  WITH_HANDLING
  {
    m = (MATRIX *) vector_to_matrix (EAT (r));
    m = (MATRIX *) power_matrix_complex (EAT (l), EAT (m));
  }
  ON_EXCEPTION
  {
    delete_vector (r);
    delete_2_matrices (m, l);
  }
  END_EXCEPTION;

  return (ENT (m));
}

ENTITY *
power_matrix_integer (MATRIX *l, MATRIX *r)
{
  ENTITY * volatile rid = NULL, * volatile cid = NULL;
  MATRIX * volatile v = NULL;
  int neg = 0;
  int i, j, k, n;
  SYMMETRY sym;

  EASSERT (l, matrix, integer);
  EASSERT (r, matrix, integer);

  WITH_HANDLING
  {
    if (l->nr != r->nr || l->nc != r->nc)
      {
	fail (bad_matrix_dims,
	      l->nr, PLURAL (l->nr), l->nc, PLURAL (l->nc),
	      r->nr, PLURAL (r->nr), r->nc, PLURAL (r->nc));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->rid, r->rid) ||
	!MATCH_VECTORS (l->cid, r->cid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    /* Save copy of labels, to put on later. */

    if (l->rid != NULL)
      {
	rid = copy_entity (l->rid);
      }
    else if (r->rid != NULL)
      {
	rid = copy_entity (r->rid);
      }
    if (l->cid != NULL)
      {
	cid = copy_entity (l->cid);
      }
    else if (r->cid != NULL)
      {
	cid = copy_entity (r->cid);
      }

    /* Check for negative elements in `r'. */

    for (i = 0; i < r->nn; i++)
      {
	if (r->a.integer[i] < 0)
	  {
	    neg = 1;
	    break;
	  }
      }
    if (!neg && r->d.integer)
      {
	for (i = 0; i < r->nr; i++)
	  {
	    if (r->d.integer[i] < 0)
	      {
		neg = 1;
		break;
	      }
	  }
      }

    r = (MATRIX *) dense_matrix (EAT (r));
    if (r->symmetry != symmetric)
      l = (MATRIX *) full_matrix (EAT (l));
    assert (r->symmetry == general || r->symmetry == symmetric);

    sym = (l->symmetry == symmetric && r->symmetry == symmetric) ?
      symmetric : general;

    switch (l->density)
      {
      case dense:

	if (neg)
	  {
	    v = (MATRIX *) form_matrix (r->nr, r->nc, real, dense);
	    for (i = 0; i < v->nn; i++)
	      {
		if (!(l->a.integer[i] || r->a.integer[i]))
		  {
		    fail (undefined_op);
		    raise_exception ();
		  }
		v->a.real[i] = pow ((REAL) l->a.integer[i],
				    (REAL) r->a.integer[i]);
	      }

	  }
	else
	  {
	    v = (MATRIX *) dup_matrix (EAT (r));
	    for (i = 0; i < v->nn; i++)
	      {
		if (!(l->a.integer[i] || v->a.integer[i]))
		  {
		    fail (undefined_op);
		    raise_exception ();
		  }
		v->a.integer[i] =
		  (int) round (pow ((REAL) l->a.integer[i],
				    (REAL) v->a.integer[i]));
	      }
	  }
	break;

      case sparse:

	/* Look for 0^0 and 1/0. */

	assert (l->order == ordered);
	for (i = 0; i < l->nr; i++)
	  {
	    k = 1;
	    for (j = l->ia[i] - 1; j < l->ia[i + 1] - 1; j++)
	      {
		for (n = k; n < l->ja[j]; n++)
		  {
		    if (r->a.integer[r->nr * (n - 1) + i] <= 0)
		      {
			fail (r->a.integer[r->nr * (n - 1) + i] ?
			      divide_by_zero : undefined_op);
			raise_exception ();
		      }
		  }
		k = l->ja[j] + 1;
	      }
	    for (n = k; n <= l->nc; n++)
	      {
		if (r->a.integer[r->nr * (n - 1) + i] <= 0)
		  {
		    fail (r->a.integer[r->nr * (n - 1) + i] ?
			  divide_by_zero : undefined_op);
		    raise_exception ();
		  }
	      }
	  }

	if (neg)
	  {
	    v = (MATRIX *) cast_matrix_integer_real (EAT (l));
	    for (i = 0; i < v->nr; i++)
	      {
		POLL_SIGINT ();
		for (j = v->ia[i] - 1; j < v->ia[i + 1] - 1; j++)
		  {
		    if (!(v->a.real[i] ||
			  r->a.integer[r->nr * (v->ja[j] - 1) + i]))
		      {
			fail (undefined_op);
			raise_exception ();
		      }
		    v->a.real[j] =
		      pow (v->a.real[j],
			   (REAL) r->a.integer[r->nr * (v->ja[j] - 1) + i]);
		  }
	      }
	  }
	else
	  {
	    v = (MATRIX *) dup_matrix (EAT (l));
	    for (i = 0; i < v->nr; i++)
	      {
		POLL_SIGINT ();
		for (j = v->ia[i] - 1; j < v->ia[i + 1] - 1; j++)
		  {
		    if (!(v->a.integer[i] ||
			  r->a.integer[r->nr * (v->ja[j] - 1) + i]))
		      {
			fail (undefined_op);
			raise_exception ();
		      }
		    v->a.integer[j] =
		      (int) round (pow ((REAL) v->a.integer[j],
			  (REAL) r->a.integer[r->nr * (v->ja[j] - 1) + i]));
		  }
	      }
	  }
	break;

      case sparse_upper:

	/* Look for 0^0 and 1/0. */

	assert (l->order == ordered);
	for (i = 0; i < l->nr; i++)
	  {
	    k = 1;
	    for (j = l->ia[i] - 1; j < l->ia[i + 1] - 1; j++)
	      {
		for (n = k; n < l->ja[j]; n++)
		  {
		    if (r->a.integer[r->nr * (n - 1) + i] <= 0)
		      {
			fail (r->a.integer[r->nr * (n - 1) + i] ?
			      divide_by_zero : undefined_op);
			raise_exception ();
		      }
		  }
		k = l->ja[j] + 1;
	      }
	    for (n = k; n <= l->nc; n++)
	      {
		if (r->a.integer[r->nr * (n - 1) + i] <= 0)
		  {
		    fail (r->a.integer[r->nr * (n - 1) + i] ?
			  divide_by_zero : undefined_op);
		    raise_exception ();
		  }
	      }
	  }

	if (neg)
	  {
	    v = (MATRIX *) cast_matrix_integer_real (EAT (l));
	    for (i = 0; i < v->nr; i++)
	      {
		for (j = v->ia[i] - 1; j < v->ia[i + 1] - 1; j++)
		  {
		    if (!(v->a.real[j] ||
			  r->a.integer[r->nr * (v->ja[j] - 1) + i]))
		      {
			fail (undefined_op);
			raise_exception ();
		      }
		    v->a.real[j] =
		      pow (v->a.real[j],
			   (REAL) r->a.integer[r->nr * (v->ja[j] - 1) + i]);
		  }
	      }
	    if (v->d.real)
	      {
		for (i = 0; i < v->nr; i++)
		  {
		    if (!(v->d.real[i] ||
			  r->a.integer[(r->nr + 1) * i]))
		      {
			fail (undefined_op);
			raise_exception ();
		      }
		    v->d.real[i] =
		      pow (v->d.real[i],
			   (REAL) r->a.integer[(r->nr + 1) * i]);
		  }
	      }
	  }
	else
	  {
	    v = (MATRIX *) dup_matrix (EAT (l));
	    for (i = 0; i < v->nr; i++)
	      {
		for (j = v->ia[i] - 1; j < v->ia[i + 1] - 1; j++)
		  {
		    if (!(v->a.integer[j] ||
			  r->a.integer[r->nr * (v->ja[j] - 1) + i]))
		      {
			fail (undefined_op);
			raise_exception ();
		      }
		    v->a.integer[j] = (int) round (pow (
						     (REAL) v->a.integer[j],
			  (REAL) r->a.integer[r->nr * (v->ja[j] - 1) + i]));
		  }
	      }
	    if (v->d.integer)
	      {
		for (i = 0; i < v->nr; i++)
		  {
		    if (!(v->d.integer[i] ||
			  r->a.integer[(r->nr + 1) * i]))
		      {
			fail (undefined_op);
			raise_exception ();
		      }
		    v->d.integer[i] =
		      (int) round (pow ((REAL) v->d.integer[i],
				     (REAL) r->a.integer[(r->nr + 1) * i]));
		  }
	      }
	  }
	break;

      default:
	BAD_DENSITY (l->density);
	raise_exception ();
      }

    v->symmetry = sym;
  }
  ON_EXCEPTION
  {
    delete_2_entities (rid, cid);
    delete_3_matrices (l, r, v);
  }
  END_EXCEPTION;

  delete_2_entities (v->rid, v->cid);
  v->rid = rid;
  v->cid = cid;

  delete_2_matrices (l, r);
  return (ENT (v));
}

ENTITY *
power_matrix_real (MATRIX *l, MATRIX *r)
{
  ENTITY * volatile rid = NULL, * volatile cid = NULL;
  MATRIX * volatile v = NULL;
  int cmplx = 0;
  int i, j, k, n;
  SYMMETRY sym;

  EASSERT (l, matrix, real);
  EASSERT (r, matrix, real);

  WITH_HANDLING
  {
    if (l->nr != r->nr || l->nc != r->nc)
      {
	fail (bad_matrix_dims,
	      l->nr, PLURAL (l->nr), l->nc, PLURAL (l->nc),
	      r->nr, PLURAL (r->nr), r->nc, PLURAL (r->nc));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->rid, r->rid) ||
	!MATCH_VECTORS (l->cid, r->cid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    if (l->rid != NULL)
      {
	rid = copy_entity (l->rid);
      }
    else if (r->rid != NULL)
      {
	rid = copy_entity (r->rid);
      }
    if (l->cid != NULL)
      {
	cid = copy_entity (l->cid);
      }
    else if (r->cid != NULL)
      {
	cid = copy_entity (r->cid);
      }

    r = (MATRIX *) dense_matrix (EAT (r));
    l = (MATRIX *) full_matrix (EAT (l));

    sym = (l->symmetry == symmetric && r->symmetry == symmetric) ?
      symmetric : general;

    switch (l->density)
      {
      case dense:

	/* See if result is complex. */

	for (i = 0; i < l->nn; i++)
	  {
	    if (l->a.real[i] < 0.0 &&
		(int) r->a.real[i] != r->a.real[i])
	      {
		cmplx = 1;
		break;
	      }
	  }

	if (cmplx)
	  {
	    COMPLEX a, b;
	    a.imag = b.imag = 0.0;

	    v = (MATRIX *) form_matrix (r->nr, r->nc, complex, dense);

	    for (i = 0; i < v->nn; i++)
	      {
		if (l->a.real[i] == 0.0 && r->a.real[i] <= 0.0)
		  {
		    fail (r->a.real[i] ? divide_by_zero : undefined_op);
		    raise_exception ();
		  }
		if (l->a.real[i] < 0.0)
		  {
		    a.real = l->a.real[i];
		    b.real = r->a.real[i];

		    v->a.complex[i] = power_complex (a, b);
		  }
		else
		  {
		    v->a.complex[i].real =
		      pow (l->a.real[i], r->a.real[i]);
		    v->a.complex[i].imag = 0.0;
		  }
	      }
	  }
	else
	  {
	    v = (MATRIX *) dup_matrix (EAT (r));
	    for (i = 0; i < v->nn; i++)
	      {
		if (!(l->a.real[i] || v->a.real[i]))
		  {
		    fail (undefined_op);
		    raise_exception ();
		  }
		v->a.real[i] = pow (l->a.real[i], v->a.real[i]);
	      }
	  }
	break;

      case sparse:

	/* Look for 0^0 and 1/0. */

	assert (l->order == ordered);

	for (i = 0; i < l->nr; i++)
	  {
	    k = 1;
	    POLL_SIGINT ();
	    for (j = l->ia[i] - 1; j < l->ia[i + 1] - 1; j++)
	      {
		for (n = k; n < l->ja[j]; n++)
		  {
		    if (r->a.real[r->nr * (n - 1) + i] <= 0.0)
		      {
			fail (r->a.real[r->nr * (n - 1) + i] ?
			      divide_by_zero : undefined_op);
			raise_exception ();
		      }
		  }
		k = l->ja[j] + 1;
	      }
	    for (n = k; n <= l->nc; n++)
	      {
		if (r->a.real[r->nr * (n - 1) + i] <= 0.0)
		  {
		    fail (r->a.real[r->nr * (n - 1) + i] ?
			  divide_by_zero : undefined_op);
		    raise_exception ();
		  }
	      }
	  }

	/* See if result is complex. */

	for (i = 0; i < l->nr; i++)
	  {
	    POLL_SIGINT ();
	    for (j = l->ia[i] - 1; j < l->ia[i + 1] - 1; j++)
	      {
		if (l->a.real[j] < 0.0 &&
		    (int) r->a.real[r->nr * (l->ja[j] - 1) + i] !=
		    r->a.real[r->nr * (l->ja[j] - 1) + i])
		  {
		    cmplx = 1;
		    break;
		  }
	      }
	  }

	if (cmplx)
	  {
	    COMPLEX b;
	    b.imag = 0.0;

	    v = (MATRIX *) cast_matrix_real_complex (EAT (l));

	    for (i = 0; i < v->nr; i++)
	      {
		POLL_SIGINT ();
		for (j = v->ia[i] - 1; j < v->ia[i + 1] - 1; j++)
		  {
		    if (!(v->a.complex[j].real ||
			  r->a.real[r->nr * (v->ja[j] - 1) + i]))
		      {
			fail (undefined_op);
			raise_exception ();
		      }
		    if (v->a.complex[j].real < 0.0)
		      {
			b.real = r->a.real[r->nr * (v->ja[j] - 1) + i];
			v->a.complex[j] =
			  power_complex (v->a.complex[j], b);
		      }
		    else
		      {
			v->a.complex[j].real =
			  pow (v->a.complex[j].real,
			       r->a.real[r->nr * (v->ja[j] - 1) + i]);
			v->a.complex[j].imag = 0.0;
		      }
		  }
	      }
	  }
	else
	  {
	    v = (MATRIX *) dup_matrix (EAT (l));

	    for (i = 0; i < v->nr; i++)
	      {
		for (j = v->ia[i] - 1; j < v->ia[i + 1] - 1; j++)
		  {
		    if (!(v->a.real[j] ||
			  r->a.real[r->nr * (v->ja[j] - 1) + i]))
		      {
			fail (undefined_op);
			raise_exception ();
		      }
		    v->a.real[j] = pow (v->a.real[j],
				     r->a.real[r->nr * (v->ja[j] - 1) + i]);
		  }
	      }
	  }

	break;

      default:
	BAD_DENSITY (l->density);
	raise_exception ();
      }

    v->symmetry = sym;

  }
  ON_EXCEPTION
  {
    delete_2_entities (rid, cid);
    delete_3_matrices (l, r, v);
  }
  END_EXCEPTION;

  delete_2_entities (v->rid, v->cid);
  v->rid = rid;
  v->cid = cid;

  delete_2_matrices (l, r);
  return (ENT (v));
}

ENTITY *
power_matrix_complex (MATRIX *l, MATRIX *r)
{
  ENTITY * volatile rid = NULL, * volatile cid = NULL;
  MATRIX * volatile v = NULL;
  int i;
  COMPLEX a, b;

  EASSERT (l, matrix, complex);
  EASSERT (r, matrix, complex);

  WITH_HANDLING
  {
    if (l->nr != r->nr || l->nc != r->nc)
      {
	fail (bad_matrix_dims,
	      l->nr, PLURAL (l->nr), l->nc, PLURAL (l->nc),
	      r->nr, PLURAL (r->nr), r->nc, PLURAL (r->nc));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->rid, r->rid) ||
	!MATCH_VECTORS (l->cid, r->cid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    if (l->rid != NULL)
      {
	rid = copy_entity (l->rid);
      }
    else if (r->rid != NULL)
      {
	rid = copy_entity (r->rid);
      }
    if (l->cid != NULL)
      {
	cid = copy_entity (l->cid);
      }
    else if (r->cid != NULL)
      {
	cid = copy_entity (r->cid);
      }

    r = (MATRIX *) dense_matrix (EAT (r));
    l = (MATRIX *) dense_matrix (EAT (l));

    v = (MATRIX *) dup_matrix (EAT (r));
    if (l->symmetry != v->symmetry)
      v->symmetry = general;

    for (i = 0; i < v->nn; i++)
      {
	a = l->a.complex[i];
	b = v->a.complex[i];
	if (!(a.real || a.imag))
	  {
	    fail (undefined_cmplx_op);
	    raise_exception ();
	  }
	v->a.complex[i] = power_complex (a, b);
      }
  }
  ON_EXCEPTION
  {
    delete_2_entities (rid, cid);
    delete_3_matrices (l, r, v);
  }
  END_EXCEPTION;

  delete_2_entities (v->rid, v->cid);
  v->rid = rid;
  v->cid = cid;

  delete_2_matrices (l, r);
  return (ENT (v));
}
