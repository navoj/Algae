/*
   fill.c -- fill an array

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: fill.c,v 1.2 2003/08/01 04:57:47 ksh Exp $";

#include "fill.h"
#include "entity.h"
#include "datum.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "cast.h"
#include "transpose.h"
#include "dense.h"

ENTITY *
bi_fill (ENTITY *shape, ENTITY *v)
{
  /*
   * This function returns an entity with the given `shape' that
   * contains the elements of `v'.  If `shape' is NULL or has zero
   * length, a scalar is returned.  If `shape' has length one, then
   * a vector with `shape[1]' elements is returned.  If `shape' has two
   * elements, then a matrix is returned with `shape[1]' rows and
   * `shape[2]' columns.  If `v' is a matrix, its elements are used
   * by rows.  If `v' doesn't contain enough elements to fill the
   * result, we wrap around and use `v' all over again.
   */

  ENTITY * volatile r = NULL;

  WITH_HANDLING
  {

    if (shape)
      shape = vector_entity (EAT (shape));

    switch (v->class)
      {

      case scalar:
	r = fill_scalar (EAT (shape), EAT (v));
	break;

      case vector:
	r = fill_vector (EAT (shape), EAT (v));
	break;

      case matrix:
	r = fill_matrix (EAT (shape), EAT (v));
	break;

      default:
	fail ("Can't use a %s in \"fill\".", class_string[v->class]);
	raise_exception ();

      }

  }
  ON_EXCEPTION
  {
    delete_3_entities (shape, v, r);
  }
  END_EXCEPTION;

  return r;
}

ENTITY *
fill_scalar (VECTOR *shape, SCALAR *v)
{
  ENTITY * volatile d = NULL;
  int cnt, i;

  WITH_HANDLING
  {
    if (shape)
      {
	shape = (VECTOR *) dense_vector (EAT (shape));
	shape = (VECTOR *) cast_vector (EAT (shape), integer);

	switch (shape->ne)
	  {
	  case 0:		/* scalar result */

	    d = copy_scalar (v);
	    break;

	  case 1:		/* vector result */

	    cnt = shape->a.integer[0];
	    if (cnt < 0)
	      {
		fail ("Specified vector dimension is negative.");
		raise_exception ();
	      }
	    d = form_vector (cnt, v->type, dense);

	    switch (v->type)
	      {
	      case integer:
		{
		  int *p = ((VECTOR *) d)->a.integer;
		  int val = v->v.integer;
		  for (i = 0; i < cnt; i++)
		    *p++ = val;
		  break;
		}
	      case real:
		{
		  REAL *p = ((VECTOR *) d)->a.real;
		  REAL val = v->v.real;
		  for (i = 0; i < cnt; i++)
		    *p++ = val;
		  break;
		}
	      case complex:
		{
		  COMPLEX *p, val;
		  p = ((VECTOR *) d)->a.complex;
		  val = v->v.complex;
		  for (i = 0; i < cnt; i++)
		    *p++ = val;
		  break;
		}
	      case character:
		{
		  char **p = ((VECTOR *) d)->a.character;
		  char *val = v->v.character;
		  for (i = 0; i < cnt; i++)
		    *p++ = dup_char (val);
		  break;
		}
	      default:
		wipeout ("Bad type.");
	      }
	    break;

	  case 2:		/* matrix result */

	    if (shape->a.integer[0] < 0 || shape->a.integer[1] < 0)
	      {
		fail ("Specified matrix dimension is negative.");
		raise_exception ();
	      }
	    d = form_matrix (shape->a.integer[0], shape->a.integer[1],
			     v->type, dense);

	    /* We know that it's symmetric if it's square. */

	    if (shape->a.integer[0] == shape->a.integer[1])
	      ((MATRIX *) d)->symmetry = symmetric;

	    cnt = ((MATRIX *) d)->nn;

	    switch (v->type)
	      {
	      case integer:
		{
		  int *p = ((MATRIX *) d)->a.integer;
		  int val = v->v.integer;
		  for (i = 0; i < cnt; i++)
		    *p++ = val;
		  break;
		}
	      case real:
		{
		  REAL *p = ((MATRIX *) d)->a.real;
		  REAL val = v->v.real;
		  for (i = 0; i < cnt; i++)
		    *p++ = val;
		  break;
		}
	      case complex:
		{
		  COMPLEX *p, val;
		  p = ((MATRIX *) d)->a.complex;
		  val = v->v.complex;
		  for (i = 0; i < cnt; i++)
		    *p++ = val;
		  break;
		}
	      case character:
		{
		  char **p = ((MATRIX *) d)->a.character;
		  char *val = v->v.character;
		  for (i = 0; i < cnt; i++)
		    *p++ = dup_char (val);
		  break;
		}
	      default:

		wipeout ("Bad type.");
	      }

	    break;

	  default:

	    fail ("Invalid dimension (%d) for shape vector in \"fill\".",
		  shape->ne);
	    raise_exception ();

	  }
      }
    else
      {
	d = copy_scalar (v);
      }
  }

  ON_EXCEPTION
  {
    delete_vector (shape);
    delete_scalar (v);
    delete_entity (d);
  }
  END_EXCEPTION;

  delete_vector (shape);
  delete_scalar (v);
  return (d);
}

