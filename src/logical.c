/*
   logical.c -- The logical operators.

   Copyright (C) 1994-2002  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: logical.c,v 1.3 2002/07/24 04:35:00 ksh Exp $";

#include "logical.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "dense.h"
#include "transpose.h"
#include "add.h"
#include "not.h"
#include "full.h"
#include "sparse.h"
#include "two_type.h"

static char bad_labels[] = "Labels don't match.";
static char incon_vector[] = "Inconsistent dimensions for vector comparison.  First vector has %d element%s; second vector has %d element%s.";
static char incon_matrix[] = "Inconsistent dimensions for matrix comparison.  First matrix has %d row%s and %d column%s; second matrix has %d row%s and %d column%s.";

int
eq_integer (int l, int r)
{
  return (l == r);
}

int
eq_real (REAL l, REAL r)
{
  return (l == r);
}

int
eq_complex (COMPLEX l, COMPLEX r)
{
  return (l.real == r.real && l.imag == r.imag);
}

int
eq_character (char *l, char *r)
{
  return (!strcmp (l, r));
}

int
ne_integer (int l, int r)
{
  return (l != r);
}

int
ne_real (REAL l, REAL r)
{
  return (l != r);
}

int
ne_complex (COMPLEX l, COMPLEX r)
{
  return (l.real != r.real || l.imag != r.imag);
}

int
ne_character (char *l, char *r)
{
  return (!!strcmp (l, r));
}

int
lt_integer (int l, int r)
{
  return (l < r);
}

int
lt_real (REAL l, REAL r)
{
  return (l < r);
}

int
lt_character (char *l, char *r)
{
  return (strcmp (l, r) < 0);
}

int
lte_integer (int l, int r)
{
  return (l <= r);
}

int
lte_real (REAL l, REAL r)
{
  return (l <= r);
}

int
lte_character (char *l, char *r)
{
  return (strcmp (l, r) <= 0);
}

int
gt_integer (int l, int r)
{
  return (l > r);
}

int
gt_real (REAL l, REAL r)
{
  return (l > r);
}

int
gt_character (char *l, char *r)
{
  return (strcmp (l, r) > 0);
}

int
gte_integer (int l, int r)
{
  return (l >= r);
}

int
gte_real (REAL l, REAL r)
{
  return (l >= r);
}

int
gte_character (char *l, char *r)
{
  return (strcmp (l, r) >= 0);
}

int
and_integer (int l, int r)
{
  return (l && r);
}

int
and_real (REAL l, REAL r)
{
  return (l && r);
}

int
and_complex (COMPLEX l, COMPLEX r)
{
  return ((l.real || l.imag) && (r.real || r.imag));
}

int
and_character (char *l, char *r)
{
  return (*l && *r);
}

int
or_integer (int l, int r)
{
  return (l || r);
}

int
or_real (REAL l, REAL r)
{
  return (l || r);
}

int
or_complex (COMPLEX l, COMPLEX r)
{
  return (l.real || r.real || l.imag || r.imag);
}

int
or_character (char *l, char *r)
{
  return (*l || *r);
}

ENTITY *
logical_scalar_vector_integer (SCALAR *l, VECTOR *r, int PROTO ((*func), (int, int)))
{
  int i;

  EASSERT (l, scalar, integer);
  EASSERT (r, vector, integer);

  if ((*func) (l->v.integer, 0))
    r = (VECTOR *) dense_vector (r);
  r = (VECTOR *) dup_vector (r);

  for (i = 0; i < r->nn; i++)
    r->a.integer[i] = (*func) (l->v.integer, r->a.integer[i]);

  delete_scalar (l);
  return (ENT (r));
}

ENTITY *
logical_scalar_vector_real (SCALAR *l, VECTOR *r, int PROTO ((*func), (REAL, REAL)))
{
  VECTOR *v;
  int i;

  EASSERT (l, scalar, real);
  EASSERT (r, vector, real);

  if ((*func) (l->v.real, 0.0))
    r = (VECTOR *) dense_vector (r);

  if (r->density == dense)
    {
      v = (VECTOR *) form_vector (r->ne, integer, dense);
      if (r->eid)
	v->eid = copy_entity (r->eid);
    }
  else
    {
      v = (VECTOR *) notnot_vector ((VECTOR *) copy_vector (r));
    }

  for (i = 0; i < v->nn; i++)
    v->a.integer[i] = (*func) (l->v.real, r->a.real[i]);

  delete_scalar (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
logical_scalar_vector_complex (SCALAR *l, VECTOR *r, int PROTO ((*func), (COMPLEX, COMPLEX)))
{
  VECTOR *v;
  int i;
  COMPLEX c;

  c.real = c.imag = 0.0;

  EASSERT (l, scalar, complex);
  EASSERT (r, vector, complex);

  if ((*func) (l->v.complex, c))
    r = (VECTOR *) dense_vector (r);

  if (r->density == dense)
    {
      v = (VECTOR *) form_vector (r->ne, integer, dense);
      if (r->eid)
	v->eid = copy_entity (r->eid);
    }
  else
    {
      v = (VECTOR *) notnot_vector ((VECTOR *) copy_vector (r));
    }

  for (i = 0; i < v->nn; i++)
    v->a.integer[i] = (*func) (l->v.complex, r->a.complex[i]);

  delete_scalar (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
logical_scalar_vector_character (SCALAR *l, VECTOR *r, int PROTO ((*func), (char *, char *)))
{
  VECTOR *v;
  int i;
  char *c = "";

  EASSERT (l, scalar, character);
  EASSERT (r, vector, character);

  if ((*func) (l->v.character, c))
    r = (VECTOR *) dense_vector (r);

  if (r->density == dense)
    {
      v = (VECTOR *) form_vector (r->ne, integer, dense);
      if (r->eid)
	v->eid = copy_entity (r->eid);
    }
  else
    {
      v = (VECTOR *) notnot_vector ((VECTOR *) copy_vector (r));
    }

  for (i = 0; i < v->nn; i++)
    v->a.integer[i] = (*func) (l->v.character, r->a.character[i]);

  delete_scalar (l);
  delete_vector (r);
  return (ENT (v));
}

ENTITY *
logical_vector_integer (VECTOR *l, VECTOR *r, int PROTO ((*func), (int, int)))
{
  VECTOR *s;
  int i, j, k;

  EASSERT (l, vector, integer);
  EASSERT (r, vector, integer);

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

  if (l->density == dense || r->density == dense || (*func) (0, 0))
    {
      l = (VECTOR *) dense_vector (l);
      r = (VECTOR *) dense_vector (r);

      s = (VECTOR *) dup_vector (EAT (l));
      if (!s->eid && r->eid)
	s->eid = copy_entity (r->eid);

      for (i = 0; i < s->nn; i++)
	s->a.integer[i] = (*func) (s->a.integer[i], r->a.integer[i]);

    }
  else
    {

      int a, b;

      s = add_symbolic_vector ((VECTOR *) copy_vector (l),
			       (VECTOR *) copy_vector (r));
      s = (VECTOR *) dup_vector (s);

      if (l->eid)
	s->eid = copy_entity (l->eid);
      else if (r->eid)
	s->eid = copy_entity (r->eid);

      for (i = j = k = 0; k < s->nn; k++)
	{
	  a = (i >= l->nn || l->ja[i] > s->ja[k]) ? 0 : l->a.integer[i++];
	  b = (j >= r->nn || r->ja[j] > s->ja[k]) ? 0 : r->a.integer[j++];
	  s->a.integer[k] = (*func) (a, b);
	}
    }

  delete_2_vectors (l, r);
  return (ENT (s));
}

ENTITY *
logical_vector_real (VECTOR *l, VECTOR *r, int PROTO ((*func), (REAL, REAL)))
{
  VECTOR *s;
  int i, j, k;

  EASSERT (l, vector, real);
  EASSERT (r, vector, real);

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

  if (l->density == dense || r->density == dense || (*func) (0.0, 0.0))
    {
      l = (VECTOR *) dense_vector (l);
      r = (VECTOR *) dense_vector (r);

      s = (VECTOR *) form_vector (l->ne, integer, dense);

      if (l->eid)
	s->eid = copy_entity (l->eid);
      else if (r->eid)
	s->eid = copy_entity (r->eid);

      for (i = 0; i < s->nn; i++)
	s->a.integer[i] = (*func) (l->a.real[i], r->a.real[i]);

    }
  else
    {

      REAL a, b;

      s = add_symbolic_vector ((VECTOR *) copy_vector (l),
			       (VECTOR *) copy_vector (r));
      s = (VECTOR *) dup_vector (s);

      /* really, we want it integer */

      if (s->nn)
	s->a.integer = REALLOC (s->a.ptr, s->nn * sizeof (int));
      s->type = integer;

      if (l->eid)
	s->eid = copy_entity (l->eid);
      else if (r->eid)
	s->eid = copy_entity (r->eid);

      for (i = j = k = 0; k < s->nn; k++)
	{
	  a = (i >= l->nn || l->ja[i] > s->ja[k]) ? 0.0 : l->a.real[i++];
	  b = (j >= r->nn || r->ja[j] > s->ja[k]) ? 0.0 : r->a.real[j++];
	  s->a.integer[k] = (*func) (a, b);
	}
    }

  delete_2_vectors (l, r);
  return (ENT (s));
}

