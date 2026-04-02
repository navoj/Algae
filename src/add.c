/*                               -*- Mode: C -*- 
 * add.c -- Addition.
 * 
 * Copyright (C) 1994-2003  K. Scott Hunziker
 * Copyright (C) 1990-1994  The Boeing Company
 * 
 * See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: add.c,v 1.5 2003/08/01 04:57:46 ksh Exp $";

#include "add.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "dense.h"
#include "transpose.h"
#include "sparse.h"
#include "full.h"
#include "thin.h"

static char bad_labels[] = "Labels don't match for addition.";
static char incon_vector[] = "Inconsistent dimensions for vector addition.  First vector has %d element%s; second vector has %d element%s.";
static char incon_matrix[] = "Inconsistent dimensions for matrix addition.  First matrix has %d row%s and %d column%s; second matrix has %d row%s and %d column%s.";

ENTITY *
add_scalar_vector_integer (SCALAR *l, VECTOR *r)
{
  VECTOR *v = NULL;
  int i;

  EASSERT (l, scalar, integer);
  EASSERT (r, vector, integer);

  if (l->v.integer == 0)
    {
      delete_scalar (l);
      return (ENT (r));
    }

  WITH_HANDLING
  {
    v = (VECTOR *) dense_vector ((VECTOR *) EAT (r));
    v = (VECTOR *) dup_vector ((VECTOR *) EAT (v));
    for (i = 0; i < v->nn; i++)
      {
	v->a.integer[i] += l->v.integer;
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (ENT (l), ENT (r), ENT (v));
  }
  END_EXCEPTION;

  delete_scalar (l);
  return (ENT (v));
}

ENTITY *
add_scalar_vector_real (SCALAR *l, VECTOR *r)
{
  VECTOR *v = NULL;
  int i;

  EASSERT (l, scalar, real);
  EASSERT (r, vector, real);

  if (l->v.real == 0.0)
    {
      delete_scalar (l);
      return (ENT (r));
    }

  WITH_HANDLING
  {
    v = (VECTOR *) dense_vector ((VECTOR *) EAT (r));
    v = (VECTOR *) dup_vector ((VECTOR *) EAT (v));
    for (i = 0; i < v->nn; i++)
      {
	v->a.real[i] += l->v.real;
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (ENT (l), ENT (r), ENT (v));
  }
  END_EXCEPTION;

  delete_scalar (l);
  return (ENT (v));
}

ENTITY *
add_scalar_vector_complex (SCALAR *l, VECTOR *r)
{
  VECTOR *v = NULL;
  int i;

  EASSERT (l, scalar, complex);
  EASSERT (r, vector, complex);

  if (l->v.complex.real == 0.0 && l->v.complex.imag == 0.0)
    {
      delete_scalar (l);
      return (ENT (r));
    }

  WITH_HANDLING
  {
    v = (VECTOR *) dense_vector ((VECTOR *) EAT (r));
    v = (VECTOR *) dup_vector ((VECTOR *) EAT (v));
    for (i = 0; i < v->nn; i++)
      {
	v->a.complex[i].real += l->v.complex.real;
	v->a.complex[i].imag += l->v.complex.imag;
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (ENT (l), ENT (r), ENT (v));
  }
  END_EXCEPTION;

  delete_scalar (l);
  return (ENT (v));
}

ENTITY *
add_scalar_vector_character (SCALAR *l, VECTOR *r)
{
  VECTOR *v;
  int i;

  EASSERT (l, scalar, character);
  EASSERT (r, vector, character);

  if (!*l->v.character)
    {
      delete_scalar (l);
      return (ENT (r));
    }

  r = (VECTOR *) dense_vector (r);
  v = (VECTOR *) form_vector (r->ne, character, dense);

  for (i = 0; i < v->nn; i++)
    {
      if (!*r->a.character[i])
	{
	  v->a.character[i] = dup_char (l->v.character);
	}
      else
	{
	  v->a.character[i] =
	    (char *) MALLOC (strlen (l->v.character) +
			     strlen (r->a.character[i]) + 1);
	  strcat (strcpy (v->a.character[i], l->v.character),
		  r->a.character[i]);
	}
    }

  if (r->eid) v->eid = copy_entity (r->eid);
  
  delete_scalar (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
add_vector_scalar_character (VECTOR *l, SCALAR *r)
{
  VECTOR *v;
  int i;

  EASSERT (l, vector, character);
  EASSERT (r, scalar, character);

  if (!*r->v.character)
    {
      delete_scalar (r);
      return (ENT (l));
    }

  l = (VECTOR *) dense_vector (l);
  v = (VECTOR *) form_vector (l->ne, character, dense);

  for (i = 0; i < v->nn; i++)
    {
      if (!*l->a.character[i])
	{
	  v->a.character[i] = dup_char (r->v.character);
	}
      else
	{
	  v->a.character[i] =
	    (char *) MALLOC (strlen (r->v.character) +
			     strlen (l->a.character[i]) + 1);
	  strcat (strcpy (v->a.character[i], l->a.character[i]),
		  r->v.character);
	}
    }

  if (l->eid) v->eid = copy_entity (l->eid);
  
  delete_scalar (r);
  delete_vector (l);
  return (ENT (v));
}

ENTITY *
add_vector_integer (VECTOR *l, VECTOR *r)
{
  ENTITY * volatile eid = NULL;
  VECTOR * volatile s = NULL;
  int i;
  void *x_tmp;
  int l_ia[2], r_ia[2], s_ia[2];

  EASSERT (l, vector, integer);
  EASSERT (r, vector, integer);

  WITH_HANDLING
  {
    if (l->ne != r->ne)
      {
	fail (incon_vector, l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->eid, r->eid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    if (l->eid != NULL)
      {
	eid = copy_entity (l->eid);
      }
    else if (r->eid != NULL)
      {
	eid = copy_entity (r->eid);
      }

    switch (l->density)
      {
      case dense:
	switch (r->density)
	  {
	  case dense:
	    s = (VECTOR *) dup_vector ((VECTOR *) EAT (l));
	    for (i = 0; i < s->nn; i++)
	      s->a.integer[i] += r->a.integer[i];
	    break;
	  case sparse:
	    /*
	     * This is an inefficient shortcut.  We should
	     * eventually write code to do this directly.
	     */
	    s = (VECTOR *) add_vector_integer (
						(VECTOR *) EAT (l),
			      (VECTOR *) dense_vector ((VECTOR *) EAT (r)));
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      case sparse:
	if (l->nn == 0)
	  {
	    s = (VECTOR *) copy_vector (r);
	    break;
	  }
	switch (r->density)
	  {
	  case dense:
	    s = (VECTOR *) add_vector_integer (
			       (VECTOR *) dense_vector ((VECTOR *) EAT (l)),
						(VECTOR *) EAT (r));
	    break;

	  case sparse:

	    if (r->nn == 0)
	      {
		s = (VECTOR *) copy_vector (l);
		break;
	      }

	    s = add_symbolic_vector ((VECTOR *) copy_vector (l),
				     (VECTOR *) copy_vector (r));

	    l_ia[0] = r_ia[0] = s_ia[0] = 1;
	    l_ia[1] = l->nn + 1;
	    r_ia[1] = r->nn + 1;
	    s_ia[1] = s->nn + 1;
	    x_tmp = CALLOC (s->ne, sizeof (int));
	    IGSADD (l_ia, l->ja, l->a.integer,
		    r_ia, r->ja, r->a.integer,
		    s_ia, &s->ne,
		    s_ia, s->ja, s->a.integer,
		    x_tmp);
	    FREE (x_tmp);

	    break;

	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;

      default:
	BAD_DENSITY (l->density);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_entity (eid);
    delete_3_vectors (l, r, s);
  }
  END_EXCEPTION;

  delete_entity (s->eid);
  s->eid = eid;

  delete_2_vectors (l, r);
  return (ENT (s));
}

