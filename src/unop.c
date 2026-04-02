/*
   unop.c -- Unary operators.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: unop.c,v 1.1.1.1 1996/04/17 05:56:23 ksh Exp $";

#include "unop.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "function.h"
#include "test.h"
#include "negate.h"
#include "transpose.h"
#include "not.h"

static char bad_unop[] = "Bad unop.";

ENTITY *
unop_entity (int op, ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      switch (op)
	{
	case UO_NEG:
	  return (negate_scalar ((SCALAR *) p));
	case UO_NOT:
	  return (not_scalar ((SCALAR *) p));
	case UO_TRANS:
	  return (hermitian_transpose_entity (p));
	default:
	  wipeout (bad_unop);
	}
    case vector:
      switch (op)
	{
	case UO_NEG:
	  return (negate_vector ((VECTOR *) p));
	case UO_NOT:
	  return (not_vector ((VECTOR *) p));
	case UO_TRANS:
	  return (hermitian_transpose_entity (p));
	default:
	  wipeout (bad_unop);
	}
    case matrix:
      switch (op)
	{
	case UO_NEG:
	  return (negate_matrix ((MATRIX *) p));
	case UO_NOT:
	  return (not_matrix ((MATRIX *) p));
	case UO_TRANS:
	  return (hermitian_transpose_matrix ((MATRIX *) p));
	default:
	  wipeout (bad_unop);
	}
    case table:
      return unop_table (op, (TABLE *) p);

    case function:
      switch (op)
	{
	case UO_NEG:
	  return (negate_entity (p));
	case UO_NOT:
	  /* Can't be NULL, if we got this far. */
	  return (int_to_scalar (0));
	case UO_TRANS:
	  return (hermitian_transpose_entity (p));
	default:
	  wipeout (bad_unop);
	}
      break;
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}
