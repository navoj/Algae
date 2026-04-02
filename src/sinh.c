/*
   sinh.c -- Hyperbolic sine.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: sinh.c,v 1.5 2003/08/01 04:57:48 ksh Exp $";

#include "sinh.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "apply.h"
#include "cast.h"
#include "sqrt.h"
#include "asin.h"
#include "log.h"

COMPLEX
sinh_complex (COMPLEX z)
{
  COMPLEX w;

  w.real = sinh (z.real) * cos (z.imag);
  w.imag = cosh (z.real) * sin (z.imag);

  return (w);
}

#if !HAVE_ASINH
double
asinh (double z)
{
  double w;
  int neg;

  errno = 0;
  if (neg = (z < 0.0)) z = -z;
  if (z > 2.0)
    w = log (2.0*z+1.0/(z+sqrt(z*z+1)));
  else
    w = log1p (z + z*z/(1+sqrt(1+z*z)));

  CHECK_MATH ();
  return neg ? -w : w;
}
#endif

COMPLEX
asinh_complex (COMPLEX z)
{
  COMPLEX w, a, b;

  errno = 0;

  a.real = -z.imag;
  a.imag = z.real;

  b = asin_complex (a);

  w.real = b.imag;
  w.imag = -b.real;

  CHECK_MATH ();

  return w;
}

ENTITY *
bi_sinh (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (sinh_scalar ((SCALAR *) p));
    case vector:
      return (sinh_vector ((VECTOR *) p));
    case matrix:
      return (sinh_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
bi_asinh (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (asinh_scalar ((SCALAR *) p));
    case vector:
      return (asinh_vector ((VECTOR *) p));
    case matrix:
      return (asinh_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
sinh_scalar (SCALAR *p)
{
  SCALAR * volatile s = NULL;

  EASSERT (p, scalar, 0);

  WITH_HANDLING
  {
    switch (p->type)
      {
      case integer:
	s = (SCALAR *) cast_scalar_integer_real (p);
	s->v.real = sinh (s->v.real);
	break;
      case real:
	s = (SCALAR *) dup_scalar (p);
	s->v.real = sinh (s->v.real);
	break;
      case complex:
	{
	  COMPLEX z;
	  z = p->v.complex;
	  s = (SCALAR *) dup_scalar (p);
	  s->v.complex.real = sinh (z.real) * cos (z.imag);
	  s->v.complex.imag = cosh (z.real) * sin (z.imag);
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
asinh_scalar (SCALAR *p)
{
  SCALAR * volatile s = NULL;

  EASSERT (p, scalar, 0);

  WITH_HANDLING
  {
    switch (p->type)
      {
      case integer:
	s = (SCALAR *) cast_scalar_integer_real (p);
	s->v.real = asinh (s->v.real);
	break;
      case real:
	s = (SCALAR *) dup_scalar (p);
	s->v.real = asinh (s->v.real);
	break;
      case complex:
	{
	  COMPLEX z;
	  z = p->v.complex;
	  s = (SCALAR *) dup_scalar (p);
	  s->v.complex = asinh_complex (z);
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
sinh_vector (VECTOR *p)
{
  VECTOR *m;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
      p = (VECTOR *) cast_vector_integer_real (p);
      m = (VECTOR *) apply_vector_real_real (sinh, p);
      break;
    case real:
      m = (VECTOR *) apply_vector_real_real (sinh, p);
      break;
    case complex:
      m = (VECTOR *) apply_vector_complex_complex (sinh_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
asinh_vector (VECTOR *p)
{
  VECTOR *m;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
      p = (VECTOR *) cast_vector_integer_real (p);
      m = (VECTOR *) apply_vector_real_real (asinh, p);
      break;
    case real:
      m = (VECTOR *) apply_vector_real_real (asinh, p);
      break;
    case complex:
      m = (VECTOR *) apply_vector_complex_complex (asinh_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
sinh_matrix (MATRIX *p)
{
  MATRIX *m;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
      p = (MATRIX *) cast_matrix_integer_real (p);
      m = (MATRIX *) apply_matrix_real_real (sinh, p);
      break;
    case real:
      m = (MATRIX *) apply_matrix_real_real (sinh, p);
      break;
    case complex:
      m = (MATRIX *) apply_matrix_complex_complex (sinh_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
asinh_matrix (MATRIX *p)
{
  MATRIX *m;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
      p = (MATRIX *) cast_matrix_integer_real (p);
      m = (MATRIX *) apply_matrix_real_real (asinh, p);
      break;
    case real:
      m = (MATRIX *) apply_matrix_real_real (asinh, p);
      break;
    case complex:
      m = (MATRIX *) apply_matrix_complex_complex (asinh_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (m));
}