ENTITY *
logical_vector_complex (VECTOR *l, VECTOR *r, int PROTO ((*func), (COMPLEX, COMPLEX)))
{
  VECTOR *s;
  int i, j, k;
  COMPLEX z;

  z.real = 0.0;
  z.imag = 0.0;

  EASSERT (l, vector, complex);
  EASSERT (r, vector, complex);

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

  if (l->density == dense || r->density == dense || (*func) (z, z))
    {
      l = (VECTOR *) dense_vector (l);
      r = (VECTOR *) dense_vector (r);

      s = (VECTOR *) form_vector (l->ne, integer, dense);

      if (l->eid)
	s->eid = copy_entity (l->eid);
      else if (r->eid)
	s->eid = copy_entity (r->eid);

      for (i = 0; i < s->nn; i++)
	s->a.integer[i] = (*func) (l->a.complex[i], r->a.complex[i]);

    }
  else
    {

      COMPLEX a, b;

      s = add_symbolic_vector ((VECTOR *) copy_vector (l),
			       (VECTOR *) copy_vector (r));
      s = (VECTOR *) dup_vector (s);

      /* really, we want it integer */

      if (s->nn)
	s->a.integer = REALLOC (s->a.ptr, s->nn * sizeof (int));
      s->type = integer;

      if (l->eid)
	s->eid = copy_entity (l->eid);
      else if (r->eid)
	s->eid = copy_entity (r->eid);

      for (i = j = k = 0; k < s->nn; k++)
	{
	  a = (i >= l->nn || l->ja[i] > s->ja[k]) ? z : l->a.complex[i++];
	  b = (j >= r->nn || r->ja[j] > s->ja[k]) ? z : r->a.complex[j++];
	  s->a.integer[k] = (*func) (a, b);
	}
    }

  delete_2_vectors (l, r);
  return (ENT (s));
}

