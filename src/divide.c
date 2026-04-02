/*
   divide.c -- Division.

   Copyright (C) 1994-2001  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: divide.c,v 1.4 2002/07/24 04:33:29 ksh Exp $";

#include "divide.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "solve.h"
#include "transpose.h"
#include "blas.h"
#include "cast.h"
#include "dense.h"
#include "full.h"

static char bad_labels[] = "Labels don't match.";

/* Have to define "div" on some machines. */

#if !HAVE_DIV
div_t
div (int num, int denom)
{
  div_t d;
  d.quot = num / denom;
  d.rem = num % denom;
  return d;
}
#endif /* !HAVE_DIV */

COMPLEX
divide_complex (COMPLEX a, COMPLEX b)
{
  /*
   * Complex division.
   *
   * This function was adapted from the f2c "z_div" function.
   * Copyright 1990 by AT&T Bell Laboratories and Bellcore.
   */

  COMPLEX c;

  REAL ratio, den;
  REAL abr, abi;

  if ((abr = b.real) < 0.)
    abr = -abr;
  if ((abi = b.imag) < 0.)
    abi = -abi;

  if (abr <= abi)
    {
      if (abi == 0)
	{
	  fail ("Complex division by zero.");
	  raise_exception ();
	}
      ratio = b.real / b.imag;
      den = b.imag * (1 + ratio * ratio);
      c.real = (a.real * ratio + a.imag) / den;
      c.imag = (a.imag * ratio - a.real) / den;

    }
  else
    {

      ratio = b.imag / b.real;
      den = b.real * (1 + ratio * ratio);
      c.real = (a.real + a.imag * ratio) / den;
      c.imag = (a.imag - a.real * ratio) / den;
    }

  return c;
}

ENTITY *
divide_scalar_matrix_integer (SCALAR *l, MATRIX *r)
{
  EASSERT (l, scalar, integer);
  EASSERT (r, matrix, integer);

  return divide_scalar_matrix_real (
				     (SCALAR *) cast_scalar_integer_real (l),
				   (MATRIX *) cast_matrix_integer_real (r));
}

ENTITY *
divide_matrix_scalar_integer (MATRIX *l, SCALAR *r)
{
  EASSERT (l, matrix, integer);
  EASSERT (r, scalar, integer);

  return divide_matrix_scalar_real (
				     (MATRIX *) cast_matrix_integer_real (l),
				   (SCALAR *) cast_scalar_integer_real (r));
}

ENTITY *
divide_scalar_matrix_real (SCALAR *l, MATRIX *r)
{
  /* Returns `x=r', where `x[i;j]=l/r[i;j]' for all `i' and `j'. */

  int i;
  REAL numerator;

  EASSERT (l, scalar, real);
  EASSERT (r, matrix, real);

  WITH_HANDLING
  {

    /*
     * There shouldn't be any zeros, so if it isn't in dense form
     * then it ought to be.
     */

    r = (MATRIX *) dup_matrix ((MATRIX *) dense_matrix (EAT (r)));

    numerator = l->v.real;
    for (i = 0; i < r->nn; i++)
      r->a.real[i] = numerator / r->a.real[i];

  }
  ON_EXCEPTION
  {
    delete_scalar (l);
    delete_matrix (r);
  }
  END_EXCEPTION;

  delete_scalar (l);
  return (ENT (r));
}

ENTITY *
divide_matrix_scalar_real (MATRIX *l, SCALAR *r)
{
  REAL f;
  int stride = 1;

  EASSERT (l, matrix, real);
  EASSERT (r, scalar, real);

  WITH_HANDLING
  {
    f = 1.0 / r->v.real;
    l = (MATRIX *) dup_matrix ((MATRIX *) EAT (l));
    SIGINT_RAISE_ON ();
    if (l->a.real)
      DSCAL (&l->nn, &f, l->a.real, &stride);
    if (l->d.real)
      DSCAL (&l->nr, &f, l->d.real, &stride);
    SIGINT_RAISE_OFF ();
  }
  ON_EXCEPTION
  {
    delete_matrix (l);
    delete_scalar (r);
  }
  END_EXCEPTION;

  delete_scalar (r);
  return (ENT (l));
}

