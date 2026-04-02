/*
   partition.c -- Partition matrices.

   Copyright (C) 1994-2001  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: partition.c,v 1.5 2001/07/21 18:52:40 ksh Exp $";

#include "partition.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "sparse.h"
#include "cast.h"
#include "dense.h"
#include "transpose.h"
#include "full.h"
#include "sort.h"
#include "find.h"
#include "table.h"

ENTITY *
partition_1d (ENTITY *v, ENTITY *index)
{
  /*
   * This routine performs vector partitioning; that is, it returns
   * selected elements out of a vector.  If `v' is not a vector,
   * it is converted to one if possible.  The entity returned is a
   * scalar if `index' is a scalar, and a vector otherwise.
   *
   * Normally, the elements of `index' specify the element numbers
   * of the desired elements of `v'.  A special case, though, is
   * when `index' has character type.  In that case, its elements
   * refer instead to the labels of `v'.
   *
   * If `v' is a table, we perform this operation on each of its
   * members.
   *
   * If `index' is NULL, `v' is returned unchanged.
   */

  /* No go on NULL. */

  if (!v)
    {
      delete_entity (index);
      fail ("Partition on NULL.");
      raise_exception ();
    }

  /* Handle table argument. */

  if (v->class == table)
    return map_table ((TABLE *) v, (PFE) partition_1d, 1, &index);

  /* Convert `v' to vector. */

  if (v->class != vector)
    {
      WITH_HANDLING
      {
	v = vector_entity (EAT (v));
      }
      ON_EXCEPTION
      {
	delete_2_entities (v, index);
      }
      END_EXCEPTION;
    }

  /* Just return `v' if `index' is NULL. */

  if (!index)
    return (ENT (v));

  /* Now switch on class and type of `index'. */

  if (index->class == scalar)
    {

      return (((SCALAR *) index)->type == character) ?
	partition_vector_label_scalar ((VECTOR *) v, (SCALAR *) index) :
	partition_vector_scalar ((VECTOR *) v, (SCALAR *) index);

    }
  else
    {

      /* Force `index' to be a vector. */

      if (index->class != vector)
	{
	  WITH_HANDLING
	  {
	    index = vector_entity (EAT (index));
	  }
	  ON_EXCEPTION
	  {
	    delete_2_entities (v, index);
	  }
	  END_EXCEPTION;
	}

      return (((VECTOR *) index)->type == character) ?
	partition_vector_label_vector ((VECTOR *) v, (VECTOR *) index) :
	partition_vector_vector ((VECTOR *) v, (VECTOR *) index);

    }
}

ENTITY *
partition_vector_label_scalar (VECTOR *v, SCALAR *index)
{
  /*
   * This routine returns a scalar that contains an element
   * of `v' selected by label as specified by `index'.
   */

  VECTOR *e;

  EASSERT (v, vector, 0);
  EASSERT (index, scalar, character);

  /* It's an error if `v' doesn't have labels. */

  if (!v->eid)
    {
      fail ("Element labels don't exist in vector partition operation.");
      delete_scalar (index);
      delete_vector (v);
      raise_exception ();
    }

  /* Get the element number. */

  e = (VECTOR *) find_vector ((VECTOR *) scalar_to_vector (index),
			      (VECTOR *) copy_entity (v->eid));
  if (e->ne != 1)
    {
      fail ("Specified label %s in vector partition operation.",
	    (e->ne > 1) ? "is not unique" : "does not exist");
      delete_2_vectors (v, e);
      raise_exception ();
    }

  index = (SCALAR *) vector_to_scalar (e);

  /* Now we have an ordinary partition operation. */

  return partition_vector_scalar (v, index);
}

ENTITY *
partition_vector_label_vector (VECTOR *v, VECTOR *index)
{
  /*
   * This routine returns a vector that contains elements of `v' selected
   * by label as specified by `index'.
   */

  EASSERT (v, vector, 0);
  EASSERT (index, vector, character);

  /* It's an error if `v' doesn't have labels. */

  if (!v->eid)
    {
      fail ("Element labels don't exist in vector partition operation.");
      delete_2_vectors (v, index);
      raise_exception ();
    }

  /* Replace contents of `index' with vector of element numbers. */

  index = (VECTOR *) find_vector (index, (VECTOR *) copy_entity (v->eid));

  /* Now we have an ordinary partition operation. */

  return partition_vector_vector (v, index);
}

ENTITY *
partition_vector_vector (VECTOR *v, VECTOR *index)
{
  /*
   * This function returns a vector that contains the elements of the
   * vector `v' given by element number from vector `index'.
   */

  VECTOR *newv = NULL;
  int *r;
  int i;

  EASSERT (v, vector, 0);
  EASSERT (index, vector, 0);

  /* Make sure element numbers are in a dense, integer array. */

  index = (VECTOR *) cast_vector ((VECTOR *) dense_vector (index), integer);
  r = index->a.integer;

  /* Check for bad dimensions. */

  for (i = 0; i < index->ne; i++)
    {
      if (r[i] < 1 || r[i] > v->ne)
	{
	  fail ("Bad index for vector partition operation.  Element %d was specified in a vector with %d element%s.",
		r[i], v->ne, PLURAL (v->ne));
	  delete_2_entities (ENT (v), ENT (index));
	  raise_exception ();
	}
    }

  switch (v->density)
    {

    case dense:

      newv = (VECTOR *) form_vector (index->ne, v->type, dense);
      switch (v->type)
	{
	case integer:
	  for (i = 0; i < index->ne; i++)
	    {
	      newv->a.integer[i] = v->a.integer[r[i] - 1];
	    }
	  break;
	case real:
	  for (i = 0; i < index->ne; i++)
	    {
	      newv->a.real[i] = v->a.real[r[i] - 1];
	    }
	  break;
	case complex:
	  for (i = 0; i < index->ne; i++)
	    {
	      newv->a.complex[i] = v->a.complex[r[i] - 1];
	    }
	  break;
	case character:
	  for (i = 0; i < index->ne; i++)
	    {
	      newv->a.character[i] = dup_char (v->a.character[r[i] - 1]);
	    }
	  break;
	default:
	  BAD_TYPE (v->type);
	  delete_3_vectors (newv, v, index);
	  raise_exception ();
	}
      break;

    case sparse:

      newv = (VECTOR *) form_vector (index->ne, v->type, sparse);

      if (index->ne > 0 && v->nn > 0)
	{

	  int k = 0;
	  int *ja = v->ja;
	  int *ix = (int *) CALLOC (index->ne, sizeof (int));

	  assert (v->order == ordered);

	  for (i = 0; i < index->ne; i++)
	    {
	      r = (int *) bsearch (index->a.integer + i, ja, v->nn,
				   sizeof (int), compare_integer);
	      if (r)
		{
		  ix[i] = r - ja + 1;
		  k++;
		}
	    }

	  if (k > 0)
	    {

	      newv->a.ptr = E_MALLOC (k, v->type);
	      newv->ja = ix;
	      k = 0;
	      switch (v->type)
		{
		case integer:
		  for (i = 0; i < index->ne; i++)
		    {
		      if (ix[i])
			{
			  newv->a.integer[k] = v->a.integer[ix[i] - 1];
			  ix[k] = i + 1;
			  k++;
			}
		    }
		  break;
		case real:
		  for (i = 0; i < index->ne; i++)
		    {
		      if (ix[i])
			{
			  newv->a.real[k] = v->a.real[ix[i] - 1];
			  ix[k] = i + 1;
			  k++;
			}
		    }
		  break;
		case complex:
		  for (i = 0; i < index->ne; i++)
		    {
		      if (ix[i])
			{
			  newv->a.complex[k] = v->a.complex[ix[i] - 1];
			  ix[k] = i + 1;
			  k++;
			}
		    }
		  break;
		case character:
		  for (i = 0; i < index->ne; i++)
		    {
		      if (ix[i])
			{
			  newv->a.character[k] = dup_char (v->a.character[ix[i] - 1]);
			  ix[k] = i + 1;
			  k++;
			}
		    }
		  break;
		default:
		  BAD_TYPE (v->type);
		  delete_3_vectors (newv, v, index);
		  raise_exception ();
		}
	      newv->ja = REALLOC (newv->ja, k * sizeof (int));
	      newv->nn = k;
	    }
	}

      break;

    default:
      BAD_DENSITY (v->density);
      delete_3_vectors (newv, v, index);
      raise_exception ();
    }

  /*  Now partition the labels.  */

  if (v->eid)
    {
      newv->eid = partition_vector_vector ((VECTOR *) copy_entity (v->eid), index);
    }
  else
    {
      delete_entity (ENT (index));
    }

  delete_vector (v);
  return (apt_vector (newv));
}

