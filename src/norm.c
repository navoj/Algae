/*
   norm.c -- Vector and matrix norms.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: norm.c,v 1.2 2003/08/01 04:57:48 ksh Exp $";

#include "norm.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "svd.h"

ENTITY *
bi_norm (int n, ENTITY *a, ENTITY *p)
{
  /*
   * This function computes the p-norm of a vector or matrix,
   * where `p' is 1, 2, "frobenius", or "infinity".  The default
   * is 2 when `p' is NULL.
   */

  ENTITY * volatile s;
  volatile NORM_TYPE nt;

  EASSERT (a, 0, 0);

  WITH_HANDLING
  {

    if (n == 1)
      {
	nt = two_norm;
      }
    else
      {

	assert (n == 2);
	EASSERT (p, 0, 0);

	switch (p->class)
	  {
	  case scalar:
	    break;
	  case vector:
	    p = vector_to_scalar ((VECTOR *) EAT (p));
	    break;
	  case matrix:
	    p = matrix_to_scalar ((MATRIX *) EAT (p));
	    break;
	  default:
	    fail ("Can't make sense of a %s entity as second argument to \"norm\".",
		  class_string[p->class]);
	    raise_exception ();
	  }

	switch (((SCALAR *) p)->type)
	  {
	  case integer:
	    n = ((SCALAR *) p)->v.integer;
	    switch (n)
	      {
	      case 1:
		nt = one_norm;
		break;
	      case 2:
		nt = two_norm;
		break;
	      default:
		fail ("Can't compute a %d-norm.", n);
		raise_exception ();
	      }
	    break;
	  case real:
	  case complex:
	    fail ("Second argument to \"norm\" must have integer or character type.");
	    raise_exception ();
	  case character:
	    if (!strcmp (((SCALAR *) p)->v.character, "1"))
	      {
		nt = one_norm;
	      }
	    else if (!strcmp (((SCALAR *) p)->v.character, "2"))
	      {
		nt = two_norm;
	      }
	    else if (!strcmp (((SCALAR *) p)->v.character, "inf") ||
		     !strcmp (((SCALAR *) p)->v.character, "infinity"))
	      {
		nt = infinity_norm;
	      }
	    else if (!strcmp (((SCALAR *) p)->v.character, "frob") ||
		     !strcmp (((SCALAR *) p)->v.character, "frobenius"))
	      {
		nt = frobenius_norm;
	      }
	    else
	      {
		fail ("Can't compute a \"%s\"-norm.", ((SCALAR *) p)->v.character);
		raise_exception ();
	      }
	    break;
	  default:
	    BAD_TYPE (((SCALAR *) p)->type);
	    raise_exception ();
	  }
      }

    delete_entity (p);
    p = NULL;

    switch (a->class)
      {
      case scalar:
	s = norm_vector ((VECTOR *) scalar_to_vector ((SCALAR *) EAT (a)), nt);
	break;
      case vector:
	s = norm_vector ((VECTOR *) EAT (a), nt);
	break;
      case matrix:
	s = norm_matrix ((MATRIX *) EAT (a), nt);
	break;
      default:
	fail ("Can't compute norm of a %s entity.", class_string[a->class]);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_2_entities (a, p);
  }
  END_EXCEPTION;

  return (s);
}

