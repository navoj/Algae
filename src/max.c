/*
   max.c -- Max and min functions.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: max.c,v 1.4 2003/12/06 04:43:58 ksh Exp $";

#include "max.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "dense.h"
#include "full.h"
#include "transpose.h"

ENTITY *
bi_max (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case vector:
      return ENT (max_vector ((VECTOR *) p));
    case matrix:
      return ENT (max_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
max_vector (VECTOR *v)
{
  /*
   * Return the number of the greatest element of `v'.  If several
   * elements share the honors, return the number of the first one.
   */

  int i;
  int elem = 1;

  EASSERT (v, vector, 0);

  if (v->ne == 0)
    {
      fail ("Vector has no elements.");
      delete_vector (v);
      raise_exception ();
    }

  if (v->density == dense)
    {
      switch (v->type)
	{
	case integer:
	  {
	    int tmp = v->a.integer[0];

	    for (i = 1; i < v->nn; i++)
	      {
		if (v->a.integer[i] > tmp)
		  {
		    tmp = v->a.integer[i];
		    elem = i + 1;
		  }
	      }
	  }
	  break;
	case real:
	  {
	    REAL tmp = v->a.real[0];

	    for (i = 1; i < v->nn; i++)
	      {
		if (v->a.real[i] > tmp)
		  {
		    tmp = v->a.real[i];
		    elem = i + 1;
		  }
	      }
	  }
	  break;
	case complex:
	  fail ("The \"max\" function does not apply to complex vectors.");
	  delete_vector (v);
	  raise_exception ();
	case character:
	  {
	    char *tmp = v->a.character[0];

	    for (i = 1; i < v->nn; i++)
	      {
		if (strcmp (v->a.character[i], tmp) > 0)
		  {
		    tmp = v->a.character[i];
		    elem = i + 1;
		  }
	      }
	  }
	  break;
	default:
	  BAD_TYPE (v->type);
	  delete_vector (v);
	  raise_exception ();
	}
    }
  else		/* for sparse vectors */
    {
      switch (v->type)
	{
	case integer:
	  {
	    int j, tmp;

	    if (v->nn)
	      {
		tmp = v->a.integer[0];
		elem = v->ja[0];

		/* check the values stored */

		for (i = 1; i < v->nn; i++)
		  {
		    if (v->a.integer[i] > tmp)
		      {
			tmp = v->a.integer[i];
			elem = v->ja[i];
		      }
		  }

		/* check the values not stored */

		if (tmp <= 0 && v->nn < v->ne)
		  {
		    for (i=0, j=1; i<v->nn; i++, j++)
		      if (j < v->ja[i]) break;
		    if (tmp < 0)
		      elem = j;
		    else
		      if (j < elem) elem = j;
		  }
	      }
	  }
	  break;
	case real:
	  {
	    int j;
	    REAL tmp;

	    if (v->nn)
	      {
		tmp = v->a.real[0];
		elem = v->ja[0];

		/* check the values stored */

		for (i = 1; i < v->nn; i++)
		  {
		    if (v->a.real[i] > tmp)
		      {
			tmp = v->a.real[i];
			elem = v->ja[i];
		      }
		  }

		/* check the values not stored */

		if (tmp <= 0.0 && v->nn < v->ne)
		  {
		    for (i=0, j=1; i<v->nn; i++, j++)
		      if (j < v->ja[i]) break;
		    if (tmp < 0.0)
		      elem = j;
		    else
		      if (j < elem) elem = j;
		  }
	      }
	  }
	  break;
	case complex:
	  fail ("The \"max\" function does not apply to complex vectors.");
	  delete_vector (v);
	  raise_exception ();
	case character:
	  {
	    int j;
	    char *tmp;

	    if (v->nn)
	      {
		tmp = v->a.character[0];
		elem = v->ja[0];

		/* check the values stored */

		for (i = 1; i < v->nn; i++)
		  {
		    if (strcmp (v->a.character[i], tmp) > 0)
		      {
			tmp = v->a.character[i];
			elem = v->ja[i];
		      }
		  }

		/* check the values not stored */

		if (!*tmp && v->nn < v->ne)
		  {
		    for (i=0, j=1; i<v->nn; i++, j++)
		      if (j < v->ja[i]) break;
		    if (j < elem) elem = j;
		  }
	      }
	  }
	  break;
	default:
	  BAD_TYPE (v->type);
	  delete_vector (v);
	  raise_exception ();
	}
    }

  delete_vector (v);
  return int_to_scalar (elem);
}

