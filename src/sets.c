/*
   sets.c -- Functions for sets.

   Copyright (C) 1994-2004  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: sets.c,v 1.4 2004/02/23 04:39:32 ksh Exp $";

#include "sets.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "dense.h"
#include "sort.h"
#include "binop.h"
#include "cast.h"
#include "partition.h"
#include "generate.h"

static int PROTO (check_set_vector, (VECTOR *));

ENTITY *
bi_set (int n, ENTITY *p)
{
  /*
   * This routine makes a "set" out of its argument `p'.  We define a
   * set as a sorted, irredundant vector.  If `p' is NULL, we return an
   * integer vector with zero length.
   */

  if (p)
    {
      return (set_vector ((VECTOR *) vector_entity (p)));
    }
  else
    {
      return (make_vector (0, integer, dense));
    }
}

ENTITY *
set_vector (VECTOR *p)
{
  /* Make a set out of `p'. */

  int i, j;
  int *k;
  VECTOR *index;

  EASSERT (p, vector, 0);

  /* Sort is expensive -- it pays to check it first. */

  if (check_set_vector ((VECTOR *) copy_vector (p)))
    {
      if (!p->eid)
	{
	  p = (VECTOR *) dup_vector ((VECTOR *) p);
	  p->eid = generate_integer (1, p->ne, 1);
	}
    }
  else
    {
      p = (VECTOR *) dup_vector ((VECTOR *) sort_vector (p, 0));

      assert (p->density == dense);	/* `sort_vector' returns dense */
      assert (p->eid);		/* `sort_vector' always provides labels */

      /* Copy labels, too. */

      k = E_MALLOC (p->ne, integer);

      j = 0;
      switch (p->type)
	{
	case integer:
	  k[0] = 1;
	  for (i = 1; i < p->ne; i++)
	    {
	      if (p->a.integer[i] != p->a.integer[j])
		{
		  p->a.integer[++j] = p->a.integer[i];
		  k[j] = i + 1;
		}
	    }
	  break;
	case real:
	  k[0] = 1;
	  for (i = 1; i < p->ne; i++)
	    {
	      if (p->a.real[i] != p->a.real[j])
		{
		  p->a.real[++j] = p->a.real[i];
		  k[j] = i + 1;
		}
	    }
	  break;
	case complex:
	  k[0] = 1;
	  for (i = 1; i < p->ne; i++)
	    {
	      if (p->a.complex[i].real != p->a.complex[j].real ||
		  p->a.complex[i].imag != p->a.complex[j].imag)
		{
		  p->a.complex[++j] = p->a.complex[i];
		  k[j] = i + 1;
		}
	    }
	  break;
	case character:
	  k[0] = 1;
	  for (i = 1; i < p->ne; i++)
	    {
	      if (strcmp (p->a.character[i], p->a.character[j]))
		{
		  p->a.character[++j] = p->a.character[i];
		  k[j] = i + 1;
		}
	      else
		{
		  FREE_CHAR (p->a.character[i]);
		}
	    }
	  break;
	default:
	  BAD_TYPE (p->type);
	  delete_vector (p);
	  raise_exception ();
	}
      p->ne = p->nn = j + 1;

      index = (VECTOR *) gift_wrap_vector (p->ne, integer, k);
      p->eid = partition_vector_vector ((VECTOR *) p->eid, index);
    }

  return ENT (p);
}

ENTITY *
bi_union (ENTITY *l, ENTITY *r)
{
  /* Form union of entities `l' and `r'. */

  return (union_vector ((VECTOR *) vector_entity (l),
			(VECTOR *) vector_entity (r)));
}

ENTITY *
union_vector (VECTOR *l, VECTOR *r)
{
  /* Form union of vectors `l' and `r'. */

  VECTOR *v;

  EASSERT (l, vector, 0);
  EASSERT (r, vector, 0);

  v = (VECTOR *) binop_vector_vector (BO_APPEND, l, r);
  v = (VECTOR *) set_vector (v);

  return ENT (v);
}

ENTITY *
bi_intersection (ENTITY *l, ENTITY *r)
{
  /* Form intersection of entities `l' and `r'. */

  EASSERT (l, 0, 0);
  EASSERT (r, 0, 0);

  return (intersection_vector ((VECTOR *) vector_entity (l),
			       (VECTOR *) vector_entity (r)));
}

