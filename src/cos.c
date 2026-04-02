/*
   cos.c -- The cosine function.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: cos.c,v 1.2 2003/08/01 04:57:47 ksh Exp $";

#include "cos.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "apply.h"
#include "cast.h"
#include "dense.h"

COMPLEX
cos_complex (COMPLEX z)
{
  COMPLEX w;

  w.real = cos (z.real) * cosh (z.imag);
  w.imag = -sin (z.real) * sinh (z.imag);

  return w;
}

ENTITY *
bi_cos (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (cos_scalar ((SCALAR *) p));
    case vector:
      return (cos_vector ((VECTOR *) p));
    case matrix:
      return (cos_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
cos_scalar (SCALAR *p)
{
  SCALAR * volatile s = NULL;

  EASSERT (p, scalar, 0);

  WITH_HANDLING
  {
    switch (p->type)
      {
      case integer:
	s = (SCALAR *) cast_scalar_integer_real (p);
	s->v.real = cos (s->v.real);
	break;
      case real:
	s = (SCALAR *) dup_scalar (p);
	s->v.real = cos (s->v.real);
	break;
      case complex:
	{
	  COMPLEX z;
	  z = p->v.complex;
	  s = (SCALAR *) dup_scalar (p);
	  s->v.complex = cos_complex (z);
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
cos_vector (VECTOR *p)
{
  VECTOR *m;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
      p = (VECTOR *) cast_vector_integer_real (p);
      p = (VECTOR *) dense_vector (p);
      m = (VECTOR *) apply_vector_real_real (cos, p);
      break;
    case real:
      p = (VECTOR *) dense_vector (p);
      m = (VECTOR *) apply_vector_real_real (cos, p);
      break;
    case complex:
      p = (VECTOR *) dense_vector (p);
      m = (VECTOR *) apply_vector_complex_complex (cos_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
cos_matrix (MATRIX *p)
{
  MATRIX *m;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
      p = (MATRIX *) cast_matrix_integer_real (p);
      p = (MATRIX *) dense_matrix (p);
      m = (MATRIX *) apply_matrix_real_real (cos, p);
      break;
    case real:
      p = (MATRIX *) dense_matrix (p);
      m = (MATRIX *) apply_matrix_real_real (cos, p);
      break;
    case complex:
      p = (MATRIX *) dense_matrix (p);
      m = (MATRIX *) apply_matrix_complex_complex (cos_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (m));
}