ENTITY *
max_matrix (MATRIX *m)
{
  /*
   * Return the row number of the greatest element in each column of
   * `m'.  If several elements share the honors, return the row number
   * of the first one.
   */

  int i, l, nr, nc, nn;
  VECTOR *r;
  int *rr;

  EASSERT (m, matrix, 0);

  nr = m->nr;
  nc = m->nc;
  nn = m->nn;

  if (nr == 0)
    {
      fail ("Matrix has no rows.");
      delete_matrix (m);
      raise_exception ();
    }

  if (m->density == dense)
    {
      /* Returning vector */
      r = (VECTOR *) form_vector (nc, integer, dense);
      rr = r->a.integer;

      if (m->cid) r->eid = copy_entity (m->cid);

      switch (m->type)
	{
	case integer:
	  {
	    int tmp;

	    for (l=0; l<nc; l++)
	      {
		tmp = m->a.integer[l*nr];
		rr[l] = 1;

		for (i = l*nr+1; i < (l+1)*nr; i++)
		  {
		    if (m->a.integer[i] > tmp)
		      {
			tmp = m->a.integer[i];
			rr[l] = i + 1 - l*nr;
		      }
		  }
	      }
	  }
	  break;
	case real:
	  {
	    REAL tmp;

	    for (l=0; l<nc; l++)
	      {
		tmp = m->a.real[l*nr];
		rr[l] = 1;

		for (i = l*nr+1; i < (l+1)*nr; i++)
		  {
		    if (m->a.real[i] > tmp)
		      {
			tmp = m->a.real[i];
			rr[l] = i + 1 - l*nr;
		      }
		  }
	      }
	  }
	  break;
	case complex:
	  fail ("The \"max\" function does not apply to complex matrices.");
	  delete_matrix (m);
	  raise_exception ();
	case character:
	  {
	    char *tmp;

	    for (l=0; l<nc; l++)
	      {
		tmp = m->a.character[l*nr];
		rr[l] = 1;

		for (i = l*nr+1; i < (l+1)*nr; i++)
		  {
		    if (strcmp (m->a.character[i], tmp) > 0)
		      {
			tmp = m->a.character[i];
			rr[l] = i + 1 - l*nr;
		      }
		  }
	      }
	  }
	  break;
	default:
	  BAD_TYPE (m->type);
	  delete_matrix (m);
	  raise_exception ();
	}
    }
  else		/* for sparse matrices */
    {
      m = (MATRIX *) transpose_matrix ((MATRIX *) full_matrix (m));

      nr = m->nr;
      nc = m->nc;
      nn = m->nn;

      /* Returning vector */
      r = (VECTOR *) form_vector (nr, integer, dense);
      rr = r->a.integer;

      if (m->rid) r->eid = copy_entity (m->rid);

      switch (m->type)
	{
	case integer:
	  {
	    int j, tmp;

	    if (!nn)
		for (l=0; l<nr; l++)
		  rr[l] = 1;
	    else
	      {
		for (l=0; l<nr; l++)
		  {
		    i = m->ia[l]-1;
		    if (m->ia[l+1]-1 > i)
		      {
			tmp = m->a.integer[i];
			rr[l] = m->ja[i];

			/* check the values stored */

			for (i = m->ia[l]; i < m->ia[l+1]-1; i++)
			  {
			    if (m->a.integer[i] > tmp)
			      {
				tmp = m->a.integer[i];
				rr[l] = m->ja[i];
			      }
			  }

			/* check the values not stored */

			if (tmp <= 0 && (m->ia[l+1]-m->ia[l]) < m->nc)
			  {
			    for (i=m->ia[l]-1, j=1; i<m->ia[l+1]-1; i++, j++)
			      if (j < m->ja[i]) break;
			    if (tmp < 0)
			      rr[l] = j;
			    else
			      if (j < rr[l]) rr[l] = j;
			  }
		      }
		    else
		      rr[l] = 1;
		  }
	      }
	  }
	  break;

	case real:
	  {
	    int j;
	    REAL tmp;

	    if (!nn)
		for (l=0; l<nr; l++)
		  rr[l] = 1;
	    else
	      {
		for (l=0; l<nr; l++)
		  {
		    i = m->ia[l]-1;
		    if (m->ia[l+1]-1 > i)
		      {
			tmp = m->a.real[i];
			rr[l] = m->ja[i];

			/* check the values stored */

			for (i = m->ia[l]; i < m->ia[l+1]-1; i++)
			  {
			    if (m->a.real[i] > tmp)
			      {
				tmp = m->a.real[i];
				rr[l] = m->ja[i];
			      }
			  }

			/* check the values not stored */

			if (tmp <= 0 && (m->ia[l+1]-m->ia[l]) < m->nc)
			  {
			    for (i=m->ia[l]-1, j=1; i<m->ia[l+1]-1; i++, j++)
			      if (j < m->ja[i]) break;
			    if (tmp < 0)
			      rr[l] = j;
			    else
			      if (j < rr[l]) rr[l] = j;
			  }
		      }
		    else
		      rr[l] = 1;
		  }
	      }
	  }
	  break;

	case complex:
	  fail ("The \"max\" function does not apply to complex matrices.");
	  delete_matrix (m);
	  raise_exception ();

	case character:
	  {
	    int j;
	    char *tmp;

	    if (!nn)
		for (l=0; l<nr; l++)
		  rr[l] = 1;
	    else
	      {
		for (l=0; l<nr; l++)
		  {
		    i = m->ia[l]-1;
		    if (m->ia[l+1]-1 > i)
		      {
			tmp = m->a.character[i];
			rr[l] = m->ja[i];

			/* check the values stored */

			for (i = m->ia[l]; i < m->ia[l+1]-1; i++)
			  {
			    if (strcmp (m->a.character[i], tmp) > 0)
			      {
				tmp = m->a.character[i];
				rr[l] = m->ja[i];
			      }
			  }

			/* check the values not stored */

			if (!*tmp && (m->ia[l+1]-m->ia[l]) < m->nc)
			  {
			    for (i=m->ia[l]-1, j=1; i<m->ia[l+1]-1; i++, j++)
			      if (j < m->ja[i]) break;
			    if (j < rr[l]) rr[l] = j;
			  }
		      }
		    else
		      rr[l] = 1;
		  }
	      }
	  }
	  break;

	default:
	  BAD_TYPE (m->type);
	  delete_matrix (m);
	  raise_exception ();
	}
    }

  delete_matrix (m);
  return ENT (r);
}

