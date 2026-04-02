/*
   pile.c -- Pile entities to form matrices.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: pile.c,v 1.2 2003/08/01 04:57:48 ksh Exp $";

#include "pile.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "binop.h"
#include "dense.h"
#include "sparse.h"
#include "full.h"

static char bad_pile_labels[] = "Labels don't match for pile operation.";

ENTITY *
pile_scalar_vector_integer (SCALAR *l, VECTOR *r)
{
  MATRIX *v;

  EASSERT (l, scalar, integer);
  EASSERT (r, vector, integer);

  if (r->ne != 1)
    {
      fail ("Inconsistent dimensions for pile.  Scalar can't be piled on a vector with %d element%s.",
	    r->ne, PLURAL (r->ne));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (r->density)
	{
	case dense:
	  v = (MATRIX *) form_matrix (2, 1, integer, dense);
	  v->a.integer[0] = l->v.integer;
	  v->a.integer[1] = r->a.integer[0];
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (2, 1, integer, dense);
	  v->a.integer[0] = l->v.integer;
	  if (r->nn > 0)
	    v->a.integer[1] = r->a.integer[0];
	  break;
	default:
	  BAD_DENSITY (r->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (r->eid != NULL)
    v->cid = copy_vector ((VECTOR *) r->eid);

  delete_scalar (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
pile_scalar_vector_real (SCALAR *l, VECTOR *r)
{
  MATRIX *v;

  EASSERT (l, scalar, real);
  EASSERT (r, vector, real);

  if (r->ne != 1)
    {
      fail ("Inconsistent dimensions for pile.  Scalar can't be piled on a vector with %d element%s.",
	    r->ne, PLURAL (r->ne));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (r->density)
	{
	case dense:
	  v = (MATRIX *) form_matrix (2, 1, real, dense);
	  v->a.real[0] = l->v.real;
	  v->a.real[1] = r->a.real[0];
	  v->symmetry = general;
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (2, 1, real, dense);
	  v->a.real[0] = l->v.real;
	  if (r->nn > 0)
	    v->a.real[1] = r->a.real[0];
	  break;
	default:
	  BAD_DENSITY (r->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (r->eid != NULL)
    v->cid = copy_vector ((VECTOR *) r->eid);

  delete_scalar (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
pile_scalar_vector_complex (SCALAR *l, VECTOR *r)
{
  MATRIX *v;

  EASSERT (l, scalar, complex);
  EASSERT (r, vector, complex);

  if (r->ne != 1)
    {
      fail ("Inconsistent dimensions for pile.  Scalar can't be piled on a vector with %d element%s.",
	    r->ne, PLURAL (r->ne));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (r->density)
	{
	case dense:
	  v = (MATRIX *) form_matrix (2, 1, complex, dense);
	  v->a.complex[0] = l->v.complex;
	  v->a.complex[1] = r->a.complex[0];
	  v->symmetry = general;
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (2, 1, complex, dense);
	  v->a.complex[0] = l->v.complex;
	  if (r->nn > 0)
	    v->a.complex[1] = r->a.complex[0];
	  break;
	default:
	  BAD_DENSITY (r->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (r->eid != NULL)
    v->cid = copy_vector ((VECTOR *) r->eid);

  delete_scalar (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
pile_scalar_vector_character (SCALAR *l, VECTOR *r)
{
  MATRIX *v;

  EASSERT (l, scalar, character);
  EASSERT (r, vector, character);

  if (r->ne != 1)
    {
      fail ("Inconsistent dimensions for pile.  Scalar can't be piled on a vector with %d element%s.",
	    r->ne, PLURAL (r->ne));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (r->density)
	{
	case dense:
	  v = (MATRIX *) form_matrix (2, 1, character, dense);
	  v->a.character[0] = dup_char (l->v.character);
	  v->a.character[1] = dup_char (r->a.character[0]);
	  v->symmetry = general;
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (2, 1, character, dense);
	  v->a.character[0] = dup_char (l->v.character);
	  if (r->nn > 0)
	    v->a.character[1] = dup_char (r->a.character[0]);
	  break;
	default:
	  BAD_DENSITY (r->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (r->eid != NULL)
    v->cid = copy_vector ((VECTOR *) r->eid);

  delete_scalar (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
pile_scalar_matrix_integer (SCALAR *l, MATRIX *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, scalar, integer);
  EASSERT (r, matrix, integer);

  if (r->nc != 1)
    {
      fail ("Inconsistent dimensions for pile.  Scalar can't be piled on a matrix with %d column%s.",
	    r->nc, PLURAL (r->nc));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (r->density)
	{
	case dense:
	  v = (MATRIX *) make_matrix (1 + r->nr, 1, integer, dense);
	  v->a.integer[0] = l->v.integer;
	  if (r->nn > 0)
	    memcpy (v->a.integer + 1, r->a.integer, r->nn * sizeof (int));
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (1 + r->nr, 1, integer, sparse);
	  v->nn = r->nn + 1;
	  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
	  v->ia[0] = 1;
	  if (r->nn > 0)
	    {
	      for (i = 0; i < r->nr + 1; i++)
		v->ia[i + 1] = r->ia[i] + 1;
	    }
	  else
	    {
	      for (i = 0; i < r->nr + 1; i++)
		v->ia[i + 1] = 2;
	    }
	  v->ja = (int *) MALLOC (v->nn * sizeof (int));
	  v->a.integer = (int *) MALLOC (v->nn * sizeof (int));
	  v->ja[0] = 1;
	  v->a.integer[0] = l->v.integer;
	  for (i = 0; i < r->nn; i++)
	    {
	      v->ja[i + 1] = r->ja[i];
	      v->a.integer[i + 1] = r->a.integer[i];
	    }
	  v = (MATRIX *) apt_matrix (v);
	  break;
	case sparse_upper:
	  return pile_scalar_matrix_integer (l, (MATRIX *) dense_matrix (r));
	default:
	  BAD_DENSITY (r->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (r->rid != NULL)
    {
      v->rid = binop_scalar_vector (BO_APPEND,
			 (SCALAR *) make_scalar (((VECTOR *) r->rid)->type),
				(VECTOR *) copy_vector ((VECTOR *) r->rid));
    }
  if (r->cid != NULL)
    v->cid = copy_vector ((VECTOR *) r->cid);

  delete_scalar (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
pile_scalar_matrix_real (SCALAR *l, MATRIX *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, scalar, real);
  EASSERT (r, matrix, real);

  if (r->nc != 1)
    {
      fail ("Inconsistent dimensions for pile.  Scalar can't be piled on a matrix with %d column%s.",
	    r->nr, PLURAL (r->nr));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (r->density)
	{
	case dense:
	  v = (MATRIX *) make_matrix (1 + r->nr, 1, real, dense);
	  v->a.real[0] = l->v.real;
	  if (r->nn > 0)
	    memcpy (v->a.real + 1, r->a.real, r->nn * sizeof (REAL));
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (1 + r->nr, 1, real, sparse);
	  v->nn = r->nn + 1;
	  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
	  v->ia[0] = 1;
	  if (r->nn > 0)
	    {
	      for (i = 0; i < r->nr + 1; i++)
		v->ia[i + 1] = r->ia[i] + 1;
	    }
	  else
	    {
	      for (i = 0; i < r->nr + 1; i++)
		v->ia[i + 1] = 2;
	    }
	  v->ja = (int *) MALLOC (v->nn * sizeof (int));
	  v->a.real = (REAL *) MALLOC (v->nn * sizeof (REAL));
	  v->ja[0] = 1;
	  v->a.real[0] = l->v.real;
	  for (i = 0; i < r->nn; i++)
	    {
	      v->ja[i + 1] = r->ja[i];
	      v->a.real[i + 1] = r->a.real[i];
	    }
	  v = (MATRIX *) apt_matrix (v);
	  break;
	case sparse_upper:
	  return pile_scalar_matrix_real (l, (MATRIX *) dense_matrix (r));
	default:
	  BAD_DENSITY (r->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (r->rid != NULL)
    {
      v->rid = binop_scalar_vector (BO_APPEND,
			 (SCALAR *) make_scalar (((VECTOR *) r->rid)->type),
				(VECTOR *) copy_vector ((VECTOR *) r->rid));
    }
  if (r->cid != NULL)
    v->cid = copy_vector ((VECTOR *) r->cid);

  delete_scalar (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
pile_scalar_matrix_complex (SCALAR *l, MATRIX *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, scalar, complex);
  EASSERT (r, matrix, complex);

  if (r->nc != 1)
    {
      fail ("Inconsistent dimensions for pile.  Scalar can't be piled on a matrix with %d column%s.",
	    r->nr, PLURAL (r->nr));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (r->density)
	{
	case dense:
	  v = (MATRIX *) make_matrix (1 + r->nr, 1, complex, dense);
	  v->symmetry = (v->nr == 1) ? symmetric : general;
	  v->a.complex[0] = l->v.complex;
	  if (r->nn > 0)
	    memcpy (v->a.complex + 1, r->a.complex,
		    r->nn * sizeof (COMPLEX));
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (1 + r->nr, 1, complex, sparse);
	  v->symmetry = (v->nr == 1) ? symmetric : general;
	  v->nn = r->nn + 1;
	  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
	  v->ia[0] = 1;
	  if (r->nn > 0)
	    {
	      for (i = 0; i < r->nr + 1; i++)
		v->ia[i + 1] = r->ia[i] + 1;
	    }
	  else
	    {
	      for (i = 0; i < r->nr + 1; i++)
		v->ia[i + 1] = 2;
	    }
	  v->ja = (int *) MALLOC (v->nn * sizeof (int));
	  v->a.complex = (COMPLEX *) MALLOC (v->nn * sizeof (COMPLEX));
	  v->ja[0] = 1;
	  v->a.complex[0] = l->v.complex;
	  for (i = 0; i < r->nn; i++)
	    {
	      v->ja[i + 1] = r->ja[i];
	      v->a.complex[i + 1] = r->a.complex[i];
	    }
	  v = (MATRIX *) apt_matrix (v);
	  break;
	case sparse_upper:
	  return pile_scalar_matrix_complex (l, (MATRIX *) dense_matrix (r));
	default:
	  BAD_DENSITY (r->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (r->rid != NULL)
    {
      v->rid = binop_scalar_vector (BO_APPEND,
			 (SCALAR *) make_scalar (((VECTOR *) r->rid)->type),
				(VECTOR *) copy_vector ((VECTOR *) r->rid));
    }
  if (r->cid != NULL)
    v->cid = copy_vector ((VECTOR *) r->cid);

  delete_scalar (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
pile_scalar_matrix_character (SCALAR *l, MATRIX *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, scalar, character);
  EASSERT (r, matrix, character);

  if (r->nc != 1)
    {
      fail ("Inconsistent dimensions for pile.  Scalar can't be piled on a matrix with %d column%s.",
	    r->nr, PLURAL (r->nr));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (r->density)
	{
	case dense:
	  v = (MATRIX *) make_matrix (1 + r->nr, 1, character, dense);
	  v->a.character[0] = dup_char (l->v.character);
	  for (i = 0; i < r->nn; i++)
	    v->a.character[i + 1] = dup_char (r->a.character[i]);
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (1 + r->nr, 1, character, sparse);
	  v->nn = r->nn + 1;
	  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
	  v->ia[0] = 1;
	  if (r->nn > 0)
	    {
	      for (i = 0; i < r->nr + 1; i++)
		v->ia[i + 1] = r->ia[i] + 1;
	    }
	  else
	    {
	      for (i = 0; i < r->nr + 1; i++)
		v->ia[i + 1] = 2;
	    }
	  v->ja = (int *) MALLOC (v->nn * sizeof (int));
	  v->a.character = (char **) MALLOC (v->nn * sizeof (char *));
	  v->ja[0] = 1;
	  v->a.character[0] = dup_char (l->v.character);
	  for (i = 0; i < r->nn; i++)
	    {
	      v->ja[i + 1] = r->ja[i];
	      v->a.character[i + 1] = dup_char (r->a.character[i]);
	    }
	  v = (MATRIX *) apt_matrix (v);
	  break;
	case sparse_upper:
	  return pile_scalar_matrix_character (l, (MATRIX *) dense_matrix (r));
	default:
	  BAD_DENSITY (r->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (r->rid != NULL)
    {
      v->rid = binop_scalar_vector (BO_APPEND,
			 (SCALAR *) make_scalar (((VECTOR *) r->rid)->type),
				(VECTOR *) copy_vector ((VECTOR *) r->rid));
    }
  if (r->cid != NULL)
    v->cid = copy_vector ((VECTOR *) r->cid);

  delete_scalar (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
pile_vector_integer (VECTOR *l, VECTOR *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, vector, integer);
  EASSERT (r, vector, integer);

  if (l->ne != r->ne)
    {
      fail ("Inconsistent dimensions for pile.  Vector with %d element%s can't be piled on a vector with %d element%s.",
	    l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->eid, r->eid))
    {
      fail (bad_pile_labels);
      delete_2_vectors (l, r);
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  switch (r->density)
	    {
	    case dense:
	      v = (MATRIX *) form_matrix (2, l->ne, integer, dense);
	      for (i = 0; i < l->ne; i++)
		{
		  v->a.integer[2 * i] = l->a.integer[i];
		  v->a.integer[2 * i + 1] = r->a.integer[i];
		}
	      v->symmetry = general;
	      break;
	    case sparse:
	      return (pile_vector_integer ((VECTOR *) sparse_vector (l), r));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse:
	  switch (r->density)
	    {
	    case dense:
	      return (pile_vector_integer (l, (VECTOR *) sparse_vector (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (2, l->ne, integer, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  v->symmetry = general;
		  v->ia = (int *) MALLOC (3 * sizeof (int));
		  v->ia[0] = 1;
		  v->ia[1] = l->nn + 1;
		  v->ia[2] = v->nn + 1;
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.integer = (int *) MALLOC (v->nn * sizeof (int));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      memcpy (v->a.integer, l->a.integer, l->nn * sizeof (int));
		    }
		  if (r->nn > 0)
		    {
		      memcpy (v->ja + l->nn, r->ja, r->nn * sizeof (int));
		      memcpy (v->a.integer + l->nn, r->a.integer, r->nn * sizeof (int));
		    }
		}
	      v = (MATRIX *) apt_matrix (v);
	      break;
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->eid != NULL)
    {
      v->cid = copy_entity (l->eid);
    }
  else if (r->eid != NULL)
    {
      v->cid = copy_entity (r->eid);
    }

  delete_2_vectors (l, r);
  return (ENT (v));
}

ENTITY *
pile_vector_real (VECTOR *l, VECTOR *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, vector, real);
  EASSERT (r, vector, real);

  if (l->ne != r->ne)
    {
      fail ("Inconsistent dimensions for pile.  Vector with %d element%s can't be piled on a vector with %d element%s.",
	    r->ne, PLURAL (r->ne), l->ne, PLURAL (l->ne));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->eid, r->eid))
    {
      fail (bad_pile_labels);
      delete_2_vectors (l, r);
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  switch (r->density)
	    {
	    case dense:
	      v = (MATRIX *) form_matrix (2, l->ne, real, dense);
	      for (i = 0; i < l->ne; i++)
		{
		  v->a.real[2 * i] = l->a.real[i];
		  v->a.real[2 * i + 1] = r->a.real[i];
		}
	      v->symmetry = general;
	      break;
	    case sparse:
	      return (pile_vector_real ((VECTOR *) sparse_vector (l), r));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse:
	  switch (r->density)
	    {
	    case dense:
	      return (pile_vector_real (l, (VECTOR *) sparse_vector (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (2, l->ne, real, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  v->symmetry = general;
		  v->ia = (int *) MALLOC (3 * sizeof (int));
		  v->ia[0] = 1;
		  v->ia[1] = l->nn + 1;
		  v->ia[2] = v->nn + 1;
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.real = (REAL *) MALLOC (v->nn * sizeof (REAL));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      memcpy (v->a.real, l->a.real, l->nn * sizeof (REAL));
		    }
		  if (r->nn > 0)
		    {
		      memcpy (v->ja + l->nn, r->ja, r->nn * sizeof (int));
		      memcpy (v->a.real + l->nn, r->a.real, r->nn * sizeof (REAL));
		    }
		}
	      v = (MATRIX *) apt_matrix (v);
	      break;
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->eid != NULL)
    {
      v->cid = copy_entity (l->eid);
    }
  else if (r->eid != NULL)
    {
      v->cid = copy_entity (r->eid);
    }

  delete_2_vectors (l, r);
  return (ENT (v));
}

ENTITY *
pile_vector_complex (VECTOR *l, VECTOR *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, vector, complex);
  EASSERT (r, vector, complex);

  if (l->ne != r->ne)
    {
      fail ("Inconsistent dimensions for pile.  Vector with %d element%s can't be piled on a vector with %d element%s.",
	    r->ne, PLURAL (r->ne), l->ne, PLURAL (l->ne));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->eid, r->eid))
    {
      fail (bad_pile_labels);
      delete_2_vectors (l, r);
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  switch (r->density)
	    {
	    case dense:
	      v = (MATRIX *) form_matrix (2, l->ne, complex, dense);
	      for (i = 0; i < l->ne; i++)
		{
		  v->a.complex[2 * i] = l->a.complex[i];
		  v->a.complex[2 * i + 1] = r->a.complex[i];
		}
	      v->symmetry = general;
	      break;
	    case sparse:
	      return (pile_vector_complex ((VECTOR *) sparse_vector (l), r));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse:
	  switch (r->density)
	    {
	    case dense:
	      return (pile_vector_complex (l, (VECTOR *) sparse_vector (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (2, l->ne, complex, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  v->symmetry = general;
		  v->ia = (int *) MALLOC (3 * sizeof (int));
		  v->ia[0] = 1;
		  v->ia[1] = l->nn + 1;
		  v->ia[2] = v->nn + 1;
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.complex = (COMPLEX *) MALLOC (v->nn * sizeof (COMPLEX));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      memcpy (v->a.complex, l->a.complex, l->nn * sizeof (COMPLEX));
		    }
		  if (r->nn > 0)
		    {
		      memcpy (v->ja + l->nn, r->ja, r->nn * sizeof (int));
		      memcpy (v->a.complex + l->nn, r->a.complex, r->nn * sizeof (COMPLEX));
		    }
		}
	      v = (MATRIX *) apt_matrix (v);
	      break;
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->eid != NULL)
    {
      v->cid = copy_entity (l->eid);
    }
  else if (r->eid != NULL)
    {
      v->cid = copy_entity (r->eid);
    }

  delete_2_vectors (l, r);
  return (ENT (v));
}

ENTITY *
pile_vector_character (VECTOR *l, VECTOR *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, vector, character);
  EASSERT (r, vector, character);

  if (l->ne != r->ne)
    {
      fail ("Inconsistent dimensions for pile.  Vector with %d element%s can't be piled on a vector with %d element%s.",
	    r->ne, PLURAL (r->ne), l->ne, PLURAL (l->ne));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->eid, r->eid))
    {
      fail (bad_pile_labels);
      delete_2_vectors (l, r);
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  switch (r->density)
	    {
	    case dense:
	      v = (MATRIX *) form_matrix (2, l->ne, character, dense);
	      for (i = 0; i < l->ne; i++)
		{
		  v->a.character[2 * i] = dup_char (l->a.character[i]);
		  v->a.character[2 * i + 1] = dup_char (r->a.character[i]);
		}
	      v->symmetry = general;
	      break;
	    case sparse:
	      return (pile_vector_character ((VECTOR *) sparse_vector (l), r));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse:
	  switch (r->density)
	    {
	    case dense:
	      return (pile_vector_character (l, (VECTOR *) sparse_vector (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (2, l->ne, character, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  v->symmetry = general;
		  v->ia = (int *) MALLOC (3 * sizeof (int));
		  v->ia[0] = 1;
		  v->ia[1] = l->nn + 1;
		  v->ia[2] = v->nn + 1;
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.character = (char **) MALLOC (v->nn * sizeof (char *));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      for (i = 0; i < l->nn; i++)
			v->a.character[i] = dup_char (l->a.character[i]);
		    }
		  if (r->nn > 0)
		    {
		      memcpy (v->ja + l->nn, r->ja, r->nn * sizeof (int));
		      for (i = 0; i < r->nn; i++)
			v->a.character[i + l->nn] = dup_char (r->a.character[i]);
		    }
		}
	      v = (MATRIX *) apt_matrix (v);
	      break;
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->eid != NULL)
    {
      v->cid = copy_entity (l->eid);
    }
  else if (r->eid != NULL)
    {
      v->cid = copy_entity (r->eid);
    }

  delete_2_vectors (l, r);
  return (ENT (v));
}

ENTITY *
pile_vector_scalar_integer (VECTOR *l, SCALAR *r)
{
  MATRIX *v;

  EASSERT (l, vector, integer);
  EASSERT (r, scalar, integer);

  if (l->ne != 1)
    {
      fail ("Inconsistent dimensions for pile.  Vector with %d element%s can't be piled on a scalar.",
	    l->ne, PLURAL (l->ne));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  v = (MATRIX *) form_matrix (2, 1, integer, dense);
	  v->a.integer[0] = l->a.integer[0];
	  v->a.integer[1] = r->v.integer;
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (2, 1, integer, dense);
	  if (l->nn > 0)
	    v->a.integer[0] = l->a.integer[0];
	  v->a.integer[1] = r->v.integer;
	  break;
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->eid != NULL)
    v->cid = copy_entity (l->eid);

  delete_vector (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
pile_vector_scalar_real (VECTOR *l, SCALAR *r)
{
  MATRIX *v;

  EASSERT (l, vector, real);
  EASSERT (r, scalar, real);

  if (l->ne != 1)
    {
      fail ("Inconsistent dimensions for pile.  Vector with %d element%s can't be piled on a scalar.",
	    l->ne, PLURAL (l->ne));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  v = (MATRIX *) form_matrix (2, 1, real, dense);
	  v->a.real[0] = l->a.real[0];
	  v->a.real[1] = r->v.real;
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (2, 1, real, dense);
	  if (l->nn > 0)
	    v->a.real[0] = l->a.real[0];
	  v->a.real[1] = r->v.real;
	  break;
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->eid != NULL)
    v->cid = copy_entity (l->eid);

  delete_vector (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
pile_vector_scalar_complex (VECTOR *l, SCALAR *r)
{
  MATRIX *v;

  EASSERT (l, vector, complex);
  EASSERT (r, scalar, complex);

  if (l->ne != 1)
    {
      fail ("Inconsistent dimensions for pile.  Vector with %d element%s can't be piled on a scalar.",
	    l->ne, PLURAL (l->ne));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  v = (MATRIX *) form_matrix (2, 1, complex, dense);
	  v->a.complex[0] = l->a.complex[0];
	  v->a.complex[1] = r->v.complex;
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (2, 1, complex, dense);
	  if (l->nn > 0)
	    v->a.complex[0] = l->a.complex[0];
	  v->a.complex[1] = r->v.complex;
	  break;
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->eid != NULL)
    v->cid = copy_entity (l->eid);

  delete_vector (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
pile_vector_scalar_character (VECTOR *l, SCALAR *r)
{
  MATRIX *v;

  EASSERT (l, vector, character);
  EASSERT (r, scalar, character);

  if (l->ne != 1)
    {
      fail ("Inconsistent dimensions for pile.  Vector with %d element%s can't be piled on a scalar.",
	    l->ne, PLURAL (l->ne));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  v = (MATRIX *) form_matrix (2, 1, character, dense);
	  v->a.character[0] = dup_char (l->a.character[0]);
	  v->a.character[1] = dup_char (r->v.character);
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (2, 1, character, dense);
	  if (l->nn > 0)
	    v->a.character[0] = dup_char (l->a.character[0]);
	  v->a.character[1] = dup_char (r->v.character);
	  break;
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->eid != NULL)
    v->cid = copy_entity (l->eid);

  delete_vector (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
pile_vector_matrix_integer (VECTOR *l, MATRIX *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, vector, integer);
  EASSERT (r, matrix, integer);

  if (l->ne != r->nc)
    {
      fail ("Inconsistent dimensions for pile.  Vector with %d element%s can't be piled on a matrix with %d column%s.",
	    l->ne, PLURAL (l->ne), r->nc, PLURAL (r->nc));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->eid, r->cid))
    {
      fail (bad_pile_labels);
      delete_vector (l);
      delete_matrix (r);
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  switch (r->density)
	    {
	    case dense:
	      v = (MATRIX *) make_matrix (1 + r->nr, l->ne, integer, dense);
	      for (i = 0; i < l->ne; i++)
		{
		  v->a.integer[i * v->nr] = l->a.integer[i];
		  if (r->nr > 0)
		    memcpy (v->a.integer + i * v->nr + 1,
			    r->a.integer + i * r->nr, r->nr * sizeof (int));
		}
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	    case sparse_upper:
	      return (pile_vector_matrix_integer (
					  (VECTOR *) sparse_vector (l), r));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse:
	  switch (r->density)
	    {
	    case dense:
	      return (pile_vector_matrix_integer (
					   (VECTOR *) dense_vector (l), r));
	    case sparse:
	      v = (MATRIX *) make_matrix (1 + r->nr, l->ne, integer, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		    v->symmetry = general;
		  v->symmetry = general;
		  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
		  v->ia[0] = 1;
		  if (r->nn > 0)
		    {
		      for (i = 0; i < r->nr + 1; i++)
			v->ia[i + 1] = r->ia[i] + l->nn;
		    }
		  else
		    {
		      for (i = 0; i < r->nr + 1; i++)
			v->ia[i + 1] = 1 + l->nn;
		    }
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.integer = (int *) MALLOC (v->nn * sizeof (int));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      memcpy (v->a.integer, l->a.integer,
			      l->nn * sizeof (int));
		    }
		  if (r->nn > 0)
		    {
		      memcpy (v->ja + l->nn, r->ja, r->nn * sizeof (int));
		      memcpy (v->a.integer + l->nn,
			      r->a.integer, r->nn * sizeof (int));
		    }
		}
	      v = (MATRIX *) apt_matrix (v);
	      break;
	    case sparse_upper:
	      return (pile_vector_matrix_integer (l,
					       (MATRIX *) full_matrix (r)));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (r->rid != NULL)
    {
      v->rid = binop_scalar_vector (BO_APPEND,
			 (SCALAR *) make_scalar (((VECTOR *) r->rid)->type),
				    (VECTOR *) copy_entity (r->rid));
    }
  if (l->eid != NULL)
    {
      v->cid = copy_entity (l->eid);
    }
  else if (r->cid != NULL)
    {
      v->cid = copy_entity (r->cid);
    }

  delete_vector (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
pile_vector_matrix_real (VECTOR *l, MATRIX *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, vector, real);
  EASSERT (r, matrix, real);

  if (l->ne != r->nc)
    {
      fail ("Inconsistent dimensions for pile.  Vector with %d element%s can't be piled on a matrix with %d column%s.",
	    l->ne, PLURAL (l->ne), r->nc, PLURAL (r->nc));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->eid, r->cid))
    {
      fail (bad_pile_labels);
      delete_vector (l);
      delete_matrix (r);
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  switch (r->density)
	    {
	    case dense:
	      v = (MATRIX *) make_matrix (1 + r->nr, l->ne, real, dense);
	      for (i = 0; i < l->ne; i++)
		{
		  v->a.real[i * v->nr] = l->a.real[i];
		  if (r->nr > 0)
		    memcpy (v->a.real + i * v->nr + 1, r->a.real + i * r->nr,
			    r->nr * sizeof (REAL));
		}
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	    case sparse_upper:
	      return pile_vector_matrix_real ((VECTOR *) sparse_vector (l), r);
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse:
	  switch (r->density)
	    {
	    case dense:
	      return (pile_vector_matrix_real (
					   (VECTOR *) dense_vector (l), r));
	    case sparse:
	      v = (MATRIX *) make_matrix (1 + r->nr, l->ne, real, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		    v->symmetry = general;
		  v->symmetry = general;
		  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
		  v->ia[0] = 1;
		  if (r->nn > 0)
		    {
		      for (i = 0; i < r->nr + 1; i++)
			v->ia[i + 1] = r->ia[i] + l->nn;
		    }
		  else
		    {
		      for (i = 0; i < r->nr + 1; i++)
			v->ia[i + 1] = 1 + l->nn;
		    }
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.real = (REAL *) MALLOC (v->nn * sizeof (REAL));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      memcpy (v->a.real, l->a.real, l->nn * sizeof (REAL));
		    }
		  if (r->nn > 0)
		    {
		      memcpy (v->ja + l->nn, r->ja, r->nn * sizeof (int));
		      memcpy (v->a.real + l->nn, r->a.real,
			      r->nn * sizeof (REAL));
		    }
		}
	      v = (MATRIX *) apt_matrix (v);
	      break;
	    case sparse_upper:
	      return (pile_vector_matrix_real (l,
					       (MATRIX *) full_matrix (r)));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (r->rid != NULL)
    {
      v->rid = binop_scalar_vector (BO_APPEND,
			 (SCALAR *) make_scalar (((VECTOR *) r->rid)->type),
				    (VECTOR *) copy_entity (r->rid));
    }
  if (l->eid != NULL)
    {
      v->cid = copy_entity (l->eid);
    }
  else if (r->cid != NULL)
    {
      v->cid = copy_entity (r->cid);
    }

  delete_vector (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
pile_vector_matrix_complex (VECTOR *l, MATRIX *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, vector, complex);
  EASSERT (r, matrix, complex);

  if (l->ne != r->nc)
    {
      fail ("Inconsistent dimensions for pile.  Vector with %d element%s can't be piled on a matrix with %d column%s.",
	    l->ne, PLURAL (l->ne), r->nc, PLURAL (r->nc));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->eid, r->cid))
    {
      fail (bad_pile_labels);
      delete_vector (l);
      delete_matrix (r);
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  switch (r->density)
	    {
	    case dense:
	      v = (MATRIX *) make_matrix (1 + r->nr, l->ne, complex, dense);
	      for (i = 0; i < l->ne; i++)
		{
		  v->a.complex[i * v->nr] = l->a.complex[i];
		  if (r->nr > 0)
		    memcpy (v->a.complex + i * v->nr + 1, r->a.complex + i * r->nr,
			    r->nr * sizeof (COMPLEX));
		}
	      v->symmetry = (v->nr > 1 || v->nc > 1 || v->nr != v->nc) ?
		general : symmetric;
	      break;
	    case sparse:
	    case sparse_upper:
	      return (pile_vector_matrix_complex (
					  (VECTOR *) sparse_vector (l), r));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse:
	  switch (r->density)
	    {
	    case dense:
	      return (pile_vector_matrix_complex (
					   (VECTOR *) dense_vector (l), r));
	    case sparse:
	      v = (MATRIX *) make_matrix (1 + r->nr, l->ne, complex, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  v->symmetry = (v->nr > 1 || v->nc > 1 || v->nr != v->nc) ?
		    general : symmetric;
		  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
		  v->ia[0] = 1;
		  if (r->nn > 0)
		    {
		      for (i = 0; i < r->nr + 1; i++)
			v->ia[i + 1] = r->ia[i] + l->nn;
		    }
		  else
		    {
		      for (i = 0; i < r->nr + 1; i++)
			v->ia[i + 1] = 1 + l->nn;
		    }
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.complex =
		    (COMPLEX *) MALLOC (v->nn * sizeof (COMPLEX));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      memcpy (v->a.complex, l->a.complex,
			      l->nn * sizeof (COMPLEX));
		    }
		  if (r->nn > 0)
		    {
		      memcpy (v->ja + l->nn, r->ja, r->nn * sizeof (int));
		      memcpy (v->a.complex + l->nn, r->a.complex,
			      r->nn * sizeof (COMPLEX));
		    }
		}
	      v = (MATRIX *) apt_matrix (v);
	      break;
	    case sparse_upper:
	      return (pile_vector_matrix_complex (l,
					       (MATRIX *) full_matrix (r)));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (r->rid != NULL)
    {
      v->rid = binop_scalar_vector (BO_APPEND,
			 (SCALAR *) make_scalar (((VECTOR *) r->rid)->type),
				    (VECTOR *) copy_entity (r->rid));
    }
  if (l->eid != NULL)
    {
      v->cid = copy_entity (l->eid);
    }
  else if (r->cid != NULL)
    {
      v->cid = copy_entity (r->cid);
    }

  delete_vector (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
pile_vector_matrix_character (VECTOR *l, MATRIX *r)
{
  MATRIX *v;
  int i, j;

  EASSERT (l, vector, character);
  EASSERT (r, matrix, character);

  if (l->ne != r->nc)
    {
      fail ("Inconsistent dimensions for pile.  Vector with %d element%s can't be piled on a matrix with %d column%s.",
	    l->ne, PLURAL (l->ne), r->nc, PLURAL (r->nc));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->eid, r->cid))
    {
      fail (bad_pile_labels);
      delete_vector (l);
      delete_matrix (r);
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  switch (r->density)
	    {
	    case dense:
	      v = (MATRIX *) make_matrix (1 + r->nr, l->ne, character, dense);
	      for (i = 0; i < l->ne; i++)
		{
		  v->a.character[i * v->nr] = dup_char (l->a.character[i]);
		  for (j = 0; j < r->nr; j++)
		    v->a.character[i * v->nr + 1 + j] =
		      dup_char (r->a.character[i * r->nr + j]);
		}
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	    case sparse_upper:
	      return (pile_vector_matrix_character (
					  (VECTOR *) sparse_vector (l), r));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse:
	  switch (r->density)
	    {
	    case dense:
	      return (pile_vector_matrix_character (
					   (VECTOR *) dense_vector (l), r));
	    case sparse:
	      v = (MATRIX *) make_matrix (1 + r->nr, l->ne,
					  character, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		    v->symmetry = general;
		  v->symmetry = general;
		  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
		  v->ia[0] = 1;
		  if (r->nn > 0)
		    {
		      for (i = 0; i < r->nr + 1; i++)
			v->ia[i + 1] = r->ia[i] + l->nn;
		    }
		  else
		    {
		      for (i = 0; i < r->nr + 1; i++)
			v->ia[i + 1] = 1 + l->nn;
		    }
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.character =
		    (char **) MALLOC (v->nn * sizeof (char *));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      for (i = 0; i < l->nn; i++)
			v->a.character[i] = dup_char (l->a.character[i]);
		    }
		  if (r->nn > 0)
		    {
		      memcpy (v->ja + l->nn, r->ja, r->nn * sizeof (int));
		      for (i = 0; i < r->nn; i++)
			v->a.character[i + l->nn] =
			  dup_char (r->a.character[i]);
		    }
		}
	      v = (MATRIX *) apt_matrix (v);
	      break;
	    case sparse_upper:
	      return (pile_vector_matrix_character (l,
					       (MATRIX *) full_matrix (r)));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (r->rid != NULL)
    {
      v->rid = binop_scalar_vector (BO_APPEND,
			 (SCALAR *) make_scalar (((VECTOR *) r->rid)->type),
				    (VECTOR *) copy_entity (r->rid));
    }
  if (l->eid != NULL)
    {
      v->cid = copy_entity (l->eid);
    }
  else if (r->cid != NULL)
    {
      v->cid = copy_entity (r->cid);
    }

  delete_vector (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
pile_matrix_integer (MATRIX *l, MATRIX *r)
{
  MATRIX * volatile v;
  int i;

  EASSERT (l, matrix, integer);
  EASSERT (r, matrix, integer);

  if (l->nc != r->nc)
    {
      fail ("Inconsistent dimensions for pile.  Matrix with %d column%s can't be piled on a matrix with %d column%s.",
	    l->nc, PLURAL (l->nc), r->nc, PLURAL (r->nc));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->cid, r->cid))
    {
      fail (bad_pile_labels);
      delete_2_matrices (l, r);
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  switch (r->density)
	    {
	    case dense:
	      v = (MATRIX *) make_matrix (l->nr + r->nr, l->nc,
					  integer, dense);
	      for (i = 0; i < l->nc; i++)
		{
		  if (l->nr > 0)
		    memcpy (v->a.integer + i * v->nr, l->a.integer + i * l->nr,
			    l->nr * sizeof (int));
		  if (r->nr > 0)
		    memcpy (v->a.integer + i * v->nr + l->nr,
			    r->a.integer + i * r->nr, r->nr * sizeof (int));
		}
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	    case sparse_upper:
	      return (pile_matrix_integer ((MATRIX *) sparse_matrix (l), r));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse:
	  switch (r->density)
	    {
	    case dense:
	      return (pile_matrix_integer (l, (MATRIX *) sparse_matrix (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (l->nr + r->nr, l->nc,
					  integer, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		    v->symmetry = general;
		  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
		  if (l->nn > 0)
		    {
		      memcpy (v->ia, l->ia, l->nr * sizeof (int));
		    }
		  else
		    {
		      for (i = 0; i < l->nr; i++)
			v->ia[i] = 1;
		    }
		  if (r->nn > 0)
		    {
		      for (i = 0; i < r->nr + 1; i++)
			v->ia[i + l->nr] = r->ia[i] + l->nn;
		    }
		  else
		    {
		      for (i = 0; i < r->nr + 1; i++)
			v->ia[i + l->nr] = 1 + l->nn;
		    }
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.integer = (int *) MALLOC (v->nn * sizeof (int));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      memcpy (v->a.integer, l->a.integer,
			      l->nn * sizeof (int));
		    }
		  if (r->nn > 0)
		    {
		      memcpy (v->ja + l->nn, r->ja, r->nn * sizeof (int));
		      memcpy (v->a.integer + l->nn, r->a.integer,
			      r->nn * sizeof (int));
		    }
		}
	      v = (MATRIX *) apt_matrix (v);
	      break;
	    case sparse_upper:
	      return (pile_matrix_integer (l, (MATRIX *) full_matrix (r)));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse_upper:
	  return (pile_matrix_integer ((MATRIX *) full_matrix (l), r));
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL || r->rid != NULL)
    {
      WITH_HANDLING
      {
	v->rid = binop_vector_vector (BO_APPEND,
				      (l->rid == NULL) ?
		   (VECTOR *) make_vector (l->nr, ((VECTOR *) r->rid)->type,
					   sparse) :
				 (VECTOR *) copy_vector ((VECTOR *) l->rid),
				      (r->rid == NULL) ?
		   (VECTOR *) make_vector (r->nr, ((VECTOR *) l->rid)->type,
					   sparse) :
				(VECTOR *) copy_vector ((VECTOR *) r->rid));
      }
      ON_EXCEPTION
      {
	delete_3_matrices (l, r, v);
      }
      END_EXCEPTION;
    }
  if (l->cid != NULL)
    {
      v->cid = copy_entity (l->cid);
    }
  else if (r->cid != NULL)
    {
      v->cid = copy_entity (r->cid);
    }

  delete_2_matrices (l, r);
  return (ENT (v));
}

ENTITY *
pile_matrix_real (MATRIX *l, MATRIX *r)
{
  MATRIX * volatile v;
  int i;

  EASSERT (l, matrix, real);
  EASSERT (r, matrix, real);

  if (l->nc != r->nc)
    {
      fail ("Inconsistent dimensions for pile.  Matrix with %d column%s can't be piled on a matrix with %d column%s.",
	    l->nc, PLURAL (l->nc), r->nc, PLURAL (r->nc));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->cid, r->cid))
    {
      fail (bad_pile_labels);
      delete_2_matrices (l, r);
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  switch (r->density)
	    {
	    case dense:
	      v = (MATRIX *) make_matrix (l->nr + r->nr, l->nc, real, dense);
	      for (i = 0; i < l->nc; i++)
		{
		  if (l->nr > 0)
		    memcpy (v->a.real + i * v->nr, l->a.real + i * l->nr,
			    l->nr * sizeof (REAL));
		  if (r->nr > 0)
		    memcpy (v->a.real + i * v->nr + l->nr, r->a.real + i * r->nr,
			    r->nr * sizeof (REAL));
		}
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	    case sparse_upper:
	      return (pile_matrix_real ((MATRIX *) sparse_matrix (l), r));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse:
	  switch (r->density)
	    {
	    case dense:
	      return (pile_matrix_real (l, (MATRIX *) sparse_matrix (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (l->nr + r->nr, l->nc, real, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		    v->symmetry = general;
		  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
		  if (l->nn > 0)
		    {
		      memcpy (v->ia, l->ia, l->nr * sizeof (int));
		    }
		  else
		    {
		      for (i = 0; i < l->nr; i++)
			v->ia[i] = 1;
		    }
		  if (r->nn > 0)
		    {
		      for (i = 0; i < r->nr + 1; i++)
			v->ia[i + l->nr] = r->ia[i] + l->nn;
		    }
		  else
		    {
		      for (i = 0; i < r->nr + 1; i++)
			v->ia[i + l->nr] = 1 + l->nn;
		    }
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.real = (REAL *) MALLOC (v->nn * sizeof (REAL));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      memcpy (v->a.real, l->a.real, l->nn * sizeof (REAL));
		    }
		  if (r->nn > 0)
		    {
		      memcpy (v->ja + l->nn, r->ja, r->nn * sizeof (int));
		      memcpy (v->a.real + l->nn, r->a.real,
			      r->nn * sizeof (REAL));
		    }
		}
	      v = (MATRIX *) apt_matrix (v);
	      break;
	    case sparse_upper:
	      return (pile_matrix_real (l, (MATRIX *) full_matrix (r)));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse_upper:
	  return (pile_matrix_real ((MATRIX *) full_matrix (l), r));
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL || r->rid != NULL)
    {
      WITH_HANDLING
      {
	v->rid = binop_vector_vector (BO_APPEND,
				      (l->rid == NULL) ?
		   (VECTOR *) make_vector (l->nr, ((VECTOR *) r->rid)->type,
					   sparse) :
				 (VECTOR *) copy_vector ((VECTOR *) l->rid),
				      (r->rid == NULL) ?
		   (VECTOR *) make_vector (r->nr, ((VECTOR *) l->rid)->type,
					   sparse) :
				(VECTOR *) copy_vector ((VECTOR *) r->rid));
      }
      ON_EXCEPTION
      {
	delete_3_matrices (l, r, v);
      }
      END_EXCEPTION;
    }
  if (l->cid != NULL)
    {
      v->cid = copy_entity (l->cid);
    }
  else if (r->cid != NULL)
    {
      v->cid = copy_entity (r->cid);
    }

  delete_2_matrices (l, r);
  return (ENT (v));
}

ENTITY *
pile_matrix_complex (MATRIX *l, MATRIX *r)
{
  MATRIX * volatile v;
  int i;

  EASSERT (l, matrix, complex);
  EASSERT (r, matrix, complex);

  if (l->nc != r->nc)
    {
      fail ("Inconsistent dimensions for pile.  Matrix with %d column%s can't be piled on a matrix with %d column%s.",
	    l->nc, PLURAL (l->nc), r->nc, PLURAL (r->nc));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->cid, r->cid))
    {
      fail (bad_pile_labels);
      delete_2_matrices (l, r);
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  switch (r->density)
	    {
	    case dense:
	      v = (MATRIX *) make_matrix (l->nr + r->nr, l->nc,
					  complex, dense);
	      for (i = 0; i < l->nc; i++)
		{
		  if (l->nr > 0)
		    memcpy (v->a.complex + i * v->nr, l->a.complex + i * l->nr,
			    l->nr * sizeof (COMPLEX));
		  if (r->nr > 0)
		    memcpy (v->a.complex + i * v->nr + l->nr,
			    r->a.complex + i * r->nr,
			    r->nr * sizeof (COMPLEX));
		}
	      v->symmetry = (v->nr > 1 || v->nc > 1 || v->nr != v->nc) ?
		general : symmetric;
	      break;
	    case sparse:
	    case sparse_upper:
	      return pile_matrix_complex ((MATRIX *) sparse_matrix (l), r);
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse:
	  switch (r->density)
	    {
	    case dense:
	      return pile_matrix_complex (l, (MATRIX *) sparse_matrix (r));
	    case sparse:
	      v = (MATRIX *) make_matrix (l->nr + r->nr, l->nc,
					  complex, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  v->symmetry = (v->nr > 1 || v->nc > 1 || v->nr != v->nc) ?
		    general : symmetric;
		  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
		  if (l->nn > 0)
		    {
		      memcpy (v->ia, l->ia, l->nr * sizeof (int));
		    }
		  else
		    {
		      for (i = 0; i < l->nr; i++)
			v->ia[i] = 1;
		    }
		  if (r->nn > 0)
		    {
		      for (i = 0; i < r->nr + 1; i++)
			v->ia[i + l->nr] = r->ia[i] + l->nn;
		    }
		  else
		    {
		      for (i = 0; i < r->nr + 1; i++)
			v->ia[i + l->nr] = 1 + l->nn;
		    }
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.complex =
		    (COMPLEX *) MALLOC (v->nn * sizeof (COMPLEX));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      memcpy (v->a.complex, l->a.complex,
			      l->nn * sizeof (COMPLEX));
		    }
		  if (r->nn > 0)
		    {
		      memcpy (v->ja + l->nn, r->ja, r->nn * sizeof (int));
		      memcpy (v->a.complex + l->nn, r->a.complex,
			      r->nn * sizeof (COMPLEX));
		    }
		}
	      v = (MATRIX *) apt_matrix (v);
	      break;
	    case sparse_upper:
	      return pile_matrix_complex (l, (MATRIX *) full_matrix (r));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse_upper:
	  return (pile_matrix_complex ((MATRIX *) full_matrix (l), r));
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL || r->rid != NULL)
    {
      WITH_HANDLING
      {
	v->rid = binop_vector_vector (BO_APPEND,
				      (l->rid == NULL) ?
		   (VECTOR *) make_vector (l->nr, ((VECTOR *) r->rid)->type,
					   sparse) :
				 (VECTOR *) copy_vector ((VECTOR *) l->rid),
				      (r->rid == NULL) ?
		   (VECTOR *) make_vector (r->nr, ((VECTOR *) l->rid)->type,
					   sparse) :
				(VECTOR *) copy_vector ((VECTOR *) r->rid));
      }
      ON_EXCEPTION
      {
	delete_3_matrices (l, r, v);
      }
      END_EXCEPTION;
    }
  if (l->cid != NULL)
    {
      v->cid = copy_entity (l->cid);
    }
  else if (r->cid != NULL)
    {
      v->cid = copy_entity (r->cid);
    }

  delete_2_matrices (l, r);
  return (ENT (v));
}

ENTITY *
pile_matrix_character (MATRIX *l, MATRIX *r)
{
  MATRIX * volatile v;
  int i, j;

  EASSERT (l, matrix, character);
  EASSERT (r, matrix, character);

  if (l->nc != r->nc)
    {
      fail ("Inconsistent dimensions for pile.  Matrix with %d column%s can't be piled on a matrix with %d column%s.",
	    l->nc, PLURAL (l->nc), r->nc, PLURAL (r->nc));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->cid, r->cid))
    {
      fail (bad_pile_labels);
      delete_2_matrices (l, r);
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  switch (r->density)
	    {
	    case dense:
	      v = (MATRIX *) make_matrix (l->nr + r->nr, l->nc,
					  character, dense);
	      for (i = 0; i < l->nc; i++)
		{
		  for (j = 0; j < l->nr; j++)
		    v->a.character[i * v->nr + j] =
		      dup_char (l->a.character[i * l->nr + j]);
		  for (j = 0; j < r->nr; j++)
		    v->a.character[i * v->nr + l->nr + j] =
		      dup_char (r->a.character[i * r->nr + j]);
		}
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	    case sparse_upper:
	      return pile_matrix_character ((MATRIX *) sparse_matrix (l), r);
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse:
	  switch (r->density)
	    {
	    case dense:
	      return pile_matrix_character (l, (MATRIX *) sparse_matrix (r));
	    case sparse:
	      v = (MATRIX *) make_matrix (l->nr + r->nr, l->nc,
					  character, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		    v->symmetry = general;
		  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
		  if (l->nn > 0)
		    {
		      memcpy (v->ia, l->ia, l->nr * sizeof (int));
		    }
		  else
		    {
		      for (i = 0; i < l->nr; i++)
			v->ia[i] = 1;
		    }
		  if (r->nn > 0)
		    {
		      for (i = 0; i < r->nr + 1; i++)
			v->ia[i + l->nr] = r->ia[i] + l->nn;
		    }
		  else
		    {
		      for (i = 0; i < r->nr + 1; i++)
			v->ia[i + l->nr] = 1 + l->nn;
		    }
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.character = (char **) MALLOC (v->nn * sizeof (char *));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      for (i = 0; i < l->nn; i++)
			v->a.character[i] = dup_char (l->a.character[i]);
		    }
		  if (r->nn > 0)
		    {
		      memcpy (v->ja + l->nn, r->ja, r->nn * sizeof (int));
		      for (i = 0; i < r->nn; i++)
			v->a.character[i + l->nn] =
			  dup_char (r->a.character[i]);
		    }
		}
	      v = (MATRIX *) apt_matrix (v);
	      break;
	    case sparse_upper:
	      return pile_matrix_character (l, (MATRIX *) full_matrix (r));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse_upper:
	  return (pile_matrix_character ((MATRIX *) full_matrix (l), r));
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL || r->rid != NULL)
    {
      WITH_HANDLING
      {
	v->rid = binop_vector_vector (BO_APPEND,
				      (l->rid == NULL) ?
		   (VECTOR *) make_vector (l->nr, ((VECTOR *) r->rid)->type,
					   sparse) :
				 (VECTOR *) copy_vector ((VECTOR *) l->rid),
				      (r->rid == NULL) ?
		   (VECTOR *) make_vector (r->nr, ((VECTOR *) l->rid)->type,
					   sparse) :
				(VECTOR *) copy_vector ((VECTOR *) r->rid));
      }
      ON_EXCEPTION
      {
	delete_3_matrices (l, r, v);
      }
      END_EXCEPTION;
    }
  if (l->cid != NULL)
    {
      v->cid = copy_entity (l->cid);
    }
  else if (r->cid != NULL)
    {
      v->cid = copy_entity (r->cid);
    }

  delete_2_matrices (l, r);
  return (ENT (v));
}

ENTITY *
pile_matrix_scalar_integer (MATRIX *l, SCALAR *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, matrix, integer);
  EASSERT (r, scalar, integer);

  if (l->nc != 1)
    {
      fail ("Inconsistent dimensions for pile.  Matrix with %d column%s can't be piled on a scalar.",
	    l->nc, PLURAL (l->nc));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  v = (MATRIX *) make_matrix (l->nr + 1, 1, integer, dense);
	  if (l->nn > 0)
	    {
	      memcpy (v->a.integer, l->a.integer, l->nn * sizeof (int));
	      v->symmetry = general;
	    }
	  v->a.integer[l->nn] = r->v.integer;
	  break;
	case sparse_upper:
	  return pile_matrix_scalar_integer ((MATRIX *) dense_matrix (l), r);
	case sparse:
	  v = (MATRIX *) make_matrix (1 + l->nr, 1, integer, sparse);
	  v->nn = l->nn + 1;
	  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
	  if (l->nn > 0)
	    {
	      memcpy (v->ia, l->ia, (l->nr + 1) * sizeof (int));
	    }
	  else
	    {
	      for (i = 0; i < l->nr + 1; i++)
		v->ia[i] = 1;
	    }
	  v->ia[v->nr] = v->ia[v->nr - 1] + 1;
	  v->ja = (int *) MALLOC (v->nn * sizeof (int));
	  v->a.integer = (int *) MALLOC (v->nn * sizeof (int));
	  for (i = 0; i < l->nn; i++)
	    {
	      v->ja[i] = l->ja[i];
	      v->a.integer[i] = l->a.integer[i];
	    }
	  v->ja[v->nn - 1] = 1;
	  v->a.integer[v->nn - 1] = r->v.integer;
	  v = (MATRIX *) apt_matrix (v);
	  break;
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL)
    {
      v->rid = binop_vector_scalar (BO_APPEND,
				    (VECTOR *) copy_entity (l->rid),
			(SCALAR *) make_scalar (((VECTOR *) l->rid)->type));
    }
  if (l->cid != NULL)
    v->cid = copy_entity (l->cid);

  delete_matrix (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
pile_matrix_scalar_real (MATRIX *l, SCALAR *r)
{
  MATRIX *v;

  EASSERT (l, matrix, real);
  EASSERT (r, scalar, real);

  if (l->nc != 1)
    {
      fail ("Inconsistent dimensions for pile.  Matrix with %d column%s can't be piled on a scalar.",
	    l->nc, PLURAL (l->nc));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  v = (MATRIX *) make_matrix (l->nr + 1, 1, real, dense);
	  if (l->nn > 0)
	    {
	      memcpy (v->a.real, l->a.real, l->nn * sizeof (REAL));
	      v->symmetry = general;
	    }
	  v->a.real[l->nn] = r->v.real;
	  break;
	case sparse:
	  /* Column matrices are best stored dense. */
	case sparse_upper:
	  return pile_matrix_scalar_real ((MATRIX *) dense_matrix (l), r);
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL)
    {
      v->rid = binop_vector_scalar (BO_APPEND,
				    (VECTOR *) copy_entity (l->rid),
			(SCALAR *) make_scalar (((VECTOR *) l->rid)->type));
    }
  if (l->cid != NULL)
    v->cid = copy_entity (l->cid);

  delete_matrix (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
pile_matrix_scalar_complex (MATRIX *l, SCALAR *r)
{
  MATRIX *v;

  EASSERT (l, matrix, complex);
  EASSERT (r, scalar, complex);

  if (l->nc != 1)
    {
      fail ("Inconsistent dimensions for pile.  Matrix with %d column%s can't be piled on a scalar.",
	    l->nc, PLURAL (l->nc));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  v = (MATRIX *) make_matrix (l->nr + 1, 1, complex, dense);
	  v->symmetry = (v->nr == 1) ? symmetric : general;
	  if (l->nn > 0)
	    {
	      memcpy (v->a.complex, l->a.complex, l->nn * sizeof (COMPLEX));
	      v->symmetry = general;
	    }
	  v->a.complex[l->nn] = r->v.complex;
	  break;
	case sparse:
	  /* Column matrices are best stored dense. */
	case sparse_upper:
	  return pile_matrix_scalar_complex ((MATRIX *) dense_matrix (l), r);
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL)
    {
      v->rid = binop_vector_scalar (BO_APPEND,
				    (VECTOR *) copy_entity (l->rid),
			(SCALAR *) make_scalar (((VECTOR *) l->rid)->type));
    }
  if (l->cid != NULL)
    v->cid = copy_entity (l->cid);

  delete_matrix (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
pile_matrix_scalar_character (MATRIX *l, SCALAR *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, matrix, character);
  EASSERT (r, scalar, character);

  if (l->nc != 1)
    {
      fail ("Inconsistent dimensions for pile.  Matrix with %d column%s can't be piled on a scalar.",
	    l->nc, PLURAL (l->nc));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  v = (MATRIX *) make_matrix (l->nr + 1, 1, character, dense);
	  if (l->nn > 0)
	    {
	      for (i = 0; i < l->nn; i++)
		v->a.character[i] = dup_char (l->a.character[i]);
	      v->symmetry = general;
	    }
	  v->a.character[l->nn] = dup_char (r->v.character);
	  break;
	case sparse:
	  /* Column matrices are best stored dense. */
	case sparse_upper:
	  return pile_matrix_scalar_character ((MATRIX *) dense_matrix (l), r);
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL)
    {
      v->rid = binop_vector_scalar (BO_APPEND,
				    (VECTOR *) copy_entity (l->rid),
			(SCALAR *) make_scalar (((VECTOR *) l->rid)->type));
    }
  if (l->cid != NULL)
    v->cid = copy_entity (l->cid);

  delete_matrix (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
pile_matrix_vector_integer (MATRIX *l, VECTOR *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, matrix, integer);
  EASSERT (r, vector, integer);

  if (l->nc != r->ne)
    {
      fail ("Inconsistent dimensions for pile.  Matrix with %d column%s can't be piled on a vector with %d element%s.",
	    l->nc, PLURAL (l->nc), r->ne, PLURAL (r->ne));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->cid, r->eid))
    {
      fail (bad_pile_labels);
      delete_matrix (l);
      delete_vector (r);
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  switch (r->density)
	    {
	    case dense:
	      v = (MATRIX *) make_matrix (l->nr + 1, l->nc, integer, dense);
	      for (i = 0; i < l->nc; i++)
		{
		  if (l->nr > 0)
		    memcpy (v->a.integer + i * v->nr, l->a.integer + i * l->nr,
			    l->nr * sizeof (int));
		  v->a.integer[i * v->nr + l->nr] = r->a.integer[i];
		}
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	    case sparse_upper:
	      return pile_matrix_vector_integer (
					    l, (VECTOR *) dense_vector (r));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse:
	  switch (r->density)
	    {
	    case dense:
	      return (pile_matrix_vector_integer (l,
					     (VECTOR *) sparse_vector (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (l->nr + 1, l->nc, integer, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		    v->symmetry = general;
		  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
		  if (l->nn > 0)
		    {
		      memcpy (v->ia, l->ia, (l->nr + 1) * sizeof (int));
		    }
		  else
		    {
		      for (i = 0; i <= l->nr; i++)
			v->ia[i] = 1;
		    }
		  v->ia[v->nr] = v->nn + 1;
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.integer = (int *) MALLOC (v->nn * sizeof (int));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      memcpy (v->a.integer, l->a.integer,
			      l->nn * sizeof (int));
		    }
		  if (r->nn > 0)
		    {
		      memcpy (v->ja + l->nn, r->ja, r->nn * sizeof (int));
		      memcpy (v->a.integer + l->nn, r->a.integer,
			      r->nn * sizeof (int));
		    }
		}
	      v = (MATRIX *) apt_matrix (v);
	      break;
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse_upper:
	  return (pile_matrix_vector_integer (
					    (MATRIX *) full_matrix (l), r));
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL)
    {
      v->rid = binop_vector_scalar (BO_APPEND,
				    (VECTOR *) copy_entity (l->rid),
			(SCALAR *) make_scalar (((VECTOR *) l->rid)->type));
    }
  if (l->cid != NULL)
    v->cid = copy_entity (l->cid);

  delete_matrix (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
pile_matrix_vector_real (MATRIX *l, VECTOR *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, matrix, real);
  EASSERT (r, vector, real);

  if (l->nc != r->ne)
    {
      fail ("Inconsistent dimensions for pile.  Matrix with %d column%s can't be piled on a vector with %d element%s.",
	    l->nc, PLURAL (l->nc), r->ne, PLURAL (r->ne));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->cid, r->eid))
    {
      fail (bad_pile_labels);
      delete_matrix (l);
      delete_vector (r);
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  switch (r->density)
	    {
	    case dense:
	      v = (MATRIX *) make_matrix (l->nr + 1, l->nc, real, dense);
	      for (i = 0; i < l->nc; i++)
		{
		  if (l->nr > 0)
		    memcpy (v->a.real + i * v->nr, l->a.real + i * l->nr,
			    l->nr * sizeof (REAL));
		  v->a.real[i * v->nr + l->nr] = r->a.real[i];
		}
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	    case sparse_upper:
	      return pile_matrix_vector_real (l, (VECTOR *) dense_vector (r));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse:
	  switch (r->density)
	    {
	    case dense:
	      return pile_matrix_vector_real (l, (VECTOR *) sparse_vector (r));
	    case sparse:
	      v = (MATRIX *) make_matrix (l->nr + 1, l->nc, real, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		    v->symmetry = general;
		  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
		  if (l->nn > 0)
		    {
		      memcpy (v->ia, l->ia, (l->nr + 1) * sizeof (int));
		    }
		  else
		    {
		      for (i = 0; i <= l->nr; i++)
			v->ia[i] = 1;
		    }
		  v->ia[v->nr] = v->nn + 1;
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.real = (REAL *) MALLOC (v->nn * sizeof (REAL));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      memcpy (v->a.real, l->a.real, l->nn * sizeof (REAL));
		    }
		  if (r->nn > 0)
		    {
		      memcpy (v->ja + l->nn, r->ja, r->nn * sizeof (int));
		      memcpy (v->a.real + l->nn, r->a.real,
			      r->nn * sizeof (REAL));
		    }
		}
	      v = (MATRIX *) apt_matrix (v);
	      break;
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse_upper:
	  return pile_matrix_vector_real ((MATRIX *) full_matrix (l), r);
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL)
    {
      v->rid = binop_vector_scalar (BO_APPEND,
				    (VECTOR *) copy_entity (l->rid),
			(SCALAR *) make_scalar (((VECTOR *) l->rid)->type));
    }
  if (l->cid != NULL)
    v->cid = copy_entity (l->cid);

  delete_matrix (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
pile_matrix_vector_complex (MATRIX *l, VECTOR *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, matrix, complex);
  EASSERT (r, vector, complex);

  if (l->nc != r->ne)
    {
      fail ("Inconsistent dimensions for pile.  Matrix with %d column%s can't be piled on a vector with %d element%s.",
	    l->nc, PLURAL (l->nc), r->ne, PLURAL (r->ne));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->cid, r->eid))
    {
      fail (bad_pile_labels);
      delete_matrix (l);
      delete_vector (r);
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  switch (r->density)
	    {
	    case dense:
	      v = (MATRIX *) make_matrix (l->nr + 1, l->nc, complex, dense);
	      for (i = 0; i < l->nc; i++)
		{
		  if (l->nr > 0)
		    memcpy (v->a.complex + i * v->nr, l->a.complex + i * l->nr,
			    l->nr * sizeof (COMPLEX));
		  v->a.complex[i * v->nr + l->nr] = r->a.complex[i];
		}
	      v->symmetry = (v->nr > 1 || v->nc > 1 || v->nr != v->nc) ?
		general : symmetric;
	      break;
	    case sparse:
	    case sparse_upper:
	      return (pile_matrix_vector_complex (l,
					      (VECTOR *) dense_vector (r)));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse:
	  switch (r->density)
	    {
	    case dense:
	      return (pile_matrix_vector_complex (l,
					     (VECTOR *) sparse_vector (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (l->nr + 1, l->nc, complex, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  v->symmetry = (v->nr > 1 || v->nc > 1 || v->nr != v->nc) ?
		    general : symmetric;
		  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
		  if (l->nn > 0)
		    {
		      memcpy (v->ia, l->ia, (l->nr + 1) * sizeof (int));
		    }
		  else
		    {
		      for (i = 0; i <= l->nr; i++)
			v->ia[i] = 1;
		    }
		  v->ia[v->nr] = v->nn + 1;
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.complex =
		    (COMPLEX *) MALLOC (v->nn * sizeof (COMPLEX));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      memcpy (v->a.complex, l->a.complex,
			      l->nn * sizeof (COMPLEX));
		    }
		  if (r->nn > 0)
		    {
		      memcpy (v->ja + l->nn, r->ja, r->nn * sizeof (int));
		      memcpy (v->a.complex + l->nn, r->a.complex,
			      r->nn * sizeof (COMPLEX));
		    }
		}
	      v = (MATRIX *) apt_matrix (v);
	      break;
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse_upper:
	  return pile_matrix_vector_complex ((MATRIX *) full_matrix (l), r);
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL)
    {
      v->rid = binop_vector_scalar (BO_APPEND,
				    (VECTOR *) copy_entity (l->rid),
			(SCALAR *) make_scalar (((VECTOR *) l->rid)->type));
    }
  if (l->cid != NULL)
    v->cid = copy_entity (l->cid);

  delete_matrix (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
pile_matrix_vector_character (MATRIX *l, VECTOR *r)
{
  MATRIX *v;
  int i, j;

  EASSERT (l, matrix, character);
  EASSERT (r, vector, character);

  if (l->nc != r->ne)
    {
      fail ("Inconsistent dimensions for pile.  Matrix with %d column%s can't be piled on a vector with %d element%s.",
	    l->nc, PLURAL (l->nc), r->ne, PLURAL (r->ne));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->cid, r->eid))
    {
      fail (bad_pile_labels);
      delete_matrix (l);
      delete_vector (r);
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  switch (r->density)
	    {
	    case dense:
	      v = (MATRIX *) make_matrix (l->nr + 1, l->nc, character, dense);
	      for (i = 0; i < l->nc; i++)
		{
		  for (j = 0; j < l->nr; j++)
		    v->a.character[i * v->nr + j] =
		      dup_char (l->a.character[i * l->nr + j]);
		  v->a.character[i * v->nr + l->nr] =
		    dup_char (r->a.character[i]);
		}
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	    case sparse_upper:
	      return (pile_matrix_vector_character (l,
					      (VECTOR *) dense_vector (r)));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse:
	  switch (r->density)
	    {
	    case dense:
	      return (pile_matrix_vector_character (l,
					     (VECTOR *) sparse_vector (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (l->nr + 1, l->nc,
					  character, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		    v->symmetry = general;
		  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
		  if (l->nn > 0)
		    {
		      memcpy (v->ia, l->ia, (l->nr + 1) * sizeof (int));
		    }
		  else
		    {
		      for (i = 0; i <= l->nr; i++)
			v->ia[i] = 1;
		    }
		  v->ia[v->nr] = v->nn + 1;
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.character = (char **) MALLOC (v->nn * sizeof (char *));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      for (i = 0; i < l->nn; i++)
			v->a.character[i] = dup_char (l->a.character[i]);
		    }
		  if (r->nn > 0)
		    {
		      memcpy (v->ja + l->nn, r->ja, r->nn * sizeof (int));
		      for (i = 0; i < r->nn; i++)
			v->a.character[i + l->nn] =
			  dup_char (r->a.character[i]);
		    }
		}
	      v = (MATRIX *) apt_matrix (v);
	      break;
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse_upper:
	  return (pile_matrix_vector_character (
					    (MATRIX *) full_matrix (l), r));
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL)
    {
      v->rid = binop_vector_scalar (BO_APPEND,
				    (VECTOR *) copy_entity (l->rid),
			(SCALAR *) make_scalar (((VECTOR *) l->rid)->type));
    }
  if (l->cid != NULL)
    v->cid = copy_entity (l->cid);

  delete_matrix (l);
  delete_vector (r);
  return (ENT (v));
}
