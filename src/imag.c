/*
   imag.c -- Imaginary part of a complex number.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: imag.c,v 1.1.1.1 1996/04/17 05:56:18 ksh Exp $";

#include "imag.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "apply.h"

REAL
imag_complex (COMPLEX z)
{
  return (z.imag);
}

ENTITY *
bi_imag (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (imag_scalar ((SCALAR *) p));
    case vector:
      return (imag_vector ((VECTOR *) p));
    case matrix:
      return (imag_matrix ((MATRIX *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
imag_scalar (SCALAR *p)
{
  EASSERT (p, scalar, 0);

  p = (SCALAR *) dup_scalar (p);

  switch (p->type)
    {

    case integer:

      p->type = real;
      p->v.real = 0.0;
      break;

    case real:

      p->v.real = 0.0;
      break;

    case complex:

      p->type = real;
      p->v.real = p->v.complex.imag;
      break;

    default:
      BAD_TYPE (p->type);
      delete_scalar (p);
      raise_exception ();
    }
  return (ENT (p));
}

ENTITY *
imag_vector (VECTOR *p)
{
  ENTITY *eid;
  int ne;

  EASSERT (p, vector, 0);

  switch (p->type)
    {

    case integer:
    case real:

      ne = p->ne;
      eid = (p->eid == NULL) ? NULL : copy_entity (p->eid);
      delete_vector (p);
      p = (VECTOR *) make_vector (ne, real, sparse);
      p->eid = eid;
      break;

    case complex:

      p = (VECTOR *) apply_vector_real_complex (imag_complex, p);
      break;

    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  return (ENT (p));
}

ENTITY *
imag_matrix (MATRIX *p)
{
  ENTITY *rid, *cid;
  int nr, nc;

  EASSERT (p, matrix, 0);

  switch (p->type)
    {

    case integer:
    case real:

      nr = p->nr;
      nc = p->nc;
      rid = (p->rid == NULL) ? NULL : copy_entity (p->rid);
      cid = (p->cid == NULL) ? NULL : copy_entity (p->cid);
      delete_matrix (p);
      p = (MATRIX *) make_matrix (nr, nc, real, sparse);
      p->rid = rid;
      p->cid = cid;
      break;

    case complex:

      p = (MATRIX *) apply_matrix_real_complex (imag_complex, p);
      break;

    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
  return (ENT (p));
}
