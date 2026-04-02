/*
   transform.c -- Linear matrix transformation.

   Copyright (C) 1994-2001  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: transform.c,v 1.4 2002/07/24 04:36:51 ksh Exp $";

#include "transform.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "multiply.h"
#include "transpose.h"
#include "cast.h"
#include "thin.h"
#include "dense.h"
#include "sparse.h"
#include "full.h"

ENTITY *
bi_transform (ENTITY *a, ENTITY *p)
{
  /*
   * This routine performs the multiplication P'*A*P for
   * the matrix P and the symmetric matrix A.
   */

  WITH_HANDLING
  {
    a = matrix_entity (EAT (a));
    p = matrix_entity (EAT (p));
  }
  ON_EXCEPTION
  {
    delete_2_entities (a, p);
  }
  END_EXCEPTION;

  return transform_matrix ((MATRIX *) a, (MATRIX *) p);
}

ENTITY *
transform_matrix (MATRIX *a, MATRIX *p)
{
  MATRIX *c = NULL;
  void *ad = NULL;
  void *work = NULL;
  int *ia = NULL;
  int i;
  SYMMETRY sym;
  TYPE p_type;

  EASSERT (a, matrix, 0);
  EASSERT (p, matrix, 0);

  WITH_HANDLING
    {
      p_type = p->type;
      AUTO_CAST_MATRIX (a, p);

      if (a->nr != a->nc)
	{
	  fail ("Coefficient matrix not square in \"transform\".");
	  raise_exception ();
	}

      if (p->nr != a->nc)
	{
	  fail ("Invalid dimensions for \"transform\".  Coefficient matrix has order %d, but transformation matrix has %d row%s.",
		a->nr, p->nr, PLURAL (p->nr));
	  raise_exception ();
	}

      sym = a->symmetry;

      switch (p->density)
	{
	case dense:

	  switch (a->type)
	    {
	    case integer:

	      switch (a->density)
		{
		case dense:

		  c = (MATRIX *) transpose_matrix ((MATRIX *) copy_matrix (p));
		  a = (MATRIX *) multiply_matrix_integer (EAT (c), EAT (a));
		  c = (MATRIX *) multiply_matrix_integer (EAT (a), EAT (p));
		  break;

		case sparse:

		  a = (MATRIX *) sparse_matrix ((MATRIX *) EAT (a));
		  if (a->density != sparse_upper)
		    {
		      c = (MATRIX *)
			transpose_matrix ((MATRIX *) copy_matrix (p));
		      a = (MATRIX *)
			multiply_matrix_integer (EAT (a), EAT (p));
		      c = (MATRIX *)
			multiply_matrix_integer (EAT (c), EAT (a));
		      break;
		    }

		case sparse_upper:	/* fall through */

		  if (!MATCH_VECTORS (a->rid, p->rid) ||
		      !MATCH_VECTORS (a->cid, p->rid))
		    {
		      fail ("Labels don't match in \"transform\".");
		      raise_exception ();
		    }

		  if (a->d.integer == NULL)
		    ad = E_CALLOC (a->nr, integer);
		  if (a->ia == NULL)
		    {
		      ia = E_MALLOC (a->nr + 1, integer);
		      for (i = 0; i < a->nr + 1; i++)
			ia[i] = 1;
		    }
		  work = E_MALLOC (p->nn, integer);

		  c = (MATRIX *) form_matrix (p->nc, p->nc, integer, dense);

		  ISSTRF (&a->nr, &p->nc, (a->ia) ? a->ia : ia, a->ja,
			  a->a.integer,
			  (a->d.integer) ? a->d.integer : (int *) ad,
			  p->a.integer, c->a.integer, (int *) work);

		  if (p->cid != NULL)
		    {
		      c->rid = copy_entity (p->cid);
		      c->cid = copy_entity (p->cid);
		    }

		  break;

		default:
		  BAD_DENSITY (a->density);
		  raise_exception ();
		}
	      break;

	    case real:

	      switch (a->density)
		{
		case dense:

		  c = (MATRIX *) transpose_matrix ((MATRIX *) copy_matrix (p));
		  a = (MATRIX *) multiply_matrix_real (EAT (c), EAT (a));
		  c = (MATRIX *) multiply_matrix_real (EAT (a), EAT (p));
		  break;

		case sparse:

		  a = (MATRIX *) sparse_matrix ((MATRIX *) EAT (a));
		  if (a->density != sparse_upper)
		    {
		      c = (MATRIX *)
			transpose_matrix ((MATRIX *) copy_matrix (p));
		      a = (MATRIX *) multiply_matrix_real (EAT (a), EAT (p));
		      c = (MATRIX *) multiply_matrix_real (EAT (c), EAT (a));
		      break;
		    }

		case sparse_upper:	/* fall through */

		  if (!MATCH_VECTORS (a->rid, p->rid) ||
		      !MATCH_VECTORS (a->cid, p->rid))
		    {
		      fail ("Labels don't match in \"transform\".");
		      raise_exception ();
		    }

		  if (a->d.real == NULL)
		    ad = E_CALLOC (a->nr, real);
		  if (a->ia == NULL)
		    {
		      ia = E_MALLOC (a->nr + 1, integer);
		      for (i = 0; i < a->nr + 1; i++)
			ia[i] = 1;
		    }
		  work = E_MALLOC (p->nn, real);

		  c = (MATRIX *) form_matrix (p->nc, p->nc, real, dense);

		  DSSTRF (&a->nr, &p->nc, (a->ia) ? a->ia : ia, a->ja,
			  a->a.real, (a->d.real) ? a->d.real : (REAL *) ad,
			  p->a.real, c->a.real, (REAL *) work);

		  if (p->cid != NULL)
		    {
		      c->rid = copy_entity (p->cid);
		      c->cid = copy_entity (p->cid);
		    }

		  break;

		default:
		  BAD_DENSITY (a->density);
		  raise_exception ();
		}
	      break;

	    case complex:

	      if (a->symmetry == symmetric && p_type == complex && p->nn)
		{
		  a = (MATRIX *) full_matrix (EAT (a));
		  a = (MATRIX *) dup_matrix (EAT (a));
		  sym = a->symmetry = general;
		}

	      switch (a->density)
		{
		case dense:

		  c = (MATRIX *) hermitian_transpose_matrix
		    ((MATRIX *) copy_matrix (p));
		  a = (MATRIX *) multiply_matrix_complex (EAT (c), EAT (a));
		  c = (MATRIX *) multiply_matrix_complex (EAT (a), EAT (p));
		  break;

		case sparse:

		  c = (MATRIX *) transpose_matrix ((MATRIX *) copy_matrix (p));
		  a = (MATRIX *) multiply_matrix_complex (EAT (a), EAT (p));
		  c = (MATRIX *) multiply_matrix_complex (EAT (c), EAT (a));
		  break;

		case sparse_upper:

		  if (!MATCH_VECTORS (a->rid, p->rid) ||
		      !MATCH_VECTORS (a->cid, p->rid))
		    {
		      fail ("Labels don't match in \"transform\".");
		      raise_exception ();
		    }

		  if (a->d.complex == NULL)
		    ad = E_CALLOC (a->nr, complex);
		  if (a->ia == NULL)
		    {
		      ia = E_MALLOC (a->nr + 1, integer);
		      for (i = 0; i < a->nr + 1; i++)
			ia[i] = 1;
		    }
		  work = E_MALLOC (p->nn, complex);

		  c = (MATRIX *) form_matrix (p->nc, p->nc, complex, dense);

		  ZHSTRF (&a->nr, &p->nc, (a->ia) ? a->ia : ia, a->ja,
			  a->a.complex, (a->d.complex) ?
			  a->d.complex : (COMPLEX *) ad,
			  p->a.complex, c->a.complex, (COMPLEX *) work);

		  if (p->cid != NULL)
		    {
		      c->rid = copy_entity (p->cid);
		      c->cid = copy_entity (p->cid);
		    }

		  break;

		default:
		  BAD_DENSITY (a->density);
		  raise_exception ();
		}
	      break;

	    default:
	      BAD_TYPE (a->type);
	      raise_exception ();
	    }

	  break;

	case sparse:
	case sparse_upper:

	  c = (MATRIX *) transpose_matrix ((MATRIX *) copy_matrix (p));

	  switch (a->type)
	    {
	    case integer:

	      a = (MATRIX *) multiply_matrix_integer (EAT (a), EAT (p));
	      c = (MATRIX *) multiply_matrix_integer (EAT (c), EAT (a));
	      break;

	    case real:

	      a = (MATRIX *) multiply_matrix_real (EAT (a), EAT (p));
	      c = (MATRIX *) multiply_matrix_real (EAT (c), EAT (a));
	      break;

	    case complex:

	      if (a->symmetry == symmetric && p_type == complex && p->nn)
		  sym = general;

	      a = (MATRIX *) multiply_matrix_complex (EAT (a), EAT (p));
	      c = (MATRIX *) multiply_matrix_complex (EAT (c), EAT (a));
	      break;

	    default:

	      BAD_TYPE (a->type);
	      raise_exception ();
	      break;
	    }

	  break;

	default:

	  BAD_DENSITY (p->density);
	  raise_exception ();
	}

      c->symmetry = sym;
    }
  ON_EXCEPTION
    {
      delete_3_matrices (a, p, c);
      TFREE (ad);
      TFREE (ia);
      TFREE (work);
    }
  END_EXCEPTION;

  delete_2_matrices (a, p);
  TFREE (ad);
  TFREE (ia);
  TFREE (work);

  return ENT (c);
}
