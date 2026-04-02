/*
   subtract.c -- Subtraction.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: subtract.c,v 1.4 2003/08/01 04:57:48 ksh Exp $";

#include "subtract.h"
#include "add.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "negate.h"
#include "thin.h"
#include "dense.h"
#include "full.h"
#include "sparse.h"

static char bad_labels[] = "Labels don't match for subtraction.";
static char incon_vector[] = "Inconsistent dimensions for vector subtraction.  First vector has %d element%s; second vector has %d element%s.";
static char incon_matrix[] = "Inconsistent dimensions for matrix subtraction.  First matrix has %d row%s and %d column%s; second matrix has %d row%s and %d column%s.";

ENTITY *
subtract_scalar_vector_integer (SCALAR *l, VECTOR *r)
{
  VECTOR *v = NULL;
  int i;

  EASSERT (l, scalar, integer);
  EASSERT (r, vector, integer);

  if (l->v.integer == 0)
    {
      delete_scalar (l);
      return (negate_vector (r));
    }

  WITH_HANDLING
  {
    v = (VECTOR *) negate_vector ((VECTOR *) EAT (r));
    v = (VECTOR *) dense_vector ((VECTOR *) EAT (v));
    assert (v->entity.ref_count == 1);
    for (i = 0; i < v->nn; i++)
      {
	v->a.integer[i] += l->v.integer;
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (ENT (l), ENT (r), ENT (v));
  }
  END_EXCEPTION;

  delete_scalar (l);
  return (ENT (v));
}

ENTITY *
subtract_vector_scalar_integer (VECTOR *l, SCALAR *r)
{
  VECTOR *v = NULL;
  int i;

  EASSERT (l, vector, integer);
  EASSERT (r, scalar, integer);

  if (r->v.integer == 0)
    {
      delete_scalar (r);
      return (ENT (l));
    }

  WITH_HANDLING
  {
    v = (VECTOR *) dup_vector ((VECTOR *) EAT (l));
    v = (VECTOR *) dense_vector ((VECTOR *) EAT (v));
    assert (v->entity.ref_count == 1);
    for (i = 0; i < v->nn; i++)
      {
	v->a.integer[i] -= r->v.integer;
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (ENT (l), ENT (r), ENT (v));
  }
  END_EXCEPTION;

  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
subtract_scalar_vector_real (SCALAR *l, VECTOR *r)
{
  VECTOR *v = NULL;
  int i;

  EASSERT (l, scalar, real);
  EASSERT (r, vector, real);

  if (l->v.real == 0.0)
    {
      delete_scalar (l);
      return (negate_vector (r));
    }

  WITH_HANDLING
  {
    v = (VECTOR *) negate_vector ((VECTOR *) EAT (r));
    v = (VECTOR *) dense_vector ((VECTOR *) EAT (v));
    assert (v->entity.ref_count == 1);
    for (i = 0; i < v->nn; i++)
      {
	v->a.real[i] += l->v.real;
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (ENT (l), ENT (r), ENT (v));
  }
  END_EXCEPTION;

  delete_scalar (l);
  return (ENT (v));
}

ENTITY *
subtract_vector_scalar_real (VECTOR *l, SCALAR *r)
{
  VECTOR *v = NULL;
  int i;

  EASSERT (l, vector, real);
  EASSERT (r, scalar, real);

  if (r->v.real == 0.0)
    {
      delete_scalar (r);
      return (ENT (l));
    }

  WITH_HANDLING
  {
    v = (VECTOR *) dup_vector ((VECTOR *) EAT (l));
    v = (VECTOR *) dense_vector ((VECTOR *) EAT (v));
    assert (v->entity.ref_count == 1);
    for (i = 0; i < v->nn; i++)
      {
	v->a.real[i] -= r->v.real;
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (ENT (l), ENT (r), ENT (v));
  }
  END_EXCEPTION;

  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
subtract_scalar_vector_complex (SCALAR *l, VECTOR *r)
{
  VECTOR *v = NULL;
  int i;

  EASSERT (l, scalar, complex);
  EASSERT (r, vector, complex);

  if (l->v.complex.real == 0.0 && l->v.complex.imag == 0.0)
    {
      delete_scalar (l);
      return (negate_vector (r));
    }

  WITH_HANDLING
  {
    v = (VECTOR *) negate_vector ((VECTOR *) EAT (r));
    v = (VECTOR *) dense_vector ((VECTOR *) EAT (v));
    assert (v->entity.ref_count == 1);
    for (i = 0; i < v->nn; i++)
      {
	v->a.complex[i].real += l->v.complex.real;
	v->a.complex[i].imag += l->v.complex.imag;
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (ENT (l), ENT (r), ENT (v));
  }
  END_EXCEPTION;

  delete_scalar (l);
  return (ENT (v));
}

ENTITY *
subtract_vector_scalar_complex (VECTOR *l, SCALAR *r)
{
  VECTOR *v = NULL;
  int i;

  EASSERT (l, vector, complex);
  EASSERT (r, scalar, complex);

  if (r->v.complex.real == 0.0 && r->v.complex.imag == 0.0)
    {
      delete_scalar (r);
      return (ENT (l));
    }

  WITH_HANDLING
  {
    v = (VECTOR *) dup_vector ((VECTOR *) EAT (l));
    v = (VECTOR *) dense_vector ((VECTOR *) EAT (v));
    assert (v->entity.ref_count == 1);
    for (i = 0; i < v->nn; i++)
      {
	v->a.complex[i].real -= r->v.complex.real;
	v->a.complex[i].imag -= r->v.complex.imag;
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (ENT (l), ENT (r), ENT (v));
  }
  END_EXCEPTION;

  delete_scalar (r);
  return (ENT (v));
}

ENTITY *
subtract_vector_integer (VECTOR *l, VECTOR *r)
{
  ENTITY * volatile eid = NULL;
  VECTOR * volatile s = NULL;
  int i;
  void *x_tmp;
  int l_ia[2], r_ia[2], s_ia[2];

  EASSERT (l, vector, integer);
  EASSERT (r, vector, integer);

  WITH_HANDLING
  {
    if (l->ne != r->ne)
      {
	fail (incon_vector, l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
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

    switch (l->density)
      {
      case dense:
	switch (r->density)
	  {
	  case dense:
	    s = (VECTOR *) dup_vector ((VECTOR *) EAT (l));
	    for (i = 0; i < s->nn; i++)
	      s->a.integer[i] -= r->a.integer[i];
	    break;
	  case sparse:
	    /*
	     * This is an inefficient shortcut.  We should
	     * eventually write code to do this directly.
	     */
	    s = (VECTOR *) subtract_vector_integer (
						     (VECTOR *) EAT (l),
			      (VECTOR *) dense_vector ((VECTOR *) EAT (r)));
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      case sparse:
	if (l->nn == 0)
	  {
	    s = (VECTOR *) negate_vector ((VECTOR *) EAT (r));
	    break;
	  }
	switch (r->density)
	  {
	  case dense:
	    s = (VECTOR *) subtract_vector_integer (
			       (VECTOR *) dense_vector ((VECTOR *) EAT (l)),
						     (VECTOR *) EAT (r));
	    break;
	  case sparse:

	    if (r->nn == 0)
	      {
		s = (VECTOR *) copy_vector (l);
		break;
	      }

	    s = add_symbolic_vector ((VECTOR *) copy_vector (l),
				     (VECTOR *) copy_vector (r));

	    l_ia[0] = r_ia[0] = s_ia[0] = 1;
	    l_ia[1] = l->nn + 1;
	    r_ia[1] = r->nn + 1;
	    s_ia[1] = s->nn + 1;
	    x_tmp = CALLOC (s->ne, sizeof (int));

	    IGSSUB (l_ia, l->ja, l->a.integer,
		    r_ia, r->ja, r->a.integer,
		    s_ia, &s->ne,
		    s_ia, s->ja, s->a.integer,
		    x_tmp);
	    FREE (x_tmp);

	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
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
    delete_3_vectors (l, r, s);
  }
  END_EXCEPTION;

  delete_entity (s->eid);
  s->eid = eid;

  delete_2_vectors (l, r);
  return (ENT (s));
}