ENTITY *
logical_vector_character (VECTOR *l, VECTOR *r, int PROTO ((*func), (char *, char *)))
{
  VECTOR *v;
  int i, j, k;

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

  if (l->density == dense || r->density == dense ||
      (*func) (NULL_string, NULL_string))
    {
      l = (VECTOR *) dense_vector (l);
      r = (VECTOR *) dense_vector (r);

      v = (VECTOR *) form_vector (l->ne, integer, dense);

      if (l->eid)
	v->eid = copy_entity (l->eid);
      else if (r->eid)
	v->eid = copy_entity (r->eid);

      for (i = 0; i < v->nn; i++)
	v->a.integer[i] = (*func) (l->a.character[i], r->a.character[i]);

    }
  else
    {

      char *a, *b;

      v = add_symbolic_vector ((VECTOR *) copy_vector (l),
			       (VECTOR *) copy_vector (r));
      v = (VECTOR *) dup_vector (v);

      /* really, we want it integer */

      if (v->nn)
	v->a.integer = REALLOC (v->a.ptr, v->nn * sizeof (int));
      v->type = integer;

      if (l->eid)
	v->eid = copy_entity (l->eid);
      else if (r->eid)
	v->eid = copy_entity (r->eid);

      for (i = j = k = 0; k < v->nn; k++)
	{
	  a = (i >= l->nn || l->ja[i] > v->ja[k]) ?
	    NULL_string : l->a.character[i++];
	  b = (j >= r->nn || r->ja[j] > v->ja[k]) ?
	    NULL_string : r->a.character[j++];
	  v->a.integer[k] = (*func) (a, b);
	}
    }

  delete_2_vectors (l, r);
  return (ENT (v));
}

ENTITY *
logical_scalar_matrix_integer (SCALAR *l, MATRIX *r, int PROTO ((*func), (int, int)))
{
  int i;

  EASSERT (l, scalar, integer);
  EASSERT (r, matrix, integer);
  assert (r->symmetry == general || r->symmetry == symmetric);

  if ((*func) (l->v.integer, 0))
    r = (MATRIX *) dense_matrix (r);
  r = (MATRIX *) dup_matrix (r);

  for (i = 0; i < r->nn; i++)
    r->a.integer[i] = (*func) (l->v.integer, r->a.integer[i]);
  if (r->d.integer)
    for (i = 0; i < r->nr; i++)
      r->d.integer[i] = (*func) (l->v.integer, r->d.integer[i]);

  delete_scalar (l);

  return (ENT (r));
}

