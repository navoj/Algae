/*
   mod.c -- Modulus.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: mod.c,v 1.2 2003/08/01 04:57:47 ksh Exp $";

#include "mod.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "cast.h"
#include "transpose.h"
#include "dense.h"
#include "full.h"

#define OWN( p )	( ENT(p)->ref_count == 1 )

static char bad_labels[] = "Labels don't match.";

ENTITY *
mod_scalar_vector_integer (SCALAR *l, VECTOR *r)
{
  VECTOR * volatile x = NULL;
  int i;

  EASSERT (l, scalar, integer);
  EASSERT (r, vector, integer);

  if (l->v.integer == 0)
    {
      x = (VECTOR *) make_vector (r->ne, integer, sparse);
    }
  else
    {
      WITH_HANDLING
      {
	r = (VECTOR *) dense_vector (EAT (r));
	x = (VECTOR *) form_vector (r->ne, real, dense);
	for (i = 0; i < r->ne; i++)
	  x->a.real[i] = fmod ((REAL) l->v.integer, (REAL) r->a.integer[i]);
      }
      ON_EXCEPTION
      {
	delete_2_vectors (r, x);
	delete_scalar (l);
      }
      END_EXCEPTION;
    }

  delete_scalar (l);
  delete_vector (r);

  return (ENT (x));
}

ENTITY *
mod_scalar_vector_real (SCALAR *l, VECTOR *r)
{
  int i;

  EASSERT (l, scalar, real);
  EASSERT (r, vector, real);

  if (l->v.real == 0.0)
    {
      VECTOR *x = (VECTOR *) make_vector (r->ne, real, sparse);
      delete_vector (r);
      r = x;
    }
  else
    {
      WITH_HANDLING
      {
	r = (VECTOR *) dense_vector ((VECTOR *) dup_vector (EAT (r)));
	for (i = 0; i < r->ne; i++)
	  r->a.real[i] = fmod (l->v.real, r->a.real[i]);
      }
      ON_EXCEPTION
      {
	delete_vector (r);
	delete_scalar (l);
      }
      END_EXCEPTION;

    }
  delete_scalar (l);

  return (ENT (r));
}

ENTITY *
mod_scalar_matrix_integer (SCALAR *l, MATRIX *r)
{
  MATRIX * volatile x = NULL;
  int i;

  EASSERT (l, scalar, integer);
  EASSERT (r, matrix, integer);

  if (l->v.integer == 0)
    {
      x = (MATRIX *) make_matrix (r->nr, r->nc, real, sparse);
    }
  else
    {
      WITH_HANDLING
      {
	r = (MATRIX *) dense_matrix (EAT (r));
	x = (MATRIX *) form_matrix (r->nr, r->nc, real, dense);
	for (i = 0; i < r->nn; i++)
	  x->a.real[i] = fmod ((REAL) l->v.integer, (REAL) r->a.integer[i]);
      }
      ON_EXCEPTION
      {
	delete_2_matrices (r, x);
	delete_scalar (l);
      }
      END_EXCEPTION;
    }

  delete_scalar (l);
  delete_matrix (r);

  return (ENT (x));
}

ENTITY *
mod_scalar_matrix_real (SCALAR *l, MATRIX *r)
{
  int i;

  EASSERT (l, scalar, real);
  EASSERT (r, matrix, real);

  if (l->v.real == 0.0)
    {
      MATRIX *x = (MATRIX *) make_matrix (r->nr, r->nc, real, sparse);
      delete_matrix (r);
      r = x;
    }
  else
    {
      WITH_HANDLING
      {
	r = (MATRIX *) dense_matrix ((MATRIX *) dup_matrix (EAT (r)));
	for (i = 0; i < r->nn; i++)
	  r->a.real[i] = fmod (l->v.real, r->a.real[i]);
      }
      ON_EXCEPTION
      {
	delete_matrix (r);
	delete_scalar (l);
      }
      END_EXCEPTION;
    }
  delete_scalar (l);

  return (ENT (r));
}


ENTITY *
mod_matrix_integer (MATRIX *l, MATRIX *r)
{
  MATRIX * volatile x = NULL;
  int i, j;
  SYMMETRY sym;

  EASSERT (l, matrix, integer);
  EASSERT (r, matrix, integer);

  assert (l->order == ordered);
  assert (r->order == ordered);

  r = (MATRIX *) dense_matrix (r);

  WITH_HANDLING
  {
    if (l->nr != r->nr)
      {
	fail ("Inconsistent dimensions for matrix modulus.  Left matrix has %d row%s, but right matrix has %d row%s.\n",
	      l->nr, PLURAL (l->nr), r->nr, PLURAL (r->nr));
	raise_exception ();
      }

    if (l->nc != r->nc)
      {
	fail ("Inconsistent dimensions for matrix modulus.  Left matrix has %d column%s, but right matrix has %d column%s.\n",
	      l->nc, PLURAL (l->nc), r->nc, PLURAL (r->nc));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->rid, r->rid) ||
	!MATCH_VECTORS (l->cid, r->cid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    sym = (l->symmetry == symmetric && r->symmetry == symmetric) ?
      symmetric : general;
    assert (l->symmetry != hermitian && r->symmetry != hermitian);

    switch (l->density)
      {
      case dense:

	x = (MATRIX *) cast_matrix_integer_real (EAT (l));
	for (i = 0; i < x->nn; i++)
	  x->a.real[i] = fmod (x->a.real[i], (REAL) r->a.integer[i]);
	x->symmetry = sym;

	break;

      case sparse:

	x = (MATRIX *) cast_matrix_integer_real (EAT (l));
	if (x->nn > 0)
	  {
	    for (i = 0; i < x->nr; i++)
	      {
		for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
		  {
		    x->a.real[j] =
		      fmod (x->a.real[j],
			    (REAL) r->a.integer[r->nr * (x->ja[j] - 1) + i]);
		  }
	      }
	  }
	x->symmetry = sym;

	break;

      case sparse_upper:

	if (sym == symmetric)
	  {
	    x = (MATRIX *) cast_matrix_integer_real (EAT (l));
	    if (x->nn > 0)
	      {
		for (i = 0; i < x->nr; i++)
		  {
		    for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
		      {
			x->a.real[j] =
			  fmod (x->a.real[j],
			      (REAL) r->a.real[r->nr * (x->ja[j] - 1) + i]);
		      }
		  }
	      }
	    if (x->d.real)
	      {
		for (i = 0; i < x->nr; i++)
		  {
		    x->d.real[i] =
		      fmod (x->d.real[i],
			    (REAL) r->a.integer[i * (r->nr + 1)]);
		  }
	      }
	  }
	else
	  {
	    l = (MATRIX *) full_matrix (EAT (l));
	    x = (MATRIX *) mod_matrix_integer (EAT (l), EAT (r));
	  }

	break;

      default:

	wipeout ("Bad density.");
      }
  }
  ON_EXCEPTION
  {
    delete_3_matrices (l, r, x);
  }
  END_EXCEPTION;

  delete_2_matrices (l, r);
  return (ENT (x));
}

ENTITY *
mod_matrix_real (MATRIX *l, MATRIX *r)
{
  MATRIX * volatile x = NULL;
  int i, j;
  SYMMETRY sym;

  EASSERT (l, matrix, real);
  EASSERT (r, matrix, real);

  assert (l->order == ordered);
  assert (r->order == ordered);

  r = (MATRIX *) dense_matrix (r);

  WITH_HANDLING
  {
    if (l->nr != r->nr)
      {
	fail ("Inconsistent dimensions for matrix modulus.  Left matrix has %d row%s, but right matrix has %d row%s.\n",
	      l->nr, PLURAL (l->nr), r->nr, PLURAL (r->nr));
	raise_exception ();
      }

    if (l->nc != r->nc)
      {
	fail ("Inconsistent dimensions for matrix modulus.  Left matrix has %d column%s, but right matrix has %d column%s.\n",
	      l->nc, PLURAL (l->nc), r->nc, PLURAL (r->nc));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->rid, r->rid) ||
	!MATCH_VECTORS (l->cid, r->cid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    sym = (l->symmetry == symmetric && r->symmetry == symmetric) ?
      symmetric : general;
    assert (l->symmetry != hermitian && r->symmetry != hermitian);

    switch (l->density)
      {
      case dense:

	x = (MATRIX *) dup_matrix (EAT (l));
	for (i = 0; i < x->nn; i++)
	  x->a.real[i] = fmod (x->a.real[i], r->a.real[i]);
	x->symmetry = sym;

	break;

      case sparse:

	x = (MATRIX *) dup_matrix (EAT (l));
	if (x->nn > 0)
	  {
	    for (i = 0; i < x->nr; i++)
	      {
		for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
		  {
		    x->a.real[j] =
		      fmod (x->a.real[j],
			    r->a.real[r->nr * (x->ja[j] - 1) + i]);
		  }
	      }
	  }
	x->symmetry = sym;

	break;

      case sparse_upper:

	if (sym == symmetric)
	  {
	    x = (MATRIX *) dup_matrix (EAT (l));
	    if (x->nn > 0)
	      {
		for (i = 0; i < x->nr; i++)
		  {
		    for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
		      {
			x->a.real[j] =
			  fmod (x->a.real[j],
				r->a.real[r->nr * (x->ja[j] - 1) + i]);
		      }
		  }
	      }
	    if (x->d.real)
	      {
		for (i = 0; i < x->nr; i++)
		  {
		    x->d.real[i] =
		      fmod (x->d.real[i], r->a.real[i * (r->nr + 1)]);
		  }
	      }
	  }
	else
	  {
	    l = (MATRIX *) full_matrix (EAT (l));
	    x = (MATRIX *) mod_matrix_real (EAT (l), EAT (r));
	  }

	break;

      default:

	wipeout ("Bad density.");

      }

  }
  ON_EXCEPTION
  {
    delete_3_matrices (l, r, x);
  }
  END_EXCEPTION;

  delete_2_matrices (l, r);
  return (ENT (x));
}

ENTITY *
mod_vector_integer (VECTOR *l, VECTOR *r)
{
  VECTOR * volatile x = NULL;
  int i;

  EASSERT (l, vector, integer);
  EASSERT (r, vector, integer);

  assert (l->order == ordered);
  assert (r->order == ordered);

  WITH_HANDLING
  {
    if (l->ne != r->ne)
      {
	fail ("Inconsistent dimensions for vector modulus.  Left vector has %d element%s, but right vector has %d element%s.\n",
	      l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->eid, r->eid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    r = (VECTOR *) dense_vector (EAT (r));

    switch (l->density)
      {
      case dense:

	x = (VECTOR *) form_vector (l->ne, real, dense);
	for (i = 0; i < r->nn; i++)
	  x->a.real[i] = fmod ((REAL) l->a.integer[i],
			       (REAL) r->a.integer[i]);

	break;

      case sparse:

	x = (VECTOR *) cast_vector_integer_real (EAT (l));
	for (i = 0; i < x->nn; i++)
	  x->a.real[i] = fmod (x->a.real[i],
			       (REAL) r->a.integer[x->ja[i] - 1]);

	break;

      default:

	wipeout ("Bad density.");

      }
  }
  ON_EXCEPTION
  {
    delete_3_vectors (l, r, x);
  }
  END_EXCEPTION;

  delete_2_vectors (l, r);
  return (ENT (x));
}

ENTITY *
mod_vector_real (VECTOR *l, VECTOR *r)
{
  VECTOR * volatile x = NULL;
  int i;

  EASSERT (l, vector, real);
  EASSERT (r, vector, real);

  assert (l->order == ordered);
  assert (r->order == ordered);

  WITH_HANDLING
  {
    if (l->ne != r->ne)
      {
	fail ("Inconsistent dimensions for vector modulus.  Left vector has %d element%s, but right vector has %d element%s.\n",
	      l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->eid, r->eid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    r = (VECTOR *) dense_vector (EAT (r));

    switch (l->density)
      {
      case dense:

	if (OWN (l))
	  {
	    x = (VECTOR *) dup_vector (EAT (l));
	    for (i = 0; i < r->nn; i++)
	      x->a.real[i] = fmod (x->a.real[i], r->a.real[i]);
	  }
	else
	  {
	    x = (VECTOR *) dup_vector (EAT (r));
	    for (i = 0; i < l->nn; i++)
	      x->a.real[i] = fmod (l->a.real[i], x->a.real[i]);
	  }

	break;

      case sparse:

	x = (VECTOR *) dup_vector (EAT (l));
	for (i = 0; i < x->nn; i++)
	  x->a.real[i] = fmod (x->a.real[i], r->a.real[x->ja[i] - 1]);

	break;

      default:

	wipeout ("Bad density.");

      }
  }
  ON_EXCEPTION
  {
    delete_3_vectors (l, r, x);
  }
  END_EXCEPTION;

  delete_2_vectors (l, r);
  return (ENT (x));
}

ENTITY *
mod_vector_scalar_integer (VECTOR * volatile l, SCALAR *r)
{
  int i;
  REAL v;

  EASSERT (l, vector, integer);
  EASSERT (r, scalar, integer);

  WITH_HANDLING
  {
    l = (VECTOR *) cast_vector_integer_real (l);
    v = (REAL) r->v.integer;

    for (i = 0; i < l->nn; i++)
      l->a.real[i] = fmod (l->a.real[i], v);
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
mod_vector_scalar_real (VECTOR * volatile l, SCALAR *r)
{
  int i;

  EASSERT (l, vector, real);
  EASSERT (r, scalar, real);

  WITH_HANDLING
  {
    l = (VECTOR *) dup_vector (l);
    for (i = 0; i < l->nn; i++)
      l->a.real[i] = fmod (l->a.real[i], r->v.real);
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
mod_vector_matrix_integer (VECTOR *l, MATRIX *r)
{
  MATRIX *x = NULL;

  WITH_HANDLING
  {
    x = (MATRIX *) vector_to_matrix (EAT (l));
    x = (MATRIX *) mod_matrix_integer (EAT (x), EAT (r));
  }
  ON_EXCEPTION
  {
    delete_vector (l);
    delete_2_matrices (r, x);
  }
  END_EXCEPTION;

  return (ENT (x));
}

ENTITY *
mod_vector_matrix_real (VECTOR *l, MATRIX *r)
{
  MATRIX *x = NULL;

  WITH_HANDLING
  {
    x = (MATRIX *) vector_to_matrix (EAT (l));
    x = (MATRIX *) mod_matrix_real (EAT (x), EAT (r));
  }
  ON_EXCEPTION
  {
    delete_vector (l);
    delete_2_matrices (r, x);
  }
  END_EXCEPTION;

  return (ENT (x));
}

ENTITY *
mod_matrix_vector_integer (MATRIX *l, VECTOR *r)
{
  MATRIX *x = NULL;

  WITH_HANDLING
  {
    x = (MATRIX *) vector_to_matrix (EAT (r));
    x = (MATRIX *) mod_matrix_integer (EAT (l), EAT (x));
  }
  ON_EXCEPTION
  {
    delete_2_matrices (l, x);
    delete_vector (r);
  }
  END_EXCEPTION;

  return (ENT (x));
}

ENTITY *
mod_matrix_vector_real (MATRIX *l, VECTOR *r)
{
  MATRIX *x = NULL;

  WITH_HANDLING
  {
    x = (MATRIX *) vector_to_matrix (EAT (r));
    x = (MATRIX *) mod_matrix_real (EAT (l), EAT (x));
  }
  ON_EXCEPTION
  {
    delete_2_matrices (l, x);
    delete_vector (r);
  }
  END_EXCEPTION;

  return (ENT (x));
}

ENTITY *
mod_matrix_scalar_integer (MATRIX * volatile l, SCALAR *r)
{
  int i;
  REAL v;

  EASSERT (l, matrix, integer);
  EASSERT (r, scalar, integer);

  WITH_HANDLING
  {
    l = (MATRIX *) cast_matrix_integer_real (l);
    v = (REAL) r->v.integer;

    for (i = 0; i < l->nn; i++)
      l->a.real[i] = fmod (l->a.real[i], v);
    if (l->d.real)
      {
	for (i = 0; i < l->nr; i++)
	  l->d.real[i] = fmod (l->d.real[i], v);
      }
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
mod_matrix_scalar_real (MATRIX * volatile l, SCALAR *r)
{
  int i;

  EASSERT (l, matrix, real);
  EASSERT (r, scalar, real);

  WITH_HANDLING
  {
    l = (MATRIX *) dup_matrix (l);

    for (i = 0; i < l->nn; i++)
      l->a.real[i] = fmod (l->a.real[i], r->v.real);
    if (l->d.real)
      {
	for (i = 0; i < l->nr; i++)
	  l->d.real[i] = fmod (l->d.real[i], r->v.real);
      }
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
