/*
   bdiag.c -- Block Diagonal.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: bdiag.c,v 1.1.1.1 1996/04/17 05:56:15 ksh Exp $";

#include "bdiag.h"
#include "entity.h"
#include "scalar.h"
#include "matrix.h"
#include "sparse.h"
#include "cast.h"
#include "partition.h"
#include "transpose.h"
#include "binop.h"
#include "generate.h"
#include "full.h"

ENTITY *
bi_bdiag (ENTITY *a, ENTITY *nbr, ENTITY *nbc)
{
  /*
   * This routine imitates `diag', but treats the input matrix `a'
   * as if it contains `nbr' block rows and `nbc' block columns.
   */

  MATRIX *d = NULL;
  MATRIX *b = NULL;
  int nr, nc, nri, nci, i, j, nn;

  WITH_HANDLING
  {

    a = matrix_entity (EAT (a));

    /* Rows and columns must be scalar integers. */

    nbr = cast_scalar ((SCALAR *) bi_scalar (1, EAT (nbr)), integer);
    nbc = cast_scalar ((SCALAR *) bi_scalar (1, EAT (nbc)), integer);

    /* Check dimensions. */

    nr = ((SCALAR *) nbr)->v.integer;
    nc = ((SCALAR *) nbc)->v.integer;

    if (nr < 0 || (nr > 0 && ((MATRIX *) a)->nr % nr))
      {
	fail ("Can't partition matrix with %d row%s into %d block row%s.",
	      ((MATRIX *) a)->nr, PLURAL (((MATRIX *) a)->nr),
	      nr, PLURAL (nr));
	raise_exception ();
      }
    nri = ((MATRIX *) a)->nr / ((nr == 0) ? 1 : nr);

    if (nc < 0 || (nc > 0 && ((MATRIX *) a)->nc % nc))
      {
	fail ("Can't partition matrix with %d column%s into %d block column%s.",
	      ((MATRIX *) a)->nc, PLURAL (((MATRIX *) a)->nc),
	      nc, PLURAL (nc));
	raise_exception ();
      }
    nci = ((MATRIX *) a)->nc / ((nc == 0) ? 1 : nc);

    if (nr == 0 && nc == 0)
      {
	fail ("Can't partition matrix into 0 rows and 0 columns.");
	raise_exception ();
      }

    /* OK, now do it. */

    if (nc == 0)
      {

	nc = 1;

	a = full_matrix ((MATRIX *) sparse_matrix ((MATRIX *) EAT (a)));
	nn = ((MATRIX *) a)->nn;
	d = (MATRIX *) make_matrix (nri * nr, nci * nr,
				    ((MATRIX *) a)->type, sparse);
	d->symmetry = general;
	d->nn = nn;

	if (nn > 0)
	  {
	    d->ia = (int *) dup_mem (((MATRIX *) a)->ia,
				     (d->nr + 1) * sizeof (int));
	    d->a.ptr = dup_mem (((MATRIX *) a)->a.ptr,
				nn * type_size[d->type]);
	    d->ja = (int *) dup_mem (((MATRIX *) a)->ja,
				     nn * sizeof (int));
	    for (i = 0; i < nr; i++)
	      {
		for (j = d->ia[i * nri] - 1; j < d->ia[(i + 1) * nri] - 1; j++)
		  {
		    d->ja[j] += i * nci;
		  }
	      }
	  }

	if (((MATRIX *) a)->rid != NULL)
	  {
	    d->rid = copy_entity (((MATRIX *) a)->rid);
	  }
	if (((MATRIX *) a)->cid != NULL)
	  {
	    d->cid = copy_entity (((MATRIX *) a)->cid);
	    for (i = 1; i < nr; i++)
	      d->cid = binop_entity (BO_APPEND, d->cid,
				     copy_entity (((MATRIX *) a)->cid));
	  }

      }
    else if (nr == 0)
      {

	nr = 1;
	a = transpose_matrix ((MATRIX *) EAT (a));
	a = sparse_matrix ((MATRIX *) EAT (a));
	a = full_matrix ((MATRIX *) EAT (a));
	nn = ((MATRIX *) a)->nn;

	d = (MATRIX *) make_matrix (nci * nc, nri * nc,
				    ((MATRIX *) a)->type, sparse);
	d->symmetry = general;
	d->nn = nn;

	if (nn > 0)
	  {
	    d->ia = (int *) dup_mem (((MATRIX *) a)->ia,
				     (d->nr + 1) * sizeof (int));
	    d->a.ptr = dup_mem (((MATRIX *) a)->a.ptr,
				nn * type_size[d->type]);
	    d->ja = (int *) dup_mem (((MATRIX *) a)->ja,
				     nn * sizeof (int));
	    for (i = 0; i < nc; i++)
	      {
		for (j = d->ia[i * nci] - 1; j < d->ia[(i + 1) * nci] - 1; j++)
		  {
		    d->ja[j] += i * nri;
		  }
	      }
	  }

	d = (MATRIX *) transpose_matrix ((MATRIX *) EAT (d));

	if (((MATRIX *) a)->rid != NULL)
	  {
	    d->rid = copy_entity (((MATRIX *) a)->rid);
	    for (i = 1; i < nc; i++)
	      d->rid = binop_entity (BO_APPEND, d->rid,
				     copy_entity (((MATRIX *) a)->rid));
	  }
	if (((MATRIX *) a)->cid != NULL)
	  {
	    d->cid = copy_entity (((MATRIX *) a)->cid);
	  }

      }
    else
      {
	for (i = 0; i < ((nr > nc) ? nc : nr); i++)
	  {
	    b = (MATRIX *) partition_matrix_vector_vector (
						 (MATRIX *) copy_entity (a),
		(VECTOR *) generate_integer (nri * i + 1, nri * (i + 1), 1),
	       (VECTOR *) generate_integer (nci * i + 1, nci * (i + 1), 1));
	    d = (MATRIX *) ((d == NULL) ?
			    dup_matrix ((MATRIX *) EAT (b)) :
			    binop_entity (BO_APPEND, EAT (d), EAT (b)));
	  }

      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (a, nbr, nbc);
    delete_2_matrices (b, d);
  }
  END_EXCEPTION;

  delete_3_entities (a, nbr, nbc);
  delete_matrix (b);
  return (ENT (d));
}
