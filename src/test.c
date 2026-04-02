/*
   test.c -- Test for nonzero.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: test.c,v 1.1.1.1 1996/04/17 05:56:22 ksh Exp $";

#include "test.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "function.h"

ENTITY *
bi_test (ENTITY *p)
{
  if (!p)
    return (int_to_scalar (0));

  switch (p->class)
    {
    case scalar:
      return (int_to_scalar (test_scalar ((SCALAR *) p)));
    case vector:
      return (int_to_scalar (test_vector ((VECTOR *) p)));
    case matrix:
      return (int_to_scalar (test_matrix ((MATRIX *) p)));
    case table:
      return (int_to_scalar (test_table ((TABLE *) p)));
    case function:
      delete_function ((FUNCTION *) p);
      return (int_to_scalar (1));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

int
test_entity (ENTITY *p)
{
  switch (p->class)
    {
    case scalar:
      return (test_scalar ((SCALAR *) p));
    case vector:
      return (test_vector ((VECTOR *) p));
    case matrix:
      return (test_matrix ((MATRIX *) p));
    case table:
      return (test_table ((TABLE *) p));
    case function:
      delete_function ((FUNCTION *) p);
      return (1);
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

int
test_scalar (SCALAR *p)
{
  int t;

  EASSERT (p, scalar, 0);

  switch (p->type)
    {
    case integer:
      t = (p->v.integer != 0);
      break;
    case real:
      t = (p->v.real != 0.0);
      break;
    case complex:
      t = (p->v.complex.real != 0.0 || p->v.complex.imag != 0.0);
      break;
    case character:
      t = (*p->v.character != '\0');
      break;
    default:
      BAD_TYPE (p->type);
      delete_scalar (p);
      raise_exception ();
    }
  delete_scalar (p);
  return (t);
}

int
test_vector (VECTOR *p)
{
  int i;
  int t = 0;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
      for (i = 0; i < p->nn; i++)
	if (p->a.integer[i] != 0)
	  {
	    t = !0;
	    break;
	  }
      break;
    case real:
      for (i = 0; i < p->nn; i++)
	if (p->a.real[i] != 0.0)
	  {
	    t = !0;
	    break;
	  }
      break;
    case complex:
      for (i = 0; i < p->nn; i++)
	{
	  if (p->a.complex[i].real != 0.0 ||
	      p->a.complex[i].imag != 0.0)
	    {
	      t = !0;
	      break;
	    }
	}
      break;
    case character:
      for (i = 0; i < p->nn; i++)
	if (*p->a.character[i] != '\0')
	  {
	    t = !0;
	    break;
	  }
      break;
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  delete_vector (p);
  return (t);
}

int
test_matrix (MATRIX *p)
{
  int i;
  int t = 0;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
      for (i = 0; i < p->nn; i++)
	if (p->a.integer[i] != 0)
	  {
	    t = !0;
	    goto bye;
	  }
      if (p->d.integer != NULL)
	for (i = 0; i < p->nr; i++)
	  if (p->d.integer[i] != 0)
	    {
	      t = !0;
	      goto bye;
	    }
      break;
    case real:
      for (i = 0; i < p->nn; i++)
	if (p->a.real[i] != 0.0)
	  {
	    t = !0;
	    goto bye;
	  }
      if (p->d.real != NULL)
	for (i = 0; i < p->nr; i++)
	  if (p->d.real[i] != 0.0)
	    {
	      t = !0;
	      goto bye;
	    }
      break;
    case complex:
      for (i = 0; i < p->nn; i++)
	if (p->a.complex[i].real != 0 ||
	    p->a.complex[i].imag)
	  {
	    t = !0;
	    goto bye;
	  }
      if (p->d.complex != NULL)
	for (i = 0; i < p->nr; i++)
	  if (p->d.complex[i].real != 0 ||
	      p->d.complex[i].imag)
	    {
	      t = !0;
	      goto bye;
	    }
      break;
    case character:
      for (i = 0; i < p->nn; i++)
	if (*p->a.character[i] != '\0')
	  {
	    t = !0;
	    goto bye;
	  }
      if (p->d.character != NULL)
	for (i = 0; i < p->nr; i++)
	  if (*p->d.character[i] != '\0')
	    {
	      t = !0;
	      goto bye;
	    }
      break;
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
bye:
  delete_matrix (p);
  return (t);
}

int
test_table (TABLE *p)
{
  int t;

  EASSERT (p, table, 0);

  t = (p->nm != 0);

  delete_table (p);
  return (t);
}

int
test_function (FUNCTION *p)
{
  delete_function (p);
  return (1);
}
