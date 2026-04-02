/*
   multiply.c -- Multiplication.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: multiply.c,v 1.5 2003/08/01 04:57:48 ksh Exp $";

#include "multiply.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "full.h"
#include "blas.h"
#include "thin.h"
#include "dense.h"
#include "cast.h"
#include "transpose.h"

static char bad_matrix[] = "Can't multiply a \"%s\" matrix.";
static char bad_labels[] = "Labels don't match.";

ENTITY *
multiply_scalar_integer (SCALAR *l, SCALAR *r)
{
  SCALAR * volatile s = NULL;

  EASSERT (l, scalar, integer);
  EASSERT (r, scalar, integer);

  WITH_HANDLING
  {
    if (r->entity.ref_count == 1)
      {
	s = (SCALAR *) EAT (r);
	s->v.integer *= l->v.integer;
      }
    else
      {
	s = (SCALAR *) dup_scalar ((SCALAR *) EAT (l));
	s->v.integer *= r->v.integer;
      }
  }
  ON_EXCEPTION
  {
    delete_3_scalars (l, r, s);
  }
  END_EXCEPTION;

  delete_2_scalars (l, r);
  return (ENT (s));
}

ENTITY *
multiply_scalar_real (SCALAR *l, SCALAR *r)
{
  SCALAR * volatile s = NULL;

  EASSERT (l, scalar, real);
  EASSERT (r, scalar, real);

  WITH_HANDLING
  {
    if (r->entity.ref_count == 1)
      {
	s = (SCALAR *) EAT (r);
	s->v.real *= l->v.real;
      }
    else
      {
	s = (SCALAR *) dup_scalar ((SCALAR *) EAT (l));
	s->v.real *= r->v.real;
      }
  }
  ON_EXCEPTION
  {
    delete_3_scalars (l, r, s);
  }
  END_EXCEPTION;

  delete_2_scalars (l, r);
  return (ENT (s));
}

ENTITY *
multiply_scalar_complex (SCALAR *l, SCALAR *r)
{
  SCALAR * volatile s = NULL;
  COMPLEX tmp;

  EASSERT (l, scalar, complex);
  EASSERT (r, scalar, complex);

  WITH_HANDLING
  {
    s = (SCALAR *) dup_scalar ((SCALAR *) EAT (l));
    tmp.real = (s->v.complex.real * r->v.complex.real -
		s->v.complex.imag * r->v.complex.imag);
    tmp.imag = (s->v.complex.real * r->v.complex.imag +
		s->v.complex.imag * r->v.complex.real);
    s->v.complex.real = tmp.real;
    s->v.complex.imag = tmp.imag;
  }
  ON_EXCEPTION
  {
    delete_3_scalars (l, r, s);
  }
  END_EXCEPTION;

  delete_2_scalars (l, r);
  return (ENT (s));
}

ENTITY *
multiply_scalar_matrix_integer (SCALAR *l, MATRIX *r)
{
  MATRIX * volatile m = NULL;
  int i;

  EASSERT (l, scalar, integer);
  EASSERT (r, matrix, integer);

  WITH_HANDLING
  {
    if (l->v.integer == 0)
      {
	m = (MATRIX *) make_matrix (r->nr, r->nc, r->type, sparse);
	m->rid = (r->rid == NULL) ? NULL : copy_entity (r->rid);
	m->cid = (r->cid == NULL) ? NULL : copy_entity (r->cid);
      }
    else
      {
	m = (MATRIX *) dup_matrix ((MATRIX *) EAT (r));
	for (i = 0; i < m->nn; i++)
	  m->a.integer[i] *= l->v.integer;
	if (m->d.integer != NULL)
	  for (i = 0; i < m->nr; i++)
	    m->d.integer[i] *= l->v.integer;
      }
  }
  ON_EXCEPTION
  {
    delete_scalar (l);
    delete_2_matrices (r, m);
  }
  END_EXCEPTION;

  delete_scalar (l);
  delete_matrix (r);
  return (ENT (m));
}

ENTITY *
multiply_scalar_matrix_real (SCALAR *l, MATRIX *r)
{
  MATRIX * volatile m = NULL;
  int i;

  EASSERT (l, scalar, real);
  EASSERT (r, matrix, real);

  WITH_HANDLING
  {
    if (l->v.real == 0.0)
      {
	m = (MATRIX *) make_matrix (r->nr, r->nc, r->type, sparse);
	m->rid = (r->rid == NULL) ? NULL : copy_entity (r->rid);
	m->cid = (r->cid == NULL) ? NULL : copy_entity (r->cid);
      }
    else
      {
	m = (MATRIX *) dup_matrix ((MATRIX *) EAT (r));
	DSCAL (&m->nn, &l->v.real, m->a.real, (i = 1, &i));
	if (m->d.real != NULL)
	  DSCAL (&m->nr, &l->v.real, m->d.real, (i = 1, &i));
      }
  }
  ON_EXCEPTION
  {
    delete_scalar (l);
    delete_2_matrices (r, m);
  }
  END_EXCEPTION;

  delete_scalar (l);
  delete_matrix (r);
  return (ENT (m));
}

ENTITY *
multiply_scalar_matrix_complex (SCALAR *l, MATRIX *r)
{
  MATRIX *m = NULL;
  int i;

  EASSERT (l, scalar, complex);
  EASSERT (r, matrix, complex);

  WITH_HANDLING
  {
    if (l->v.complex.real == 0.0 && l->v.complex.imag == 0.0)
      {
	m = (MATRIX *) make_matrix (r->nr, r->nc, r->type, sparse);
	m->rid = (r->rid == NULL) ? NULL : copy_entity (r->rid);
	m->cid = (r->cid == NULL) ? NULL : copy_entity (r->cid);
      }
    else
      {
	m = (MATRIX *) dup_matrix ((MATRIX *) EAT (r));

	/*
	 * If the imaginary part is not zero, then hermitian
	 * symmetry is lost.  If it's diagonal it can drop back to
	 * symmetric, otherwise it goes to general.
	 */

	if ((m->symmetry == hermitian) &&
	    l->v.complex.imag != 0)
	  {
	    if (m->nn == 0)
	      {
		m->symmetry = symmetric;
	      }
	    else
	      {
		m = (MATRIX *) full_matrix ((MATRIX *) EAT (m));
		m->symmetry = general;
	      }
	  }
	ZSCAL (&m->nn, &l->v.complex, m->a.complex, (i = 1, &i));
	if (m->d.complex != NULL)
	  {
	    ZSCAL (&m->nr, &l->v.complex, m->d.complex, (i = 1, &i));
	  }
      }
  }
  ON_EXCEPTION
  {
    delete_scalar (l);
    delete_2_matrices (r, m);
  }
  END_EXCEPTION;

  delete_scalar (l);
  delete_matrix (r);
  return (ENT (m));
}