ENTITY *
partition_vector_scalar (VECTOR *v, SCALAR *index)
{
  /*
   * This function returns a scalar that contains the element of the
   * vector `v' given by element number from scalar `index'.
   */

  SCALAR *new;
  int e;
  int *r;

  EASSERT (v, vector, 0);
  EASSERT (index, scalar, 0);

  /* Get the element number. */

  index = (SCALAR *) cast_scalar (index, integer);
  e = index->v.integer;
  delete_scalar (index);

  /* Check for bad dimensions. */

  if (e < 1 || e > v->ne)
    {
      fail ("Bad index for vector partition operation.  Element %d was specified in a vector with %d element%s.",
	    e, v->ne, PLURAL (v->ne));
      delete_vector (v);
      raise_exception ();
    }

  new = (SCALAR *) make_scalar (v->type);

  switch (v->density)
    {

    case dense:

      switch (v->type)
	{
	case integer:
	  new->v.integer = v->a.integer[e - 1];
	  break;
	case real:
	  new->v.real = v->a.real[e - 1];
	  break;
	case complex:
	  new->v.complex = v->a.complex[e - 1];
	  break;
	case character:
	  new->v.character = dup_char (v->a.character[e - 1]);
	  break;
	default:
	  BAD_TYPE (v->type);
	  delete_vector (v);
	  delete_scalar (new);
	  raise_exception ();
	}
      break;

    case sparse:

      assert (v->order == ordered);

      if (v->nn > 0)
	{

	  r = (int *) bsearch (&e, v->ja, v->nn,
			       sizeof (int), compare_integer);
	  if (r)
	    {

	      switch (v->type)
		{
		case integer:
		  new->v.integer = v->a.integer[r - v->ja];
		  break;
		case real:
		  new->v.real = v->a.real[r - v->ja];
		  break;
		case complex:
		  new->v.complex = v->a.complex[r - v->ja];
		  break;
		case character:
		  new->v.character = dup_char (v->a.character[r - v->ja]);
		  break;
		default:
		  BAD_TYPE (v->type);
		  delete_vector (v);
		  delete_scalar (new);
		  raise_exception ();
		}
	    }
	}
      break;

    default:
      BAD_DENSITY (v->density);
      delete_vector (v);
      delete_scalar (new);
      raise_exception ();
    }

  delete_vector (v);
  return (ENT (new));
}

