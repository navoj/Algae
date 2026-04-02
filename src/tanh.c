/*
   tanh.c -- Hyperbolic tangent.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: tanh.c,v 1.4 2003/08/01 04:57:48 ksh Exp $";

#include "tanh.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "apply.h"
#include "cast.h"
#include "log.h"

COMPLEX
tanh_complex (COMPLEX z)
{
  COMPLEX w;

  w.real = w.imag = 1.0 / (cosh (2 * z.real) + cos (2 * z.imag));
  w.real *= sinh (2 * z.real);
  w.imag *= sin (2 * z.imag);

  return (w);
}

#if !HAVE_ATANH
double
atanh (double z)
{
  double w;
  int neg;

  errno = 0;
  if (neg = (z < 0.0)) z = -z;
  if (z >= 0.5)
    w = 0.5 * log1p (2.0 * z/(1.0-z));
  else
    w = 0.5 * log1p (2.0*z + 2.0*z*z/(1.0-z));

  CHECK_MATH ();
  return neg ? -w : w;
}
#endif

COMPLEX
atanh_complex (COMPLEX z)
{
  /*
   * Compute atanh(z) = log((1+z)/(1-z))/2 using a numerically
   * robust decomposition that avoids overflow and cancellation.
   *
   * atanh(x+iy) = Re + i*Im where:
   *   Re = log1p(4x / ((1-x)^2 + y^2)) / 4   when |x| < 0.5
   *      = log((y^2 + (1+x)^2) / (y^2 + (1-x)^2)) / 4  otherwise
   *   Im = atan2(2y, 1-x^2-y^2) / 2
   *
   * Note: atanh(z) = -i * atan(i*z), so the formula parallels atan_complex
   * with x and y roles swapped.
   */

  COMPLEX w;
  double x = z.real;
  double y = z.imag;
  double y2 = y * y;

  /* Real part */
  if (fabs (x) < 0.5)
    {
      /* Use log1p for better accuracy near x=0 */
      double omx = 1.0 - x;
      double d = omx * omx + y2;
      w.real = 0.25 * log1p (4.0 * x / d);
    }
  else
    {
      /* General formula */
      double num = y2 + (1.0 + x) * (1.0 + x);
      double den = y2 + (1.0 - x) * (1.0 - x);
      w.real = 0.25 * log (num / den);
    }

  /* Imaginary part */
  {
    double denom = 1.0 - x * x - y2;
    w.imag = 0.5 * atan2 (2.0 * y, denom);
  }

  return w;
}

ENTITY *
bi_tanh (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (tanh_scalar ((SCALAR *) p));
    case vector:
      return (tanh_vector ((VECTOR *) p));
    case matrix:
      return (tanh_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
bi_atanh (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (atanh_scalar ((SCALAR *) p));
    case vector:
      return (atanh_vector ((VECTOR *) p));
    case matrix:
      return (atanh_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
tanh_scalar (SCALAR *p)
{
  SCALAR * volatile s = NULL;

  EASSERT (p, scalar, 0);

  WITH_HANDLING
  {
    switch (p->type)
      {
      case integer:
	s = (SCALAR *) cast_scalar_integer_real (p);
	s->v.real = tanh (s->v.real);
	break;
      case real:
	s = (SCALAR *) dup_scalar (p);
	s->v.real = tanh (s->v.real);
	break;
      case complex:
	{
	  COMPLEX z;
	  z = p->v.complex;
	  s = (SCALAR *) dup_scalar (p);
	  s->v.complex.real = s->v.complex.imag =
	    1.0 / (cosh (2 * z.real) + cos (2 * z.imag));
	  s->v.complex.real *= sinh (2 * z.real);
	  s->v.complex.imag *= sin (2 * z.imag);
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
atanh_scalar (SCALAR *p)
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
	    s->v.complex = atanh_complex (s->v.complex);
	  }
	else
	  {
	    s = (SCALAR *) cast_scalar_integer_real (p);
	    s->v.real = atanh (s->v.real);
	  }
	break;
      case real:
	if (fabs (p->v.real) > 1.0)
	  {
	    s = (SCALAR *) cast_scalar_real_complex (p);
	    s->v.complex = atanh_complex (s->v.complex);
	  }
	else
	  {
	    s = (SCALAR *) dup_scalar (p);
	    s->v.real = atanh (p->v.real);
	  }
	break;
      case complex:
	{
	  COMPLEX z;
	  z = p->v.complex;
	  s = (SCALAR *) dup_scalar (p);
	  s->v.complex = atanh_complex (z);
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
tanh_vector (VECTOR *p)
{
  VECTOR *m;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
      p = (VECTOR *) cast_vector_integer_real (p);
      m = (VECTOR *) apply_vector_real_real (tanh, p);
      break;
    case real:
      m = (VECTOR *) apply_vector_real_real (tanh, p);
      break;
    case complex:
      m = (VECTOR *) apply_vector_complex_complex (tanh_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
atanh_vector (VECTOR *p)
{
  VECTOR *m;
  int i;
  int cmplx = 0;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
      for (i = 0; i < p->nn; i++)
	if (abs (p->a.integer[i]) > 1) {cmplx++; break;}
      if (cmplx)
	{
	  p = (VECTOR *) cast_vector_integer_complex (p);
	  m = (VECTOR *) apply_vector_complex_complex (atanh_complex, p);
	}
      else
	{
	  p = (VECTOR *) cast_vector_integer_real (p);
	  m = (VECTOR *) apply_vector_real_real (atanh, p);
	}
      break;
    case real:
      for (i = 0; i < p->nn; i++)
	if (fabs (p->a.real[i]) > 1.0) {cmplx++; break;}
      if (cmplx)
	{
	  p = (VECTOR *) cast_vector_real_complex (p);
	  m = (VECTOR *) apply_vector_complex_complex (atanh_complex, p);
	}
      else
	{
	  m = (VECTOR *) apply_vector_real_real (atanh, p);
	}
      break;
    case complex:
      m = (VECTOR *) apply_vector_complex_complex (atanh_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
tanh_matrix (MATRIX *p)
{
  MATRIX *m;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
      p = (MATRIX *) cast_matrix_integer_real (p);
      m = (MATRIX *) apply_matrix_real_real (tanh, p);
      break;
    case real:
      m = (MATRIX *) apply_matrix_real_real (tanh, p);
      break;
    case complex:
      m = (MATRIX *) apply_matrix_complex_complex (tanh_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
atanh_matrix (MATRIX *p)
{
  MATRIX *m;
  int i;
  int cmplx = 0;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
      for (i = 0; i < p->nn; i++)
	if (abs (p->a.integer[i]) > 1) {cmplx++; break;}
      if (cmplx)
	{
	  p = (MATRIX *) cast_matrix_integer_complex (p);
	  m = (MATRIX *) apply_matrix_complex_complex (atanh_complex, p);
	}
      else
	{
	  p = (MATRIX *) cast_matrix_integer_real (p);
	  m = (MATRIX *) apply_matrix_real_real (atanh, p);
	}
      break;
    case real:
      for (i = 0; i < p->nn; i++)
	if (fabs (p->a.real[i]) > 1) {cmplx++; break;}
      if (cmplx)
	{
	  p = (MATRIX *) cast_matrix_real_complex (p);
	  m = (MATRIX *) apply_matrix_complex_complex (atanh_complex, p);
	}
      else
	{
	  m = (MATRIX *) apply_matrix_real_real (atanh, p);
	}
      break;
    case complex:
      m = (MATRIX *) apply_matrix_complex_complex (atanh_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (m));
}
