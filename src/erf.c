/*
   erf.c -- The error function.

   Copyright (C) 2003  K. Scott Hunziker.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: erf.c,v 1.2 2003/12/06 04:43:32 ksh Exp $";

#include "erf.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "apply.h"
#include "cast.h"
#include "dense.h"

ENTITY *
bi_erf (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (erf_scalar ((SCALAR *) p));
    case vector:
      return (erf_vector ((VECTOR *) p));
    case matrix:
      return (erf_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
erf_scalar (SCALAR *p)
{
  SCALAR * volatile s = NULL;

  EASSERT (p, scalar, 0);

  WITH_HANDLING
  {
    switch (p->type)
      {
      case integer:
	s = (SCALAR *) cast_scalar_integer_real (p);
	s->v.real = erf (s->v.real);
	break;
      case real:
	s = (SCALAR *) dup_scalar (p);
	s->v.real = erf (s->v.real);
	break;
      case complex:
	/* not yet */
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
erf_vector (VECTOR *p)
{
  VECTOR *m;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
      p = (VECTOR *) cast_vector_integer_real (p);
      m = (VECTOR *) apply_vector_real_real (erf, p);
      break;
    case real:
      m = (VECTOR *) apply_vector_real_real (erf, p);
      break;
    case complex:
      /* not yet */
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return ENT (m);
}

ENTITY *
erf_matrix (MATRIX *p)
{
  MATRIX *m;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
      p = (MATRIX *) cast_matrix_integer_real (p);
      m = (MATRIX *) apply_matrix_real_real (erf, p);
      break;
    case real:
      m = (MATRIX *) apply_matrix_real_real (erf, p);
      break;
    case complex:
      /* not yet */
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
bi_erfc (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (erfc_scalar ((SCALAR *) p));
    case vector:
      return (erfc_vector ((VECTOR *) p));
    case matrix:
      return (erfc_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
erfc_scalar (SCALAR *p)
{
  SCALAR * volatile s = NULL;

  EASSERT (p, scalar, 0);

  WITH_HANDLING
  {
    switch (p->type)
      {
      case integer:
	s = (SCALAR *) cast_scalar_integer_real (p);
	s->v.real = erfc (s->v.real);
	break;
      case real:
	s = (SCALAR *) dup_scalar (p);
	s->v.real = erfc (s->v.real);
	break;
      case complex:
	/* not yet */
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
erfc_vector (VECTOR *p)
{
  VECTOR *m;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
      p = (VECTOR *) cast_vector_integer_real (p);
      p = (VECTOR *) dense_vector (p);
      m = (VECTOR *) apply_vector_real_real (erfc, p);
      break;
    case real:
      p = (VECTOR *) dense_vector (p);
      m = (VECTOR *) apply_vector_real_real (erfc, p);
      break;
    case complex:
	/* not yet */
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return ENT (m);
}

ENTITY *
erfc_matrix (MATRIX *p)
{
  MATRIX *m;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
      p = (MATRIX *) cast_matrix_integer_real (p);
      p = (MATRIX *) dense_matrix (p);
      m = (MATRIX *) apply_matrix_real_real (erfc, p);
      break;
    case real:
      p = (MATRIX *) dense_matrix (p);
      m = (MATRIX *) apply_matrix_real_real (erfc, p);
      break;
    case complex:
	/* not yet */
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return ENT (m);
}