ENTITY *
bi_min (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case vector:
      return ENT (min_vector ((VECTOR *) p));
    case matrix:
      return ENT (min_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
min_vector (VECTOR *v)
{
  /*
   * Return the number of the least element of `v'.  If several
   * elements share the honors, return the number of the last one.
   */

  int i;
  int elem = 1;

  EASSERT (v, vector, 0);

  if (v->ne == 0)
    {
      fail ("Vector has no elements.");
      delete_vector (v);
      raise_exception ();
    }

  if (v->density == dense)
    {
      switch (v->type)
	{
	case integer:
	  {
	    int tmp = v->a.integer[0];

	    for (i = 1; i < v->nn; i++)
	      {
		if (v->a.integer[i] <= tmp)
		  {
		    tmp = v->a.integer[i];
		    elem = i + 1;
		  }
	      }
	  }
	  break;
	case real:
	  {
	    REAL tmp = v->a.real[0];

	    for (i = 1; i < v->nn; i++)
	      {
		if (v->a.real[i] <= tmp)
		  {
		    tmp = v->a.real[i];
		    elem = i + 1;
		  }
	      }
	  }
	  break;
	case complex:
	  fail ("The \"min\" function does not apply to complex vectors.");
	  delete_vector (v);
	  raise_exception ();
	case character:
	  {
	    char *tmp = v->a.character[0];

	    for (i = 1; i < v->nn; i++)
	      {
		if (strcmp (v->a.character[i], tmp) <= 0)
		  {
		    tmp = v->a.character[i];
		    elem = i + 1;
		  }
	      }
	  }
	  break;
	default:
	  BAD_TYPE (v->type);
	  delete_vector (v);
	  raise_exception ();
	}
    }
  else		/* for sparse vectors */
    {
      elem = v->ne;

      switch (v->type)
	{
	case integer:
	  {
	    int j, tmp;

	    if (v->nn)
	      {
		tmp = v->a.integer[0];
		elem = v->ja[0];

		/* check the values stored */

		for (i = 1; i < v->nn; i++)
		  {
		    if (v->a.integer[i] <= tmp)
		      {
			tmp = v->a.integer[i];
			elem = v->ja[i];
		      }
		  }

		/* check the values not stored */

		if (tmp >= 0 && v->nn < v->ne)
		  {
		    for (i=v->nn-1, j=v->ne; i>=0; i--, j--)
		      if (j > v->ja[i]) break;
		    if (tmp > 0)
		      elem = j;
		    else
		      if (j > elem) elem = j;
		  }
	      }
	  }
	  break;
	case real:
	  {
	    int j;
	    REAL tmp;

	    if (v->nn)
	      {
		tmp = v->a.real[0];
		elem = v->ja[0];

		/* check the values stored */

		for (i = 1; i < v->nn; i++)
		  {
		    if (v->a.real[i] <= tmp)
		      {
			tmp = v->a.real[i];
			elem = v->ja[i];
		      }
		  }

		/* check the values not stored */

		if (tmp >= 0.0 && v->nn < v->ne)
		  {
		    for (i=v->nn-1, j=v->ne; i>=0; i--, j--)
		      if (j > v->ja[i]) break;
		    if (tmp > 0.0)
		      elem = j;
		    else
		      if (j > elem) elem = j;
		  }
	      }
	  }
	  break;
	case complex:
	  fail ("The \"min\" function does not apply to complex vectors.");
	  delete_vector (v);
	  raise_exception ();
	case character:
	  {
	    int j;
	    char *tmp;

	    if (v->nn)
	      {
		tmp = v->a.character[0];
		elem = v->ja[0];

		/* check the values stored */

		for (i = 1; i < v->nn; i++)
		  {
		    if (strcmp (v->a.character[i], tmp) <= 0)
		      {
			tmp = v->a.character[i];
			elem = v->ja[i];
		      }
		  }

		/* check the values not stored */

		if (v->nn < v->ne)
		  {
		    for (i=v->nn-1, j=v->ne; i>=0; i--, j--)
		      if (j > v->ja[i]) break;
		    if (*tmp)
		      elem = j;
		    else
		      if (j > elem) elem = j;
		  }
	      }
	  }
	  break;
	default:
	  BAD_TYPE (v->type);
	  delete_vector (v);
	  raise_exception ();
	}
    }

  delete_vector (v);
  return int_to_scalar (elem);
}

ENTITY *
min_matrix (MATRIX *m)
{
  /*
   * Return the row number of the least element in each column of
   * `m'.  If several elements share the honors, return the row number
   * of the last one.
   */

  int i, l, nr, nc, nn;
  VECTOR *r;
  int *rr;

  EASSERT (m, matrix, 0);

  nr = m->nr;
  nc = m->nc;
  nn = m->nn;

  if (nr == 0)
    {
      fail ("Matrix has no rows.");
      delete_matrix (m);
      raise_exception ();
    }

  if (m->density == dense)
    {
      /* Returning vector */
      r = (VECTOR *) form_vector (nc, integer, dense);
      rr = r->a.integer;

      if (m->cid) r->eid = copy_entity (m->cid);

      switch (m->type)
	{
	case integer:
	  {
	    int tmp;

	    for (l=0; l<nc; l++)
	      {
		tmp = m->a.integer[l*nr];
		rr[l] = 1;

		for (i = l*nr+1; i < (l+1)*nr; i++)
		  {
		    if (m->a.integer[i] <= tmp)
		      {
			tmp = m->a.integer[i];
			rr[l] = i + 1 - l*nr;
		      }
		  }
	      }
	  }
	  break;
	case real:
	  {
	    REAL tmp;

	    for (l=0; l<nc; l++)
	      {
		tmp = m->a.real[l*nr];
		rr[l] = 1;

		for (i = l*nr+1; i < (l+1)*nr; i++)
		  {
		    if (m->a.real[i] <= tmp)
		      {
			tmp = m->a.real[i];
			rr[l] = i + 1 - l*nr;
		      }
		  }
	      }
	  }
	  break;
	case complex:
	  fail ("The \"min\" function does not apply to complex matrices.");
	  delete_matrix (m);
	  raise_exception ();
	case character:
	  {
	    char *tmp;

	    for (l=0; l<nc; l++)
	      {
		tmp = m->a.character[l*nr];
		rr[l] = 1;

		for (i = l*nr+1; i < (l+1)*nr; i++)
		  {
		    if (strcmp (m->a.character[i], tmp) <= 0)
		      {
			tmp = m->a.character[i];
			rr[l] = i + 1 - l*nr;
		      }
		  }
	      }
	  }
	  break;
	default:
	  BAD_TYPE (m->type);
	  delete_matrix (m);
	  raise_exception ();
	}
    }
  else		/* for sparse matrices */
    {
      m = (MATRIX *) transpose_matrix ((MATRIX *) full_matrix (m));

      nr = m->nr;
      nc = m->nc;
      nn = m->nn;

      /* Returning vector */
      r = (VECTOR *) form_vector (nr, integer, dense);
      rr = r->a.integer;

      if (m->rid) r->eid = copy_entity (m->rid);

      switch (m->type)
	{
	case integer:
	  {
	    int j, tmp;

	    if (!nn)
		for (l=0; l<nr; l++)
		  rr[l] = nc;
	    else
	      {
		for (l=0; l<nr; l++)
		  {
		    i = m->ia[l]-1;
		    if (m->ia[l+1]-1 > i)
		      {
			tmp = m->a.integer[i];
			rr[l] = m->ja[i];

			/* check the values stored */

			for (i = m->ia[l]; i < m->ia[l+1]-1; i++)
			  {
			    if (m->a.integer[i] <= tmp)
			      {
				tmp = m->a.integer[i];
				rr[l] = m->ja[i];
			      }
			  }

			/* check the values not stored */

			if (tmp >= 0 && (m->ia[l+1]-m->ia[l]) < m->nc)
			  {
			    for (i=m->ia[l+1]-2, j=nc; i>=m->ia[l]-1; i--, j--)
			      if (j > m->ja[i]) break;
			    if (tmp > 0)
			      rr[l] = j;
			    else
			      if (j > rr[l]) rr[l] = j;
			  }
		      }
		    else
		      rr[l] = nc;
		  }
	      }
	  }
	  break;

	case real:
	  {
	    int j;
	    REAL tmp;

	    if (!nn)
		for (l=0; l<nr; l++)
		  rr[l] = nc;
	    else
	      {
		for (l=0; l<nr; l++)
		  {
		    i = m->ia[l]-1;
		    if (m->ia[l+1]-1 > i)
		      {
			tmp = m->a.real[i];
			rr[l] = m->ja[i];

			/* check the values stored */

			for (i = m->ia[l]; i < m->ia[l+1]-1; i++)
			  {
			    if (m->a.real[i] <= tmp)
			      {
				tmp = m->a.real[i];
				rr[l] = m->ja[i];
			      }
			  }

			/* check the values not stored */

			if (tmp >= 0 && (m->ia[l+1]-m->ia[l]) < m->nc)
			  {
			    for (i=m->ia[l+1]-2, j=nc; i>=m->ia[l]-1; i--, j--)
			      if (j > m->ja[i]) break;
			    if (tmp > 0)
			      rr[l] = j;
			    else
			      if (j > rr[l]) rr[l] = j;
			  }
		      }
		    else
		      rr[l] = nc;
		  }
	      }
	  }
	  break;

	case complex:
	  fail ("The \"min\" function does not apply to complex matrices.");
	  delete_matrix (m);
	  raise_exception ();

	case character:
	  {
	    int j;
	    char *tmp;

	    if (!nn)
		for (l=0; l<nr; l++)
		  rr[l] = nc;
	    else
	      {
		for (l=0; l<nr; l++)
		  {
		    i = m->ia[l]-1;
		    if (m->ia[l+1]-1 > i)
		      {
			tmp = m->a.character[i];
			rr[l] = m->ja[i];

			/* check the values stored */

			for (i = m->ia[l]; i < m->ia[l+1]-1; i++)
			  {
			    if (strcmp (m->a.character[i], tmp) <= 0)
			      {
				tmp = m->a.character[i];
				rr[l] = m->ja[i];
			      }
			  }

			/* check the values not stored */

			if ((m->ia[l+1]-m->ia[l]) < m->nc)
			  {
			    for (i=m->ia[l+1]-2, j=nc; i>=m->ia[l]-1; i--, j--)
			      if (j > m->ja[i]) break;
			    if (*tmp)
			      rr[l] = j;
			    else
			      if (j > rr[l]) rr[l] = j;
			  }
		      }
		    else
		      rr[l] = nc;
		  }
	      }
	  }
	  break;

	default:
	  BAD_TYPE (m->type);
	  delete_matrix (m);
	  raise_exception ();
	}
    }

  delete_matrix (m);
  return ENT (r);
}