ENTITY *
intersection_vector (VECTOR *l, VECTOR *r)
{
  /* Form intersection of vectors `l' and `r'. */

  VECTOR * volatile v = NULL;
  int i, jl, jr, k;

  EASSERT (l, vector, 0);
  EASSERT (r, vector, 0);

  WITH_HANDLING
  {
    AUTO_CAST_VECTOR (l, r);

    /* Doesn't make much sense to keep labels. */

    l = (VECTOR *) dup_vector ((VECTOR *) set_vector ((VECTOR *) EAT (l)));
    delete_entity (l->eid);
    l->eid = NULL;
    r = (VECTOR *) dup_vector ((VECTOR *) set_vector ((VECTOR *) EAT (r)));
    delete_entity (r->eid);
    r->eid = NULL;

    v = (VECTOR *) binop_vector_vector (BO_APPEND,
					(VECTOR *) copy_vector (l),
					(VECTOR *) copy_vector (r));
    v = (VECTOR *) set_vector (v);
    delete_entity (v->eid);
    v->eid = NULL;
  }
  ON_EXCEPTION
  {
    delete_3_vectors (l, r, v);
  }
  END_EXCEPTION;

  jl = jr = k = 0;

  switch (v->type)
    {

    case integer:
      for (i = 0; i < v->ne; i++)
	{
	  while (jl < l->nn && l->a.integer[jl] < v->a.integer[i])
	    jl++;
	  while (jr < r->nn && r->a.integer[jr] < v->a.integer[i])
	    jr++;
	  if (jl != l->nn && jr != r->nn &&
	      l->a.integer[jl] == r->a.integer[jr])
	    {
	      v->a.integer[k++] = v->a.integer[i];
	    }
	}
      break;

    case real:
      for (i = 0; i < v->ne; i++)
	{
	  while (jl < l->nn && l->a.real[jl] < v->a.real[i])
	    jl++;
	  while (jr < r->nn && r->a.real[jr] < v->a.real[i])
	    jr++;
	  if (jl != l->nn && jr != r->nn &&
	      l->a.real[jl] == r->a.real[jr])
	    {
	      v->a.real[k++] = v->a.real[i];
	    }
	}
      break;

    case complex:
      for (i = 0; i < v->ne; i++)
	{			/* Sorted by real then imaginary. */
	  while (jl < l->nn &&
		 (l->a.complex[jl].real < v->a.complex[i].real ||
		  (l->a.complex[jl].real == v->a.complex[i].real &&
		   l->a.complex[jl].imag <
		   v->a.complex[i].imag)))
	    jl++;
	  while (jr < r->nn &&
		 (r->a.complex[jr].real < v->a.complex[i].real ||
		  (r->a.complex[jr].real == v->a.complex[i].real &&
		   r->a.complex[jr].imag <
		   v->a.complex[i].imag)))
	    jr++;
	  if (jl != l->nn && jr != r->nn &&
	      l->a.complex[jl].real == r->a.complex[jr].real &&
	      l->a.complex[jl].imag == r->a.complex[jr].imag)
	    {
	      v->a.complex[k++] = v->a.complex[i];
	    }
	}
      break;

    case character:
      for (i = 0; i < v->ne; i++)
	{
	  while (jl < l->nn &&
		 strcmp (l->a.character[jl], v->a.character[i]) < 0)
	    jl++;
	  while (jr < r->nn &&
		 strcmp (r->a.character[jr], v->a.character[i]) < 0)
	    jr++;
	  if (jl != l->nn && jr != r->nn &&
	      !strcmp (l->a.character[jl], r->a.character[jr]))
	    {
	      v->a.character[k++] = v->a.character[i];
	    }
	  else
	    {
	      FREE_CHAR (v->a.character[i]);
	    }
	}
      break;

    default:
      BAD_TYPE (v->type);
      delete_3_vectors (l, r, v);
      raise_exception ();
    }

  v->ne = k;
  if (v->ne != v->nn)
    {
      if (v->ne > 0)
	{
	  v->a.ptr = REALLOC (v->a.ptr, v->ne * type_size[v->type]);
	}
      else
	{
	  FREE (v->a.ptr);
	  v->a.ptr = NULL;
	}
    }
  v->nn = v->ne;

  delete_2_vectors (l, r);
  return (ENT (v));
}

ENTITY *
bi_complement (ENTITY *l, ENTITY *r)
{
  /* Form relative complement of `l' in `r'. */

  EASSERT (l, 0, 0);
  EASSERT (r, 0, 0);

  return (complement_vector ((VECTOR *) vector_entity (l),
			     (VECTOR *) vector_entity (r)));
}

