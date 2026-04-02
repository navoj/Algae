/*
   btrans.c -- Block transpose.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: btrans.c,v 1.1.1.1 1996/04/17 05:56:15 ksh Exp $";

#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "generate.h"
#include "partition.h"
#include "binop.h"
#include "cast.h"
#include "full.h"

ENTITY *
bi_btrans (ENTITY *a, ENTITY *nbr, ENTITY *nbc)
{
  /*
   * This routine transposes the matrix `a', but treates it as
   * if it contains `nbr' block rows and `nbc' block columns.
   * The individual blocks are moved, but are not themselves
   * transposed.  Thus `btrans([a,b;c,d];2;2)==[a,c;b,d]'.
   */

  MATRIX *t = NULL;
  MATRIX *brow = NULL;
  MATRIX *bcol = NULL;
  int nr, nc, nri, nci, i, j;

  EASSERT (a, 0, 0);
  EASSERT (nbr, 0, 0);
  EASSERT (nbc, 0, 0);

  WITH_HANDLING
  {

    /* It's got to be a matrix. */

    a = matrix_entity (EAT (a));

    /* Rows and columns must be scalar integers. */

    nbr = cast_scalar ((SCALAR *) scalar_entity (EAT (nbr)), integer);
    nbc = cast_scalar ((SCALAR *) scalar_entity (EAT (nbc)), integer);

    /* Check dimensions. */

    nr = ((SCALAR *) nbr)->v.integer;
    nc = ((SCALAR *) nbc)->v.integer;

    if (nr < 0 || (nr > 0 && ((MATRIX *) a)->nr % nr))
      {
	fail ("Can't partition matrix with %d row%s into %d block row%s.",
	  ((MATRIX *) a)->nr, PLURAL (((MATRIX *) a)->nr), nr, PLURAL (nr));
	raise_exception ();
      }
    nri = ((MATRIX *) a)->nr / ((nr == 0) ? 1 : nr);

    if (nc < 0 || (nc > 0 && ((MATRIX *) a)->nc % nc))
      {
	fail ("Can't partition matrix with %d column%s into %d block column%s.",
	  ((MATRIX *) a)->nc, PLURAL (((MATRIX *) a)->nc), nc, PLURAL (nc));
	raise_exception ();
      }
    nci = ((MATRIX *) a)->nc / ((nc == 0) ? 1 : nc);

    if (nr == 0 && nc == 0)
      {
	fail ("Can't partition matrix into 0 rows and 0 columns.");
	raise_exception ();
      }

    /*
     * This job is pretty easy if the matrix is dense.  Otherwise,
     * let's use a painfully inefficient approach and come back and
     * fix it some day.
     */

    if (((MATRIX *) a)->density == dense)
      {

	int k, s, nra;

	t = (MATRIX *) form_matrix (nc * nri, nr * nci,
				    ((MATRIX *) a)->type, dense);
	nra = ((MATRIX *) a)->nr;

	switch (((MATRIX *) a)->type)
	  {

	  case integer:
	    {
	      int *p = ((MATRIX *) a)->a.integer;
	      int *q = t->a.integer;

	      s = nri * type_size[integer];

	      for (i = 0; i < nr; i++)
		{
		  for (j = 0; j < nc; j++)
		    {
		      for (k = 0; k < nci; k++)
			{
			  memcpy (q + j * nri + (i * nci + k) * nri * nc,
				  p + i * nri + j * nra * nci + k * nra, s);
			}
		    }
		}
	    }
	    break;

	  case real:
	    {
	      REAL *p = ((MATRIX *) a)->a.real;
	      REAL *q = t->a.real;

	      s = nri * type_size[real];

	      for (i = 0; i < nr; i++)
		{
		  for (j = 0; j < nc; j++)
		    {
		      for (k = 0; k < nci; k++)
			{
			  memcpy (q + j * nri + (i * nci + k) * nri * nc,
				  p + i * nri + j * nra * nci + k * nra, s);
			}
		    }
		}
	    }
	    break;

	  case complex:
	    {
	      COMPLEX *p = ((MATRIX *) a)->a.complex;
	      COMPLEX *q = t->a.complex;

	      s = nri * type_size[complex];

	      for (i = 0; i < nr; i++)
		{
		  for (j = 0; j < nc; j++)
		    {
		      for (k = 0; k < nci; k++)
			{
			  memcpy (q + j * nri + (i * nci + k) * nri * nc,
				  p + i * nri + j * nra * nci + k * nra, s);
			}
		    }
		}
	    }
	    break;

	  case character:
	    {
	      char **p = ((MATRIX *) a)->a.character;
	      char **q = t->a.character;

	      s = nri * type_size[character];

	      for (i = 0; i < nr; i++)
		{
		  for (j = 0; j < nc; j++)
		    {
		      for (k = 0; k < nci; k++)
			{
			  memcpy (q + j * nri + (i * nci + k) * nri * nc,
				  p + i * nri + j * nra * nci + k * nra, s);
			}
		    }
		}
	      for (i = 0; i < t->nn; i++)
		q[i] = dup_char (q[i]);
	    }
	    break;

	  default:
	    wipeout ("Bad type.");

	  }

      }
    else
      {

	/* `partition_matrix' calls `full_matrix', so let's beat it to the punch. */

	a = full_matrix ((MATRIX *) EAT (a));

	/* OK, now do it. */

	for (i = 0; i < nc; i++)
	  {

	    bcol = (MATRIX *) partition_matrix_vector_vector (
				      (MATRIX *) copy_matrix ((MATRIX *) a),
							    (VECTOR *) NULL,
	       (VECTOR *) generate_integer (nci * i + 1, nci * (i + 1), 1));

	    brow = (MATRIX *) partition_matrix_vector_vector (
				   (MATRIX *) copy_matrix ((MATRIX *) bcol),
				    (VECTOR *) generate_integer (1, nri, 1),
							   (VECTOR *) NULL);
	    for (j = 1; j < nr; j++)
	      {
		brow = (MATRIX *) binop_entity (BO_APPEND, EAT (brow),
					    partition_matrix_vector_vector (
				   (MATRIX *) copy_matrix ((MATRIX *) bcol),
					       (VECTOR *) generate_integer (
					     nri * j + 1, nri * (j + 1), 1),
							  (VECTOR *) NULL));
	      }

	    t = (MATRIX *) ((t == NULL) ? (ENTITY *) EAT (brow) :
			    binop_entity (BO_PILE, EAT (t), EAT (brow)));
	    delete_matrix (bcol);
	  }
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (a, nbr, nbc);
    delete_2_matrices (t, brow);
  }
  END_EXCEPTION;

  delete_3_entities (a, nbr, nbc);
  delete_matrix (brow);
  return (ENT (t));
}
