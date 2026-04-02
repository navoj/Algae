/*
   apply.c -- Apply functions to entity elements.

   Copyright (C) 1994-2001  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: apply.c,v 1.5 2001/10/12 18:31:28 ksh Exp $";

#include "apply.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "cast.h"
#include "not.h"

ENTITY *
apply_vector_integer_integer (int PROTO ((*func), (int)), VECTOR *p)
{
  VECTOR *m;
  int i;

  EASSERT (p, vector, integer);

  m = (VECTOR *) dup_vector (p);

  WITH_HANDLING
  {
    errno = 0;
    for (i = 0; i < p->nn; i++)
      m->a.integer[i] = (*func) (m->a.integer[i]);
    CHECK_MATH ();
  }
  ON_EXCEPTION
  {
    delete_vector (m);
  }
  END_EXCEPTION;

  return (ENT (m));
}

ENTITY *
apply_vector_integer_real (int PROTO ((*func), (REAL)), VECTOR *p)
{
  VECTOR *m;
  int i;

  EASSERT (p, vector, real);

  m = (VECTOR *) notnot_vector ((VECTOR *) copy_vector (p));

  WITH_HANDLING
  {
    errno = 0;
    for (i = 0; i < p->nn; i++)
      m->a.integer[i] = (*func) (p->a.real[i]);
    CHECK_MATH ();
  }
  ON_EXCEPTION
  {
    delete_2_vectors (p, m);
  }
  END_EXCEPTION;

  delete_vector (p);
  return (ENT (m));
}

ENTITY *
apply_vector_integer_complex (int PROTO ((*func), (COMPLEX)), VECTOR *p)
{
  VECTOR *m;
  int i;

  EASSERT (p, vector, complex);

  m = (VECTOR *) notnot_vector ((VECTOR *) copy_vector (p));

  WITH_HANDLING
  {
    errno = 0;
    for (i = 0; i < p->nn; i++)
      m->a.integer[i] = (*func) (p->a.complex[i]);
    CHECK_MATH ();
  }
  ON_EXCEPTION
  {
    delete_2_vectors (p, m);
  }
  END_EXCEPTION;

  delete_vector (p);
  return (ENT (m));
}

ENTITY *
apply_vector_integer_character (int PROTO ((*func), (char *)), VECTOR *p)
{
  VECTOR *m;
  int i;

  EASSERT (p, vector, character);

  m = (VECTOR *) cast_vector_character_integer ((VECTOR *) copy_vector (p));

  WITH_HANDLING
  {
    errno = 0;
    for (i = 0; i < p->nn; i++)
      m->a.integer[i] = (*func) (p->a.character[i]);
    CHECK_MATH ();
  }
  ON_EXCEPTION
  {
    delete_2_vectors (p, m);
  }
  END_EXCEPTION;

  delete_vector (p);
  return (ENT (m));
}

ENTITY *
apply_vector_real_real (REAL PROTO ((*func), (REAL)), VECTOR *p)
{
  VECTOR *m;
  int i;

  EASSERT (p, vector, real);

  m = (VECTOR *) dup_vector (p);

  WITH_HANDLING
  {
    errno = 0;
    for (i = 0; i < p->nn; i++)
      m->a.real[i] = (*func) (m->a.real[i]);
    CHECK_MATH ();
  }
  ON_EXCEPTION
  {
    delete_vector (m);
  }
  END_EXCEPTION;

  return (ENT (m));
}

ENTITY *
apply_vector_real_complex (REAL PROTO ((*func), (COMPLEX)), VECTOR *p)
{
  VECTOR *m;
  int i;

  EASSERT (p, vector, complex);

  m = (VECTOR *) cast_vector_complex_real ((VECTOR *) copy_vector (p));

  WITH_HANDLING
  {
    errno = 0;
    for (i = 0; i < p->nn; i++)
      m->a.real[i] = (*func) (p->a.complex[i]);
    CHECK_MATH ();
  }
  ON_EXCEPTION
  {
    delete_2_vectors (p, m);
  }
  END_EXCEPTION;

  delete_vector (p);
  return (ENT (m));
}

ENTITY *
apply_vector_complex_complex (COMPLEX PROTO ((*func), (COMPLEX)), VECTOR *p)
{
  VECTOR *m;
  int i;

  EASSERT (p, vector, complex);

  m = (VECTOR *) dup_vector (p);

  WITH_HANDLING
  {
    errno = 0;
    for (i = 0; i < p->nn; i++)
      m->a.complex[i] = (*func) (m->a.complex[i]);
    CHECK_MATH ();
  }
  ON_EXCEPTION
  {
    delete_vector (m);
  }
  END_EXCEPTION;

  return (ENT (m));
}

ENTITY *
apply_vector_character_character (char * PROTO ((*func), (char *)), VECTOR *p)
{
  int i;

  EASSERT (p, vector, character);

  p = (VECTOR *) dup_vector (p);

  for (i=0; i<p->nn; i++)
    p->a.character[i] = (*func) (p->a.character[i]);

  return (ENT (p));
}

ENTITY *
apply_matrix_character_character (char * PROTO ((*func), (char *)), MATRIX *p)
{
  int i;

  EASSERT (p, matrix, character);

  p = (MATRIX *) dup_matrix (p);

  for (i=0; i<p->nn; i++)
    p->a.character[i] = (*func) (p->a.character[i]);
    
  if (p->d.character != NULL)
    {
      for (i=0; i<p->nr; i++)
        p->d.character[i] = (*func) (p->d.character[i]);
    }

  return (ENT (p));
}

ENTITY *
apply_matrix_integer_integer (int PROTO ((*func), (int)), MATRIX *p)
{
  MATRIX *m;
  int i;

  EASSERT (p, matrix, integer);

  m = (MATRIX *) dup_matrix (p);

  WITH_HANDLING
  {
    errno = 0;
    for (i = 0; i < p->nn; i++)
      m->a.integer[i] = (*func) (m->a.integer[i]);
    if (p->d.integer != NULL)
      {
	for (i = 0; i < p->nr; i++)
	  m->d.integer[i] = (*func) (m->d.integer[i]);
      }
    CHECK_MATH ();
  }
  ON_EXCEPTION
  {
    delete_matrix (m);
  }
  END_EXCEPTION;

  return (ENT (m));
}

ENTITY *
apply_matrix_integer_real (int PROTO ((*func), (REAL)), MATRIX *p)
{
  MATRIX *m;
  int i;

  EASSERT (p, matrix, real);

  m = (MATRIX *) notnot_matrix ((MATRIX *) copy_matrix (p));

  WITH_HANDLING
  {
    errno = 0;
    for (i = 0; i < p->nn; i++)
      m->a.integer[i] = (*func) (p->a.real[i]);
    if (p->d.integer != NULL)
      for (i = 0; i < p->nr; i++)
	m->d.integer[i] = (*func) (p->d.real[i]);
    CHECK_MATH ();
  }
  ON_EXCEPTION
  {
    delete_2_matrices (p, m);
  }
  END_EXCEPTION;

  delete_matrix (p);
  return (ENT (m));
}

ENTITY *
apply_matrix_integer_complex (int PROTO ((*func), (COMPLEX)), MATRIX *p)
{
  MATRIX *m;
  int i;

  EASSERT (p, matrix, complex);

  m = (MATRIX *) notnot_matrix ((MATRIX *) copy_matrix (p));

  WITH_HANDLING
  {
    errno = 0;
    for (i = 0; i < p->nn; i++)
      m->a.integer[i] = (*func) (p->a.complex[i]);
    if (p->d.integer != NULL)
      for (i = 0; i < p->nr; i++)
	m->d.integer[i] = (*func) (p->d.complex[i]);
    CHECK_MATH ();
  }
  ON_EXCEPTION
  {
    delete_2_matrices (p, m);
  }
  END_EXCEPTION;

  delete_matrix (p);
  return (ENT (m));
}

ENTITY *
apply_matrix_integer_character (int PROTO ((*func), (char *)), MATRIX *p)
{
  MATRIX *m;
  int i;

  EASSERT (p, matrix, character);

  m = (MATRIX *) cast_matrix_character_integer ((MATRIX *) copy_matrix (p));

  WITH_HANDLING
  {
    errno = 0;
    for (i = 0; i < p->nn; i++)
      m->a.integer[i] = (*func) (p->a.character[i]);
    if (p->d.integer != NULL)
      for (i = 0; i < p->nr; i++)
	m->d.integer[i] = (*func) (p->d.character[i]);
    CHECK_MATH ();
  }
  ON_EXCEPTION
  {
    delete_2_matrices (p, m);
  }
  END_EXCEPTION;

  delete_matrix (p);
  return (ENT (m));
}

ENTITY *
apply_matrix_real_real (REAL PROTO ((*func), (REAL)), MATRIX *p)
{
  MATRIX *m;
  int i;

  EASSERT (p, matrix, real);

  m = (MATRIX *) dup_matrix (p);

  WITH_HANDLING
  {
    errno = 0;
    for (i = 0; i < p->nn; i++)
      m->a.real[i] = (*func) (m->a.real[i]);
    if (p->d.real != NULL)
      {
	for (i = 0; i < p->nr; i++)
	  m->d.real[i] = (*func) (m->d.real[i]);
      }
    CHECK_MATH ();
  }
  ON_EXCEPTION
  {
    delete_matrix (m);
  }
  END_EXCEPTION;

  return (ENT (m));
}

ENTITY *
apply_matrix_real_complex (REAL PROTO ((*func), (COMPLEX)), MATRIX *p)
{
  MATRIX *m;
  int i;

  EASSERT (p, matrix, complex);

  m = (MATRIX *) cast_matrix_complex_real ((MATRIX *) copy_matrix (p));

  WITH_HANDLING
  {
    errno = 0;
    for (i = 0; i < p->nn; i++)
      m->a.real[i] = (*func) (p->a.complex[i]);
    if (p->d.real != NULL)
      for (i = 0; i < p->nr; i++)
	m->d.real[i] = (*func) (p->d.complex[i]);
    CHECK_MATH ();
  }
  ON_EXCEPTION
  {
    delete_2_matrices (p, m);
  }
  END_EXCEPTION;

  delete_matrix (p);
  return (ENT (m));
}

ENTITY *
apply_matrix_complex_complex (COMPLEX PROTO ((*func), (COMPLEX)), MATRIX *p)
{
  MATRIX *m;
  int i;

  EASSERT (p, matrix, complex);

  m = (MATRIX *) dup_matrix (p);

  WITH_HANDLING
  {
    errno = 0;
    for (i = 0; i < p->nn; i++)
      m->a.complex[i] = (*func) (m->a.complex[i]);
    if (p->d.complex != NULL)
      {
	for (i = 0; i < p->nr; i++)
	  m->d.complex[i] = (*func) (m->d.complex[i]);
	CHECK_MATH ();
      }
  }
  ON_EXCEPTION
  {
    delete_matrix (m);
  }
  END_EXCEPTION;

  return (ENT (m));
}