ENTITY *
complement_vector (VECTOR *l, VECTOR *r)
{
  /* Form relative complement of `l' in `r'. */

  VECTOR * volatile v = NULL;
  VECTOR *vl;
  int i, jl, k;
  int *p;

  EASSERT (l, vector, 0);
  EASSERT (r, vector, 0);

  WITH_HANDLING
  {
    AUTO_CAST_VECTOR (l, r);
    l = (VECTOR *) set_vector ((VECTOR *) EAT (l));
    v = (VECTOR *) dup_vector ((VECTOR *) set_vector (EAT (r)));
    p = E_MALLOC (v->ne, integer);
  }
  ON_EXCEPTION
  {
    delete_3_vectors (l, r, v);
  }
  END_EXCEPTION;

  jl = k = 0;

  switch (v->type)
    {

    case integer:
      for (i = 0; i < v->ne; i++)
	{
	  while (jl < l->nn && l->a.integer[jl] < v->a.integer[i])
	    jl++;
	  if (jl == l->nn || l->a.integer[jl] != v->a.integer[i])
	    {
	      p[k] = i;
	      v->a.integer[k++] = v->a.integer[i];
	    }
	}
      break;

    case real:
      for (i = 0; i < v->ne; i++)
	{
	  while (jl < l->nn && l->a.real[jl] < v->a.real[i])
	    jl++;
	  if (jl == l->nn || l->a.real[jl] != v->a.real[i])
	    {
	      p[k] = i;
	      v->a.real[k++] = v->a.real[i];
	    }
	}
      break;

    case complex:
      for (i = 0; i < v->ne; i++)
	{			/* Sorted by real then imaginary. */
	  while (jl < l->nn &&
		 (l->a.complex[jl].real < v->a.complex[i].real ||
		  (l->a.complex[jl].real == v->a.complex[i].real &&
		   l->a.complex[jl].imag <
		   v->a.complex[i].imag)))
	    jl++;
	  if (jl == l->nn ||
	      (l->a.complex[jl].real != v->a.complex[i].real ||
	       l->a.complex[jl].imag != v->a.complex[i].imag))
	    {
	      p[k] = i;
	      v->a.complex[k++] = v->a.complex[i];
	    }
	}
      break;

    case character:
      for (i = 0; i < v->ne; i++)
	{
	  while (jl < l->nn &&
		 strcmp (l->a.character[jl], v->a.character[i]) < 0)
	    jl++;
	  if (jl == l->nn ||
	      strcmp (l->a.character[jl], v->a.character[i]))
	    {
	      p[k] = i;
	      v->a.character[k++] = v->a.character[i];
	    }
	  else
	    {
	      FREE_CHAR (v->a.character[i]);
	    }
	}
      break;

    default:
      BAD_TYPE (v->type);
      delete_3_vectors (l, r, v);
      raise_exception ();
    }

  v->ne = k;
  if (v->ne != v->nn)
    {
      if (v->ne > 0)
	{
	  v->a.ptr = REALLOC (v->a.ptr, v->ne * type_size[v->type]);
	}
      else
	{
	  FREE (v->a.ptr);
	  v->a.ptr = NULL;
	}
    }
  v->nn = v->ne;

  assert (v->eid);
  vl = (VECTOR *) dup_vector ((VECTOR *) dense_vector ((VECTOR *) v->eid));
  switch (vl->type)
    {
    case integer:
      for (i=0; i<v->ne; i++)
	vl->a.integer[i] = vl->a.integer[p[i]];
      break;
    case real:
      for (i=0; i<v->ne; i++)
	vl->a.real[i] = vl->a.real[p[i]];
      break;
    case complex:
      for (i=0; i<v->ne; i++)
	vl->a.complex[i] = vl->a.complex[p[i]];
      break;
    case character:
      for (k=i=0; i<v->ne; i++)
	{
	  while (k < p[i]) { FREE_CHAR (vl->a.character[k]); k++; }
	  k++;
	  vl->a.character[i] = vl->a.character[p[i]];
	}
      break;
    default:
      BAD_TYPE (vl->type);
      delete_vector (v);
      raise_exception ();
    }
  vl->ne = v->ne;
  if (vl->ne != vl->nn)
    {
      if (vl->ne > 0)
	{
	  vl->a.ptr = REALLOC (vl->a.ptr, vl->ne * type_size[vl->type]);
	}
      else
	{
	  FREE (vl->a.ptr);
	  vl->a.ptr = NULL;
	}
    }
  vl->nn = vl->ne;
  v->eid = ENT (vl);

  FREE (p);
  delete_2_vectors (l, r);
  return (ENT (v));
}

static int
check_set_vector (VECTOR *p)
{
  /* Is `p' a set (that is, sorted and irredundant)? */

  int i;

  EASSERT (p, vector, 0);

  if (!p->ne)
    return 1;

  /* If it's sparse and there's more than one zero, it ain't a set. */

  if (p->density == sparse)
    {
      if ((p->ne - p->nn) > 1)
	return 0;
      p = (VECTOR *) dense_vector (p);
    }

  switch (p->type)
    {
    case integer:
      for (i = 0; i < p->ne-1; i++)
	if (p->a.integer[i] >= p->a.integer[i+1])
	  return 0;
      break;
    case real:
      for (i = 0; i < p->ne-1; i++)
	if (p->a.real[i] >= p->a.real[i+1])
	  return 0;
      break;
    case complex:
      for (i = 0; i < p->ne-1; i++)
	{
	  if (p->a.complex[i].real < p->a.complex[i+1].real)
	    continue;
	  if (p->a.complex[i].real > p->a.complex[i+1].real)
	    return 0;
	  if (p->a.complex[i].imag >= p->a.complex[i+1].imag)
	    return 0;
	}
      break;
    case character:
      for (i = 0; i < p->ne-1; i++)
	if (strcmp (p->a.character[i], p->a.character[i+1]) >= 0)
	  return 0;
      break;
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }

  return 1;
}
