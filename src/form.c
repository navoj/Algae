/*
   form.c -- form an array

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: form.c,v 1.4 2003/08/01 04:57:47 ksh Exp $";

#include "form.h"
#include "entity.h"
#include "datum.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "cast.h"
#include "transpose.h"
#include "dense.h"

ENTITY *
bi_form (ENTITY *shape, ENTITY *v)
{
  /*
   * This function returns an entity with the given `shape' that
   * contains the elements of `v'.  If `shape' is NULL or has zero
   * length, a scalar is returned.  If `shape' has length one, then
   * a vector with `shape[1]' elements is returned.  If `shape' has two
   * elements, then a matrix is returned with `shape[1]' rows and
   * `shape[2]' columns.  If `v' is a matrix, its elements are used
   * by rows.  If `v' doesn't contain enough elements to form the
   * result, it is padded with zeros or null strings.
   */

  ENTITY * volatile r = NULL;

  WITH_HANDLING
  {

    if (shape)
      shape = vector_entity (EAT (shape));

    switch (v->class)
      {

      case scalar:
	r = bi_form_scalar (EAT (shape), EAT (v));
	break;

      case vector:
	r = bi_form_vector (EAT (shape), EAT (v));
	break;

      case matrix:
	r = bi_form_matrix (EAT (shape), EAT (v));
	break;

      default:
	fail ("Can't use a %s in \"form\".", class_string[v->class]);
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
bi_form_scalar (VECTOR *shape, SCALAR *v)
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

	  case 0:
	    d = copy_scalar (v);
	    break;

	  case 1:
	    cnt = shape->a.integer[0];
	    if (cnt < 0)
	      {
		fail ("Specified vector dimension is negative.");
		raise_exception ();
	      }
	    d = make_vector (cnt, v->type, dense);
	    if (cnt > 1)
	      cnt = 1;
	    if (cnt)
	      memcpy (((VECTOR *) d)->a.ptr, &v->v.integer,
		      type_size[v->type]);
	    if (v->type == character)
	      {
		for (i = 0; i < cnt; i++)
		  ((VECTOR *) d)->a.character[i] =
		    dup_char (((VECTOR *) d)->a.character[i]);
	      }
	    break;

	  case 2:
	    if (shape->a.integer[0] < 0 || shape->a.integer[1] < 0)
	      {
		fail ("Specified matrix dimension is negative.");
		raise_exception ();
	      }
	    d = make_matrix (shape->a.integer[0], shape->a.integer[1],
			     v->type, dense);

	    /*
	     * Since it has only one nonzero element, we know that it's
	     * symmetric if it's square.
	     */

	    ((MATRIX *) d)->symmetry =
	      (shape->a.integer[0] == shape->a.integer[1]) ?
	      symmetric : general;

	    cnt = ((MATRIX *) d)->nn;
	    if (cnt > 1)
	      cnt = 1;
	    if (cnt)
	      memcpy (((MATRIX *) d)->a.ptr, &v->v.integer,
		      type_size[v->type]);
	    if (v->type == character)
	      {
		for (i = 0; i < cnt; i++)
		  ((MATRIX *) d)->a.character[i] =
		    dup_char (((MATRIX *) d)->a.character[i]);
	      }
	    break;

	  default:
	    fail ("Invalid dimension (%d) for shape vector in \"form\".",
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
bi_form_vector (VECTOR *shape, VECTOR *v)
{
  ENTITY *d = NULL;
  int cnt, i;

  WITH_HANDLING
  {

    v = (VECTOR *) dense_vector (EAT (v));

    if (shape)
      {

	shape = (VECTOR *) dense_vector (EAT (shape));
	shape = (VECTOR *) cast_vector (EAT (shape), integer);

	switch (shape->ne)
	  {

	  case 0:
	    d = make_scalar (v->type);
	    if (v->ne)
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

	  case 1:
	    cnt = shape->a.integer[0];
	    if (cnt < 0)
	      {
		fail ("Specified vector dimension is negative.");
		raise_exception ();
	      }
	    d = make_vector (cnt, v->type, dense);
	    if (cnt > v->ne)
	      cnt = v->ne;
	    if (cnt)
	      memcpy (((VECTOR *) d)->a.ptr, v->a.ptr,
		      cnt * type_size[v->type]);
	    if (v->type == character)
	      {
		for (i = 0; i < cnt; i++)
		  ((VECTOR *) d)->a.character[i] =
		    dup_char (((VECTOR *) d)->a.character[i]);
	      }
	    break;

	  case 2:
	    if (shape->a.integer[0] < 0 || shape->a.integer[1] < 0)
	      {
		fail ("Specified matrix dimension is negative.");
		raise_exception ();
	      }
	    d = make_matrix (shape->a.integer[1], shape->a.integer[0],
			     v->type, dense);
	    ((MATRIX *) d)->symmetry = general;
	    cnt = ((MATRIX *) d)->nn;
	    if (cnt > v->ne)
	      cnt = v->ne;
	    if (cnt)
	      memcpy (((MATRIX *) d)->a.ptr, v->a.ptr,
		      cnt * type_size[v->type]);
	    if (v->type == character)
	      {
		for (i = 0; i < cnt; i++)
		  ((MATRIX *) d)->a.character[i] =
		    dup_char (((MATRIX *) d)->a.character[i]);
	      }
	    d = transpose_matrix (EAT (d));
	    break;

	  default:
	    fail ("Invalid dimension (%d) for shape vector in \"form\".",
		  shape->ne);
	    raise_exception ();

	  }
      }
    else
      {
	d = make_scalar (v->type);
	if (v->ne)
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
bi_form_matrix (VECTOR *shape, MATRIX *v)
{
  ENTITY *d = NULL;
  int cnt, i;

  WITH_HANDLING
  {

    v = (MATRIX *) dense_matrix (EAT (v));
    v = (MATRIX *) transpose_matrix (EAT (v));

    if (shape)
      {

	shape = (VECTOR *) dense_vector (EAT (shape));
	shape = (VECTOR *) cast_vector (EAT (shape), integer);

	switch (shape->ne)
	  {

	  case 0:
	    d = make_scalar (v->type);
	    if (v->nn)
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

	  case 1:
	    cnt = shape->a.integer[0];
	    if (cnt < 0)
	      {
		fail ("Specified vector dimension is negative.");
		raise_exception ();
	      }
	    d = make_vector (cnt, v->type, dense);
	    if (cnt > v->nn)
	      cnt = v->nn;
	    if (cnt)
	      memcpy (((VECTOR *) d)->a.ptr, v->a.ptr,
		      cnt * type_size[v->type]);
	    if (v->type == character)
	      {
		for (i = 0; i < cnt; i++)
		  ((VECTOR *) d)->a.character[i] =
		    dup_char (((VECTOR *) d)->a.character[i]);
	      }
	    break;

	  case 2:
	    if (shape->a.integer[0] < 0 || shape->a.integer[1] < 0)
	      {
		fail ("Specified matrix dimension is negative.");
		raise_exception ();
	      }
	    d = make_matrix (shape->a.integer[1], shape->a.integer[0],
			     v->type, dense);
	    ((MATRIX *) d)->symmetry = general;
	    cnt = ((MATRIX *) d)->nn;
	    if (cnt > v->nn)
	      cnt = v->nn;
	    if (cnt)
	      memcpy (((MATRIX *) d)->a.ptr, v->a.ptr,
		      cnt * type_size[v->type]);
	    if (v->type == character)
	      {
		for (i = 0; i < cnt; i++)
		  ((MATRIX *) d)->a.character[i] =
		    dup_char (((MATRIX *) d)->a.character[i]);
	      }
	    d = transpose_matrix (EAT (d));
	    break;

	  default:
	    fail ("Invalid dimension (%d) for shape vector in \"form\".",
		  shape->ne);
	    raise_exception ();

	  }
      }
    else
      {
	d = make_scalar (v->type);
	if (v->nn)
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
