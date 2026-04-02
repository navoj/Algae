/*
   lose.c -- Lose vector elements.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: lose.c,v 1.1.1.1 1996/04/17 05:56:18 ksh Exp $";

#include "lose.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "two_type.h"
#include "cast.h"
#include "dense.h"

ENTITY *
bi_lose (ENTITY *ap, ENTITY *bp)
{
  /*
   * Returns the locations (that is, element numbers) of the elements
   * of vector `bp' that do not have the values given by the elements
   * of the vector `ap'.
   */

  EASSERT (ap, 0, 0);
  EASSERT (bp, 0, 0);

  switch (TWO_CLASS (ap, bp))
    {
    case scalar_scalar:
      return (lose_vector ((VECTOR *) scalar_to_vector ((SCALAR *) ap),
			   (VECTOR *) scalar_to_vector ((SCALAR *) bp)));
    case scalar_vector:
      return (lose_vector ((VECTOR *) scalar_to_vector ((SCALAR *) ap),
			   (VECTOR *) bp));
    case scalar_matrix:
      return (lose_vector ((VECTOR *) scalar_to_vector ((SCALAR *) ap),
			   (VECTOR *) matrix_to_vector ((MATRIX *) bp)));
    case vector_scalar:
      return (lose_vector ((VECTOR *) ap,
			   (VECTOR *) scalar_to_vector ((SCALAR *) bp)));
    case vector_vector:
      return (lose_vector ((VECTOR *) ap, (VECTOR *) bp));
    case vector_matrix:
      return (lose_vector ((VECTOR *) ap,
			   (VECTOR *) matrix_to_vector ((MATRIX *) bp)));
    case matrix_scalar:
      return (lose_vector ((VECTOR *) matrix_to_vector ((MATRIX *) ap),
			   (VECTOR *) scalar_to_vector ((SCALAR *) bp)));
    case matrix_vector:
      return (lose_vector ((VECTOR *) matrix_to_vector ((MATRIX *) ap),
			   (VECTOR *) bp));
    case matrix_matrix:
      return (lose_vector ((VECTOR *) matrix_to_vector ((MATRIX *) ap),
			   (VECTOR *) matrix_to_vector ((MATRIX *) bp)));
    default:
      fail ("Can't \"lose\" with a combination of %s and %s entities.",
	    class_string[ap->class], class_string[bp->class]);
      delete_2_entities (ap, bp);
      raise_exception ();
    }
}

ENTITY *
lose_vector (VECTOR *ap, VECTOR *bp)
{
  VECTOR *p;
  int nn, i, j;

  EASSERT (ap, vector, 0);
  EASSERT (bp, vector, 0);

  AUTO_CAST_VECTOR (ap, bp);	/* Cast to the same format. */

  ap = (VECTOR *) dense_vector (ap);	/* Convert to dense. */
  bp = (VECTOR *) dense_vector (bp);

  /* Initialize return matrix. */

  p = (VECTOR *) make_vector (bp->nn, integer, dense);

  switch (ap->type)
    {
    case integer:
      for (j = 0; j < bp->nn; j++)
	{
	  for (i = 0; i < ap->nn; i++)
	    {
	      if (ap->a.integer[i] == bp->a.integer[j])
		{
		  p->a.integer[j] = 1;
		}
	    }
	}
      break;
    case real:
      for (j = 0; j < bp->nn; j++)
	{
	  for (i = 0; i < ap->nn; i++)
	    {
	      if (ap->a.real[i] == bp->a.real[j])
		{
		  p->a.integer[j] = 1;
		}
	    }
	}
      break;
    case complex:
      for (j = 0; j < bp->nn; j++)
	{
	  for (i = 0; i < ap->nn; i++)
	    {
	      if (ap->a.complex[i].real == bp->a.complex[j].real &&
		  ap->a.complex[i].imag == bp->a.complex[j].imag)
		{
		  p->a.integer[j] = 1;
		}
	    }
	}
      break;
    case character:
      for (j = 0; j < bp->nn; j++)
	{
	  for (i = 0; i < ap->nn; i++)
	    {
	      if (!strcmp (ap->a.character[i], bp->a.character[j]))
		{
		  p->a.integer[j] = 1;
		}
	    }
	}
      break;
    default:
      BAD_TYPE (ap->type);
      delete_3_vectors (ap, bp, p);
      raise_exception ();
    }

  nn = 0;
  for (i = 0; i < p->nn; i++)
    {
      if (p->a.integer[i] == 0)
	p->a.integer[nn++] = i + 1;
    }

  if (nn != p->nn)
    {
      p->ne = p->nn = nn;
      if (nn > 0)
	{
	  p->a.integer = (int *) REALLOC (p->a.integer, nn * sizeof (int));
	}
      else
	{
	  FREE (p->a.integer);
	  p->a.integer = NULL;
	}
    }

  delete_2_vectors (ap, bp);
  return (ENT (p));
}