ENTITY *
logical_scalar_matrix_real (SCALAR *l, MATRIX *r, int PROTO ((*func), (REAL, REAL)))
{
  MATRIX *m;
  int i;

  EASSERT (l, scalar, real);
  EASSERT (r, matrix, real);
  assert (r->symmetry == general || r->symmetry == symmetric);

  if ((*func) (l->v.real, 0.0))
    r = (MATRIX *) dense_matrix (r);
  if (r->density == dense)
    {
      m = (MATRIX *) form_matrix (r->nr, r->nc, integer, dense);
      m->symmetry = r->symmetry;
      if (r->rid)
	m->rid = copy_entity (r->rid);
      if (r->cid)
	m->cid = copy_entity (r->cid);
    }
  else
    {
      m = (MATRIX *) notnot_matrix ((MATRIX *) copy_matrix (r));
    }

  for (i = 0; i < m->nn; i++)
    m->a.integer[i] = (*func) (l->v.real, r->a.real[i]);
  if (m->d.integer)
    for (i = 0; i < m->nr; i++)
      m->d.integer[i] = (*func) (l->v.real, r->d.real[i]);

  delete_scalar (l);
  delete_matrix (r);
  return (ENT (m));
}

ENTITY *
logical_scalar_matrix_complex (SCALAR *l, MATRIX *r, int PROTO ((*func), (COMPLEX, COMPLEX)))
{
  MATRIX *m;
  int i;
  COMPLEX c;

  c.real = c.imag = 0.0;

  EASSERT (l, scalar, complex);
  EASSERT (r, matrix, complex);

  if ((*func) (l->v.complex, c))
    r = (MATRIX *) dense_matrix (r);

  if (r->density == dense)
    {
      m = (MATRIX *) form_matrix (r->nr, r->nc, integer, dense);
      m->symmetry = (r->symmetry == general) ? general : symmetric;
      if (r->rid)
	m->rid = copy_entity (r->rid);
      if (r->cid)
	m->cid = copy_entity (r->cid);
    }
  else
    {
      m = (MATRIX *) notnot_matrix ((MATRIX *) copy_matrix (r));
    }

  for (i = 0; i < m->nn; i++)
    m->a.integer[i] = (*func) (l->v.complex, r->a.complex[i]);
  if (m->d.integer)
    for (i = 0; i < m->nr; i++)
      m->d.integer[i] = (*func) (l->v.complex, r->d.complex[i]);

  delete_scalar (l);
  delete_matrix (r);
  return (ENT (m));
}

ENTITY *
logical_scalar_matrix_character (SCALAR *l, MATRIX *r, int PROTO ((*func), (char *, char *)))
{
  MATRIX *m;
  int i;
  char *c = "";

  EASSERT (l, scalar, character);
  EASSERT (r, matrix, character);

  if ((*func) (l->v.character, c))
    r = (MATRIX *) dense_matrix (r);

  if (r->density == dense)
    {
      m = (MATRIX *) form_matrix (r->nr, r->nc, integer, dense);
      m->symmetry = r->symmetry;
      if (r->rid)
	m->rid = copy_entity (r->rid);
      if (r->cid)
	m->cid = copy_entity (r->cid);
    }
  else
    {
      m = (MATRIX *) notnot_matrix ((MATRIX *) copy_matrix (r));
    }

  for (i = 0; i < m->nn; i++)
    m->a.integer[i] = (*func) (l->v.character, r->a.character[i]);
  if (m->d.integer)
    for (i = 0; i < m->nr; i++)
      m->d.integer[i] = (*func) (l->v.character, r->d.character[i]);

  delete_scalar (l);
  delete_matrix (r);
  return (ENT (m));
}

ENTITY *
logical_vector_matrix_integer (VECTOR *l, MATRIX *r, int PROTO ((*func), (int, int)))
{
  return (logical_matrix_integer ((MATRIX *) vector_to_matrix (l),
				  r, (*func)));
}

ENTITY *
logical_matrix_vector_integer (MATRIX *l, VECTOR *r, int PROTO ((*func), (int, int)))
{
  return (logical_matrix_integer (l, (MATRIX *) vector_to_matrix (r),
				  (*func)));
}

ENTITY *
logical_vector_matrix_real (VECTOR *l, MATRIX *r, int PROTO ((*func), (REAL, REAL)))
{
  return (logical_matrix_real ((MATRIX *) vector_to_matrix (l),
			       r, (*func)));
}

ENTITY *
logical_matrix_vector_real (MATRIX *l, VECTOR *r, int PROTO ((*func), (REAL, REAL)))
{
  return (logical_matrix_real (l, (MATRIX *) vector_to_matrix (r),
			       (*func)));
}

