/*
   abs.c -- Absolute value.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: abs.c,v 1.2 2003/08/01 04:57:46 ksh Exp $";

#include "abs.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "apply.h"

/* For systems without an `abs' function. */

#if !HAVE_ABS
int
abs (int i)
{
  return ((i > 0) ? i : -i);
}
#endif

REAL
abs_complex (COMPLEX z)
{
  /*
   * Complex magnitude.
   *
   * This function was adapted from the f2c "z_abs" function.
   * Copyright 1990 by AT&T Bell Laboratories and Bellcore.
   */

  REAL r, i, temp;

  r = z.real;
  i = z.imag;

  if (r < 0)
    r = -r;
  if (i < 0)
    i = -i;
  if (i > r)
    {
      temp = r;
      r = i;
      i = temp;
    }
  if ((r + i) == r)
    return r;

  temp = i / r;
  temp = r * sqrt (1.0 + temp * temp);

  return temp;
}

ENTITY *
bi_abs (ENTITY *p)
{
  /* The built-in absolute value function. */

  switch (p->class)
    {
    case scalar:
      return abs_scalar ((SCALAR *) p);
    case vector:
      return abs_vector ((VECTOR *) p);
    case matrix:
      return abs_matrix ((MATRIX *) p);
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
abs_scalar (SCALAR *p)
{
  SCALAR * volatile s = NULL;

  EASSERT (p, scalar, 0);

  WITH_HANDLING
  {
    switch (p->type)
      {
      case integer:
	s = (SCALAR *) dup_scalar (p);
	s->v.integer = abs (s->v.integer);
	break;
      case real:
	s = (SCALAR *) dup_scalar (p);
	s->v.real = fabs (s->v.real);
	break;
      case complex:
	s = (SCALAR *) dup_scalar (p);
	s->v.real = abs_complex (s->v.complex);
	s->type = real;
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

  return ENT (s);
}

ENTITY *
abs_vector (VECTOR *p)
{
  VECTOR *m;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
      m = (VECTOR *) apply_vector_integer_integer (abs, p);
      break;
    case real:
      m = (VECTOR *) apply_vector_real_real (fabs, p);
      break;
    case complex:
      m = (VECTOR *) apply_vector_real_complex (abs_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return ENT (m);
}

ENTITY *
abs_matrix (MATRIX *p)
{
  MATRIX *m;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
      m = (MATRIX *) apply_matrix_integer_integer (abs, p);
      break;
    case real:
      m = (MATRIX *) apply_matrix_real_real (fabs, p);
      break;
    case complex:
      m = (MATRIX *) apply_matrix_real_complex (abs_complex, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return ENT (m);
}