ENTITY *
partition_2d (ENTITY *m, ENTITY *rindex, ENTITY *cindex)
{
  /*
   * This function performs matrix partitioning; that is, it returns
   * selected elements out of a matrix.  If `m' is not a matrix, it
   * is converted to one if possible.  The entity returned is a
   * scalar if both indices are scalars, a vector if one of the
   * indices is a vector, and a matrix if both indices are vectors.
   *
   * An exception is raised if an index is not a scalar or cannot be
   * converted to a vector.  You'll also get an exception if a
   * nonexistent element is specified.
   *
   * Normally, the elements of the indices specify the element
   * numbers of the desired elements of `m'.  A special case,
   * though, is when an index has character type.  In that case, its
   * elements refer instead to the corresponding labels of `m'.
   *
   * If `m' is a table, then this operation is performed on each of its
   * members.
   *
   * If `rindex' is NULL, all of the rows are specified -- likewise
   * for `cindex'.
   */

  VECTOR *e;

  /* No go on NULL. */

  if (!m)
    {
      delete_2_entities (rindex, cindex);
      fail ("Partition on NULL.");
      raise_exception ();
    }

  /* Handle table argument. */

  if (m->class == table)
    {
      ENTITY *args[2];
      args[0] = rindex;
      args[1] = cindex;
      return map_table ((TABLE *) m, (PFE) partition_2d, 2, args);
    }

  /* Convert `m' to matrix. */

  if (m->class != matrix)
    {
      WITH_HANDLING
      {
	m = matrix_entity (EAT (m));
      }
      ON_EXCEPTION
      {
	delete_3_entities (m, rindex, cindex);
      }
      END_EXCEPTION;
    }

  /* Switch on class and type of indices. */

  if (rindex && rindex->class == scalar)
    {

      /* Are we referencing labels? */

      if (((SCALAR *) rindex)->type == character)
	{

	  /* Row labels must exist. */

	  if (!((MATRIX *) m)->rid)
	    {
	      fail ("Row labels don't exist in matrix partition operation.");
	      delete_3_entities (m, rindex, cindex);
	      raise_exception ();
	    }

	  /* Get the element number. */

	  e = (VECTOR *) find_vector (
			    (VECTOR *) scalar_to_vector ((SCALAR *) rindex),
			      (VECTOR *) copy_entity (((MATRIX *) m)->rid));
	  if (e->ne != 1)
	    {
	      fail ("Specified row label %s in matrix partition operation.",
		    (e->ne > 1) ? "is not unique" : "does not exist");
	      delete_vector (e);
	      delete_2_entities (m, cindex);
	      raise_exception ();
	    }

	  rindex = vector_to_scalar (e);

	}

      /* Is cindex also a scalar? */

      if (cindex && cindex->class == scalar)
	{

	  /* Are we referencing labels? */

	  if (((SCALAR *) cindex)->type == character)
	    {

	      /* Column labels must exist. */

	      if (!((MATRIX *) m)->cid)
		{
		  fail ("Column labels don't exist in matrix partition operation.");
		  delete_3_entities (m, rindex, cindex);
		  raise_exception ();
		}

	      /* Get the element number. */

	      e = (VECTOR *) find_vector (
			    (VECTOR *) scalar_to_vector ((SCALAR *) cindex),
			      (VECTOR *) copy_entity (((MATRIX *) m)->cid));
	      if (e->ne != 1)
		{
		  fail ("Specified column label %s in matrix partition operation.",
			(e->ne > 1) ? "is not unique" : "does not exist");
		  delete_vector (e);
		  delete_2_entities (m, rindex);
		  raise_exception ();
		}

	      cindex = vector_to_scalar (e);

	    }

	  /* OK, now go partition it. */

	  return partition_matrix_scalar_scalar ((MATRIX *) m,
						 (SCALAR *) rindex,
						 (SCALAR *) cindex);

	}
      else
	{

	  if (cindex)
	    {

	      /* Convert `cindex' to vector. */

	      if (cindex->class != vector)
		{
		  WITH_HANDLING
		  {
		    cindex = vector_entity (EAT (cindex));
		  }
		  ON_EXCEPTION
		  {
		    delete_3_entities (m, rindex, cindex);
		  }
		  END_EXCEPTION;
		}

	      /* Are we referencing labels? */

	      if (((VECTOR *) cindex)->type == character)
		{

		  /* Column labels must exist. */

		  if (!((MATRIX *) m)->cid)
		    {
		      fail ("Column labels don't exist in matrix partition operation.");
		      delete_3_entities (m, rindex, cindex);
		      raise_exception ();
		    }

		  /* Get the element number. */

		  cindex = find_vector (
					 (VECTOR *) cindex,
			      (VECTOR *) copy_entity (((MATRIX *) m)->cid));

		}
	    }

	  return partition_matrix_scalar_vector ((MATRIX *) m,
						 (SCALAR *) rindex,
						 (VECTOR *) cindex);

	}

    }
  else
    {

      if (rindex)
	{

	  /* Convert `rindex' to vector. */

	  if (rindex->class != vector)
	    {
	      WITH_HANDLING
	      {
		rindex = vector_entity (EAT (rindex));
	      }
	      ON_EXCEPTION
	      {
		delete_3_entities (m, rindex, cindex);
	      }
	      END_EXCEPTION;
	    }

	  /* Are we referencing labels? */

	  if (((VECTOR *) rindex)->type == character)
	    {

	      /* Row labels must exist. */

	      if (!((MATRIX *) m)->rid)
		{
		  fail ("Row labels don't exist in matrix partition operation.");
		  delete_3_entities (m, rindex, cindex);
		  raise_exception ();
		}

	      /* Get the element number. */

	      rindex = find_vector (
				     (VECTOR *) rindex,
			      (VECTOR *) copy_entity (((MATRIX *) m)->rid));

	    }
	}

      /* Is cindex a scalar? */

      if (cindex && cindex->class == scalar)
	{

	  /* Are we referencing labels? */

	  if (((SCALAR *) cindex)->type == character)
	    {

	      /* Column labels must exist. */

	      if (!((MATRIX *) m)->cid)
		{
		  fail ("Column labels don't exist in matrix partition operation.");
		  delete_3_entities (m, rindex, cindex);
		  raise_exception ();
		}

	      /* Get the element number. */

	      e = (VECTOR *) find_vector (
			    (VECTOR *) scalar_to_vector ((SCALAR *) cindex),
			      (VECTOR *) copy_entity (((MATRIX *) m)->cid));
	      if (e->ne != 1)
		{
		  fail ("Specified column label %s in matrix partition operation.",
			(e->ne > 1) ? "is not unique" : "does not exist");
		  delete_vector (e);
		  delete_2_entities (m, rindex);
		  raise_exception ();
		}

	      cindex = vector_to_scalar (e);

	    }

	  /* OK, now go partition it. */

	  return partition_matrix_vector_scalar ((MATRIX *) m,
						 (VECTOR *) rindex,
						 (SCALAR *) cindex);

	}
      else
	{

	  if (cindex)
	    {

	      /* Convert `cindex' to vector. */

	      if (cindex->class != vector)
		{
		  WITH_HANDLING
		  {
		    cindex = vector_entity (EAT (cindex));
		  }
		  ON_EXCEPTION
		  {
		    delete_3_entities (m, rindex, cindex);
		  }
		  END_EXCEPTION;
		}

	      /* Are we referencing labels? */

	      if (((VECTOR *) cindex)->type == character)
		{

		  /* Column labels must exist. */

		  if (!((MATRIX *) m)->cid)
		    {
		      fail ("Column labels don't exist in matrix partition operation.");
		      delete_3_entities (m, rindex, cindex);
		      raise_exception ();
		    }

		  /* Get the element number. */

		  cindex = find_vector (
					 (VECTOR *) cindex,
			      (VECTOR *) copy_entity (((MATRIX *) m)->cid));

		}
	    }

	  return partition_matrix_vector_vector ((MATRIX *) m,
						 (VECTOR *) rindex,
						 (VECTOR *) cindex);

	}
    }
}

