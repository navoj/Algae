/*
   cosh.c -- Hyperbolic Cosine.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: cosh.c,v 1.5 2003/08/01 04:57:47 ksh Exp $";

#include "cosh.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "apply.h"
#include "cast.h"
#include "dense.h"
#include "sqrt.h"
#include "sinh.h"

COMPLEX
cosh_complex (COMPLEX z)
{
  COMPLEX w;

  w.real = cosh (z.real) * cos (z.imag);
  w.imag = sinh (z.real) * sin (z.imag);

  return (w);
}

#if !HAVE_ACOSH
double
acosh (double z)
{
  double w;

  errno = 0;
  if (z > 2.0)
    w = log (2.0*z-1.0/(sqrt(z*z-1)+z));
  else
    {
      z -= 1.0;
      w = log1p (z+sqrt(z*(2.0+z)));
    }

  CHECK_MATH ();
  return w;
}
#endif

COMPLEX
acosh_complex (COMPLEX z)
{
  /* Based on algorithm by W. Kahan. */

  COMPLEX w, a, b, c, d;

  errno = 0;

  a.real = z.real - 1.0;
  a.imag = -z.imag;

  b.real = z.real + 1.0;
  b.imag = z.imag;

  c = sqrt_complex (a);
  d = sqrt_complex (b);

  w.real = asinh (c.real * d.real - c.imag * d.imag);

  a.imag = -a.imag;
  c = sqrt_complex (a);

  w.imag = 2.0 * atan2 (c.imag, d.real);

  CHECK_MATH ();

  return w;
}

ENTITY *
bi_cosh (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (cosh_scalar ((SCALAR *) p));
    case vector:
      return (cosh_vector ((VECTOR *) p));
    case matrix:
      return (cosh_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
bi_acosh (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (acosh_scalar ((SCALAR *) p));
    case vector:
      return (acosh_vector ((VECTOR *) p));
    case matrix:
      return (acosh_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
cosh_scalar (SCALAR *p)
{
  SCALAR * volatile s = NULL;

  EASSERT (p, scalar, 0);

  WITH_HANDLING
  {
    switch (p->type)
      {
      case integer:
	s = (SCALAR *) cast_scalar_integer_real (p);
	s->v.real = cosh (s->v.real);
	break;
      case real:
	s = (SCALAR *) dup_scalar (p);
	s->v.real = cosh (s->v.real);
	break;
      case complex:
	{
	  COMPLEX z;
	  z = p->v.complex;
	  s = (SCALAR *) dup_scalar (p);
	  s->v.complex.real = cosh (z.real) * cos (z.imag);
	  s->v.complex.imag = sinh (z.real) * sin (z.imag);
	}
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
acosh_scalar (SCALAR *p)
{
  SCALAR * volatile s = NULL;

  EASSERT (p, scalar, 0);

  WITH_HANDLING
  {
    switch (p->type)
      {
      case integer:
	if (p->v.integer < 1)
	  {
	    s = (SCALAR *) cast_scalar_integer_complex (p);
	    s->v.complex = acosh_complex (s->v.complex);
	  }
	else
	  {
	    s = (SCALAR *) cast_scalar_integer_real (p);
	    s->v.real = acosh (s->v.real);
	  }
	break;
      case real:
	if (p->v.real < 1.0)
	  {
	    s = (SCALAR *) cast_scalar_real_complex (p);
	    s->v.complex = acosh_complex (s->v.complex);
	  }
	else
	  {
	    s = (SCALAR *) dup_scalar (p);
	    s->v.real = acosh (s->v.real);
	  }
	break;
      case complex:
	{
	  COMPLEX z;
	  z = p->v.complex;
	  s = (SCALAR *) dup_scalar (p);
	  s->v.complex = acosh_complex (z);
	}
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
cosh_vector (VECTOR *p)
{
  VECTOR *m;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
      p = (VECTOR *) cast_vector_integer_real (p);
      p = (VECTOR *) dense_vector (p);
      m = (VECTOR *) apply_vector_real_real (cosh, p);
      break;
    case real:
      p = (VECTOR *) dense_vector (p);
      m = (VECTOR *) apply_vector_real_real (cosh, p);
      break;
    case complex:
      p = (VECTOR *) dense_vector (p);
      m = (VECTOR *) apply_vector_complex_complex (cosh_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
acosh_vector (VECTOR *p)
{
  VECTOR *m;
  int i;
  int cmplx = 0;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
      p = (VECTOR *) dense_vector (p);
      for (i = 0; i < p->nn; i++)
	if (p->a.integer[i] < 1) {cmplx++; break;}
      if (cmplx)
	{
	  p = (VECTOR *) cast_vector_integer_complex (p);
	  m = (VECTOR *) apply_vector_complex_complex (acosh_complex, p);
	}
      else
	{
	  p = (VECTOR *) cast_vector_integer_real (p);
	  m = (VECTOR *) apply_vector_real_real (acosh, p);
	}
      break;
    case real:
      p = (VECTOR *) dense_vector (p);
      for (i = 0; i < p->nn; i++)
	if (p->a.real[i] < 1.0) {cmplx++; break;}
      if (cmplx)
	{
	  p = (VECTOR *) cast_vector_real_complex (p);
	  m = (VECTOR *) apply_vector_complex_complex (acosh_complex, p);
	}
      else
	{
	  m = (VECTOR *) apply_vector_real_real (acosh, p);
	}
      break;
    case complex:
      p = (VECTOR *) dense_vector (p);
      m = (VECTOR *) apply_vector_complex_complex (acosh_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
cosh_matrix (MATRIX *p)
{
  MATRIX *m;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
      p = (MATRIX *) cast_matrix_integer_real (p);
      p = (MATRIX *) dense_matrix (p);
      m = (MATRIX *) apply_matrix_real_real (cosh, p);
      break;
    case real:
      p = (MATRIX *) dense_matrix (p);
      m = (MATRIX *) apply_matrix_real_real (cosh, p);
      break;
    case complex:
      p = (MATRIX *) dense_matrix (p);
      m = (MATRIX *) apply_matrix_complex_complex (cosh_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
acosh_matrix (MATRIX *p)
{
  MATRIX *m;
  int i;
  int cmplx = 0;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
      p = (MATRIX *) dense_matrix (p);
      for (i = 0; i < p->nn; i++)
	if (p->a.integer[i] < 1) {cmplx++; break;}
      if (cmplx)
	{
	  p = (MATRIX *) cast_matrix_integer_complex (p);
	  m = (MATRIX *) apply_matrix_complex_complex (acosh_complex, p);
	}
      else
	{
	  p = (MATRIX *) cast_matrix_integer_real (p);
	  m = (MATRIX *) apply_matrix_real_real (acosh, p);
	}
      break;
    case real:
      p = (MATRIX *) dense_matrix (p);
      for (i = 0; i < p->nn; i++)
	if (p->a.real[i] < 1.0) {cmplx++; break;}
      if (cmplx)
	{
	  p = (MATRIX *) cast_matrix_real_complex (p);
	  m = (MATRIX *) apply_matrix_complex_complex (acosh_complex, p);
	}
      else
	{
	  m = (MATRIX *) apply_matrix_real_real (acosh, p);
	}
      break;
    case complex:
      p = (MATRIX *) dense_matrix (p);
      m = (MATRIX *) apply_matrix_complex_complex (acosh_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (m));
}
