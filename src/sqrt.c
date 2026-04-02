/*
   sqrt.c -- Square roots.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: sqrt.c,v 1.2 2003/08/01 04:57:48 ksh Exp $";

#include "sqrt.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "apply.h"
#include "cast.h"
#include "abs.h"


ENTITY *
bi_sqrt (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (sqrt_scalar ((SCALAR *) p));
    case vector:
      return (sqrt_vector ((VECTOR *) p));
    case matrix:
      return (sqrt_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
sqrt_scalar (SCALAR *p)
{
  SCALAR * volatile s = NULL;

  EASSERT (p, scalar, 0);

  WITH_HANDLING
  {
    switch (p->type)
      {
      case integer:
	if (p->v.integer < 0)
	  {
	    s = (SCALAR *) cast_scalar_integer_complex (p);
	    s->v.complex.imag = sqrt (fabs (s->v.complex.real));
	    s->v.complex.real = 0.0;
	  }
	else
	  {
	    s = (SCALAR *) cast_scalar_integer_real (p);
	    s->v.real = sqrt (s->v.real);
	  }
	break;
      case real:
	if (p->v.real < 0)
	  {
	    s = (SCALAR *) cast_scalar_real_complex (p);
	    s->v.complex.imag = sqrt (fabs (s->v.complex.real));
	    s->v.complex.real = 0.0;
	  }
	else
	  {
	    s = (SCALAR *) dup_scalar (p);
	    s->v.real = sqrt (s->v.real);
	  }
	break;
      case complex:
	s = (SCALAR *) dup_scalar (p);
	s->v.complex = sqrt_complex (s->v.complex);
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
sqrt_vector (VECTOR *p)
{
  VECTOR *m;
  int i;
  int neg = 0;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
      for (i = 0; i < p->nn; i++)
	if (p->a.integer[i] < 0)
	  {
	    neg++;
	    break;
	  }
      if (neg)
	{
	  p = (VECTOR *) cast_vector_integer_complex (p);
	  m = (VECTOR *) apply_vector_complex_complex (sqrt_complex, p);
	}
      else
	{
	  p = (VECTOR *) cast_vector_integer_real (p);
	  m = (VECTOR *) apply_vector_real_real (sqrt, p);
	}
      break;
    case real:
      for (i = 0; i < p->nn; i++)
	if (p->a.real[i] < 0.0)
	  {
	    neg++;
	    break;
	  }
      if (neg)
	{
	  p = (VECTOR *) cast_vector_real_complex (p);
	  m = (VECTOR *) apply_vector_complex_complex (sqrt_complex, p);
	}
      else
	{
	  m = (VECTOR *) apply_vector_real_real (sqrt, p);
	}
      break;
    case complex:
      m = (VECTOR *) apply_vector_complex_complex (sqrt_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
sqrt_matrix (MATRIX *p)
{
  MATRIX *m;
  int i;
  int neg = 0;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
      for (i = 0; i < p->nn; i++)
	if (p->a.integer[i] < 0)
	  {
	    neg++;
	    break;
	  }
      if (p->d.integer != NULL && !neg)
	for (i = 0; i < p->nr; i++)
	  if (p->d.integer[i] < 0)
	    {
	      neg++;
	      break;
	    }
      if (neg)
	{
	  p = (MATRIX *) cast_matrix_integer_complex (p);
	  m = (MATRIX *) apply_matrix_complex_complex (sqrt_complex, p);
	}
      else
	{
	  p = (MATRIX *) cast_matrix_integer_real (p);
	  m = (MATRIX *) apply_matrix_real_real (sqrt, p);
	}
      break;
    case real:
      for (i = 0; i < p->nn; i++)
	if (p->a.real[i] < 0)
	  {
	    neg++;
	    break;
	  }
      if (p->d.real != NULL && !neg)
	for (i = 0; i < p->nr; i++)
	  if (p->d.real[i] < 0)
	    {
	      neg++;
	      break;
	    }
      if (neg)
	{
	  p = (MATRIX *) cast_matrix_real_complex (p);
	  m = (MATRIX *) apply_matrix_complex_complex (sqrt_complex, p);
	}
      else
	{
	  m = (MATRIX *) apply_matrix_real_real (sqrt, p);
	}
      break;
    case complex:
      m = (MATRIX *) apply_matrix_complex_complex (sqrt_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (m));
}

COMPLEX
sqrt_complex (COMPLEX z)
{
  /*
   * Complex square root.
   *
   * This function was adapted from the f2c "z_sqrt" function.
   * Copyright 1990 by AT&T Bell Laboratories and Bellcore.
   */

  COMPLEX r;
  REAL mag;

  if ((mag = abs_complex (z)) == 0.0)
    r.real = r.imag = 0.;
  else if (z.real > 0.0)
    {
      r.real = sqrt (0.5 * (mag + z.real));
      r.imag = z.imag / r.real / 2.0;
    }
  else
    {
      r.imag = sqrt (0.5 * (mag - z.real));
      if (z.imag < 0.0)
	r.imag = -r.imag;
      r.real = z.imag / r.imag / 2.0;
    }

  return r;
}
