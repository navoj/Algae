/*
   product.c -- Element-by-element multiplication.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: product.c,v 1.1.1.1 1996/04/17 05:56:21 ksh Exp $";

#include "product.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "solve.h"
#include "transpose.h"
#include "blas.h"
#include "cast.h"
#include "full.h"
#include "sparse.h"

static char bad_labels[] = "Labels don't match.";

#define COPY_EID( x, l, r )	(x)->eid ? (x)->eid : \
					   ( ( (l) ? (l)->eid : NULL ) ? \
					     copy_entity((l)->eid) : \
					     ( (r) ? (r)->eid : NULL ) ? \
					       copy_entity((r)->eid) : NULL )
#define COPY_RID( x, l, r )	(x)->rid ? (x)->rid : \
					   ( ( (l) ? (l)->rid : NULL ) ? \
					     copy_entity((l)->rid) : \
					     ( (r) ? (r)->rid : NULL ) ? \
					       copy_entity((r)->rid) : NULL )
#define COPY_CID( x, l, r )	(x)->cid ? (x)->cid : \
					   ( ( (l) ? (l)->cid : NULL ) ? \
					     copy_entity((l)->cid) : \
					     ( (r) ? (r)->cid : NULL ) ? \
					       copy_entity((r)->cid) : NULL )

ENTITY *
product_matrix_integer (MATRIX *l, MATRIX *r)
{
  /* Element-by-element multiplication. */

  MATRIX *x = NULL;
  int i, j, k, km;
  SYMMETRY sym;

  EASSERT (l, matrix, integer);
  EASSERT (r, matrix, integer);

  assert (l->order == ordered);
  assert (r->order == ordered);

  WITH_HANDLING
  {

    if (l->nr != r->nr)
      {
	fail ("Inconsistent dimensions for matrix product.  Left matrix has %d row%s, but right matrix has %d row%s.\n",
	      l->nr, PLURAL (l->nr), r->nr, PLURAL (r->nr));
	raise_exception ();
      }

    if (l->nc != r->nc)
      {
	fail ("Inconsistent dimensions for matrix product.  Left matrix has %d column%s, but right matrix has %d column%s.\n",
	      l->nc, PLURAL (l->nc), r->nc, PLURAL (r->nc));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->rid, r->rid) ||
	!MATCH_VECTORS (l->cid, r->cid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    sym = (l->symmetry == symmetric && r->symmetry == symmetric) ?
      symmetric : general;
    assert (l->symmetry != hermitian && r->symmetry != hermitian);

    switch (l->density)
      {

      case dense:

	switch (r->density)
	  {

	  case dense:

	    if (OWN (l))
	      {
		x = (MATRIX *) dup_matrix (EAT (l));
		for (i = 0; i < r->nn; i++)
		  x->a.integer[i] *= r->a.integer[i];
	      }
	    else
	      {
		x = (MATRIX *) dup_matrix (EAT (r));
		for (i = 0; i < l->nn; i++)
		  x->a.integer[i] *= l->a.integer[i];
	      }
	    x->symmetry = sym;

	    break;

	  case sparse:

	    x = (MATRIX *) dup_matrix (EAT (r));
	    if (x->nn > 0)
	      {
		for (i = 0; i < x->nr; i++)
		  {
		    for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
		      {
			x->a.integer[j] *=
			  l->a.integer[l->nr * (x->ja[j] - 1) + i];
		      }
		  }
	      }
	    x->symmetry = sym;

	    break;

	  case sparse_upper:

	    if (sym == symmetric)
	      {

		x = (MATRIX *) dup_matrix (EAT (r));
		if (x->nn > 0)
		  {
		    for (i = 0; i < x->nr; i++)
		      {
			for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
			  {
			    x->a.integer[j] *=
			      l->a.integer[l->nr * (x->ja[j] - 1) + i];
			  }
		      }
		  }
		if (x->d.integer)
		  {
		    for (i = 0; i < x->nr; i++)
		      {
			x->d.integer[i] *= l->a.integer[i * (l->nr + 1)];
		      }
		  }

	      }
	    else
	      {

		r = (MATRIX *) full_matrix (EAT (r));
		x = (MATRIX *) product_matrix_integer (EAT (l), EAT (r));

	      }

	    break;

	  default:

	    wipeout ("Bad density.");

	  }

	break;

      case sparse:

	switch (r->density)
	  {

	  case dense:

	    x = (MATRIX *) dup_matrix (EAT (l));
	    if (x->nn > 0)
	      {
		for (i = 0; i < x->nr; i++)
		  {
		    for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
		      {
			x->a.integer[j] *=
			  r->a.integer[r->nr * (x->ja[j] - 1) + i];
		      }
		  }
	      }
	    x->symmetry = sym;

	    break;

	  case sparse:

	    /* We'll dup `l', so swap them if `r' is more sparse. */

	    if (l->nn > r->nn)
	      {
		x = l;
		l = r;
		r = x;
		x = NULL;
	      }

	    x = (MATRIX *) dup_matrix (EAT (l));
	    for (i = 0; i < x->nr; i++)
	      {
		k = r->ia[i] - 1;
		km = r->ia[i + 1] - 1;
		for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
		  {
		    while (r->ja[k] < x->ja[j] && k < km)
		      k++;
		    if (k >= km)
		      break;
		    if (r->ja[k] == x->ja[j])
		      {
			x->a.integer[j] *= r->a.integer[k];
			k++;
		      }
		    else
		      {
			x->a.integer[j] = 0.0;
		      }
		  }
		for (; j < x->ia[i + 1] - 1; j++)
		  {
		    x->a.integer[j] = 0.0;
		  }
	      }
	    x->symmetry = sym;

	    /* We probably added some zeros, so run it through sparse. */

	    x = (MATRIX *) sparse_matrix (EAT (x));

	    break;

	  case sparse_upper:

	    if (sym == symmetric)
	      {
		/* Both symmetric, so convert `l' to sparse_upper. */
		l = (MATRIX *) sparse_matrix (EAT (l));
	      }
	    else
	      {
		/* Not symmetric, so use full form. */
		r = (MATRIX *) full_matrix (EAT (r));
	      }
	    x = (MATRIX *) product_matrix_integer (EAT (l), EAT (r));

	    break;

	  default:

	    wipeout ("Bad density.");

	  }

	break;

      case sparse_upper:

	switch (r->density)
	  {

	  case dense:

	    if (sym == symmetric)
	      {

		x = (MATRIX *) dup_matrix (EAT (l));
		if (x->nn > 0)
		  {
		    for (i = 0; i < x->nr; i++)
		      {
			for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
			  {
			    x->a.integer[j] *=
			      r->a.integer[r->nr * (x->ja[j] - 1) + i];
			  }
		      }
		  }
		if (x->d.integer)
		  {
		    for (i = 0; i < x->nr; i++)
		      {
			x->d.integer[i] *= r->a.integer[i * (r->nr + 1)];
		      }
		  }

	      }
	    else
	      {

		l = (MATRIX *) full_matrix (EAT (l));
		x = (MATRIX *) product_matrix_integer (EAT (l), EAT (r));

	      }

	    break;

	  case sparse:

	    if (sym == symmetric)
	      {
		/* Both symmetric, so convert `r' to sparse_upper. */
		r = (MATRIX *) sparse_matrix (EAT (r));
	      }
	    else
	      {
		/* Not symmetric, so use full form. */
		l = (MATRIX *) full_matrix (EAT (l));
	      }
	    x = (MATRIX *) product_matrix_integer (EAT (l), EAT (r));

	    break;

	  case sparse_upper:

	    if (sym != symmetric)
	      {
		l = (MATRIX *) full_matrix (EAT (l));
		r = (MATRIX *) full_matrix (EAT (r));
		x = (MATRIX *) product_matrix_integer (EAT (l), EAT (r));
	      }
	    else
	      {

		/* We'll dup `l', so swap them if `r' is more sparse. */

		if (l->nn > r->nn)
		  {
		    x = l;
		    l = r;
		    r = x;
		    x = NULL;
		  }

		x = (MATRIX *) dup_matrix (EAT (l));
		for (i = 0; i < x->nr; i++)
		  {
		    k = r->ia[i] - 1;
		    km = r->ia[i + 1] - 1;
		    for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
		      {
			while (r->ja[k] < x->ja[j] && k < km)
			  k++;
			if (k >= km)
			  break;
			if (r->ja[k] == x->ja[j])
			  {
			    x->a.integer[j] *= r->a.integer[k];
			    k++;
			  }
			else
			  {
			    x->a.integer[j] = 0.0;
			  }
		      }
		    for (; j < x->ia[i + 1] - 1; j++)
		      {
			x->a.integer[j] = 0.0;
		      }
		  }
		if (x->d.integer)
		  {
		    if (r->d.integer)
		      {
			for (i = 0; i < x->nr; i++)
			  x->d.integer[i] *= r->d.integer[i];
		      }
		    else
		      {
			for (i = 0; i < x->nr; i++)
			  x->d.integer[i] = 0.0;
		      }
		  }
		x->symmetry = sym;

		/* We probably added some zeros, so run sparse. */

		x = (MATRIX *) sparse_matrix (EAT (x));

	      }
	    break;

	  default:

	    wipeout ("Bad density.");

	  }

	break;

      default:

	wipeout ("Bad density.");

      }

  }
  ON_EXCEPTION
  {
    delete_3_matrices (l, r, x);
  }
  END_EXCEPTION;

  x->rid = COPY_RID (x, l, r);
  x->cid = COPY_CID (x, l, r);

  delete_2_matrices (l, r);
  return (ENT (x));
}