ENTITY *
logical_vector_matrix_complex (VECTOR *l, MATRIX *r, int PROTO ((*func), (COMPLEX, COMPLEX)))
{
  return (logical_matrix_complex ((MATRIX *) vector_to_matrix (l),
				  r, (*func)));
}

ENTITY *
logical_matrix_vector_complex (MATRIX *l, VECTOR *r, int PROTO ((*func), (COMPLEX, COMPLEX)))
{
  return (logical_matrix_complex (l, (MATRIX *) vector_to_matrix (r),
				  (*func)));
}

ENTITY *
logical_vector_matrix_character (VECTOR *l, MATRIX *r, int PROTO ((*func), (char *, char *)))
{
  return (logical_matrix_character ((MATRIX *) vector_to_matrix (l), r, (*func)));
}

ENTITY *
logical_matrix_vector_character (MATRIX *l, VECTOR *r, int PROTO ((*func), (char *, char *)))
{
  return (logical_matrix_character (l, (MATRIX *) vector_to_matrix (r),
				    (*func)));
}

ENTITY *
logical_matrix_integer (MATRIX *l, MATRIX *r, int PROTO ((*func), (int, int)))
{
  MATRIX *s = NULL;
  int i, k;

  EASSERT (l, matrix, integer);
  EASSERT (r, matrix, integer);

  WITH_HANDLING
    {
      if (l->nr != r->nr || l->nc != r->nc)
	{
	  fail (incon_matrix,
		l->nr, PLURAL (l->nr), l->nc, PLURAL (l->nc),
		r->nr, PLURAL (r->nr), r->nc, PLURAL (r->nc));
	  raise_exception ();
	}

      if (!MATCH_VECTORS (l->rid, r->rid) ||
	  !MATCH_VECTORS (l->cid, r->cid))
	{
	  fail (bad_labels);
	  raise_exception ();
	}

      if (l->density == dense || r->density == dense || (*func) (0, 0))
	{
	  l = (MATRIX *) dense_matrix ((MATRIX *) EAT(l));
	  r = (MATRIX *) dense_matrix ((MATRIX *) EAT(r));

	  if (r->entity.ref_count == 1)
	    {
	      s = (MATRIX *) dup_matrix ((MATRIX *) EAT (r));
	      if (s->symmetry != l->symmetry) s->symmetry = general;

	      if (!s->rid && l->rid)
		s->rid = copy_entity (l->rid);
	      if (!s->cid && l->cid)
		s->cid = copy_entity (l->cid);

	      for (i = 0; i < s->nn; i++)
		s->a.integer[i] = (*func) (l->a.integer[i], s->a.integer[i]);
	    }
	  else
	    {
	      s = (MATRIX *) dup_matrix ((MATRIX *) EAT (l));
	      if (s->symmetry != r->symmetry) s->symmetry = general;

	      if (!s->rid && r->rid)
		s->rid = copy_entity (r->rid);
	      if (!s->cid && r->cid)
		s->cid = copy_entity (r->cid);

	      for (i = 0; i < s->nn; i++)
		s->a.integer[i] = (*func) (s->a.integer[i], r->a.integer[i]);
	    }
	}
      else
	{
	  int a, b;
	  int j, m;

	  assert (l->density == sparse || l->density == sparse_upper);
	  assert (r->density == sparse || r->density == sparse_upper);

	  if (l->density != r->density)
	    {
	      l = (MATRIX *) full_matrix ((MATRIX *) EAT (l));
	      r = (MATRIX *) full_matrix ((MATRIX *) EAT (r));
	    }

	  s = add_symbolic_matrix ((MATRIX *) copy_matrix (l),
				   (MATRIX *) copy_matrix (r));
	  s = (MATRIX *) dup_matrix (s);

	  s->rid = l->rid ? copy_entity (l->rid) :
	    r->rid ? copy_entity (r->rid) : NULL;
	  s->cid = l->cid ? copy_entity (l->cid) :
	    r->cid ? copy_entity (r->cid) : NULL;

	  if (l->nn)
	    {
	      if (r->nn)
		{
		  for (m=0; m<s->nr; m++)
		    {
		      for (i=l->ia[m]-1, j=r->ia[m]-1, k=s->ia[m]-1;
			   k<s->ia[m+1]-1; k++)
			{
			  a = (i>=l->ia[m+1]-1 || l->ja[i] > s->ja[k]) ?
			    0 : l->a.integer[i++];
			  b = (j>=r->ia[m+1]-1 || r->ja[j] > s->ja[k]) ?
			    0 : r->a.integer[j++];
			  s->a.integer[k] = (*func) (a, b);
			}
		    }
		}
	      else
		{
		  for (k=0; k<l->nn; k++)
		    s->a.integer[k] = (*func) (l->a.integer[k], 0);
		}
	    }
	  else
	    if (r->nn)
	      {
		for (k=0; k<r->nn; k++)
		  s->a.integer[k] = (*func) (0, r->a.integer[k]);
	      }

	  assert (s->density == l->density && s->density == r->density);
	  if (s->density == sparse_upper)
	    {
	      if (l->d.integer || r->d.integer)
		s->d.integer = CALLOC (s->nr, sizeof (int));

	      if (l->d.integer)
		{
		  if (r->d.integer)
		    {
		      for (k=0; k<s->nr; k++)
			s->d.integer[k] = (*func) (l->d.integer[k], r->d.integer[k]);
		    }
		  else
		    {	    
		      for (k=0; k<s->nr; k++)
			s->d.integer[k] = (*func) (l->d.integer[k], 0);
		    }
		}
	      else
		{
		  if (r->d.integer)
		    {
		      for (k=0; k<s->nr; k++)
			s->d.integer[k] = (*func) (0, r->d.integer[k]);
		    }
		}
	    }

	  s = (MATRIX *) sparse_matrix ((MATRIX *) EAT (s));
	  s = (MATRIX *) apt_matrix ((MATRIX *) EAT (s));
	}
    }
  ON_EXCEPTION
    {
      delete_3_matrices (l, r, s);
    }
  END_EXCEPTION;

  delete_2_matrices (l, r);
  return ENT (s);
}

