/*
   generate.c -- Generate vectors.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: generate.c,v 1.3 2003/08/01 04:57:47 ksh Exp $";

#include "generate.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "cast.h"
#include "two_type.h"

static char neg_msg[] = "Generation increment is less than or equal to zero.\n";

ENTITY *
generate_entity (ENTITY *start, ENTITY *end, ENTITY *inc)
{
  ENTITY * volatile v;

  EASSERT (start, 0, 0);
  EASSERT (end, 0, 0);
  EASSERT (inc, 0, 0);

  WITH_HANDLING
  {

    /* Make sure they're scalars. */

    if (start->class != scalar)
      start = scalar_entity (EAT (start));
    if (end->class != scalar)
      end = scalar_entity (EAT (end));
    if (inc->class != scalar)
      inc = scalar_entity (EAT (inc));

    /* Convert to common type. */

    AUTO_CAST_SCALAR (start, end);
    AUTO_CAST_SCALAR (start, inc);
    AUTO_CAST_SCALAR (start, end);

    /* Now do the work. */
    switch (((SCALAR *) start)->type)
      {
      case integer:
	v = generate_integer (((SCALAR *) start)->v.integer,
			      ((SCALAR *) end)->v.integer,
			      ((SCALAR *) inc)->v.integer);
	break;
      case real:
	v = generate_real (((SCALAR *) start)->v.real,
			   ((SCALAR *) end)->v.real,
			   ((SCALAR *) inc)->v.real);
	break;
      case complex:
	v = generate_complex (((SCALAR *) start)->v.complex,
			      ((SCALAR *) end)->v.complex,
			      ((SCALAR *) inc)->v.complex);
	break;
      default:
	fail ("Invalid type for vector generation.\n");
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_3_entities (start, end, inc);
  }
  END_EXCEPTION;

  delete_3_entities (start, end, inc);
  return (v);
}

ENTITY *
generate_integer (int start, int end, int inc)
{
  VECTOR *v;
  int i, n;

  if (inc <= 0)
    {
      fail (neg_msg);
      raise_exception ();
    }

  n = abs (end - start) / inc + 1;
  if (end - start < 0)
    inc = -inc;

  v = (VECTOR *) form_vector (n, integer, dense);
  for (i = 0; i < n; i++)
    v->a.integer[i] = start + i * inc;

  return (ENT (v));
}

ENTITY *
generate_real (REAL start, REAL end, REAL inc)
{
  VECTOR *v;
  int i, n;

  if (inc <= 0)
    {
      fail (neg_msg);
      raise_exception ();
    }

  n = fabs (end - start) / inc + 1;
  if (end - start < 0)
    inc = -inc;

  v = (VECTOR *) form_vector (n, real, dense);
  for (i = 0; i < n; i++)
    v->a.real[i] = start + i * inc;

  return (ENT (v));
}

ENTITY *
generate_complex (COMPLEX start, COMPLEX end, COMPLEX inc)
{
  /*
   * For this complex case, the elements are generated along a line
   * between `start' and `end'.  The value of `inc' must be real, and
   * is taken as the distance between the generated points.
   */

  VECTOR *v;
  REAL dr, di, a;
  int i, n;

  if (inc.imag != 0)
    {
      fail ("Generation increment must be real.\n");
      raise_exception ();
    }
  if (inc.real <= 0)
    {
      fail (neg_msg);
      raise_exception ();
    }

  dr = end.real - start.real;
  di = end.imag - start.imag;
  n = sqrt (dr * dr + di * di) / inc.real + 1;
  a = (!di && !dr) ? 0.0 : atan2 (di, dr);
  inc.imag = inc.real * sin (a);
  inc.real = inc.real * cos (a);

  v = (VECTOR *) form_vector (n, complex, dense);
  for (i = 0; i < n; i++)
    {
      v->a.complex[i].real = start.real + i * inc.real;
      v->a.complex[i].imag = start.imag + i * inc.imag;
    }

  return (ENT (v));
}
