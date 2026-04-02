/*
   tan.c -- Tangent.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: tan.c,v 1.2 2003/08/01 04:57:48 ksh Exp $";

#include "tan.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "apply.h"
#include "cast.h"

COMPLEX
tan_complex (COMPLEX z)
{
  COMPLEX w;

  w.real = w.imag = 1.0 / (cos (2 * z.real) + cosh (2 * z.imag));
  w.real *= sin (2 * z.real);
  w.imag *= sinh (2 * z.imag);

  return (w);
}

ENTITY *
bi_tan (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (tan_scalar ((SCALAR *) p));
    case vector:
      return (tan_vector ((VECTOR *) p));
    case matrix:
      return (tan_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
tan_scalar (SCALAR *p)
{
  SCALAR * volatile s = NULL;

  EASSERT (p, scalar, 0);

  WITH_HANDLING
  {
    switch (p->type)
      {
      case integer:
	s = (SCALAR *) cast_scalar_integer_real (p);
	errno = 0;
	s->v.real = tan (s->v.real);
	CHECK_MATH ();
	break;
      case real:
	s = (SCALAR *) dup_scalar (p);
	errno = 0;
	s->v.real = tan (s->v.real);
	CHECK_MATH ();
	break;
      case complex:
	{
	  COMPLEX z;
	  z = p->v.complex;
	  s = (SCALAR *) dup_scalar (p);
	  s->v.complex.real = s->v.complex.imag = 1.0 / (cos (2 * z.real) + cosh (2 * z.imag));
	  s->v.complex.real *= sin (2 * z.real);
	  s->v.complex.imag *= sinh (2 * z.imag);
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
tan_vector (VECTOR *p)
{
  VECTOR *m;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
      p = (VECTOR *) cast_vector_integer_real (p);
      m = (VECTOR *) apply_vector_real_real (tan, p);
      break;
    case real:
      m = (VECTOR *) apply_vector_real_real (tan, p);
      break;
    case complex:
      m = (VECTOR *) apply_vector_complex_complex (tan_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
tan_matrix (MATRIX *p)
{
  MATRIX *m;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
      p = (MATRIX *) cast_matrix_integer_real (p);
      m = (MATRIX *) apply_matrix_real_real (tan, p);
      break;
    case real:
      m = (MATRIX *) apply_matrix_real_real (tan, p);
      break;
    case complex:
      m = (MATRIX *) apply_matrix_complex_complex (tan_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (m));
}
