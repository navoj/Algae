/*
   cast.c -- Cast types.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: cast.c,v 1.7 2003/12/06 04:42:58 ksh Exp $";

#include "cast.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "two_type.h"
#include "real.h"
#include "dense.h"
#include "datum.h"

extern DATUM *num_digits;
#define DIGITS datum_to_int (num_digits)

ENTITY *
bi_integer (ENTITY *p)
{
  EASSERT (p, 0, 0);

  return (cast_entity (p, integer));
}

ENTITY *
cast_entity (ENTITY *ip, TYPE type)
{
  EASSERT (ip, 0, 0);

  switch (ip->class)
    {
    case scalar:
      return (cast_scalar ((SCALAR *) ip, type));
    case vector:
      return (cast_vector ((VECTOR *) ip, type));
    case matrix:
      return (cast_matrix ((MATRIX *) ip, type));
    default:
      BAD_CLASS (ip->class);
      delete_entity (ip);
      raise_exception ();
    }
}

ENTITY *
cast_scalar (SCALAR *ips, TYPE type)
{
  EASSERT (ips, scalar, 0);

  switch (TWO_TYPE (ips->type, type))
    {

    case integer_integer:
      return (ENT (ips));
    case integer_real:
      return (cast_scalar_integer_real (ips));
    case integer_complex:
      return (cast_scalar_integer_complex (ips));
    case integer_character:
      return (cast_scalar_integer_character (ips));

    case real_integer:
      return (cast_scalar_real_integer (ips));
    case real_real:
      return (ENT (ips));
    case real_complex:
      return (cast_scalar_real_complex (ips));
    case real_character:
      return (cast_scalar_real_character (ips));

    case complex_integer:
      return (cast_scalar_complex_integer (ips));
    case complex_real:
      return (cast_scalar_complex_real (ips));
    case complex_complex:
      return (ENT (ips));
    case complex_character:
      return (cast_scalar_complex_character (ips));

    case character_character:
      return (ENT (ips));
    default:
      fail ("Can't cast scalar from \"%s\" to \"%s\".",
	    type_string[ips->type], type_string[type]);
    }
  delete_entity (ENT (ips));
  raise_exception ();
}

ENTITY *
cast_vector (VECTOR *ipv, TYPE type)
{
  EASSERT (ipv, vector, 0);

  switch (TWO_TYPE (ipv->type, type))
    {
    case integer_integer:
      return (ENT (ipv));
    case integer_real:
      return (cast_vector_integer_real (ipv));
    case integer_complex:
      return (cast_vector_integer_complex (ipv));
    case integer_character:
      return (cast_vector_integer_character (ipv));
    case real_integer:
      return (cast_vector_real_integer (ipv));
    case real_real:
      return (ENT (ipv));
    case real_complex:
      return (cast_vector_real_complex (ipv));
    case real_character:
      return (cast_vector_real_character (ipv));
    case complex_integer:
      return (cast_vector_complex_integer (ipv));
    case complex_real:
      return (cast_vector_complex_real (ipv));
    case complex_complex:
      return (ENT (ipv));
    case complex_character:
      return (cast_vector_complex_character (ipv));
    case character_character:
      return (ENT (ipv));
    default:
      fail ("Can't cast vector from \"%s\" to \"%s\".",
	    type_string[ipv->type], type_string[type]);
    }
  delete_entity (ENT (ipv));
  raise_exception ();
}

ENTITY *
cast_matrix (MATRIX *ipm, TYPE type)
{
  EASSERT (ipm, matrix, 0);

  switch (TWO_TYPE (ipm->type, type))
    {
    case integer_integer:
      return (ENT (ipm));
    case integer_real:
      return (cast_matrix_integer_real (ipm));
    case integer_complex:
      return (cast_matrix_integer_complex (ipm));
    case integer_character:
      return (cast_matrix_integer_character (ipm));
    case real_integer:
      return (cast_matrix_real_integer (ipm));
    case real_real:
      return (ENT (ipm));
    case real_complex:
      return (cast_matrix_real_complex (ipm));
    case real_character:
      return (cast_matrix_real_character (ipm));
    case complex_integer:
      return (cast_matrix_complex_integer (ipm));
    case complex_real:
      return (cast_matrix_complex_real (ipm));
    case complex_complex:
      return (ENT (ipm));
    case complex_character:
      return (cast_matrix_complex_character (ipm));
    case character_character:
      return (ENT (ipm));
    default:
      fail ("Can't cast matrix from \"%s\" to \"%s\".",
	    type_string[ipm->type], type_string[type]);
    }
  delete_entity (ENT (ipm));
  raise_exception ();
}

ENTITY *
cast_scalar_integer_real (SCALAR *ips)
{
  SCALAR *s;

  EASSERT (ips, scalar, integer);

  s = (SCALAR *) dup_scalar (ips);
  s->v.real = (REAL) s->v.integer;
  s->type = real;
  return (ENT (s));
}

ENTITY *
cast_scalar_integer_complex (SCALAR *ips)
{
  SCALAR *s;

  EASSERT (ips, scalar, integer);

  s = (SCALAR *) dup_scalar (ips);
  s->v.complex.real = (REAL) s->v.integer;
  s->v.complex.imag = 0;
  s->type = complex;
  return (ENT (s));
}

ENTITY *
cast_scalar_integer_character (SCALAR *p)
{
  SCALAR *s;

  EASSERT (p, scalar, integer);

  s = (SCALAR *) dup_scalar (p);
#if HAVE_SNPRINTF
  if (snprintf (printf_buf, PRINTF_BUF_SIZE, "%d", s->v.integer)
      >= PRINTF_BUF_SIZE)
    {
      fail ("Overflow in sprintf buffer.");
      raise_exception ();
    }
#else
  sprintf (printf_buf, "%d", s->v.integer);
#endif
  s->v.character = dup_char (printf_buf);
  s->type = character;
  return (ENT (s));
}

ENTITY *
cast_scalar_real_integer (SCALAR *ips)
{
  SCALAR *s;

  EASSERT (ips, scalar, real);

  s = (SCALAR *) dup_scalar (ips);
  s->v.integer = (int) round (s->v.real);
  s->type = integer;
  return (ENT (s));
}

ENTITY *
cast_scalar_real_complex (SCALAR *ips)
{
  SCALAR *s;

  EASSERT (ips, scalar, real);

  s = (SCALAR *) dup_scalar (ips);
  s->v.complex.real = (REAL) s->v.real;
  s->v.complex.imag = 0;
  s->type = complex;
  return (ENT (s));
}

ENTITY *
cast_scalar_real_character (SCALAR *p)
{
  SCALAR *s;

  EASSERT (p, scalar, real);

  s = (SCALAR *) dup_scalar (p);
#if HAVE_SNPRINTF
  if (snprintf (printf_buf, PRINTF_BUF_SIZE, "%#.*g", DIGITS, s->v.real)
      >= PRINTF_BUF_SIZE)
    {
      fail ("Overflow in sprintf buffer.");
      raise_exception ();
    }
#else
  sprintf (printf_buf, "%#.*g", DIGITS, s->v.real);
#endif
  s->v.character = dup_char (printf_buf);
  s->type = character;
  return (ENT (s));
}

ENTITY *
cast_scalar_complex_integer (SCALAR *ips)
{
  SCALAR *s;

  EASSERT (ips, scalar, complex);

  s = (SCALAR *) dup_scalar (ips);
  s->v.integer = (int) round (s->v.complex.real);
  s->type = integer;
  return (ENT (s));
}

ENTITY *
cast_scalar_complex_real (SCALAR *ips)
{
  SCALAR *s;

  EASSERT (ips, scalar, complex);

  s = (SCALAR *) dup_scalar (ips);
  s->v.real = s->v.complex.real;
  s->type = real;
  return (ENT (s));
}

ENTITY *
cast_scalar_complex_character (SCALAR *p)
{
  SCALAR *s;

  EASSERT (p, scalar, complex);

  s = (SCALAR *) dup_scalar (p);
#if HAVE_SNPRINTF
  if (snprintf (printf_buf, PRINTF_BUF_SIZE, "%#.*g + %#.*g*i",
		DIGITS, s->v.complex.real,
		DIGITS, s->v.complex.imag) >= PRINTF_BUF_SIZE)
    {
      fail ("Overflow in sprintf buffer.");
      raise_exception ();
    }
#else
  sprintf (printf_buf, "%#.*g + %#.*g*i",
	   DIGITS, s->v.complex.real, DIGITS, s->v.complex.imag);
#endif
  s->v.character = dup_char (printf_buf);
  s->type = character;
  return (ENT (s));
}

ENTITY *
cast_scalar_character_integer (SCALAR *ips)
{
  SCALAR *s;
  int i;

  EASSERT (ips, scalar, character);

  i = *ips->v.character;
  s = (SCALAR *) dup_scalar (ips);
  FREE_CHAR (s->v.character);
  s->v.integer = i;
  s->type = integer;
  return (ENT (s));
}

ENTITY *
cast_scalar_character_real (SCALAR *ips)
{
  detour ("No code for `cast_scalar_character_real'.");
  delete_entity (ENT (ips));
  raise_exception ();
}

ENTITY *
cast_scalar_character_complex (SCALAR *ips)
{
  detour ("No code for `cast_scalar_character_complex'.");
  delete_entity (ENT (ips));
  raise_exception ();
}

ENTITY *
cast_vector_integer_real (VECTOR *ipv)
{
  VECTOR *v;
  int i;

  EASSERT (ipv, vector, integer);

  v = (VECTOR *) form_vector (ipv->ne, real, ipv->density);
  if (ipv->eid != NULL)
    v->eid = copy_entity (ipv->eid);
  if (ipv->ja != NULL)
    v->ja = dup_mem (ipv->ja, ipv->nn * sizeof (int));

  v->nn = ipv->nn;
  if (ipv->a.integer != NULL)
    {
      if (v->a.real == NULL)
	v->a.real = E_MALLOC (v->nn, real);
      for (i = 0; i < v->nn; i++)
	v->a.real[i] = (REAL) ipv->a.integer[i];
    }

  delete_entity (ENT (ipv));
  return (ENT (v));
}

ENTITY *
cast_vector_integer_complex (VECTOR *ipv)
{
  VECTOR *v;
  int i;

  EASSERT (ipv, vector, integer);

  v = (VECTOR *) form_vector (ipv->ne, complex, ipv->density);
  if (ipv->eid != NULL)
    v->eid = copy_entity (ipv->eid);
  if (ipv->ja != NULL)
    v->ja = dup_mem (ipv->ja, ipv->nn * sizeof (int));

  v->nn = ipv->nn;
  if (ipv->a.integer != NULL)
    {
      if (v->a.complex == NULL)
	v->a.complex = E_MALLOC (v->nn, complex);
      for (i = 0; i < v->nn; i++)
	{
	  v->a.complex[i].real = (REAL) ipv->a.integer[i];
	  v->a.complex[i].imag = 0.0;
	}
    }

  delete_vector (ipv);
  return (ENT (v));
}

ENTITY *
cast_vector_integer_character (VECTOR *ipv)
{
  VECTOR *v;
  int i;

  EASSERT (ipv, vector, integer);

  ipv = (VECTOR *) dense_vector (ipv);

  v = (VECTOR *) form_vector (ipv->ne, character, ipv->density);
  if (ipv->eid != NULL)
    v->eid = copy_entity (ipv->eid);
  if (ipv->ja != NULL)
    v->ja = dup_mem (ipv->ja, ipv->nn * sizeof (int));

  v->nn = ipv->nn;
  if (v->nn > 0)
    {
      if (v->a.character == NULL)
	v->a.character = E_MALLOC (v->nn, character);
      for (i = 0; i < v->nn; i++)
	{
#if HAVE_SNPRINTF
	  if (snprintf (printf_buf, PRINTF_BUF_SIZE, "%d", ipv->a.integer[i])
	      >= PRINTF_BUF_SIZE)
	    {
	      fail ("Overflow in sprintf buffer.");
	      raise_exception ();
	    }
#else
	  sprintf (printf_buf, "%d", ipv->a.integer[i]);
#endif
	  v->a.character[i] = dup_char (printf_buf);
	}
    }

  delete_vector (ipv);
  return (ENT (v));
}

ENTITY *
cast_vector_real_integer (VECTOR *ipv)
{
  VECTOR *v;
  int i;

  EASSERT (ipv, vector, real);

  v = (VECTOR *) form_vector (ipv->ne, integer, ipv->density);
  if (ipv->eid != NULL)
    v->eid = copy_entity (ipv->eid);
  if (ipv->ja != NULL)
    v->ja = dup_mem (ipv->ja, ipv->nn * sizeof (int));

  v->nn = ipv->nn;
  if (ipv->a.real != NULL)
    {
      if (v->a.integer == NULL)
	v->a.integer = E_MALLOC (v->nn, integer);
      for (i = 0; i < v->nn; i++)
	v->a.integer[i] = (int) round (ipv->a.real[i]);
    }

  delete_entity (ENT (ipv));
  return (ENT (v));
}

ENTITY *
cast_vector_real_complex (VECTOR *ipv)
{
  VECTOR *v;
  int i;

  EASSERT (ipv, vector, real);

  v = (VECTOR *) form_vector (ipv->ne, complex, ipv->density);
  if (ipv->eid != NULL)
    v->eid = copy_entity (ipv->eid);
  if (ipv->ja != NULL)
    v->ja = dup_mem (ipv->ja, ipv->nn * sizeof (int));

  v->nn = ipv->nn;
  if (ipv->a.real != NULL)
    {
      if (v->a.complex == NULL)
	v->a.complex = E_MALLOC (v->nn, complex);
      for (i = 0; i < v->nn; i++)
	{
	  v->a.complex[i].real = ipv->a.real[i];
	  v->a.complex[i].imag = 0.0;
	}
    }

  delete_vector (ipv);
  return (ENT (v));
}

ENTITY *
cast_vector_real_character (VECTOR *ipv)
{
  VECTOR *v;
  int i;
  int d = DIGITS;

  EASSERT (ipv, vector, real);

  ipv = (VECTOR *) dense_vector (ipv);

  v = (VECTOR *) form_vector (ipv->ne, character, ipv->density);
  if (ipv->eid != NULL)
    v->eid = copy_entity (ipv->eid);
  if (ipv->ja != NULL)
    v->ja = dup_mem (ipv->ja, ipv->nn * sizeof (int));

  v->nn = ipv->nn;
  if (v->nn > 0)
    {
      if (v->a.character == NULL)
	v->a.character = E_MALLOC (v->nn, character);
      for (i = 0; i < v->nn; i++)
	{
#if HAVE_SNPRINTF
	  if (snprintf (printf_buf, PRINTF_BUF_SIZE,
			"%#.*g", d, ipv->a.real[i])
	      >= PRINTF_BUF_SIZE)
	    {
	      fail ("Overflow in sprintf buffer.");
	      raise_exception ();
	    }
#else
	  sprintf (printf_buf, "%#.*g", d, ipv->a.real[i]);
#endif
	  v->a.character[i] = dup_char (printf_buf);
	}
    }

  delete_vector (ipv);
  return (ENT (v));
}

ENTITY *
cast_vector_complex_integer (VECTOR *ipv)
{
  VECTOR *v;
  int i;

  EASSERT (ipv, vector, complex);

  v = (VECTOR *) form_vector (ipv->ne, integer, ipv->density);
  if (ipv->eid != NULL)
    v->eid = copy_entity (ipv->eid);
  if (ipv->ja != NULL)
    v->ja = dup_mem (ipv->ja, ipv->nn * sizeof (int));

  v->nn = ipv->nn;
  if (ipv->a.complex != NULL)
    {
      if (v->a.integer == NULL)
	v->a.integer = E_MALLOC (v->nn, integer);
      for (i = 0; i < v->nn; i++)
	v->a.integer[i] = (int) round (ipv->a.complex[i].real);
    }

  delete_entity (ENT (ipv));
  return (ENT (v));
}

ENTITY *
cast_vector_complex_real (VECTOR *ipv)
{
  VECTOR *v;
  int i;

  EASSERT (ipv, vector, complex);

  v = (VECTOR *) form_vector (ipv->ne, real, ipv->density);
  if (ipv->eid != NULL)
    v->eid = copy_entity (ipv->eid);
  if (ipv->ja != NULL)
    v->ja = dup_mem (ipv->ja, ipv->nn * sizeof (int));

  v->nn = ipv->nn;
  if (ipv->a.complex != NULL)
    {
      if (v->a.real == NULL)
	v->a.real = E_MALLOC (v->nn, real);
      for (i = 0; i < v->nn; i++)
	v->a.real[i] = ipv->a.complex[i].real;
    }

  delete_entity (ENT (ipv));
  return (ENT (v));
}

ENTITY *
cast_vector_complex_character (VECTOR *ipv)
{
  VECTOR *v;
  int i;
  int d = DIGITS;

  EASSERT (ipv, vector, complex);

  v = (VECTOR *) form_vector (ipv->ne, character, ipv->density);
  if (ipv->eid != NULL) v->eid = copy_entity (ipv->eid);
  if (ipv->ja != NULL)
    v->ja = dup_mem (ipv->ja, ipv->nn * sizeof (int));

  v->nn = ipv->nn;
  if (ipv->a.complex != NULL)
    {
      if (v->a.character == NULL)
	v->a.character = E_MALLOC (v->nn, character);
      for (i = 0; i < v->nn; i++)
	{
#if HAVE_SNPRINTF
	  if (snprintf (printf_buf, PRINTF_BUF_SIZE, "%#.*g + %#.*g*i",
			d, ipv->a.complex[i].real,
			d, ipv->a.complex[i].imag) >= PRINTF_BUF_SIZE)
	    {
	      fail ("Overflow in sprintf buffer.");
	      raise_exception ();
	    }
#else
	  sprintf (printf_buf, "%#.*g + %#.*g*i",
		   d, ipv->a.complex[i].real, d, ipm->a.complex[i].imag);
#endif
	  v->a.character[i] = dup_char (printf_buf);
	}
    }

  delete_entity (ENT (ipv));
  return ENT (v);
}

ENTITY *
cast_vector_character_integer (VECTOR *ipv)
{
  VECTOR *v;
  int i;

  EASSERT (ipv, vector, character);

  v = (VECTOR *) form_vector (ipv->ne, integer, ipv->density);
  if (ipv->eid != NULL)
    v->eid = copy_entity (ipv->eid);
  if (ipv->ja != NULL)
    v->ja = dup_mem (ipv->ja, ipv->nn * sizeof (int));

  v->nn = ipv->nn;
  if (ipv->a.character != NULL)
    {
      if (v->a.integer == NULL)
	v->a.integer = E_MALLOC (v->nn, integer);
      for (i = 0; i < v->nn; i++)
	v->a.integer[i] = *ipv->a.character[i];
    }

  delete_entity (ENT (ipv));
  return (ENT (v));
}

ENTITY *
cast_matrix_integer_real (MATRIX *ipm)
{
  MATRIX *m;
  int i;

  EASSERT (ipm, matrix, integer);

  m = (MATRIX *) form_matrix (ipm->nr, ipm->nc, real, ipm->density);
  m->symmetry = ipm->symmetry;
  m->order = ipm->order;
  if (ipm->rid != NULL)
    m->rid = copy_entity (ipm->rid);
  if (ipm->cid != NULL)
    m->cid = copy_entity (ipm->cid);
  if (ipm->ia != NULL)
    m->ia = dup_mem (ipm->ia, (ipm->nr + 1) * sizeof (int));
  if (ipm->ja != NULL)
    m->ja = dup_mem (ipm->ja, ipm->nn * sizeof (int));

  m->nn = ipm->nn;
  if (ipm->a.integer != NULL)
    {
      if (m->a.real == NULL)
	m->a.real = E_MALLOC (m->nn, real);
      for (i = 0; i < m->nn; i++)
	m->a.real[i] = (REAL) ipm->a.integer[i];
    }
  if (ipm->d.integer != NULL)
    {
      if (m->d.real == NULL)
	m->d.real = E_MALLOC (m->nr, real);
      for (i = 0; i < m->nr; i++)
	m->d.real[i] = (REAL) ipm->d.integer[i];
    }

  delete_entity (ENT (ipm));
  return (ENT (m));
}

ENTITY *
cast_matrix_integer_complex (MATRIX *ipm)
{
  MATRIX *m;
  int i;

  EASSERT (ipm, matrix, integer);

  m = (MATRIX *) form_matrix (ipm->nr, ipm->nc, complex, ipm->density);
  switch (ipm->symmetry)
    {
    case symmetric:
      m->symmetry = hermitian;
      break;
    default:
      m->symmetry = ipm->symmetry;
    }
  m->order = ipm->order;
  if (ipm->rid != NULL)
    m->rid = copy_entity (ipm->rid);
  if (ipm->cid != NULL)
    m->cid = copy_entity (ipm->cid);
  if (ipm->ia != NULL)
    m->ia = dup_mem (ipm->ia, (ipm->nr + 1) * sizeof (int));
  if (ipm->ja != NULL)
    m->ja = dup_mem (ipm->ja, ipm->nn * sizeof (int));

  m->nn = ipm->nn;
  if (ipm->a.integer != NULL)
    {
      if (m->a.complex == NULL)
	m->a.complex = E_MALLOC (m->nn, complex);
      for (i = 0; i < m->nn; i++)
	{
	  m->a.complex[i].real = (REAL) ipm->a.integer[i];
	  m->a.complex[i].imag = 0.0;
	}
    }
  if (ipm->d.integer != NULL)
    {
      if (m->d.complex == NULL)
	m->d.complex = E_MALLOC (m->nr, complex);
      for (i = 0; i < m->nr; i++)
	{
	  m->d.complex[i].real = (REAL) ipm->d.integer[i];
	  m->d.complex[i].imag = 0.0;
	}
    }

  delete_entity (ENT (ipm));
  return (ENT (m));
}

ENTITY *
cast_matrix_integer_character (MATRIX *ipm)
{
  MATRIX *m;
  int i;

  EASSERT (ipm, matrix, integer);

  ipm = (MATRIX *) dense_matrix (ipm);

  m = (MATRIX *) form_matrix (ipm->nr, ipm->nc, character, ipm->density);

  if (ipm->rid != NULL) m->rid = copy_entity (ipm->rid);
  if (ipm->cid != NULL) m->cid = copy_entity (ipm->cid);

  m->nn = ipm->nn;
  if (m->nn > 0)
    {
      assert (m->a.character);
      for (i = 0; i < m->nn; i++)
	{
#if HAVE_SNPRINTF
	  if (snprintf (printf_buf, PRINTF_BUF_SIZE, "%d", ipm->a.integer[i])
	      >= PRINTF_BUF_SIZE)
	    {
	      fail ("Overflow in sprintf buffer.");
	      raise_exception ();
	    }
#else
	  sprintf (printf_buf, "%d", ipm->a.integer[i]);
#endif
	  m->a.character[i] = dup_char (printf_buf);
	}
    }

  delete_matrix (ipm);
  return ENT (m);
}

ENTITY *
cast_matrix_real_integer (MATRIX *ipm)
{
  MATRIX *m;
  int i;

  EASSERT (ipm, matrix, real);

  m = (MATRIX *) form_matrix (ipm->nr, ipm->nc, integer, ipm->density);
  m->symmetry = ipm->symmetry;
  m->order = ipm->order;
  if (ipm->rid != NULL)
    m->rid = copy_entity (ipm->rid);
  if (ipm->cid != NULL)
    m->cid = copy_entity (ipm->cid);
  if (ipm->ia != NULL)
    m->ia = dup_mem (ipm->ia, (ipm->nr + 1) * sizeof (int));
  if (ipm->ja != NULL)
    m->ja = dup_mem (ipm->ja, ipm->nn * sizeof (int));

  m->nn = ipm->nn;
  if (ipm->a.real != NULL)
    {
      if (m->a.integer == NULL)
	m->a.integer = E_MALLOC (m->nn, integer);
      for (i = 0; i < m->nn; i++)
	m->a.integer[i] = (int) round (ipm->a.real[i]);
    }
  if (ipm->d.real != NULL)
    {
      if (m->d.integer == NULL)
	m->d.integer = E_MALLOC (m->nr, integer);
      for (i = 0; i < m->nr; i++)
	m->d.integer[i] = (int) round (ipm->d.real[i]);
    }

  delete_entity (ENT (ipm));
  return (ENT (m));
}

ENTITY *
cast_matrix_real_complex (MATRIX *ipm)
{
  MATRIX *m;
  int i;

  EASSERT (ipm, matrix, real);

  m = (MATRIX *) form_matrix (ipm->nr, ipm->nc, complex, ipm->density);
  switch (ipm->symmetry)
    {
    case symmetric:
      m->symmetry = hermitian;
      break;
    default:
      m->symmetry = ipm->symmetry;
    }
  m->order = ipm->order;
  if (ipm->rid != NULL)
    m->rid = copy_entity (ipm->rid);
  if (ipm->cid != NULL)
    m->cid = copy_entity (ipm->cid);
  if (ipm->ia != NULL)
    m->ia = dup_mem (ipm->ia, (ipm->nr + 1) * sizeof (int));
  if (ipm->ja != NULL)
    m->ja = dup_mem (ipm->ja, ipm->nn * sizeof (int));

  m->nn = ipm->nn;
  if (ipm->a.real != NULL)
    {
      if (m->a.complex == NULL)
	m->a.complex = E_MALLOC (m->nn, complex);
      for (i = 0; i < m->nn; i++)
	{
	  m->a.complex[i].real = ipm->a.real[i];
	  m->a.complex[i].imag = 0.0;
	}
    }
  if (ipm->d.real != NULL)
    {
      if (m->d.complex == NULL)
	m->d.complex = E_MALLOC (m->nr, complex);
      for (i = 0; i < m->nr; i++)
	{
	  m->d.complex[i].real = ipm->d.real[i];
	  m->d.complex[i].imag = 0.0;
	}
    }

  delete_matrix (ipm);
  return (ENT (m));
}

ENTITY *
cast_matrix_real_character (MATRIX *ipm)
{
  MATRIX *m;
  int i;
  int d = DIGITS;

  EASSERT (ipm, matrix, real);

  ipm = (MATRIX *) dense_matrix (ipm);

  m = (MATRIX *) form_matrix (ipm->nr, ipm->nc, character, ipm->density);

  if (ipm->rid != NULL) m->rid = copy_entity (ipm->rid);
  if (ipm->cid != NULL) m->cid = copy_entity (ipm->cid);

  m->nn = ipm->nn;
  if (m->nn > 0)
    {
      assert (m->a.character);
      for (i = 0; i < m->nn; i++)
	{
#if HAVE_SNPRINTF
	  if (snprintf (printf_buf, PRINTF_BUF_SIZE,
			"%#.*g", d, ipm->a.real[i])
	      >= PRINTF_BUF_SIZE)
	    {
	      fail ("Overflow in sprintf buffer.");
	      raise_exception ();
	    }
#else
	  sprintf (printf_buf, "%#.*g", d, ipm->a.real[i]);
#endif
	  m->a.character[i] = dup_char (printf_buf);
	}
    }

  delete_matrix (ipm);
  return ENT (m);
}

ENTITY *
cast_matrix_complex_integer (MATRIX *ipm)
{
  MATRIX *m;
  int i;

  EASSERT (ipm, matrix, complex);

  m = (MATRIX *) form_matrix (ipm->nr, ipm->nc, integer, ipm->density);
  m->symmetry = ipm->symmetry;
  if (m->symmetry == hermitian)
    m->symmetry = symmetric;
  m->order = ipm->order;
  if (ipm->rid != NULL)
    m->rid = copy_entity (ipm->rid);
  if (ipm->cid != NULL)
    m->cid = copy_entity (ipm->cid);
  if (ipm->ia != NULL)
    m->ia = dup_mem (ipm->ia, (ipm->nr + 1) * sizeof (int));
  if (ipm->ja != NULL)
    m->ja = dup_mem (ipm->ja, ipm->nn * sizeof (int));

  m->nn = ipm->nn;
  if (ipm->a.complex != NULL)
    {
      if (m->a.integer == NULL)
	m->a.integer = E_MALLOC (m->nn, integer);
      for (i = 0; i < m->nn; i++)
	m->a.integer[i] = (int) round (ipm->a.complex[i].real);
    }
  if (ipm->d.complex != NULL)
    {
      if (m->d.integer == NULL)
	m->d.integer = E_MALLOC (m->nr, integer);
      for (i = 0; i < m->nr; i++)
	m->d.integer[i] = (int) round (ipm->d.complex[i].real);
    }

  delete_entity (ENT (ipm));
  return (ENT (m));
}

ENTITY *
cast_matrix_complex_real (MATRIX *ipm)
{
  MATRIX *m;
  int i;

  EASSERT (ipm, matrix, complex);

  m = (MATRIX *) form_matrix (ipm->nr, ipm->nc, real, ipm->density);
  m->symmetry = ipm->symmetry;
  if (m->symmetry == hermitian)
    m->symmetry = symmetric;
  m->order = ipm->order;
  if (ipm->rid != NULL)
    m->rid = copy_entity (ipm->rid);
  if (ipm->cid != NULL)
    m->cid = copy_entity (ipm->cid);
  if (ipm->ia != NULL)
    m->ia = dup_mem (ipm->ia, (ipm->nr + 1) * sizeof (int));
  if (ipm->ja != NULL)
    m->ja = dup_mem (ipm->ja, ipm->nn * sizeof (int));

  m->nn = ipm->nn;
  if (ipm->a.complex != NULL)
    {
      if (m->a.real == NULL)
	m->a.real = E_MALLOC (m->nn, real);
      for (i = 0; i < m->nn; i++)
	m->a.real[i] = ipm->a.complex[i].real;
    }
  if (ipm->d.complex != NULL)
    {
      if (m->d.real == NULL)
	m->d.real = E_MALLOC (m->nr, real);
      for (i = 0; i < m->nr; i++)
	m->d.real[i] = ipm->d.complex[i].real;
    }

  delete_entity (ENT (ipm));
  return (ENT (m));
}

ENTITY *
cast_matrix_complex_character (MATRIX *ipm)
{
  MATRIX *m;
  int i;
  int d = DIGITS;

  EASSERT (ipm, matrix, complex);

  m = (MATRIX *) form_matrix (ipm->nr, ipm->nc, character, ipm->density);
  m->symmetry = ipm->symmetry == hermitian ? general : ipm->symmetry;
  if (ipm->rid != NULL) m->rid = copy_entity (ipm->rid);
  if (ipm->cid != NULL) m->cid = copy_entity (ipm->cid);
  if (ipm->ia != NULL)
    m->ia = dup_mem (ipm->ia, (ipm->nr + 1) * sizeof (int));
  if (ipm->ja != NULL)
    m->ja = dup_mem (ipm->ja, ipm->nn * sizeof (int));

  m->nn = ipm->nn;
  if (ipm->a.complex != NULL)
    {
      if (m->a.character == NULL)
	m->a.character = E_MALLOC (m->nn, character);
      for (i = 0; i < m->nn; i++)
	{
#if HAVE_SNPRINTF
	  if (snprintf (printf_buf, PRINTF_BUF_SIZE, "%#.*g + %#.*g*i",
			d, ipm->a.complex[i].real,
			d, ipm->a.complex[i].imag) >= PRINTF_BUF_SIZE)
	    {
	      fail ("Overflow in sprintf buffer.");
	      raise_exception ();
	    }
#else
	  sprintf (printf_buf, "%#.*g + %#.*g*i",
		   d, ipm->a.complex[i].real, d, ipm->a.complex[i].imag);
#endif
	  m->a.character[i] = dup_char (printf_buf);
	}
    }
  if (ipm->d.complex != NULL)
    {
      if (m->d.character == NULL)
	m->d.character = E_MALLOC (m->nr, character);
      for (i = 0; i < m->nr; i++)
	{
#if HAVE_SNPRINTF
	  if (snprintf (printf_buf, PRINTF_BUF_SIZE, "%#.*g + %#.*g*i",
			d, ipm->d.complex[i].real,
			d, ipm->d.complex[i].imag) >= PRINTF_BUF_SIZE)
	    {
	      fail ("Overflow in sprintf buffer.");
	      raise_exception ();
	    }
#else
	  sprintf (printf_buf, "%#.*g + %#.*g*i",
		   d, ipm->d.complex[i].real, d, ipm->d.complex[i].imag);
#endif
	  m->d.character[i] = dup_char (printf_buf);
	}
    }

  delete_entity (ENT (ipm));
  return ENT (m);
}

ENTITY *
cast_matrix_character_integer (MATRIX *ipm)
{
  MATRIX *m;
  int i;

  EASSERT (ipm, matrix, character);

  m = (MATRIX *) form_matrix (ipm->nr, ipm->nc, integer, ipm->density);
  m->symmetry = ipm->symmetry;
  m->order = ipm->order;
  if (ipm->rid != NULL)
    m->rid = copy_entity (ipm->rid);
  if (ipm->cid != NULL)
    m->cid = copy_entity (ipm->cid);
  if (ipm->ia != NULL)
    m->ia = dup_mem (ipm->ia, (ipm->nr + 1) * sizeof (int));
  if (ipm->ja != NULL)
    m->ja = dup_mem (ipm->ja, ipm->nn * sizeof (int));

  m->nn = ipm->nn;
  if (ipm->a.character != NULL)
    {
      if (m->a.integer == NULL)
	m->a.integer = E_MALLOC (m->nn, integer);
      for (i = 0; i < m->nn; i++)
	m->a.integer[i] = *ipm->a.character[i];
    }
  if (ipm->d.character != NULL)
    {
      if (m->d.integer == NULL)
	m->d.integer = E_MALLOC (m->nr, integer);
      for (i = 0; i < m->nr; i++)
	m->d.integer[i] = *ipm->d.character[i];
    }

  delete_entity (ENT (ipm));
  return (ENT (m));
}

#ifdef STUPID_CPP

void
auto_cast (ENTITY **l, TYPE l_type, ENTITY **r, TYPE r_type)
{
  switch (TWO_TYPE (l_type, r_type))
    {
    case integer_integer:
    case real_real:
    case complex_complex:
    case character_character:
      break;
    case integer_real:
      *l = cast_entity (EAT (*l), real);
      break;
    case integer_complex:
    case real_complex:
      *l = cast_entity (EAT (*l), complex);
      break;
    case integer_character:
    case real_character:
    case complex_character:
      *l = cast_entity (EAT (*l), character);
      break;
    case real_integer:
      *r = cast_entity (EAT (*r), real);
      break;
    case complex_integer:
    case complex_real:
      *r = cast_entity (EAT (*r), complex);
      break;
    case character_integer:
    case character_real:
    case character_complex:
      *r = cast_entity (EAT (*r), character);
      break;
    default:
      wipeout ("Bad type.");
    }
}

void
auto_cast_scalar (SCALAR **l, SCALAR **r)
{
  if ((*l)->type != (*r)->type)
    {
      switch (TWO_TYPE ((*l)->type, (*r)->type))
	{
	case integer_real:
	  (*l) = (SCALAR *) cast_scalar_integer_real ((SCALAR *) EAT (*l));
	  break;
	case integer_complex:
	  (*l) = (SCALAR *) cast_scalar_integer_complex ((SCALAR *) EAT (*l));
	  break;
	case real_complex:
	  (*l) = (SCALAR *) cast_scalar_real_complex ((SCALAR *) EAT (*l));
	  break;
	case integer_character:
	  (*l) = (SCALAR *) cast_scalar_integer_character ((SCALAR *) EAT (*l));
	  break;
	case real_character:
	  (*l) = (SCALAR *) cast_scalar_real_character ((SCALAR *) EAT (*l));
	  break;
	case complex_character:
	  (*l) = (SCALAR *) cast_scalar_complex_character ((SCALAR *) EAT (*l));
	  break;
	case real_integer:
	  (*r) = (SCALAR *) cast_scalar_integer_real ((SCALAR *) EAT (*r));
	  break;
	case complex_integer:
	  (*r) = (SCALAR *) cast_scalar_integer_complex ((SCALAR *) EAT (*r));
	  break;
	case complex_real:
	  (*r) = (SCALAR *) cast_scalar_real_complex ((SCALAR *) EAT (*r));
	  break;
	case character_integer:
	  (*r) = (SCALAR *) cast_scalar_integer_character ((SCALAR *) EAT (*r));
	  break;
	case character_real:
	  (*r) = (SCALAR *) cast_scalar_real_character ((SCALAR *) EAT (*r));
	  break;
	case character_complex:
	  (*r) = (SCALAR *) cast_scalar_complex_character ((SCALAR *) EAT (*r));
	  break;
	default:
	  wipeout ("Bad type.");
	}
    }
}

void
auto_cast_vector (VECTOR **l, VECTOR **r)
{
  if ((*l)->type != (*r)->type)
    {
      switch (TWO_TYPE ((*l)->type, (*r)->type))
	{
	case integer_real:
	  (*l) = (VECTOR *) cast_vector_integer_real ((VECTOR *) EAT (*l));
	  break;
	case integer_complex:
	  (*l) = (VECTOR *) cast_vector_integer_complex ((VECTOR *) EAT (*l));
	  break;
	case real_complex:
	  (*l) = (VECTOR *) cast_vector_real_complex ((VECTOR *) EAT (*l));
	  break;
	case integer_character:
	  (*l) = (VECTOR *) cast_vector_integer_character ((VECTOR *) EAT (*l));
	  break;
	case real_character:
	  (*l) = (VECTOR *) cast_vector_real_character ((VECTOR *) EAT (*l));
	  break;
	case complex_character:
	  (*l) = (VECTOR *) cast_vector_complex_character ((VECTOR *) EAT (*l));
	  break;
	case real_integer:
	  (*r) = (VECTOR *) cast_vector_integer_real ((VECTOR *) EAT (*r));
	  break;
	case complex_integer:
	  (*r) = (VECTOR *) cast_vector_integer_complex ((VECTOR *) EAT (*r));
	  break;
	case complex_real:
	  (*r) = (VECTOR *) cast_vector_real_complex ((VECTOR *) EAT (*r));
	  break;
	case character_integer:
	  (*r) = (VECTOR *) cast_vector_integer_character ((VECTOR *) EAT (*r));
	  break;
	case character_real:
	  (*r) = (VECTOR *) cast_vector_real_character ((VECTOR *) EAT (*r));
	  break;
	case character_complex:
	  (*r) = (VECTOR *) cast_vector_complex_character ((VECTOR *) EAT (*r));
	  break;
	default:
	  wipeout ("Bad type.");
	}
    }
}

void
auto_cast_matrix (MATRIX **l, MATRIX **r)
{
  if ((*l)->type != (*r)->type)
    {
      switch (TWO_TYPE ((*l)->type, (*r)->type))
	{
	case integer_real:
	  (*l) = (MATRIX *) cast_matrix_integer_real ((MATRIX *) EAT (*l));
	  break;
	case integer_complex:
	  (*l) = (MATRIX *) cast_matrix_integer_complex ((MATRIX *) EAT (*l));
	  break;
	case real_complex:
	  (*l) = (MATRIX *) cast_matrix_real_complex ((MATRIX *) EAT (*l));
	  break;
	case integer_character:
	  (*l) = (MATRIX *) cast_matrix_integer_character ((MATRIX *) EAT (*l));
	  break;
	case real_character:
	  (*l) = (MATRIX *) cast_matrix_real_character ((MATRIX *) EAT (*l));
	  break;
	case complex_character:
	  (*l) = (MATRIX *) cast_matrix_complex_character ((MATRIX *) EAT (*l));
	  break;
	case real_integer:
	  (*r) = (MATRIX *) cast_matrix_integer_real ((MATRIX *) EAT (*r));
	  break;
	case complex_integer:
	  (*r) = (MATRIX *) cast_matrix_integer_complex ((MATRIX *) EAT (*r));
	  break;
	case complex_real:
	  (*r) = (MATRIX *) cast_matrix_real_complex ((MATRIX *) EAT (*r));
	  break;
	case character_integer:
	  (*r) = (MATRIX *) cast_matrix_integer_character ((MATRIX *) EAT (*r));
	  break;
	case character_real:
	  (*r) = (MATRIX *) cast_matrix_real_character ((MATRIX *) EAT (*r));
	  break;
	case character_complex:
	  (*r) = (MATRIX *) cast_matrix_complex_character ((MATRIX *) EAT (*r));
	  break;
	default:
	  wipeout ("Bad type.");
	}
    }
}

void
auto_cast_vector_scalar (VECTOR **l, SCALAR **r)
{
  if ((*l)->type != (*r)->type)
    {
      switch (TWO_TYPE ((*l)->type, (*r)->type))
	{
	case integer_real:
	  (*l) = (VECTOR *) cast_vector_integer_real ((VECTOR *) EAT (*l));
	  break;
	case integer_complex:
	  (*l) = (VECTOR *) cast_vector_integer_complex ((VECTOR *) EAT (*l));
	  break;
	case real_complex:
	  (*l) = (VECTOR *) cast_vector_real_complex ((VECTOR *) EAT (*l));
	  break;
	case integer_character:
	  (*l) = (VECTOR *) cast_vector_integer_character ((VECTOR *) EAT (*l));
	  break;
	case real_character:
	  (*l) = (VECTOR *) cast_vector_real_character ((VECTOR *) EAT (*l));
	  break;
	case complex_character:
	  (*l) = (VECTOR *) cast_vector_complex_character ((VECTOR *) EAT (*l));
	  break;
	case real_integer:
	  (*r) = (SCALAR *) cast_scalar_integer_real ((SCALAR *) EAT (*r));
	  break;
	case complex_integer:
	  (*r) = (SCALAR *) cast_scalar_integer_complex ((SCALAR *) EAT (*r));
	  break;
	case complex_real:
	  (*r) = (SCALAR *) cast_scalar_real_complex ((SCALAR *) EAT (*r));
	  break;
	case character_integer:
	  (*r) = (SCALAR *) cast_scalar_integer_character ((SCALAR *) EAT (*r));
	  break;
	case character_real:
	  (*r) = (SCALAR *) cast_scalar_real_character ((SCALAR *) EAT (*r));
	  break;
	case character_complex:
	  (*r) = (SCALAR *) cast_scalar_complex_character ((SCALAR *) EAT (*r));
	  break;
	default:
	  wipeout ("Bad type.");
	}
    }
}

void
auto_cast_matrix_scalar (MATRIX **l, SCALAR **r)
{
  if ((*l)->type != (*r)->type)
    {
      switch (TWO_TYPE ((*l)->type, (*r)->type))
	{
	case integer_real:
	  (*l) = (MATRIX *) cast_matrix_integer_real ((MATRIX *) EAT (*l));
	  break;
	case integer_complex:
	  (*l) = (MATRIX *) cast_matrix_integer_complex ((MATRIX *) EAT (*l));
	  break;
	case real_complex:
	  (*l) = (MATRIX *) cast_matrix_real_complex ((MATRIX *) EAT (*l));
	  break;
	case integer_character:
	  (*l) = (MATRIX *) cast_matrix_integer_character ((MATRIX *) EAT (*l));
	  break;
	case real_character:
	  (*l) = (MATRIX *) cast_matrix_real_character ((MATRIX *) EAT (*l));
	  break;
	case complex_character:
	  (*l) = (MATRIX *) cast_matrix_complex_character ((MATRIX *) EAT (*l));
	  break;
	case real_integer:
	  (*r) = (SCALAR *) cast_scalar_integer_real ((SCALAR *) EAT (*r));
	  break;
	case complex_integer:
	  (*r) = (SCALAR *) cast_scalar_integer_complex ((SCALAR *) EAT (*r));
	  break;
	case complex_real:
	  (*r) = (SCALAR *) cast_scalar_real_complex ((SCALAR *) EAT (*r));
	  break;
	case character_integer:
	  (*r) = (SCALAR *) cast_scalar_integer_character ((SCALAR *) EAT (*r));
	  break;
	case character_real:
	  (*r) = (SCALAR *) cast_scalar_real_character ((SCALAR *) EAT (*r));
	  break;
	case character_complex:
	  (*r) = (SCALAR *) cast_scalar_complex_character ((SCALAR *) EAT (*r));
	  break;
	default:
	  wipeout ("Bad type.");
	}
    }
}

void
auto_cast_matrix_vector (MATRIX **l, VECTOR **r)
{
  if ((*l)->type != (*r)->type)
    {
      switch (TWO_TYPE ((*l)->type, (*r)->type))
	{
	case integer_real:
	  (*l) = (MATRIX *) cast_matrix_integer_real ((MATRIX *) EAT (*l));
	  break;
	case integer_complex:
	  (*l) = (MATRIX *) cast_matrix_integer_complex ((MATRIX *) EAT (*l));
	  break;
	case real_complex:
	  (*l) = (MATRIX *) cast_matrix_real_complex ((MATRIX *) EAT (*l));
	  break;
	case integer_character:
	  (*l) = (MATRIX *) cast_matrix_integer_character ((MATRIX *) EAT (*l));
	  break;
	case real_character:
	  (*l) = (MATRIX *) cast_matrix_real_character ((MATRIX *) EAT (*l));
	  break;
	case complex_character:
	  (*l) = (MATRIX *) cast_matrix_complex_character ((MATRIX *) EAT (*l));
	  break;
	case real_integer:
	  (*r) = (VECTOR *) cast_vector_integer_real ((VECTOR *) EAT (*r));
	  break;
	case complex_integer:
	  (*r) = (VECTOR *) cast_vector_integer_complex ((VECTOR *) EAT (*r));
	  break;
	case complex_real:
	  (*r) = (VECTOR *) cast_vector_real_complex ((VECTOR *) EAT (*r));
	  break;
	case character_integer:
	  (*r) = (VECTOR *) cast_vector_integer_character ((VECTOR *) EAT (*r));
	  break;
	case character_real:
	  (*r) = (VECTOR *) cast_vector_real_character ((VECTOR *) EAT (*r));
	  break;
	case character_complex:
	  (*r) = (VECTOR *) cast_vector_complex_character ((VECTOR *) EAT (*r));
	  break;
	default:
	  wipeout ("Bad type.");
	}
    }
}

#endif /* STUPID_CPP */