ENTITY *
partition_matrix_vector_vector (MATRIX *oldm, VECTOR *rindex, VECTOR *cindex)
{
  /*
   * Partition matrix with the given vector indices.  Either index
   * may be NULL.
   */

  MATRIX *newm;
  int *r, *c;
  int i, j, nr, nc;

  EASSERT (oldm, matrix, 0);

  /*
   * If rindex and cindex are both NULL, just return the matrix
   * unchanged.  THE CODE BELOW REQUIRES THAT ONE OF THEM BE NON-NULL.
   */

  if (!rindex && !cindex)
    return ENT (oldm);

  if (rindex)
    {

      EASSERT (rindex, vector, 0);

      /* Make sure rindex is a dense, integer vector. */

      rindex = (VECTOR *) cast_vector ((VECTOR *) dense_vector (rindex),
				       integer);
      r = rindex->a.integer;
      nr = rindex->ne;

      /* Check for bad dimensions. */

      for (i = 0; i < rindex->ne; i++)
	{
	  if (r[i] < 1 || r[i] > oldm->nr)
	    {
	      fail ("Bad index for matrix partition operation.  Row %d was specified in a matrix with %d row%s.",
		    r[i], oldm->nr, PLURAL (oldm->nr));
	      delete_2_vectors (rindex, cindex);
	      delete_matrix (oldm);
	      raise_exception ();
	    }
	}

    }
  else
    {

      /* rindex is NULL, so we'll grab all rows. */

      nr = oldm->nr;

    }

  if (cindex)
    {

      EASSERT (cindex, vector, 0);

      /* Make sure cindex is a dense, integer vector. */

      cindex = (VECTOR *) cast_vector ((VECTOR *) dense_vector (cindex),
				       integer);
      c = cindex->a.integer;
      nc = cindex->ne;

      /* Check for bad dimensions. */

      for (i = 0; i < cindex->ne; i++)
	{
	  if (c[i] < 1 || c[i] > oldm->nc)
	    {
	      fail ("Bad index for matrix partition operation.  Column %d was specified in a matrix with %d column%s.",
		    c[i], oldm->nc, PLURAL (oldm->nc));
	      delete_2_vectors (rindex, cindex);
	      delete_matrix (oldm);
	      raise_exception ();
	    }
	}

    }
  else
    {

      /* cindex is NULL, so we'll grab all columns. */

      nc = oldm->nc;

    }

  switch (oldm->density)
    {

    case dense:

      newm = (MATRIX *) form_matrix (nr, nc, oldm->type, dense);
      newm->symmetry = general;
      switch (oldm->type)
	{

	case integer:

	  if (!rindex)
	    {
	      for (j = 0; j < nc; j++)
		{
		  for (i = 0; i < nr; i++)
		    {
		      newm->a.integer[newm->nr * j + i] =
			oldm->a.integer[oldm->nr * (c[j] - 1) + i];
		    }
		}
	    }
	  else if (!cindex)
	    {
	      for (j = 0; j < nc; j++)
		{
		  for (i = 0; i < nr; i++)
		    {
		      newm->a.integer[newm->nr * j + i] =
			oldm->a.integer[oldm->nr * j + r[i] - 1];
		    }
		}
	    }
	  else
	    {
	      for (j = 0; j < nc; j++)
		{
		  for (i = 0; i < nr; i++)
		    {
		      newm->a.integer[newm->nr * j + i] =
			oldm->a.integer[oldm->nr * (c[j] - 1) + r[i] - 1];
		    }
		}
	    }
	  break;

	case real:

	  if (!rindex)
	    {
	      for (j = 0; j < nc; j++)
		{
		  for (i = 0; i < nr; i++)
		    {
		      newm->a.real[newm->nr * j + i] =
			oldm->a.real[oldm->nr * (c[j] - 1) + i];
		    }
		}
	    }
	  else if (!cindex)
	    {
	      for (j = 0; j < nc; j++)
		{
		  for (i = 0; i < nr; i++)
		    {
		      newm->a.real[newm->nr * j + i] =
			oldm->a.real[oldm->nr * j + r[i] - 1];
		    }
		}
	    }
	  else
	    {
	      for (j = 0; j < nc; j++)
		{
		  for (i = 0; i < nr; i++)
		    {
		      newm->a.real[newm->nr * j + i] =
			oldm->a.real[oldm->nr * (c[j] - 1) + r[i] - 1];
		    }
		}
	    }
	  break;

	case complex:

	  if (!rindex)
	    {
	      for (j = 0; j < nc; j++)
		{
		  for (i = 0; i < nr; i++)
		    {
		      newm->a.complex[newm->nr * j + i] =
			oldm->a.complex[oldm->nr * (c[j] - 1) + i];
		    }
		}
	    }
	  else if (!cindex)
	    {
	      for (j = 0; j < nc; j++)
		{
		  for (i = 0; i < nr; i++)
		    {
		      newm->a.complex[newm->nr * j + i] =
			oldm->a.complex[oldm->nr * j + r[i] - 1];
		    }
		}
	    }
	  else
	    {
	      for (j = 0; j < nc; j++)
		{
		  for (i = 0; i < nr; i++)
		    {
		      newm->a.complex[newm->nr * j + i] =
			oldm->a.complex[oldm->nr * (c[j] - 1) + r[i] - 1];
		    }
		}
	    }
	  break;

	case character:

	  if (!rindex)
	    {
	      for (j = 0; j < nc; j++)
		{
		  for (i = 0; i < nr; i++)
		    {
		      newm->a.character[newm->nr * j + i] =
			dup_char (oldm->a.character[oldm->nr * (c[j] - 1) + i]);
		    }
		}
	    }
	  else if (!cindex)
	    {
	      for (j = 0; j < nc; j++)
		{
		  for (i = 0; i < nr; i++)
		    {
		      newm->a.character[newm->nr * j + i] =
			dup_char (oldm->a.character[oldm->nr * j + r[i] - 1]);
		    }
		}
	    }
	  else
	    {
	      for (j = 0; j < nc; j++)
		{
		  for (i = 0; i < nr; i++)
		    {
		      newm->a.character[newm->nr * j + i] =
			dup_char (oldm->a.character[oldm->nr * (c[j] - 1) + r[i] - 1]);
		    }
		}
	    }
	  break;

	default:

	  BAD_TYPE (oldm->type);
	  delete_2_vectors (rindex, cindex);
	  delete_2_matrices (oldm, newm);
	  raise_exception ();
	}

      if (oldm->rid)
	newm->rid = (!rindex) ?
	  copy_entity (oldm->rid) :
	  partition_vector_vector ((VECTOR *) copy_entity (oldm->rid),
				   (VECTOR *) copy_entity (ENT (rindex)));
      if (oldm->cid)
	newm->cid = (!cindex) ?
	  copy_entity (oldm->cid) :
	  partition_vector_vector ((VECTOR *) copy_entity (oldm->cid),
				   (VECTOR *) copy_entity (ENT (cindex)));

      /* Is the result obviously symmetric or hermitian? */

      if (oldm->symmetry == symmetric &&
	  rindex && cindex &&
	  (rindex == cindex || match_vectors (rindex, cindex)))
	{
	  newm->symmetry = symmetric;
	}
      else if (oldm->symmetry == hermitian &&
	       rindex && cindex &&
	       (rindex == cindex || match_vectors (rindex, cindex)))
	{
	  newm->symmetry = hermitian;
	}

      delete_2_vectors (rindex, cindex);
      delete_matrix (oldm);
      break;

    case sparse:

      if (nr > 0 && nc > 0)
	{

	  /* Is the result obviously symmetric or hermitian? */

	  SYMMETRY sym = general;
	  if (oldm->symmetry == symmetric &&
	      rindex && cindex &&
	      (rindex == cindex ||
	       match_vectors (rindex, cindex)))
	    {
	      sym = symmetric;
	    }
	  else if (oldm->symmetry == hermitian &&
		   rindex && cindex &&
		   (rindex == cindex ||
		    match_vectors (rindex, cindex)))
	    {
	      sym = hermitian;
	    }

	  newm = (MATRIX *) row_partition_matrix_sparse (oldm, rindex);
	  newm = (MATRIX *) transpose_matrix (newm);
	  newm = (MATRIX *) row_partition_matrix_sparse (newm, cindex);

	  newm->symmetry = sym;

	  newm = (MATRIX *) transpose_matrix (newm);
	  newm = (MATRIX *) apt_matrix (newm);

	}
      else
	{
	  newm = (MATRIX *) make_matrix (nr, nc, oldm->type, sparse);
	  if (oldm->rid)
	    newm->rid = (!rindex) ?
	      copy_entity (oldm->rid) :
	      partition_vector_vector ((VECTOR *) copy_entity (oldm->rid),
				     (VECTOR *) copy_entity (ENT (rindex)));
	  if (oldm->cid)
	    newm->cid = (!cindex) ?
	      copy_entity (oldm->cid) :
	      partition_vector_vector ((VECTOR *) copy_entity (oldm->cid),
				     (VECTOR *) copy_entity (ENT (cindex)));
	  delete_2_vectors (rindex, cindex);
	  delete_matrix (oldm);
	}
      break;

    case sparse_upper:

      newm = (MATRIX *) partition_matrix_vector_vector (
			     (MATRIX *) full_matrix (oldm), rindex, cindex);
      break;

    default:

      BAD_DENSITY (oldm->density);
      delete_2_vectors (rindex, cindex);
      delete_matrix (oldm);
      raise_exception ();
    }

  return (ENT (newm));
}