ENTITY *
logical_matrix_real (MATRIX *l, MATRIX *r, int PROTO ((*func), (REAL, REAL)))
{
  MATRIX *s = NULL;
  int i, k;

  EASSERT (l, matrix, real);
  EASSERT (r, matrix, real);

  WITH_HANDLING
    {
      if (l->nr != r->nr || l->nc != r->nc)
	{
	  fail (incon_matrix,
		l->nr, PLURAL (l->nr), l->nc, PLURAL (l->nc),
		r->nr, PLURAL (r->nr), r->nc, PLURAL (r->nc));
	  raise_exception ();
	}

      if (!MATCH_VECTORS (l->rid, r->rid) ||
	  !MATCH_VECTORS (l->cid, r->cid))
	{
	  fail (bad_labels);
	  raise_exception ();
	}

      if (l->density == dense || r->density == dense || (*func) (0.0, 0.0))
	{
	  l = (MATRIX *) dense_matrix (l);
	  r = (MATRIX *) dense_matrix (r);

	  s = (MATRIX *) form_matrix (l->nr, l->nc, integer, dense);
	  if (l->symmetry != r->symmetry) s->symmetry = general;

	  if (l->rid)
	    s->rid = copy_entity (l->rid);
	  else if (r->rid)
	    s->rid = copy_entity (r->rid);
	  if (l->cid)
	    s->cid = copy_entity (l->cid);
	  else if (r->cid)
	    s->cid = copy_entity (r->cid);

	  for (i = 0; i < s->nn; i++)
	    s->a.integer[i] = (*func) (l->a.real[i], r->a.real[i]);
	}
      else
	{
	  REAL a, b;
	  int j, m;

	  assert (l->density == sparse || l->density == sparse_upper);
	  assert (r->density == sparse || r->density == sparse_upper);

	  if (l->density != r->density)
	    {
	      l = (MATRIX *) full_matrix ((MATRIX *) EAT (l));
	      r = (MATRIX *) full_matrix ((MATRIX *) EAT (r));
	    }

	  s = add_symbolic_matrix ((MATRIX *) copy_matrix (l),
				   (MATRIX *) copy_matrix (r));
	  s = (MATRIX *) dup_matrix (s);

	  /* really, we want it integer */

	  if (s->nn)
	    s->a.integer = REALLOC (s->a.ptr, s->nn * sizeof (int));
	  s->type = integer;

	  s->rid = l->rid ? copy_entity (l->rid) :
	    r->rid ? copy_entity (r->rid) : NULL;
	  s->cid = l->cid ? copy_entity (l->cid) :
	    r->cid ? copy_entity (r->cid) : NULL;

	  if (l->nn)
	    {
	      if (r->nn)
		{
		  for (m=0; m<s->nr; m++)
		    {
		      for (i=l->ia[m]-1, j=r->ia[m]-1, k=s->ia[m]-1;
			   k<s->ia[m+1]-1; k++)
			{
			  a = (i>=l->ia[m+1]-1 || l->ja[i] > s->ja[k]) ?
			    0.0 : l->a.real[i++];
			  b = (j>=r->ia[m+1]-1 || r->ja[j] > s->ja[k]) ?
			    0.0 : r->a.real[j++];
			  s->a.integer[k] = (*func) (a, b);
			}
		    }
		}
	      else
		{
		  for (k=0; k<l->nn; k++)
		    s->a.integer[k] = (*func) (l->a.real[k], 0.0);
		}
	    }
	  else
	    if (r->nn)
	      {
		for (k=0; k<r->nn; k++)
		  s->a.integer[k] = (*func) (0.0, r->a.real[k]);
	      }

	  assert (s->density == l->density && s->density == r->density);
	  if (s->density == sparse_upper)
	    {
	      if (l->d.real || r->d.real)
		s->d.integer = CALLOC (s->nr, sizeof (int));

	      if (l->d.real)
		{
		  if (r->d.real)
		    {
		      for (k=0; k<s->nr; k++)
			s->d.integer[k] = (*func) (l->d.real[k], r->d.real[k]);
		    }
		  else
		    {	    
		      for (k=0; k<s->nr; k++)
			s->d.integer[k] = (*func) (l->d.real[k], 0.0);
		    }
		}
	      else
		{
		  if (r->d.real)
		    {
		      for (k=0; k<s->nr; k++)
			s->d.integer[k] = (*func) (0.0, r->d.real[k]);
		    }
		}
	    }

	  s = (MATRIX *) sparse_matrix ((MATRIX *) EAT (s));
	  s = (MATRIX *) apt_matrix ((MATRIX *) EAT (s));
	}
    }
  ON_EXCEPTION
    {
      delete_3_matrices (l, r, s);
    }
  END_EXCEPTION;

  delete_2_matrices (l, r);
  return ENT (s);
}

