/*
   real.c -- Real part of a complex number.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: real.c,v 1.1.1.1 1996/04/17 05:56:21 ksh Exp $";

#include "real.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "apply.h"
#include "cast.h"

ENTITY *
bi_real (ENTITY *p)
{
  /* Convert to real. */

  EASSERT (p, 0, 0);

  return (cast_entity (p, real));
}

COMPLEX
ceil_complex (COMPLEX z)
{
  /* Ceiling of a complex number:  ceil(z) = ceil(x)+i*ceil(y) */

  z.real = ceil (z.real);
  z.imag = ceil (z.imag);

  return z;
}

ENTITY *
bi_ceil (ENTITY *p)
{
  /* Compute ceiling of an entity. */

  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (ceil_scalar ((SCALAR *) p));
    case vector:
      return (ceil_vector ((VECTOR *) p));
    case matrix:
      return (ceil_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
ceil_scalar (SCALAR *p)
{
  EASSERT (p, scalar, 0);

  switch (p->type)
    {

    case integer:

      break;

    case real:

      p = (SCALAR *) dup_scalar (p);
      p->v.real = ceil (p->v.real);
      break;

    case complex:

      p = (SCALAR *) dup_scalar (p);
      p->v.complex.real = ceil (p->v.complex.real);
      p->v.complex.imag = ceil (p->v.complex.imag);
      break;

    default:

      BAD_TYPE (p->type);
      delete_scalar (p);
      raise_exception ();
    }

  return (ENT (p));
}

ENTITY *
ceil_vector (VECTOR *p)
{
  /* Ceiling of a vector. */

  EASSERT (p, vector, 0);

  switch (p->type)
    {

    case integer:

      break;

    case real:

      p = (VECTOR *) apply_vector_real_real (ceil, p);
      break;

    case complex:

      p = (VECTOR *) apply_vector_complex_complex (ceil_complex, p);
      break;

    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (p));
}

ENTITY *
ceil_matrix (MATRIX *p)
{
  /* Ceiling of a matrix. */

  EASSERT (p, matrix, 0);

  switch (p->type)
    {

    case integer:

      break;

    case real:

      p = (MATRIX *) apply_matrix_real_real (ceil, p);
      break;

    case complex:

      p = (MATRIX *) apply_matrix_complex_complex (ceil_complex, p);
      break;

    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (p));
}

COMPLEX
floor_complex (COMPLEX z)
{
  /* Floor of a complex number:  floor(z) = floor(x)+i*floor(y) */

  z.real = floor (z.real);
  z.imag = floor (z.imag);

  return z;
}

ENTITY *
bi_floor (ENTITY *p)
{
  /* Compute floor of an entity. */

  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (floor_scalar ((SCALAR *) p));
    case vector:
      return (floor_vector ((VECTOR *) p));
    case matrix:
      return (floor_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
floor_scalar (SCALAR *p)
{
  EASSERT (p, scalar, 0);

  switch (p->type)
    {

    case integer:

      break;

    case real:

      p = (SCALAR *) dup_scalar (p);
      p->v.real = floor (p->v.real);
      break;

    case complex:

      p = (SCALAR *) dup_scalar (p);
      p->v.complex.real = floor (p->v.complex.real);
      p->v.complex.imag = floor (p->v.complex.imag);
      break;

    default:

      BAD_TYPE (p->type);
      delete_scalar (p);
      raise_exception ();
    }

  return (ENT (p));
}

ENTITY *
floor_vector (VECTOR *p)
{
  /* Floor of a vector. */

  EASSERT (p, vector, 0);

  switch (p->type)
    {

    case integer:

      break;

    case real:

      p = (VECTOR *) apply_vector_real_real (floor, p);
      break;

    case complex:

      p = (VECTOR *) apply_vector_complex_complex (floor_complex, p);
      break;

    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (p));
}

ENTITY *
floor_matrix (MATRIX *p)
{
  /* Floor of a matrix. */

  EASSERT (p, matrix, 0);

  switch (p->type)
    {

    case integer:

      break;

    case real:

      p = (MATRIX *) apply_matrix_real_real (floor, p);
      break;

    case complex:

      p = (MATRIX *) apply_matrix_complex_complex (floor_complex, p);
      break;

    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (p));
}

#if ! HAVE_RINT
REAL
round_real (REAL r)
{
  /* Round a real number. */

  return round (r);
}
#endif

COMPLEX
round_complex (COMPLEX z)
{
  /* Round a complex number:  round(z) = round(x)+i*round(y) */

  z.real = round (z.real);
  z.imag = round (z.imag);

  return z;
}

ENTITY *
bi_round (ENTITY *p)
{
  /* Round a numeric entity. */

  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (round_scalar ((SCALAR *) p));
    case vector:
      return (round_vector ((VECTOR *) p));
    case matrix:
      return (round_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
round_scalar (SCALAR *p)
{
  /* Round a scalar. */

  EASSERT (p, scalar, 0);

  switch (p->type)
    {

    case integer:

      break;

    case real:

      p = (SCALAR *) dup_scalar (p);
      p->v.real = round (p->v.real);
      break;

    case complex:

      p = (SCALAR *) dup_scalar (p);
      p->v.complex.real = round (p->v.complex.real);
      p->v.complex.imag = round (p->v.complex.imag);
      break;

    default:

      BAD_TYPE (p->type);
      delete_scalar (p);
      raise_exception ();
    }

  return (ENT (p));
}

ENTITY *
round_vector (VECTOR *p)
{
  /* Round a vector. */

  EASSERT (p, vector, 0);

  switch (p->type)
    {

    case integer:

      break;

    case real:

#if HAVE_RINT
      p = (VECTOR *) apply_vector_real_real (rint, p);
#else
      p = (VECTOR *) apply_vector_real_real (round_real, p);
#endif
      break;

    case complex:

      p = (VECTOR *) apply_vector_complex_complex (round_complex, p);
      break;

    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (p));
}

ENTITY *
round_matrix (MATRIX *p)
{
  /* Round a matrix. */

  EASSERT (p, matrix, 0);

  switch (p->type)
    {

    case integer:

      break;

    case real:

#if HAVE_RINT
      p = (MATRIX *) apply_matrix_real_real (rint, p);
#else
      p = (MATRIX *) apply_matrix_real_real (round_real, p);
#endif
      break;

    case complex:

      p = (MATRIX *) apply_matrix_complex_complex (round_complex, p);
      break;

    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (p));
}