ENTITY *
partition_matrix_scalar_vector (MATRIX *m, SCALAR *rindex, VECTOR *cindex)
{
  /*
   * Partition matrix with the given indices.  The vector cindex
   * may be NULL.
   */

  VECTOR *v;
  int r, *c;
  int j, nc;

  EASSERT (m, matrix, 0);
  EASSERT (rindex, scalar, 0);

  /* Get row number. */

  rindex = (SCALAR *) cast_scalar (rindex, integer);
  r = rindex->v.integer;

  /* Check for bad row dimension. */

  if (r < 1 || r > m->nr)
    {
      fail ("Bad index for matrix partition operation.  Row %d was specified in a matrix with %d row%s.",
	    r, m->nr, PLURAL (m->nr));
      delete_scalar (rindex);
      delete_vector (cindex);
      delete_matrix (m);
      raise_exception ();
    }

  if (cindex)
    {

      EASSERT (cindex, vector, 0);

      /* Make sure cindex is a dense, integer vector. */

      cindex = (VECTOR *) cast_vector ((VECTOR *) dense_vector (cindex),
				       integer);
      c = cindex->a.integer;
      nc = cindex->ne;

      /* Check for bad dimensions. */

      for (j = 0; j < cindex->ne; j++)
	{
	  if (c[j] < 1 || c[j] > m->nc)
	    {
	      fail ("Bad index for matrix partition operation.  Column %d was specified in a matrix with %d column%s.",
		    c[j], m->nc, PLURAL (m->nc));
	      delete_scalar (rindex);
	      delete_vector (cindex);
	      delete_matrix (m);
	      raise_exception ();
	    }
	}

    }
  else
    {

      /* cindex is NULL, so we'll grab all columns. */

      nc = m->nc;

    }

  switch (m->density)
    {

    case dense:

      v = (VECTOR *) form_vector (nc, m->type, dense);

      switch (m->type)
	{

	case integer:

	  if (!cindex)
	    {
	      for (j = 0; j < nc; j++)
		{
		  v->a.integer[j] = m->a.integer[m->nr * j + r - 1];
		}
	    }
	  else
	    {
	      for (j = 0; j < nc; j++)
		{
		  v->a.integer[j] = m->a.integer[m->nr * (c[j] - 1) + r - 1];
		}
	    }
	  break;

	case real:

	  if (!cindex)
	    {
	      for (j = 0; j < nc; j++)
		{
		  v->a.real[j] = m->a.real[m->nr * j + r - 1];
		}
	    }
	  else
	    {
	      for (j = 0; j < nc; j++)
		{
		  v->a.real[j] = m->a.real[m->nr * (c[j] - 1) + r - 1];
		}
	    }
	  break;

	case complex:

	  if (!cindex)
	    {
	      for (j = 0; j < nc; j++)
		{
		  v->a.complex[j] = m->a.complex[m->nr * j + r - 1];
		}
	    }
	  else
	    {
	      for (j = 0; j < nc; j++)
		{
		  v->a.complex[j] = m->a.complex[m->nr * (c[j] - 1) + r - 1];
		}
	    }
	  break;

	case character:

	  if (!cindex)
	    {
	      for (j = 0; j < nc; j++)
		{
		  v->a.character[j] =
		    dup_char (m->a.character[m->nr * j + r - 1]);
		}
	    }
	  else
	    {
	      for (j = 0; j < nc; j++)
		{
		  v->a.character[j] =
		    dup_char (m->a.character[m->nr * (c[j] - 1) + r - 1]);
		}
	    }
	  break;

	default:

	  BAD_TYPE (m->type);
	  delete_scalar (rindex);
	  delete_2_vectors (v, cindex);
	  delete_matrix (m);
	  raise_exception ();
	}

      if (m->cid)
	v->eid = (!cindex) ?
	  copy_entity (m->cid) :
	  partition_vector_vector ((VECTOR *) copy_entity (m->cid),
				   (VECTOR *) copy_entity (ENT (cindex)));

      delete_scalar (rindex);
      delete_vector (cindex);
      delete_matrix (m);
      break;

    case sparse:

      if (nc > 0)
	{

	  v = (VECTOR *) matrix_to_vector (
				  (MATRIX *) row_partition_matrix_sparse (m,
				     (VECTOR *) scalar_to_vector (rindex)));
	  if (cindex)
	    v = (VECTOR *) partition_vector_vector (v, cindex);

	}
      else
	{

	  v = (VECTOR *) make_vector (nc, m->type, sparse);

	  if (m->cid)
	    v->eid = (!cindex) ?
	      copy_entity (m->cid) :
	      partition_vector_vector ((VECTOR *) copy_entity (m->cid),
				     (VECTOR *) copy_entity (ENT (cindex)));
	  delete_scalar (rindex);
	  delete_vector (cindex);
	  delete_matrix (m);
	}
      break;

    case sparse_upper:

      v = (VECTOR *) partition_matrix_scalar_vector (
				(MATRIX *) full_matrix (m), rindex, cindex);
      break;

    default:

      BAD_DENSITY (m->density);
      delete_scalar (rindex);
      delete_vector (cindex);
      delete_matrix (m);
      raise_exception ();
    }

  return (ENT (v));
}

