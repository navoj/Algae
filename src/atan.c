/* -*- Mode: C -*-
 *
 * atan.c -- Arc tangent.
 *
 * Copyright (C) 1995-2003  K. Scott Hunziker
 * Copyright (C) 1994       The Boeing Company.
 *
 * See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: atan.c,v 1.2 2003/08/01 04:57:46 ksh Exp $";

#include "atan.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "apply.h"
#include "log.h"
#include "cast.h"
#include "dense.h"

static char bad_labels[] = "Labels don't match in atan2.";
static char bad_dims[] = "atan2: Array dimensions don't match.";

COMPLEX
atan_complex (COMPLEX z)
{
  COMPLEX w;

  /* atan(z) = i*log((i+z)/(i-z))/2 */
  /*         = i*log(-((x^2+y^2-1)+i*(2*x))/(x^2+y^2-2*y+1))/2 */

  w.real = w.imag = -1.0 / (z.real * z.real + z.imag * z.imag
			    - 2 * z.imag + 1.0);

  w.real *= z.real * z.real + z.imag * z.imag - 1.0;
  w.imag *= 2 * z.real;

  z = log_complex (w);

  w.real = -0.5 * z.imag;
  w.imag = 0.5 * z.real;

  return (w);
}

ENTITY *
bi_atan (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (atan_scalar ((SCALAR *) p));
    case vector:
      return (atan_vector ((VECTOR *) p));
    case matrix:
      return (atan_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
atan_scalar (SCALAR *p)
{
  SCALAR * volatile s = NULL;

  EASSERT (p, scalar, 0);

  WITH_HANDLING
  {
    switch (p->type)
      {
      case integer:
	s = (SCALAR *) cast_scalar_integer_real (p);
	s->v.real = atan (s->v.real);
	break;
      case real:
	s = (SCALAR *) dup_scalar (p);
	s->v.real = atan (s->v.real);
	break;
      case complex:
	s = (SCALAR *) dup_scalar (p);
	s->v.complex = atan_complex (s->v.complex);
	break;
      default:
	BAD_TYPE (p->type);
	delete_scalar (p);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_scalar (s);
  }
  END_EXCEPTION;

  return (ENT (s));
}

ENTITY *
atan_vector (VECTOR *p)
{
  VECTOR *m;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
      p = (VECTOR *) cast_vector_integer_real (p);
      m = (VECTOR *) apply_vector_real_real (atan, p);
      break;
    case real:
      m = (VECTOR *) apply_vector_real_real (atan, p);
      break;
    case complex:
      m = (VECTOR *) apply_vector_complex_complex (atan_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
atan_matrix (MATRIX *p)
{
  MATRIX *m;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
      p = (MATRIX *) cast_matrix_integer_real (p);
      m = (MATRIX *) apply_matrix_real_real (atan, p);
      break;
    case real:
      m = (MATRIX *) apply_matrix_real_real (atan, p);
      break;
    case complex:
      m = (MATRIX *) apply_matrix_complex_complex (atan_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
bi_atan2 (ENTITY *y, ENTITY *x)
{
  EASSERT (y, 0, 0);
  EASSERT (x, 0, 0);

  switch (TWO_CLASS (y, x))
    {

    case scalar_scalar:
      return (atan2_scalar_scalar ((SCALAR *) y, (SCALAR *) x));
    case scalar_vector:
      return (atan2_scalar_vector ((SCALAR *) y, (VECTOR *) x));
    case scalar_matrix:
      return (atan2_scalar_matrix ((SCALAR *) y, (MATRIX *) x));

    case vector_scalar:
      return (atan2_vector_scalar ((VECTOR *) y, (SCALAR *) x));
    case vector_vector:
      return (atan2_vector_vector ((VECTOR *) y, (VECTOR *) x));
    case vector_matrix:
      return (atan2_vector_matrix ((VECTOR *) y, (MATRIX *) x));

    case matrix_scalar:
      return (atan2_matrix_scalar ((MATRIX *) y, (SCALAR *) x));
    case matrix_vector:
      return (atan2_matrix_vector ((MATRIX *) y, (VECTOR *) x));
    case matrix_matrix:
      return (atan2_matrix_matrix ((MATRIX *) y, (MATRIX *) x));

    default:
      fail ("Invalid class (%s and/or %s) for atan2.",
	    class_string[y->class], class_string[x->class]);
      delete_2_entities (y, x);
      raise_exception ();
    }
}

ENTITY *
atan2_scalar_scalar (SCALAR *y, SCALAR *x)
{
  SCALAR *s;

  EASSERT (y, scalar, 0);
  EASSERT (x, scalar, 0);

  /* cast to real */

  if (y->type != real)
    {
      if (y->type != integer)
	{
	  fail ("Invalid %s type in atan2.", type_string[y->type]);
	  delete_2_scalars (y, x);
	  raise_exception ();
	}
      else
	{
	  y = (SCALAR *) cast_scalar (y, real);
	}
    }

  if (x->type != real)
    {
      if (x->type != integer)
	{
	  fail ("Invalid %s type in atan2.", type_string[x->type]);
	  delete_2_scalars (y, x);
	  raise_exception ();
	}
      else
	{
	  x = (SCALAR *) cast_scalar (x, real);
	}
    }

  errno = 0;
  s = (SCALAR *) real_to_scalar (atan2 (y->v.real, x->v.real));
  CHECK_MATH ();

  delete_2_scalars (y, x);
  return ENT (s);
}

ENTITY *
atan2_scalar_vector (SCALAR *y, VECTOR *x)
{
  int i;
  VECTOR * volatile v = NULL;

  EASSERT (y, scalar, 0);
  EASSERT (x, vector, 0);

  WITH_HANDLING
    {
      /* cast to real */
      
      if (y->type != real)
	{
	  if (y->type != integer)
	    {
	      fail ("Invalid %s type in atan2.", type_string[y->type]);
	      raise_exception ();
	    }
	  else
	    {
	      y = (SCALAR *) cast_scalar (EAT (y), real);
	    }
	}
      
      if (x->type != real)
	{
	  if (x->type != integer)
	    {
	      fail ("Invalid %s type in atan2.", type_string[x->type]);
	      raise_exception ();
	    }
	  else
	    {
	      x = (VECTOR *) cast_vector (EAT (x), real);
	    }
	}
      
      /* forget about sparse, just convert to dense */
      
      x = (VECTOR *) dense_vector (EAT (x));
      
      /* get our own copy to work with */
      
      v = (VECTOR *) dup_vector (EAT (x));
      
      /* apply atan2 */
      
      errno = 0;
      for (i = 0; i < v->ne; i++)
	v->a.real[i] = atan2 (y->v.real, v->a.real[i]);
      CHECK_MATH ();
    }
  ON_EXCEPTION
    {
      delete_scalar (y);
      delete_2_vectors (x, v);
    }
  END_EXCEPTION;

  delete_scalar (y);
  return ENT (v);
}

ENTITY *
atan2_scalar_matrix (SCALAR *y, MATRIX *x)
{
  int i;
  MATRIX * volatile m = NULL;

  EASSERT (y, scalar, 0);
  EASSERT (x, matrix, 0);

  WITH_HANDLING
    {
      /* cast to real */
      
      if (y->type != real)
	{
	  if (y->type != integer)
	    {
	      fail ("Invalid %s type in atan2.", type_string[y->type]);
	      raise_exception ();
	    }
	  else
	    {
	      y = (SCALAR *) cast_scalar (EAT (y), real);
	    }
	}
      
      if (x->type != real)
	{
	  if (x->type != integer)
	    {
	      fail ("Invalid %s type in atan2.", type_string[x->type]);
	      raise_exception ();
	    }
	  else
	    {
	      x = (MATRIX *) cast_matrix (EAT (x), real);
	    }
	}
      
      /* forget about sparse, just convert to dense */
      
      x = (MATRIX *) dense_matrix (EAT (x));
      
      /* get our own copy to work with */
      
      m = (MATRIX *) dup_matrix (EAT (x));
      
      /* apply atan2 */
      
      errno = 0;
      for (i = 0; i < m->nn; i++)
	m->a.real[i] = atan2 (y->v.real, m->a.real[i]);
      CHECK_MATH ();
    }
  ON_EXCEPTION
    {
      delete_scalar (y);
      delete_2_matrices (x, m);
    }
  END_EXCEPTION;

  delete_scalar (y);
  return ENT (m);
}

ENTITY *
atan2_vector_scalar (VECTOR *y, SCALAR *x)
{
  int i;
  VECTOR * volatile v = NULL;

  EASSERT (y, vector, 0);
  EASSERT (x, scalar, 0);

  WITH_HANDLING
    {
      /* cast to real */
      
      if (y->type != real)
	{
	  if (y->type != integer)
	    {
	      fail ("Invalid %s type in atan2.", type_string[y->type]);
	      raise_exception ();
	    }
	  else
	    {
	      y = (VECTOR *) cast_vector (EAT (y), real);
	    }
	}
      
      if (x->type != real)
	{
	  if (x->type != integer)
	    {
	      fail ("Invalid %s type in atan2.", type_string[x->type]);
	      raise_exception ();
	    }
	  else
	    {
	      x = (SCALAR *) cast_scalar (EAT (x), real);
	    }
	}
      
      /* forget about sparse, just convert to dense */
      
      y = (VECTOR *) dense_vector (EAT (y));
      
      /* get our own copy to work with */
      
      v = (VECTOR *) dup_vector (EAT (y));
      
      /* apply atan2 */
      
      errno = 0;
      for (i = 0; i < v->ne; i++)
	v->a.real[i] = atan2 (v->a.real[i], x->v.real);
      CHECK_MATH ();
    }
  ON_EXCEPTION
    {
      delete_scalar (x);
      delete_2_vectors (y, v);
    }
  END_EXCEPTION;

  delete_scalar (x);
  return ENT (v);
}

ENTITY *
atan2_vector_vector (VECTOR *y, VECTOR *x)
{
  int i;
  VECTOR * volatile v = NULL;

  EASSERT (y, vector, 0);
  EASSERT (x, vector, 0);

  WITH_HANDLING
    {
      /* check dimensions */
      
      if (y->ne != x->ne)
	{
	  fail (bad_dims);
	  raise_exception ();
	}
      
      /* check for matching labels */
      
      if (!MATCH_VECTORS (y->eid, x->eid))
	{
	  fail (bad_labels);
	  raise_exception ();
	}
      
      /* cast to real */
      
      if (y->type != real)
	{
	  if (y->type != integer)
	    {
	      fail ("Invalid %s type in atan2.", type_string[y->type]);
	      raise_exception ();
	    }
	  else
	    {
	      y = (VECTOR *) cast_vector (EAT (y), real);
	    }
	}
      
      if (x->type != real)
	{
	  if (x->type != integer)
	    {
	      fail ("Invalid %s type in atan2.", type_string[x->type]);
	      raise_exception ();
	    }
	  else
	    {
	      x = (VECTOR *) cast_vector (EAT (x), real);
	    }
	}
      
      /* forget about sparse, just convert to dense */
      
      y = (VECTOR *) dense_vector (EAT (y));
      x = (VECTOR *) dense_vector (EAT (x));
      
      /* get our own copy to work with */
      
      v = (VECTOR *) dup_vector (EAT (y));
      if (v->eid == NULL && x->eid != NULL)
	v->eid = copy_entity (x->eid);
      
      /* apply atan2 */
      
      errno = 0;
      for (i = 0; i < v->ne; i++)
	v->a.real[i] = atan2 (v->a.real[i], x->a.real[i]);
      CHECK_MATH ();
    }
  ON_EXCEPTION
    {
      delete_3_vectors (y, x, v);
    }
  END_EXCEPTION;

  delete_vector (x);
  return ENT (v);
}

ENTITY *
atan2_vector_matrix (VECTOR *y, MATRIX *x)
{
  return atan2_matrix_matrix ((MATRIX *) vector_to_matrix (y), x);
}

ENTITY *
atan2_matrix_scalar (MATRIX *y, SCALAR *x)
{
  int i;
  MATRIX * volatile m = NULL;

  EASSERT (y, matrix, 0);
  EASSERT (x, scalar, 0);

  WITH_HANDLING
    {
      /* cast to real */
      
      if (y->type != real)
	{
	  if (y->type != integer)
	    {
	      fail ("Invalid %s type in atan2.", type_string[y->type]);
	      raise_exception ();
	    }
	  else
	    {
	      y = (MATRIX *) cast_matrix (EAT (y), real);
	    }
	}
      
      if (x->type != real)
	{
	  if (x->type != integer)
	    {
	      fail ("Invalid %s type in atan2.", type_string[x->type]);
	      raise_exception ();
	    }
	  else
	    {
	      x = (SCALAR *) cast_scalar (EAT (x), real);
	    }
	}
      
      /* forget about sparse, just convert to dense */
      
      y = (MATRIX *) dense_matrix (EAT (y));
      
      /* get our own copy to work with */
      
      m = (MATRIX *) dup_matrix (EAT (y));
      
      /* apply atan2 */
      
      errno = 0;
      for (i = 0; i < m->nn; i++)
	m->a.real[i] = atan2 (m->a.real[i], x->v.real);
      CHECK_MATH ();
    }
  ON_EXCEPTION
    {
      delete_scalar (x);
      delete_2_matrices (y, m);
    }
  END_EXCEPTION;

  delete_scalar (x);
  return ENT (m);
}

ENTITY *
atan2_matrix_vector (MATRIX *y, VECTOR *x)
{
  return atan2_matrix_matrix (y, (MATRIX *) vector_to_matrix (x));
}

ENTITY *
atan2_matrix_matrix (MATRIX *y, MATRIX *x)
{
  int i;
  MATRIX * volatile m = NULL;

  EASSERT (y, matrix, 0);
  EASSERT (x, matrix, 0);

  WITH_HANDLING
    {
      /* check dimensions */
      
      if (y->nr != x->nr || y->nc != x->nc)
	{
	  fail (bad_dims);
	  raise_exception ();
	}
      
      /* check for matching labels */
      
      if (!MATCH_VECTORS (y->rid, x->rid) ||
	  !MATCH_VECTORS (y->cid, x->cid))
	{
	  fail (bad_labels);
	  raise_exception ();
	}
      
      /* cast to real */
      
      if (y->type != real)
	{
	  if (y->type != integer)
	    {
	      fail ("Invalid %s type in atan2.", type_string[y->type]);
	      raise_exception ();
	    }
	  else
	    {
	      y = (MATRIX *) cast_matrix (EAT (y), real);
	    }
	}
      
      if (x->type != real)
	{
	  if (x->type != integer)
	    {
	      fail ("Invalid %s type in atan2.", type_string[x->type]);
	      raise_exception ();
	    }
	  else
	    {
	      x = (MATRIX *) cast_matrix (EAT (x), real);
	    }
	}
      
      /* forget about sparse, just convert to dense */
      
      y = (MATRIX *) dense_matrix (EAT (y));
      x = (MATRIX *) dense_matrix (EAT (x));
      
      /* get our own copy to work with */
      
      m = (MATRIX *) dup_matrix (EAT (y));
      if (m->rid == NULL && x->rid != NULL)
	m->rid = copy_entity (x->rid);
      if (m->cid == NULL && x->cid != NULL)
	m->cid = copy_entity (x->cid);
      
      /* apply atan2 */
      
      errno = 0;
      for (i = 0; i < m->nn; i++)
	m->a.real[i] = atan2 (m->a.real[i], x->a.real[i]);
      CHECK_MATH ();
    }
  ON_EXCEPTION
    {
      delete_3_matrices (y, x, m);
    }
  END_EXCEPTION;

  delete_matrix (x);
  return ENT (m);
}
