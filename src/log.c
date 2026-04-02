/*
   log.c -- Logarithms.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: log.c,v 1.5 2003/08/01 04:57:47 ksh Exp $";

#include "log.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "apply.h"
#include "cast.h"
#include "dense.h"
#include "abs.h"

/* On some lame systems (AIX and HPUX), log(0) does not set errno. */

#if LAX_LOG
double
xlog (double x)
{
  if (!x) errno = ERANGE;
  return log (x);
}
#endif

COMPLEX
log_complex (COMPLEX z)
{
  /*
   * Complex logarithm.
   *
   * This function was adapted from the f2c "z_log" function.
   * Copyright (C) 1997, 1998, 2000 Lucent Technologies
   */

  COMPLEX w;
  REAL zr = z.real;
  REAL zi = z.imag;
#if 0
  REAL s, s0, t, t2, u, v;
#endif

  errno = 0;

  w.imag = atan2 (zi, zr);

  /*
   * I've reverted to the old calculation for the real part.  The new code
   * from f2c appears to get botched with "gcc -O2".  It would be nice to
   * track this down.
   */

  w.real = xlog (abs_complex (z));

#if 0
  if (zi < 0) zi = -zi;
  if (zr < 0) zr = -zr;
  if (zr < zi)
    {
      t = zi;
      zi = zr;
      zr = t;
    }
  t = zi/zr;
  s = zr * sqrt (1 + t*t);

  /* now s = f__cabs(zi,zr), and zr = |zr| >= |zi| = zi */

  if ((t = s - 1) < 0) t = -t;
  if (t > .01)
    {
      w.real = xlog (s);
    }
  else
    {
      /*
       *  log(1+x) = x - x^2/2 + x^3/3 - x^4/4 + - ...
       *           = x(1 - x/2 + x^2/3 -+...)
       *
       * [sqrt(y^2 + z^2) - 1] * [sqrt(y^2 + z^2) + 1] = y^2 + z^2 - 1, so
       *  sqrt(y^2 + z^2) - 1 = (y^2 + z^2 - 1) / [sqrt(y^2 + z^2) + 1]
       */

      t = ((zr*zr - 1.0) + zi*zi) / (s + 1);
      t2 = t*t;
      s = 1.0 - 0.5*t;
      u = v = 1;
      do {
	s0 = s;
	u *= t2;
	v += 2;
	s += u/v - t*u/(v+1);
      } while (s > s0);

      w.real = s*t;
    }
#endif

  CHECK_MATH ();

  return w;
}

ENTITY *
bi_log (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (log_scalar ((SCALAR *) p));
    case vector:
      return (log_vector ((VECTOR *) p));
    case matrix:
      return (log_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
log_scalar (SCALAR *p)
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
	    s->v.complex.real = xlog (-s->v.real);
	    s->v.complex.imag = atan2 (0.0, -1.0);
	  }
	else
	  {
	    s = (SCALAR *) cast_scalar_integer_real (p);
	    errno = 0;
	    s->v.real = xlog (s->v.real);
	    CHECK_MATH ();
	  }
	break;
      case real:
	if (p->v.real < 0.0)
	  {
	    s = (SCALAR *) cast_scalar_real_complex (p);
	    s->v.complex.real = xlog (-s->v.real);
	    s->v.complex.imag = atan2 (0.0, -1.0);
	  }
	else
	  {
	    s = (SCALAR *) dup_scalar (p);
	    errno = 0;
	    s->v.real = xlog (s->v.real);
	    CHECK_MATH ();
	  }
	break;

      case complex:
	s = (SCALAR *) dup_scalar (p);
	errno = 0;
	s->v.complex = log_complex (p->v.complex);
	CHECK_MATH ();
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
log_vector (VECTOR *p)
{
  VECTOR *m;
  int i;
  int neg = 0;

  EASSERT (p, vector, 0);

  errno = 0;

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
	  p = (VECTOR *) dense_vector (p);
	  m = (VECTOR *) apply_vector_complex_complex (log_complex, p);
	  CHECK_MATH ();
	}
      else
	{
	  p = (VECTOR *) cast_vector_integer_real (p);
	  p = (VECTOR *) dense_vector (p);
	  m = (VECTOR *) apply_vector_real_real (xlog, p);
	  CHECK_MATH ();
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
	  p = (VECTOR *) dense_vector (p);
	  m = (VECTOR *) apply_vector_complex_complex (log_complex, p);
	  CHECK_MATH ();
	}
      else
	{
	  p = (VECTOR *) dense_vector (p);
	  m = (VECTOR *) apply_vector_real_real (xlog, p);
	  CHECK_MATH ();
	}
      break;

    case complex:
      p = (VECTOR *) dense_vector (p);
      m = (VECTOR *) apply_vector_complex_complex (log_complex, p);
      CHECK_MATH ();
      break;

    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
log_matrix (MATRIX *p)
{
  MATRIX *m;
  int i;
  int neg = 0;

  EASSERT (p, matrix, 0);

  errno = 0;

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
	  p = (MATRIX *) dense_matrix (p);
	  m = (MATRIX *) apply_matrix_complex_complex (log_complex, p);
	  CHECK_MATH ();
	}
      else
	{
	  p = (MATRIX *) cast_matrix_integer_real (p);
	  p = (MATRIX *) dense_matrix (p);
	  m = (MATRIX *) apply_matrix_real_real (xlog, p);
	  CHECK_MATH ();
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
	  p = (MATRIX *) dense_matrix (p);
	  m = (MATRIX *) apply_matrix_complex_complex (log_complex, p);
	  CHECK_MATH ();
	}
      else
	{
	  p = (MATRIX *) dense_matrix (p);
	  m = (MATRIX *) apply_matrix_real_real (xlog, p);
	  CHECK_MATH ();
	}
      break;

    case complex:
      p = (MATRIX *) dense_matrix (p);
      m = (MATRIX *) apply_matrix_complex_complex (log_complex, p);
      CHECK_MATH ();
      break;

    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (m));
}