ENTITY *
fill_vector (VECTOR *shape, VECTOR *v)
{
  ENTITY *d = NULL;
  int cnt, i;
  int n = v->ne;

  WITH_HANDLING
  {
    v = (VECTOR *) dense_vector (EAT (v));

    if (shape)
      {
	shape = (VECTOR *) dense_vector (EAT (shape));
	shape = (VECTOR *) cast_vector (EAT (shape), integer);

	switch (shape->ne)
	  {
	  case 0:		/* scalar result */

	    d = make_scalar (v->type);
	    if (n)
	      {
		memcpy (&((SCALAR *) d)->v.integer, v->a.ptr,
			type_size[v->type]);
		if (v->type == character)
		  {
		    ((SCALAR *) d)->v.character =
		      dup_char (((SCALAR *) d)->v.character);
		  }
	      }
	    break;

	  case 1:		/* vector result */

	    cnt = shape->a.integer[0];

	    if (cnt < 0)
	      {
		fail ("Specified vector dimension is negative.");
		raise_exception ();
	      }

	    if (n)
	      {
		char *p, *a;
		int sz = type_size[v->type];
		int nb = n * sz;

		d = form_vector (cnt, v->type, dense);

		p = ((VECTOR *) d)->a.ptr;
		a = v->a.ptr;

		for (i = 0; i <= cnt - n; i += n)
		  {
		    memcpy (p, a, nb);
		    p += nb;
		  }
		if (i < cnt)
		  memcpy (p, a, (cnt - i) * sz);

		if (v->type == character)
		  {
		    char **pc = ((VECTOR *) d)->a.character;

		    for (i = 0; i < cnt; i++)
		      {
			*pc = dup_char (*pc);
			pc++;
		      }
		  }
	      }
	    else
	      {
		d = make_vector (cnt, v->type, dense);
	      }

	    break;

	  case 2:		/* matrix result */

	    if (shape->a.integer[0] < 0 || shape->a.integer[1] < 0)
	      {
		fail ("Specified matrix dimension is negative.");
		raise_exception ();
	      }

	    if (n)
	      {
		char *p, *a;
		int sz = type_size[v->type];
		int nb = n * sz;

		d = form_matrix (shape->a.integer[1], shape->a.integer[0],
				 v->type, dense);
		cnt = ((MATRIX *) d)->nn;

		p = ((MATRIX *) d)->a.ptr;
		a = v->a.ptr;

		for (i = 0; i <= cnt - n; i += n)
		  {
		    memcpy (p, a, nb);
		    p += nb;
		  }
		if (i < cnt)
		  memcpy (p, a, (cnt - i) * sz);

		if (v->type == character)
		  {
		    char **pc = ((MATRIX *) d)->a.character;

		    for (i = 0; i < cnt; i++)
		      {
			*pc = dup_char (*pc);
			pc++;
		      }
		  }

		d = transpose_matrix (EAT (d));
	      }
	    else
	      {
		d = make_matrix (shape->a.integer[0], shape->a.integer[1],
				 v->type, dense);
	      }

	    break;

	  default:

	    fail ("Invalid dimension (%d) for shape vector in \"fill\".",
		  shape->ne);
	    raise_exception ();

	  }
      }
    else
      {
	d = make_scalar (v->type);
	if (n)
	  {
	    memcpy (&((SCALAR *) d)->v.integer, v->a.ptr,
		    type_size[v->type]);
	    if (v->type == character)
	      {
		((SCALAR *) d)->v.character =
		  dup_char (((SCALAR *) d)->v.character);
	      }
	  }
      }
  }
  ON_EXCEPTION
  {
    delete_2_vectors (shape, v);
    delete_entity (d);
  }
  END_EXCEPTION;

  delete_2_vectors (shape, v);
  return (d);
}

