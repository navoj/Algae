/*                               -*- Mode: C -*- 
 *********************************************
 * Copyright (C) 1994-2003  K. Scott Hunziker
 * Copyright (C) 1994 by The Boeing Company. *
 *********************************************
 *
 * append.c -- Append entities to form vectors or matrices.
 *
 */

static char rcsid[] =
"$Id: append.c,v 1.2 2003/08/01 04:57:46 ksh Exp $";

#include "append.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "binop.h"
#include "dense.h"
#include "sparse.h"
#include "full.h"

static char bad_append_labels[] = "Labels don't match for append operation.";

ENTITY *
append_scalar_vector_integer (SCALAR *l, VECTOR *r)
{
  VECTOR *v;
  int i;

  EASSERT (l, scalar, integer);
  EASSERT (r, vector, integer);

  switch (r->density)
    {
    case dense:
      v = (VECTOR *) form_vector (r->ne + 1, integer, dense);
      v->a.integer[0] = l->v.integer;
      if (r->ne > 0)
	memcpy (v->a.integer + 1, r->a.integer, r->ne * sizeof (int));
      break;
    case sparse:
      v = (VECTOR *) make_vector (r->ne + 1, integer, sparse);
      v->nn = r->nn + 1;
      v->ja = (int *) MALLOC (v->nn * sizeof (int));
      v->ja[0] = 1;
      for (i = 1; i < v->nn; i++)
	v->ja[i] = r->ja[i - 1] + 1;
      v->a.integer = (int *) MALLOC (v->nn * sizeof (int));
      v->a.integer[0] = l->v.integer;
      if (r->nn > 0)
	memcpy (v->a.integer + 1, r->a.integer, r->nn * sizeof (int));
      v = (VECTOR *) apt_vector (v);
      break;
    default:
      BAD_DENSITY (r->density);
      delete_scalar (l);
      delete_vector (r);
      raise_exception ();
    }

  if (r->eid != NULL)
    {
      v->eid = binop_scalar_vector (BO_APPEND,
			 (SCALAR *) make_scalar (((VECTOR *) r->eid)->type),
				(VECTOR *) copy_vector ((VECTOR *) r->eid));
    }

  delete_scalar (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
append_scalar_vector_real (SCALAR *l, VECTOR *r)
{
  VECTOR *v;
  int i;

  EASSERT (l, scalar, real);
  EASSERT (r, vector, real);

  switch (r->density)
    {
    case dense:
      v = (VECTOR *) form_vector (r->ne + 1, real, dense);
      v->a.real[0] = l->v.real;
      if (r->ne > 0)
	memcpy (v->a.real + 1, r->a.real, r->ne * sizeof (REAL));
      break;
    case sparse:
      v = (VECTOR *) make_vector (r->ne + 1, real, sparse);
      v->nn = r->nn + 1;
      v->ja = (int *) MALLOC (v->nn * sizeof (int));
      v->ja[0] = 1;
      for (i = 1; i < v->nn; i++)
	v->ja[i] = r->ja[i - 1] + 1;
      v->a.real = (REAL *) MALLOC (v->nn * sizeof (REAL));
      v->a.real[0] = l->v.real;
      if (r->nn > 0)
	memcpy (v->a.real + 1, r->a.real, r->nn * sizeof (REAL));
      v = (VECTOR *) apt_vector (v);
      break;
    default:
      BAD_DENSITY (r->density);
      delete_scalar (l);
      delete_vector (r);
      raise_exception ();
    }

  if (r->eid != NULL)
    {
      v->eid = binop_scalar_vector (BO_APPEND,
			 (SCALAR *) make_scalar (((VECTOR *) r->eid)->type),
				(VECTOR *) copy_vector ((VECTOR *) r->eid));
    }

  delete_scalar (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
append_scalar_vector_complex (SCALAR *l, VECTOR *r)
{
  VECTOR *v;
  int i;

  EASSERT (l, scalar, complex);
  EASSERT (r, vector, complex);

  switch (r->density)
    {
    case dense:
      v = (VECTOR *) form_vector (r->ne + 1, complex, dense);
      v->a.complex[0] = l->v.complex;
      if (r->ne > 0)
	memcpy (v->a.complex + 1, r->a.complex, r->ne * sizeof (COMPLEX));
      break;
    case sparse:
      v = (VECTOR *) make_vector (r->ne + 1, complex, sparse);
      v->nn = r->nn + 1;
      v->ja = (int *) MALLOC (v->nn * sizeof (int));
      v->ja[0] = 1;
      for (i = 1; i < v->nn; i++)
	v->ja[i] = r->ja[i - 1] + 1;
      v->a.complex = (COMPLEX *) MALLOC (v->nn * sizeof (COMPLEX));
      v->a.complex[0] = l->v.complex;
      if (r->nn > 0)
	memcpy (v->a.complex + 1, r->a.complex, r->nn * sizeof (COMPLEX));
      v = (VECTOR *) apt_vector (v);
      break;
    default:
      BAD_DENSITY (r->density);
      delete_scalar (l);
      delete_vector (r);
      raise_exception ();
    }

  if (r->eid != NULL)
    {
      v->eid = binop_scalar_vector (BO_APPEND,
			 (SCALAR *) make_scalar (((VECTOR *) r->eid)->type),
				(VECTOR *) copy_vector ((VECTOR *) r->eid));
    }

  delete_scalar (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
append_scalar_vector_character (SCALAR *l, VECTOR *r)
{
  VECTOR *v;
  int i;

  EASSERT (l, scalar, character);
  EASSERT (r, vector, character);

  switch (r->density)
    {
    case dense:
      v = (VECTOR *) form_vector (r->ne + 1, character, dense);
      v->a.character[0] = dup_char (l->v.character);
      for (i = 0; i < r->ne; i++)
	v->a.character[i + 1] = dup_char (r->a.character[i]);
      break;
    case sparse:
      v = (VECTOR *) make_vector (r->ne + 1, character, sparse);
      v->nn = r->nn + 1;
      v->ja = (int *) MALLOC (v->nn * sizeof (int));
      v->ja[0] = 1;
      for (i = 1; i < v->nn; i++)
	v->ja[i] = r->ja[i - 1] + 1;
      v->a.character = (char **) MALLOC (v->nn * sizeof (char *));
      v->a.character[0] = dup_char (l->v.character);
      for (i = 0; i < r->nn; i++)
	v->a.character[i + 1] = dup_char (r->a.character[i]);
      v = (VECTOR *) apt_vector (v);
      break;
    default:
      BAD_DENSITY (r->density);
      delete_scalar (l);
      delete_vector (r);
      raise_exception ();
    }

  if (r->eid != NULL)
    {
      v->eid = binop_scalar_vector (BO_APPEND,
			 (SCALAR *) make_scalar (((VECTOR *) r->eid)->type),
				(VECTOR *) copy_vector ((VECTOR *) r->eid));
    }

  delete_scalar (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
append_scalar_matrix_integer (SCALAR *l, MATRIX *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, scalar, integer);
  EASSERT (r, matrix, integer);

  if (r->nr != 1)
    {
      fail ("Inconsistent dimensions for append.  Matrix with %d row%s can't be appended to a scalar.",
	    r->nr, PLURAL (r->nr));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (r->density)
	{
	case dense:
	  v = (MATRIX *) form_matrix (r->nr, r->nc + 1, integer, dense);
	  v->a.integer[0] = l->v.integer;
	  if (r->nc > 0)
	    {
	      memcpy (v->a.integer + 1, r->a.integer, r->nn * sizeof (int));
	      v->symmetry = general;
	    }
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (r->nr, r->nc + 1, integer, sparse);
	  if (r->nc > 0)
	    v->symmetry = general;
	  v->nn = r->nn + 1;
	  v->ja = (int *) MALLOC (v->nn * sizeof (int));
	  v->ja[0] = 1;
	  for (i = 1; i < v->nn; i++)
	    v->ja[i] = r->ja[i - 1] + 1;
	  v->a.integer = (int *) MALLOC (v->nn * sizeof (int));
	  v->a.integer[0] = l->v.integer;
	  if (r->nn > 0)
	    memcpy (v->a.integer + 1, r->a.integer, r->nn * sizeof (int));
	  v->ia = (int *) MALLOC (2 * sizeof (int));
	  v->ia[0] = 1;
	  v->ia[1] = v->nn + 1;
	  v = (MATRIX *) apt_matrix (v);
	  break;
	case sparse_upper:
	  return (append_scalar_matrix_integer (l,
					      (MATRIX *) dense_matrix (r)));
	default:
	  BAD_DENSITY (r->density);
	  delete_scalar (l);
	  delete_matrix (r);
	  raise_exception ();
	}
    }

  if (r->rid != NULL)
    v->rid = copy_vector ((VECTOR *) r->rid);
  if (r->cid != NULL)
    {
      v->cid = binop_scalar_vector (BO_APPEND,
			 (SCALAR *) make_scalar (((VECTOR *) r->cid)->type),
				(VECTOR *) copy_vector ((VECTOR *) r->cid));
    }

  delete_scalar (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
append_scalar_matrix_real (SCALAR *l, MATRIX *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, scalar, real);
  EASSERT (r, matrix, real);

  if (r->nr != 1)
    {
      fail ("Inconsistent dimensions for append.  Matrix with %d row%s can't be appended to a scalar.",
	    r->nr, PLURAL (r->nr));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (r->density)
	{
	case dense:
	  v = (MATRIX *) form_matrix (r->nr, r->nc + 1, real, dense);
	  v->a.real[0] = l->v.real;
	  if (r->nc > 0)
	    {
	      memcpy (v->a.real + 1, r->a.real, r->nn * sizeof (REAL));
	      v->symmetry = general;
	    }
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (r->nr, r->nc + 1, real, sparse);
	  if (r->nc > 0)
	    v->symmetry = general;
	  v->nn = r->nn + 1;
	  v->ja = (int *) MALLOC (v->nn * sizeof (int));
	  v->ja[0] = 1;
	  for (i = 1; i < v->nn; i++)
	    v->ja[i] = r->ja[i - 1] + 1;
	  v->a.real = (REAL *) MALLOC (v->nn * sizeof (REAL));
	  v->a.real[0] = l->v.real;
	  if (r->nn > 0)
	    memcpy (v->a.real + 1, r->a.real, r->nn * sizeof (REAL));
	  v->ia = (int *) MALLOC (2 * sizeof (int));
	  v->ia[0] = 1;
	  v->ia[1] = v->nn + 1;
	  v = (MATRIX *) apt_matrix (v);
	  break;
	case sparse_upper:
	  return (append_scalar_matrix_real (l,
					     (MATRIX *) dense_matrix (r)));
	default:
	  BAD_DENSITY (r->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (r->rid != NULL)
    v->rid = copy_vector ((VECTOR *) r->rid);
  if (r->cid != NULL)
    {
      v->cid = binop_scalar_vector (BO_APPEND,
			 (SCALAR *) make_scalar (((VECTOR *) r->cid)->type),
				(VECTOR *) copy_vector ((VECTOR *) r->cid));
    }

  delete_scalar (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
append_scalar_matrix_complex (SCALAR *l, MATRIX *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, scalar, complex);
  EASSERT (r, matrix, complex);

  if (r->nr != 1)
    {
      fail ("Inconsistent dimensions for append.  Matrix with %d row%s can't be appended to a scalar.",
	    r->nr, PLURAL (r->nr));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (r->density)
	{
	case dense:
	  v = (MATRIX *) form_matrix (r->nr, r->nc + 1, complex, dense);
	  v->a.complex[0] = l->v.complex;
	  if (r->nc > 0)
	    {
	      memcpy (v->a.complex + 1, r->a.complex,
		      r->nn * sizeof (COMPLEX));
	      v->symmetry = general;
	    }
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (r->nr, r->nc + 1, complex, sparse);
	  v->nn = r->nn + 1;
	  v->ja = (int *) MALLOC (v->nn * sizeof (int));
	  v->ja[0] = 1;
	  for (i = 1; i < v->nn; i++)
	    v->ja[i] = r->ja[i - 1] + 1;
	  v->a.complex = (COMPLEX *) MALLOC (v->nn * sizeof (COMPLEX));
	  v->a.complex[0] = l->v.complex;
	  if (r->nn > 0)
	    memcpy (v->a.complex + 1, r->a.complex,
		    r->nn * sizeof (COMPLEX));
	  v->ia = (int *) MALLOC (2 * sizeof (int));
	  v->ia[0] = 1;
	  v->ia[1] = v->nn + 1;
	  v = (MATRIX *) apt_matrix (v);
	  break;
	case sparse_upper:
	  return (append_scalar_matrix_complex (l,
					      (MATRIX *) dense_matrix (r)));
	default:
	  BAD_DENSITY (r->density);
	  delete_scalar (l);
	  delete_matrix (r);
	  raise_exception ();
	}
    }

  if (r->rid != NULL)
    v->rid = copy_vector ((VECTOR *) r->rid);
  if (r->cid != NULL)
    {
      v->cid = binop_scalar_vector (BO_APPEND,
			 (SCALAR *) make_scalar (((VECTOR *) r->cid)->type),
				(VECTOR *) copy_vector ((VECTOR *) r->cid));
    }

  delete_scalar (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
append_scalar_matrix_character (SCALAR *l, MATRIX *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, scalar, character);
  EASSERT (r, matrix, character);

  if (r->nr != 1)
    {
      fail ("Inconsistent dimensions for append.  Matrix with %d row%s can't be appended to a scalar.",
	    r->nr, PLURAL (r->nr));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (r->density)
	{
	case dense:
	  v = (MATRIX *) form_matrix (r->nr, r->nc + 1, character, dense);
	  v->a.character[0] = dup_char (l->v.character);
	  if (r->nc > 0)
	    {
	      for (i = 0; i < r->nn; i++)
		v->a.character[i + 1] = dup_char (r->a.character[i]);
	      v->symmetry = general;
	    }
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (r->nr, r->nc + 1, character, sparse);
	  v->nn = r->nn + 1;
	  v->ja = (int *) MALLOC (v->nn * sizeof (int));
	  v->ja[0] = 1;
	  for (i = 1; i < v->nn; i++)
	    v->ja[i] = r->ja[i - 1] + 1;
	  v->a.character = (char **) MALLOC (v->nn * sizeof (char *));
	  v->a.character[0] = dup_char (l->v.character);
	  for (i = 0; i < r->nn; i++)
	    v->a.character[i + 1] = dup_char (r->a.character[i]);
	  if (r->nc > 0)
	    v->symmetry = general;
	  v->ia = (int *) MALLOC (2 * sizeof (int));
	  v->ia[0] = 1;
	  v->ia[1] = v->nn + 1;
	  v = (MATRIX *) apt_matrix (v);
	  break;
	case sparse_upper:
	  return (append_scalar_matrix_character (l,
					      (MATRIX *) dense_matrix (r)));
	default:
	  BAD_DENSITY (r->density);
	  delete_scalar (l);
	  delete_matrix (r);
	  raise_exception ();
	}
    }

  if (r->rid != NULL)
    v->rid = copy_vector ((VECTOR *) r->rid);
  if (r->cid != NULL)
    {
      v->cid = binop_scalar_vector (BO_APPEND,
			 (SCALAR *) make_scalar (((VECTOR *) r->cid)->type),
				(VECTOR *) copy_vector ((VECTOR *) r->cid));
    }

  delete_scalar (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
append_vector_integer (VECTOR *l, VECTOR *r)
{
  VECTOR * volatile v;
  int i;

  EASSERT (l, vector, integer);
  EASSERT (r, vector, integer);

  switch (l->density)
    {
    case dense:
      switch (r->density)
	{
	case dense:
	  v = (VECTOR *) form_vector (l->ne + r->ne, integer, dense);
	  if (l->nn > 0)
	    memcpy (v->a.integer, l->a.integer,
		    l->ne * sizeof (int));
	  if (r->nn > 0)
	    memcpy (v->a.integer + l->ne, r->a.integer,
		    r->ne * sizeof (int));
	  break;
	case sparse:
	  return (append_vector_integer ((VECTOR *) sparse_vector (l), r));
	default:
	  BAD_DENSITY (r->density);
	  delete_2_vectors (l, r);
	  raise_exception ();
	}
      break;
    case sparse:
      switch (r->density)
	{
	case dense:
	  return (append_vector_integer (l, (VECTOR *) sparse_vector (r)));
	case sparse:
	  v = (VECTOR *) make_vector (l->ne + r->ne, integer, sparse);
	  v->nn = l->nn + r->nn;
	  if (v->nn > 0)
	    {
	      v->ja = (int *) MALLOC (v->nn * sizeof (int));
	      v->a.integer = (int *) MALLOC (v->nn * sizeof (int));
	      if (l->nn > 0)
		{
		  memcpy (v->ja, l->ja, l->nn * sizeof (int));
		  memcpy (v->a.integer, l->a.integer, l->nn * sizeof (int));
		}
	      for (i = 0; i < r->nn; i++)
		{
		  v->ja[i + l->nn] = r->ja[i] + l->ne;
		  v->a.integer[i + l->nn] = r->a.integer[i];
		}
	      v = (VECTOR *) apt_vector (v);
	    }
	  break;
	default:
	  BAD_DENSITY (r->density);
	  delete_2_vectors (l, r);
	  raise_exception ();
	}
      break;
    default:
      BAD_DENSITY (l->density);
      delete_2_vectors (l, r);
      raise_exception ();
    }

  if (l->eid != NULL || r->eid != NULL)
    {
      WITH_HANDLING
      {
	v->eid = binop_vector_vector (BO_APPEND,
				      (l->eid == NULL) ?
		   (VECTOR *) make_vector (l->ne, ((VECTOR *) r->eid)->type,
					   sparse) :
				 (VECTOR *) copy_vector ((VECTOR *) l->eid),
				      (r->eid == NULL) ?
		   (VECTOR *) make_vector (r->ne, ((VECTOR *) l->eid)->type,
					   sparse) :
				(VECTOR *) copy_vector ((VECTOR *) r->eid));
      }
      ON_EXCEPTION
      {
	delete_3_vectors (l, r, v);
      }
      END_EXCEPTION;
    }

  delete_2_vectors (l, r);
  return (ENT (v));
}

ENTITY *
append_vector_real (VECTOR *l, VECTOR *r)
{
  VECTOR * volatile v;
  int i;

  EASSERT (l, vector, real);
  EASSERT (r, vector, real);

  switch (l->density)
    {
    case dense:
      switch (r->density)
	{
	case dense:
	  v = (VECTOR *) form_vector (l->ne + r->ne, real, dense);
	  if (l->nn > 0)
	    memcpy (v->a.real, l->a.real,
		    l->ne * sizeof (REAL));
	  if (r->nn > 0)
	    memcpy (v->a.real + l->ne, r->a.real,
		    r->ne * sizeof (REAL));
	  break;
	case sparse:
	  return (append_vector_real ((VECTOR *) sparse_vector (l), r));
	default:
	  BAD_DENSITY (r->density);
	  delete_2_vectors (l, r);
	  raise_exception ();
	}
      break;
    case sparse:
      switch (r->density)
	{
	case dense:
	  return (append_vector_real (l, (VECTOR *) sparse_vector (r)));
	case sparse:
	  v = (VECTOR *) make_vector (l->ne + r->ne, real, sparse);
	  v->nn = l->nn + r->nn;
	  if (v->nn > 0)
	    {
	      v->ja = (int *) MALLOC (v->nn * sizeof (int));
	      v->a.real = (REAL *) MALLOC (v->nn * sizeof (REAL));
	      if (l->nn > 0)
		{
		  memcpy (v->ja, l->ja, l->nn * sizeof (int));
		  memcpy (v->a.real, l->a.real, l->nn * sizeof (REAL));
		}
	      for (i = 0; i < r->nn; i++)
		{
		  v->ja[i + l->nn] = r->ja[i] + l->ne;
		  v->a.real[i + l->nn] = r->a.real[i];
		}
	      v = (VECTOR *) apt_vector (v);
	    }
	  break;
	default:
	  BAD_DENSITY (r->density);
	  delete_2_vectors (l, r);
	  raise_exception ();
	}
      break;
    default:
      BAD_DENSITY (l->density);
      delete_2_vectors (l, r);
      raise_exception ();
    }

  if (l->eid != NULL || r->eid != NULL)
    {
      WITH_HANDLING
      {
	v->eid = binop_vector_vector (BO_APPEND,
				      (l->eid == NULL) ?
		   (VECTOR *) make_vector (l->ne, ((VECTOR *) r->eid)->type,
					   sparse) :
				 (VECTOR *) copy_vector ((VECTOR *) l->eid),
				      (r->eid == NULL) ?
		   (VECTOR *) make_vector (r->ne, ((VECTOR *) l->eid)->type,
					   sparse) :
				(VECTOR *) copy_vector ((VECTOR *) r->eid));
      }
      ON_EXCEPTION
      {
	delete_3_vectors (l, r, v);
      }
      END_EXCEPTION;
    }

  delete_2_vectors (l, r);
  return (ENT (v));
}

ENTITY *
append_vector_complex (VECTOR *l, VECTOR *r)
{
  VECTOR * volatile v;
  int i;

  EASSERT (l, vector, complex);
  EASSERT (r, vector, complex);

  switch (l->density)
    {
    case dense:
      switch (r->density)
	{
	case dense:
	  v = (VECTOR *) form_vector (l->ne + r->ne, complex, dense);
	  if (l->nn > 0)
	    memcpy (v->a.complex, l->a.complex,
		    l->ne * sizeof (COMPLEX));
	  if (r->nn > 0)
	    memcpy (v->a.complex + l->ne, r->a.complex,
		    r->ne * sizeof (COMPLEX));
	  break;
	case sparse:
	  return (append_vector_complex ((VECTOR *) sparse_vector (l), r));
	default:
	  BAD_DENSITY (r->density);
	  delete_2_vectors (l, r);
	  raise_exception ();
	}
      break;
    case sparse:
      switch (r->density)
	{
	case dense:
	  return (append_vector_complex (l, (VECTOR *) sparse_vector (r)));
	case sparse:
	  v = (VECTOR *) make_vector (l->ne + r->ne, complex, sparse);
	  v->nn = l->nn + r->nn;
	  if (v->nn > 0)
	    {
	      v->ja = (int *) MALLOC (v->nn * sizeof (int));
	      v->a.complex = (COMPLEX *) MALLOC (v->nn * sizeof (COMPLEX));
	      if (l->nn > 0)
		{
		  memcpy (v->ja, l->ja, l->nn * sizeof (int));
		  memcpy (v->a.complex, l->a.complex,
			  l->nn * sizeof (COMPLEX));
		}
	      for (i = 0; i < r->nn; i++)
		{
		  v->ja[i + l->nn] = r->ja[i] + l->ne;
		  v->a.complex[i + l->nn] = r->a.complex[i];
		}
	      v = (VECTOR *) apt_vector (v);
	    }
	  break;
	default:
	  BAD_DENSITY (r->density);
	  delete_2_vectors (l, r);
	  raise_exception ();
	}
      break;
    default:
      BAD_DENSITY (l->density);
      delete_2_vectors (l, r);
      raise_exception ();
    }

  if (l->eid != NULL || r->eid != NULL)
    {
      WITH_HANDLING
      {
	v->eid = binop_vector_vector (BO_APPEND,
				      (l->eid == NULL) ?
		   (VECTOR *) make_vector (l->ne, ((VECTOR *) r->eid)->type,
					   sparse) :
				 (VECTOR *) copy_vector ((VECTOR *) l->eid),
				      (r->eid == NULL) ?
		   (VECTOR *) make_vector (r->ne, ((VECTOR *) l->eid)->type,
					   sparse) :
				(VECTOR *) copy_vector ((VECTOR *) r->eid));
      }
      ON_EXCEPTION
      {
	delete_3_vectors (l, r, v);
      }
      END_EXCEPTION;
    }

  delete_2_vectors (l, r);
  return (ENT (v));
}

ENTITY *
append_vector_character (VECTOR *l, VECTOR *r)
{
  VECTOR * volatile v;
  int i;

  EASSERT (l, vector, character);
  EASSERT (r, vector, character);

  switch (l->density)
    {
    case dense:
      switch (r->density)
	{
	case dense:
	  v = (VECTOR *) form_vector (l->ne + r->ne, character, dense);
	  for (i = 0; i < l->ne; i++)
	    v->a.character[i] = dup_char (l->a.character[i]);
	  for (i = 0; i < r->ne; i++)
	    v->a.character[i + l->ne] = dup_char (r->a.character[i]);
	  break;
	case sparse:
	  return (append_vector_character ((VECTOR *) sparse_vector (l), r));
	default:
	  BAD_DENSITY (r->density);
	  delete_2_vectors (l, r);
	  raise_exception ();
	}
      break;
    case sparse:
      switch (r->density)
	{
	case dense:
	  return (append_vector_character (l, (VECTOR *) sparse_vector (r)));
	case sparse:
	  v = (VECTOR *) make_vector (l->ne + r->ne, character, sparse);
	  v->nn = l->nn + r->nn;
	  if (v->nn > 0)
	    {
	      v->ja = (int *) MALLOC (v->nn * sizeof (int));
	      v->a.character = (char **) MALLOC (v->nn * sizeof (char *));
	      if (l->nn > 0)
		{
		  memcpy (v->ja, l->ja, l->nn * sizeof (int));
		  for (i = 0; i < l->nn; i++)
		    v->a.character[i] = dup_char (l->a.character[i]);
		}
	      for (i = 0; i < r->nn; i++)
		{
		  v->ja[i + l->nn] = r->ja[i] + l->ne;
		  v->a.character[i + l->nn] = dup_char (r->a.character[i]);
		}
	      v = (VECTOR *) apt_vector (v);
	    }
	  break;
	default:
	  BAD_DENSITY (r->density);
	  delete_2_vectors (l, r);
	  raise_exception ();
	}
      break;
    default:
      BAD_DENSITY (l->density);
      delete_2_vectors (l, r);
      raise_exception ();
    }

  if (l->eid != NULL || r->eid != NULL)
    {
      WITH_HANDLING
      {
	v->eid = binop_vector_vector (BO_APPEND,
				      (l->eid == NULL) ?
		   (VECTOR *) make_vector (l->ne, ((VECTOR *) r->eid)->type,
					   sparse) :
				 (VECTOR *) copy_vector ((VECTOR *) l->eid),
				      (r->eid == NULL) ?
		   (VECTOR *) make_vector (r->ne, ((VECTOR *) l->eid)->type,
					   sparse) :
				(VECTOR *) copy_vector ((VECTOR *) r->eid));
      }
      ON_EXCEPTION
      {
	delete_3_vectors (l, r, v);
      }
      END_EXCEPTION;
    }

  delete_2_vectors (l, r);
  return (ENT (v));
}

ENTITY *
append_vector_scalar_integer (VECTOR *l, SCALAR *r)
{
  VECTOR *v;

  EASSERT (l, vector, integer);
  EASSERT (r, scalar, integer);

  switch (l->density)
    {
    case dense:
      if (l->entity.ref_count == 1)
	{
	  /*
	   * Here's a little shortcut.  We own the only copy of this
	   * vector, so we make a second copy of it but then modify
	   * both copies at once.  (The extra copy is deleted below.)
	   * If we were to raise an exception we'd leak memory, but
	   * that won't happen.
	   */
	  v = (VECTOR *) copy_vector (l);
	  v->nn = ++v->ne;
	  if (v->ne > 1)
	    v->a.integer = REALLOC (v->a.integer, v->ne * sizeof (int));
	  else
	    v->a.integer = MALLOC (sizeof (int));
	  v->a.integer[v->ne - 1] = r->v.integer;
	}
      else
	{
	  v = (VECTOR *) form_vector (l->ne + 1, integer, dense);
	  if (l->ne > 0)
	    memcpy (v->a.integer, l->a.integer, l->ne * sizeof (int));
	  v->a.integer[l->ne] = r->v.integer;
	}
      break;
    case sparse:
      v = (VECTOR *) make_vector (l->ne + 1, integer, sparse);
      v->nn = l->nn + 1;
      v->ja = (int *) MALLOC (v->nn * sizeof (int));
      if (l->nn > 0)
	memcpy (v->ja, l->ja, l->nn * sizeof (int));
      v->ja[v->nn - 1] = v->ne;
      v->a.integer = (int *) MALLOC (v->nn * sizeof (int));
      if (l->nn > 0)
	memcpy (v->a.integer, l->a.integer, l->nn * sizeof (int));
      v->a.integer[v->nn - 1] = r->v.integer;
      v = (VECTOR *) apt_vector (v);
      break;
    default:
      BAD_DENSITY (l->density);
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }

  if (l->eid != NULL)
    {
      v->eid = binop_vector_scalar (BO_APPEND,
				 (VECTOR *) copy_vector ((VECTOR *) l->eid),
			(SCALAR *) make_scalar (((VECTOR *) l->eid)->type));
    }

  delete_vector (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
append_vector_scalar_real (VECTOR *l, SCALAR *r)
{
  VECTOR *v;

  EASSERT (l, vector, real);
  EASSERT (r, scalar, real);

  switch (l->density)
    {
    case dense:
      if (l->entity.ref_count == 1)
	{
	  /*
	   * Here's a little shortcut.  We own the only copy of this
	   * vector, so we make a second copy of it but then modify
	   * both copies at once.  (The extra copy is deleted below.)
	   * If we were to raise an exception we'd leak memory, but
	   * that won't happen.
	   */
	  v = (VECTOR *) copy_vector (l);
	  v->nn = ++v->ne;
	  if (v->ne > 1)
	    v->a.real = REALLOC (v->a.real, v->ne * sizeof (REAL));
	  else
	    v->a.real = MALLOC (sizeof (REAL));
	  v->a.real[v->ne - 1] = r->v.real;
	}
      else
	{
	  v = (VECTOR *) form_vector (l->ne + 1, real, dense);
	  if (l->ne > 0)
	    memcpy (v->a.real, l->a.real, l->ne * sizeof (REAL));
	  v->a.real[l->ne] = r->v.real;
	}
      break;
    case sparse:
      v = (VECTOR *) make_vector (l->ne + 1, real, sparse);
      v->nn = l->nn + 1;
      v->ja = (int *) MALLOC (v->nn * sizeof (int));
      if (l->nn > 0)
	memcpy (v->ja, l->ja, l->nn * sizeof (int));
      v->ja[v->nn - 1] = v->ne;
      v->a.real = (REAL *) MALLOC (v->nn * sizeof (REAL));
      if (l->nn > 0)
	memcpy (v->a.real, l->a.real, l->nn * sizeof (REAL));
      v->a.real[v->nn - 1] = r->v.real;
      v = (VECTOR *) apt_vector (v);
      break;
    default:
      BAD_DENSITY (l->density);
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }

  if (l->eid != NULL)
    {
      v->eid = binop_vector_scalar (BO_APPEND,
				 (VECTOR *) copy_vector ((VECTOR *) l->eid),
			(SCALAR *) make_scalar (((VECTOR *) l->eid)->type));
    }

  delete_vector (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
append_vector_scalar_complex (VECTOR *l, SCALAR *r)
{
  VECTOR *v;

  EASSERT (l, vector, complex);
  EASSERT (r, scalar, complex);

  switch (l->density)
    {
    case dense:
      if (l->entity.ref_count == 1)
	{
	  /*
	   * Here's a little shortcut.  We own the only copy of this
	   * vector, so we make a second copy of it but then modify
	   * both copies at once.  (The extra copy is deleted below.)
	   * If we were to raise an exception we'd leak memory, but
	   * that won't happen.
	   */
	  v = (VECTOR *) copy_vector (l);
	  v->nn = ++v->ne;
	  if (v->ne > 1)
	    v->a.complex = REALLOC (v->a.complex, v->ne * sizeof (COMPLEX));
	  else
	    v->a.complex = MALLOC (sizeof (COMPLEX));
	  v->a.complex[v->ne - 1] = r->v.complex;
	}
      else
	{
	  v = (VECTOR *) form_vector (l->ne + 1, complex, dense);
	  if (l->ne > 0)
	    memcpy (v->a.complex, l->a.complex, l->ne * sizeof (COMPLEX));
	  v->a.complex[l->ne] = r->v.complex;
	}
      break;
    case sparse:
      v = (VECTOR *) make_vector (l->ne + 1, complex, sparse);
      v->nn = l->nn + 1;
      v->ja = (int *) MALLOC (v->nn * sizeof (int));
      if (l->nn > 0)
	memcpy (v->ja, l->ja, l->nn * sizeof (int));
      v->ja[v->nn - 1] = v->ne;
      v->a.complex = (COMPLEX *) MALLOC (v->nn * sizeof (COMPLEX));
      if (l->nn > 0)
	memcpy (v->a.complex, l->a.complex, l->nn * sizeof (COMPLEX));
      v->a.complex[v->nn - 1] = r->v.complex;
      v = (VECTOR *) apt_vector (v);
      break;
    default:
      BAD_DENSITY (l->density);
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }

  if (l->eid != NULL)
    {
      v->eid = binop_vector_scalar (BO_APPEND,
				 (VECTOR *) copy_vector ((VECTOR *) l->eid),
			(SCALAR *) make_scalar (((VECTOR *) l->eid)->type));
    }

  delete_vector (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
append_vector_scalar_character (VECTOR *l, SCALAR *r)
{
  VECTOR *v;
  int i;

  EASSERT (l, vector, character);
  EASSERT (r, scalar, character);

  switch (l->density)
    {
    case dense:
      if (l->entity.ref_count == 1)
	{
	  /*
	   * Here's a little shortcut.  We own the only copy of this
	   * vector, so we make a second copy of it but then modify
	   * both copies at once.  (The extra copy is deleted below.)
	   * If we were to raise an exception we'd leak memory, but
	   * that won't happen.
	   */
	  v = (VECTOR *) copy_vector (l);
	  v->nn = ++v->ne;
	  if (v->ne > 1)
	    v->a.character = REALLOC (v->a.character,
				      v->ne * sizeof (char *));
	  else
	    v->a.character = MALLOC (sizeof (char *));
	  v->a.character[v->ne - 1] = entity_to_string (EAT (r));
	}
      else
	{
	  v = (VECTOR *) form_vector (l->ne + 1, character, dense);
	  for (i = 0; i < l->ne; i++)
	    v->a.character[i] = dup_char (l->a.character[i]);
	  v->a.character[l->ne] = dup_char (r->v.character);
	}
      break;
    case sparse:
      v = (VECTOR *) make_vector (l->ne + 1, character, sparse);
      v->nn = l->nn + 1;
      v->ja = (int *) MALLOC (v->nn * sizeof (int));
      if (l->nn > 0)
	memcpy (v->ja, l->ja, l->nn * sizeof (int));
      v->ja[v->nn - 1] = v->ne;
      v->a.character = (char **) MALLOC (v->nn * sizeof (char *));
      for (i = 0; i < l->nn; i++)
	v->a.character[i] = dup_char (l->a.character[i]);
      v->a.character[v->nn - 1] = dup_char (r->v.character);
      v = (VECTOR *) apt_vector (v);
      break;
    default:
      BAD_DENSITY (l->density);
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }

  if (l->eid != NULL)
    {
      v->eid = binop_vector_scalar (BO_APPEND,
				 (VECTOR *) copy_vector ((VECTOR *) l->eid),
			(SCALAR *) make_scalar (((VECTOR *) l->eid)->type));
    }

  delete_vector (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
append_vector_matrix_integer (VECTOR *l, MATRIX *r)
{
  MATRIX * volatile v;
  int i;

  EASSERT (l, vector, integer);
  EASSERT (r, matrix, integer);

  if (r->nr != 1)
    {
      fail ("Inconsistent dimensions for append.  Matrix with %d row%s can't be appended to a vector.",
	    r->nr, PLURAL (r->nr));
      delete_2_entities (ENT (l), ENT (r));
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
	      v = (MATRIX *) form_matrix (r->nr, l->ne + r->nc,
					  integer, dense);
	      if (l->nn > 0)
		memcpy (v->a.integer, l->a.integer, l->nn * sizeof (int));
	      if (r->nn > 0)
		memcpy (v->a.integer + l->nn, r->a.integer,
			r->nn * sizeof (int));
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	    case sparse_upper:
	      return (append_vector_matrix_integer (
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
	      return (append_vector_matrix_integer (
					  l, (MATRIX *) sparse_matrix (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (1, l->ne + r->nc, integer, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.integer = (int *) MALLOC (v->nn * sizeof (int));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      memcpy (v->a.integer, l->a.integer,
			      l->nn * sizeof (int));
		    }
		  for (i = 0; i < r->nn; i++)
		    {
		      v->ja[i + l->nn] = r->ja[i] + l->ne;
		      v->a.integer[i + l->nn] = r->a.integer[i];
		    }
		  v->ia = (int *) MALLOC (2 * sizeof (int));
		  v->ia[0] = 1;
		  v->ia[1] = v->nn + 1;
		  v = (MATRIX *) apt_matrix (v);
		}
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

  if (r->rid != NULL)
    v->rid = copy_vector ((VECTOR *) r->rid);
  if (l->eid != NULL || r->cid != NULL)
    {
      WITH_HANDLING
      {
	v->cid = binop_vector_vector (BO_APPEND,
				      (l->eid == NULL) ?
		   (VECTOR *) make_vector (l->ne, ((VECTOR *) r->cid)->type,
					   sparse) :
				 (VECTOR *) copy_vector ((VECTOR *) l->eid),
				      (r->cid == NULL) ?
		   (VECTOR *) make_vector (r->nc, ((VECTOR *) l->eid)->type,
					   sparse) :
				(VECTOR *) copy_vector ((VECTOR *) r->cid));
      }
      ON_EXCEPTION
      {
	delete_vector (l);
	delete_2_matrices (r, v);
      }
      END_EXCEPTION;
    }

  delete_vector (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
append_vector_matrix_real (VECTOR *l, MATRIX *r)
{
  MATRIX * volatile v;
  int i;

  EASSERT (l, vector, real);
  EASSERT (r, matrix, real);

  if (r->nr != 1)
    {
      fail ("Inconsistent dimensions for append.  Matrix with %d row%s can't be appended to a vector.",
	    r->nr, PLURAL (r->nr));
      delete_2_entities (ENT (l), ENT (r));
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
	      v = (MATRIX *) form_matrix (r->nr, l->ne + r->nc, real, dense);
	      if (l->nn > 0)
		memcpy (v->a.real, l->a.real, l->nn * sizeof (REAL));
	      if (r->nn > 0)
		memcpy (v->a.real + l->nn, r->a.real, r->nn * sizeof (REAL));
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	    case sparse_upper:
	      return (append_vector_matrix_real (
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
	      return (append_vector_matrix_real (
					  l, (MATRIX *) sparse_matrix (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (1, l->ne + r->nc, real, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.real = (REAL *) MALLOC (v->nn * sizeof (REAL));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      memcpy (v->a.real, l->a.real, l->nn * sizeof (REAL));
		    }
		  for (i = 0; i < r->nn; i++)
		    {
		      v->ja[i + l->nn] = r->ja[i] + l->ne;
		      v->a.real[i + l->nn] = r->a.real[i];
		    }
		  v->ia = (int *) MALLOC (2 * sizeof (int));
		  v->ia[0] = 1;
		  v->ia[1] = v->nn + 1;
		  v = (MATRIX *) apt_matrix (v);
		}
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

  if (r->rid != NULL)
    v->rid = copy_vector ((VECTOR *) r->rid);
  if (l->eid != NULL || r->cid != NULL)
    {
      WITH_HANDLING
      {
	v->cid = binop_vector_vector (BO_APPEND,
				      (l->eid == NULL) ?
		   (VECTOR *) make_vector (l->ne, ((VECTOR *) r->cid)->type,
					   sparse) :
				 (VECTOR *) copy_vector ((VECTOR *) l->eid),
				      (r->cid == NULL) ?
		   (VECTOR *) make_vector (r->nc, ((VECTOR *) l->eid)->type,
					   sparse) :
				(VECTOR *) copy_vector ((VECTOR *) r->cid));
      }
      ON_EXCEPTION
      {
	delete_vector (l);
	delete_2_matrices (r, v);
      }
      END_EXCEPTION;
    }

  delete_vector (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
append_vector_matrix_complex (VECTOR *l, MATRIX *r)
{
  MATRIX * volatile v;
  int i;

  EASSERT (l, vector, complex);
  EASSERT (r, matrix, complex);

  if (r->nr != 1)
    {
      fail ("Inconsistent dimensions for append.  Matrix with %d row%s can't be appended to a vector.",
	    r->nr, PLURAL (r->nr));
      delete_2_entities (ENT (l), ENT (r));
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
	      v = (MATRIX *) form_matrix (r->nr, l->ne + r->nc,
					  complex, dense);
	      if (l->nn > 0)
		memcpy (v->a.complex, l->a.complex,
			l->nn * sizeof (COMPLEX));
	      if (r->nn > 0)
		memcpy (v->a.complex + l->nn, r->a.complex,
			r->nn * sizeof (COMPLEX));
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	    case sparse_upper:
	      return (append_vector_matrix_complex (
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
	      return (append_vector_matrix_complex (
					  l, (MATRIX *) sparse_matrix (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (1, l->ne + r->nc, complex, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.complex =
		    (COMPLEX *) MALLOC (v->nn * sizeof (COMPLEX));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      memcpy (v->a.complex, l->a.complex,
			      l->nn * sizeof (COMPLEX));
		    }
		  for (i = 0; i < r->nn; i++)
		    {
		      v->ja[i + l->nn] = r->ja[i] + l->ne;
		      v->a.complex[i + l->nn] = r->a.complex[i];
		    }
		  v->ia = (int *) MALLOC (2 * sizeof (int));
		  v->ia[0] = 1;
		  v->ia[1] = v->nn + 1;
		  v = (MATRIX *) apt_matrix (v);
		}
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

  if (r->rid != NULL)
    v->rid = copy_vector ((VECTOR *) r->rid);
  if (l->eid != NULL || r->cid != NULL)
    {
      WITH_HANDLING
      {
	v->cid = binop_vector_vector (BO_APPEND,
				      (l->eid == NULL) ?
		   (VECTOR *) make_vector (l->ne, ((VECTOR *) r->cid)->type,
					   sparse) :
				 (VECTOR *) copy_vector ((VECTOR *) l->eid),
				      (r->cid == NULL) ?
		   (VECTOR *) make_vector (r->nc, ((VECTOR *) l->eid)->type,
					   sparse) :
				(VECTOR *) copy_vector ((VECTOR *) r->cid));
      }
      ON_EXCEPTION
      {
	delete_vector (l);
	delete_2_matrices (r, v);
      }
      END_EXCEPTION;
    }

  delete_vector (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
append_vector_matrix_character (VECTOR *l, MATRIX *r)
{
  MATRIX * volatile v;
  int i;

  EASSERT (l, vector, character);
  EASSERT (r, matrix, character);

  if (r->nr != 1)
    {
      fail ("Inconsistent dimensions for append.  Matrix with %d row%s can't be appended to a vector.",
	    r->nr, PLURAL (r->nr));
      delete_2_entities (ENT (l), ENT (r));
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
	      v = (MATRIX *) form_matrix (r->nr, l->ne + r->nc,
					  character, dense);
	      for (i = 0; i < l->nn; i++)
		v->a.character[i] = dup_char (l->a.character[i]);
	      for (i = 0; i < r->nn; i++)
		v->a.character[i + l->nn] = dup_char (r->a.character[i]);
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	    case sparse_upper:
	      return (append_vector_matrix_character (
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
	      return (append_vector_matrix_character (
					  l, (MATRIX *) sparse_matrix (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (1, l->ne + r->nc,
					  character, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.character =
		    (char **) MALLOC (v->nn * sizeof (char *));
		  if (l->nn > 0)
		    {
		      memcpy (v->ja, l->ja, l->nn * sizeof (int));
		      for (i = 0; i < l->nn; i++)
			v->a.character[i] = dup_char (l->a.character[i]);
		    }
		  for (i = 0; i < r->nn; i++)
		    {
		      v->ja[i + l->nn] = r->ja[i] + l->ne;
		      v->a.character[i + l->nn] =
			dup_char (r->a.character[i]);
		    }
		  v->ia = (int *) MALLOC (2 * sizeof (int));
		  v->ia[0] = 1;
		  v->ia[1] = v->nn + 1;
		  v = (MATRIX *) apt_matrix (v);
		}
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

  if (r->rid != NULL)
    v->rid = copy_vector ((VECTOR *) r->rid);
  if (l->eid != NULL || r->cid != NULL)
    {
      WITH_HANDLING
      {
	v->cid = binop_vector_vector (BO_APPEND,
				      (l->eid == NULL) ?
		   (VECTOR *) make_vector (l->ne, ((VECTOR *) r->cid)->type,
					   sparse) :
				 (VECTOR *) copy_vector ((VECTOR *) l->eid),
				      (r->cid == NULL) ?
		   (VECTOR *) make_vector (r->nc, ((VECTOR *) l->eid)->type,
					   sparse) :
				(VECTOR *) copy_vector ((VECTOR *) r->cid));
      }
      ON_EXCEPTION
      {
	delete_vector (l);
	delete_2_matrices (r, v);
      }
      END_EXCEPTION;
    }

  delete_vector (l);
  delete_matrix (r);
  return (ENT (v));
}

ENTITY *
append_matrix_integer (MATRIX *l, MATRIX *r)
{
  MATRIX * volatile v;
  int i, j, k;

  EASSERT (l, matrix, integer);
  EASSERT (r, matrix, integer);

  if (l->nr != r->nr)
    {
      fail ("Inconsistent dimensions for append.  Matrix with %d row%s can't be appended to a matrix with %d row%s.",
	    l->nr, PLURAL (l->nr), r->nr, PLURAL (r->nr));
      delete_2_matrices (l, r);
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->rid, r->rid))
    {
      fail (bad_append_labels);
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
	      v = (MATRIX *) form_matrix (l->nr, l->nc + r->nc,
					  integer, dense);
	      v->symmetry = general;
	      if (l->nn > 0)
		memcpy (v->a.integer, l->a.integer, l->nn * sizeof (int));
	      if (r->nn > 0)
		memcpy (v->a.integer + l->nn, r->a.integer,
			r->nn * sizeof (int));
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	    case sparse_upper:
	      return (append_matrix_integer (
					  (MATRIX *) sparse_matrix (l), r));
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
	      return (append_matrix_integer (
					  l, (MATRIX *) sparse_matrix (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (l->nr, l->nc + r->nc,
					  integer, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		    v->symmetry = general;
		  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.integer = (int *) MALLOC (v->nn * sizeof (int));
		  v->ia[0] = 1;
		  for (i = k = 0; i < v->nr; i++)
		    {
		      if (l->nn > 0)
			{
			  for (j = l->ia[i] - 1; j < l->ia[i + 1] - 1; j++)
			    {
			      v->ja[k] = l->ja[j];
			      v->a.integer[k++] = l->a.integer[j];
			    }
			}
		      if (r->nn > 0)
			{
			  for (j = r->ia[i] - 1; j < r->ia[i + 1] - 1; j++)
			    {
			      v->ja[k] = r->ja[j] + l->nc;
			      v->a.integer[k++] = r->a.integer[j];
			    }
			}
		      v->ia[i + 1] = k + 1;
		    }
		  v = (MATRIX *) apt_matrix (v);
		}
	      break;
	    case sparse_upper:
	      return (append_matrix_integer (
					    l, (MATRIX *) full_matrix (r)));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse_upper:
	  return (append_matrix_integer ((MATRIX *) full_matrix (l), r));
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid)
    v->rid = copy_vector ((VECTOR *) l->rid);
  else if (r->rid)
    v->rid = copy_vector ((VECTOR *) r->rid);

  if (l->cid != NULL || r->cid != NULL)
    {
      WITH_HANDLING
      {
	v->cid = binop_vector_vector (BO_APPEND,
				      (l->cid == NULL) ?
		   (VECTOR *) make_vector (l->nc, ((VECTOR *) r->cid)->type,
					   sparse) :
				 (VECTOR *) copy_vector ((VECTOR *) l->cid),
				      (r->cid == NULL) ?
		   (VECTOR *) make_vector (r->nc, ((VECTOR *) l->cid)->type,
					   sparse) :
				(VECTOR *) copy_vector ((VECTOR *) r->cid));
      }
      ON_EXCEPTION
      {
	delete_3_matrices (l, r, v);
      }
      END_EXCEPTION;
    }
  delete_2_matrices (l, r);
  return (ENT (v));
}

ENTITY *
append_matrix_real (MATRIX *l, MATRIX *r)
{
  MATRIX * volatile v;
  int i, j, k;

  EASSERT (l, matrix, real);
  EASSERT (r, matrix, real);

  if (l->nr != r->nr)
    {
      fail ("Inconsistent dimensions for append.  Matrix with %d row%s can't be appended to a matrix with %d row%s.",
	    l->nr, PLURAL (l->nr), r->nr, PLURAL (r->nr));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->rid, r->rid))
    {
      fail (bad_append_labels);
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
	      v = (MATRIX *) form_matrix (l->nr, l->nc + r->nc, real, dense);
	      if (l->nn > 0)
		memcpy (v->a.real, l->a.real, l->nn * sizeof (REAL));
	      if (r->nn > 0)
		memcpy (v->a.real + l->nn, r->a.real, r->nn * sizeof (REAL));
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	    case sparse_upper:
	      return (append_matrix_real (
					   (MATRIX *) sparse_matrix (l), r));
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
	      return (append_matrix_real (l, (MATRIX *) sparse_matrix (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (l->nr, l->nc + r->nc, real, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		    v->symmetry = general;
		  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.real = (REAL *) MALLOC (v->nn * sizeof (REAL));
		  v->ia[0] = 1;
		  for (i = k = 0; i < v->nr; i++)
		    {
		      if (l->nn > 0)
			{
			  for (j = l->ia[i] - 1; j < l->ia[i + 1] - 1; j++)
			    {
			      v->ja[k] = l->ja[j];
			      v->a.real[k++] = l->a.real[j];
			    }
			}
		      if (r->nn > 0)
			{
			  for (j = r->ia[i] - 1; j < r->ia[i + 1] - 1; j++)
			    {
			      v->ja[k] = r->ja[j] + l->nc;
			      v->a.real[k++] = r->a.real[j];
			    }
			}
		      v->ia[i + 1] = k + 1;
		    }
		  v = (MATRIX *) apt_matrix (v);
		}
	      break;
	    case sparse_upper:
	      return (append_matrix_real (l, (MATRIX *) full_matrix (r)));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse_upper:
	  return (append_matrix_real ((MATRIX *) full_matrix (l), r));
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid)
    v->rid = copy_vector ((VECTOR *) l->rid);
  else if (r->rid)
    v->rid = copy_vector ((VECTOR *) r->rid);

  if (l->cid != NULL || r->cid != NULL)
    {
      WITH_HANDLING
      {
	v->cid = binop_vector_vector (BO_APPEND,
				      (l->cid == NULL) ?
		   (VECTOR *) make_vector (l->nc, ((VECTOR *) r->cid)->type,
					   sparse) :
				 (VECTOR *) copy_vector ((VECTOR *) l->cid),
				      (r->cid == NULL) ?
		   (VECTOR *) make_vector (r->nc, ((VECTOR *) l->cid)->type,
					   sparse) :
				(VECTOR *) copy_vector ((VECTOR *) r->cid));
      }
      ON_EXCEPTION
      {
	delete_3_matrices (l, r, v);
      }
      END_EXCEPTION;
    }

  delete_2_matrices (l, r);
  return (ENT (v));
}

ENTITY *
append_matrix_complex (MATRIX *l, MATRIX *r)
{
  MATRIX * volatile v;
  int i, j, k;

  EASSERT (l, matrix, complex);
  EASSERT (r, matrix, complex);

  if (l->nr != r->nr)
    {
      fail ("Inconsistent dimensions for append.  Matrix with %d row%s can't be appended to a matrix with %d row%s.",
	    l->nr, PLURAL (l->nr), r->nr, PLURAL (r->nr));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->rid, r->rid))
    {
      fail (bad_append_labels);
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
	      v = (MATRIX *) form_matrix (l->nr, l->nc + r->nc,
					  complex, dense);
	      if (l->nn > 0)
		memcpy (v->a.complex, l->a.complex,
			l->nn * sizeof (COMPLEX));
	      if (r->nn > 0)
		memcpy (v->a.complex + l->nn, r->a.complex,
			r->nn * sizeof (COMPLEX));
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	    case sparse_upper:
	      return (append_matrix_complex (
					  (MATRIX *) sparse_matrix (l), r));
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
	      return (append_matrix_complex (
					  l, (MATRIX *) sparse_matrix (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (l->nr, l->nc + r->nc, complex,
					  sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		    v->symmetry = general;
		  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.complex =
		    (COMPLEX *) MALLOC (v->nn * sizeof (COMPLEX));
		  v->ia[0] = 1;
		  for (i = k = 0; i < v->nr; i++)
		    {
		      if (l->nn > 0)
			{
			  for (j = l->ia[i] - 1; j < l->ia[i + 1] - 1; j++)
			    {
			      v->ja[k] = l->ja[j];
			      v->a.complex[k++] = l->a.complex[j];
			    }
			}
		      if (r->nn > 0)
			{
			  for (j = r->ia[i] - 1; j < r->ia[i + 1] - 1; j++)
			    {
			      v->ja[k] = r->ja[j] + l->nc;
			      v->a.complex[k++] = r->a.complex[j];
			    }
			}
		      v->ia[i + 1] = k + 1;
		    }
		  v = (MATRIX *) apt_matrix (v);
		}
	      break;
	    case sparse_upper:
	      return (append_matrix_complex (
					    l, (MATRIX *) full_matrix (r)));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse_upper:
	  return (append_matrix_complex ((MATRIX *) full_matrix (l), r));
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid)
    v->rid = copy_vector ((VECTOR *) l->rid);
  else if (r->rid)
    v->rid = copy_vector ((VECTOR *) r->rid);

  if (l->cid != NULL || r->cid != NULL)
    {
      WITH_HANDLING
      {
	v->cid = binop_vector_vector (BO_APPEND,
				      (l->cid == NULL) ?
		   (VECTOR *) make_vector (l->nc, ((VECTOR *) r->cid)->type,
					   sparse) :
				 (VECTOR *) copy_vector ((VECTOR *) l->cid),
				      (r->cid == NULL) ?
		   (VECTOR *) make_vector (r->nc, ((VECTOR *) l->cid)->type,
					   sparse) :
				(VECTOR *) copy_vector ((VECTOR *) r->cid));
      }
      ON_EXCEPTION
      {
	delete_3_matrices (l, r, v);
      }
      END_EXCEPTION;
    }

  delete_2_matrices (l, r);
  return (ENT (v));
}

ENTITY *
append_matrix_character (MATRIX *l, MATRIX *r)
{
  MATRIX * volatile v;
  int i, j, k;

  EASSERT (l, matrix, character);
  EASSERT (r, matrix, character);

  if (l->nr != r->nr)
    {
      fail ("Inconsistent dimensions for append.  Matrix with %d row%s can't be appended to a matrix with %d row%s.",
	    l->nr, PLURAL (l->nr), r->nr, PLURAL (r->nr));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->rid, r->rid))
    {
      fail (bad_append_labels);
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
	      v = (MATRIX *) form_matrix (l->nr, l->nc + r->nc,
					  character, dense);
	      for (i = 0; i < l->nn; i++)
		v->a.character[i] = dup_char (l->a.character[i]);
	      for (i = 0; i < r->nn; i++)
		v->a.character[i + l->nn] = dup_char (r->a.character[i]);
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	    case sparse_upper:
	      return (append_matrix_character (
					  (MATRIX *) sparse_matrix (l), r));
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
	      return (append_matrix_character (
					  l, (MATRIX *) sparse_matrix (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (l->nr, l->nc + r->nc,
					  character, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		    v->symmetry = general;
		  v->ia = (int *) MALLOC ((v->nr + 1) * sizeof (int));
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.character =
		    (char **) MALLOC (v->nn * sizeof (char *));
		  v->ia[0] = 1;
		  for (i = k = 0; i < v->nr; i++)
		    {
		      if (l->nn > 0)
			{
			  for (j = l->ia[i] - 1; j < l->ia[i + 1] - 1; j++)
			    {
			      v->ja[k] = l->ja[j];
			      v->a.character[k++] =
				dup_char (l->a.character[j]);
			    }
			}
		      if (r->nn > 0)
			{
			  for (j = r->ia[i] - 1; j < r->ia[i + 1] - 1; j++)
			    {
			      v->ja[k] = r->ja[j] + l->nc;
			      v->a.character[k++] =
				dup_char (r->a.character[j]);
			    }
			}
		      v->ia[i + 1] = k + 1;
		    }
		  v = (MATRIX *) apt_matrix (v);
		}
	      break;
	    case sparse_upper:
	      return (append_matrix_character (
					    l, (MATRIX *) full_matrix (r)));
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse_upper:
	  return (append_matrix_character ((MATRIX *) full_matrix (l), r));
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid)
    v->rid = copy_vector ((VECTOR *) l->rid);
  else if (r->rid)
    v->rid = copy_vector ((VECTOR *) r->rid);

  if (l->cid != NULL || r->cid != NULL)
    {
      WITH_HANDLING
      {
	v->cid = binop_vector_vector (BO_APPEND,
				      (l->cid == NULL) ?
		   (VECTOR *) make_vector (l->nc, ((VECTOR *) r->cid)->type,
					   sparse) :
				 (VECTOR *) copy_vector ((VECTOR *) l->cid),
				      (r->cid == NULL) ?
		   (VECTOR *) make_vector (r->nc, ((VECTOR *) l->cid)->type,
					   sparse) :
				(VECTOR *) copy_vector ((VECTOR *) r->cid));
      }
      ON_EXCEPTION
      {
	delete_3_matrices (l, r, v);
      }
      END_EXCEPTION;
    }

  delete_2_matrices (l, r);
  return (ENT (v));
}

ENTITY *
append_matrix_scalar_integer (MATRIX *l, SCALAR *r)
{
  MATRIX *v;

  EASSERT (l, matrix, integer);
  EASSERT (r, scalar, integer);

  if (l->nr != 1)
    {
      fail ("Inconsistent dimensions for append.  Scalar can't be appended to a matrix with %d row%s.",
	    l->nr, PLURAL (l->nr));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  v = (MATRIX *) form_matrix (l->nr, l->nc + 1, integer, dense);
	  if (l->nc > 0)
	    {
	      memcpy (v->a.integer, l->a.integer, l->nn * sizeof (int));
	      v->symmetry = general;
	    }
	  v->a.integer[l->nn] = r->v.integer;
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (l->nr, l->nc + 1, integer, sparse);
	  v->nn = l->nn + 1;
	  v->ja = (int *) MALLOC (v->nn * sizeof (int));
	  v->ja[v->nn - 1] = v->nc;
	  if (l->nn > 0)
	    memcpy (v->ja, l->ja, l->nn * sizeof (int));
	  v->a.integer = (int *) MALLOC (v->nn * sizeof (int));
	  v->a.integer[v->nn - 1] = r->v.integer;
	  if (l->nn > 0)
	    memcpy (v->a.integer, l->a.integer, l->nn * sizeof (int));
	  v->ia = (int *) MALLOC (2 * sizeof (int));
	  v->ia[0] = 1;
	  v->ia[1] = v->nn + 1;
	  v = (MATRIX *) apt_matrix (v);
	  break;
	case sparse_upper:
	  return (append_matrix_scalar_integer (
					   (MATRIX *) dense_matrix (l), r));
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL)
    v->rid = copy_vector ((VECTOR *) l->rid);
  if (l->cid != NULL)
    {
      v->cid = binop_vector_scalar (BO_APPEND,
				 (VECTOR *) copy_vector ((VECTOR *) l->cid),
			(SCALAR *) make_scalar (((VECTOR *) l->cid)->type));
    }

  delete_matrix (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
append_matrix_scalar_real (MATRIX *l, SCALAR *r)
{
  MATRIX *v;

  EASSERT (l, matrix, real);
  EASSERT (r, scalar, real);

  if (l->nr != 1)
    {
      fail ("Inconsistent dimensions for append.  Scalar can't be appended to a matrix with %d row%s.",
	    l->nr, PLURAL (l->nr));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  v = (MATRIX *) form_matrix (l->nr, l->nc + 1, real, dense);
	  if (l->nc > 0)
	    {
	      memcpy (v->a.real, l->a.real, l->nn * sizeof (REAL));
	      v->symmetry = general;
	    }
	  v->a.real[l->nn] = r->v.real;
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (l->nr, l->nc + 1, real, sparse);
	  v->nn = l->nn + 1;
	  v->ja = (int *) MALLOC (v->nn * sizeof (int));
	  v->ja[v->nn - 1] = v->nc;
	  if (l->nn > 0)
	    memcpy (v->ja, l->ja, l->nn * sizeof (int));
	  v->a.real = (REAL *) MALLOC (v->nn * sizeof (REAL));
	  v->a.real[v->nn - 1] = r->v.real;
	  if (l->nn > 0)
	    memcpy (v->a.real, l->a.real, l->nn * sizeof (REAL));
	  v->ia = (int *) MALLOC (2 * sizeof (int));
	  v->ia[0] = 1;
	  v->ia[1] = v->nn + 1;
	  v = (MATRIX *) apt_matrix (v);
	  break;
	case sparse_upper:
	  return (append_matrix_scalar_real (
					   (MATRIX *) dense_matrix (l), r));
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL)
    v->rid = copy_vector ((VECTOR *) l->rid);
  if (l->cid != NULL)
    {
      v->cid = binop_vector_scalar (BO_APPEND,
				 (VECTOR *) copy_vector ((VECTOR *) l->cid),
			(SCALAR *) make_scalar (((VECTOR *) l->cid)->type));
    }

  delete_matrix (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
append_matrix_scalar_complex (MATRIX *l, SCALAR *r)
{
  MATRIX *v;

  EASSERT (l, matrix, complex);
  EASSERT (r, scalar, complex);

  if (l->nr != 1)
    {
      fail ("Inconsistent dimensions for append.  Scalar can't be appended to a matrix with %d row%s.",
	    l->nr, PLURAL (l->nr));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  v = (MATRIX *) form_matrix (l->nr, l->nc + 1, complex, dense);
	  if (l->nc > 0)
	    {
	      memcpy (v->a.complex, l->a.complex,
		      l->nn * sizeof (COMPLEX));
	      v->symmetry = general;
	    }
	  v->a.complex[l->nn] = r->v.complex;
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (l->nr, l->nc + 1, complex, sparse);
	  v->nn = l->nn + 1;
	  v->ja = (int *) MALLOC (v->nn * sizeof (int));
	  v->ja[v->nn - 1] = v->nc;
	  if (l->nn > 0)
	    memcpy (v->ja, l->ja, l->nn * sizeof (int));
	  v->a.complex = (COMPLEX *) MALLOC (v->nn * sizeof (COMPLEX));
	  v->a.complex[v->nn - 1] = r->v.complex;
	  if (l->nn > 0)
	    memcpy (v->a.complex, l->a.complex, l->nn * sizeof (COMPLEX));
	  v->ia = (int *) MALLOC (2 * sizeof (int));
	  v->ia[0] = 1;
	  v->ia[1] = v->nn + 1;
	  v = (MATRIX *) apt_matrix (v);
	  break;
	case sparse_upper:
	  return (append_matrix_scalar_complex (
					   (MATRIX *) dense_matrix (l), r));
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL)
    v->rid = copy_vector ((VECTOR *) l->rid);
  if (l->cid != NULL)
    {
      v->cid = binop_vector_scalar (BO_APPEND,
				 (VECTOR *) copy_vector ((VECTOR *) l->cid),
			(SCALAR *) make_scalar (((VECTOR *) l->cid)->type));
    }

  delete_matrix (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
append_matrix_scalar_character (MATRIX *l, SCALAR *r)
{
  MATRIX *v;
  int i;

  EASSERT (l, matrix, character);
  EASSERT (r, scalar, character);

  if (l->nr != 1)
    {
      fail ("Inconsistent dimensions for append.  Scalar can't be appended to a matrix with %d row%s.",
	    l->nr, PLURAL (l->nr));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else
    {
      switch (l->density)
	{
	case dense:
	  v = (MATRIX *) form_matrix (l->nr, l->nc + 1, character, dense);
	  if (l->nc > 0)
	    {
	      for (i = 0; i < l->nn; i++)
		v->a.character[i] = dup_char (l->a.character[i]);
	      v->symmetry = general;
	    }
	  v->a.character[l->nn] = dup_char (r->v.character);
	  break;
	case sparse:
	  v = (MATRIX *) make_matrix (l->nr, l->nc + 1, character, sparse);
	  v->nn = l->nn + 1;
	  v->ja = (int *) MALLOC (v->nn * sizeof (int));
	  v->ja[v->nn - 1] = v->nc;
	  if (l->nn > 0)
	    memcpy (v->ja, l->ja, l->nn * sizeof (int));
	  v->a.character = (char **) MALLOC (v->nn * sizeof (char *));
	  v->a.character[v->nn - 1] = dup_char (r->v.character);
	  for (i = 0; i < l->nn; i++)
	    v->a.character[i] = dup_char (l->a.character[i]);
	  v->ia = (int *) MALLOC (2 * sizeof (int));
	  v->ia[0] = 1;
	  v->ia[1] = v->nn + 1;
	  v = (MATRIX *) apt_matrix (v);
	  break;
	case sparse_upper:
	  return (append_matrix_scalar_character (
					   (MATRIX *) dense_matrix (l), r));
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL)
    v->rid = copy_vector ((VECTOR *) l->rid);
  if (l->cid != NULL)
    {
      v->cid = binop_vector_scalar (BO_APPEND,
				 (VECTOR *) copy_vector ((VECTOR *) l->cid),
			(SCALAR *) make_scalar (((VECTOR *) l->cid)->type));
    }

  delete_matrix (l);
  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
append_matrix_vector_integer (MATRIX *l, VECTOR *r)
{
  MATRIX *v;
  int i, k;

  EASSERT (l, matrix, integer);
  EASSERT (r, vector, integer);

  if (l->nr != r->ne)
    {
      fail ("Inconsistent dimensions for append.  Vector with %d element%s can't be appended to a matrix with %d row%s.",
	    r->ne, PLURAL (r->ne), l->nr, PLURAL (l->nr));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->rid, r->eid))
    {
      fail (bad_append_labels);
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
	      v = (MATRIX *) form_matrix (l->nr, l->nc + 1, integer, dense);
	      if (l->nn > 0)
		memcpy (v->a.integer, l->a.integer, l->nn * sizeof (int));
	      if (r->nn > 0)
		memcpy (v->a.integer + l->nn, r->a.integer,
			r->nn * sizeof (int));
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	      return (append_matrix_vector_integer (
					   l, (VECTOR *) dense_vector (r)));
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
	      return (append_matrix_vector_integer (
					  l, (VECTOR *) sparse_vector (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (l->nr, l->nc + 1,
					  integer, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		    v->symmetry = general;
		  v->ia = (int *) CALLOC (v->nr + 1, sizeof (int));
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.integer = (int *) MALLOC (v->nn * sizeof (int));

		  /* Add up the number of nonzeros in each row. */

		  if (l->nn > 0)
		    for (i = 0; i < v->nr; i++)
		      v->ia[i + 1] = l->ia[i + 1] - l->ia[i];
		  for (i = 0; i < r->nn; i++)
		    if (r->ja[i] - 1 == i)
		      v->ia[i + 1]++;

		  /* Now set up `ia' */

		  v->ia[0] = 1;
		  for (i = 0; i < v->nr; i++)
		    v->ia[i + 1] += v->ia[i];

		  /* Make the `ja' and `a' assignments. */

		  if (l->nn > 0)
		    {
		      for (i = 0; i < v->nr; i++)
			{
			  if ((k = l->ia[i + 1] - l->ia[i]) > 0)
			    {
			      memcpy (v->ja + v->ia[i] - 1, l->ja + l->ia[i] - 1, k * sizeof (int));
			      memcpy (v->a.integer + v->ia[i] - 1, l->a.integer + l->ia[i] - 1, k * sizeof (int));
			    }
			}
		    }
		  for (i = 0; i < r->nn; i++)
		    {
		      v->ja[v->ia[r->ja[i]] - 2] = v->nc;
		      v->a.integer[v->ia[r->ja[i]] - 2] = r->a.integer[i];
		    }
		  v = (MATRIX *) apt_matrix (v);
		}
	      break;
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse_upper:
	  return (append_matrix_vector_integer (
					    (MATRIX *) full_matrix (l), r));
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL)
    v->rid = copy_vector ((VECTOR *) l->rid);
  if (l->cid != NULL)
    {
      v->cid = binop_vector_scalar (BO_APPEND,
				 (VECTOR *) copy_vector ((VECTOR *) l->cid),
			(SCALAR *) make_scalar (((VECTOR *) l->cid)->type));
    }

  delete_matrix (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
append_matrix_vector_real (MATRIX *l, VECTOR *r)
{
  MATRIX *v;
  int i, k;

  EASSERT (l, matrix, real);
  EASSERT (r, vector, real);

  if (l->nr != r->ne)
    {
      fail ("Inconsistent dimensions for append.  Vector with %d element%s can't be appended to a matrix with %d row%s.",
	    r->ne, PLURAL (r->ne), l->nr, PLURAL (l->nr));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->rid, r->eid))
    {
      fail (bad_append_labels);
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
	      v = (MATRIX *) form_matrix (l->nr, l->nc + 1, real, dense);
	      if (l->nn > 0)
		memcpy (v->a.real, l->a.real, l->nn * sizeof (REAL));
	      if (r->nn > 0)
		memcpy (v->a.real + l->nn, r->a.real, r->nn * sizeof (REAL));
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	      return (append_matrix_vector_real (
					   l, (VECTOR *) dense_vector (r)));
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
	      return (append_matrix_vector_real (
					  l, (VECTOR *) sparse_vector (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (l->nr, l->nc + 1, real, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		    v->symmetry = general;
		  v->ia = (int *) CALLOC (v->nr + 1, sizeof (int));
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.real = (REAL *) MALLOC (v->nn * sizeof (REAL));

		  /* Add up the number of nonzeros in each row. */

		  if (l->nn > 0)
		    for (i = 0; i < v->nr; i++)
		      v->ia[i + 1] = l->ia[i + 1] - l->ia[i];
		  for (i = 0; i < r->nn; i++)
		    if (r->ja[i] - 1 == i)
		      v->ia[i + 1]++;

		  /* Now set up `ia' */

		  v->ia[0] = 1;
		  for (i = 0; i < v->nr; i++)
		    v->ia[i + 1] += v->ia[i];

		  /* Make the `ja' and `a' assignments. */

		  if (l->nn > 0)
		    {
		      for (i = 0; i < v->nr; i++)
			{
			  if ((k = l->ia[i + 1] - l->ia[i]) > 0)
			    {
			      memcpy (v->ja + v->ia[i] - 1, l->ja + l->ia[i] - 1,
				      k * sizeof (int));
			      memcpy (v->a.real + v->ia[i] - 1,
				      l->a.real + l->ia[i] - 1,
				      k * sizeof (REAL));
			    }
			}
		    }
		  for (i = 0; i < r->nn; i++)
		    {
		      v->ja[v->ia[r->ja[i]] - 2] = v->nc;
		      v->a.real[v->ia[r->ja[i]] - 2] = r->a.real[i];
		    }
		  v = (MATRIX *) apt_matrix (v);
		}
	      break;
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse_upper:
	  return (append_matrix_vector_real (
					    (MATRIX *) full_matrix (l), r));
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL)
    v->rid = copy_vector ((VECTOR *) l->rid);
  if (l->cid != NULL)
    {
      v->cid = binop_vector_scalar (BO_APPEND,
				 (VECTOR *) copy_vector ((VECTOR *) l->cid),
			(SCALAR *) make_scalar (((VECTOR *) l->cid)->type));
    }

  delete_matrix (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
append_matrix_vector_complex (MATRIX *l, VECTOR *r)
{
  MATRIX *v;
  int i, k;

  EASSERT (l, matrix, complex);
  EASSERT (r, vector, complex);

  if (l->nr != r->ne)
    {
      fail ("Inconsistent dimensions for append.  Vector with %d element%s can't be appended to a matrix with %d row%s.",
	    r->ne, PLURAL (r->ne), l->nr, PLURAL (l->nr));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->rid, r->eid))
    {
      fail (bad_append_labels);
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
	      v = (MATRIX *) form_matrix (l->nr, l->nc + 1, complex, dense);
	      if (l->nn > 0)
		memcpy (v->a.complex, l->a.complex,
			l->nn * sizeof (COMPLEX));
	      if (r->nn > 0)
		memcpy (v->a.complex + l->nn, r->a.complex,
			r->nn * sizeof (COMPLEX));
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	      return (append_matrix_vector_complex (
					   l, (VECTOR *) dense_vector (r)));
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
	      return (append_matrix_vector_complex (
					  l, (VECTOR *) sparse_vector (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (l->nr, l->nc + 1,
					  complex, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		    v->symmetry = general;
		  v->ia = (int *) CALLOC (v->nr + 1, sizeof (int));
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.complex =
		    (COMPLEX *) MALLOC (v->nn * sizeof (COMPLEX));

		  /* Add up the number of nonzeros in each row. */

		  if (l->nn > 0)
		    for (i = 0; i < v->nr; i++)
		      v->ia[i + 1] = l->ia[i + 1] - l->ia[i];
		  for (i = 0; i < r->nn; i++)
		    if (r->ja[i] - 1 == i)
		      v->ia[i + 1]++;

		  /* Now set up `ia' */

		  v->ia[0] = 1;
		  for (i = 0; i < v->nr; i++)
		    v->ia[i + 1] += v->ia[i];

		  /* Make the `ja' and `a' assignments. */

		  if (l->nn > 0)
		    {
		      for (i = 0; i < v->nr; i++)
			{
			  if ((k = l->ia[i + 1] - l->ia[i]) > 0)
			    {
			      memcpy (v->ja + v->ia[i] - 1, l->ja + l->ia[i] - 1,
				      k * sizeof (int));
			      memcpy (v->a.complex + v->ia[i] - 1,
				      l->a.complex + l->ia[i] - 1,
				      k * sizeof (COMPLEX));
			    }
			}
		    }
		  for (i = 0; i < r->nn; i++)
		    {
		      v->ja[v->ia[r->ja[i]] - 2] = v->nc;
		      v->a.complex[v->ia[r->ja[i]] - 2] = r->a.complex[i];
		    }
		  v = (MATRIX *) apt_matrix (v);
		}
	      break;
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse_upper:
	  return (append_matrix_vector_complex ((MATRIX *) full_matrix (l),
						r));
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL)
    v->rid = copy_vector ((VECTOR *) l->rid);
  if (l->cid != NULL)
    {
      v->cid = binop_vector_scalar (BO_APPEND,
				 (VECTOR *) copy_vector ((VECTOR *) l->cid),
			(SCALAR *) make_scalar (((VECTOR *) l->cid)->type));
    }

  delete_matrix (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
append_matrix_vector_character (MATRIX *l, VECTOR *r)
{
  MATRIX *v;
  int i, j, k;

  EASSERT (l, matrix, character);
  EASSERT (r, vector, character);

  if (l->nr != r->ne)
    {
      fail ("Inconsistent dimensions for append.  Vector with %d element%s can't be appended to a matrix with %d row%s.",
	    r->ne, PLURAL (r->ne), l->nr, PLURAL (l->nr));
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  else if (!MATCH_VECTORS (l->rid, r->eid))
    {
      fail (bad_append_labels);
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
	      v = (MATRIX *) form_matrix (l->nr, l->nc + 1, character, dense);
	      for (i = 0; i < l->nn; i++)
		v->a.character[i] = dup_char (l->a.character[i]);
	      for (i = 0; i < r->nn; i++)
		v->a.character[i + l->nn] = dup_char (r->a.character[i]);
	      if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		v->symmetry = general;
	      break;
	    case sparse:
	      return (append_matrix_vector_character (
					   l, (VECTOR *) dense_vector (r)));
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
	      return (append_matrix_vector_character (
					  l, (VECTOR *) sparse_vector (r)));
	    case sparse:
	      v = (MATRIX *) make_matrix (l->nr, l->nc + 1,
					  character, sparse);
	      v->nn = l->nn + r->nn;
	      if (v->nn > 0)
		{
		  if (v->nr > 1 || v->nc > 1 || v->nr != v->nc)
		    v->symmetry = general;
		  v->ia = (int *) CALLOC (v->nr + 1, sizeof (int));
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  v->a.character =
		    (char **) MALLOC (v->nn * sizeof (char *));

		  /* Add up the number of nonzeros in each row. */

		  if (l->nn > 0)
		    for (i = 0; i < v->nr; i++)
		      v->ia[i + 1] = l->ia[i + 1] - l->ia[i];
		  for (i = 0; i < r->nn; i++)
		    if (r->ja[i] - 1 == i)
		      v->ia[i + 1]++;

		  /* Now set up `ia' */

		  v->ia[0] = 1;
		  for (i = 0; i < v->nr; i++)
		    v->ia[i + 1] += v->ia[i];

		  /* Make the `ja' and `a' assignments. */

		  if (l->nn > 0)
		    {
		      for (i = 0; i < v->nr; i++)
			{
			  if ((k = l->ia[i + 1] - l->ia[i]) > 0)
			    {
			      memcpy (v->ja + v->ia[i] - 1, l->ja + l->ia[i] - 1,
				      k * sizeof (int));
			      for (j = 0; j < k; j++)
				v->a.character[v->ia[i] - 1 + j] =
				  dup_char (l->a.character[l->ia[i] - 1 + j]);
			    }
			}
		    }
		  for (i = 0; i < r->nn; i++)
		    {
		      v->ja[v->ia[r->ja[i]] - 2] = v->nc;
		      v->a.character[v->ia[r->ja[i]] - 2] =
			dup_char (r->a.character[i]);
		    }
		  v = (MATRIX *) apt_matrix (v);
		}
	      break;
	    default:
	      BAD_DENSITY (r->density);
	      delete_2_entities (ENT (l), ENT (r));
	      raise_exception ();
	    }
	  break;
	case sparse_upper:
	  return (append_matrix_vector_character (
					    (MATRIX *) full_matrix (l), r));
	default:
	  BAD_DENSITY (l->density);
	  delete_2_entities (ENT (l), ENT (r));
	  raise_exception ();
	}
    }

  if (l->rid != NULL)
    v->rid = copy_vector ((VECTOR *) l->rid);
  if (l->cid != NULL)
    {
      v->cid = binop_vector_scalar (BO_APPEND,
				 (VECTOR *) copy_vector ((VECTOR *) l->cid),
			(SCALAR *) make_scalar (((VECTOR *) l->cid)->type));
    }

  delete_matrix (l);
  delete_vector (r);
  return (ENT (v));
}
