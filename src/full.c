/*
   full.c -- Convert to full storage.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: full.c,v 1.3 2003/08/01 04:57:47 ksh Exp $";

#include "full.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "transpose.h"
#include "negate.h"

ENTITY *
bi_full (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
    case vector:
      return (p);
    case matrix:
      return (full_matrix ((MATRIX *) p));
    default:
      fail ("Can't apply \"full\" to a %s entity.", class_string[p->class]);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
full_matrix (MATRIX *p)
{
  MATRIX * volatile m = NULL;
  MATRIX *t = NULL;
  size_t size;
  int i, j, k;

  EASSERT (p, matrix, 0);

  WITH_HANDLING
  {

    switch (p->density)
      {
      case dense:
      case sparse:
	m = (MATRIX *) EAT (p);
	break;
      case sparse_upper:
	switch (p->symmetry)
	  {
	  case general:
	    break;
	  case symmetric:
	    t = (MATRIX *) dup_matrix ((MATRIX *) copy_matrix (p));
	    t->density = sparse;
	    t->symmetry = general;
	    TFREE (t->d.ptr);
	    t = (MATRIX *) transpose_matrix ((MATRIX *) EAT (t));
	    break;
	  case hermitian:
	    t = (MATRIX *) dup_matrix ((MATRIX *) copy_matrix (p));
	    t->density = sparse;
	    t->symmetry = general;
	    TFREE (t->d.ptr);
	    t = (MATRIX *) hermitian_transpose_matrix ((MATRIX *) EAT (t));
	    break;
	  default:
	    BAD_SYMMETRY (p->symmetry);
	    raise_exception ();
	  }

	m = (MATRIX *) make_matrix (p->nr, p->nc, p->type, sparse);
	m->symmetry = p->symmetry;
	if (p->rid != NULL)
	  m->rid = copy_entity (p->rid);
	if (p->cid != NULL)
	  m->cid = copy_entity (p->cid);

	size = type_size[p->type];

	if (p->d.ptr == NULL)
	  {
	    if (t == NULL || t->nn == 0)
	      {
		m->nn = p->nn;
		if (m->nn > 0)
		  {
		    m->ia = (int *) dup_mem (p->ia,
					     (m->nr + 1) * sizeof (int));
		    m->ja = (int *) dup_mem (p->ja,
					     m->nn * sizeof (int));
		    m->a.ptr = dup_mem (p->a.ptr,
					m->nn * type_size[m->type]);
		    if (p->type == character)
		      for (i = 0; i < p->nn; i++)
			m->a.character[i] =
			  dup_char (m->a.character[i]);
		  }
	      }
	    else
	      {
		m->ia = (int *) MALLOC ((m->nr + 1) * sizeof (int));
		m->nn = p->nn + t->nn;
		m->ja = (int *) MALLOC (m->nn * sizeof (int));
		m->a.ptr = E_MALLOC (m->nn, m->type);

		m->ia[0] = 1;
		for (i = 1; i <= m->nr; i++)
		  {
		    POLL_SIGINT ();
		    j = t->ia[i] - t->ia[i - 1];
		    if (j > 0)
		      {
			memcpy (m->ja + m->ia[i - 1] - 1,
				t->ja + t->ia[i - 1] - 1, j * sizeof (int));
			memcpy ((char *) m->a.ptr +
				(m->ia[i - 1] - 1) * size,
				(char *) t->a.ptr +
				(t->ia[i - 1] - 1) * size,
				j * size);
		      }
		    k = p->ia[i] - p->ia[i - 1];
		    if (k > 0)
		      {
			memcpy (m->ja + m->ia[i - 1] + j - 1,
				p->ja + p->ia[i - 1] - 1, k * sizeof (int));
			memcpy ((char *) m->a.ptr +
				(m->ia[i - 1] + j - 1) * size,
				(char *) p->a.ptr +
				(p->ia[i - 1] - 1) * size,
				k * size);
		      }
		    m->ia[i] = m->ia[i - 1] + j + k;
		  }
		if (m->type == character)
		  for (i = 0; i < m->nn; i++)
		    m->a.character[i] = dup_char (m->a.character[i]);
	      }
	  }
	else
	  {
	    if (t == NULL || t->nn == 0)
	      {
		m->nn = p->nn + p->nr;
		m->ia = (int *) MALLOC ((m->nr + 1) * sizeof (int));
		m->ja = (int *) MALLOC (m->nn * sizeof (int));
		m->a.ptr = (char *) E_MALLOC (m->nn, m->type);

		m->ia[0] = 1;
		if (p->nn == 0)
		  {
		    for (i = 1; i <= m->nr; i++)
		      {
			m->ja[m->ia[i - 1] - 1] = i;
			memcpy ((char *) m->a.ptr +
				(m->ia[i - 1] - 1) * size,
				(char *) p->d.ptr + (i - 1) * size,
				size);
			m->ia[i] = m->ia[i - 1] + 1;
		      }
		  }
		else
		  {
		    for (i = 1; i <= m->nr; i++)
		      {
			POLL_SIGINT ();
			m->ja[m->ia[i - 1] - 1] = i;
			memcpy ((char *) m->a.ptr +
				(m->ia[i - 1] - 1) * size,
				(char *) p->d.ptr + (i - 1) * size,
				size);
			k = p->ia[i] - p->ia[i - 1];
			if (k > 0)
			  {
			    memcpy (m->ja + m->ia[i - 1],
				p->ja + p->ia[i - 1] - 1, k * sizeof (int));
			    memcpy ((char *) m->a.ptr +
				    (m->ia[i - 1]) * size,
				    (char *) p->a.ptr +
				    (p->ia[i - 1] - 1) * size,
				    k * size);
			  }
			m->ia[i] = m->ia[i - 1] + k + 1;
		      }
		  }
		if (m->type == character)
		  for (i = 0; i < m->nn; i++)
		    m->a.character[i] = dup_char (m->a.character[i]);
	      }
	    else
	      {
		m->nn = p->nn + t->nn + p->nr;
		m->ia = (int *) MALLOC ((m->nr + 1) * sizeof (int));
		m->ja = (int *) MALLOC (m->nn * sizeof (int));
		m->a.ptr = E_MALLOC (m->nn, m->type);

		m->ia[0] = 1;
		assert (p->nn > 0);
		for (i = 1; i <= m->nr; i++)
		  {
		    POLL_SIGINT ();
		    j = t->ia[i] - t->ia[i - 1];
		    if (j > 0)
		      {
			memcpy (m->ja + m->ia[i - 1] - 1,
				t->ja + t->ia[i - 1] - 1, j * sizeof (int));
			memcpy ((char *) m->a.ptr +
				size * (m->ia[i - 1] - 1),
				(char *) t->a.ptr +
				size * (t->ia[i - 1] - 1), j * size);
		      }
		    m->ja[m->ia[i - 1] + j - 1] = i;
		    memcpy ((char *) m->a.ptr +
			    (m->ia[i - 1] + j - 1) * size,
			    (char *) p->d.ptr + (i - 1) * size,
			    size);
		    k = p->ia[i] - p->ia[i - 1];
		    if (k > 0)
		      {
			memcpy (m->ja + m->ia[i - 1] + j,
				p->ja + p->ia[i - 1] - 1, k * sizeof (int));
			memcpy ((char *) m->a.ptr + size * (m->ia[i - 1] + j),
			      (char *) p->a.ptr + size * (p->ia[i - 1] - 1),
				k * size);
		      }
		    m->ia[i] = m->ia[i - 1] + j + k + 1;
		  }
		if (m->type == character)
		  for (i = 0; i < m->nn; i++)
		    m->a.character[i] = dup_char (m->a.character[i]);
	      }
	  }
	break;
      default:
	BAD_DENSITY (p->density);
	raise_exception ();
      }

  }
  ON_EXCEPTION
  {
    delete_3_matrices (p, m, t);
  }
  END_EXCEPTION;

  delete_2_matrices (p, t);
  return (ENT (m));
}