ENTITY *
divide_scalar_matrix_complex (SCALAR *l, MATRIX *r)
{
  /* Returns `x=r', where `x[i;j]=l/r[i;j]' for all `i' and `j'. */

  int i;

  EASSERT (l, scalar, complex);
  EASSERT (r, matrix, complex);

  WITH_HANDLING
  {

    /*
     * There shouldn't be any zeros, so if it isn't in dense form
     * then it ought to be.
     */

    r = (MATRIX *) dup_matrix ((MATRIX *) dense_matrix (EAT (r)));

    /* If it's hermitian, then it goes to general. */

    if (r->symmetry == hermitian && l->v.complex.imag != 0.0)
      r->symmetry = general;

    for (i = 0; i < r->nn; i++)
      r->a.complex[i] = divide_complex (l->v.complex, r->a.complex[i]);

  }
  ON_EXCEPTION
  {
    delete_scalar (l);
    delete_matrix (r);
  }
  END_EXCEPTION;

  delete_scalar (l);
  return (ENT (r));
}

ENTITY *
divide_matrix_scalar_complex (MATRIX *l, SCALAR *r)
{
  int i;

  EASSERT (l, matrix, complex);
  EASSERT (r, scalar, complex);

  WITH_HANDLING
  {
    l = (MATRIX *) dup_matrix (EAT (l));

    /*
     * If the imaginary part is not zero, then hermitian
     * symmetry is lost.  If it's diagonal it can drop back to
     * symmetric, otherwise it goes to general.
     */

    if ((l->symmetry == hermitian) &&
	r->v.complex.imag != 0)
      {
	if (l->nn == 0)
	  {
	    l->symmetry = symmetric;
	  }
	else
	  {
	    l = (MATRIX *) full_matrix ((MATRIX *) EAT (l));
	    l->symmetry = general;
	  }
      }

    if (l->a.complex)
      for (i = 0; i < l->nn; i++)
	l->a.complex[i] = divide_complex (l->a.complex[i],
					  r->v.complex);
    if (l->d.complex)
      for (i = 0; i < l->nr; i++)
	l->d.complex[i] = divide_complex (l->d.complex[i],
					  r->v.complex);
  }
  ON_EXCEPTION
  {
    delete_matrix (l);
    delete_scalar (r);
  }
  END_EXCEPTION;

  delete_scalar (r);
  return (ENT (l));
}

ENTITY *
divide_scalar_vector_integer (SCALAR *l, VECTOR *r)
{
  EASSERT (l, scalar, integer);
  EASSERT (r, vector, integer);

  return divide_scalar_vector_real (
				     (SCALAR *) cast_scalar_integer_real (l),
				   (VECTOR *) cast_vector_integer_real (r));
}

ENTITY *
divide_vector_scalar_integer (VECTOR *l, SCALAR *r)
{
  EASSERT (l, vector, integer);
  EASSERT (r, scalar, integer);

  return divide_vector_scalar_real (
				     (VECTOR *) cast_vector_integer_real (l),
				   (SCALAR *) cast_scalar_integer_real (r));
}

ENTITY *
divide_scalar_vector_real (SCALAR *l, VECTOR *r)
{
  /* Returns `x=r', where `x[i]=l/r[i]' for all `i'. */

  EASSERT (l, scalar, real);
  EASSERT (r, vector, real);

  WITH_HANDLING
  {

    int i;
    REAL numerator;

    r = (VECTOR *) dup_vector ((VECTOR *) dense_vector (EAT (r)));
    numerator = l->v.real;
    for (i = 0; i < r->nn; i++)
      r->a.real[i] = numerator / r->a.real[i];

  }
  ON_EXCEPTION
  {
    delete_scalar (l);
    delete_vector (r);
  }
  END_EXCEPTION;

  delete_scalar (l);
  return (ENT (r));
}

ENTITY *
divide_vector_scalar_real (VECTOR *l, SCALAR *r)
{
  REAL f;
  int i;

  EASSERT (l, vector, real);
  EASSERT (r, scalar, real);

  WITH_HANDLING
  {
    f = 1.0 / r->v.real;
    l = (VECTOR *) dup_vector ((VECTOR *) EAT (l));
    DSCAL (&l->nn, &f, l->a.real, (i = 1, &i));
  }
  ON_EXCEPTION
  {
    delete_vector (l);
    delete_scalar (r);
  }
  END_EXCEPTION;

  delete_scalar (r);
  return (ENT (l));
}

