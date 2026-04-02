/*
   symmetric.c -- Return symmetric part of a matrix.

   Copyright (C) 1994-96  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: symmetric.c,v 1.3 2001/10/26 06:18:23 ksh Exp $";

#include "symmetric.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "binop.h"
#include "transpose.h"
#include "diag.h"
#include "conjugate.h"
#include "cast.h"

ENTITY *
bi_symmetric (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (ENT (p));
    case vector:
      return symmetric_matrix ((MATRIX *) vector_to_matrix ((VECTOR *) p));
    case matrix:
      return (symmetric_matrix ((MATRIX *) p));
    default:
      fail ("Can't find symmetric part of a \"%s\" entity.",
	    class_string[p->class]);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
bi_hermitian (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (ENT (p));
    case vector:
      return hermitian_matrix ((MATRIX *) vector_to_matrix ((VECTOR *) p));
    case matrix:
      return (hermitian_matrix ((MATRIX *) p));
    default:
      fail ("Can't find hermitian part of a \"%s\" entity.",
	    class_string[p->class]);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
symmetric_matrix (MATRIX *p)
{
  ENTITY *t = NULL;

  EASSERT (p, matrix, 0);

  switch (p->symmetry)
    {
    case symmetric:
      return (ENT (p));
    case general:
    case hermitian:

      WITH_HANDLING
      {

	if (p->nr != p->nc)
	  {
	    fail ("Can't find the symmetric part of a non-square matrix.");
	    raise_exception ();
	  }
	if (!MATCH_VECTORS (p->rid, p->cid))
	  {
	    fail ("Labels don't match.");
	    raise_exception ();
	  }
	if (p->type == character)
	  {
	    fail ("Can't find the symmetric part of a character matrix.");
	    raise_exception ();
	  }

	/*  Now we just add the matrix to its transpose & divide by 2. */

	t = transpose_matrix ((MATRIX *) copy_matrix (p));
	t = binop_matrix_matrix (BO_ADD, EAT (p), EAT (t));
	t = binop_scalar_matrix (BO_MUL,
				 (SCALAR *) real_to_scalar (0.5),
				 EAT (t));
	assert (t->ref_count == 1);
	((MATRIX *) t)->symmetry = symmetric;

      }
      ON_EXCEPTION
      {
	delete_matrix (p);
	delete_entity (t);
      }
      END_EXCEPTION;

      break;
    default:
      BAD_SYMMETRY (p->symmetry);
      raise_exception ();
    }
  return (t);
}

ENTITY *
hermitian_matrix (MATRIX *p)
{
  ENTITY *t = NULL;

  EASSERT (p, matrix, 0);

  if (p->type == integer || p->type == real)
    return symmetric_matrix (p);

  switch (p->symmetry)
    {

    case hermitian:
      return (ENT (p));

    case general:
    case symmetric:

      WITH_HANDLING
      {

	if (p->nr != p->nc)
	  {
	    fail ("Can't find the hermitian part of a non-square matrix.");
	    raise_exception ();
	  }
	if (!MATCH_VECTORS (p->rid, p->cid))
	  {
	    fail ("Labels don't match.");
	    raise_exception ();
	  }
	if (p->type == character)
	  {
	    fail ("Can't find the hermitian part of a character matrix.");
	    raise_exception ();
	  }

	/* Now add the matrix to its hermitian transpose & divide by 2. */

	t = hermitian_transpose_matrix ((MATRIX *) copy_matrix (p));
	t = cast_matrix (EAT (t), complex);
	t = binop_matrix_matrix (BO_ADD, EAT (p), EAT (t));
	t = binop_scalar_matrix (BO_MUL,
				 (SCALAR *) real_to_scalar (0.5),
				 EAT (t));
	assert (t->ref_count == 1);
	((MATRIX *) t)->symmetry = hermitian;

      }
      ON_EXCEPTION
      {
	delete_matrix (p);
	delete_entity (t);
      }
      END_EXCEPTION;

      break;

    default:
      BAD_SYMMETRY (p->symmetry);
      raise_exception ();
    }

  return (t);
}