ENTITY *
multiply_scalar_vector_integer (SCALAR *l, VECTOR *r)
{
  VECTOR * volatile v = NULL;
  int i;

  EASSERT (l, scalar, integer);
  EASSERT (r, vector, integer);

  WITH_HANDLING
  {
    if (l->v.integer == 0)
      {
	v = (VECTOR *) make_vector (r->ne, r->type, sparse);
	v->eid = (r->eid == NULL) ? NULL : copy_entity (r->eid);
      }
    else
      {
	v = (VECTOR *) dup_vector ((VECTOR *) EAT (r));
	for (i = 0; i < v->nn; i++)
	  v->a.integer[i] *= l->v.integer;
      }
  }
  ON_EXCEPTION
  {
    delete_scalar (l);
    delete_2_vectors (r, v);
  }
  END_EXCEPTION;

  delete_scalar (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
multiply_scalar_vector_real (SCALAR *l, VECTOR *r)
{
  VECTOR * volatile v = NULL;
  int i;

  EASSERT (l, scalar, real);
  EASSERT (r, vector, real);

  WITH_HANDLING
  {
    if (l->v.real == 0.0)
      {
	v = (VECTOR *) make_vector (r->ne, r->type, sparse);
	v->eid = (r->eid == NULL) ? NULL : copy_entity (r->eid);
      }
    else
      {
	v = (VECTOR *) dup_vector ((VECTOR *) EAT (r));
	DSCAL (&v->nn, &l->v.real, v->a.real, (i = 1, &i));
      }
  }
  ON_EXCEPTION
  {
    delete_scalar (l);
    delete_2_vectors (r, v);
  }
  END_EXCEPTION;

  delete_scalar (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
multiply_scalar_vector_complex (SCALAR *l, VECTOR *r)
{
  VECTOR * volatile v = NULL;
  int i;

  EASSERT (l, scalar, complex);
  EASSERT (r, vector, complex);

  WITH_HANDLING
  {
    if (l->v.complex.real == 0.0 && l->v.complex.imag == 0.0)
      {
	v = (VECTOR *) make_vector (r->ne, r->type, sparse);
	v->eid = (r->eid == NULL) ? NULL : copy_entity (r->eid);
      }
    else
      {
	v = (VECTOR *) dup_vector ((VECTOR *) EAT (r));
	ZSCAL (&v->nn, &l->v.complex, v->a.complex, (i = 1, &i));
      }
  }
  ON_EXCEPTION
  {
    delete_scalar (l);
    delete_2_vectors (r, v);
  }
  END_EXCEPTION;

  delete_scalar (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
multiply_matrix_integer (MATRIX *l, MATRIX *r)
{
  ENTITY * volatile rid = NULL, * volatile cid = NULL;
  int i, j, k, maxjc;
  int *tmp = NULL;
  int *ja_tmp = NULL;
  int *a_tmp = NULL;
  MATRIX * volatile m = NULL;

  EASSERT (l, matrix, integer);
  EASSERT (r, matrix, integer);

  if (l->nc != r->nr)
    {
      fail ("Inconsistent dimensions for matrix multiply.  Left matrix has %d column%s, but right matrix has %d row%s.\n",
	    l->nc, PLURAL (l->nc), r->nr, PLURAL (r->nr));
      delete_2_matrices (l, r);
      raise_exception ();
    }

  if (!MATCH_VECTORS (l->cid, r->rid))
    {
      fail (bad_labels);
      delete_2_matrices (l, r);
      raise_exception ();
    }

  if (l->rid != NULL)
    rid = copy_entity (l->rid);
  if (r->cid != NULL)
    cid = copy_entity (r->cid);

  WITH_HANDLING
  {
    switch (l->density)
      {
      case dense:
	switch (r->density)
	  {
	  case dense:
	    m = (MATRIX *) form_matrix (l->nr, r->nc, integer, dense);
	    m->symmetry = general;
	    for (i = 0; i < l->nr; i++)
	      {
		POLL_SIGINT ();
		for (j = 0; j < r->nc; j++)
		  {
		    m->a.integer[i + m->nr * j] = 0;
		    for (k = 0; k < l->nc; k++)
		      {
			m->a.integer[i + m->nr * j] +=
			  l->a.integer[i + l->nr * k] *
			  r->a.integer[k + r->nr * j];
		      }
		  }
	      }
	    break;
	  case sparse:
	    if (r->nn == 0)
	      {
		m = (MATRIX *) make_matrix (l->nr, r->nc, integer, sparse);
	      }
	    else
	      {
		m = (MATRIX *) make_matrix (l->nr, r->nc, integer, dense);
		m->symmetry = general;
		for (i = 0; i < m->nr; i++)
		  {
		    POLL_SIGINT ();
		    for (j = 0; j < r->nr; j++)
		      {
			for (k = r->ia[j]; k < r->ia[j + 1]; k++)
			  {
			    m->a.integer[i + m->nr * (r->ja[k - 1] - 1)] +=
			      l->a.integer[i + l->nr * j] * r->a.integer[k - 1];
			  }
		      }
		  }
	      }
	    break;
	  case sparse_upper:

	    /*
	     * Here we have dense times sparse_upper.  Let's do it if it's
	     * diagonal and convert to full otherwise.
	     */

	    if (r->nn > 0)
	      {			/* Are there off-diagonal non-zeros? */
		r = (MATRIX *) full_matrix (EAT (r));
		m = (MATRIX *) multiply_matrix_integer (EAT (l), EAT (r));
	      }
	    else
	      {
		/* Is it completely zero? */
		if (r->d.integer == NULL)
		  {
		    m = (MATRIX *) make_matrix (l->nr, r->nc,
						integer, sparse);
		  }
		else
		  {
		    int *p, s;
		    m = (MATRIX *) dup_matrix (EAT (l));
		    m->symmetry = general;
		    p = m->a.integer;
		    for (j = 0; j < m->nc; j++)
		      {
			POLL_SIGINT ();
			s = r->d.integer[j];
			for (i = 0; i < m->nr; i++)
			  {
			    *p++ *= s;
			  }
		      }
		  }
	      }
	    break;

	  default:
	    fail (bad_matrix, density_string[r->density]);
	    raise_exception ();
	  }
	break;
      case sparse:
	switch (r->density)
	  {
	  case dense:
	    if (l->nn == 0)
	      {
		m = (MATRIX *) make_matrix (l->nr, r->nc, integer, sparse);
	      }
	    else
	      {
		m = (MATRIX *) make_matrix (l->nr, r->nc, integer, dense);
		m->symmetry = general;
		for (i = 0; i < m->nr; i++)
		  {
		    POLL_SIGINT ();
		    for (j = 0; j < m->nc; j++)
		      {
			for (k = l->ia[i] - 1; k < l->ia[i + 1] - 1; k++)
			  {
			    m->a.integer[i + m->nr * j] +=
			      l->a.integer[k] *
			      r->a.integer[l->ja[k] - 1 + r->nr * j];
			  }
		      }
		  }
	      }
	    break;
	  case sparse:
	    m = (MATRIX *) make_matrix (l->nr, r->nc, integer, sparse);
	    if (l->nn > 0 && r->nn > 0)
	      {
		if (m->ia == NULL)
		  m->ia = (int *) MALLOC ((m->nr + 1) * sizeof (int));
		tmp = (int *) MALLOC ((m->nc + 1) * sizeof (int));
		maxjc = l->nn + r->nn;
		ja_tmp = NULL;
		do
		  {
		    POLL_SIGINT ();
		    maxjc *= 2;
		    TFREE (ja_tmp);
		    ja_tmp = (int *) MALLOC (maxjc * sizeof (int));
		    XGSMUL (l->ia, l->ja, r->ia, r->ja, &m->nr, &m->nc,
			    m->ia, ja_tmp, &maxjc, tmp);
		  }
		while (!m->ia[0]);
		m->nn = m->ia[m->nr] - 1;
		m->order = ordered;
		if (m->nn)
		  {
		    m->symmetry = general;
		    m->ja = dup_mem (ja_tmp, m->nn * sizeof (int));
		    SIGINT_RAISE_ON ();
		    XGSTRN (m->ia, m->ja, &m->nr, &m->nc, tmp, ja_tmp);
		    XGSTRN (tmp, ja_tmp, &m->nc, &m->nr, m->ia, m->ja);
		    SIGINT_RAISE_OFF ();
		    FREE (EAT (tmp));
		    FREE (EAT (ja_tmp));
		    m->a.integer = (int *) MALLOC (m->nn * sizeof (int));
		    a_tmp = MALLOC ((m->nc + 1) * sizeof (int));
		    SIGINT_RAISE_ON ();
		    IGSMUL (l->ia, l->ja, l->a.integer,
			    r->ia, r->ja, r->a.integer,
			    &m->nr, &m->nc,
			    m->ia, m->ja, m->a.integer,
			    a_tmp);
		    SIGINT_RAISE_OFF ();
		    FREE (EAT (a_tmp));
		  }
		else
		  {
		    m->symmetry = (m->nr == m->nc) ? symmetric : general;
		    FREE (EAT (m->ia));
		    FREE (EAT (tmp));
		    FREE (EAT (ja_tmp));
		  }
	      }
	    break;
	  case sparse_upper:
	    m = (MATRIX *) multiply_matrix_integer (
						     (MATRIX *) EAT (l),
			       (MATRIX *) full_matrix ((MATRIX *) EAT (r)));
	    break;
	  default:
	    fail (bad_matrix, density_string[r->density]);
	    raise_exception ();
	  }
	break;

      case sparse_upper:

	switch (r->density)
	  {
	  case dense:

	    /* sparse_upper * dense */

	    if (l->nn == 0)
	      {

		/* `l' is diagonal */

		if (l->d.integer == NULL)
		  {		/* Is it completely zero? */
		    m = (MATRIX *) make_matrix (l->nr, r->nc,
						integer, sparse);
		  }
		else
		  {
		    int *p, *s;
		    m = (MATRIX *) dup_matrix (EAT (r));
		    m->symmetry = general;
		    p = m->a.integer;
		    for (j = 0; j < m->nc; j++)
		      {
			POLL_SIGINT ();
			s = l->d.integer;
			for (i = 0; i < m->nr; i++)
			  {
			    *p++ *= *s++;
			  }
		      }
		  }
	      }
	    else
	      {
		l = (MATRIX *) full_matrix (EAT (l));
		m = (MATRIX *) multiply_matrix_integer (EAT (l), EAT (r));
	      }

	    break;

	  case sparse_upper:

	    /* Are they both diagonal? */

	    if (l->nn == 0 && r->nn == 0)
	      {
		/* Are both non-zero? */

		if (l->d.integer)
		  {
		    if (r->d.integer)
		      {
			m = (MATRIX *) dup_matrix (EAT (l));
			for (i = 0; i < m->nr; i++)
			  m->d.integer[i] *= r->d.integer[i];
		      }
		    else
		      {
			m = (MATRIX *) copy_matrix (EAT (r));
		      }
		  }
		else
		  {
		    m = (MATRIX *) copy_matrix (EAT (l));
		  }
		break;
	      }

	    /* Otherwise, fall through. */

	  case sparse:

	    /*
	     * Someday, we'll write code to really do these cases, but for
	     * now we'll just convert to full storage.
	     */

	    l = (MATRIX *) full_matrix (EAT (l));
	    r = (MATRIX *) full_matrix (EAT (r));
	    m = (MATRIX *) multiply_matrix_integer (EAT (l), EAT (r));

	    break;

	  default:
	    fail (bad_matrix, density_string[r->density]);
	    raise_exception ();
	  }
	break;

      default:
	fail (bad_matrix, density_string[l->density]);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    TFREE (tmp);
    TFREE (ja_tmp);
    TFREE (a_tmp);
    delete_2_entities (rid, cid);
    delete_3_matrices (l, r, m);
  }
  END_EXCEPTION;

  delete_2_entities (m->rid, m->cid);
  m->rid = rid;
  m->cid = cid;

  delete_2_matrices (l, r);
  return (ENT (m));
}

ENTITY *
multiply_matrix_real (MATRIX *l, MATRIX *r)
{
  /* Multiply two real matrices. */

  ENTITY * volatile rid = NULL, * volatile cid = NULL;
  int i, j, k, maxjc;
  int *tmp = NULL;
  int *ja_tmp = NULL;
  REAL *a_tmp = NULL;
  MATRIX * volatile m = NULL;

  EASSERT (l, matrix, real);
  EASSERT (r, matrix, real);

  /* Dimensions must be consistent. */

  if (l->nc != r->nr)
    {
      fail ("Inconsistent dimensions for matrix multiply.  Left matrix has %d column%s, but right matrix has %d row%s.\n",
	    l->nc, PLURAL (l->nc), r->nr, PLURAL (r->nr));
      delete_2_matrices (l, r);
      raise_exception ();
    }

  /* Labels must match. */

  if (!MATCH_VECTORS (l->cid, r->rid))
    {
      fail (bad_labels);
      delete_2_matrices (l, r);
      raise_exception ();
    }

  /* These will be the new labels. */

  if (l->rid != NULL)
    rid = copy_entity (l->rid);
  if (r->cid != NULL)
    cid = copy_entity (r->cid);

  WITH_HANDLING
  {
    switch (l->density)
      {
      case dense:
	switch (r->density)
	  {
	  case dense:
	    m = (MATRIX *) form_matrix (l->nr, r->nc, real, dense);
	    m->symmetry = general;
	    if (m->nn > 0 && l->nc > 0)
	      {
		int itrnsa = 'n';
		int itrnsb = 'n';
		REAL alpha = 1.0;
		REAL beta = 0.0;
		SIGINT_RAISE_ON ();
		RGEMM (&itrnsa, &itrnsb, &l->nr, &r->nc, &l->nc, &alpha,
		       l->a.real, &l->nr, r->a.real, &r->nr, &beta,
		       m->a.real, &m->nr);
		SIGINT_RAISE_OFF ();
	      }
	    else
	      {
		for (i = 0; i < m->nn; i++)
		  m->a.real[i] = 0.0;
	      }
	    break;
	  case sparse:
	    if (r->nn == 0)
	      {
		m = (MATRIX *) make_matrix (l->nr, r->nc, real, sparse);
	      }
	    else
	      {
		m = (MATRIX *) make_matrix (l->nr, r->nc, real, dense);
		m->symmetry = general;
		for (i = 0; i < m->nr; i++)
		  {
		    POLL_SIGINT ();
		    for (j = 0; j < r->nr; j++)
		      {
			for (k = r->ia[j]; k < r->ia[j + 1]; k++)
			  {
			    m->a.real[i + m->nr * (r->ja[k - 1] - 1)] +=
			      l->a.real[i + l->nr * j] * r->a.real[k - 1];
			  }
		      }
		  }
	      }
	    break;
	  case sparse_upper:

	    /*
	     * Here we have dense times sparse_upper.  Let's do it if it's
	     * diagonal and convert to full otherwise.
	     */

	    if (r->nn > 0)
	      {			/* Are there off-diagonal non-zeros? */
		r = (MATRIX *) full_matrix (EAT (r));
		m = (MATRIX *) multiply_matrix_real (EAT (l), EAT (r));
	      }
	    else
	      {
		if (r->d.real == NULL)
		  {		/* Is it completely zero? */
		    m = (MATRIX *) make_matrix (l->nr, r->nc, real,
						sparse);
		  }
		else
		  {
		    REAL *p, s;
		    m = (MATRIX *) dup_matrix (EAT (l));
		    m->symmetry = general;
		    p = m->a.real;
		    for (j = 0; j < m->nc; j++)
		      {
			POLL_SIGINT ();
			s = r->d.real[j];
			for (i = 0; i < m->nr; i++)
			  {
			    *p++ *= s;
			  }
		      }
		  }
	      }
	    break;

	  default:
	    fail (bad_matrix, density_string[r->density]);
	    raise_exception ();
	  }
	break;
      case sparse:
	switch (r->density)
	  {
	  case dense:
	    if (l->nn == 0)
	      {
		m = (MATRIX *) make_matrix (l->nr, r->nc, real, sparse);
	      }
	    else
	      {
		m = (MATRIX *) make_matrix (l->nr, r->nc, real, dense);
		m->symmetry = general;
		for (i = 0; i < m->nr; i++)
		  {
		    POLL_SIGINT ();
		    for (j = 0; j < m->nc; j++)
		      {
			for (k = l->ia[i] - 1; k < l->ia[i + 1] - 1; k++)
			  {
			    m->a.real[i + m->nr * j] +=
			      l->a.real[k] * r->a.real[l->ja[k]-1 + r->nr * j];
			  }
		      }
		  }
	      }
	    break;
	  case sparse:
	    m = (MATRIX *) make_matrix (l->nr, r->nc, real, sparse);
	    if (l->nn > 0 && r->nn > 0)
	      {
		if (m->ia == NULL)
		  m->ia = (int *) MALLOC ((m->nr + 1) * sizeof (int));
		tmp = (int *) MALLOC ((m->nc + 1) * sizeof (int));
		maxjc = l->nn + r->nn;
		ja_tmp = NULL;
		do
		  {
		    POLL_SIGINT ();
		    maxjc *= 2;
		    TFREE (ja_tmp);
		    ja_tmp = (int *) MALLOC (maxjc * sizeof (int));
		    XGSMUL (l->ia, l->ja, r->ia, r->ja, &m->nr, &m->nc,
			    m->ia, ja_tmp, &maxjc, tmp);
		  }
		while (!m->ia[0]);
		m->nn = m->ia[m->nr] - 1;
		m->order = ordered;
		if (m->nn)
		  {
		    m->symmetry = general;
		    m->ja = dup_mem (ja_tmp, m->nn * sizeof (int));
		    SIGINT_RAISE_ON ();
		    XGSTRN (m->ia, m->ja, &m->nr, &m->nc, tmp, ja_tmp);
		    XGSTRN (tmp, ja_tmp, &m->nc, &m->nr, m->ia, m->ja);
		    SIGINT_RAISE_OFF ();
		    FREE (EAT (tmp));
		    FREE (EAT (ja_tmp));
		    m->a.real = (REAL *) MALLOC (m->nn * sizeof (REAL));
		    a_tmp = (REAL *) MALLOC ((m->nc + 1) * sizeof (REAL));
		    SIGINT_RAISE_ON ();
		    DGSMUL (l->ia, l->ja, l->a.real,
			    r->ia, r->ja, r->a.real,
			    &m->nr, &m->nc,
			    m->ia, m->ja, m->a.real,
			    a_tmp);
		    SIGINT_RAISE_OFF ();
		    FREE (EAT (a_tmp));
		  }
		else
		  {
		    m->symmetry = (m->nr == m->nc) ? symmetric : general;
		    FREE (EAT (m->ia));
		    FREE (EAT (tmp));
		    FREE (EAT (ja_tmp));
		  }
	      }
	    break;
	  case sparse_upper:
	    m = (MATRIX *) multiply_matrix_real ((MATRIX *) EAT (l),
			       (MATRIX *) full_matrix ((MATRIX *) EAT (r)));
	    break;
	  default:
	    fail (bad_matrix, density_string[r->density]);
	    raise_exception ();
	  }
	break;

      case sparse_upper:

	switch (r->density)
	  {
	  case dense:

	    /* sparse_upper * dense */

	    if (l->nn == 0)
	      {

		/* `l' is diagonal */

		if (l->d.real == NULL)
		  {		/* Is it completely zero? */
		    m = (MATRIX *) make_matrix (l->nr, r->nc,
						real, sparse);
		  }
		else
		  {
		    REAL *p, *s;
		    m = (MATRIX *) dup_matrix (EAT (r));
		    m->symmetry = general;
		    p = m->a.real;
		    for (j = 0; j < m->nc; j++)
		      {
			POLL_SIGINT ();
			s = l->d.real;
			for (i = 0; i < m->nr; i++)
			  {
			    *p++ *= *s++;
			  }
		      }
		  }
		break;
	      }
	    else
	      {
		l = (MATRIX *) full_matrix (EAT (l));
		m = (MATRIX *) multiply_matrix_real (EAT (l), EAT (r));
	      }
 
	    break;
 
	  case sparse_upper:
 
	    /* Are they both diagonal? */
 
	    if (l->nn == 0 && r->nn == 0)
	      {
		/* Are both non-zero? */
 
		if (l->d.real)
		  {
		    if (r->d.real)
		      {
			m = (MATRIX *) dup_matrix (EAT (l));
			for (i = 0; i < m->nr; i++)
			  m->d.real[i] *= r->d.real[i];
		      }
		    else
		      {
			m = (MATRIX *) copy_matrix (EAT (r));
		      }
		  }
		else
		  {
		    m = (MATRIX *) copy_matrix (EAT (l));
		  }
		break;
	      }

	    /* Otherwise, fall through. */

	  case sparse:

	    /*
	     * Someday, we'll write code to really do these cases, but for
	     * now we'll just convert to full storage.
	     */

	    l = (MATRIX *) full_matrix (EAT (l));
	    r = (MATRIX *) full_matrix (EAT (r));
	    m = (MATRIX *) multiply_matrix_real (EAT (l), EAT (r));

	    break;

	  default:
	    fail (bad_matrix, density_string[r->density]);
	    raise_exception ();
	  }
	break;

      default:
	fail (bad_matrix, density_string[l->density]);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    TFREE (tmp);
    TFREE (ja_tmp);
    TFREE (a_tmp);
    delete_2_entities (rid, cid);
    delete_3_matrices (l, r, m);
  }
  END_EXCEPTION;

  delete_2_entities (m->rid, m->cid);
  m->rid = rid;
  m->cid = cid;

  delete_2_matrices (l, r);
  return (ENT (m));
}

ENTITY *
multiply_matrix_complex (MATRIX *l, MATRIX *r)
{
  ENTITY * volatile rid = NULL, * volatile cid = NULL;
  int i, j, k, maxjc;
  int *tmp = NULL;
  int *ja_tmp = NULL;
  COMPLEX *a_tmp = NULL;
  MATRIX * volatile m = NULL;

  EASSERT (l, matrix, complex);
  EASSERT (r, matrix, complex);

  if (l->nc != r->nr)
    {
      fail ("Inconsistent dimensions for matrix multiply.  Left matrix has %d column%s, but right matrix has %d row%s.\n",
	    l->nc, PLURAL (l->nc), r->nr, PLURAL (r->nr));
      delete_2_matrices (l, r);
      raise_exception ();
    }

  if (!MATCH_VECTORS (l->cid, r->rid))
    {
      fail (bad_labels);
      delete_2_matrices (l, r);
      raise_exception ();
    }

  if (l->rid != NULL)
    rid = copy_entity (l->rid);
  if (r->cid != NULL)
    cid = copy_entity (r->cid);

  WITH_HANDLING
  {
    switch (l->density)
      {
      case dense:
	switch (r->density)
	  {
	  case dense:
	    m = (MATRIX *) form_matrix (l->nr, r->nc, complex, dense);
	    m->symmetry = general;
	    if (m->nn > 0 && l->nc > 0)
	      {
		int itrnsa = 'n';
		int itrnsb = 'n';
		COMPLEX alpha, beta;
		alpha.real = 1.0;
		alpha.imag = 0.0;
		beta.real = 0.0;
		beta.imag = 0.0;
		SIGINT_RAISE_ON ();
		XGEMM (&itrnsa, &itrnsb, &l->nr, &r->nc, &l->nc, &alpha,
		       l->a.complex, &l->nr, r->a.complex, &r->nr, &beta,
		       m->a.complex, &m->nr);
		SIGINT_RAISE_OFF ();
	      }
	    else
	      {
		for (i = 0; i < m->nn; i++)
		  {
		    m->a.complex[i].real = 0.0;
		    m->a.complex[i].imag = 0.0;
		  }
	      }
	    break;
	  case sparse:
	    if (r->nn == 0)
	      {
		m = (MATRIX *) make_matrix (l->nr, r->nc,
					    complex, sparse);
	      }
	    else
	      {
		m = (MATRIX *) make_matrix (l->nr, r->nc, complex, dense);
		m->symmetry = general;
		for (i = 0; i < m->nr; i++)
		  {
		    POLL_SIGINT ();
		    for (j = 0; j < r->nr; j++)
		      {
			for (k = r->ia[j]; k < r->ia[j + 1]; k++)
			  {
			    m->a.complex[i + m->nr * (r->ja[k - 1] - 1)].real +=
			      l->a.complex[i + l->nr * j].real *
			      r->a.complex[k - 1].real -
			      l->a.complex[i + l->nr * j].imag *
			      r->a.complex[k - 1].imag;
			    m->a.complex[i + m->nr * (r->ja[k - 1] - 1)].imag +=
			      l->a.complex[i + l->nr * j].real *
			      r->a.complex[k - 1].imag +
			      l->a.complex[i + l->nr * j].imag *
			      r->a.complex[k - 1].real;
			  }
		      }
		  }
	      }
	    break;
	  case sparse_upper:

	    /*
	     * Here we have dense times sparse_upper.  Let's do it if it's
	     * diagonal and convert to full otherwise.
	     */

	    if (r->nn > 0)
	      {			/* Are there off-diagonal non-zeros? */
		r = (MATRIX *) full_matrix (EAT (r));
		m = (MATRIX *) multiply_matrix_complex (EAT (l), EAT (r));
	      }
	    else
	      {
		/* Is it completely zero? */
		if (r->d.complex == NULL)
		  {
		    m = (MATRIX *) make_matrix (l->nr, r->nc,
						complex, sparse);
		  }
		else
		  {
		    m = (MATRIX *) dup_matrix (EAT (l));
		    m->symmetry = general;
		    for (j = 0; j < m->nc; j++)
		      {
			POLL_SIGINT ();
			ZSCAL (&m->nr, r->d.complex + j,
			       m->a.complex + j * m->nr, (i = 1, &i));
		      }
		  }
	      }
	    break;

	  default:
	    fail (bad_matrix, density_string[r->density]);
	    raise_exception ();
	  }
	break;
      case sparse:
	switch (r->density)
	  {
	  case dense:
	    if (l->nn == 0)
	      {
		m = (MATRIX *) make_matrix (l->nr, r->nc,
					    complex, sparse);
	      }
	    else
	      {
		m = (MATRIX *) make_matrix (l->nr, r->nc, complex, dense);
		m->symmetry = general;
		for (i = 0; i < m->nr; i++)
		  {
		    POLL_SIGINT ();
		    for (j = 0; j < m->nc; j++)
		      {
			for (k = l->ia[i] - 1; k < l->ia[i + 1] - 1; k++)
			  {
			    m->a.complex[i + m->nr * j].real +=
			      l->a.complex[k].real *
			      r->a.complex[l->ja[k] - 1 + r->nr * j].real -
			      l->a.complex[k].imag *
			      r->a.complex[l->ja[k] - 1 + r->nr * j].imag;
			    m->a.complex[i + m->nr * j].imag +=
			      l->a.complex[k].real *
			      r->a.complex[l->ja[k] - 1 + r->nr * j].imag +
			      l->a.complex[k].imag *
			      r->a.complex[l->ja[k] - 1 + r->nr * j].real;
			  }
		      }
		  }
	      }
	    break;
	  case sparse:
	    m = (MATRIX *) make_matrix (l->nr, r->nc, complex, sparse);
	    if (l->nn > 0 && r->nn > 0)
	      {
		if (m->ia == NULL)
		  m->ia = (int *) MALLOC ((m->nr + 1) * sizeof (int));
		tmp = (int *) MALLOC ((m->nc + 1) * sizeof (int));
		maxjc = l->nn + r->nn;
		ja_tmp = NULL;
		do
		  {
		    POLL_SIGINT ();
		    maxjc *= 2;
		    TFREE (ja_tmp);
		    ja_tmp = (int *) MALLOC (maxjc * sizeof (int));
		    XGSMUL (l->ia, l->ja, r->ia, r->ja, &m->nr, &m->nc,
			    m->ia, ja_tmp, &maxjc, tmp);
		  }
		while (!m->ia[0]);
		m->nn = m->ia[m->nr] - 1;
		m->order = ordered;
		if (m->nn)
		  {
		    m->symmetry = general;
		    m->ja = dup_mem (ja_tmp, m->nn * sizeof (int));
		    SIGINT_RAISE_ON ();
		    XGSTRN (m->ia, m->ja, &m->nr, &m->nc, tmp, ja_tmp);
		    XGSTRN (tmp, ja_tmp, &m->nc, &m->nr, m->ia, m->ja);
		    SIGINT_RAISE_OFF ();
		    FREE (EAT (tmp));
		    FREE (EAT (ja_tmp));
		    m->a.complex =
		      (COMPLEX *) MALLOC (m->nn * sizeof (COMPLEX));
		    a_tmp = (COMPLEX *) MALLOC ((m->nc+1) * sizeof (COMPLEX));
		    SIGINT_RAISE_ON ();
		    ZGSMUL (l->ia, l->ja, l->a.complex,
			    r->ia, r->ja, r->a.complex,
			    &m->nr, &m->nc,
			    m->ia, m->ja, m->a.complex,
			    a_tmp);
		    SIGINT_RAISE_OFF ();
		    FREE (EAT (a_tmp));
		  }
		else
		  {
		    m->symmetry = (m->nr == m->nc) ? hermitian : general;
		    FREE (EAT (m->ia));
		    FREE (EAT (tmp));
		    FREE (EAT (ja_tmp));
		  }
	      }
	    break;
	  case sparse_upper:
	    m = (MATRIX *) multiply_matrix_complex (
						     (MATRIX *) EAT (l),
			       (MATRIX *) full_matrix ((MATRIX *) EAT (r)));
	    break;
	  default:
	    fail (bad_matrix, density_string[r->density]);
	    raise_exception ();
	  }
	break;
      case sparse_upper:

	switch (r->density)
	  {

	  case dense:

	    /* sparse_upper * dense */

	    if (l->nn == 0)
	      {

		/* `l' is diagonal */

		if (l->d.complex == NULL)
		  {		/* Is it completely zero? */
		    m = (MATRIX *) make_matrix (l->nr, r->nc,
						complex, sparse);
		  }
		else
		  {
		    m = (MATRIX *) dup_matrix (EAT (r));
		    m->symmetry = general;
		    for (i = 0; i < m->nr; i++)
		      {
			POLL_SIGINT ();
			ZSCAL (&m->nc, l->d.complex + i,
			       m->a.complex + i, (j = m->nr, &j));
		      }
		  }
		break;
	      }
	    else
	      {
		l = (MATRIX *) full_matrix (EAT (l));
		m = (MATRIX *) multiply_matrix_complex (EAT (l), EAT (r));
	      }
 
	    break;
 
	  case sparse_upper:
 
	    /* Are they both diagonal? */
 
	    if (l->nn == 0 && r->nn == 0)
	      {
		/* Are both non-zero? */
 
		if (l->d.complex)
		  {
		    if (r->d.complex)
		      {
			REAL lr, li, rr, ri;

			m = (MATRIX *) dup_matrix (EAT (l));

			/*
			 * If both are hermitian, so is result.  Otherwise,
			 * result is symmetric.
			 */

			m->symmetry = (m->symmetry == hermitian &&
				       r->symmetry == hermitian) ?
			  hermitian : symmetric;
			  
			for (i = 0; i < m->nr; i++)
			  {
			    lr = m->d.complex[i].real;
			    li = m->d.complex[i].imag;
			    rr = r->d.complex[i].real;
			    ri = r->d.complex[i].imag;
			    m->d.complex[i].real = lr * rr - li * ri;
			    m->d.complex[i].imag = lr * ri + li * rr;
			  }
		      }
		    else
		      {
			m = (MATRIX *) copy_matrix (EAT (r));
		      }
		  }
		else
		  {
		    m = (MATRIX *) copy_matrix (EAT (l));
		  }
		break;
	      }

	    /* Otherwise, fall through. */

	  case sparse:

	    /*
	     * Someday, we'll write code to really do these cases, but for
	     * now we'll just convert to full storage.
	     */

	    l = (MATRIX *) full_matrix (EAT (l));
	    r = (MATRIX *) full_matrix (EAT (r));
	    m = (MATRIX *) multiply_matrix_complex (EAT (l), EAT (r));

	    break;

	  default:
	    fail (bad_matrix, density_string[r->density]);
	    raise_exception ();
	  }
	break;

      default:
	fail (bad_matrix, density_string[l->density]);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    TFREE (tmp);
    TFREE (ja_tmp);
    TFREE (a_tmp);
    delete_2_entities (rid, cid);
    delete_3_matrices (l, r, m);
  }
  END_EXCEPTION;

  delete_2_entities (m->rid, m->cid);
  m->rid = rid;
  m->cid = cid;

  delete_2_matrices (l, r);
  return (ENT (m));
}

ENTITY *
multiply_vector_integer (VECTOR * volatile l, VECTOR * volatile r)
{
  int i;
  SCALAR *s = NULL;

  EASSERT (l, vector, integer);
  EASSERT (r, vector, integer);

  if (l->ne != r->ne)
    {
      fail ("Inconsistent dimensions for vector multiply.  Left vector has %d element%s, but right vector has %d element%s.\n",
	    l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
      delete_2_vectors (l, r);
      raise_exception ();
    }

  if (!MATCH_VECTORS (l->eid, r->eid))
    {
      fail (bad_labels);
      delete_2_vectors (l, r);
      raise_exception ();
    }

  l = (VECTOR *) dense_vector (l);
  r = (VECTOR *) dense_vector (r);

  s = (SCALAR *) int_to_scalar (0);

  WITH_HANDLING
  {
    for (i = 0; i < l->ne; i++)
      s->v.integer += l->a.integer[i] * r->a.integer[i];
  }
  ON_EXCEPTION
  {
    fail ("Exception during vector multiplication.\n");
    delete_2_vectors (l, r);
    delete_scalar (s);
  }
  END_EXCEPTION;

  delete_2_vectors (l, r);
  return (ENT (s));
}

ENTITY *
multiply_vector_real (VECTOR * volatile l, VECTOR * volatile r)
{
  int i;
  SCALAR *s = NULL;

  EASSERT (l, vector, real);
  EASSERT (r, vector, real);

  if (l->ne != r->ne)
    {
      fail ("Inconsistent dimensions for vector multiply.  Left vector has %d element%s, but right vector has %d element%s.\n",
	    l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
      delete_2_vectors (l, r);
      raise_exception ();
    }

  if (!MATCH_VECTORS (l->eid, r->eid))
    {
      fail (bad_labels);
      delete_2_vectors (l, r);
      raise_exception ();
    }

  l = (VECTOR *) dense_vector (l);
  r = (VECTOR *) dense_vector (r);

  s = (SCALAR *) real_to_scalar (0.0);

  WITH_HANDLING
  {
    for (i = 0; i < l->ne; i++)
      s->v.real += l->a.real[i] * r->a.real[i];
  }
  ON_EXCEPTION
  {
    fail ("Exception during vector multiplication.\n");
    delete_2_vectors (l, r);
    delete_scalar (s);
  }
  END_EXCEPTION;

  delete_2_vectors (l, r);
  return (ENT (s));
}

ENTITY *
multiply_vector_complex (VECTOR * volatile l, VECTOR * volatile r)
{
  int i;
  SCALAR *s = NULL;

  EASSERT (l, vector, complex);
  EASSERT (r, vector, complex);

  if (l->ne != r->ne)
    {
      fail ("Inconsistent dimensions for vector multiply.  Left vector has %d element%s, but right vector has %d element%s.\n",
	    l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
      delete_2_vectors (l, r);
      raise_exception ();
    }

  if (!MATCH_VECTORS (l->eid, r->eid))
    {
      fail (bad_labels);
      delete_2_vectors (l, r);
      raise_exception ();
    }

  l = (VECTOR *) dense_vector (l);
  r = (VECTOR *) dense_vector (r);

  s = (SCALAR *) cast_scalar_integer_complex ((SCALAR *) int_to_scalar (0));

  WITH_HANDLING
  {
    for (i = 0; i < l->ne; i++)
      {
	s->v.complex.real +=
	  l->a.complex[i].real * r->a.complex[i].real -
	  l->a.complex[i].imag * r->a.complex[i].imag;
	s->v.complex.imag +=
	  l->a.complex[i].real * r->a.complex[i].imag +
	  l->a.complex[i].imag * r->a.complex[i].real;
      }
  }
  ON_EXCEPTION
  {
    fail ("Exception during vector multiplication.\n");
    delete_2_vectors (l, r);
    delete_scalar (s);
  }
  END_EXCEPTION;

  delete_2_vectors (l, r);
  return (ENT (s));
}

ENTITY *
multiply_vector_matrix_integer (VECTOR *l, MATRIX *r)
{
  MATRIX *m;

  EASSERT (l, vector, integer);
  EASSERT (r, matrix, integer);

  m = (MATRIX *) vector_to_matrix (l);
  return (matrix_to_vector ((MATRIX *) multiply_matrix_integer (m, r)));
}

ENTITY *
multiply_vector_matrix_real (VECTOR *l, MATRIX *r)
{
  MATRIX *m;

  EASSERT (l, vector, real);
  EASSERT (r, matrix, real);

  m = (MATRIX *) vector_to_matrix (l);
  return (matrix_to_vector ((MATRIX *) multiply_matrix_real (m, r)));
}

ENTITY *
multiply_vector_matrix_complex (VECTOR *l, MATRIX *r)
{
  MATRIX *m;

  EASSERT (l, vector, complex);
  EASSERT (r, matrix, complex);

  m = (MATRIX *) vector_to_matrix (l);
  return (matrix_to_vector ((MATRIX *) multiply_matrix_complex (m, r)));
}

ENTITY *
multiply_matrix_vector_integer (MATRIX *l, VECTOR *r)
{
  MATRIX *m;

  EASSERT (l, matrix, integer);
  EASSERT (r, vector, integer);

  m = (MATRIX *) transpose_matrix ((MATRIX *) vector_to_matrix (r));
  return (matrix_to_vector ((MATRIX *) multiply_matrix_integer (l, m)));
}

ENTITY *
multiply_matrix_vector_real (MATRIX *l, VECTOR *r)
{
  MATRIX *m;

  EASSERT (l, matrix, real);
  EASSERT (r, vector, real);

  m = (MATRIX *) transpose_matrix ((MATRIX *) vector_to_matrix (r));
  return (matrix_to_vector ((MATRIX *) multiply_matrix_real (l, m)));
}

ENTITY *
multiply_matrix_vector_complex (MATRIX *l, VECTOR *r)
{
  MATRIX *m;

  EASSERT (l, matrix, complex);
  EASSERT (r, vector, complex);

  m = (MATRIX *) transpose_matrix ((MATRIX *) vector_to_matrix (r));
  return (matrix_to_vector ((MATRIX *) multiply_matrix_complex (l, m)));
}