ENTITY *
partition_matrix_vector_scalar (MATRIX *m, VECTOR *rindex, SCALAR *cindex)
{
  /*
   * Partition matrix with the given indices.  The vector rindex
   * may be NULL.
   */

  VECTOR *v;
  int *r, c;
  int i, nr;

  EASSERT (m, matrix, 0);
  EASSERT (cindex, scalar, 0);

  if (rindex)
    {

      EASSERT (rindex, vector, 0);

      /* Make sure rindex is a dense, integer vector. */

      rindex = (VECTOR *) cast_vector ((VECTOR *) dense_vector (rindex),
				       integer);
      r = rindex->a.integer;
      nr = rindex->ne;

      /* Check for bad dimensions. */

      for (i = 0; i < rindex->ne; i++)
	{
	  if (r[i] < 1 || r[i] > m->nr)
	    {
	      fail ("Bad index for matrix partition operation.  Row %d was specified in a matrix with %d row%s.",
		    r[i], m->nr, PLURAL (m->nr));
	      delete_scalar (cindex);
	      delete_vector (rindex);
	      delete_matrix (m);
	      raise_exception ();
	    }
	}

    }
  else
    {

      /* rindex is NULL, so we'll grab all rows. */

      nr = m->nr;

    }

  /* Get column number. */

  cindex = (SCALAR *) cast_scalar (cindex, integer);
  c = cindex->v.integer;

  /* Check for bad column dimension. */

  if (c < 1 || c > m->nc)
    {
      fail ("Bad index for matrix partition operation.  Column %d was specified in a matrix with %d column%s.",
	    c, m->nc, PLURAL (m->nc));
      delete_scalar (cindex);
      delete_vector (rindex);
      delete_matrix (m);
      raise_exception ();
    }

  switch (m->density)
    {

    case dense:

      v = (VECTOR *) form_vector (nr, m->type, dense);

      switch (m->type)
	{

	case integer:

	  if (!rindex)
	    {
	      for (i = 0; i < nr; i++)
		{
		  v->a.integer[i] = m->a.integer[m->nr * (c - 1) + i];
		}
	    }
	  else
	    {
	      for (i = 0; i < nr; i++)
		{
		  v->a.integer[i] = m->a.integer[m->nr * (c - 1) + r[i] - 1];
		}
	    }
	  break;

	case real:

	  if (!rindex)
	    {
	      for (i = 0; i < nr; i++)
		{
		  v->a.real[i] = m->a.real[m->nr * (c - 1) + i];
		}
	    }
	  else
	    {
	      for (i = 0; i < nr; i++)
		{
		  v->a.real[i] = m->a.real[m->nr * (c - 1) + r[i] - 1];
		}
	    }
	  break;

	case complex:

	  if (!rindex)
	    {
	      for (i = 0; i < nr; i++)
		{
		  v->a.complex[i] = m->a.complex[m->nr * (c - 1) + i];
		}
	    }
	  else
	    {
	      for (i = 0; i < nr; i++)
		{
		  v->a.complex[i] = m->a.complex[m->nr * (c - 1) + r[i] - 1];
		}
	    }
	  break;

	case character:

	  if (!rindex)
	    {
	      for (i = 0; i < nr; i++)
		{
		  v->a.character[i] =
		    dup_char (m->a.character[m->nr * (c - 1) + i]);
		}
	    }
	  else
	    {
	      for (i = 0; i < nr; i++)
		{
		  v->a.character[i] =
		    dup_char (m->a.character[m->nr * (c - 1) + r[i] - 1]);
		}
	    }
	  break;

	default:

	  BAD_TYPE (m->type);
	  delete_scalar (cindex);
	  delete_2_vectors (v, rindex);
	  delete_matrix (m);
	  raise_exception ();
	}

      if (m->rid)
	v->eid = (!rindex) ?
	  copy_entity (m->rid) :
	  partition_vector_vector ((VECTOR *) copy_entity (m->rid),
				   (VECTOR *) copy_entity (ENT (rindex)));

      delete_scalar (cindex);
      delete_vector (rindex);
      delete_matrix (m);
      break;

    case sparse:

      if (nr > 0)
	{

	  m = (MATRIX *) row_partition_matrix_sparse (m, rindex);
	  m = (MATRIX *) transpose_matrix (m);
	  m = (MATRIX *) row_partition_matrix_sparse (m,
				      (VECTOR *) scalar_to_vector (cindex));
	  v = (VECTOR *) matrix_to_vector (m);

	}
      else
	{

	  v = (VECTOR *) make_vector (nr, m->type, sparse);

	  if (m->rid)
	    v->eid = (!rindex) ?
	      copy_entity (m->rid) :
	      partition_vector_vector ((VECTOR *) copy_entity (m->rid),
				     (VECTOR *) copy_entity (ENT (rindex)));

	  delete_scalar (cindex);
	  delete_vector (rindex);
	  delete_matrix (m);
	}
      break;

    case sparse_upper:

      v = (VECTOR *) partition_matrix_vector_scalar (
				(MATRIX *) full_matrix (m), rindex, cindex);
      break;

    default:

      BAD_DENSITY (m->density);
      delete_scalar (cindex);
      delete_vector (rindex);
      delete_matrix (m);
      raise_exception ();
    }

  return (ENT (v));
}

ENTITY *
partition_matrix_scalar_scalar (MATRIX *m, SCALAR *rindex, SCALAR *cindex)
{
  /* Partition matrix with the given scalar indices. */

  SCALAR *s;
  int r, c;

  EASSERT (m, matrix, 0);
  EASSERT (rindex, scalar, 0);
  EASSERT (cindex, scalar, 0);

  /* Get row number. */

  rindex = (SCALAR *) cast_scalar (rindex, integer);
  r = rindex->v.integer;

  /* Check for bad row dimension. */

  if (r < 1 || r > m->nr)
    {
      fail ("Bad index for matrix partition operation.  Row %d was specified in a matrix with %d row%s.",
	    r, m->nr, PLURAL (m->nr));
      delete_2_scalars (rindex, cindex);
      delete_matrix (m);
      raise_exception ();
    }

  /* Get column number. */

  cindex = (SCALAR *) cast_scalar (cindex, integer);
  c = cindex->v.integer;

  /* Check for bad column dimension. */

  if (c < 1 || c > m->nc)
    {
      fail ("Bad index for matrix partition operation.  Column %d was specified in a matrix with %d column%s.",
	    c, m->nc, PLURAL (m->nc));
      delete_2_scalars (rindex, cindex);
      delete_matrix (m);
      raise_exception ();
    }

  switch (m->density)
    {

    case dense:

      switch (m->type)
	{

	case integer:

	  s = (SCALAR *) int_to_scalar (m->a.integer[m->nr * (c - 1) + r - 1]);
	  break;

	case real:

	  s = (SCALAR *) real_to_scalar (m->a.real[m->nr * (c - 1) + r - 1]);
	  break;

	case complex:

	  s = (SCALAR *) complex_to_scalar (m->a.complex[m->nr * (c - 1) + r - 1]);
	  break;

	case character:

	  s = (SCALAR *)
	    char_to_scalar (dup_char (m->a.character[m->nr * (c - 1) + r - 1]));
	  break;

	default:

	  BAD_TYPE (m->type);
	  delete_2_scalars (rindex, cindex);
	  delete_matrix (m);
	  raise_exception ();
	}

      delete_2_scalars (rindex, cindex);
      delete_matrix (m);
      break;

    case sparse:
    case sparse_upper:

      switch (m->type)
	{

	case integer:

	  s = (SCALAR *)
	    int_to_scalar (get_matrix_element_integer (m, r, c));
	  break;

	case real:

	  s = (SCALAR *)
	    real_to_scalar (get_matrix_element_real (m, r, c));
	  break;

	case complex:

	  s = (SCALAR *)
	    complex_to_scalar (get_matrix_element_complex (m, r, c));
	  break;

	case character:

	  s = (SCALAR *)
	    char_to_scalar (get_matrix_element_character (m, r, c));
	  break;

	default:

	  BAD_TYPE (m->type);
	  delete_2_scalars (rindex, cindex);
	  delete_matrix (m);
	  raise_exception ();
	}

      delete_2_scalars (rindex, cindex);

      break;

    default:

      BAD_DENSITY (m->density);
      delete_2_scalars (rindex, cindex);
      delete_matrix (m);
      raise_exception ();
    }

  return (ENT (s));
}