ENTITY *
divide_scalar_vector_complex (SCALAR *l, VECTOR *r)
{
  /* Returns `x=r', where `x[i;j]=l/r[i;j]' for all `i' and `j'. */

  EASSERT (l, scalar, complex);
  EASSERT (r, vector, complex);

  WITH_HANDLING
  {
    int i;

    r = (VECTOR *) dup_vector ((VECTOR *) dense_vector (EAT (r)));

    for (i = 0; i < r->nn; i++)
      r->a.complex[i] = divide_complex (l->v.complex, r->a.complex[i]);

  }
  ON_EXCEPTION
  {
    delete_vector (r);
    delete_scalar (l);
  }
  END_EXCEPTION;

  delete_scalar (l);
  return (ENT (r));
}

ENTITY *
divide_vector_scalar_complex (VECTOR *l, SCALAR *r)
{
  int i;

  EASSERT (l, vector, complex);
  EASSERT (r, scalar, complex);

  WITH_HANDLING
  {
    l = (VECTOR *) dup_vector (EAT (l));
    for (i = 0; i < l->nn; i++)
      l->a.complex[i] = divide_complex (l->a.complex[i], r->v.complex);
  }
  ON_EXCEPTION
  {
    delete_vector (l);
    delete_scalar (r);
  }
  END_EXCEPTION;

  delete_scalar (r);
  return (ENT (l));
}

ENTITY *
divide_matrix_integer (MATRIX *l, MATRIX *r)
{
  EASSERT (l, matrix, integer);
  EASSERT (r, matrix, integer);

  return divide_matrix_real ((MATRIX *) cast_matrix_integer_real (l),
			     (MATRIX *) cast_matrix_integer_real (r));
}