ENTITY *
subtract_vector_real (VECTOR *l, VECTOR *r)
{
  ENTITY * volatile eid = NULL;
  VECTOR * volatile s = NULL;
  int i;
  void *x_tmp;
  int l_ia[2], r_ia[2], s_ia[2];

  EASSERT (l, vector, real);
  EASSERT (r, vector, real);

  WITH_HANDLING
  {
    if (l->ne != r->ne)
      {
	fail (incon_vector, l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
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

    switch (l->density)
      {
      case dense:
	switch (r->density)
	  {
	  case dense:
	    s = (VECTOR *) dup_vector ((VECTOR *) EAT (l));
	    for (i = 0; i < s->nn; i++)
	      s->a.real[i] -= r->a.real[i];
	    break;
	  case sparse:
	    /*
	     * This is an inefficient shortcut.  We should
	     * eventually write code to do this directly.
	     */
	    s = (VECTOR *) subtract_vector_real (
						  (VECTOR *) EAT (l),
			      (VECTOR *) dense_vector ((VECTOR *) EAT (r)));
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      case sparse:
	if (l->nn == 0)
	  {
	    s = (VECTOR *) negate_vector ((VECTOR *) EAT (r));
	    break;
	  }
	switch (r->density)
	  {
	  case dense:
	    s = (VECTOR *) subtract_vector_real (
			       (VECTOR *) dense_vector ((VECTOR *) EAT (l)),
						  (VECTOR *) EAT (r));
	    break;

	  case sparse:

	    if (r->nn == 0)
	      {
		s = (VECTOR *) copy_vector (l);
		break;
	      }

	    s = add_symbolic_vector ((VECTOR *) copy_vector (l),
				     (VECTOR *) copy_vector (r));

	    l_ia[0] = r_ia[0] = s_ia[0] = 1;
	    l_ia[1] = l->nn + 1;
	    r_ia[1] = r->nn + 1;
	    s_ia[1] = s->nn + 1;
	    x_tmp = CALLOC (s->ne, sizeof (REAL));
	    DGSSUB (l_ia, l->ja, l->a.real,
		    r_ia, r->ja, r->a.real,
		    s_ia, &s->ne,
		    s_ia, s->ja, s->a.real,
		    x_tmp);
	    FREE (x_tmp);

	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
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
    delete_3_vectors (l, r, s);
  }
  END_EXCEPTION;

  delete_entity (s->eid);
  s->eid = eid;

  delete_2_vectors (l, r);
  return (ENT (s));
}

ENTITY *
subtract_vector_complex (VECTOR *l, VECTOR *r)
{
  ENTITY * volatile eid = NULL;
  VECTOR * volatile s = NULL;
  int i;
  void *x_tmp;
  int l_ia[2], r_ia[2], s_ia[2];

  EASSERT (l, vector, complex);
  EASSERT (r, vector, complex);

  WITH_HANDLING
  {
    if (l->ne != r->ne)
      {
	fail (incon_vector, l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
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

    switch (l->density)
      {
      case dense:
	switch (r->density)
	  {
	  case dense:
	    s = (VECTOR *) dup_vector ((VECTOR *) EAT (l));
	    for (i = 0; i < s->nn; i++)
	      {
		s->a.complex[i].real -= r->a.complex[i].real;
		s->a.complex[i].imag -= r->a.complex[i].imag;
	      }
	    break;
	  case sparse:
	    /*
	     * This is an inefficient shortcut.  We should
	     * eventually write code to do this directly.
	     */
	    s = (VECTOR *) subtract_vector_complex (
						     (VECTOR *) EAT (l),
			      (VECTOR *) dense_vector ((VECTOR *) EAT (r)));
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      case sparse:
	if (l->nn == 0)
	  {
	    s = (VECTOR *) negate_vector ((VECTOR *) EAT (r));
	    break;
	  }
	switch (r->density)
	  {
	  case dense:
	    s = (VECTOR *) subtract_vector_complex (
			       (VECTOR *) dense_vector ((VECTOR *) EAT (l)),
						     (VECTOR *) EAT (r));
	    break;

	  case sparse:

	    if (r->nn == 0)
	      {
		s = (VECTOR *) copy_vector (l);
		break;
	      }

	    s = add_symbolic_vector ((VECTOR *) copy_vector (l),
				     (VECTOR *) copy_vector (r));

	    l_ia[0] = r_ia[0] = s_ia[0] = 1;
	    l_ia[1] = l->nn + 1;
	    r_ia[1] = r->nn + 1;
	    s_ia[1] = s->nn + 1;

	    x_tmp = CALLOC (s->ne, sizeof (COMPLEX));
	    ZGSSUB (l_ia, l->ja, l->a.complex,
		    r_ia, r->ja, r->a.complex,
		    s_ia, &s->ne,
		    s_ia, s->ja, s->a.complex,
		    x_tmp);
	    FREE (x_tmp);

	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
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
    delete_3_vectors (l, r, s);
  }
  END_EXCEPTION;

  delete_entity (s->eid);
  s->eid = eid;

  delete_2_vectors (l, r);
  return (ENT (s));
}

ENTITY *
subtract_scalar_matrix_integer (SCALAR *l, MATRIX *r)
{
  MATRIX *m = NULL;
  int i;

  EASSERT (l, scalar, integer);
  EASSERT (r, matrix, integer);

  if (l->v.integer == 0)
    {
      delete_scalar (l);
      return (negate_matrix (r));
    }

  WITH_HANDLING
  {
    m = (MATRIX *) negate_matrix ((MATRIX *) EAT (r));
    m = (MATRIX *) dense_matrix ((MATRIX *) EAT (m));
    assert (m->entity.ref_count == 1);
    for (i = 0; i < m->nn; i++)
      {
	m->a.integer[i] += l->v.integer;
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (ENT (l), ENT (r), ENT (m));
  }
  END_EXCEPTION;

  delete_scalar (l);
  return (ENT (m));
}

ENTITY *
subtract_matrix_scalar_integer (MATRIX *l, SCALAR *r)
{
  MATRIX *m = NULL;
  int i;

  EASSERT (l, matrix, integer);
  EASSERT (r, scalar, integer);

  if (r->v.integer == 0)
    {
      delete_scalar (r);
      return (ENT (l));
    }

  WITH_HANDLING
  {
    m = (MATRIX *) dup_matrix ((MATRIX *) EAT (l));
    m = (MATRIX *) dense_matrix ((MATRIX *) EAT (m));
    assert (m->entity.ref_count == 1);
    for (i = 0; i < m->nn; i++)
      {
	m->a.integer[i] -= r->v.integer;
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (ENT (l), ENT (r), ENT (m));
  }
  END_EXCEPTION;

  delete_scalar (r);
  return (ENT (m));
}

ENTITY *
subtract_scalar_matrix_real (SCALAR *l, MATRIX *r)
{
  MATRIX *m = NULL;
  int i;

  EASSERT (l, scalar, real);
  EASSERT (r, matrix, real);

  if (l->v.real == 0.0)
    {
      delete_scalar (l);
      return (negate_matrix (r));
    }

  WITH_HANDLING
  {
    if (r->density == sparse_upper && r->symmetry != general)
      {
	r = (MATRIX *) full_matrix ((MATRIX *) EAT (r));
      }
    m = (MATRIX *) negate_matrix ((MATRIX *) EAT (r));
    m = (MATRIX *) dense_matrix ((MATRIX *) EAT (m));
    assert (m->entity.ref_count == 1);

    for (i = 0; i < m->nn; i++)
      {
	m->a.real[i] += l->v.real;
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (ENT (l), ENT (r), ENT (m));
  }
  END_EXCEPTION;

  delete_scalar (l);
  return (ENT (m));
}

ENTITY *
subtract_matrix_scalar_real (MATRIX *l, SCALAR *r)
{
  MATRIX *m = NULL;
  int i;

  EASSERT (l, matrix, real);
  EASSERT (r, scalar, real);

  if (r->v.real == 0.0)
    {
      delete_scalar (r);
      return (ENT (l));
    }

  WITH_HANDLING
  {
    m = (MATRIX *) dup_matrix ((MATRIX *) EAT (l));
    m = (MATRIX *) dense_matrix ((MATRIX *) EAT (m));
    assert (m->entity.ref_count == 1);
    for (i = 0; i < m->nn; i++)
      {
	m->a.real[i] -= r->v.real;
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (ENT (l), ENT (r), ENT (m));
  }
  END_EXCEPTION;

  delete_scalar (r);
  return (ENT (m));
}

ENTITY *
subtract_scalar_matrix_complex (SCALAR *l, MATRIX *r)
{
  MATRIX *m = NULL;
  int i;

  EASSERT (l, scalar, complex);
  EASSERT (r, matrix, complex);

  if (l->v.complex.real == 0.0 && l->v.complex.imag == 0.0)
    {
      delete_scalar (l);
      return (negate_matrix (r));
    }

  WITH_HANDLING
  {
    int dgnl = r->nn == 0 &&
      (r->symmetry == symmetric || r->symmetry == hermitian);

    if (r->density == sparse_upper && r->symmetry != general)
      {
	r = (MATRIX *) full_matrix ((MATRIX *) EAT (r));
      }
    m = (MATRIX *) negate_matrix ((MATRIX *) EAT (r));
    m = (MATRIX *) dense_matrix ((MATRIX *) EAT (m));
    assert (m->entity.ref_count == 1);

    /*
     * If it's hermitian, then it goes to symmetric if it's diagonal
     * or general otherwise.
     */

    if (m->symmetry == hermitian && l->v.complex.imag != 0.0)
      m->symmetry = dgnl ? symmetric : general;

    for (i = 0; i < m->nn; i++)
      {
	m->a.complex[i].real += l->v.complex.real;
	m->a.complex[i].imag += l->v.complex.imag;
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (ENT (l), ENT (r), ENT (m));
  }
  END_EXCEPTION;

  delete_scalar (l);
  return (ENT (m));
}

ENTITY *
subtract_matrix_scalar_complex (MATRIX *l, SCALAR *r)
{
  MATRIX *m = NULL;
  int i;

  EASSERT (l, matrix, complex);
  EASSERT (r, scalar, complex);

  if (r->v.complex.real == 0.0 && r->v.complex.imag == 0.0)
    {
      delete_scalar (r);
      return (ENT (l));
    }

  WITH_HANDLING
  {
    int dgnl = l->nn == 0 &&
      (l->symmetry == symmetric || l->symmetry == hermitian);

    m = (MATRIX *) dup_matrix ((MATRIX *) EAT (l));
    m = (MATRIX *) dense_matrix ((MATRIX *) EAT (m));
    assert (m->entity.ref_count == 1);

    /*
     * If it's hermitian, then it goes to symmetric if it's diagonal
     * or general otherwise.
     */

    if (m->symmetry == hermitian && r->v.complex.imag != 0.0)
      m->symmetry = dgnl ? symmetric : general;

    for (i = 0; i < m->nn; i++)
      {
	m->a.complex[i].real -= r->v.complex.real;
	m->a.complex[i].imag -= r->v.complex.imag;
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (ENT (l), ENT (r), ENT (m));
  }
  END_EXCEPTION;

  delete_scalar (r);
  return (ENT (m));
}

ENTITY *
subtract_vector_matrix_integer (VECTOR *l, MATRIX *r)
{
  return (subtract_matrix_integer ((MATRIX *) vector_to_matrix (l), r));
}

ENTITY *
subtract_matrix_vector_integer (MATRIX *l, VECTOR *r)
{
  return (subtract_matrix_integer (l, (MATRIX *) vector_to_matrix (r)));
}

ENTITY *
subtract_vector_matrix_real (VECTOR *l, MATRIX *r)
{
  return (subtract_matrix_real ((MATRIX *) vector_to_matrix (l), r));
}

ENTITY *
subtract_matrix_vector_real (MATRIX *l, VECTOR *r)
{
  return (subtract_matrix_real (l, (MATRIX *) vector_to_matrix (r)));
}

ENTITY *
subtract_vector_matrix_complex (VECTOR *l, MATRIX *r)
{
  return (subtract_matrix_complex ((MATRIX *) vector_to_matrix (l), r));
}

ENTITY *
subtract_matrix_vector_complex (MATRIX *l, VECTOR *r)
{
  return (subtract_matrix_complex (l, (MATRIX *) vector_to_matrix (r)));
}

ENTITY *
subtract_matrix_integer (MATRIX *l, MATRIX *r)
{
  ENTITY * volatile rid = NULL, * volatile cid = NULL;
  MATRIX * volatile s = NULL;
  int i;
  void *x_tmp;

  EASSERT (l, matrix, integer);
  EASSERT (r, matrix, integer);

  WITH_HANDLING
  {
    if (l->nr != r->nr || l->nc != r->nc)
      {
	fail (incon_matrix, l->nr, PLURAL (l->nr), l->nc, PLURAL (l->nc),
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

    switch (l->density)
      {
      case dense:
	switch (r->density)
	  {
	  case dense:
	    s = (MATRIX *) dup_matrix ((MATRIX *) EAT (l));
	    if (s->symmetry != r->symmetry)
	      s->symmetry = general;
	    for (i = 0; i < s->nn; i++)
	      s->a.integer[i] -= r->a.integer[i];
	    break;
	  case sparse:
	  case sparse_upper:
	    /*
	     * This is an inefficient shortcut.  We should eventually
	     * write code to do this directly.
	     */
	    s = (MATRIX *) subtract_matrix_integer ((MATRIX *) EAT (l),
			      (MATRIX *) dense_matrix ((MATRIX *) EAT (r)));
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      case sparse:
	if (l->nn == 0)
	  {
	    s = (MATRIX *) negate_matrix ((MATRIX *) EAT (r));
	    break;
	  }
	switch (r->density)
	  {
	  case dense:
	    s = (MATRIX *) subtract_matrix_integer (
			       (MATRIX *) dense_matrix ((MATRIX *) EAT (l)),
						     (MATRIX *) EAT (r));
	    break;

	  case sparse:

	    if (r->nn == 0)
	      {
		s = (MATRIX *) copy_matrix (l);
		break;
	      }

	    s = add_symbolic_matrix ((MATRIX *) copy_matrix (l),
				     (MATRIX *) copy_matrix (r));

	    x_tmp = CALLOC (s->nc, sizeof (int));
	    IGSSUB (l->ia, l->ja, l->a.integer,
		    r->ia, r->ja, r->a.integer,
		    &s->nr, &s->nc,
		    s->ia, s->ja, s->a.integer,
		    x_tmp);
	    FREE (x_tmp);

	    break;
	  case sparse_upper:
	    if (l->symmetry == r->symmetry)
	      {
		/*
		 * They have the same symmetry, so convert to the
		 * sparse_upper form.
		 */
		s = (MATRIX *) subtract_matrix_integer (
			      (MATRIX *) sparse_matrix ((MATRIX *) EAT (l)),
							 (MATRIX *) EAT (r));
	      }
	    else
	      {
		/* They have different symmetry, so convert to full. */
		s = (MATRIX *) subtract_matrix_integer ((MATRIX *) EAT (l),
			       (MATRIX *) full_matrix ((MATRIX *) EAT (r)));
	      }
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      case sparse_upper:
	switch (r->density)
	  {
	  case dense:
	    s = (MATRIX *) subtract_matrix_integer (
			       (MATRIX *) dense_matrix ((MATRIX *) EAT (l)),
						     (MATRIX *) EAT (r));
	    break;
	  case sparse:
	    if (r->nn == 0)
	      {
		s = (MATRIX *) copy_matrix (l);
		break;
	      }

	    if (l->symmetry == r->symmetry)
	      {
		/*
		 * They have the same symmetry, but are stored differently.
		 * Convert to the sparse_upper form.
		 */
		s = (MATRIX *) subtract_matrix_integer (
							 (MATRIX *) EAT (l),
			     (MATRIX *) sparse_matrix ((MATRIX *) EAT (r)));
	      }
	    else
	      {
		/* They have different symmetry, so convert to full. */
		s = (MATRIX *) subtract_matrix_integer (
				(MATRIX *) full_matrix ((MATRIX *) EAT (l)),
							 (MATRIX *) EAT (r));
	      }
	    break;
	  case sparse_upper:
	    if (l->symmetry == r->symmetry)
	      {
		if (l->nn == 0)
		  {
		    s = (MATRIX *) negate_matrix ((MATRIX *) EAT (r));
		    if (l->d.integer == NULL)
		      break;

		    if (s->d.integer == NULL)
		      s->d.integer =
			(int *) CALLOC (s->nr, sizeof (int));
		    for (i = 0; i < s->nr; i++)
		      s->d.integer[i] += l->d.integer[i];
		  }
		else if (r->nn == 0)
		  {
		    if (r->d.integer == NULL)
		      {
			s = (MATRIX *) EAT (l);
			break;
		      }
		    s = (MATRIX *) dup_matrix ((MATRIX *) EAT (l));

		    if (s->d.integer == NULL)
		      s->d.integer =
			(int *) CALLOC (s->nr, sizeof (int));
		    for (i = 0; i < s->nr; i++)
		      s->d.integer[i] -= r->d.integer[i];
		  }
		else
		  {

		    s = add_symbolic_matrix ((MATRIX *) copy_matrix (l),
					     (MATRIX *) copy_matrix (r));

		    x_tmp = CALLOC (s->nc, sizeof (int));
		    IGSSUB (l->ia, l->ja, l->a.integer,
			    r->ia, r->ja, r->a.integer,
			    &s->nr, &s->nc,
			    s->ia, s->ja, s->a.integer,
			    x_tmp);
		    FREE (x_tmp);

		    if (l->d.integer != NULL)
		      {
			s->d.integer =
			  dup_mem (l->d.integer, s->nr * sizeof (int));
			if (r->d.integer != NULL)
			  {
			    for (i = 0; i < s->nr; i++)
			      s->d.integer[i] -= r->d.integer[i];
			  }
		      }
		    else if (r->d.integer != NULL)
		      {
			s->d.integer = dup_mem (r->d.integer,
						s->nr * sizeof (int));
			for (i = 0; i < s->nr; i++)
			  s->d.integer[i] = -s->d.integer[i];
		      }
		  }
	      }
	    else
	      {

		/*
		 * They have different symmetry, so we'll change them
		 * to sparse.
		 */

		s = (MATRIX *) subtract_matrix_integer (
				(MATRIX *) full_matrix ((MATRIX *) EAT (l)),
			       (MATRIX *) full_matrix ((MATRIX *) EAT (r)));
	      }
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      default:
	BAD_DENSITY (l->density);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_2_entities (rid, cid);
    delete_3_matrices (l, r, s);
  }
  END_EXCEPTION;

  delete_2_entities (s->rid, s->cid);
  s->rid = rid;
  s->cid = cid;

  delete_2_matrices (l, r);
  return (ENT (s));
}

ENTITY *
subtract_matrix_real (MATRIX *l, MATRIX *r)
{
  ENTITY * volatile rid = NULL, * volatile cid = NULL;
  MATRIX * volatile s = NULL;
  int i;
  void *x_tmp;

  EASSERT (l, matrix, real);
  EASSERT (r, matrix, real);

  WITH_HANDLING
  {
    if (l->nr != r->nr || l->nc != r->nc)
      {
	fail (incon_matrix, l->nr, PLURAL (l->nr), l->nc, PLURAL (l->nc),
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

    switch (l->density)
      {
      case dense:
	switch (r->density)
	  {
	  case dense:
	    s = (MATRIX *) dup_matrix ((MATRIX *) EAT (l));
	    if (s->symmetry != r->symmetry)
	      s->symmetry = general;
	    for (i = 0; i < s->nn; i++)
	      s->a.real[i] -= r->a.real[i];
	    break;
	  case sparse:
	  case sparse_upper:
	    /*
	     * This is an inefficient shortcut.  We should eventually
	     * write code to do this directly.
	     */
	    s = (MATRIX *) subtract_matrix_real ((MATRIX *) EAT (l),
			      (MATRIX *) dense_matrix ((MATRIX *) EAT (r)));
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      case sparse:
	if (l->nn == 0)
	  {
	    s = (MATRIX *) negate_matrix ((MATRIX *) EAT (r));
	    break;
	  }
	switch (r->density)
	  {
	  case dense:
	    s = (MATRIX *) subtract_matrix_real (
			       (MATRIX *) dense_matrix ((MATRIX *) EAT (l)),
						  (MATRIX *) EAT (r));
	    break;

	  case sparse:

	    if (r->nn == 0)
	      {
		s = (MATRIX *) copy_entity (ENT (l));
		break;
	      }

	    s = add_symbolic_matrix ((MATRIX *) copy_matrix (l),
				     (MATRIX *) copy_matrix (r));

	    x_tmp = CALLOC (s->nc, sizeof (REAL));
	    DGSSUB (l->ia, l->ja, l->a.real,
		    r->ia, r->ja, r->a.real,
		    &s->nr, &s->nc,
		    s->ia, s->ja, s->a.real,
		    x_tmp);
	    FREE (x_tmp);

	    break;

	  case sparse_upper:

	    if (l->symmetry == r->symmetry)
	      {
		/*
		 * They have the same symmetry, so convert to the
		 * sparse_upper form.
		 */
		s = (MATRIX *) subtract_matrix_real (
			      (MATRIX *) sparse_matrix ((MATRIX *) EAT (l)),
						      (MATRIX *) EAT (r));
	      }
	    else
	      {
		/* They have different symmetry, so convert to full. */
		s = (MATRIX *) subtract_matrix_real ((MATRIX *) EAT (l),
			       (MATRIX *) full_matrix ((MATRIX *) EAT (r)));
	      }
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      case sparse_upper:
	switch (r->density)
	  {
	  case dense:
	    s = (MATRIX *) subtract_matrix_real (
			       (MATRIX *) dense_matrix ((MATRIX *) EAT (l)),
						  (MATRIX *) EAT (r));
	    break;
	  case sparse:
	    if (r->nn == 0)
	      {
		s = (MATRIX *) copy_matrix (l);
		break;
	      }

	    if (l->symmetry == r->symmetry)
	      {
		/*
		 * They have the same symmetry, but are stored differently.
		 * Convert to the sparse_upper form.
		 */
		s = (MATRIX *) subtract_matrix_real ((MATRIX *) EAT (l),
			     (MATRIX *) sparse_matrix ((MATRIX *) EAT (r)));
	      }
	    else
	      {
		/* They have different symmetry, so convert to full. */
		s = (MATRIX *) subtract_matrix_real (
				(MATRIX *) full_matrix ((MATRIX *) EAT (l)),
						      (MATRIX *) EAT (r));
	      }
	    break;
	  case sparse_upper:
	    if (l->symmetry == r->symmetry)
	      {
		if (l->nn == 0)
		  {
		    s = (MATRIX *) negate_matrix ((MATRIX *) EAT (r));
		    if (l->d.real == NULL)
		      break;

		    if (s->d.real == NULL)
		      s->d.real = (REAL *) CALLOC (s->nr, sizeof (REAL));
		    for (i = 0; i < s->nr; i++)
		      s->d.real[i] += l->d.real[i];
		  }
		else if (r->nn == 0)
		  {
		    if (r->d.real == NULL)
		      {
			s = (MATRIX *) EAT (l);
			break;
		      }
		    s = (MATRIX *) dup_matrix ((MATRIX *) EAT (l));

		    if (s->d.real == NULL)
		      s->d.real = (REAL *) CALLOC (s->nr, sizeof (REAL));
		    for (i = 0; i < s->nr; i++)
		      s->d.real[i] -= r->d.real[i];
		  }
		else
		  {

		    s = add_symbolic_matrix ((MATRIX *) copy_matrix (l),
					     (MATRIX *) copy_matrix (r));

		    x_tmp = CALLOC (s->nc, sizeof (REAL));
		    DGSSUB (l->ia, l->ja, l->a.real,
			    r->ia, r->ja, r->a.real,
			    &s->nr, &s->nc,
			    s->ia, s->ja, s->a.real,
			    x_tmp);
		    FREE (x_tmp);

		    if (l->d.real != NULL)
		      {
			s->d.real =
			  dup_mem (l->d.real, s->nr * sizeof (REAL));
			if (r->d.real != NULL)
			  {
			    for (i = 0; i < s->nr; i++)
			      s->d.real[i] -= r->d.real[i];
			  }
		      }
		    else if (r->d.real != NULL)
		      {
			s->d.real =
			  dup_mem (r->d.real, s->nr * sizeof (REAL));
			for (i = 0; i < s->nr; i++)
			  s->d.real[i] = -s->d.real[i];
		      }
		  }
	      }
	    else
	      {

		/*
		 * They have different symmetry, so we'll change them
		 * to sparse.
		 */

		s = (MATRIX *) subtract_matrix_real (
				(MATRIX *) full_matrix ((MATRIX *) EAT (l)),
			       (MATRIX *) full_matrix ((MATRIX *) EAT (r)));
	      }
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      default:
	BAD_DENSITY (l->density);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_2_entities (rid, cid);
    delete_3_matrices (l, r, s);
  }
  END_EXCEPTION;

  delete_2_entities (s->rid, s->cid);
  s->rid = rid;
  s->cid = cid;

  delete_2_matrices (l, r);
  return (ENT (s));
}

ENTITY *
subtract_matrix_complex (MATRIX *l, MATRIX *r)
{
  ENTITY * volatile rid = NULL, * volatile cid = NULL;
  MATRIX * volatile s = NULL;
  int i;
  void *x_tmp;

  EASSERT (l, matrix, complex);
  EASSERT (r, matrix, complex);

  WITH_HANDLING
  {
    if (l->nr != r->nr || l->nc != r->nc)
      {
	fail (incon_matrix, l->nr, PLURAL (l->nr), l->nc, PLURAL (l->nc),
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

    switch (l->density)
      {
      case dense:
	switch (r->density)
	  {
	  case dense:
	    s = (MATRIX *) dup_matrix ((MATRIX *) EAT (l));
	    if (s->symmetry != r->symmetry)
	      s->symmetry = general;
	    for (i = 0; i < s->nn; i++)
	      {
		s->a.complex[i].real -= r->a.complex[i].real;
		s->a.complex[i].imag -= r->a.complex[i].imag;
	      }
	    break;
	  case sparse:
	  case sparse_upper:
	    /*
	     * This is an inefficient shortcut.  We should eventually
	     * write code to do this directly.
	     */
	    s = (MATRIX *) subtract_matrix_complex ((MATRIX *) EAT (l),
			      (MATRIX *) dense_matrix ((MATRIX *) EAT (r)));
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      case sparse:
	if (l->nn == 0)
	  {
	    s = (MATRIX *) negate_matrix ((MATRIX *) EAT (r));
	    break;
	  }
	switch (r->density)
	  {
	  case dense:
	    s = (MATRIX *) subtract_matrix_complex (
			       (MATRIX *) dense_matrix ((MATRIX *) EAT (l)),
						     (MATRIX *) EAT (r));
	    break;

	  case sparse:

	    if (r->nn == 0)
	      {
		s = (MATRIX *) copy_matrix (l);
		break;
	      }

	    s = add_symbolic_matrix ((MATRIX *) copy_matrix (l),
				     (MATRIX *) copy_matrix (r));

	    x_tmp = CALLOC (s->nc, sizeof (COMPLEX));
	    ZGSSUB (l->ia, l->ja, l->a.complex,
		    r->ia, r->ja, r->a.complex,
		    &s->nr, &s->nc,
		    s->ia, s->ja, s->a.complex,
		    x_tmp);
	    FREE (x_tmp);

	    break;

	  case sparse_upper:

	    if (l->symmetry == r->symmetry)
	      {
		/*
		 * They have the same symmetry, so convert to the
		 * sparse_upper form.
		 */
		s = (MATRIX *) subtract_matrix_complex (
			      (MATRIX *) sparse_matrix ((MATRIX *) EAT (l)),
							 (MATRIX *) EAT (r));
	      }
	    else
	      {
		/* They have different symmetry, so convert to full. */
		s = (MATRIX *) subtract_matrix_complex ((MATRIX *) EAT (l),
			       (MATRIX *) full_matrix ((MATRIX *) EAT (r)));
	      }
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      case sparse_upper:
	switch (r->density)
	  {
	  case dense:
	    s = (MATRIX *) subtract_matrix_complex (
			       (MATRIX *) dense_matrix ((MATRIX *) EAT (l)),
						     (MATRIX *) EAT (r));
	    break;
	  case sparse:
	    if (r->nn == 0)
	      {
		s = (MATRIX *) copy_matrix (l);
		break;
	      }

	    if (l->symmetry == r->symmetry)
	      {
		/*
		 * They have the same symmetry, but are stored differently.
		 * Convert to the sparse_upper form.
		 */
		s = (MATRIX *) subtract_matrix_complex (
							 (MATRIX *) EAT (l),
			     (MATRIX *) sparse_matrix ((MATRIX *) EAT (r)));
	      }
	    else
	      {
		/* They have different symmetry, so convert to full. */
		s = (MATRIX *) subtract_matrix_complex (
				(MATRIX *) full_matrix ((MATRIX *) EAT (l)),
							 (MATRIX *) EAT (r));
	      }
	    break;

	  case sparse_upper:

	    if (l->nn == 0 && r->nn == 0)
	      {
		s = (MATRIX *)
		  form_matrix (l->nr, l->nc, complex, sparse_upper);
		s->symmetry = (l->symmetry == r->symmetry) ?
		  l->symmetry : symmetric;
		if (l->d.complex)
		  {
		    s->d.complex = (COMPLEX *) E_MALLOC (l->nr, complex);
		    if (r->d.complex)
		      {
			for (i=0; i<l->nr; i++)
			  {
			    s->d.complex[i].real =
			      l->d.complex[i].real - r->d.complex[i].real;
			    s->d.complex[i].imag =
			      l->d.complex[i].imag - r->d.complex[i].imag;
			  }
		      }
		    else
		      {
			for (i=0; i<l->nr; i++)
			  {
			    s->d.complex[i].real = l->d.complex[i].real;
			    s->d.complex[i].imag = l->d.complex[i].imag;
			  }
		      }
		  }
		else
		  {
		    if (r->d.complex)
		      {
			s->d.complex = (COMPLEX *) E_MALLOC (l->nr, complex);
			for (i=0; i<l->nr; i++)
			  {
			    s->d.complex[i].real = -r->d.complex[i].real;
			    s->d.complex[i].imag = -r->d.complex[i].imag;
			  }
		      }
		  }
	      }
	    else if (l->symmetry == r->symmetry)
	      {
		if (l->nn == 0)
		  {
		    s = (MATRIX *) negate_matrix ((MATRIX *) EAT (r));
		    if (l->d.complex == NULL)
		      break;

		    if (s->d.complex == NULL)
		      s->d.complex =
			(COMPLEX *) CALLOC (s->nr, sizeof (COMPLEX));
		    for (i = 0; i < s->nr; i++)
		      {
			s->d.complex[i].real += l->d.complex[i].real;
			s->d.complex[i].imag +=
			  l->d.complex[i].imag;
		      }
		  }
		else if (r->nn == 0)
		  {
		    if (r->d.complex == NULL)
		      {
			s = (MATRIX *) EAT (l);
			break;
		      }
		    s = (MATRIX *) dup_matrix ((MATRIX *) EAT (l));
		    if (s->d.complex == NULL)
		      s->d.complex =
			(COMPLEX *) CALLOC (s->nr, sizeof (COMPLEX));
		    for (i = 0; i < s->nr; i++)
		      {
			s->d.complex[i].real -= r->d.complex[i].real;
			s->d.complex[i].imag -=
			  r->d.complex[i].imag;
		      }
		  }
		else
		  {

		    s = add_symbolic_matrix ((MATRIX *) copy_matrix (l),
					     (MATRIX *) copy_matrix (r));

		    x_tmp = CALLOC (s->nc, sizeof (COMPLEX));
		    ZGSSUB (l->ia, l->ja, l->a.complex,
			    r->ia, r->ja, r->a.complex,
			    &s->nr, &s->nc,
			    s->ia, s->ja, s->a.complex,
			    x_tmp);
		    FREE (x_tmp);

		    if (l->d.complex != NULL)
		      {
			s->d.complex =
			  dup_mem (l->d.complex, s->nr * sizeof (COMPLEX));
			if (r->d.complex != NULL)
			  {
			    for (i = 0; i < s->nr; i++)
			      {
				s->d.complex[i].real -=
				  r->d.complex[i].real;
				s->d.complex[i].imag -=
				  r->d.complex[i].imag;
			      }
			  }
		      }
		    else if (r->d.complex != NULL)
		      {
			s->d.complex =
			  dup_mem (r->d.complex, s->nr * sizeof (COMPLEX));
			for (i = 0; i < s->nr; i++)
			  {
			    s->d.complex[i].real = -s->d.complex[i].real;
			    s->d.complex[i].imag =
			      -s->d.complex[i].imag;
			  }
		      }
		  }
	      }
	    else
	      {

		/*
		 * They have different symmetry, so we'll change them
		 * to sparse.
		 */

		s = (MATRIX *) subtract_matrix_complex (
				(MATRIX *) full_matrix ((MATRIX *) EAT (l)),
			       (MATRIX *) full_matrix ((MATRIX *) EAT (r)));
	      }
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      default:
	BAD_DENSITY (l->density);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_2_entities (rid, cid);
    delete_3_matrices (l, r, s);
  }
  END_EXCEPTION;

  delete_2_entities (s->rid, s->cid);
  s->rid = rid;
  s->cid = cid;

  delete_2_matrices (l, r);
  return (ENT (s));
}
