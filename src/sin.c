/*
   sin.c -- The sine function.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: sin.c,v 1.2 2003/08/01 04:57:48 ksh Exp $";

#include "sin.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "apply.h"
#include "cast.h"

COMPLEX
sin_complex (COMPLEX z)
{
  COMPLEX w;

  w.real = sin (z.real) * cosh (z.imag);
  w.imag = cos (z.real) * sinh (z.imag);

  return (w);
}

ENTITY *
bi_sin (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (sin_scalar ((SCALAR *) p));
    case vector:
      return (sin_vector ((VECTOR *) p));
    case matrix:
      return (sin_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
sin_scalar (SCALAR *p)
{
  SCALAR * volatile s = NULL;

  EASSERT (p, scalar, 0);

  WITH_HANDLING
  {
    switch (p->type)
      {
      case integer:
	s = (SCALAR *) cast_scalar_integer_real (p);
	s->v.real = sin (s->v.real);
	break;
      case real:
	s = (SCALAR *) dup_scalar (p);
	s->v.real = sin (s->v.real);
	break;
      case complex:
	{
	  COMPLEX z;
	  z = p->v.complex;
	  s = (SCALAR *) dup_scalar (p);
	  s->v.complex.real = sin (z.real) * cosh (z.imag);
	  s->v.complex.imag = cos (z.real) * sinh (z.imag);
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
sin_vector (VECTOR *p)
{
  VECTOR *m;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
      p = (VECTOR *) cast_vector_integer_real (p);
      m = (VECTOR *) apply_vector_real_real (sin, p);
      break;
    case real:
      m = (VECTOR *) apply_vector_real_real (sin, p);
      break;
    case complex:
      m = (VECTOR *) apply_vector_complex_complex (sin_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
sin_matrix (MATRIX *p)
{
  MATRIX *m;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
      p = (MATRIX *) cast_matrix_integer_real (p);
      m = (MATRIX *) apply_matrix_real_real (sin, p);
      break;
    case real:
      m = (MATRIX *) apply_matrix_real_real (sin, p);
      break;
    case complex:
      m = (MATRIX *) apply_matrix_complex_complex (sin_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (m));
}
