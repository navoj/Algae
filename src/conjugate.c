/*
   conjugate.c -- Complex conjugate.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: conjugate.c,v 1.1.1.1 1996/04/17 05:56:16 ksh Exp $";

#include "conjugate.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"

ENTITY *
bi_conjugate (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (conjugate_scalar ((SCALAR *) p));
    case vector:
      return (conjugate_vector ((VECTOR *) p));
    case matrix:
      return (conjugate_matrix ((MATRIX *) p));
    default:
      fail ("Can't find conjugate of a \"%s\" entity.", class_string[p->class]);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
conjugate_scalar (SCALAR *p)
{
  EASSERT (p, scalar, 0);

  switch (p->type)
    {
    case integer:
    case real:
      break;
    case complex:
      if (p->v.complex.imag != 0.0)
	{
	  p = (SCALAR *) dup_scalar (p);
	  p->v.complex.imag = -p->v.complex.imag;
	}
      break;
    default:
      fail ("Can't find conjugate of a \"%s\" scalar.", type_string[p->type]);
      delete_scalar (p);
      raise_exception ();
    }
  return (ENT (p));
}

ENTITY *
conjugate_vector (VECTOR *p)
{
  int i;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
    case real:
      break;
    case complex:
      p = (VECTOR *) dup_vector (p);
      for (i = 0; i < p->nn; i++)
	{
	  p->a.complex[i].imag = -p->a.complex[i].imag;
	}
      break;
    default:
      fail ("Can't find conjugate of a \"%s\" vector.", type_string[p->type]);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (p));
}

ENTITY *
conjugate_matrix (MATRIX *p)
{
  int i;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
    case real:
      break;
    case complex:
      p = (MATRIX *) dup_matrix (p);
      for (i = 0; i < p->nn; i++)
	{
	  p->a.complex[i].imag = -p->a.complex[i].imag;
	}
      if (p->d.complex != NULL)
	{
	  for (i = 0; i < p->nr; i++)
	    {
	      p->d.complex[i].imag = -p->d.complex[i].imag;
	    }
	}
      break;
    default:
      fail ("Can't find conjugate of a \"%s\" matrix.", type_string[p->type]);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (p));
}
