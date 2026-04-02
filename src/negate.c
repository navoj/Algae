/*
   negate.c -- Negation.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: negate.c,v 1.1.1.1 1996/04/17 05:56:20 ksh Exp $";

#include "negate.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"

ENTITY *
negate_entity (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (negate_scalar ((SCALAR *) p));
    case vector:
      return (negate_vector ((VECTOR *) p));
    case matrix:
      return (negate_matrix ((MATRIX *) p));
    default:
      fail ("Can't negate a \"%s\" entity.", class_string[p->class]);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
negate_scalar (SCALAR *p)
{
  EASSERT (p, scalar, 0);

  p = (SCALAR *) dup_scalar (p);
  switch (p->type)
    {
    case integer:
      p->v.integer = -p->v.integer;
      break;
    case real:
      p->v.real = -p->v.real;
      break;
    case complex:
      p->v.complex.real = -p->v.complex.real;
      p->v.complex.imag = -p->v.complex.imag;
      break;
    case character:
      fail ("Can't negate a \"%s\" scalar.", type_string[p->type]);
      delete_scalar (p);
      raise_exception ();
    default:
      BAD_TYPE (p->type);
      delete_scalar (p);
      raise_exception ();
    }
  return (ENT (p));
}

ENTITY *
negate_vector (VECTOR *p)
{
  int i;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
      p = (VECTOR *) dup_vector (p);
      for (i = 0; i < p->nn; i++)
	p->a.integer[i] = -p->a.integer[i];
      break;
    case real:
      p = (VECTOR *) dup_vector (p);
      for (i = 0; i < p->nn; i++)
	p->a.real[i] = -p->a.real[i];
      break;
    case complex:
      p = (VECTOR *) dup_vector (p);
      for (i = 0; i < p->nn; i++)
	{
	  p->a.complex[i].real = -p->a.complex[i].real;
	  p->a.complex[i].imag = -p->a.complex[i].imag;
	}
      break;
    case character:
      fail ("Can't negate a \"%s\" vector.", type_string[p->type]);
      delete_vector (p);
      raise_exception ();
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (p));
}

ENTITY *
negate_matrix (MATRIX *p)
{
  int i;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
      p = (MATRIX *) dup_matrix (p);
      for (i = 0; i < p->nn; i++)
	p->a.integer[i] = -p->a.integer[i];
      if (p->d.integer != NULL)
	{
	  for (i = 0; i < p->nr; i++)
	    p->d.integer[i] = -p->d.integer[i];
	}
      break;
    case real:
      p = (MATRIX *) dup_matrix (p);
      for (i = 0; i < p->nn; i++)
	p->a.real[i] = -p->a.real[i];
      if (p->d.real != NULL)
	{
	  for (i = 0; i < p->nr; i++)
	    p->d.real[i] = -p->d.real[i];
	}
      break;
    case complex:
      p = (MATRIX *) dup_matrix (p);
      for (i = 0; i < p->nn; i++)
	{
	  p->a.complex[i].real = -p->a.complex[i].real;
	  p->a.complex[i].imag = -p->a.complex[i].imag;
	}
      if (p->d.complex != NULL)
	{
	  for (i = 0; i < p->nr; i++)
	    {
	      p->d.complex[i].real = -p->d.complex[i].real;
	      p->d.complex[i].imag = -p->d.complex[i].imag;
	    }
	}
      break;
    case character:
      fail ("Can't negate a \"%s\" matrix.", type_string[p->type]);
      delete_matrix (p);
      raise_exception ();
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (p));
}