ENTITY *
fill_matrix (VECTOR *shape, MATRIX *v)
{
  ENTITY *d = NULL;
  int cnt, i;
  int n;

  WITH_HANDLING
  {
    v = (MATRIX *) dense_matrix (EAT (v));
    v = (MATRIX *) transpose_matrix (EAT (v));
    n = v->nn;

    if (shape)
      {
	shape = (VECTOR *) dense_vector (EAT (shape));
	shape = (VECTOR *) cast_vector (EAT (shape), integer);

	switch (shape->ne)
	  {
	  case 0:		/* scalar result */

	    d = make_scalar (v->type);
	    if (n)
	      {
		memcpy (&((SCALAR *) d)->v.integer, v->a.ptr,
			type_size[v->type]);
		if (v->type == character)
		  {
		    ((SCALAR *) d)->v.character =
		      dup_char (((SCALAR *) d)->v.character);
		  }
	      }
	    break;

	  case 1:		/* vector result */

	    cnt = shape->a.integer[0];

	    if (cnt < 0)
	      {
		fail ("Specified vector dimension is negative.");
		raise_exception ();
	      }

	    if (n)
	      {
		char *p, *a;
		int sz = type_size[v->type];
		int nb = n * sz;

		d = form_vector (cnt, v->type, dense);

		p = ((VECTOR *) d)->a.ptr;
		a = v->a.ptr;

		for (i = 0; i <= cnt - n; i += n)
		  {
		    memcpy (p, a, nb);
		    p += nb;
		  }
		if (i < cnt)
		  memcpy (p, a, (cnt - i) * sz);

		if (v->type == character)
		  {
		    char **pc = ((VECTOR *) d)->a.character;

		    for (i = 0; i < cnt; i++)
		      {
			*pc = dup_char (*pc);
			pc++;
		      }
		  }
	      }
	    else
	      {
		d = make_vector (cnt, v->type, dense);
	      }

	    break;

	  case 2:		/* matrix result */

	    if (shape->a.integer[0] < 0 || shape->a.integer[1] < 0)
	      {
		fail ("Specified matrix dimension is negative.");
		raise_exception ();
	      }

	    if (n)
	      {
		char *p, *a;
		int sz = type_size[v->type];
		int nb = n * sz;

		d = form_matrix (shape->a.integer[1], shape->a.integer[0],
				 v->type, dense);
		cnt = ((MATRIX *) d)->nn;

		p = ((MATRIX *) d)->a.ptr;
		a = v->a.ptr;

		for (i = 0; i <= cnt - n; i += n)
		  {
		    memcpy (p, a, nb);
		    p += nb;
		  }
		if (i < cnt)
		  memcpy (p, a, (cnt - i) * sz);

		if (v->type == character)
		  {
		    char **pc = ((MATRIX *) d)->a.character;

		    for (i = 0; i < cnt; i++)
		      {
			*pc = dup_char (*pc);
			pc++;
		      }
		  }

		d = transpose_matrix (EAT (d));
	      }
	    else
	      {
		d = make_matrix (shape->a.integer[0], shape->a.integer[1],
				 v->type, dense);
	      }

	    break;

	  default:

	    fail ("Invalid dimension (%d) for shape vector in \"fill\".",
		  shape->ne);
	    raise_exception ();
	  }
      }
    else
      {
	d = make_scalar (v->type);
	if (n)
	  {
	    memcpy (&((SCALAR *) d)->v.integer, v->a.ptr,
		    type_size[v->type]);
	    if (v->type == character)
	      {
		((SCALAR *) d)->v.character =
		  dup_char (((SCALAR *) d)->v.character);
	      }
	  }
      }
  }

  ON_EXCEPTION
  {
    delete_vector (shape);
    delete_matrix (v);
    delete_entity (d);
  }
  END_EXCEPTION;

  delete_vector (shape);
  delete_matrix (v);
  return (d);
}