ENTITY *
norm_matrix (MATRIX *a, NORM_TYPE nt)
{
  volatile TYPE type;
  int i, j, nr, nc, nn;
  int tmp_int;
  volatile int tmp2_int;
  REAL tmp_real;
  volatile REAL tmp2_real;
  VECTOR * volatile v = NULL;
  ENTITY *t = NULL;

  EASSERT (a, matrix, 0);

  nr = a->nr;
  nc = a->nc;
  nn = a->nn;

  WITH_HANDLING
  {

    switch (nt)
      {
      case infinity_norm:
	switch (a->type)
	  {
	  case integer:
	    type = integer;
	    tmp2_int = 0;
	    switch (a->density)
	      {
	      case dense:
		for (i = 0; i < nr; i++)
		  {
		    tmp_int = 0;
		    for (j = 0; j < nc; j++)
		      tmp_int += abs (a->a.integer[i + j * nr]);
		    if (tmp_int > tmp2_int)
		      tmp2_int = tmp_int;
		  }
		break;
	      case sparse:
		if (nn > 0)
		  {
		    for (i = 0; i < nr; i++)
		      {
			tmp_int = 0;
			for (j = a->ia[i]; j < a->ia[i + 1]; j++)
			  tmp_int += abs (a->a.integer[j - 1]);
			if (tmp_int > tmp2_int)
			  tmp2_int = tmp_int;
		      }
		  }
		break;
	      case sparse_upper:
		{
		  int *d = (int *) CALLOC (nr, sizeof (int));

		  if (a->d.integer != NULL)
		    memcpy (d, a->d.integer, nr * sizeof (int));
		  for (i = 0; i < nr; i++)
		    d[i] = abs (d[i]);

		  if (nn > 0)
		    {
		      if (a->symmetry == general)
			{
			  for (i = 0; i < nr; i++)
			    {
			      for (j = a->ia[i]; j < a->ia[i + 1]; j++)
				{
				  d[i] += abs (a->a.integer[j - 1]);
				}
			    }
			}
		      else
			{
			  for (i = 0; i < nr; i++)
			    {
			      for (j = a->ia[i]; j < a->ia[i + 1]; j++)
				{
				  d[i] += abs (a->a.integer[j - 1]);
				  d[a->ja[j - 1] - 1] +=
				    abs (a->a.integer[j - 1]);
				}
			    }
			}
		    }

		  for (i = 0; i < nr; i++)
		    if (d[i] > tmp2_int)
		      tmp2_int = d[i];
		  FREE (d);
		}
		break;
	      default:
		BAD_DENSITY (a->density);
		raise_exception ();
	      }
	    break;
	  case real:
	    type = real;
	    tmp2_real = 0.0;
	    switch (a->density)
	      {
	      case dense:
		for (i = 0; i < nr; i++)
		  {
		    tmp_real = 0;
		    for (j = 0; j < nc; j++)
		      tmp_real += fabs (a->a.real[i + j * nr]);
		    if (tmp_real > tmp2_real)
		      tmp2_real = tmp_real;
		  }
		break;
	      case sparse:
		if (nn > 0)
		  {
		    for (i = 0; i < nr; i++)
		      {
			tmp_real = 0;
			for (j = a->ia[i]; j < a->ia[i + 1]; j++)
			  tmp_real += fabs (a->a.real[j - 1]);
			if (tmp_real > tmp2_real)
			  tmp2_real = tmp_real;
		      }
		  }
		break;
	      case sparse_upper:
		{
		  REAL *d = (REAL *) CALLOC (nr, sizeof (REAL));

		  if (a->d.real != NULL)
		    memcpy (d, a->d.real, nr * sizeof (REAL));
		  for (i = 0; i < nr; i++)
		    d[i] = fabs (d[i]);

		  if (nn > 0)
		    {
		      if (a->symmetry == general)
			{
			  for (i = 0; i < nr; i++)
			    {
			      for (j = a->ia[i]; j < a->ia[i + 1]; j++)
				{
				  d[i] += fabs (a->a.real[j - 1]);
				}
			    }
			}
		      else
			{
			  for (i = 0; i < nr; i++)
			    {
			      for (j = a->ia[i]; j < a->ia[i + 1]; j++)
				{
				  d[i] += fabs (a->a.real[j - 1]);
				  d[a->ja[j - 1] - 1] +=
				    fabs (a->a.real[j - 1]);
				}
			    }
			}
		    }

		  for (i = 0; i < nr; i++)
		    if (d[i] > tmp2_real)
		      tmp2_real = d[i];
		  FREE (d);
		}
		break;
	      default:
		BAD_DENSITY (a->density);
		raise_exception ();
	      }
	    break;
	  case complex:
	    type = real;
	    tmp2_real = 0.0;
	    switch (a->density)
	      {
	      case dense:
		for (i = 0; i < nr; i++)
		  {
		    tmp_real = 0.0;
		    for (j = 0; j < nc; j++)
		      {
			tmp_real +=
			  (fabs (a->a.complex[i + j * nr].real) +
			   fabs (a->a.complex[i + j * nr].imag));
		      }
		    if (tmp_real > tmp2_real)
		      tmp2_real = tmp_real;
		  }
		break;
	      case sparse:
		if (nn > 0)
		  {
		    for (i = 0; i < nr; i++)
		      {
			tmp_real = 0;
			for (j = a->ia[i]; j < a->ia[i + 1]; j++)
			  {
			    tmp_real +=
			      (fabs (a->a.complex[j - 1].real) +
			       fabs (a->a.complex[j - 1].imag));
			  }
			if (tmp_real > tmp2_real)
			  tmp2_real = tmp_real;
		      }
		  }
		break;
	      case sparse_upper:
		{
		  REAL *d = (REAL *) CALLOC (nr, sizeof (REAL));

		  if (a->d.real != NULL)
		    {
		      for (i = 0; i < nr; i++)
			d[i] = (fabs (a->d.complex[i].real) +
				fabs (a->d.complex[i].imag));
		    }
		  if (nn > 0)
		    {
		      if (a->symmetry == general)
			{
			  for (i = 0; i < nr; i++)
			    {
			      for (j = a->ia[i]; j < a->ia[i + 1]; j++)
				{
				  d[i] +=
				    (fabs (a->a.complex[j - 1].real) +
				     fabs (a->a.complex[j - 1].imag));
				}
			    }
			}
		      else
			{
			  for (i = 0; i < nr; i++)
			    {
			      for (j = a->ia[i]; j < a->ia[i + 1]; j++)
				{
				  d[i] +=
				    (fabs (a->a.complex[j - 1].real) +
				     fabs (a->a.complex[j - 1].imag));
				  d[a->ja[j - 1] - 1] +=
				    (fabs (a->a.complex[j - 1].real) +
				     fabs (a->a.complex[j - 1].imag));
				}
			    }
			}
		    }

		  for (i = 0; i < nr; i++)
		    if (d[i] > tmp2_real)
		      tmp2_real = d[i];
		  FREE (d);
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
      case one_norm:
	switch (a->type)
	  {
	  case integer:
	    type = integer;
	    tmp2_int = 0;
	    switch (a->density)
	      {
	      case dense:
		for (j = 0; j < nc; j++)
		  {
		    tmp_int = 0;
		    for (i = 0; i < nr; i++)
		      tmp_int += abs (a->a.integer[i + j * nr]);
		    if (tmp_int > tmp2_int)
		      tmp2_int = tmp_int;
		  }
		break;
	      case sparse:
		if (nn > 0)
		  {
		    {
		      int *d = (int *) CALLOC (nc, sizeof (int));

		      for (i = 0; i < nr; i++)
			{
			  for (j = a->ia[i]; j < a->ia[i + 1]; j++)
			    {
			      d[a->ja[j - 1] - 1] +=
				abs (a->a.integer[j - 1]);
			    }
			}
		      for (j = 0; j < nc; j++)
			{
			  if (d[j] > tmp2_int)
			    tmp2_int = d[j];
			}
		      FREE (d);
		    }
		  }
		break;
	      case sparse_upper:
		{
		  int *d = (int *) CALLOC (nr, sizeof (int));

		  if (a->d.integer != NULL)
		    memcpy (d, a->d.integer, nr * sizeof (int));
		  for (i = 0; i < nr; i++)
		    d[i] = abs (d[i]);

		  if (nn > 0)
		    {
		      if (a->symmetry == general)
			{
			  for (i = 0; i < nr; i++)
			    {
			      for (j = a->ia[i]; j < a->ia[i + 1]; j++)
				{
				  d[i] += abs (a->a.integer[j - 1]);
				}
			    }
			}
		      else
			{
			  for (i = 0; i < nr; i++)
			    {
			      for (j = a->ia[i]; j < a->ia[i + 1]; j++)
				{
				  d[i] += abs (a->a.integer[j - 1]);
				  d[a->ja[j - 1] - 1] +=
				    abs (a->a.integer[j - 1]);
				}
			    }
			}
		    }

		  for (i = 0; i < nr; i++)
		    if (d[i] > tmp2_int)
		      tmp2_int = d[i];
		  FREE (d);
		}
		break;
	      default:
		BAD_DENSITY (a->density);
		raise_exception ();
	      }
	    break;
	  case real:
	    type = real;
	    tmp2_real = 0;
	    switch (a->density)
	      {
	      case dense:
		for (j = 0; j < nc; j++)
		  {
		    tmp_real = 0;
		    for (i = 0; i < nr; i++)
		      tmp_real += fabs (a->a.real[i + j * nr]);
		    if (tmp_real > tmp2_real)
		      tmp2_real = tmp_real;
		  }
		break;
	      case sparse:
		if (nn > 0)
		  {
		    {
		      REAL *d = (REAL *) CALLOC (nc, sizeof (REAL));

		      for (i = 0; i < nr; i++)
			{
			  for (j = a->ia[i]; j < a->ia[i + 1]; j++)
			    {
			      d[a->ja[j - 1] - 1] += fabs (a->a.real[j - 1]);
			    }
			}
		      for (j = 0; j < nc; j++)
			{
			  if (d[j] > tmp2_real)
			    tmp2_real = d[j];
			}
		      FREE (d);
		    }
		  }
		break;
	      case sparse_upper:
		{
		  REAL *d = (REAL *) CALLOC (nr, sizeof (REAL));

		  if (a->d.real != NULL)
		    memcpy (d, a->d.real, nr * sizeof (REAL));
		  for (i = 0; i < nr; i++)
		    d[i] = fabs (d[i]);

		  if (nn > 0)
		    {
		      if (a->symmetry == general)
			{
			  for (i = 0; i < nr; i++)
			    {
			      for (j = a->ia[i]; j < a->ia[i + 1]; j++)
				{
				  d[i] += fabs (a->a.real[j - 1]);
				}
			    }
			}
		      else
			{
			  for (i = 0; i < nr; i++)
			    {
			      for (j = a->ia[i]; j < a->ia[i + 1]; j++)
				{
				  d[i] += fabs (a->a.real[j - 1]);
				  d[a->ja[j - 1] - 1] +=
				    fabs (a->a.real[j - 1]);
				}
			    }
			}
		    }

		  for (i = 0; i < nr; i++)
		    if (d[i] > tmp2_real)
		      tmp2_real = d[i];
		  FREE (d);
		}
		break;
	      default:
		BAD_DENSITY (a->density);
		raise_exception ();
	      }
	    break;
	  case complex:
	    type = real;
	    tmp2_real = 0;
	    switch (a->density)
	      {
	      case dense:
		for (j = 0; j < nc; j++)
		  {
		    tmp_real = 0;
		    for (i = 0; i < nr; i++)
		      tmp_real += (fabs (a->a.complex[i + j * nr].real) +
				   fabs (a->a.complex[i + j * nr].imag));
		    if (tmp_real > tmp2_real)
		      tmp2_real = tmp_real;
		  }
		break;
	      case sparse:
		if (nn > 0)
		  {
		    {
		      REAL *d = (REAL *) CALLOC (nc, sizeof (REAL));

		      for (i = 0; i < nr; i++)
			{
			  for (j = a->ia[i]; j < a->ia[i + 1]; j++)
			    {
			      d[a->ja[j - 1] - 1] +=
				(fabs (a->a.complex[j - 1].real) +
				 fabs (a->a.complex[j - 1].imag));
			    }
			}
		      for (j = 0; j < nc; j++)
			{
			  if (d[j] > tmp2_real)
			    tmp2_real = d[j];
			}
		      FREE (d);
		    }
		  }
		break;
	      case sparse_upper:
		{
		  REAL *d = (REAL *) CALLOC (nr, sizeof (REAL));

		  if (a->d.complex != NULL)
		    {
		      for (i = 0; i < nr; i++)
			d[i] = (fabs (a->d.complex[i].real) +
				fabs (a->d.complex[i].imag));
		    }

		  if (nn > 0)
		    {
		      if (a->symmetry == general)
			{
			  for (i = 0; i < nr; i++)
			    {
			      for (j = a->ia[i]; j < a->ia[i + 1]; j++)
				{
				  d[i] +=
				    (fabs (a->a.complex[j - 1].real) +
				     fabs (a->a.complex[j - 1].imag));
				}
			    }
			}
		      else
			{
			  for (i = 0; i < nr; i++)
			    {
			      for (j = a->ia[i]; j < a->ia[i + 1]; j++)
				{
				  d[i] +=
				    (fabs (a->a.complex[j - 1].real) +
				     fabs (a->a.complex[j - 1].imag));
				  d[a->ja[j - 1] - 1] +=
				    (fabs (a->a.complex[j - 1].real) +
				     fabs (a->a.complex[j - 1].imag));
				}
			    }
			}
		    }

		  for (i = 0; i < nr; i++)
		    if (d[i] > tmp2_real)
		      tmp2_real = d[i];
		  FREE (d);
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

      case two_norm:

	/*
	 * The 2-norm requires svd.  For efficiency, we'll tell
	 * LAPACK not to compute the singular vectors.  To do that,
	 * we have to build a table with the member "novectors".
	 * (This is pretty sad...)
	 */

	t = replace_in_table ((TABLE *) make_table (),
			      int_to_scalar (1),
			      dup_char ("novectors"));

	v = (VECTOR *) search_in_table ((TABLE *) svd_matrix (EAT (a),
							      EAT (t)),
					dup_char ("sigma"));
	if (v == NULL)
	  wipeout ("Problem computing SVD.");
	EASSERT (v, vector, real);
	type = real;
	tmp2_real = v->a.real[0];
	break;

      case frobenius_norm:
	switch (a->type)
	  {
	  case integer:
	    type = real;
	    tmp2_real = 0.0;
	    for (i = 0; i < nn; i++)
	      {
		tmp2_real += (REAL) (a->a.integer[i] * a->a.integer[i]);
	      }
	    if (a->density == sparse_upper)
	      {
		tmp2_real *= 2;
		if (a->d.integer != NULL)
		  {
		    for (i = 0; i < nr; i++)
		      tmp2_real +=
			(REAL) (a->d.integer[i] * a->d.integer[i]);
		  }
	      }
	    tmp2_real = sqrt (tmp2_real);
	    break;
	  case real:
	    type = real;
	    tmp2_real = 0.0;
	    for (i = 0; i < nn; i++)
	      {
		tmp2_real += a->a.real[i] * a->a.real[i];
	      }
	    if (a->density == sparse_upper)
	      {
		tmp2_real *= 2;
		if (a->d.real != NULL)
		  {
		    for (i = 0; i < nr; i++)
		      tmp2_real += a->d.real[i] * a->d.real[i];
		  }
	      }
	    tmp2_real = sqrt (tmp2_real);
	    break;
	  case complex:
	    type = real;
	    tmp2_real = 0.0;
	    for (i = 0; i < nn; i++)
	      {
		tmp2_real +=
		  (a->a.complex[i].real * a->a.complex[i].real +
		   a->a.complex[i].imag * a->a.complex[i].imag);
	      }
	    if (a->density == sparse_upper)
	      {
		tmp2_real *= 2;
		if (a->d.complex != NULL)
		  {
		    for (i = 0; i < nr; i++)
		      {
			tmp2_real +=
			  (a->d.complex[i].real *
			   a->d.complex[i].real +
			   a->d.complex[i].imag *
			   a->d.complex[i].imag);
		      }
		  }
	      }
	    tmp2_real = sqrt (tmp2_real);
	    break;
	  default:
	    BAD_TYPE (a->type);
	    raise_exception ();
	  }
	break;
      default:
	wipeout ("Bad norm type.");
      }

  }
  ON_EXCEPTION
  {
    delete_matrix (a);
    delete_vector (v);
    delete_entity (t);
  }
  END_EXCEPTION;

  delete_matrix (a);
  delete_vector (v);
  delete_entity (t);
  return ((type == integer) ?
	  int_to_scalar (tmp2_int) : real_to_scalar (tmp2_real));
}

ENTITY *
norm_vector (VECTOR *a, NORM_TYPE nt)
{
  volatile TYPE type;
  int i, j, nn;
  int tmp_int;
  volatile int tmp2_int;
  REAL tmp_real;
  volatile REAL tmp2_real;

  EASSERT (a, vector, 0);

  nn = a->nn;

  WITH_HANDLING
  {

    switch (nt)
      {
      case one_norm:
	switch (a->type)
	  {
	  case integer:
	    type = integer;
	    tmp2_int = 0;
	    switch (a->density)
	      {
	      case dense:
	      case sparse:
		for (j = 0; j < nn; j++)
		  tmp2_int += abs (a->a.integer[j]);
		break;
	      default:
		BAD_DENSITY (a->density);
		raise_exception ();
	      }
	    break;
	  case real:
	    type = real;
	    tmp2_real = 0.0;
	    switch (a->density)
	      {
	      case dense:
	      case sparse:
		for (j = 0; j < nn; j++)
		  tmp2_real += fabs (a->a.real[j]);
		break;
	      default:
		BAD_DENSITY (a->density);
		raise_exception ();
	      }
	    break;
	  case complex:
	    type = real;
	    tmp2_real = 0.0;
	    switch (a->density)
	      {
	      case dense:
	      case sparse:
		for (j = 0; j < nn; j++)
		  {
		    tmp2_real += (fabs (a->a.complex[j].real) +
				  fabs (a->a.complex[j].imag));
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
      case infinity_norm:
	switch (a->type)
	  {
	  case integer:
	    type = integer;
	    tmp2_int = 0;
	    switch (a->density)
	      {
	      case dense:
	      case sparse:
		for (j = 0; j < nn; j++)
		  {
		    tmp_int = abs (a->a.integer[j]);
		    if (tmp_int > tmp2_int)
		      tmp2_int = tmp_int;
		  }
		break;
	      default:
		BAD_DENSITY (a->density);
		raise_exception ();
	      }
	    break;
	  case real:
	    type = real;
	    tmp2_real = 0;
	    switch (a->density)
	      {
	      case dense:
	      case sparse:
		for (j = 0; j < nn; j++)
		  {
		    tmp_real = fabs (a->a.real[j]);
		    if (tmp_real > tmp2_real)
		      tmp2_real = tmp_real;
		  }
		break;
	      default:
		BAD_DENSITY (a->density);
		raise_exception ();
	      }
	    break;
	  case complex:
	    type = real;
	    tmp2_real = 0;
	    switch (a->density)
	      {
	      case dense:
	      case sparse:
		for (j = 0; j < nn; j++)
		  {
		    tmp_real = (fabs (a->a.complex[j].real) +
				fabs (a->a.complex[j].imag));
		    if (tmp_real > tmp2_real)
		      tmp2_real = tmp_real;
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
      case two_norm:
      case frobenius_norm:
	switch (a->type)
	  {
	  case integer:
	    type = real;
	    tmp2_real = 0.0;
	    for (i = 0; i < nn; i++)
	      {
		tmp2_real += (REAL) (a->a.integer[i] * a->a.integer[i]);
	      }
	    tmp2_real = sqrt (tmp2_real);
	    break;
	  case real:
	    type = real;
	    tmp2_real = 0.0;
	    for (i = 0; i < nn; i++)
	      {
		tmp2_real += a->a.real[i] * a->a.real[i];
	      }
	    tmp2_real = sqrt (tmp2_real);
	    break;
	  case complex:
	    type = real;
	    tmp2_real = 0.0;
	    for (i = 0; i < nn; i++)
	      {
		tmp2_real +=
		  (a->a.complex[i].real * a->a.complex[i].real +
		   a->a.complex[i].imag * a->a.complex[i].imag);
	      }
	    tmp2_real = sqrt (tmp2_real);
	    break;
	  default:
	    BAD_TYPE (a->type);
	    raise_exception ();
	  }
	break;
      default:
	wipeout ("Bad norm type.");
      }

  }
  ON_EXCEPTION
  {
    delete_vector (a);
  }
  END_EXCEPTION;

  delete_vector (a);
  return ((type == integer) ?
	  int_to_scalar (tmp2_int) : real_to_scalar (tmp2_real));
}