ENTITY *
row_partition_matrix_sparse (MATRIX *oldm, VECTOR *rindex)
{
  MATRIX *newm;
  int i, k, m, kk;
  int *r;
  size_t obj_size;

  EASSERT (oldm, matrix, 0);
  if (rindex == NULL)
    return (ENT (oldm));
  EASSERT (rindex, vector, integer);
  assert (oldm->density == sparse);
  assert (rindex->density == dense);

  newm = (MATRIX *) make_matrix (rindex->ne, oldm->nc, oldm->type, sparse);
  newm->symmetry = general;
  if (oldm->cid)
    newm->cid = copy_vector ((VECTOR *) oldm->cid);

  obj_size = type_size[newm->type];
  r = rindex->a.integer;

  if (oldm->nn > 0)
    {
      k = 0;
      for (i = 0; i < newm->nr; i++)
	{
	  k += oldm->ia[r[i]] - oldm->ia[r[i] - 1];
	}
      if (k > 0)
	{
	  newm->nn = k;
	  newm->a.ptr = E_MALLOC (newm->nn, newm->type);
	  newm->ja = (int *) MALLOC (newm->nn * sizeof (int));
	  newm->ia = (int *) MALLOC ((newm->nr + 1) * sizeof (int));

	  k = 0;
	  for (i = 0; i < newm->nr; i++)
	    {
	      newm->ia[i] = k + 1;
	      m = oldm->ia[r[i]] - oldm->ia[r[i] - 1];
	      if (m > 0)
		{
		  kk = oldm->ia[r[i] - 1] - 1;
		  memcpy (newm->ja + k, oldm->ja + kk, m * sizeof (int));
		  memcpy ((char *) newm->a.ptr + k * obj_size,
			  (char *) oldm->a.ptr + kk * obj_size,
			  m * obj_size);
		}
	      k += m;
	    }
	  newm->ia[i] = k + 1;

	  if (newm->type == character)
	    {
	      for (i = 0; i < newm->nn; i++)
		{
		  newm->a.character[i] = dup_char (newm->a.character[i]);
		}
	    }
	}
    }
  if (oldm->rid != NULL)
    newm->rid = partition_vector_vector ((VECTOR *) copy_entity (oldm->rid),
				     (VECTOR *) copy_entity (ENT (rindex)));
  delete_2_entities (ENT (oldm), ENT (rindex));
  return (ENT (newm));
}

int
get_matrix_element_integer (MATRIX *m, int r, int c)
{
  int e = 0;
  int k;

  switch (m->density)
    {
    case dense:
      e = m->a.integer[r - 1 + m->nr * (c - 1)];
      break;
    case sparse:
      if (m->nn > 0)
	{
	  for (k = m->ia[r - 1] - 1; k < m->ia[r] - 1; k++)
	    {
	      if (m->ja[k] > c && m->order == ordered)
		break;
	      if (m->ja[k] == c)
		e = m->a.integer[k];
	    }
	}
      break;
    case sparse_upper:
      if (c > r)
	{
	  if (m->nn > 0)
	    {
	      for (k = m->ia[r - 1] - 1; k < m->ia[r] - 1; k++)
		{
		  if (m->ja[k] > c && m->order == ordered)
		    break;
		  if (m->ja[k] == c)
		    e = m->a.integer[k];
		}
	    }
	}
      else if (c == r)
	{
	  if (m->d.integer != NULL)
	    e = m->d.integer[r - 1];
	}
      else
	{
	  if (m->nn > 0)
	    {
	      for (k = m->ia[c - 1] - 1; k < m->ia[c] - 1; k++)
		{
		  if (m->ja[k] > r && m->order == ordered)
		    break;
		  if (m->ja[k] == r)
		    e = m->a.integer[k];
		}
	      switch (m->symmetry)
		{
		case general:
		  e = 0;
		  break;
		case symmetric:
		case hermitian:
		  break;
		default:
		  BAD_SYMMETRY (m->symmetry);
		  delete_matrix (m);
		  raise_exception ();
		}
	    }
	}
      break;
    default:
      BAD_DENSITY (m->density);
      delete_matrix (m);
      raise_exception ();
    }

  delete_matrix (m);
  return (e);
}

REAL
get_matrix_element_real (MATRIX *m, int r, int c)
{
  REAL e = 0;
  int k;

  switch (m->density)
    {
    case dense:
      e = m->a.real[r - 1 + m->nr * (c - 1)];
      break;
    case sparse:
      if (m->nn > 0)
	{
	  for (k = m->ia[r - 1] - 1; k < m->ia[r] - 1; k++)
	    {
	      if (m->ja[k] > c && m->order == ordered)
		break;
	      if (m->ja[k] == c)
		e = m->a.real[k];
	    }
	}
      break;
    case sparse_upper:
      if (c > r)
	{
	  if (m->nn > 0)
	    {
	      for (k = m->ia[r - 1] - 1; k < m->ia[r] - 1; k++)
		{
		  if (m->ja[k] > c && m->order == ordered)
		    break;
		  if (m->ja[k] == c)
		    e = m->a.real[k];
		}
	    }
	}
      else if (c == r)
	{
	  if (m->d.real != NULL)
	    e = m->d.real[r - 1];
	}
      else
	{
	  if (m->nn > 0)
	    {
	      for (k = m->ia[c - 1] - 1; k < m->ia[c] - 1; k++)
		{
		  if (m->ja[k] > r && m->order == ordered)
		    break;
		  if (m->ja[k] == r)
		    e = m->a.real[k];
		}
	      switch (m->symmetry)
		{
		case general:
		  e = 0;
		  break;
		case symmetric:
		case hermitian:
		  break;
		default:
		  BAD_SYMMETRY (m->symmetry);
		  delete_matrix (m);
		  raise_exception ();
		}
	    }
	}
      break;
    default:
      BAD_DENSITY (m->density);
      delete_matrix (m);
      raise_exception ();
    }

  delete_matrix (m);
  return (e);
}

