/*
   not.c -- Algae's `not' operator.

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: not.c,v 1.2 1997/02/21 09:54:02 ksh Exp $";

#include "not.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "apply.h"
#include "dense.h"

static int 
not_integer (int i)
{
  return (!i);
}

static int 
not_real (REAL i)
{
  return (!i);
}

static int 
not_complex (COMPLEX i)
{
  return (!i.real && !i.imag);
}

static int 
not_character (char *i)
{
  return (!*i);
}

ENTITY *
not_scalar (SCALAR *p)
{
  SCALAR *s = NULL;

  EASSERT (p, scalar, 0);

  switch (p->type)
    {
    case integer:
      s = (SCALAR *) int_to_scalar (!p->v.integer);
      break;
    case real:
      s = (SCALAR *) int_to_scalar (!p->v.real);
      break;
    case complex:
      s = (SCALAR *) int_to_scalar (!p->v.complex.real &&
				    !p->v.complex.imag);
      break;
    case character:
      s = (SCALAR *) int_to_scalar (!*p->v.character);
      break;
    default:
      BAD_TYPE (p->type);
      delete_scalar (p);
      raise_exception ();
    }

  delete_scalar (p);
  return (ENT (s));
}

ENTITY *
not_vector (VECTOR *p)
{
  VECTOR *m;

  EASSERT (p, vector, 0);

  switch (p->type)
    {
    case integer:
      p = (VECTOR *) dense_vector (p);
      m = (VECTOR *) apply_vector_integer_integer (not_integer, p);
      break;
    case real:
      p = (VECTOR *) dense_vector (p);
      m = (VECTOR *) apply_vector_integer_real (not_real, p);
      break;
    case complex:
      p = (VECTOR *) dense_vector (p);
      m = (VECTOR *) apply_vector_integer_complex (not_complex, p);
      break;
    case character:
      p = (VECTOR *) dense_vector (p);
      m = (VECTOR *) apply_vector_integer_character (not_character, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
not_matrix (MATRIX *p)
{
  MATRIX *m;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {
    case integer:
      p = (MATRIX *) dense_matrix (p);
      m = (MATRIX *) apply_matrix_integer_integer (not_integer, p);
      break;
    case real:
      p = (MATRIX *) dense_matrix (p);
      m = (MATRIX *) apply_matrix_integer_real (not_real, p);
      break;
    case complex:
      p = (MATRIX *) dense_matrix (p);
      m = (MATRIX *) apply_matrix_integer_complex (not_complex, p);
      break;
    case character:
      p = (MATRIX *) dense_matrix (p);
      m = (MATRIX *) apply_matrix_integer_character (not_character, p);
      break;
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (m));
}

ENTITY *
notnot_scalar (SCALAR *p)
{
  SCALAR *s = NULL;

  EASSERT (p, scalar, 0);

  switch (p->type)
    {
    case integer:
      s = (SCALAR *) int_to_scalar (p->v.integer != 0);
      break;
    case real:
      s = (SCALAR *) int_to_scalar (p->v.real != 0.0);
      break;
    case complex:
      s = (SCALAR *) int_to_scalar (p->v.complex.real != 0.0 ||
				    p->v.complex.imag != 0.0);
      break;
    case character:
      s = (SCALAR *) int_to_scalar (*p->v.character != '\0');
      break;
    default:
      BAD_TYPE (p->type);
      delete_scalar (p);
      raise_exception ();
    }

  delete_scalar (p);
  return (ENT (s));
}

ENTITY *
notnot_vector (VECTOR *p)
{
  int i;
  VECTOR *m;

  EASSERT (p, vector, 0);

  switch (p->type)
    {

    case integer:

      m = (VECTOR *) dup_vector (EAT (p));

      for (i = 0; i < m->nn; i++)
	m->a.integer[i] = m->a.integer[i] != 0;

      break;

    case real:

      m = (VECTOR *) form_vector (p->ne, integer, p->density);
      if (p->eid)
	m->eid = copy_entity (p->eid);
      if (p->ja)
	m->ja = dup_mem (p->ja, p->nn * sizeof (int));

      m->nn = p->nn;

      if (p->a.real)
	{
	  if (m->a.integer == NULL)
	    m->a.integer = MALLOC (m->nn * sizeof (int));
	  for (i = 0; i < m->nn; i++)
	    m->a.integer[i] = p->a.real[i] != 0.0;
	}

      break;

    case complex:

      m = (VECTOR *) form_vector (p->ne, integer, p->density);
      if (p->eid)
	m->eid = copy_entity (p->eid);
      if (p->ja)
	m->ja = dup_mem (p->ja, p->nn * sizeof (int));

      m->nn = p->nn;

      if (p->a.complex)
	{
	  if (m->a.integer == NULL)
	    m->a.integer = MALLOC (m->nn * sizeof (int));
	  for (i = 0; i < m->nn; i++)
	    m->a.integer[i] = (p->a.complex[i].real != 0.0 ||
			       p->a.complex[i].imag != 0.0);
	}

      break;

    case character:

      m = (VECTOR *) form_vector (p->ne, integer, p->density);
      if (p->eid)
	m->eid = copy_entity (p->eid);
      if (p->ja)
	m->ja = dup_mem (p->ja, p->nn * sizeof (int));

      m->nn = p->nn;

      if (p->a.character)
	{
	  if (m->a.integer == NULL)
	    m->a.integer = MALLOC (m->nn * sizeof (int));
	  for (i = 0; i < m->nn; i++)
	    m->a.integer[i] = *p->a.character[i] != '\0';
	}

      break;

    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }

  delete_vector (p);
  return (ENT (m));
}

ENTITY *
notnot_matrix (MATRIX *p)
{
  int i;
  MATRIX *m;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {

    case integer:

      m = (MATRIX *) dup_matrix (EAT (p));

      if (m->a.integer)
	{
	  for (i = 0; i < m->nn; i++)
	    m->a.integer[i] = m->a.integer[i] != 0;
	}

      if (m->d.integer)
	{
	  for (i = 0; i < m->nr; i++)
	    m->d.integer[i] = m->d.integer[i] != 0;
	}

      break;

    case real:

      m = (MATRIX *) form_matrix (p->nr, p->nc, integer, p->density);
      m->symmetry = p->symmetry;
      if (p->rid)
	m->rid = copy_entity (p->rid);
      if (p->cid)
	m->cid = copy_entity (p->cid);
      if (p->ia)
	m->ia = dup_mem (p->ia, (p->nr + 1) * sizeof (int));
      if (p->ja)
	m->ja = dup_mem (p->ja, p->nn * sizeof (int));

      m->nn = p->nn;

      if (p->a.real)
	{
	  if (m->a.integer == NULL)
	    m->a.integer = MALLOC (m->nn * sizeof (int));
	  for (i = 0; i < m->nn; i++)
	    m->a.integer[i] = p->a.real[i] != 0.0;
	}

      if (p->d.real)
	{
	  if (m->d.integer == NULL)
	    m->d.integer = MALLOC (m->nr * sizeof (int));
	  for (i = 0; i < m->nr; i++)
	    m->d.integer[i] = p->d.real[i] != 0.0;
	}

      break;

    case complex:

      m = (MATRIX *) form_matrix (p->nr, p->nc, integer, p->density);
      m->symmetry = (p->symmetry == general) ? general : symmetric;
      if (p->rid)
	m->rid = copy_entity (p->rid);
      if (p->cid)
	m->cid = copy_entity (p->cid);
      if (p->ia)
	m->ia = dup_mem (p->ia, (p->nr + 1) * sizeof (int));
      if (p->ja)
	m->ja = dup_mem (p->ja, p->nn * sizeof (int));

      m->nn = p->nn;

      if (p->a.complex)
	{
	  if (m->a.integer == NULL)
	    m->a.integer = MALLOC (m->nn * sizeof (int));
	  for (i = 0; i < m->nn; i++)
	    m->a.integer[i] = (p->a.complex[i].real != 0.0 ||
			       p->a.complex[i].imag != 0.0);
	}

      if (p->d.complex)
	{
	  if (m->d.integer == NULL)
	    m->d.integer = MALLOC (m->nr * sizeof (int));
	  for (i = 0; i < m->nr; i++)
	    m->d.integer[i] = (p->d.complex[i].real != 0.0 ||
			       p->d.complex[i].imag != 0.0);
	}

      break;

    case character:

      m = (MATRIX *) form_matrix (p->nr, p->nc, integer, p->density);
      m->symmetry = p->symmetry;
      if (p->rid)
	m->rid = copy_entity (p->rid);
      if (p->cid)
	m->cid = copy_entity (p->cid);
      if (p->ia)
	m->ia = dup_mem (p->ia, (p->nr + 1) * sizeof (int));
      if (p->ja)
	m->ja = dup_mem (p->ja, p->nn * sizeof (int));

      m->nn = p->nn;

      if (p->a.character)
	{
	  if (m->a.integer == NULL)
	    m->a.integer = MALLOC (m->nn * sizeof (int));
	  for (i = 0; i < m->nn; i++)
	    m->a.integer[i] = *p->a.character[i] != '\0';
	}

      if (p->d.character)
	{
	  if (m->d.integer == NULL)
	    m->d.integer = MALLOC (m->nr * sizeof (int));
	  for (i = 0; i < m->nr; i++)
	    m->d.integer[i] = *p->d.character[i] != '\0';
	}

      break;

    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }

  delete_matrix (p);
  return (ENT (m));
}