ENTITY *
add_vector_real (VECTOR *l, VECTOR *r)
{
  ENTITY * volatile eid = NULL;
  VECTOR * volatile s = NULL;
  int i;
  void *x_tmp;
  int l_ia[2], r_ia[2], s_ia[2];

  EASSERT (l, vector, real);
  EASSERT (r, vector, real);

  WITH_HANDLING
  {
    if (l->ne != r->ne)
      {
	fail (incon_vector, l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->eid, r->eid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    if (l->eid != NULL)
      {
	eid = copy_entity (l->eid);
      }
    else if (r->eid != NULL)
      {
	eid = copy_entity (r->eid);
      }

    switch (l->density)
      {
      case dense:
	switch (r->density)
	  {
	  case dense:
	    s = (VECTOR *) dup_vector ((VECTOR *) EAT (l));
	    for (i = 0; i < s->nn; i++)
	      s->a.real[i] += r->a.real[i];
	    break;
	  case sparse:
	    /*
	     * This is an inefficient shortcut.  We should
	     * eventually write code to do this directly.
	     */
	    r = (VECTOR *) dense_vector ((VECTOR *) EAT (r));
	    s = (VECTOR *) add_vector_real ((VECTOR *) EAT (l),
					    (VECTOR *) EAT (r));
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      case sparse:
	if (l->nn == 0)
	  {
	    s = (VECTOR *) copy_vector (r);
	    break;
	  }
	switch (r->density)
	  {
	  case dense:
	    s = (VECTOR *) add_vector_real (
			       (VECTOR *) dense_vector ((VECTOR *) EAT (l)),
					     (VECTOR *) EAT (r));
	    break;

	  case sparse:

	    if (r->nn == 0)
	      {
		s = (VECTOR *) copy_vector (l);
		break;
	      }

	    s = add_symbolic_vector ((VECTOR *) copy_vector (l),
				     (VECTOR *) copy_vector (r));

	    l_ia[0] = r_ia[0] = s_ia[0] = 1;
	    l_ia[1] = l->nn + 1;
	    r_ia[1] = r->nn + 1;
	    s_ia[1] = s->nn + 1;
	    x_tmp = CALLOC (s->ne, sizeof (REAL));
	    DGSADD (l_ia, l->ja, l->a.real,
		    r_ia, r->ja, r->a.real,
		    s_ia, &s->ne,
		    s_ia, s->ja, s->a.real,
		    x_tmp);
	    FREE (x_tmp);

	    break;

	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;

      default:
	BAD_DENSITY (l->density);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_entity (eid);
    delete_3_vectors (l, r, s);
  }
  END_EXCEPTION;

  delete_entity (s->eid);
  s->eid = eid;

  delete_2_vectors (l, r);
  return (ENT (s));
}

ENTITY *
add_vector_complex (VECTOR *l, VECTOR *r)
{
  ENTITY * volatile eid = NULL;
  VECTOR * volatile s = NULL;
  int i;
  void *x_tmp;
  int l_ia[2], r_ia[2], s_ia[2];

  EASSERT (l, vector, complex);
  EASSERT (r, vector, complex);

  WITH_HANDLING
  {
    if (l->ne != r->ne)
      {
	fail (incon_vector, l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->eid, r->eid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    if (l->eid != NULL)
      {
	eid = copy_entity (l->eid);
      }
    else if (r->eid != NULL)
      {
	eid = copy_entity (r->eid);
      }

    switch (l->density)
      {
      case dense:
	switch (r->density)
	  {
	  case dense:
	    s = (VECTOR *) dup_vector ((VECTOR *) EAT (l));
	    for (i = 0; i < s->nn; i++)
	      {
		s->a.complex[i].real += r->a.complex[i].real;
		s->a.complex[i].imag += r->a.complex[i].imag;
	      }
	    break;
	  case sparse:
	    /*
	     * This is an inefficient shortcut.  We should
	     * eventually write code to do this directly.
	     */
	    s = (VECTOR *) add_vector_complex (
						(VECTOR *) EAT (l),
			      (VECTOR *) dense_vector ((VECTOR *) EAT (r)));
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      case sparse:
	if (l->nn == 0)
	  {
	    s = (VECTOR *) copy_vector (r);
	    break;
	  }
	switch (r->density)
	  {
	  case dense:
	    s = (VECTOR *) add_vector_complex (
			       (VECTOR *) dense_vector ((VECTOR *) EAT (l)),
						(VECTOR *) EAT (r));
	    break;
	  case sparse:
	    if (r->nn == 0)
	      {
		s = (VECTOR *) copy_vector (l);
		break;
	      }

	    s = add_symbolic_vector ((VECTOR *) copy_vector (l),
				     (VECTOR *) copy_vector (r));

	    l_ia[0] = r_ia[0] = s_ia[0] = 1;
	    l_ia[1] = l->nn + 1;
	    r_ia[1] = r->nn + 1;
	    s_ia[1] = s->nn + 1;
	    x_tmp = CALLOC (s->ne, sizeof (COMPLEX));

	    ZGSADD (l_ia, l->ja, l->a.complex,
		    r_ia, r->ja, r->a.complex,
		    s_ia, &s->ne,
		    s_ia, s->ja, s->a.complex,
		    x_tmp);
	    FREE (x_tmp);

	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      default:
	BAD_DENSITY (l->density);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_entity (eid);
    delete_3_vectors (l, r, s);
  }
  END_EXCEPTION;

  delete_entity (s->eid);
  s->eid = eid;

  delete_2_vectors (l, r);
  return (ENT (s));
}

ENTITY *
add_vector_character (VECTOR *l, VECTOR *r)
{
  ENTITY *eid = NULL;
  VECTOR *v;
  int i;

  EASSERT (l, vector, character);
  EASSERT (r, vector, character);

  if (l->ne != r->ne)
    {
      fail (incon_vector, l->ne, PLURAL (l->ne), r->ne, PLURAL (r->ne));
      delete_2_vectors (l, r);
      raise_exception ();
    }

  if (!MATCH_VECTORS (l->eid, r->eid))
    {
      fail (bad_labels);
      delete_2_vectors (l, r);
      raise_exception ();
    }

  if (l->eid != NULL)
    {
      eid = copy_entity (l->eid);
    }
  else if (r->eid != NULL)
    {
      eid = copy_entity (r->eid);
    }

  l = (VECTOR *) dense_vector (l);
  r = (VECTOR *) dense_vector (r);
  v = (VECTOR *) form_vector (l->ne, character, dense);

  for (i = 0; i < v->nn; i++)
    {
      if (!*l->a.character[i] && !*r->a.character[i])
	{
	  v->a.character[i] = NULL_string;
	}
      else
	{
	  v->a.character[i] =
	    (char *) MALLOC (strlen (l->a.character[i]) +
			     strlen (r->a.character[i]) + 1);
	  strcat (strcpy (v->a.character[i], l->a.character[i]),
		  r->a.character[i]);
	}
    }

  delete_entity (v->eid);
  v->eid = eid;

  delete_2_vectors (l, r);
  return (ENT (v));
}

ENTITY *
add_scalar_matrix_integer (SCALAR *l, MATRIX *r)
{
  MATRIX *m = NULL;
  int i;

  EASSERT (l, scalar, integer);
  EASSERT (r, matrix, integer);

  if (l->v.integer == 0)
    {
      delete_scalar (l);
      return (ENT (r));
    }

  WITH_HANDLING
  {
    m = (MATRIX *) dense_matrix ((MATRIX *) EAT (r));
    m = (MATRIX *) dup_matrix ((MATRIX *) EAT (m));
    for (i = 0; i < m->nn; i++)
      {
	m->a.integer[i] += l->v.integer;
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (ENT (l), ENT (r), ENT (m));
  }
  END_EXCEPTION;

  delete_scalar (l);
  return (ENT (m));
}

ENTITY *
add_scalar_matrix_real (SCALAR *l, MATRIX *r)
{
  MATRIX *m = NULL;
  int i;

  EASSERT (l, scalar, real);
  EASSERT (r, matrix, real);

  if (l->v.real == 0.0)
    {
      delete_scalar (l);
      return (ENT (r));
    }

  WITH_HANDLING
  {
    m = (MATRIX *) dense_matrix ((MATRIX *) EAT (r));
    m = (MATRIX *) dup_matrix ((MATRIX *) EAT (m));
    for (i = 0; i < m->nn; i++)
      {
	m->a.real[i] += l->v.real;
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (ENT (l), ENT (r), ENT (m));
  }
  END_EXCEPTION;

  delete_scalar (l);
  return (ENT (m));
}

ENTITY *
add_scalar_matrix_complex (SCALAR *l, MATRIX *r)
{
  MATRIX *m = NULL;
  int i;

  EASSERT (l, scalar, complex);
  EASSERT (r, matrix, complex);

  if (l->v.complex.real == 0.0 && l->v.complex.imag == 0.0)
    {
      delete_scalar (l);
      return (ENT (r));
    }

  WITH_HANDLING
  {
    int dgnl = r->nn == 0 &&
      (r->symmetry == symmetric || r->symmetry == hermitian);

    m = (MATRIX *) dense_matrix ((MATRIX *) EAT (r));
    m = (MATRIX *) dup_matrix ((MATRIX *) EAT (m));

    /*
     * If it's hermitian, then it goes to symmetric if it's diagonal
     * or general otherwise.
     */

    if (m->symmetry == hermitian && l->v.complex.imag != 0.0)
      m->symmetry = dgnl ? symmetric : general;

    for (i = 0; i < m->nn; i++)
      {
	m->a.complex[i].real += l->v.complex.real;
	m->a.complex[i].imag += l->v.complex.imag;
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (ENT (l), ENT (r), ENT (m));
  }
  END_EXCEPTION;

  delete_scalar (l);
  return (ENT (m));
}

ENTITY *
add_scalar_matrix_character (SCALAR *l, MATRIX *r)
{
  MATRIX *m;
  int i;

  EASSERT (l, scalar, character);
  EASSERT (r, matrix, character);

  if (!*l->v.character)
    {
      delete_scalar (l);
      return (ENT (r));
    }

  r = (MATRIX *) dense_matrix (r);
  m = (MATRIX *) form_matrix (r->nr, r->nc, character, dense);

  for (i = 0; i < m->nn; i++)
    {
      if (!*r->a.character[i])
	{
	  m->a.character[i] = dup_char (l->v.character);
	}
      else
	{
	  m->a.character[i] =
	    (char *) MALLOC (strlen (l->v.character) +
			     strlen (r->a.character[i]) + 1);
	  strcat (strcpy (m->a.character[i], l->v.character),
		  r->a.character[i]);
	}
    }

  if (r->rid) m->rid = copy_entity (r->rid);
  if (r->cid) m->cid = copy_entity (r->cid);

  delete_scalar (l);
  delete_matrix (r);
  return (ENT (m));
}

ENTITY *
add_matrix_scalar_character (MATRIX *l, SCALAR *r)
{
  MATRIX *m;
  int i;

  EASSERT (l, matrix, character);
  EASSERT (r, scalar, character);

  if (!*r->v.character)
    {
      delete_scalar (r);
      return (ENT (l));
    }

  l = (MATRIX *) dense_matrix (l);
  m = (MATRIX *) form_matrix (l->nr, l->nc, character, dense);

  for (i = 0; i < m->nn; i++)
    {
      if (!*l->a.character[i])
	{
	  m->a.character[i] = dup_char (r->v.character);
	}
      else
	{
	  m->a.character[i] =
	    (char *) MALLOC (strlen (r->v.character) +
			     strlen (l->a.character[i]) + 1);
	  strcat (strcpy (m->a.character[i], l->a.character[i]),
		  r->v.character);
	}
    }

  if (l->rid) m->rid = copy_entity (l->rid);
  if (l->cid) m->cid = copy_entity (l->cid);

  delete_scalar (r);
  delete_matrix (l);
  return (ENT (m));
}

ENTITY *
add_vector_matrix_integer (VECTOR *l, MATRIX *r)
{
  return (add_matrix_integer ((MATRIX *) vector_to_matrix (l), r));
}

ENTITY *
add_matrix_vector_integer (MATRIX *l, VECTOR *r)
{
  return (add_matrix_integer (l, (MATRIX *) vector_to_matrix (r)));
}

ENTITY *
add_vector_matrix_real (VECTOR *l, MATRIX *r)
{
  return (add_matrix_real ((MATRIX *) vector_to_matrix (l), r));
}

ENTITY *
add_matrix_vector_real (MATRIX *l, VECTOR *r)
{
  return (add_matrix_real (l, (MATRIX *) vector_to_matrix (r)));
}

ENTITY *
add_vector_matrix_complex (VECTOR *l, MATRIX *r)
{
  return (add_matrix_complex ((MATRIX *) vector_to_matrix (l), r));
}

ENTITY *
add_matrix_vector_complex (MATRIX *l, VECTOR *r)
{
  return (add_matrix_complex (l, (MATRIX *) vector_to_matrix (r)));
}

ENTITY *
add_vector_matrix_character (VECTOR *l, MATRIX *r)
{
  return (add_matrix_character ((MATRIX *) vector_to_matrix (l), r));
}

ENTITY *
add_matrix_vector_character (MATRIX *l, VECTOR *r)
{
  return (add_matrix_character (l, (MATRIX *) vector_to_matrix (r)));
}

ENTITY *
add_matrix_integer (MATRIX *l, MATRIX *r)
{
  ENTITY * volatile rid = NULL, * volatile cid = NULL;
  MATRIX * volatile s = NULL;
  int i;
  void *x_tmp;

  EASSERT (l, matrix, integer);
  EASSERT (r, matrix, integer);

  WITH_HANDLING
  {
    if (l->nr != r->nr || l->nc != r->nc)
      {
	fail (incon_matrix, l->nr, PLURAL (l->nr), l->nc, PLURAL (l->nc),
	      r->nr, PLURAL (r->nr), r->nc, PLURAL (r->nc));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->rid, r->rid) ||
	!MATCH_VECTORS (l->cid, r->cid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    if (l->rid != NULL)
      {
	rid = copy_entity (l->rid);
      }
    else if (r->rid != NULL)
      {
	rid = copy_entity (r->rid);
      }
    if (l->cid != NULL)
      {
	cid = copy_entity (l->cid);
      }
    else if (r->cid != NULL)
      {
	cid = copy_entity (r->cid);
      }

    switch (l->density)
      {
      case dense:
	switch (r->density)
	  {
	  case dense:
	    s = (MATRIX *) dup_matrix ((MATRIX *) EAT (l));
	    if (s->symmetry != r->symmetry)
	      s->symmetry = general;
	    for (i = 0; i < s->nn; i++)
	      s->a.integer[i] += r->a.integer[i];
	    break;
	  case sparse:
	  case sparse_upper:
	    /*
	     * This is an inefficient shortcut.  We should eventually
	     * write code to do this directly.
	     */
	    s = (MATRIX *) add_matrix_integer ((MATRIX *) EAT (l),
			      (MATRIX *) dense_matrix ((MATRIX *) EAT (r)));
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      case sparse:
	if (l->nn == 0)
	  {
	    s = (MATRIX *) copy_entity (ENT (r));
	    break;
	  }
	switch (r->density)
	  {
	  case dense:
	    s = (MATRIX *) add_matrix_integer (
			       (MATRIX *) dense_matrix ((MATRIX *) EAT (l)),
						(MATRIX *) EAT (r));
	    break;

	  case sparse:

	    if (r->nn == 0)
	      {
		s = (MATRIX *) copy_entity (ENT (l));
		break;
	      }

	    s = add_symbolic_matrix ((MATRIX *) copy_matrix (l),
				     (MATRIX *) copy_matrix (r));

	    x_tmp = CALLOC (s->nc, sizeof (int));
	    IGSADD (l->ia, l->ja, l->a.integer,
		    r->ia, r->ja, r->a.integer,
		    &s->nr, &s->nc,
		    s->ia, s->ja, s->a.integer,
		    x_tmp);
	    FREE (x_tmp);

	    break;

	  case sparse_upper:

	    if (l->symmetry == r->symmetry)
	      {
		/*
		 * They have the same symmetry, so convert to the
		 * sparse_upper form.
		 */
		s = (MATRIX *) add_matrix_integer (
			      (MATRIX *) sparse_matrix ((MATRIX *) EAT (l)),
						    (MATRIX *) EAT (r));
	      }
	    else
	      {
		/* They have different symmetry, so convert to full. */
		s = (MATRIX *) add_matrix_integer ((MATRIX *) EAT (l),
			       (MATRIX *) full_matrix ((MATRIX *) EAT (r)));
	      }
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;

      case sparse_upper:

	switch (r->density)
	  {
	  case dense:
	    s = (MATRIX *) add_matrix_integer (
			       (MATRIX *) dense_matrix ((MATRIX *) EAT (l)),
						(MATRIX *) EAT (r));
	    break;
	  case sparse:
	    if (r->nn == 0)
	      {
		s = (MATRIX *) copy_entity (ENT (l));
		break;
	      }

	    if (l->symmetry == r->symmetry)
	      {
		/*
		 * They have the same symmetry, but are stored differently.
		 * Convert to the sparse_upper form.
		 */
		s = (MATRIX *) add_matrix_integer ((MATRIX *) EAT (l),
			     (MATRIX *) sparse_matrix ((MATRIX *) EAT (r)));
	      }
	    else
	      {
		/* They have different symmetry, so convert to full. */
		s = (MATRIX *) add_matrix_integer (
				(MATRIX *) full_matrix ((MATRIX *) EAT (l)),
						    (MATRIX *) EAT (r));
	      }
	    break;
	  case sparse_upper:

	    if (l->symmetry == r->symmetry)
	      {
		if (l->nn == 0)
		  {
		    if (l->d.integer == NULL)
		      {
			s = (MATRIX *) copy_entity (ENT (r));
			break;
		      }
		    s = (MATRIX *) dup_matrix ((MATRIX *) EAT (r));
		    if (s->d.integer == NULL)
		      s->d.integer = (int *) CALLOC (s->nr, sizeof (int));
		    for (i = 0; i < s->nr; i++)
		      s->d.integer[i] += l->d.integer[i];
		  }
		else if (r->nn == 0)
		  {
		    if (r->d.integer == NULL)
		      {
			s = (MATRIX *) copy_entity (ENT (l));
			break;
		      }
		    s = (MATRIX *) dup_matrix ((MATRIX *) EAT (l));
		    if (s->d.integer == NULL)
		      s->d.integer = (int *) CALLOC (s->nr, sizeof (int));
		    for (i = 0; i < s->nr; i++)
		      s->d.integer[i] += r->d.integer[i];
		  }
		else
		  {

		    s = add_symbolic_matrix ((MATRIX *) copy_matrix (l),
					     (MATRIX *) copy_matrix (r));

		    x_tmp = CALLOC (s->nc, sizeof (int));
		    IGSADD (l->ia, l->ja, l->a.integer,
			    r->ia, r->ja, r->a.integer,
			    &s->nr, &s->nc,
			    s->ia, s->ja, s->a.integer,
			    x_tmp);
		    FREE (x_tmp);

		    if (l->d.integer != NULL)
		      {
			s->d.integer = dup_mem (l->d.integer,
						s->nr * sizeof (int));
			if (r->d.integer != NULL)
			  {
			    for (i = 0; i < s->nr; i++)
			      s->d.integer[i] += r->d.integer[i];
			  }
		      }
		    else if (r->d.integer != NULL)
		      {
			s->d.integer = dup_mem (r->d.integer,
						s->nr * sizeof (int));
		      }
		  }
	      }
	    else
	      {

		/*
		 * They have different symmetry, so we'll change them
		 * to sparse.
		 */

		s = (MATRIX *) add_matrix_integer (
				(MATRIX *) full_matrix ((MATRIX *) EAT (l)),
			       (MATRIX *) full_matrix ((MATRIX *) EAT (r)));
	      }
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      default:
	BAD_DENSITY (l->density);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_2_entities (rid, cid);
    delete_3_matrices (l, r, s);
  }
  END_EXCEPTION;

  delete_2_entities (s->rid, s->cid);
  s->rid = rid;
  s->cid = cid;

  delete_2_matrices (l, r);
  return (ENT (s));
}

ENTITY *
add_matrix_real (MATRIX *l, MATRIX *r)
{
  ENTITY * volatile rid = NULL, * volatile cid = NULL;
  MATRIX * volatile s = NULL;
  int i;
  void *x_tmp;

  EASSERT (l, matrix, real);
  EASSERT (r, matrix, real);

  WITH_HANDLING
  {
    if (l->nr != r->nr || l->nc != r->nc)
      {
	fail (incon_matrix, l->nr, PLURAL (l->nr), l->nc, PLURAL (l->nc),
	      r->nr, PLURAL (r->nr), r->nc, PLURAL (r->nc));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->rid, r->rid) ||
	!MATCH_VECTORS (l->cid, r->cid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    if (l->rid != NULL)
      {
	rid = copy_entity (l->rid);
      }
    else if (r->rid != NULL)
      {
	rid = copy_entity (r->rid);
      }
    if (l->cid != NULL)
      {
	cid = copy_entity (l->cid);
      }
    else if (r->cid != NULL)
      {
	cid = copy_entity (r->cid);
      }

    switch (l->density)
      {
      case dense:
	switch (r->density)
	  {
	  case dense:
	    s = (MATRIX *) dup_matrix ((MATRIX *) EAT (l));
	    if (s->symmetry != r->symmetry)
	      s->symmetry = general;
	    for (i = 0; i < s->nn; i++)
	      s->a.real[i] += r->a.real[i];
	    break;
	  case sparse:
	  case sparse_upper:
	    /*
	     * This is an inefficient shortcut.  We should eventually
	     * write code to do this directly.
	     */
	    s = (MATRIX *) add_matrix_real ((MATRIX *) EAT (l),
			      (MATRIX *) dense_matrix ((MATRIX *) EAT (r)));
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      case sparse:
	if (l->nn == 0)
	  {
	    s = (MATRIX *) copy_entity (ENT (r));
	    break;
	  }
	switch (r->density)
	  {
	  case dense:
	    s = (MATRIX *) add_matrix_real (
			       (MATRIX *) dense_matrix ((MATRIX *) EAT (l)),
					     (MATRIX *) EAT (r));
	    break;
	  case sparse:

	    if (r->nn == 0)
	      {
		s = (MATRIX *) copy_entity (ENT (l));
		break;
	      }

	    s = add_symbolic_matrix ((MATRIX *) copy_matrix (l),
				     (MATRIX *) copy_matrix (r));

	    x_tmp = CALLOC (s->nc, sizeof (REAL));
	    DGSADD (l->ia, l->ja, l->a.real,
		    r->ia, r->ja, r->a.real,
		    &s->nr, &s->nc,
		    s->ia, s->ja, s->a.real,
		    x_tmp);
	    FREE (x_tmp);

	    if (l->symmetry != r->symmetry)
	      s->symmetry = general;

	    break;
	  case sparse_upper:
	    if (l->symmetry == r->symmetry)
	      {
		/*
		 * They have the same symmetry, so convert to the
		 * sparse_upper form.
		 */
		s = (MATRIX *) add_matrix_real (
			      (MATRIX *) sparse_matrix ((MATRIX *) EAT (l)),
						 (MATRIX *) EAT (r));
	      }
	    else
	      {
		/* They have different symmetry, so convert to full. */
		s = (MATRIX *) add_matrix_real ((MATRIX *) EAT (l),
			       (MATRIX *) full_matrix ((MATRIX *) EAT (r)));
	      }
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      case sparse_upper:
	switch (r->density)
	  {
	  case dense:
	    s = (MATRIX *) add_matrix_real (
			       (MATRIX *) dense_matrix ((MATRIX *) EAT (l)),
					     (MATRIX *) EAT (r));
	    break;
	  case sparse:
	    if (r->nn == 0)
	      {
		s = (MATRIX *) copy_entity (ENT (l));
		break;
	      }

	    if (l->symmetry == r->symmetry)
	      {
		/*
		 * They have the same symmetry, but are stored differently.
		 * Convert to the sparse_upper form.
		 */
		s = (MATRIX *) add_matrix_real ((MATRIX *) EAT (l),
			     (MATRIX *) sparse_matrix ((MATRIX *) EAT (r)));
	      }
	    else
	      {
		/* They have different symmetry, so convert to full. */
		s = (MATRIX *) add_matrix_real (
				(MATRIX *) full_matrix ((MATRIX *) EAT (l)),
						 (MATRIX *) EAT (r));
	      }
	    break;
	  case sparse_upper:
	    if (l->symmetry == r->symmetry)
	      {
		if (l->nn == 0)
		  {
		    if (l->d.real == NULL)
		      {
			s = (MATRIX *) copy_entity (ENT (r));
			break;
		      }
		    s = (MATRIX *) dup_matrix ((MATRIX *) EAT (r));
		    if (s->d.real == NULL)
		      s->d.real = (REAL *) CALLOC (s->nr, sizeof (REAL));
		    for (i = 0; i < s->nr; i++)
		      s->d.real[i] += l->d.real[i];
		  }
		else if (r->nn == 0)
		  {
		    if (r->d.real == NULL)
		      {
			s = (MATRIX *) copy_entity (ENT (l));
			break;
		      }
		    s = (MATRIX *) dup_matrix ((MATRIX *) EAT (l));
		    if (s->d.real == NULL)
		      s->d.real = (REAL *) CALLOC (s->nr, sizeof (REAL));
		    for (i = 0; i < s->nr; i++)
		      s->d.real[i] += r->d.real[i];
		  }
		else
		  {

		    s = add_symbolic_matrix ((MATRIX *) copy_matrix (l),
					     (MATRIX *) copy_matrix (r));

		    x_tmp = CALLOC (s->nc, sizeof (REAL));
		    DGSADD (l->ia, l->ja, l->a.real,
			    r->ia, r->ja, r->a.real,
			    &s->nr, &s->nc,
			    s->ia, s->ja, s->a.real,
			    x_tmp);
		    FREE (x_tmp);

		    if (l->d.real != NULL)
		      {
			s->d.real = dup_mem (l->d.real,
					     s->nr * sizeof (REAL));
			if (r->d.real != NULL)
			  {
			    for (i = 0; i < s->nr; i++)
			      s->d.real[i] += r->d.real[i];
			  }
		      }
		    else if (r->d.real != NULL)
		      {
			s->d.real = dup_mem (r->d.real,
					     s->nr * sizeof (REAL));
		      }
		  }
	      }
	    else
	      {

		/*
		 * They have different symmetry, so we'll change them
		 * to sparse.
		 */

		s = (MATRIX *) add_matrix_real (
				(MATRIX *) full_matrix ((MATRIX *) EAT (l)),
			       (MATRIX *) full_matrix ((MATRIX *) EAT (r)));
	      }
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      default:
	BAD_DENSITY (l->density);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_2_entities (rid, cid);
    delete_3_matrices (l, r, s);
  }
  END_EXCEPTION;

  delete_2_entities (s->rid, s->cid);
  s->rid = rid;
  s->cid = cid;

  delete_2_matrices (l, r);
  return (ENT (s));
}

ENTITY *
add_matrix_complex (MATRIX *l, MATRIX *r)
{
  ENTITY * volatile rid = NULL, * volatile cid = NULL;
  MATRIX * volatile s = NULL;
  int i;
  void *x_tmp;

  EASSERT (l, matrix, complex);
  EASSERT (r, matrix, complex);

  WITH_HANDLING
  {
    if (l->nr != r->nr || l->nc != r->nc)
      {
	fail (incon_matrix, l->nr, PLURAL (l->nr), l->nc, PLURAL (l->nc),
	      r->nr, PLURAL (r->nr), r->nc, PLURAL (r->nc));
	raise_exception ();
      }

    if (!MATCH_VECTORS (l->rid, r->rid) ||
	!MATCH_VECTORS (l->cid, r->cid))
      {
	fail (bad_labels);
	raise_exception ();
      }

    if (l->rid != NULL)
      {
	rid = copy_entity (l->rid);
      }
    else if (r->rid != NULL)
      {
	rid = copy_entity (r->rid);
      }
    if (l->cid != NULL)
      {
	cid = copy_entity (l->cid);
      }
    else if (r->cid != NULL)
      {
	cid = copy_entity (r->cid);
      }

    switch (l->density)
      {
      case dense:
	switch (r->density)
	  {
	  case dense:
	    s = (MATRIX *) dup_matrix ((MATRIX *) EAT (l));
	    if (s->symmetry != r->symmetry)
	      s->symmetry = general;
	    for (i = 0; i < s->nn; i++)
	      {
		s->a.complex[i].real += r->a.complex[i].real;
		s->a.complex[i].imag += r->a.complex[i].imag;
	      }
	    break;
	  case sparse:
	  case sparse_upper:
	    /*
	     * This is an inefficient shortcut.  We should eventually
	     * write code to do this directly.
	     */
	    s = (MATRIX *) add_matrix_complex ((MATRIX *) EAT (l),
			      (MATRIX *) dense_matrix ((MATRIX *) EAT (r)));
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      case sparse:
	if (l->nn == 0)
	  {
	    s = (MATRIX *) copy_entity (ENT (r));
	    break;
	  }
	switch (r->density)
	  {
	  case dense:
	    s = (MATRIX *) add_matrix_complex (
			       (MATRIX *) dense_matrix ((MATRIX *) EAT (l)),
						(MATRIX *) EAT (r));
	    break;
	  case sparse:

	    if (r->nn == 0)
	      {
		s = (MATRIX *) copy_entity (ENT (l));
		break;
	      }

	    s = add_symbolic_matrix ((MATRIX *) copy_matrix (l),
				     (MATRIX *) copy_matrix (r));

	    x_tmp = CALLOC (s->nc, sizeof (COMPLEX));
	    ZGSADD (l->ia, l->ja, l->a.complex,
		    r->ia, r->ja, r->a.complex,
		    &s->nr, &s->nc,
		    s->ia, s->ja, s->a.complex,
		    x_tmp);
	    FREE (x_tmp);

	    break;

	  case sparse_upper:
	    if (l->symmetry == r->symmetry)
	      {
		/*
		 * They have the same symmetry, so convert to the
		 * sparse_upper form.
		 */
		s = (MATRIX *) add_matrix_complex (
			      (MATRIX *) sparse_matrix ((MATRIX *) EAT (l)),
						    (MATRIX *) EAT (r));
	      }
	    else
	      {
		/* They have different symmetry, so convert to full. */
		s = (MATRIX *) add_matrix_complex ((MATRIX *) EAT (l),
			       (MATRIX *) full_matrix ((MATRIX *) EAT (r)));
	      }
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      case sparse_upper:
	switch (r->density)
	  {
	  case dense:
	    s = (MATRIX *) add_matrix_complex (
			       (MATRIX *) dense_matrix ((MATRIX *) EAT (l)),
						(MATRIX *) EAT (r));
	    break;
	  case sparse:
	    if (r->nn == 0)
	      {
		s = (MATRIX *) copy_entity (ENT (l));
		break;
	      }

	    if (l->symmetry == r->symmetry)
	      {
		/*
		 * They have the same symmetry, but are stored differently.
		 * Convert to the sparse_upper form.
		 */
		s = (MATRIX *) add_matrix_complex ((MATRIX *) EAT (l),
			     (MATRIX *) sparse_matrix ((MATRIX *) EAT (r)));
	      }
	    else
	      {
		/* They have different symmetry, so convert to full. */
		s = (MATRIX *) add_matrix_complex (
				(MATRIX *) full_matrix ((MATRIX *) EAT (l)),
						    (MATRIX *) EAT (r));
	      }
	    break;
	  case sparse_upper:

	    if (l->nn == 0 && r->nn == 0)
	      {
		s = (MATRIX *)
		  form_matrix (l->nr, l->nc, complex, sparse_upper);
		s->symmetry = (l->symmetry == r->symmetry) ?
		  l->symmetry : symmetric;
		if (l->d.complex)
		  {
		    s->d.complex = (COMPLEX *) E_MALLOC (l->nr, complex);
		    if (r->d.complex)
		      {
			for (i=0; i<l->nr; i++)
			  {
			    s->d.complex[i].real =
			      l->d.complex[i].real + r->d.complex[i].real;
			    s->d.complex[i].imag =
			      l->d.complex[i].imag + r->d.complex[i].imag;
			  }
		      }
		    else
		      {
			for (i=0; i<l->nr; i++)
			  {
			    s->d.complex[i].real = l->d.complex[i].real;
			    s->d.complex[i].imag = l->d.complex[i].imag;
			  }
		      }
		  }
		else
		  {
		    if (r->d.complex)
		      {
			s->d.complex = (COMPLEX *) E_MALLOC (l->nr, complex);
			for (i=0; i<l->nr; i++)
			  {
			    s->d.complex[i].real = r->d.complex[i].real;
			    s->d.complex[i].imag = r->d.complex[i].imag;
			  }
		      }
		  }
	      }
	    else if (l->symmetry == r->symmetry)
	      {
		if (l->nn == 0)
		  {
		    if (l->d.complex == NULL)
		      {
			s = (MATRIX *) copy_entity (ENT (r));
			break;
		      }
		    s = (MATRIX *) dup_matrix ((MATRIX *) EAT (r));
		    if (s->d.complex == NULL)
		      s->d.complex =
			(COMPLEX *) CALLOC (s->nr, sizeof (COMPLEX));
		    for (i = 0; i < s->nr; i++)
		      {
			s->d.complex[i].real += l->d.complex[i].real;
			s->d.complex[i].imag +=
			  l->d.complex[i].imag;
		      }
		  }
		else if (r->nn == 0)
		  {
		    if (r->d.complex == NULL)
		      {
			s = (MATRIX *) copy_entity (ENT (l));
			break;
		      }
		    s = (MATRIX *) dup_matrix ((MATRIX *) EAT (l));
		    if (s->d.complex == NULL)
		      s->d.complex =
			(COMPLEX *) CALLOC (s->nr, sizeof (COMPLEX));
		    for (i = 0; i < s->nr; i++)
		      {
			s->d.complex[i].real += r->d.complex[i].real;
			s->d.complex[i].imag +=
			  r->d.complex[i].imag;
		      }
		  }
		else
		  {

		    s = add_symbolic_matrix ((MATRIX *) copy_matrix (l),
					     (MATRIX *) copy_matrix (r));

		    x_tmp = CALLOC (s->nc, sizeof (COMPLEX));
		    ZGSADD (l->ia, l->ja, l->a.complex,
			    r->ia, r->ja, r->a.complex,
			    &s->nr, &s->nc,
			    s->ia, s->ja, s->a.complex,
			    x_tmp);
		    FREE (x_tmp);

		    if (l->d.complex != NULL)
		      {
			s->d.complex =
			  dup_mem (l->d.complex, s->nr * sizeof (COMPLEX));
			if (r->d.complex != NULL)
			  {
			    for (i = 0; i < s->nr; i++)
			      {
				s->d.complex[i].real +=
				  r->d.complex[i].real;
				s->d.complex[i].imag +=
				  r->d.complex[i].imag;
			      }
			  }
		      }
		    else if (r->d.complex != NULL)
		      {
			s->d.complex =
			  dup_mem (r->d.complex, s->nr * sizeof (COMPLEX));
		      }
		  }
	      }
	    else
	      {

		/*
		 * They have different symmetry, so we'll change them
		 * to sparse.
		 */

		s = (MATRIX *) add_matrix_complex (
				(MATRIX *) full_matrix ((MATRIX *) EAT (l)),
			       (MATRIX *) full_matrix ((MATRIX *) EAT (r)));
	      }
	    break;
	  default:
	    BAD_DENSITY (r->density);
	    raise_exception ();
	  }
	break;
      default:
	BAD_DENSITY (l->density);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_2_entities (rid, cid);
    delete_3_matrices (l, r, s);
  }
  END_EXCEPTION;

  delete_2_entities (s->rid, s->cid);
  s->rid = rid;
  s->cid = cid;

  delete_2_matrices (l, r);
  return (ENT (s));
}

ENTITY *
add_matrix_character (MATRIX *l, MATRIX *r)
{
  ENTITY *rid = NULL, *cid = NULL;
  MATRIX *s = NULL;
  int i;

  EASSERT (l, matrix, character);
  EASSERT (r, matrix, character);

  if (l->nr != r->nr || l->nc != r->nc)
    {
      fail (incon_matrix, l->nr, PLURAL (l->nr), l->nc, PLURAL (l->nc),
	    r->nr, PLURAL (r->nr), r->nc, PLURAL (r->nc));
      delete_2_matrices (l, r);
      raise_exception ();
    }

  if (!MATCH_VECTORS (l->rid, r->rid) ||
      !MATCH_VECTORS (l->cid, r->cid))
    {
      fail (bad_labels);
      delete_2_matrices (l, r);
      raise_exception ();
    }

  if (l->rid != NULL)
    {
      rid = copy_entity (l->rid);
    }
  else if (r->rid != NULL)
    {
      rid = copy_entity (r->rid);
    }
  if (l->cid != NULL)
    {
      cid = copy_entity (l->cid);
    }
  else if (r->cid != NULL)
    {
      cid = copy_entity (r->cid);
    }

  l = (MATRIX *) dense_matrix (l);
  r = (MATRIX *) dense_matrix (r);

  s = (MATRIX *) form_matrix (l->nr, l->nc, character, dense);
  s->symmetry = (l->symmetry == r->symmetry) ? l->symmetry : general;
  for (i = 0; i < s->nn; i++)
    {
      if (!*l->a.character[i] && !*r->a.character[i])
	{
	  s->a.character[i] = NULL_string;
	}
      else
	{
	  s->a.character[i] =
	    (char *) MALLOC (strlen (l->a.character[i]) +
			     strlen (r->a.character[i]) + 1);
	  strcat (strcpy (s->a.character[i], l->a.character[i]),
		  r->a.character[i]);
	}
    }

  delete_2_entities (s->rid, s->cid);
  s->rid = rid;
  s->cid = cid;

  delete_2_matrices (l, r);
  return (ENT (s));
}

VECTOR *
add_symbolic_vector (VECTOR *l, VECTOR *r)
{
  /*
   * This function determines the structure of the union of its two
   * vector arguments.  These arguments must both be sparse, and must
   * have the same type and size.  The result has uninitialized space
   * allocated for the values.  Labels are neither checked nor copied.
   */

  VECTOR *s;
  void *ja_tmp, *x_tmp;
  int l_ia[2], r_ia[2], s_ia[2];

  assert (l->density == sparse);
  assert (r->density == sparse);
  assert (l->type == r->type);
  assert (l->ne == r->ne);

  if (l->nn == 0)
    {
      delete_vector (l);
      return r;
    }
  else if (r->nn == 0)
    {
      delete_vector (r);
      return l;
    }

  s = (VECTOR *) make_vector (l->ne, l->type, sparse);

  /* Temporary space. */

  ja_tmp = CALLOC (l->nn + r->nn, sizeof (int));
  x_tmp = CALLOC (l->ne + 1, sizeof (int));

  l_ia[0] = r_ia[0] = 1;
  l_ia[1] = l->nn + 1;
  r_ia[1] = r->nn + 1;

  /* Symbolic addition, result unordered. */

  XGSADD (l_ia, l->ja, r_ia, r->ja,
	  l_ia, &l->ne, s_ia, ja_tmp, x_tmp);

  /* Two transposes to order it. */

  s->nn = s_ia[1] - 1;
  s->ja = (int *) CALLOC (s->nn, sizeof (int));
  memcpy (s->ja, ja_tmp, s->nn * sizeof (int));

  XGSTRN (s_ia, s->ja, s_ia, &s->ne,
	  x_tmp, ja_tmp);
  XGSTRN (x_tmp, ja_tmp, &s->ne, s_ia,
	  s_ia, s->ja);

  FREE (x_tmp);
  FREE (ja_tmp);

  /* Allocate space for values. */

  s->a.ptr = MALLOC (s->nn * type_size[l->type]);

  delete_2_vectors (l, r);
  return s;
}

MATRIX *
add_symbolic_matrix (MATRIX *l, MATRIX *r)
{
  /*
   * This function determines the structure of the union of its two
   * matrix arguments.  These arguments must both be either sparse
   * or sparse_upper, and must have the same type and size.  The
   * result has uninitialized space allocated for the values.
   * Labels are neither checked nor copied.
   */

  /*
   * The input matrices need not have identical symmetry, but no
   * conversion to full storage will be performed.  For example,
   * given two matrices in sparse_upper form, the result will also
   * be in sparse_upper form, even if one was symmetric and the
   * other hermitian.  Instead, you should convert them to sparse
   * form first.
   */

  MATRIX *s;
  void *ja_tmp, *x_tmp;

  assert (l->density == sparse || l->density == sparse_upper);
  assert (r->density == sparse || r->density == sparse_upper);
  assert (l->type == r->type);
  assert (l->nr == r->nr);
  assert (l->nc == r->nc);

  if (l->nn == 0)
    {
      delete_matrix (l);
      return r;
    }
  else if (r->nn == 0)
    {
      delete_matrix (r);
      return l;
    }

  s = (MATRIX *) make_matrix (l->nr, l->nc, l->type, l->density);
  s->symmetry = (l->symmetry == r->symmetry) ? l->symmetry : general;
  s->ia = (int *) CALLOC (l->nr + 1, sizeof (int));

  /* Temporary space. */

  ja_tmp = CALLOC (l->nn + r->nn, sizeof (int));
  x_tmp = CALLOC (l->nc + 1, sizeof (int));

  /* Symbolic addition, result unordered. */

  XGSADD (l->ia, l->ja, r->ia, r->ja,
	  &l->nr, &l->nc, s->ia, ja_tmp, x_tmp);

  s->nn = s->ia[s->nr] - 1;
  s->ja = (int *) CALLOC (s->nn, sizeof (int));
  memcpy (s->ja, ja_tmp, s->nn * sizeof (int));

  /* Transpose twice to order it. */

  XGSTRN (s->ia, s->ja, &s->nr, &s->nc, x_tmp, ja_tmp);
  XGSTRN (x_tmp, ja_tmp, &s->nc, &s->nr, s->ia, s->ja);

  FREE (x_tmp);
  FREE (ja_tmp);

  /* Allocate space for values. */

  s->a.ptr = MALLOC (s->nn * type_size[l->type]);

  delete_2_matrices (l, r);
  return s;
}