COMPLEX
get_matrix_element_complex (MATRIX *m, int r, int c)
{
  COMPLEX e;
  int k;

  e.real = 0;
  e.imag = 0;

  switch (m->density)
    {
    case dense:
      e = m->a.complex[r - 1 + m->nr * (c - 1)];
      break;
    case sparse:
      if (m->nn > 0)
	{
	  for (k = m->ia[r - 1] - 1; k < m->ia[r] - 1; k++)
	    {
	      if (m->ja[k] > c && m->order == ordered)
		break;
	      if (m->ja[k] == c)
		e = m->a.complex[k];
	    }
	}
      break;
    case sparse_upper:
      if (c > r)
	{
	  if (m->nn > 0)
	    {
	      for (k = m->ia[r - 1] - 1; k < m->ia[r] - 1; k++)
		{
		  if (m->ja[k] > c && m->order == ordered)
		    break;
		  if (m->ja[k] == c)
		    e = m->a.complex[k];
		}
	    }
	}
      else if (c == r)
	{
	  if (m->d.complex != NULL)
	    e = m->d.complex[r - 1];
	}
      else
	{
	  if (m->nn > 0)
	    {
	      for (k = m->ia[c - 1] - 1; k < m->ia[c] - 1; k++)
		{
		  if (m->ja[k] > r && m->order == ordered)
		    break;
		  if (m->ja[k] == r)
		    e = m->a.complex[k];
		}
	      switch (m->symmetry)
		{
		case general:
		  e.real = 0;
		  e.imag = 0;
		  break;
		case symmetric:
		  break;
		case hermitian:
		  e.imag = -e.imag;
		  break;
		default:
		  BAD_SYMMETRY (m->symmetry);
		  delete_matrix (m);
		  raise_exception ();
		}
	    }
	}
      break;
    default:
      BAD_DENSITY (m->density);
      delete_matrix (m);
      raise_exception ();
    }

  delete_matrix (m);
  return (e);
}

char *
get_matrix_element_character (MATRIX *m, int r, int c)
{
  char *e = NULL_string;
  int k;

  switch (m->density)
    {
    case dense:
      e = m->a.character[r - 1 + m->nr * (c - 1)];
      break;
    case sparse:
      if (m->nn > 0)
	{
	  for (k = m->ia[r - 1] - 1; k < m->ia[r] - 1; k++)
	    {
	      if (m->ja[k] > c && m->order == ordered)
		break;
	      if (m->ja[k] == c)
		e = m->a.character[k];
	    }
	}
      break;
    case sparse_upper:
      if (c > r)
	{
	  if (m->nn > 0)
	    {
	      for (k = m->ia[r - 1] - 1; k < m->ia[r] - 1; k++)
		{
		  if (m->ja[k] > c && m->order == ordered)
		    break;
		  if (m->ja[k] == c)
		    e = m->a.character[k];
		}
	    }
	}
      else if (c == r)
	{
	  if (m->d.character != NULL)
	    e = m->d.character[r - 1];
	}
      else
	{
	  if (m->nn > 0)
	    {
	      for (k = m->ia[c - 1] - 1; k < m->ia[c] - 1; k++)
		{
		  if (m->ja[k] > r && m->order == ordered)
		    break;
		  if (m->ja[k] == r)
		    e = m->a.character[k];
		}
	      switch (m->symmetry)
		{
		case general:
		  e = NULL_string;
		  break;
		case symmetric:
		  break;
		case hermitian:
		default:
		  BAD_SYMMETRY (m->symmetry);
		  delete_matrix (m);
		  raise_exception ();
		}
	    }
	}
      break;
    default:
      BAD_DENSITY (m->density);
      delete_matrix (m);
      raise_exception ();
    }

  e = dup_char (e);
  delete_matrix (m);
  return e;
}

int
get_vector_element_integer (VECTOR *v, int elem)
{
  int e = 0;
  int k;

  switch (v->density)
    {
    case dense:
      e = v->a.integer[elem - 1];
      break;
    case sparse:
      if (v->nn > 0)
	{
	  for (k = 0; k < v->nn; k++)
	    {
	      if (v->ja[k] > elem && v->order == ordered)
		break;
	      if (v->ja[k] == elem)
		e = v->a.integer[k];
	    }
	}
      break;
    default:
      BAD_DENSITY (v->density);
      delete_vector (v);
      raise_exception ();
    }

  delete_vector (v);
  return (e);
}

REAL
get_vector_element_real (VECTOR *v, int elem)
{
  REAL e = 0.0;
  int k;

  switch (v->density)
    {
    case dense:
      e = v->a.real[elem - 1];
      break;
    case sparse:
      if (v->nn > 0)
	{
	  for (k = 0; k < v->nn; k++)
	    {
	      if (v->ja[k] > elem && v->order == ordered)
		break;
	      if (v->ja[k] == elem)
		e = v->a.real[k];
	    }
	}
      break;
    default:
      BAD_DENSITY (v->density);
      delete_vector (v);
      raise_exception ();
    }

  delete_vector (v);
  return (e);
}

COMPLEX
get_vector_element_complex (VECTOR *v, int elem)
{
  COMPLEX e;
  int k;

  e.real = 0.0;
  e.imag = 0.0;

  switch (v->density)
    {
    case dense:
      e = v->a.complex[elem - 1];
      break;
    case sparse:
      if (v->nn > 0)
	{
	  for (k = 0; k < v->nn; k++)
	    {
	      if (v->ja[k] > elem && v->order == ordered)
		break;
	      if (v->ja[k] == elem)
		e = v->a.complex[k];
	    }
	}
      break;
    default:
      BAD_DENSITY (v->density);
      delete_vector (v);
      raise_exception ();
    }

  delete_vector (v);
  return (e);
}

char *
get_vector_element_character (VECTOR *v, int elem)
{
  char *e = NULL_string;
  int k;

  switch (v->density)
    {
    case dense:
      e = v->a.character[elem - 1];
      break;
    case sparse:
      if (v->nn > 0)
	{
	  for (k = 0; k < v->nn; k++)
	    {
	      if (v->ja[k] > elem && v->order == ordered)
		break;
	      if (v->ja[k] == elem)
		e = v->a.character[k];
	    }
	}
      break;
    default:
      BAD_DENSITY (v->density);
      delete_vector (v);
      raise_exception ();
    }

  e = dup_char (e);
  delete_vector (v);
  return e;
}