ENTITY *
product_matrix_real (MATRIX *l, MATRIX *r)
{
  /* Element-by-element multiplication. */

  MATRIX *x = NULL;
  int i, j, k, km;
  SYMMETRY sym;

  EASSERT (l, matrix, real);
  EASSERT (r, matrix, real);

  assert (l->order == ordered);
  assert (r->order == ordered);

  WITH_HANDLING
  {

    if (l->nr != r->nr)
      {
	fail ("Inconsistent dimensions for matrix product.  Left matrix has %d row%s, but right matrix has %d row%s.\n",
	      l->nr, PLURAL (l->nr), r->nr, PLURAL (r->nr));
	raise_exception ();
      }

    if (l->nc != r->nc)
      {
	fail ("Inconsistent dimensions for matrix product.  Left matrix has %d column%s, but right matrix has %d column%s.\n",
	      l->nc, PLURAL (l->nc), r->nc, PLURAL (r->nc));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->rid, r->rid) ||
	!MATCH_VECTORS (l->cid, r->cid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    sym = (l->symmetry == symmetric && r->symmetry == symmetric) ?
      symmetric : general;
    assert (l->symmetry != hermitian && r->symmetry != hermitian);

    switch (l->density)
      {

      case dense:

	switch (r->density)
	  {

	  case dense:

	    if (OWN (l))
	      {
		x = (MATRIX *) dup_matrix (EAT (l));
		for (i = 0; i < r->nn; i++)
		  x->a.real[i] *= r->a.real[i];
	      }
	    else
	      {
		x = (MATRIX *) dup_matrix (EAT (r));
		for (i = 0; i < l->nn; i++)
		  x->a.real[i] *= l->a.real[i];
	      }
	    x->symmetry = sym;

	    break;

	  case sparse:

	    x = (MATRIX *) dup_matrix (EAT (r));
	    if (x->nn > 0)
	      {
		for (i = 0; i < x->nr; i++)
		  {
		    for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
		      {
			x->a.real[j] *= l->a.real[l->nr * (x->ja[j] - 1) + i];
		      }
		  }
	      }
	    x->symmetry = sym;

	    break;

	  case sparse_upper:

	    if (sym == symmetric)
	      {

		x = (MATRIX *) dup_matrix (EAT (r));
		if (x->nn > 0)
		  {
		    for (i = 0; i < x->nr; i++)
		      {
			for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
			  {
			    x->a.real[j] *=
			      l->a.real[l->nr * (x->ja[j] - 1) + i];
			  }
		      }
		  }
		if (x->d.real)
		  {
		    for (i = 0; i < x->nr; i++)
		      {
			x->d.real[i] *= l->a.real[i * (l->nr + 1)];
		      }
		  }

	      }
	    else
	      {

		r = (MATRIX *) full_matrix (EAT (r));
		x = (MATRIX *) product_matrix_real (EAT (l), EAT (r));

	      }

	    break;

	  default:

	    wipeout ("Bad density.");

	  }

	break;

      case sparse:

	switch (r->density)
	  {

	  case dense:

	    x = (MATRIX *) dup_matrix (EAT (l));
	    if (x->nn > 0)
	      {
		for (i = 0; i < x->nr; i++)
		  {
		    for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
		      {
			x->a.real[j] *= r->a.real[r->nr * (x->ja[j] - 1) + i];
		      }
		  }
	      }
	    x->symmetry = sym;

	    break;

	  case sparse:

	    /* We'll dup `l', so swap them if `r' is more sparse. */

	    if (l->nn > r->nn)
	      {
		x = l;
		l = r;
		r = x;
		x = NULL;
	      }

	    x = (MATRIX *) dup_matrix (EAT (l));
	    for (i = 0; i < x->nr; i++)
	      {
		k = r->ia[i] - 1;
		km = r->ia[i + 1] - 1;
		for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
		  {
		    while (r->ja[k] < x->ja[j] && k < km)
		      k++;
		    if (k >= km)
		      break;
		    if (r->ja[k] == x->ja[j])
		      {
			x->a.real[j] *= r->a.real[k];
			k++;
		      }
		    else
		      {
			x->a.real[j] = 0.0;
		      }
		  }
		for (; j < x->ia[i + 1] - 1; j++)
		  {
		    x->a.real[j] = 0.0;
		  }
	      }
	    x->symmetry = sym;

	    /* We probably added some zeros, so run it through sparse. */

	    x = (MATRIX *) sparse_matrix (EAT (x));

	    break;

	  case sparse_upper:

	    if (sym == symmetric)
	      {
		/* Both symmetric, so convert `l' to sparse_upper. */
		l = (MATRIX *) sparse_matrix (EAT (l));
	      }
	    else
	      {
		/* Not symmetric, so use full form. */
		r = (MATRIX *) full_matrix (EAT (r));
	      }
	    x = (MATRIX *) product_matrix_real (EAT (l), EAT (r));

	    break;

	  default:

	    wipeout ("Bad density.");

	  }

	break;

      case sparse_upper:

	switch (r->density)
	  {

	  case dense:

	    if (sym == symmetric)
	      {

		x = (MATRIX *) dup_matrix (EAT (l));
		if (x->nn > 0)
		  {
		    for (i = 0; i < x->nr; i++)
		      {
			for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
			  {
			    x->a.real[j] *=
			      r->a.real[r->nr * (x->ja[j] - 1) + i];
			  }
		      }
		  }
		if (x->d.real)
		  {
		    for (i = 0; i < x->nr; i++)
		      {
			x->d.real[i] *= r->a.real[i * (r->nr + 1)];
		      }
		  }

	      }
	    else
	      {

		l = (MATRIX *) full_matrix (EAT (l));
		x = (MATRIX *) product_matrix_real (EAT (l), EAT (r));

	      }

	    break;

	  case sparse:

	    if (sym == symmetric)
	      {
		/* Both symmetric, so convert `r' to sparse_upper. */
		r = (MATRIX *) sparse_matrix (EAT (r));
	      }
	    else
	      {
		/* Not symmetric, so use full form. */
		l = (MATRIX *) full_matrix (EAT (l));
	      }
	    x = (MATRIX *) product_matrix_real (EAT (l), EAT (r));

	    break;

	  case sparse_upper:

	    if (sym != symmetric)
	      {
		l = (MATRIX *) full_matrix (EAT (l));
		r = (MATRIX *) full_matrix (EAT (r));
		x = (MATRIX *) product_matrix_real (EAT (l), EAT (r));
	      }
	    else
	      {

		/* We'll dup `l', so swap them if `r' is more sparse. */

		if (l->nn > r->nn)
		  {
		    x = l;
		    l = r;
		    r = x;
		    x = NULL;
		  }

		x = (MATRIX *) dup_matrix (EAT (l));
		for (i = 0; i < x->nr; i++)
		  {
		    k = r->ia[i] - 1;
		    km = r->ia[i + 1] - 1;
		    for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
		      {
			while (r->ja[k] < x->ja[j] && k < km)
			  k++;
			if (k >= km)
			  break;
			if (r->ja[k] == x->ja[j])
			  {
			    x->a.real[j] *= r->a.real[k];
			    k++;
			  }
			else
			  {
			    x->a.real[j] = 0.0;
			  }
		      }
		    for (; j < x->ia[i + 1] - 1; j++)
		      {
			x->a.real[j] = 0.0;
		      }
		  }
		if (x->d.real)
		  {
		    if (r->d.real)
		      {
			for (i = 0; i < x->nr; i++)
			  x->d.real[i] *= r->d.real[i];
		      }
		    else
		      {
			for (i = 0; i < x->nr; i++)
			  x->d.real[i] = 0.0;
		      }
		  }
		x->symmetry = sym;

		/* We probably added some zeros, so run sparse. */

		x = (MATRIX *) sparse_matrix (EAT (x));

	      }
	    break;

	  default:

	    wipeout ("Bad density.");

	  }

	break;

      default:

	wipeout ("Bad density.");

      }

  }
  ON_EXCEPTION
  {
    delete_3_matrices (l, r, x);
  }
  END_EXCEPTION;

  x->rid = COPY_RID (x, l, r);
  x->cid = COPY_CID (x, l, r);

  delete_2_matrices (l, r);
  return (ENT (x));
}

ENTITY *
product_matrix_complex (MATRIX *l, MATRIX *r)
{
  /* Element-by-element multiplication. */

  MATRIX *x = NULL;
  int i, j, k, km;
  SYMMETRY sym;
  COMPLEX tmp;

  EASSERT (l, matrix, complex);
  EASSERT (r, matrix, complex);

  assert (l->order == ordered);
  assert (r->order == ordered);

  WITH_HANDLING
  {

    if (l->nr != r->nr)
      {
	fail ("Inconsistent dimensions for matrix product.  Left matrix has %d row%s, but right matrix has %d row%s.\n",
	      l->nr, PLURAL (l->nr), r->nr, PLURAL (r->nr));
	raise_exception ();
      }

    if (l->nc != r->nc)
      {
	fail ("Inconsistent dimensions for matrix product.  Left matrix has %d column%s, but right matrix has %d column%s.\n",
	      l->nc, PLURAL (l->nc), r->nc, PLURAL (r->nc));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->rid, r->rid) ||
	!MATCH_VECTORS (l->cid, r->cid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    sym = general;
    if (l->symmetry == r->symmetry)
      {
	if (l->symmetry == symmetric)
	  sym = symmetric;
	if (l->symmetry == hermitian)
	  sym = hermitian;
      }

    switch (l->density)
      {

      case dense:

	switch (r->density)
	  {

	  case dense:

	    if (OWN (l))
	      {
		x = (MATRIX *) dup_matrix (EAT (l));
		for (i = 0; i < r->nn; i++)
		  {
		    tmp.real = (x->a.complex[i].real *
				r->a.complex[i].real -
				x->a.complex[i].imag *
				r->a.complex[i].imag);
		    tmp.imag = (x->a.complex[i].real *
				r->a.complex[i].imag +
				x->a.complex[i].imag *
				r->a.complex[i].real);
		    x->a.complex[i].real = tmp.real;
		    x->a.complex[i].imag = tmp.imag;
		  }
	      }
	    else
	      {
		x = (MATRIX *) dup_matrix (EAT (r));
		for (i = 0; i < l->nn; i++)
		  {
		    tmp.real = (x->a.complex[i].real *
				l->a.complex[i].real -
				x->a.complex[i].imag *
				l->a.complex[i].imag);
		    tmp.imag = (x->a.complex[i].real *
				l->a.complex[i].imag +
				x->a.complex[i].imag *
				l->a.complex[i].real);
		    x->a.complex[i].real = tmp.real;
		    x->a.complex[i].imag = tmp.imag;
		  }
	      }
	    x->symmetry = sym;

	    break;

	  case sparse:

	    x = (MATRIX *) dup_matrix (EAT (r));
	    if (x->nn > 0)
	      {
		for (i = 0; i < x->nr; i++)
		  {
		    for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
		      {
			k = l->nr * (x->ja[j] - 1) + i;
			tmp.real = (x->a.complex[j].real *
				    l->a.complex[k].real -
				    x->a.complex[j].imag *
				    l->a.complex[k].imag);
			tmp.imag = (x->a.complex[j].real *
				    l->a.complex[k].imag +
				    x->a.complex[j].imag *
				    l->a.complex[k].real);
			x->a.complex[j].real = tmp.real;
			x->a.complex[j].imag = tmp.imag;
		      }
		  }
	      }
	    x->symmetry = sym;

	    break;

	  case sparse_upper:

	    if (sym == symmetric || sym == hermitian)
	      {

		x = (MATRIX *) dup_matrix (EAT (r));
		if (x->nn > 0)
		  {
		    for (i = 0; i < x->nr; i++)
		      {
			for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
			  {
			    k = l->nr * (x->ja[j] - 1) + i;
			    tmp.real = (x->a.complex[j].real *
					l->a.complex[k].real -
					x->a.complex[j].imag *
					l->a.complex[k].imag);
			    tmp.imag = (x->a.complex[j].real *
					l->a.complex[k].imag +
					x->a.complex[j].imag *
					l->a.complex[k].real);
			    x->a.complex[j].real = tmp.real;
			    x->a.complex[j].imag = tmp.imag;
			  }
		      }
		  }
		if (x->d.real)
		  {
		    for (i = 0; i < x->nr; i++)
		      {
			k = i * (l->nr + 1);
			tmp.real = (x->d.complex[i].real *
				    l->a.complex[k].real -
				    x->d.complex[i].imag *
				    l->a.complex[k].imag);
			tmp.imag = (x->d.complex[i].real *
				    l->a.complex[k].imag +
				    x->d.complex[i].imag *
				    l->a.complex[k].real);
			x->d.complex[i].real = tmp.real;
			x->d.complex[i].imag = tmp.imag;
		      }
		  }

	      }
	    else
	      {

		r = (MATRIX *) full_matrix (EAT (r));
		x = (MATRIX *) product_matrix_complex (EAT (l), EAT (r));

	      }

	    break;

	  default:

	    wipeout ("Bad density.");

	  }

	break;

      case sparse:

	switch (r->density)
	  {

	  case dense:

	    x = (MATRIX *) dup_matrix (EAT (l));
	    if (x->nn > 0)
	      {
		for (i = 0; i < x->nr; i++)
		  {
		    for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
		      {
			k = r->nr * (x->ja[j] - 1) + i;
			tmp.real = (x->a.complex[j].real *
				    r->a.complex[k].real -
				    x->a.complex[j].imag *
				    r->a.complex[k].imag);
			tmp.imag = (x->a.complex[j].real *
				    r->a.complex[k].imag +
				    x->a.complex[j].imag *
				    r->a.complex[k].real);
			x->a.complex[j].real = tmp.real;
			x->a.complex[j].imag = tmp.imag;
		      }
		  }
	      }
	    x->symmetry = sym;

	    break;

	  case sparse:

	    /* We'll dup `l', so swap them if `r' is more sparse. */

	    if (l->nn > r->nn)
	      {
		x = l;
		l = r;
		r = x;
		x = NULL;
	      }

	    x = (MATRIX *) dup_matrix (EAT (l));
	    for (i = 0; i < x->nr; i++)
	      {
		k = r->ia[i] - 1;
		km = r->ia[i + 1] - 1;
		for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
		  {
		    while (r->ja[k] < x->ja[j] && k < km)
		      k++;
		    if (k >= km)
		      break;
		    if (r->ja[k] == x->ja[j])
		      {
			tmp.real = (x->a.complex[j].real *
				    r->a.complex[k].real -
				    x->a.complex[j].imag *
				    r->a.complex[k].imag);
			tmp.imag = (x->a.complex[j].real *
				    r->a.complex[k].imag +
				    x->a.complex[j].imag *
				    r->a.complex[k].real);
			x->a.complex[j].real = tmp.real;
			x->a.complex[j].imag = tmp.imag;
			k++;
		      }
		    else
		      {
			x->a.complex[j].real = 0.0;
			x->a.complex[j].imag = 0.0;
		      }
		  }
		for (; j < x->ia[i + 1] - 1; j++)
		  {
		    x->a.complex[j].real = 0.0;
		    x->a.complex[j].imag = 0.0;
		  }
	      }
	    x->symmetry = sym;

	    /* We probably added some zeros, so run it through sparse. */

	    x = (MATRIX *) sparse_matrix (EAT (x));

	    break;

	  case sparse_upper:

	    if (sym == symmetric || sym == hermitian)
	      {
		/* Both symmetric, so convert `l' to sparse_upper. */
		l = (MATRIX *) sparse_matrix (EAT (l));
		assert (l->density == sparse_upper);
	      }
	    else
	      {
		/* Not symmetric, so use full form. */
		r = (MATRIX *) full_matrix (EAT (r));
	      }
	    x = (MATRIX *) product_matrix_complex (EAT (l), EAT (r));

	    break;

	  default:

	    wipeout ("Bad density.");

	  }

	break;

      case sparse_upper:

	switch (r->density)
	  {

	  case dense:

	    if (sym == symmetric || sym == hermitian)
	      {

		x = (MATRIX *) dup_matrix (EAT (l));
		if (x->nn > 0)
		  {
		    for (i = 0; i < x->nr; i++)
		      {
			for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
			  {
			    k = r->nr * (x->ja[j] - 1) + i;
			    tmp.real = (x->a.complex[j].real *
					r->a.complex[k].real -
					x->a.complex[j].imag *
					r->a.complex[k].imag);
			    tmp.imag = (x->a.complex[j].real *
					r->a.complex[k].imag +
					x->a.complex[j].imag *
					r->a.complex[k].real);
			    x->a.complex[j].real = tmp.real;
			    x->a.complex[j].imag = tmp.imag;
			  }
		      }
		  }
		if (x->d.real)
		  {
		    for (i = 0; i < x->nr; i++)
		      {
			k = i * (r->nr + 1);
			tmp.real = (x->d.complex[i].real *
				    r->a.complex[k].real -
				    x->d.complex[i].imag *
				    r->a.complex[k].imag);
			tmp.imag = (x->d.complex[i].real *
				    r->a.complex[k].imag +
				    x->d.complex[i].imag *
				    r->a.complex[k].real);
			x->d.complex[i].real = tmp.real;
			x->d.complex[i].imag = tmp.imag;
		      }
		  }

	      }
	    else
	      {

		l = (MATRIX *) full_matrix (EAT (l));
		x = (MATRIX *) product_matrix_complex (EAT (l), EAT (r));

	      }

	    break;

	  case sparse:

	    if (sym == symmetric || sym == hermitian)
	      {
		/* Both symmetric, so convert `r' to sparse_upper. */
		r = (MATRIX *) sparse_matrix (EAT (r));
		assert (r->density == sparse_upper);
	      }
	    else
	      {
		/* Not symmetric, so use full form. */
		l = (MATRIX *) full_matrix (EAT (l));
	      }
	    x = (MATRIX *) product_matrix_complex (EAT (l), EAT (r));

	    break;

	  case sparse_upper:

	    if (sym != symmetric && sym != hermitian)
	      {
		l = (MATRIX *) full_matrix (EAT (l));
		r = (MATRIX *) full_matrix (EAT (r));
		x = (MATRIX *) product_matrix_complex (EAT (l), EAT (r));
	      }
	    else
	      {

		/* We'll dup `l', so swap them if `r' is more sparse. */

		if (l->nn > r->nn)
		  {
		    x = l;
		    l = r;
		    r = x;
		    x = NULL;
		  }

		x = (MATRIX *) dup_matrix (EAT (l));
		for (i = 0; i < x->nr; i++)
		  {
		    k = r->ia[i] - 1;
		    km = r->ia[i + 1] - 1;
		    for (j = x->ia[i] - 1; j < x->ia[i + 1] - 1; j++)
		      {
			while (r->ja[k] < x->ja[j] && k < km)
			  k++;
			if (k >= km)
			  break;
			if (r->ja[k] == x->ja[j])
			  {
			    tmp.real = (x->a.complex[j].real *
					r->a.complex[k].real -
					x->a.complex[j].imag *
					x->a.complex[k].imag);
			    tmp.imag = (x->a.complex[j].real *
					r->a.complex[k].imag +
					x->a.complex[j].imag *
					r->a.complex[k].real);
			    x->a.complex[j].real = tmp.real;
			    x->a.complex[j].imag = tmp.imag;
			    k++;
			  }
			else
			  {
			    x->a.complex[j].real = 0.0;
			    x->a.complex[j].imag = 0.0;
			  }
		      }
		    for (; j < x->ia[i + 1] - 1; j++)
		      {
			x->a.complex[j].real = 0.0;
			x->a.complex[j].imag = 0.0;
		      }
		  }
		if (x->d.real)
		  {
		    if (r->d.real)
		      {
			for (i = 0; i < x->nr; i++)
			  {
			    tmp.real = (x->d.complex[i].real *
					r->d.complex[i].real -
					x->d.complex[i].imag *
					r->d.complex[i].imag);
			    tmp.imag = (x->d.complex[i].real *
					r->d.complex[i].imag -
					x->d.complex[i].imag *
					r->d.complex[i].real);
			    x->d.complex[i].real = tmp.real;
			    x->d.complex[i].imag = tmp.imag;
			  }
		      }
		    else
		      {
			for (i = 0; i < x->nr; i++)
			  x->d.complex[i].real = 0.0;
			x->d.complex[i].imag = 0.0;
		      }
		  }
		x->symmetry = sym;

		/* We probably added some zeros, so run sparse. */

		x = (MATRIX *) sparse_matrix (EAT (x));

	      }
	    break;

	  default:

	    wipeout ("Bad density.");

	  }

	break;

      default:

	wipeout ("Bad density.");

      }

  }
  ON_EXCEPTION
  {
    delete_3_matrices (l, r, x);
  }
  END_EXCEPTION;

  x->rid = COPY_RID (x, l, r);
  x->cid = COPY_CID (x, l, r);

  delete_2_matrices (l, r);
  return (ENT (x));
}

ENTITY *
product_vector_integer (VECTOR *l, VECTOR *r)
{
  /* Element-by-element multiplication. */

  VECTOR *x = NULL;
  int i, j, k, km;

  EASSERT (l, vector, integer);
  EASSERT (r, vector, integer);

  assert (l->order == ordered);
  assert (r->order == ordered);

  WITH_HANDLING
  {

    if (l->ne != r->ne)
      {
	fail ("Inconsistent dimensions for vector product.  Left vector has %d element%s, but right vector has %d element%s.\n",
	      l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->eid, r->eid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    switch (l->density)
      {

      case dense:

	switch (r->density)
	  {

	  case dense:

	    if (OWN (l))
	      {
		x = (VECTOR *) dup_vector (EAT (l));
		for (i = 0; i < r->nn; i++)
		  x->a.integer[i] *= r->a.integer[i];
	      }
	    else
	      {
		x = (VECTOR *) dup_vector (EAT (r));
		for (i = 0; i < l->nn; i++)
		  x->a.integer[i] *= l->a.integer[i];
	      }

	    break;

	  case sparse:

	    x = (VECTOR *) dup_vector (EAT (r));
	    if (x->nn > 0)
	      {
		for (j = 0; j < x->nn; j++)
		  {
		    x->a.integer[j] *= l->a.integer[x->ja[j] - 1];
		  }
	      }

	    break;

	  default:

	    wipeout ("Bad density.");

	  }

	break;

      case sparse:

	switch (r->density)
	  {

	  case dense:

	    x = (VECTOR *) dup_vector (EAT (l));
	    if (x->nn > 0)
	      {
		for (j = 0; j < x->nn; j++)
		  {
		    x->a.integer[j] *= r->a.integer[x->ja[j] - 1];
		  }
	      }

	    break;

	  case sparse:

	    /* We'll dup `l', so swap them if `r' is more sparse. */

	    if (l->nn > r->nn)
	      {
		x = l;
		l = r;
		r = x;
		x = NULL;
	      }

	    x = (VECTOR *) dup_vector (EAT (l));

	    k = 0;
	    km = r->nn;
	    for (j = 0; j < x->nn; j++)
	      {
		while (r->ja[k] < x->ja[j] && k < km)
		  k++;
		if (k >= km)
		  break;
		if (r->ja[k] == x->ja[j])
		  {
		    x->a.integer[j] *= r->a.integer[k];
		    k++;
		  }
		else
		  {
		    x->a.integer[j] = 0.0;
		  }
	      }
	    for (; j < x->nn; j++)
	      {
		x->a.integer[j] = 0.0;
	      }

	    /* We probably added some zeros, so run it through sparse. */

	    x = (VECTOR *) sparse_vector (EAT (x));

	    break;

	  default:

	    wipeout ("Bad density.");

	  }

	break;

      default:

	wipeout ("Bad density.");

      }
  }
  ON_EXCEPTION
  {
    delete_3_vectors (l, r, x);
  }
  END_EXCEPTION;

  x->eid = COPY_EID (x, l, r);

  delete_2_vectors (l, r);
  return (ENT (x));
}

ENTITY *
product_vector_real (VECTOR *l, VECTOR *r)
{
  /* Element-by-element multiplication. */

  VECTOR *x = NULL;
  int i, j, k, km;

  EASSERT (l, vector, real);
  EASSERT (r, vector, real);

  assert (l->order == ordered);
  assert (r->order == ordered);

  WITH_HANDLING
  {

    if (l->ne != r->ne)
      {
	fail ("Inconsistent dimensions for vector product.  Left vector has %d element%s, but right vector has %d element%s.\n",
	      l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->eid, r->eid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    switch (l->density)
      {

      case dense:

	switch (r->density)
	  {

	  case dense:

	    if (OWN (l))
	      {
		x = (VECTOR *) dup_vector (EAT (l));
		for (i = 0; i < r->nn; i++)
		  x->a.real[i] *= r->a.real[i];
	      }
	    else
	      {
		x = (VECTOR *) dup_vector (EAT (r));
		for (i = 0; i < l->nn; i++)
		  x->a.real[i] *= l->a.real[i];
	      }

	    break;

	  case sparse:

	    x = (VECTOR *) dup_vector (EAT (r));
	    if (x->nn > 0)
	      {
		for (j = 0; j < x->nn; j++)
		  {
		    x->a.real[j] *= l->a.real[x->ja[j] - 1];
		  }
	      }

	    break;

	  default:

	    wipeout ("Bad density.");

	  }

	break;

      case sparse:

	switch (r->density)
	  {

	  case dense:

	    x = (VECTOR *) dup_vector (EAT (l));
	    if (x->nn > 0)
	      {
		for (j = 0; j < x->nn; j++)
		  {
		    x->a.real[j] *= r->a.real[x->ja[j] - 1];
		  }
	      }

	    break;

	  case sparse:

	    /* We'll dup `l', so swap them if `r' is more sparse. */

	    if (l->nn > r->nn)
	      {
		x = l;
		l = r;
		r = x;
		x = NULL;
	      }

	    x = (VECTOR *) dup_vector (EAT (l));

	    k = 0;
	    km = r->nn;
	    for (j = 0; j < x->nn; j++)
	      {
		while (r->ja[k] < x->ja[j] && k < km)
		  k++;
		if (k >= km)
		  break;
		if (r->ja[k] == x->ja[j])
		  {
		    x->a.real[j] *= r->a.real[k];
		    k++;
		  }
		else
		  {
		    x->a.real[j] = 0.0;
		  }
	      }
	    for (; j < x->nn; j++)
	      {
		x->a.real[j] = 0.0;
	      }

	    /* We probably added some zeros, so run it through sparse. */

	    x = (VECTOR *) sparse_vector (EAT (x));

	    break;

	  default:

	    wipeout ("Bad density.");

	  }

	break;

      default:

	wipeout ("Bad density.");

      }
  }
  ON_EXCEPTION
  {
    delete_3_vectors (l, r, x);
  }
  END_EXCEPTION;

  x->eid = COPY_EID (x, l, r);

  delete_2_vectors (l, r);
  return (ENT (x));
}

ENTITY *
product_vector_complex (VECTOR *l, VECTOR *r)
{
  /* Element-by-element multiplication. */

  VECTOR *x = NULL;
  int i, j, k, km;
  COMPLEX tmp;

  EASSERT (l, vector, complex);
  EASSERT (r, vector, complex);

  assert (l->order == ordered);
  assert (r->order == ordered);

  WITH_HANDLING
  {

    if (l->ne != r->ne)
      {
	fail ("Inconsistent dimensions for vector product.  Left vector has %d element%s, but right vector has %d element%s.\n",
	      l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->eid, r->eid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    switch (l->density)
      {

      case dense:

	switch (r->density)
	  {

	  case dense:

	    if (OWN (l))
	      {
		x = (VECTOR *) dup_vector (EAT (l));
		for (i = 0; i < r->nn; i++)
		  {
		    tmp.real = (x->a.complex[i].real *
				r->a.complex[i].real -
				x->a.complex[i].imag *
				r->a.complex[i].imag);
		    tmp.imag = (x->a.complex[i].real *
				r->a.complex[i].imag +
				x->a.complex[i].imag *
				r->a.complex[i].real);
		    x->a.complex[i].real = tmp.real;
		    x->a.complex[i].imag = tmp.imag;
		  }
	      }
	    else
	      {
		x = (VECTOR *) dup_vector (EAT (r));
		for (i = 0; i < l->nn; i++)
		  {
		    tmp.real = (x->a.complex[i].real *
				l->a.complex[i].real -
				x->a.complex[i].imag *
				l->a.complex[i].imag);
		    tmp.imag = (x->a.complex[i].real *
				l->a.complex[i].imag +
				x->a.complex[i].imag *
				l->a.complex[i].real);
		    x->a.complex[i].real = tmp.real;
		    x->a.complex[i].imag = tmp.imag;
		  }
	      }

	    break;

	  case sparse:

	    x = (VECTOR *) dup_vector (EAT (r));
	    for (j = 0; j < x->nn; j++)
	      {
		k = x->ja[j] - 1;
		tmp.real = (x->a.complex[j].real *
			    l->a.complex[k].real -
			    x->a.complex[j].imag *
			    l->a.complex[k].imag);
		tmp.imag = (x->a.complex[j].real *
			    l->a.complex[k].imag +
			    x->a.complex[j].imag *
			    l->a.complex[k].real);
		x->a.complex[j].real = tmp.real;
		x->a.complex[j].imag = tmp.imag;
	      }

	    break;

	  default:

	    wipeout ("Bad density.");

	  }

	break;

      case sparse:

	switch (r->density)
	  {

	  case dense:

	    x = (VECTOR *) dup_vector (EAT (l));
	    for (j = 0; j < x->nn; j++)
	      {
		k = x->ja[j] - 1;
		tmp.real = (x->a.complex[j].real *
			    r->a.complex[k].real -
			    x->a.complex[j].imag *
			    r->a.complex[k].imag);
		tmp.imag = (x->a.complex[j].real *
			    r->a.complex[k].imag +
			    x->a.complex[j].imag *
			    r->a.complex[k].real);
		x->a.complex[j].real = tmp.real;
		x->a.complex[j].imag = tmp.imag;
	      }

	    break;

	  case sparse:

	    /* We'll dup `l', so swap them if `r' is more sparse. */

	    if (l->nn > r->nn)
	      {
		x = l;
		l = r;
		r = x;
		x = NULL;
	      }

	    x = (VECTOR *) dup_vector (EAT (l));

	    k = 0;
	    km = r->nn;
	    for (j = 0; j < x->nn; j++)
	      {
		while (r->ja[k] < x->ja[j] && k < km)
		  k++;
		if (k >= km)
		  break;
		if (r->ja[k] == x->ja[j])
		  {
		    tmp.real = (x->a.complex[j].real *
				r->a.complex[k].real -
				x->a.complex[j].imag *
				r->a.complex[k].imag);
		    tmp.imag = (x->a.complex[j].real *
				r->a.complex[k].imag +
				x->a.complex[j].imag *
				r->a.complex[k].real);
		    x->a.complex[j].real = tmp.real;
		    x->a.complex[j].imag = tmp.imag;
		    k++;
		  }
		else
		  {
		    x->a.complex[j].real = 0.0;
		    x->a.complex[j].imag = 0.0;
		  }
	      }
	    for (; j < x->nn; j++)
	      {
		x->a.complex[j].real = 0.0;
		x->a.complex[j].imag = 0.0;
	      }

	    /* We probably added some zeros, so run it through sparse. */

	    x = (VECTOR *) sparse_vector (EAT (x));

	    break;

	  default:

	    wipeout ("Bad density.");

	  }

	break;

      default:

	wipeout ("Bad density.");

      }

  }
  ON_EXCEPTION
  {
    delete_3_vectors (l, r, x);
  }
  END_EXCEPTION;

  x->eid = COPY_EID (x, l, r);

  delete_2_vectors (l, r);
  return (ENT (x));
}