ENTITY *
divide_matrix_real (MATRIX *l, MATRIX *r)
{
  /* Element-by-element division. */

  int i;

  EASSERT (l, matrix, real);
  EASSERT (r, matrix, real);

  WITH_HANDLING
  {

    if (l->nr != r->nr)
      {
	fail ("Inconsistent dimensions for matrix division.  Left matrix has %d row%s, but right matrix has %d row%s.\n",
	      l->nr, PLURAL (l->nr), r->nr, PLURAL (r->nr));
	raise_exception ();
      }

    if (l->nc != r->nc)
      {
	fail ("Inconsistent dimensions for matrix division.  Left matrix has %d column%s, but right matrix has %d column%s.\n",
	      l->nc, PLURAL (l->nc), r->nc, PLURAL (r->nc));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->rid, r->rid) ||
	!MATCH_VECTORS (l->cid, r->cid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    l = (MATRIX *) dup_matrix ((MATRIX *) dense_matrix (EAT (l)));
    r = (MATRIX *) dense_matrix (EAT (r));

    for (i = 0; i < l->nn; i++)
      l->a.real[i] /= r->a.real[i];

  }
  ON_EXCEPTION
  {
    delete_2_matrices (l, r);
  }
  END_EXCEPTION;

  if (!l->rid && r->rid) l->rid = copy_entity (r->rid);
  if (!l->cid && r->cid) l->cid = copy_entity (r->cid);

  delete_matrix (r);
  return (ENT (l));
}

ENTITY *
divide_matrix_complex (MATRIX *l, MATRIX *r)
{
  /* Element-by-element division. */

  int i;

  EASSERT (l, matrix, complex);
  EASSERT (r, matrix, complex);

  WITH_HANDLING
  {
    if (l->nr != r->nr)
      {
	fail ("Inconsistent dimensions for matrix division.  Left matrix has %d row%s, but right matrix has %d row%s.\n",
	      l->nr, PLURAL (l->nr), r->nr, PLURAL (r->nr));
	raise_exception ();
      }

    if (l->nc != r->nc)
      {
	fail ("Inconsistent dimensions for matrix division.  Left matrix has %d column%s, but right matrix has %d column%s.\n",
	      l->nc, PLURAL (l->nc), r->nc, PLURAL (r->nc));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->rid, r->rid) ||
	!MATCH_VECTORS (l->cid, r->cid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    l = (MATRIX *) dup_matrix ((MATRIX *) dense_matrix (EAT (l)));
    r = (MATRIX *) dense_matrix (EAT (r));

    for (i = 0; i < l->nn; i++)
      l->a.complex[i] = divide_complex (l->a.complex[i],
					r->a.complex[i]);
  }
  ON_EXCEPTION
  {
    delete_2_matrices (l, r);
  }
  END_EXCEPTION;

  if (!l->rid && r->rid) l->rid = copy_entity (r->rid);
  if (!l->cid && r->cid) l->cid = copy_entity (r->cid);

  delete_matrix (r);
  return (ENT (l));
}

ENTITY *
divide_vector_integer (VECTOR *l, VECTOR *r)
{
  EASSERT (l, vector, integer);
  EASSERT (r, vector, integer);

  return (divide_vector_real ((VECTOR *) cast_vector_integer_real (l),
			      (VECTOR *) cast_vector_integer_real (r)));
}

ENTITY *
divide_vector_real (VECTOR *l, VECTOR *r)
{
  int i;

  EASSERT (l, vector, real);
  EASSERT (r, vector, real);

  WITH_HANDLING
  {

    if (l->ne != r->ne)
      {
	fail ("Inconsistent dimensions for vector division.  Left vector has %d element%s, but right vector has %d element%s.\n",
	      l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->eid, r->eid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    l = (VECTOR *) dup_vector ((VECTOR *) dense_vector (EAT (l)));
    r = (VECTOR *) dense_vector (EAT (r));

    for (i = 0; i < l->nn; i++)
      l->a.real[i] /= r->a.real[i];

  }
  ON_EXCEPTION
  {
    delete_2_vectors (l, r);
  }
  END_EXCEPTION;

  if (!l->eid && r->eid) l->eid = copy_entity (r->eid);

  delete_vector (r);
  return (ENT (l));
}

ENTITY *
divide_vector_complex (VECTOR *l, VECTOR *r)
{
  int i;

  EASSERT (l, vector, complex);
  EASSERT (r, vector, complex);

  WITH_HANDLING
  {
    if (l->ne != r->ne)
      {
	fail ("Inconsistent dimensions for vector division.  Left vector has %d element%s, but right vector has %d element%s.\n",
	      l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->eid, r->eid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    l = (VECTOR *) dup_vector ((VECTOR *) dense_vector (EAT (l)));
    r = (VECTOR *) dense_vector (EAT (r));

    for (i = 0; i < l->nn; i++)
      l->a.complex[i] = divide_complex (l->a.complex[i],
					r->a.complex[i]);
  }
  ON_EXCEPTION
  {
    delete_2_vectors (l, r);
  }
  END_EXCEPTION;

  if (!l->eid && r->eid) l->eid = copy_entity (r->eid);

  delete_vector (r);
  return (ENT (l));
}

ENTITY *
divide_vector_matrix_integer (VECTOR *l, MATRIX *r)
{
  MATRIX *m;

  EASSERT (l, vector, integer);
  EASSERT (r, matrix, integer);

  m = (MATRIX *) vector_to_matrix (l);
  return (divide_matrix_integer (m, r));
}

ENTITY *
divide_vector_matrix_real (VECTOR *l, MATRIX *r)
{
  MATRIX *m;

  EASSERT (l, vector, real);
  EASSERT (r, matrix, real);

  m = (MATRIX *) vector_to_matrix (l);
  return (divide_matrix_real (m, r));
}

ENTITY *
divide_vector_matrix_complex (VECTOR *l, MATRIX *r)
{
  MATRIX *m;

  EASSERT (l, vector, complex);
  EASSERT (r, matrix, complex);

  m = (MATRIX *) vector_to_matrix (l);
  return (divide_matrix_complex (m, r));
}

ENTITY *
divide_matrix_vector_integer (MATRIX *l, VECTOR *r)
{
  EASSERT (l, matrix, integer);
  EASSERT (r, vector, integer);

  return (divide_matrix_integer (l, (MATRIX *) vector_to_matrix (r)));
}

ENTITY *
divide_matrix_vector_real (MATRIX *l, VECTOR *r)
{
  EASSERT (l, matrix, real);
  EASSERT (r, vector, real);

  return (divide_matrix_real (l, (MATRIX *) vector_to_matrix (r)));
}

ENTITY *
divide_matrix_vector_complex (MATRIX *l, VECTOR *r)
{
  EASSERT (l, matrix, complex);
  EASSERT (r, vector, complex);

  return (divide_matrix_complex (l, (MATRIX *) vector_to_matrix (r)));
}
