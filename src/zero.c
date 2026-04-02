/*
   zero.c -- Matrix or vector of zeros.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: zero.c,v 1.1.1.1 1996/04/17 05:56:23 ksh Exp $";

#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "cast.h"
#include "partition.h"

ENTITY *
bi_zero (int n, ENTITY *shape)
{
  /*
   * This function returns an integer scalar, vector, or matrix
   * (depending on the vector `shape') whose elements are all zero.
   * If `shape' is NULL or a zero length vector, then a scalar is
   * returned.  If `shape' has one element, then a vector with
   * `shape[1]' elements is returned.  If `shape' has two elements,
   * then a matrix with `shape[1]' rows and `shape[2]' columns is
   * returned.  In the latter two cases, the return entity is sparse.
   */

  ENTITY *z;

  if (shape)
    {

      shape = cast_vector ((VECTOR *) vector_entity (shape), integer);

      switch (((VECTOR *) shape)->ne)
	{
	case 0:
	  z = make_scalar (integer);
	  delete_vector ((VECTOR *) shape);
	  break;
	case 1:
	  {
	    int ne = get_vector_element_integer ((VECTOR *) shape, 1);

	    if (ne < 0)
	      {
		fail ("Specified dimension is negative.");
		raise_exception ();
	      }
	    z = make_vector (ne, integer, sparse);
	  }
	  break;
	case 2:
	  {
	    int nr, nc;

	    nr = get_vector_element_integer
	      ((VECTOR *) copy_vector ((VECTOR *) shape), 1);
	    nc = get_vector_element_integer ((VECTOR *) shape, 2);

	    if (nr < 0 || nc < 0)
	      {
		fail ("Specified dimension is negative.");
		raise_exception ();
	      }

	    z = make_matrix (nr, nc, integer, sparse);
	  }
	  break;
	default:
	  fail ("Too many (%d) dimensions specified in \"zero\".",
		((VECTOR *) shape)->ne);
	  delete_vector ((VECTOR *) shape);
	  raise_exception ();
	}

    }
  else
    {

      /* `shape' is NULL, so just return scalar zero. */

      z = make_scalar (integer);
    }

  return (z);
}