ENTITY *
logical_matrix_complex (MATRIX *l, MATRIX *r, int PROTO ((*func), (COMPLEX, COMPLEX)))
{
  MATRIX *s = NULL;
  int i, k;
  COMPLEX x, y, z;

  EASSERT (l, matrix, complex);
  EASSERT (r, matrix, complex);

  WITH_HANDLING
    {
      if (l->nr != r->nr || l->nc != r->nc)
	{
	  fail (incon_matrix,
		l->nr, PLURAL (l->nr), l->nc, PLURAL (l->nc),
		r->nr, PLURAL (r->nr), r->nc, PLURAL (r->nc));
	  raise_exception ();
	}

      if (!MATCH_VECTORS (l->rid, r->rid) ||
	  !MATCH_VECTORS (l->cid, r->cid))
	{
	  fail (bad_labels);
	  raise_exception ();
	}

      x.real = x.imag = 0.0;

      if (l->density == dense || r->density == dense || (*func) (x, x))
	{
	  l = (MATRIX *) dense_matrix (l);
	  r = (MATRIX *) dense_matrix (r);

	  s = (MATRIX *) form_matrix (l->nr, l->nc, integer, dense);

	  switch (TWO_SYMMETRY (l->symmetry, r->symmetry))
	    {
	    case hermitian_hermitian:
	    case symmetric_symmetric:

	      s->symmetry = symmetric;
	      break;

	    case symmetric_hermitian:
	    case hermitian_symmetric:

	      /*
	       * The only possible operators here are &, |, ==, or !=.
	       * With the first two, the result is symmetric.
	       */

	      y.real = z.real = 0.0;
	      y.imag = 1.0;
	      z.imag = -1.0;

	      s->symmetry = ((*func)(y, y) && (*func)(y, z)) ?
		symmetric : general;
	      break;

	    default:

	      s->symmetry = general;
	    }

	  if (l->rid)
	    s->rid = copy_entity (l->rid);
	  else if (r->rid)
	    s->rid = copy_entity (r->rid);
	  if (l->cid)
	    s->cid = copy_entity (l->cid);
	  else if (r->cid)
	    s->cid = copy_entity (r->cid);

	  for (i = 0; i < s->nn; i++)
	    s->a.integer[i] = (*func) (l->a.complex[i], r->a.complex[i]);
	}
      else
	{
	  int j, m;

	  assert (l->density == sparse || l->density == sparse_upper);
	  assert (r->density == sparse || r->density == sparse_upper);

	  if (l->density != r->density || l->symmetry != r->symmetry)
	    {
	      l = (MATRIX *) full_matrix ((MATRIX *) EAT (l));
	      r = (MATRIX *) full_matrix ((MATRIX *) EAT (r));
	    }

	  s = add_symbolic_matrix ((MATRIX *) copy_matrix (l),
				   (MATRIX *) copy_matrix (r));
	  s = (MATRIX *) dup_matrix (s);

	  /* really, we want it integer */

	  if (s->nn)
	    s->a.integer = REALLOC (s->a.ptr, s->nn * sizeof (int));
	  s->type = integer;

	  switch (TWO_SYMMETRY (l->symmetry, r->symmetry))
	    {
	    case hermitian_hermitian:
	    case symmetric_symmetric:

	      s->symmetry = symmetric;
	      break;

	    case symmetric_hermitian:
	    case hermitian_symmetric:

	      /*
	       * The only possible operators here are &, |, or !=.
	       * With the first two, the result is symmetric.
	       */

	      y.real = y.imag = 1.0;

	      if ((*func)(y, y))
		{
		  s->symmetry = symmetric;
		}
	      else
		{
		  assert (s->density != sparse_upper);
		  s->symmetry = general;
		}
	      break;

	    default:

	      s->symmetry = general;
	    }

	  s->rid = l->rid ? copy_entity (l->rid) :
	    r->rid ? copy_entity (r->rid) : NULL;
	  s->cid = l->cid ? copy_entity (l->cid) :
	    r->cid ? copy_entity (r->cid) : NULL;

	  if (l->nn)
	    {
	      if (r->nn)
		{
		  for (m=0; m<s->nr; m++)
		    {
		      for (i=l->ia[m]-1, j=r->ia[m]-1, k=s->ia[m]-1;
			   k<s->ia[m+1]-1; k++)
			{
			  if (i>=l->ia[m+1]-1 || l->ja[i] > s->ja[k])
			    y.real = y.imag = 0.0;
			  else
			    y = l->a.complex[i++];

			  if (j>=r->ia[m+1]-1 || r->ja[j] > s->ja[k])
			    z.real = z.imag = 0.0;
			  else
			    z = r->a.complex[j++];

			  s->a.integer[k] = (*func) (y, z);
			}
		    }
		}
	      else
		{
		  for (k=0; k<l->nn; k++)
		    s->a.integer[k] = (*func) (l->a.complex[k], x);
		}
	    }
	  else
	    if (r->nn)
	      {
		for (k=0; k<r->nn; k++)
		  s->a.integer[k] = (*func) (x, r->a.complex[k]);
	      }

	  assert (s->density != sparse_upper ||
		  (l->density == sparse_upper && r->density == sparse_upper));
	  if (s->density == sparse_upper)
	    {
	      if (l->d.complex || r->d.complex)
		s->d.integer = CALLOC (s->nr, sizeof (int));

	      if (l->d.complex)
		{
		  if (r->d.complex)
		    {
		      for (k=0; k<s->nr; k++)
			s->d.integer[k] =
			  (*func) (l->d.complex[k], r->d.complex[k]);
		    }
		  else
		    {	    
		      for (k=0; k<s->nr; k++)
			s->d.integer[k] = (*func) (l->d.complex[k], x);
		    }
		}
	      else
		{
		  if (r->d.complex)
		    {
		      for (k=0; k<s->nr; k++)
			s->d.integer[k] = (*func) (x, r->d.complex[k]);
		    }
		}
	    }

	  s = (MATRIX *) sparse_matrix ((MATRIX *) EAT (s));
	  s = (MATRIX *) apt_matrix ((MATRIX *) EAT (s));
	}
    }
  ON_EXCEPTION
    {
      delete_3_matrices (l, r, s);
    }
  END_EXCEPTION;

  delete_2_matrices (l, r);
  return ENT (s);
}

ENTITY *
logical_matrix_character (MATRIX *l, MATRIX *r, int PROTO ((*func), (char *, char *)))
{
  MATRIX *s;
  int i;

  EASSERT (l, matrix, character);
  EASSERT (r, matrix, character);

  if (l->nr != r->nr || l->nc != r->nc)
    {
      fail (incon_matrix,
	    l->nr, PLURAL (l->nr), l->nc, PLURAL (l->nc),
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

  l = (MATRIX *) dense_matrix (l);
  r = (MATRIX *) dense_matrix (r);
  s = (MATRIX *) form_matrix (l->nr, l->nc, integer, dense);
  s->symmetry = general;

  if (l->rid)
    s->rid = copy_entity (l->rid);
  else if (r->rid)
    s->rid = copy_entity (r->rid);
  if (l->cid)
    s->cid = copy_entity (l->cid);
  else if (r->cid)
    s->cid = copy_entity (r->cid);

  for (i = 0; i < s->nn; i++)
    s->a.integer[i] = (*func) (l->a.character[i], r->a.character[i]);

  delete_2_matrices (l, r);
  return (ENT (s));
}
