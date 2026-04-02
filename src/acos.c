/*
   acos.c -- Arc Cosine.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: acos.c,v 1.3 2003/08/01 04:57:46 ksh Exp $";

#include "acos.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "apply.h"
#include "sqrt.h"
#include "cast.h"
#include "dense.h"

COMPLEX
acos_complex (COMPLEX z)
{
  /* Based on algorithm by W. Kahan. */

  COMPLEX w, a, b, c, d;

  errno = 0;

  a.real = 1.0 - z.real;
  a.imag = -z.imag;

  b.real = 1.0 + z.real;
  b.imag = z.imag;

  c = sqrt_complex (a);
  d = sqrt_complex (b);

  w.real = 2.0 * atan2 (c.real, d.real);

  b.imag = -b.imag;
  d = sqrt_complex (b);

  w.imag = asinh (c.real * d.imag + c.imag * d.real);

  CHECK_MATH ();

  return w;
}


ENTITY *
bi_acos (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (acos_scalar ((SCALAR *) p));
    case vector:
      return (acos_vector ((VECTOR *) p));
    case matrix:
      return (acos_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
acos_scalar (SCALAR *p)
{
  SCALAR * volatile s = NULL;

  EASSERT (p, scalar, 0);

  WITH_HANDLING
  {
    switch (p->type)
      {
      case integer:
	if (abs (p->v.integer) > 1)
	  {
	    s = (SCALAR *) cast_scalar_integer_complex (p);
	    /* no need to check errno here */
	    s->v.complex = acos_complex (s->v.complex);
	  }
	else
	  {
	    s = (SCALAR *) cast_scalar_integer_real (p);
	    /* no need to check errno here */
	    s->v.real = acos (s->v.real);
	  }
	break;
      case real:
	if (fabs (p->v.real) > 1.0)
	  {
	    s = (SCALAR *) cast_scalar_real_complex (p);
	    /* no need to check errno here */
	    s->v.complex = acos_complex (s->v.complex);
	  }
	else
	  {
	    s = (SCALAR *) dup_scalar (p);
	    /* no need to check errno here */
	    s->v.real = acos (s->v.real);
	  }
	break;
      case complex:
	s = (SCALAR *) dup_scalar (p);
	s->v.complex = acos_complex (s->v.complex);
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
acos_vector (VECTOR *p)
{
  VECTOR *m;
  int i;
  short int neg = 0;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
      for (i = 0; i < p->nn; i++)
	if (abs (p->a.integer[i]) > 1)
	  {
	    neg++;
	    break;
	  }
      if (neg)
	{
	  p = (VECTOR *) cast_vector_integer_complex (p);
	  p = (VECTOR *) dense_vector (p);
	  m = (VECTOR *) apply_vector_complex_complex (acos_complex, p);
	}
      else
	{
	  p = (VECTOR *) cast_vector_integer_real (p);
	  p = (VECTOR *) dense_vector (p);
	  m = (VECTOR *) apply_vector_real_real (acos, p);
	}
      break;
    case real:
      for (i = 0; i < p->nn; i++)
	if (fabs (p->a.real[i]) > 1.0)
	  {
	    neg++;
	    break;
	  }
      if (neg)
	{
	  p = (VECTOR *) cast_vector_real_complex (p);
	  p = (VECTOR *) dense_vector (p);
	  m = (VECTOR *) apply_vector_complex_complex (acos_complex, p);
	}
      else
	{
	  p = (VECTOR *) dense_vector (p);
	  m = (VECTOR *) apply_vector_real_real (acos, p);
	}
      break;
    case complex:
      p = (VECTOR *) dense_vector (p);
      m = (VECTOR *) apply_vector_complex_complex (acos_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
acos_matrix (MATRIX *p)
{
  MATRIX *m;
  int i;
  short int neg = 0;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
      for (i = 0; i < p->nn; i++)
	if (abs (p->a.integer[i]) > 1)
	  {
	    neg++;
	    break;
	  }
      if (p->d.integer != NULL && !neg)
	for (i = 0; i < p->nr; i++)
	  if (abs (p->d.integer[i]) > 1)
	    {
	      neg++;
	      break;
	    }
      if (neg)
	{
	  p = (MATRIX *) cast_matrix_integer_complex (p);
	  p = (MATRIX *) dense_matrix (p);
	  m = (MATRIX *) apply_matrix_complex_complex (acos_complex, p);
	}
      else
	{
	  p = (MATRIX *) cast_matrix_integer_real (p);
	  p = (MATRIX *) dense_matrix (p);
	  m = (MATRIX *) apply_matrix_real_real (acos, p);
	}
      break;
    case real:
      for (i = 0; i < p->nn; i++)
	if (fabs (p->a.real[i]) > 1.0)
	  {
	    neg++;
	    break;
	  }
      if (p->d.real != NULL && !neg)
	for (i = 0; i < p->nr; i++)
	  if (fabs (p->d.real[i]) > 1.0)
	    {
	      neg++;
	      break;
	    }
      if (neg)
	{
	  p = (MATRIX *) cast_matrix_real_complex (p);
	  p = (MATRIX *) dense_matrix (p);
	  m = (MATRIX *) apply_matrix_complex_complex (acos_complex, p);
	}
      else
	{
	  p = (MATRIX *) dense_matrix (p);
	  m = (MATRIX *) apply_matrix_real_real (acos, p);
	}
      break;
    case complex:
      p = (MATRIX *) dense_matrix (p);
      m = (MATRIX *) apply_matrix_complex_complex (acos_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (m));
}
