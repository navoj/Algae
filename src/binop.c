/*
   binop.c -- Binary operations on entities.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: binop.c,v 1.3 2003/08/01 04:57:46 ksh Exp $";

#include "binop.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "function.h"
#include "table.h"
#include "cast.h"
#include "add.h"
#include "subtract.h"
#include "multiply.h"
#include "divide.h"
#include "two_type.h"
#include "append.h"
#include "pile.h"
#include "logical.h"
#include "log.h"
#include "exp.h"
#include "not.h"
#include "power.h"
#include "product.h"
#include "mod.h"
#include "real.h"

static char invalid_binop[] = "Invalid binary operation.";
static char undefined_binop[] = "%s operator is undefined for %s operands.";

ENTITY *
binop_entity (int op, ENTITY *l, ENTITY *r)
{
  EASSERT (l, 0, 0);
  EASSERT (r, 0, 0);

  switch (TWO_CLASS (l, r))
    {
    case scalar_scalar:
      return (binop_scalar_scalar (op, (SCALAR *) l, (SCALAR *) r));
    case scalar_vector:
      return (binop_scalar_vector (op, (SCALAR *) l, (VECTOR *) r));
    case scalar_matrix:
      return (binop_scalar_matrix (op, (SCALAR *) l, (MATRIX *) r));

    case vector_scalar:
      return (binop_vector_scalar (op, (VECTOR *) l, (SCALAR *) r));
    case vector_vector:
      return (binop_vector_vector (op, (VECTOR *) l, (VECTOR *) r));
    case vector_matrix:
      return (binop_vector_matrix (op, (VECTOR *) l, (MATRIX *) r));

    case matrix_scalar:
      return (binop_matrix_scalar (op, (MATRIX *) l, (SCALAR *) r));
    case matrix_vector:
      return (binop_matrix_vector (op, (MATRIX *) l, (VECTOR *) r));
    case matrix_matrix:
      return (binop_matrix_matrix (op, (MATRIX *) l, (MATRIX *) r));

    case table_scalar:
    case table_vector:
    case table_matrix:
      return binop_table_other (op, (TABLE *) l, r);

    case scalar_table:
    case vector_table:
    case matrix_table:
      return binop_other_table (op, l, (TABLE *) r);

    case table_table:
      return binop_table_table (op, (TABLE *) l, (TABLE *) r);

    case function_function:
      return binop_function_function (op, (FUNCTION *) l, (FUNCTION *) r);

    default:
      fail ("Invalid binary operation between a %s and a %s.",
	    class_string[l->class], class_string[r->class]);
      delete_2_entities (l, r);
      raise_exception ();
    }
}

ENTITY *
binop_scalar_scalar (int op, SCALAR *l, SCALAR *r)
{
  /*
   * Perform a binary operation on two scalars, converting to a common type
   * if necessary.  Exception handling is not performed, so if an exception
   * occurs in the `cast_*' functions we may wind up leaving around an entity
   * that should have been freed.  I think this is better than spending the
   * time at execution to make the `setjmp' call.
   */

  EASSERT (l, scalar, 0);
  EASSERT (r, scalar, 0);

  switch (TWO_TYPE (l->type, r->type))
    {
    case integer_integer:
      return (binop_scalar_integer (op, l, r));
    case integer_real:
      return (binop_scalar_real (op, (SCALAR *) cast_scalar_integer_real (l), r));
    case integer_complex:
      return (binop_scalar_complex (op, (SCALAR *) cast_scalar_integer_complex (l), r));
    case integer_character:
      return (binop_scalar_integer_character (op, l, r));

    case real_integer:
      return (binop_scalar_real (op, l, (SCALAR *) cast_scalar_integer_real (r)));
    case real_real:
      return (binop_scalar_real (op, l, r));
    case real_complex:
      return (binop_scalar_complex (op, (SCALAR *) cast_scalar_real_complex (l), r));
    case real_character:
      return (binop_scalar_real_character (op, l, r));

    case complex_integer:
      return (binop_scalar_complex (op, l, (SCALAR *) cast_scalar_integer_complex (r)));
    case complex_real:
      return (binop_scalar_complex (op, l, (SCALAR *) cast_scalar_real_complex (r)));
    case complex_complex:
      return (binop_scalar_complex (op, l, r));
    case complex_character:
      return (binop_scalar_complex_character (op, l, r));

    case character_integer:
      return (binop_scalar_integer_character (op, r, l));	/* They're commutative. */
    case character_real:
      return (binop_scalar_real_character (op, r, l));
    case character_complex:
      return (binop_scalar_complex_character (op, r, l));
    case character_character:
      return (binop_scalar_character (op, l, r));

    default:
      wipeout (invalid_binop);
    }
}

ENTITY *
binop_scalar_vector (int op, SCALAR *l, VECTOR *r)
{
  /*
   * Binary operation between scalar `l' and vector `r'.  Here again,
   * exceptions are not handled.
   */

  EASSERT (l, scalar, 0);
  EASSERT (r, vector, 0);

  switch (TWO_TYPE (l->type, r->type))
    {
    case integer_integer:
      return (binop_scalar_vector_integer (op, l, r));
    case integer_real:
      return (binop_scalar_vector_real (op, (SCALAR *) cast_scalar_integer_real (l), r));
    case integer_complex:
      return (binop_scalar_vector_complex (op, (SCALAR *) cast_scalar_integer_complex (l), r));
    case integer_character:
      return (binop_scalar_vector_num_char (op, l, r));

    case real_integer:
      return (binop_scalar_vector_real (op, l, (VECTOR *) cast_vector_integer_real (r)));
    case real_real:
      return (binop_scalar_vector_real (op, l, r));
    case real_complex:
      return (binop_scalar_vector_complex (op, (SCALAR *) cast_scalar_real_complex (l), r));
    case real_character:
      return (binop_scalar_vector_num_char (op, l, r));

    case complex_integer:
      return (binop_scalar_vector_complex (op, l, (VECTOR *) cast_vector_integer_complex (r)));
    case complex_real:
      return (binop_scalar_vector_complex (op, l, (VECTOR *) cast_vector_real_complex (r)));
    case complex_complex:
      return (binop_scalar_vector_complex (op, l, r));
    case complex_character:
      return (binop_scalar_vector_num_char (op, l, r));

    case character_integer:	/* The only ops we have are commutative. */
    case character_real:
    case character_complex:
      return (binop_vector_scalar_num_char (op, r, l));
    case character_character:
      return (binop_scalar_vector_character (op, l, r));

    default:
      wipeout (invalid_binop);
    }
}

ENTITY *
binop_scalar_matrix (int op, SCALAR *l, MATRIX *r)
{
  /*
   * Binary operation between scalar `l' and matrix `r'.  Here again,
   * exceptions are not handled.
   */

  EASSERT (l, scalar, 0);
  EASSERT (r, matrix, 0);

  switch (TWO_TYPE (l->type, r->type))
    {
    case integer_integer:
      return (binop_scalar_matrix_integer (op, l, r));
    case integer_real:
      return (binop_scalar_matrix_real (op, (SCALAR *) cast_scalar_integer_real (l), r));
    case integer_complex:
      return (binop_scalar_matrix_complex (op, (SCALAR *) cast_scalar_integer_complex (l), r));
    case integer_character:
      return (binop_scalar_matrix_num_char (op, l, r));

    case real_integer:
      return (binop_scalar_matrix_real (op, l, (MATRIX *) cast_matrix_integer_real (r)));
    case real_real:
      return (binop_scalar_matrix_real (op, l, r));
    case real_complex:
      return (binop_scalar_matrix_complex (op, (SCALAR *) cast_scalar_real_complex (l), r));
    case real_character:
      return (binop_scalar_matrix_num_char (op, l, r));

    case complex_integer:
      return (binop_scalar_matrix_complex (op, l, (MATRIX *) cast_matrix_integer_complex (r)));
    case complex_real:
      return (binop_scalar_matrix_complex (op, l, (MATRIX *) cast_matrix_real_complex (r)));
    case complex_complex:
      return (binop_scalar_matrix_complex (op, l, r));
    case complex_character:
      return (binop_scalar_matrix_num_char (op, l, r));

    case character_integer:	/* The only ops we have are commutative. */
    case character_real:
    case character_complex:
      return (binop_matrix_scalar_num_char (op, r, l));
    case character_character:
      return (binop_scalar_matrix_character (op, l, r));

    default:
      wipeout (invalid_binop);
    }
}

ENTITY *
binop_vector_scalar (int op, VECTOR *l, SCALAR *r)
{
  /*
   * Binary operation between vector `l' and scalar `r'.  Here again,
   * exceptions are not handled.
   */

  EASSERT (l, vector, 0);
  EASSERT (r, scalar, 0);

  switch (TWO_TYPE (l->type, r->type))
    {
    case integer_integer:
      return (binop_vector_scalar_integer (op, l, r));
    case integer_real:
      return (binop_vector_scalar_real (op, (VECTOR *) cast_vector_integer_real (l), r));
    case integer_complex:
      return (binop_vector_scalar_complex (op, (VECTOR *) cast_vector_integer_complex (l), r));
    case integer_character:
      return (binop_vector_scalar_num_char (op, l, r));

    case real_integer:
      return (binop_vector_scalar_real (op, l, (SCALAR *) cast_scalar_integer_real (r)));
    case real_real:
      return (binop_vector_scalar_real (op, l, r));
    case real_complex:
      return (binop_vector_scalar_complex (op, (VECTOR *) cast_vector_real_complex (l), r));
    case real_character:
      return (binop_vector_scalar_num_char (op, l, r));

    case complex_integer:
      return (binop_vector_scalar_complex (op, l, (SCALAR *) cast_scalar_integer_complex (r)));
    case complex_real:
      return (binop_vector_scalar_complex (op, l, (SCALAR *) cast_scalar_real_complex (r)));
    case complex_complex:
      return (binop_vector_scalar_complex (op, l, r));
    case complex_character:
      return (binop_vector_scalar_num_char (op, l, r));

    case character_integer:	/* The only ops we have are commutative. */
    case character_real:
    case character_complex:
      return (binop_scalar_vector_num_char (op, r, l));
    case character_character:
      return (binop_vector_scalar_character (op, l, r));

    default:
      wipeout (invalid_binop);
    }
}

ENTITY *
binop_vector_vector (int op, VECTOR *l, VECTOR *r)
{
  /*
   * Binary operation between vector `l' and vector `r'.  Here again,
   * exceptions are not handled.
   */

  EASSERT (l, vector, 0);
  EASSERT (r, vector, 0);

  switch (TWO_TYPE (l->type, r->type))
    {
    case integer_integer:
      return (binop_vector_integer (op, l, r));
    case integer_real:
      return (binop_vector_real (op, (VECTOR *) cast_vector_integer_real (l), r));
    case integer_complex:
      return (binop_vector_complex (op, (VECTOR *) cast_vector_integer_complex (l), r));
    case integer_character:
      return (binop_vector_integer_character (op, l, r));

    case real_integer:
      return (binop_vector_real (op, l, (VECTOR *) cast_vector_integer_real (r)));
    case real_real:
      return (binop_vector_real (op, l, r));
    case real_complex:
      return (binop_vector_complex (op, (VECTOR *) cast_vector_real_complex (l), r));
    case real_character:
      return (binop_vector_real_character (op, l, r));

    case complex_integer:
      return (binop_vector_complex (op, l, (VECTOR *) cast_vector_integer_complex (r)));
    case complex_real:
      return (binop_vector_complex (op, l, (VECTOR *) cast_vector_real_complex (r)));
    case complex_complex:
      return (binop_vector_complex (op, l, r));
    case complex_character:
      return (binop_vector_complex_character (op, l, r));

    case character_integer:	/* They're all commutative. */
      return (binop_vector_integer_character (op, r, l));
    case character_real:
      return (binop_vector_real_character (op, r, l));
    case character_complex:
      return (binop_vector_complex_character (op, r, l));
    case character_character:
      return (binop_vector_character (op, l, r));

    default:
      wipeout (invalid_binop);
    }
}

ENTITY *
binop_vector_matrix (int op, VECTOR *l, MATRIX *r)
{
  /*
   * Binary operation between vector `l' and matrix `r'.  Here again,
   * exceptions are not handled.
   */

  EASSERT (l, vector, 0);
  EASSERT (r, matrix, 0);

  switch (TWO_TYPE (l->type, r->type))
    {
    case integer_integer:
      return (binop_vector_matrix_integer (op, l, r));
    case integer_real:
      return (binop_vector_matrix_real (op, (VECTOR *) cast_vector_integer_real (l), r));
    case integer_complex:
      return (binop_vector_matrix_complex (op, (VECTOR *) cast_vector_integer_complex (l), r));
    case integer_character:
      return (binop_vector_matrix_num_char (op, l, r));

    case real_integer:
      return (binop_vector_matrix_real (op, l, (MATRIX *) cast_matrix_integer_real (r)));
    case real_real:
      return (binop_vector_matrix_real (op, l, r));
    case real_complex:
      return (binop_vector_matrix_complex (op, (VECTOR *) cast_vector_real_complex (l), r));
    case real_character:
      return (binop_vector_matrix_num_char (op, l, r));

    case complex_integer:
      return (binop_vector_matrix_complex (op, l, (MATRIX *) cast_matrix_integer_complex (r)));
    case complex_real:
      return (binop_vector_matrix_complex (op, l, (MATRIX *) cast_matrix_real_complex (r)));
    case complex_complex:
      return (binop_vector_matrix_complex (op, l, r));
    case complex_character:
      return (binop_vector_matrix_num_char (op, l, r));

    case character_integer:
    case character_real:
    case character_complex:
      return (binop_vector_matrix_char_num (op, l, r));
    case character_character:
      return (binop_vector_matrix_character (op, l, r));

    default:
      wipeout (invalid_binop);
    }
}

ENTITY *
binop_matrix_scalar (int op, MATRIX *l, SCALAR *r)
{
  /*
   * Binary operation between matrix `l' and scalar `r'.  Here again,
   * exceptions are not handled.
   */

  EASSERT (l, matrix, 0);
  EASSERT (r, scalar, 0);

  switch (TWO_TYPE (l->type, r->type))
    {
    case integer_integer:
      return (binop_matrix_scalar_integer (op, l, r));
    case integer_real:
      return (binop_matrix_scalar_real (op, (MATRIX *) cast_matrix_integer_real (l), r));
    case integer_complex:
      return (binop_matrix_scalar_complex (op, (MATRIX *) cast_matrix_integer_complex (l), r));
    case integer_character:
      return (binop_matrix_scalar_num_char (op, l, r));

    case real_integer:
      return (binop_matrix_scalar_real (op, l, (SCALAR *) cast_scalar_integer_real (r)));
    case real_real:
      return (binop_matrix_scalar_real (op, l, r));
    case real_complex:
      return (binop_matrix_scalar_complex (op, (MATRIX *) cast_matrix_real_complex (l), r));
    case real_character:
      return (binop_matrix_scalar_num_char (op, l, r));

    case complex_integer:
      return (binop_matrix_scalar_complex (op, l, (SCALAR *) cast_scalar_integer_complex (r)));
    case complex_real:
      return (binop_matrix_scalar_complex (op, l, (SCALAR *) cast_scalar_real_complex (r)));
    case complex_complex:
      return (binop_matrix_scalar_complex (op, l, r));
    case complex_character:
      return (binop_matrix_scalar_num_char (op, l, r));

    case character_integer:	/* The only ops we have are commutative. */
    case character_real:
    case character_complex:
      return (binop_scalar_matrix_num_char (op, r, l));
    case character_character:
      return (binop_matrix_scalar_character (op, l, r));

    default:
      wipeout (invalid_binop);
    }
}

ENTITY *
binop_matrix_vector (int op, MATRIX *l, VECTOR *r)
{
  /*
   * Binary operation between matrix `l' and vector `r'.  Here again,
   * exceptions are not handled.
   */

  EASSERT (l, matrix, 0);
  EASSERT (r, vector, 0);

  switch (TWO_TYPE (l->type, r->type))
    {
    case integer_integer:
      return (binop_matrix_vector_integer (op, l, r));
    case integer_real:
      return (binop_matrix_vector_real (op, (MATRIX *) cast_matrix_integer_real (l), r));
    case integer_complex:
      return (binop_matrix_vector_complex (op, (MATRIX *) cast_matrix_integer_complex (l), r));
    case integer_character:
      return (binop_matrix_vector_num_char (op, l, r));

    case real_integer:
      return (binop_matrix_vector_real (op, l, (VECTOR *) cast_vector_integer_real (r)));
    case real_real:
      return (binop_matrix_vector_real (op, l, r));
    case real_complex:
      return (binop_matrix_vector_complex (op, (MATRIX *) cast_matrix_real_complex (l), r));
    case real_character:
      return (binop_matrix_vector_num_char (op, l, r));

    case complex_integer:
      return (binop_matrix_vector_complex (op, l, (VECTOR *) cast_vector_integer_complex (r)));
    case complex_real:
      return (binop_matrix_vector_complex (op, l, (VECTOR *) cast_vector_real_complex (r)));
    case complex_complex:
      return (binop_matrix_vector_complex (op, l, r));
    case complex_character:
      return (binop_matrix_vector_num_char (op, l, r));

    case character_integer:
    case character_real:
    case character_complex:
      return (binop_matrix_vector_char_num (op, l, r));
    case character_character:
      return (binop_matrix_vector_character (op, l, r));

    default:
      wipeout (invalid_binop);
    }
}

ENTITY *
binop_matrix_matrix (int op, MATRIX *l, MATRIX *r)
{
  /*
   * Binary operation between matrix `l' and matrix `r'.  Here again,
   * exceptions are not handled.
   */

  EASSERT (l, matrix, 0);
  EASSERT (r, matrix, 0);

  switch (TWO_TYPE (l->type, r->type))
    {
    case integer_integer:
      return (binop_matrix_integer (op, l, r));
    case integer_real:
      return (binop_matrix_real (op, (MATRIX *) cast_matrix_integer_real (l), r));
    case integer_complex:
      return (binop_matrix_complex (op, (MATRIX *) cast_matrix_integer_complex (l), r));
    case integer_character:
      return (binop_matrix_num_char (op, l, r));

    case real_integer:
      return (binop_matrix_real (op, l, (MATRIX *) cast_matrix_integer_real (r)));
    case real_real:
      return (binop_matrix_real (op, l, r));
    case real_complex:
      return (binop_matrix_complex (op, (MATRIX *) cast_matrix_real_complex (l), r));
    case real_character:
      return (binop_matrix_num_char (op, l, r));

    case complex_integer:
      return (binop_matrix_complex (op, l, (MATRIX *) cast_matrix_integer_complex (r)));
    case complex_real:
      return (binop_matrix_complex (op, l, (MATRIX *) cast_matrix_real_complex (r)));
    case complex_complex:
      return (binop_matrix_complex (op, l, r));
    case complex_character:
      return (binop_matrix_num_char (op, l, r));

    case character_integer:	/* They're commutative. */
    case character_real:
    case character_complex:
      return (binop_matrix_num_char (op, r, l));
    case character_character:
      return (binop_matrix_character (op, l, r));

    default:
      wipeout (invalid_binop);
    }
}

ENTITY *
binop_table_table (int op, TABLE *l, TABLE *r)
{
  EASSERT (l, table, 0);
  EASSERT (r, table, 0);

  switch (op)
    {
    case BO_ADD:
      return (add_table (l, r));
    case BO_SUB:
      return (subtract_table (l, r));
    default:
      fail (invalid_binop);
      delete_2_tables (l, r);
      raise_exception ();
    }
}

ENTITY *
binop_function_function (int op, FUNCTION *l, FUNCTION *r)
{
  int i;

  switch (op)
    {
    case BO_EQ:
      i = l->funct_p == r->funct_p;
      break;
    case BO_NE:
      i = l->funct_p != r->funct_p;
      break;
    default:
      fail (invalid_binop);
      delete_2_functions (l, r);
      raise_exception ();
    }
  delete_2_functions (l, r);
  return (int_to_scalar (i));
}

/*==========================================================
  binop_scalar_type
  ==========================================================*/

ENTITY *
binop_scalar_integer (int op, SCALAR *l, SCALAR *r)
{
  ENTITY *x = NULL;
  div_t d;

  EASSERT (l, scalar, integer);
  EASSERT (r, scalar, integer);

  /*
   * A math error might occur here, so I'd normally put in a
   * WITH_HANDLING block to catch it.  Since we're just dealing
   * with scalars, I'll skip it and save the time it takes to
   * do the setjmp's.  If an exception does occur, we'll wind
   * up with a little memory loss.
   */

  switch (op)
    {
    case BO_ADD:
      x = dup_scalar ((SCALAR *) EAT (l));
      ((SCALAR *) x)->v.integer += r->v.integer;
      break;
    case BO_SUB:
      x = dup_scalar ((SCALAR *) EAT (l));
      ((SCALAR *) x)->v.integer -= r->v.integer;
      break;
    case BO_MUL:
    case BO_PROD:
      x = dup_scalar ((SCALAR *) EAT (l));
      ((SCALAR *) x)->v.integer *= r->v.integer;
      break;
    case BO_DIV:
      if (!r->v.integer)
	{
	  fail ("Integer divide by zero.");
	  delete_2_scalars (l, r);
	  raise_exception ();
	}
      d = div (l->v.integer, r->v.integer);
      x = (d.rem == 0) ? int_to_scalar (d.quot) :
	real_to_scalar ((REAL) l->v.integer / (REAL) r->v.integer);
      break;

    case BO_POWER:

      if (r->v.integer < 0)
	{
	  if (l->v.integer == 0)
	    {
	      fail ("Invalid power operation (1/0).");
	      delete_2_scalars (l, r);
	      raise_exception ();
	    }

	  x = real_to_scalar (pow ((REAL) l->v.integer,
				   (REAL) r->v.integer));
	}
      else
	{
	  if (l->v.integer == 0 && r->v.integer == 0)
	    {
	      fail ("Invalid power operation (0^0).");
	      delete_2_scalars (l, r);
	      raise_exception ();
	    }

	  x = int_to_scalar ((int) round (pow ((REAL) l->v.integer,
					       (REAL) r->v.integer)));
	}
      break;

    case BO_APPEND:
      x = form_vector (2, integer, dense);
      ((VECTOR *) x)->a.integer[0] = l->v.integer;
      ((VECTOR *) x)->a.integer[1] = r->v.integer;
      break;
    case BO_PILE:
      x = form_matrix (2, 1, integer, dense);
      ((MATRIX *) x)->a.integer[0] = l->v.integer;
      ((MATRIX *) x)->a.integer[1] = r->v.integer;
      break;
    case BO_MOD:
      x = int_to_scalar (l->v.integer % r->v.integer);
      break;
    case BO_AND:
      x = int_to_scalar (l->v.integer && r->v.integer);
      break;
    case BO_OR:
      x = int_to_scalar (l->v.integer || r->v.integer);
      break;
    case BO_EQ:
      x = int_to_scalar (l->v.integer == r->v.integer);
      break;
    case BO_NE:
      x = int_to_scalar (l->v.integer != r->v.integer);
      break;
    case BO_LT:
      x = int_to_scalar (l->v.integer < r->v.integer);
      break;
    case BO_LTE:
      x = int_to_scalar (l->v.integer <= r->v.integer);
      break;
    case BO_GT:
      x = int_to_scalar (l->v.integer > r->v.integer);
      break;
    case BO_GTE:
      x = int_to_scalar (l->v.integer >= r->v.integer);
      break;
    default:
      fail (invalid_binop);
      delete_2_scalars (l, r);
      delete_entity (x);
      raise_exception ();
    }

  delete_2_scalars (l, r);
  return (x);
}

ENTITY *
binop_scalar_real (int op, SCALAR *l, SCALAR *r)
{
  ENTITY *x = NULL;

  EASSERT (l, scalar, real);
  EASSERT (r, scalar, real);

  /*
   * A math error might occur here, so I'd normally put in a
   * WITH_HANDLING block to catch it.  Since we're just dealing
   * with scalars, I'll skip it and save the time it takes to
   * do the setjmp's.  If an exception does occur, we'll wind
   * up with a little memory loss.
   */

  switch (op)
    {
    case BO_ADD:
      x = dup_scalar ((SCALAR *) EAT (l));
      ((SCALAR *) x)->v.real += r->v.real;
      break;
    case BO_SUB:
      x = dup_scalar ((SCALAR *) EAT (l));
      ((SCALAR *) x)->v.real -= r->v.real;
      break;
    case BO_MUL:
    case BO_PROD:
      x = dup_scalar ((SCALAR *) EAT (l));
      ((SCALAR *) x)->v.real *= r->v.real;
      break;
    case BO_DIV:
      x = dup_scalar ((SCALAR *) EAT (l));
      ((SCALAR *) x)->v.real /= r->v.real;
      break;

    case BO_POWER:

      if (l->v.real < 0 && ((int) r->v.real) != r->v.real)
	{
	  COMPLEX a, b;
	  x = make_scalar (complex);
	  a.real = l->v.real;
	  b.real = r->v.real;
	  a.imag = b.imag = 0.0;
	  ((SCALAR *) x)->v.complex = power_complex (a, b);
	}
      else
	{
	  if (l->v.real == 0.0 && r->v.real <= 0.0)
	    {
	      if (r->v.real == 0.0)
		fail ("Invalid power operation (1/0).");
	      else
		fail ("Invalid power operation (0^0).");
	      delete_2_scalars (l, r);
	      raise_exception ();
	    }
	  x = dup_scalar ((SCALAR *) EAT (l));
	  ((SCALAR *) x)->v.real = pow (((SCALAR *) x)->v.real, r->v.real);
	}
      break;

    case BO_APPEND:
      x = form_vector (2, real, dense);
      ((VECTOR *) x)->a.real[0] = l->v.real;
      ((VECTOR *) x)->a.real[1] = r->v.real;
      break;
    case BO_PILE:
      x = form_matrix (2, 1, real, dense);
      ((MATRIX *) x)->a.real[0] = l->v.real;
      ((MATRIX *) x)->a.real[1] = r->v.real;
      break;
    case BO_MOD:
      x = dup_scalar (EAT (l));
      ((SCALAR *) x)->v.real = fmod (((SCALAR *) x)->v.real, r->v.real);
      break;
    case BO_AND:
      x = int_to_scalar (l->v.real && r->v.real);
      break;
    case BO_OR:
      x = int_to_scalar (l->v.real || r->v.real);
      break;
    case BO_EQ:
      x = int_to_scalar (l->v.real == r->v.real);
      break;
    case BO_NE:
      x = int_to_scalar (l->v.real != r->v.real);
      break;
    case BO_LT:
      x = int_to_scalar (l->v.real < r->v.real);
      break;
    case BO_LTE:
      x = int_to_scalar (l->v.real <= r->v.real);
      break;
    case BO_GT:
      x = int_to_scalar (l->v.real > r->v.real);
      break;
    case BO_GTE:
      x = int_to_scalar (l->v.real >= r->v.real);
      break;
    default:
      fail (invalid_binop);
      delete_2_scalars (l, r);
      delete_entity (x);
      raise_exception ();
    }

  delete_2_scalars (l, r);
  return (x);
}

ENTITY *
binop_scalar_complex (int op, SCALAR *l, SCALAR *r)
{
  ENTITY *x = NULL;

  EASSERT (l, scalar, complex);
  EASSERT (r, scalar, complex);

  /*
   * A math error might occur here, so I'd normally put in a
   * WITH_HANDLING block to catch it.  Since we're just dealing
   * with scalars, I'll skip it and save the time it takes to
   * do the setjmp's.  If an exception does occur, we'll wind
   * up with a little memory loss.
   */

  switch (op)
    {
    case BO_ADD:
      x = dup_scalar ((SCALAR *) EAT (l));
      ((SCALAR *) x)->v.complex.real += r->v.complex.real;
      ((SCALAR *) x)->v.complex.imag += r->v.complex.imag;
      break;
    case BO_SUB:
      x = dup_scalar ((SCALAR *) EAT (l));
      ((SCALAR *) x)->v.complex.real -= r->v.complex.real;
      ((SCALAR *) x)->v.complex.imag -= r->v.complex.imag;
      break;
    case BO_MUL:
    case BO_PROD:
      x = dup_scalar ((SCALAR *) copy_scalar (l));
      ((SCALAR *) x)->v.complex.real = l->v.complex.real * r->v.complex.real -
	l->v.complex.imag * r->v.complex.imag;
      ((SCALAR *) x)->v.complex.imag = l->v.complex.real * r->v.complex.imag +
	l->v.complex.imag * r->v.complex.real;
      break;

    case BO_DIV:

      x = dup_scalar ((SCALAR *) copy_scalar (l));
      ((SCALAR *) x)->v.complex = divide_complex (((SCALAR *) l)->v.complex,
						  ((SCALAR *) r)->v.complex);
      break;

    case BO_POWER:

      if (!(l->v.complex.real || l->v.complex.imag))
	{
	  fail ("Undefined complex power operation (0^z).");
	  delete_2_scalars (l, r);
	  raise_exception ();
	}
      x = dup_scalar (EAT (l));
      ((SCALAR *) x)->v.complex =
	power_complex (((SCALAR *) x)->v.complex, r->v.complex);
      break;

    case BO_APPEND:
      x = form_vector (2, complex, dense);
      ((VECTOR *) x)->a.complex[0].real = l->v.complex.real;
      ((VECTOR *) x)->a.complex[0].imag = l->v.complex.imag;
      ((VECTOR *) x)->a.complex[1].real = r->v.complex.real;
      ((VECTOR *) x)->a.complex[1].imag = r->v.complex.imag;
      break;
    case BO_PILE:
      x = form_matrix (2, 1, complex, dense);
      ((MATRIX *) x)->a.complex[0].real = l->v.complex.real;
      ((MATRIX *) x)->a.complex[0].imag = l->v.complex.imag;
      ((MATRIX *) x)->a.complex[1].real = r->v.complex.real;
      ((MATRIX *) x)->a.complex[1].imag = r->v.complex.imag;
      break;
    case BO_MOD:
      fail (undefined_binop, "Modulus", "complex");
      raise_exception ();
    case BO_AND:
      x = int_to_scalar ((l->v.complex.real || l->v.complex.imag) &&
			 (r->v.complex.real || r->v.complex.imag));
      break;
    case BO_OR:
      x = int_to_scalar (l->v.complex.real || r->v.complex.real ||
			 l->v.complex.imag || r->v.complex.imag);
      break;
    case BO_EQ:
      x = int_to_scalar (l->v.complex.real == r->v.complex.real &&
			 l->v.complex.imag == r->v.complex.imag);
      break;
    case BO_NE:
      x = int_to_scalar (l->v.complex.real != r->v.complex.real ||
			 l->v.complex.imag != r->v.complex.imag);
      break;
    case BO_LT:
    case BO_LTE:
    case BO_GT:
    case BO_GTE:
      fail (undefined_binop, "Comparison", type_string[complex]);
      delete_2_scalars (l, r);
      delete_entity (x);
      raise_exception ();
    default:
      fail (invalid_binop);
      delete_2_scalars (l, r);
      delete_entity (x);
      raise_exception ();
    }

  delete_2_scalars (l, r);
  return (x);
}

ENTITY *
binop_scalar_character (int op, SCALAR *l, SCALAR *r)
{
  ENTITY * volatile x = NULL;
  char *p;

  EASSERT (l, scalar, character);
  EASSERT (r, scalar, character);

  WITH_HANDLING
  {
    switch (op)
      {
      case BO_ADD:
	if (!*l->v.character && !*r->v.character)
	  {
	    x = copy_scalar (l);
	  }
	else
	  {
	    p = (char *) MALLOC (strlen (l->v.character) +
				 strlen (r->v.character) + 1);
	    x = char_to_scalar (strcat (strcpy (p, l->v.character),
					r->v.character));
	  }
	break;
      case BO_APPEND:
	x = form_vector (2, character, dense);
	((VECTOR *) x)->a.character[0] = dup_char (l->v.character);
	((VECTOR *) x)->a.character[1] = dup_char (r->v.character);
	break;
      case BO_PILE:
	x = form_matrix (2, 1, character, dense);
	((MATRIX *) x)->a.character[0] = dup_char (l->v.character);
	((MATRIX *) x)->a.character[1] = dup_char (r->v.character);
	break;
      case BO_AND:
	x = int_to_scalar (*l->v.character && *r->v.character);
	break;
      case BO_OR:
	x = int_to_scalar (*l->v.character || *r->v.character);
	break;
      case BO_EQ:
	x = int_to_scalar (!strcmp (l->v.character, r->v.character));
	break;
      case BO_NE:
	x = int_to_scalar (strcmp (l->v.character, r->v.character) != 0);
	break;
      case BO_LT:
	x = int_to_scalar (strcmp (l->v.character, r->v.character) < 0);
	break;
      case BO_LTE:
	x = int_to_scalar (strcmp (l->v.character, r->v.character) <= 0);
	break;
      case BO_GT:
	x = int_to_scalar (strcmp (l->v.character, r->v.character) > 0);
	break;
      case BO_GTE:
	x = int_to_scalar (strcmp (l->v.character, r->v.character) >= 0);
	break;
      case BO_MOD:
      case BO_PROD:
      case BO_SUB:
      case BO_MUL:
      case BO_DIV:
      case BO_POWER:
      default:
	fail (invalid_binop);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_2_scalars (l, r);
    delete_entity (x);
  }
  END_EXCEPTION;

  delete_2_scalars (l, r);
  return (x);
}

ENTITY *
binop_scalar_integer_character (int op, SCALAR *l, SCALAR *r)
{
  int i;

  switch (op)
    {
    case BO_AND:
      i = l->v.integer && *r->v.character;
      break;
    case BO_OR:
      i = l->v.integer || *r->v.character;
      break;
    default:
      fail (invalid_binop);
      delete_2_scalars (l, r);
      raise_exception ();
    }
  delete_2_scalars (l, r);
  return (int_to_scalar (i));
}

ENTITY *
binop_scalar_real_character (int op, SCALAR *l, SCALAR *r)
{
  int i;

  switch (op)
    {
    case BO_AND:
      i = l->v.real && *r->v.character;
      break;
    case BO_OR:
      i = l->v.real || *r->v.character;
      break;
    default:
      fail (invalid_binop);
      delete_2_scalars (l, r);
      raise_exception ();
    }
  delete_2_scalars (l, r);
  return (int_to_scalar (i));
}

ENTITY *
binop_scalar_complex_character (int op, SCALAR *l, SCALAR *r)
{
  int i;

  switch (op)
    {
    case BO_AND:
      i = (l->v.complex.real || l->v.complex.imag) && *r->v.character;
      break;
    case BO_OR:
      i = l->v.complex.real || l->v.complex.imag || *r->v.character;
      break;
    default:
      fail (invalid_binop);
      delete_2_scalars (l, r);
      raise_exception ();
    }
  delete_2_scalars (l, r);
  return (int_to_scalar (i));
}

/*=============================================
  binop_scalar_vector_type
  *=============================================*/


ENTITY *
binop_scalar_vector_integer (int op, SCALAR *l, VECTOR *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_scalar_vector_integer (l, r));
    case BO_SUB:
      return (subtract_scalar_vector_integer (l, r));
    case BO_MUL:
    case BO_PROD:
      return (multiply_scalar_vector_integer (l, r));
    case BO_DIV:
      return (divide_scalar_vector_integer (l, r));
    case BO_APPEND:
      return (append_scalar_vector_integer (l, r));
    case BO_PILE:
      return (pile_scalar_vector_integer (l, r));
    case BO_AND:
      return (logical_scalar_vector_integer (l, r, and_integer));
    case BO_OR:
      return (logical_scalar_vector_integer (l, r, or_integer));
    case BO_EQ:
      return (logical_scalar_vector_integer (l, r, eq_integer));
    case BO_NE:
      return (logical_scalar_vector_integer (l, r, ne_integer));
    case BO_LT:
      return (logical_scalar_vector_integer (l, r, lt_integer));
    case BO_LTE:
      return (logical_scalar_vector_integer (l, r, lte_integer));
    case BO_GT:
      return (logical_scalar_vector_integer (l, r, gt_integer));
    case BO_GTE:
      return (logical_scalar_vector_integer (l, r, gte_integer));
    case BO_MOD:
      return (mod_scalar_vector_integer (l, r));
    case BO_POWER:
      return (power_scalar_vector_integer (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_scalar (l);
  delete_vector (r);
  raise_exception ();
}

ENTITY *
binop_scalar_vector_real (int op, SCALAR *l, VECTOR *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_scalar_vector_real (l, r));
    case BO_SUB:
      return (subtract_scalar_vector_real (l, r));
    case BO_MUL:
    case BO_PROD:
      return (multiply_scalar_vector_real (l, r));
    case BO_DIV:
      return (divide_scalar_vector_real (l, r));
    case BO_APPEND:
      return (append_scalar_vector_real (l, r));
    case BO_PILE:
      return (pile_scalar_vector_real (l, r));
    case BO_AND:
      return (logical_scalar_vector_real (l, r, and_real));
    case BO_OR:
      return (logical_scalar_vector_real (l, r, or_real));
    case BO_EQ:
      return (logical_scalar_vector_real (l, r, eq_real));
    case BO_NE:
      return (logical_scalar_vector_real (l, r, ne_real));
    case BO_LT:
      return (logical_scalar_vector_real (l, r, lt_real));
    case BO_LTE:
      return (logical_scalar_vector_real (l, r, lte_real));
    case BO_GT:
      return (logical_scalar_vector_real (l, r, gt_real));
    case BO_GTE:
      return (logical_scalar_vector_real (l, r, gte_real));
    case BO_MOD:
      return (mod_scalar_vector_real (l, r));
    case BO_POWER:
      return (power_scalar_vector_real (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_scalar (l);
  delete_vector (r);
  raise_exception ();
}

ENTITY *
binop_scalar_vector_complex (int op, SCALAR *l, VECTOR *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_scalar_vector_complex (l, r));
    case BO_SUB:
      return (subtract_scalar_vector_complex (l, r));
    case BO_MUL:
    case BO_PROD:
      return (multiply_scalar_vector_complex (l, r));
    case BO_DIV:
      return (divide_scalar_vector_complex (l, r));
    case BO_APPEND:
      return (append_scalar_vector_complex (l, r));
    case BO_AND:
      return (logical_scalar_vector_complex (l, r, and_complex));
    case BO_OR:
      return (logical_scalar_vector_complex (l, r, or_complex));
    case BO_EQ:
      return (logical_scalar_vector_complex (l, r, eq_complex));
    case BO_NE:
      return (logical_scalar_vector_complex (l, r, ne_complex));
    case BO_GT:
    case BO_GTE:
    case BO_LT:
    case BO_LTE:
      fail (undefined_binop, "Comparison", type_string[complex]);
      break;
    case BO_MOD:
      fail (undefined_binop, "Modulus", type_string[complex]);
      break;
    case BO_POWER:
      return (power_scalar_vector_complex (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_scalar (l);
  delete_vector (r);
  raise_exception ();
}

ENTITY *
binop_scalar_vector_character (int op, SCALAR *l, VECTOR *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_scalar_vector_character (l, r));
    case BO_APPEND:
      return (append_scalar_vector_character (l, r));
    case BO_PILE:
      return (pile_scalar_vector_character (l, r));
    case BO_AND:
      return (logical_scalar_vector_character (l, r, and_character));
    case BO_OR:
      return (logical_scalar_vector_character (l, r, or_character));
    case BO_EQ:
      return (logical_scalar_vector_character (l, r, eq_character));
    case BO_NE:
      return (logical_scalar_vector_character (l, r, ne_character));
    case BO_LT:
      return (logical_scalar_vector_character (l, r, lt_character));
    case BO_LTE:
      return (logical_scalar_vector_character (l, r, lte_character));
    case BO_GT:
      return (logical_scalar_vector_character (l, r, gt_character));
    case BO_GTE:
      return (logical_scalar_vector_character (l, r, gte_character));
    case BO_SUB:
    case BO_MUL:
    case BO_PROD:
    case BO_DIV:
    case BO_MOD:
    case BO_POWER:
      fail (undefined_binop, "This", type_string[character]);
      break;
    default:
      fail (invalid_binop);
    }
  delete_scalar (l);
  delete_vector (r);
  raise_exception ();
}

ENTITY *
binop_scalar_vector_num_char (int op, SCALAR *l, VECTOR *r)
{
  switch (op)
    {
    case BO_AND:
      return (logical_scalar_vector_integer ((SCALAR *) notnot_scalar (l),
					     (VECTOR *) notnot_vector (r),
					     and_integer));
    case BO_OR:
      return (logical_scalar_vector_integer ((SCALAR *) notnot_scalar (l),
					     (VECTOR *) notnot_vector (r),
					     or_integer));
    default:
      fail (invalid_binop);
      break;
    }
  delete_scalar (l);
  delete_vector (r);
  raise_exception ();
}

ENTITY *
binop_scalar_matrix_integer (int op, SCALAR *l, MATRIX *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_scalar_matrix_integer (l, r));
    case BO_SUB:
      return (subtract_scalar_matrix_integer (l, r));
    case BO_MUL:
    case BO_PROD:
      return (multiply_scalar_matrix_integer (l, r));
    case BO_DIV:
      return (divide_scalar_matrix_integer (l, r));
    case BO_APPEND:
      return (append_scalar_matrix_integer (l, r));
    case BO_PILE:
      return (pile_scalar_matrix_integer (l, r));
    case BO_AND:
      return (logical_scalar_matrix_integer (l, r, and_integer));
    case BO_OR:
      return (logical_scalar_matrix_integer (l, r, or_integer));
    case BO_EQ:
      return (logical_scalar_matrix_integer (l, r, eq_integer));
    case BO_NE:
      return (logical_scalar_matrix_integer (l, r, ne_integer));
    case BO_LT:
      return (logical_scalar_matrix_integer (l, r, lt_integer));
    case BO_LTE:
      return (logical_scalar_matrix_integer (l, r, lte_integer));
    case BO_GT:
      return (logical_scalar_matrix_integer (l, r, gt_integer));
    case BO_GTE:
      return (logical_scalar_matrix_integer (l, r, gte_integer));
    case BO_MOD:
      return (mod_scalar_matrix_integer (l, r));
    case BO_POWER:
      return (power_scalar_matrix_integer (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_scalar (l);
  delete_matrix (r);
  raise_exception ();
}

ENTITY *
binop_scalar_matrix_real (int op, SCALAR *l, MATRIX *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_scalar_matrix_real (l, r));
    case BO_SUB:
      return (subtract_scalar_matrix_real (l, r));
    case BO_MUL:
    case BO_PROD:
      return (multiply_scalar_matrix_real (l, r));
    case BO_DIV:
      return (divide_scalar_matrix_real (l, r));
    case BO_APPEND:
      return (append_scalar_matrix_real (l, r));
    case BO_PILE:
      return (pile_scalar_matrix_real (l, r));
    case BO_AND:
      return (logical_scalar_matrix_real (l, r, and_real));
    case BO_OR:
      return (logical_scalar_matrix_real (l, r, or_real));
    case BO_EQ:
      return (logical_scalar_matrix_real (l, r, eq_real));
    case BO_NE:
      return (logical_scalar_matrix_real (l, r, ne_real));
    case BO_LT:
      return (logical_scalar_matrix_real (l, r, lt_real));
    case BO_LTE:
      return (logical_scalar_matrix_real (l, r, lte_real));
    case BO_GT:
      return (logical_scalar_matrix_real (l, r, gt_real));
    case BO_GTE:
      return (logical_scalar_matrix_real (l, r, gte_real));
    case BO_MOD:
      return (mod_scalar_matrix_real (l, r));
    case BO_POWER:
      return (power_scalar_matrix_real (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_scalar (l);
  delete_matrix (r);
  raise_exception ();
}

ENTITY *
binop_scalar_matrix_complex (int op, SCALAR *l, MATRIX *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_scalar_matrix_complex (l, r));
    case BO_SUB:
      return (subtract_scalar_matrix_complex (l, r));
    case BO_MUL:
    case BO_PROD:
      return (multiply_scalar_matrix_complex (l, r));
    case BO_DIV:
      return (divide_scalar_matrix_complex (l, r));
    case BO_APPEND:
      return (append_scalar_matrix_complex (l, r));
    case BO_PILE:
      return (pile_scalar_matrix_complex (l, r));
    case BO_AND:
      return (logical_scalar_matrix_complex (l, r, and_complex));
    case BO_OR:
      return (logical_scalar_matrix_complex (l, r, or_complex));
    case BO_EQ:
      return (logical_scalar_matrix_complex (l, r, eq_complex));
    case BO_NE:
      return (logical_scalar_matrix_complex (l, r, ne_complex));
    case BO_MOD:
      fail (undefined_binop, "Modulus", type_string[complex]);
      break;
    case BO_GT:
    case BO_GTE:
    case BO_LT:
    case BO_LTE:
      fail (undefined_binop, "Comparison", type_string[complex]);
      break;
    case BO_POWER:
      return (power_scalar_matrix_complex (l, r));
    default:
      fail (invalid_binop);
      delete_2_entities (ENT (l), ENT (r));
      raise_exception ();
    }
  delete_2_entities (ENT (l), ENT (r));
  raise_exception ();
}

ENTITY *
binop_scalar_matrix_character (int op, SCALAR *l, MATRIX *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_scalar_matrix_character (l, r));
    case BO_APPEND:
      return (append_scalar_matrix_character (l, r));
    case BO_PILE:
      return (pile_scalar_matrix_character (l, r));
    case BO_AND:
      return (logical_scalar_matrix_character (l, r, and_character));
    case BO_OR:
      return (logical_scalar_matrix_character (l, r, or_character));
    case BO_EQ:
      return (logical_scalar_matrix_character (l, r, eq_character));
    case BO_NE:
      return (logical_scalar_matrix_character (l, r, ne_character));
    case BO_LT:
      return (logical_scalar_matrix_character (l, r, lt_character));
    case BO_LTE:
      return (logical_scalar_matrix_character (l, r, lte_character));
    case BO_GT:
      return (logical_scalar_matrix_character (l, r, gt_character));
    case BO_GTE:
      return (logical_scalar_matrix_character (l, r, gte_character));
    case BO_MOD:
    case BO_SUB:
    case BO_MUL:
    case BO_PROD:
    case BO_DIV:
    case BO_POWER:
      fail (undefined_binop, "This", type_string[character]);
      break;
    default:
      fail (invalid_binop);
      break;
    }
  delete_scalar (l);
  delete_matrix (r);
  raise_exception ();
}

ENTITY *
binop_scalar_matrix_num_char (int op, SCALAR *l, MATRIX *r)
{
  switch (op)
    {
    case BO_AND:
      return (logical_scalar_matrix_integer ((SCALAR *) notnot_scalar (l),
					     (MATRIX *) notnot_matrix (r),
					     and_integer));
    case BO_OR:
      return (logical_scalar_matrix_integer ((SCALAR *) notnot_scalar (l),
					     (MATRIX *) notnot_matrix (r),
					     or_integer));
    default:
      fail (invalid_binop);
      break;
    }
  delete_scalar (l);
  delete_matrix (r);
  raise_exception ();
}

ENTITY *
binop_vector_integer (int op, VECTOR *l, VECTOR *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_vector_integer (l, r));
    case BO_SUB:
      return (subtract_vector_integer (l, r));
    case BO_MUL:
      return (multiply_vector_integer (l, r));
    case BO_PROD:
      return (product_vector_integer (l, r));
    case BO_DIV:
      return (divide_vector_integer (l, r));
    case BO_APPEND:
      return (append_vector_integer (l, r));
    case BO_PILE:
      return (pile_vector_integer (l, r));
    case BO_AND:
      return (logical_vector_integer (l, r, and_integer));
    case BO_OR:
      return (logical_vector_integer (l, r, or_integer));
    case BO_EQ:
      return (logical_vector_integer (l, r, eq_integer));
    case BO_NE:
      return (logical_vector_integer (l, r, ne_integer));
    case BO_LT:
      return (logical_vector_integer (l, r, lt_integer));
    case BO_LTE:
      return (logical_vector_integer (l, r, lte_integer));
    case BO_GT:
      return (logical_vector_integer (l, r, gt_integer));
    case BO_GTE:
      return (logical_vector_integer (l, r, gte_integer));
    case BO_MOD:
      return (mod_vector_integer (l, r));
    case BO_POWER:
      return (power_vector_integer (l, r));
    default:
      fail (invalid_binop);
    }
  delete_2_vectors (l, r);
  raise_exception ();
}

ENTITY *
binop_vector_real (int op, VECTOR *l, VECTOR *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_vector_real (l, r));
    case BO_SUB:
      return (subtract_vector_real (l, r));
    case BO_MUL:
      return (multiply_vector_real (l, r));
    case BO_PROD:
      return (product_vector_real (l, r));
    case BO_DIV:
      return (divide_vector_real (l, r));
    case BO_APPEND:
      return (append_vector_real (l, r));
    case BO_PILE:
      return (pile_vector_real (l, r));
    case BO_AND:
      return (logical_vector_real (l, r, and_real));
    case BO_OR:
      return (logical_vector_real (l, r, or_real));
    case BO_EQ:
      return (logical_vector_real (l, r, eq_real));
    case BO_NE:
      return (logical_vector_real (l, r, ne_real));
    case BO_LT:
      return (logical_vector_real (l, r, lt_real));
    case BO_LTE:
      return (logical_vector_real (l, r, lte_real));
    case BO_GT:
      return (logical_vector_real (l, r, gt_real));
    case BO_GTE:
      return (logical_vector_real (l, r, gte_real));
    case BO_MOD:
      return (mod_vector_real (l, r));
    case BO_POWER:
      return (power_vector_real (l, r));
    default:
      fail (invalid_binop);
    }
  delete_2_vectors (l, r);
  raise_exception ();
}

ENTITY *
binop_vector_complex (int op, VECTOR *l, VECTOR *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_vector_complex (l, r));
    case BO_SUB:
      return (subtract_vector_complex (l, r));
    case BO_MUL:
      return (multiply_vector_complex (l, r));
    case BO_PROD:
      return (product_vector_complex (l, r));
    case BO_DIV:
      return (divide_vector_complex (l, r));
    case BO_APPEND:
      return (append_vector_complex (l, r));
    case BO_PILE:
      return (pile_vector_complex (l, r));
    case BO_AND:
      return (logical_vector_complex (l, r, and_complex));
    case BO_OR:
      return (logical_vector_complex (l, r, or_complex));
    case BO_EQ:
      return (logical_vector_complex (l, r, eq_complex));
    case BO_NE:
      return (logical_vector_complex (l, r, ne_complex));
    case BO_MOD:
      fail (undefined_binop, "Modulus", type_string[complex]);
      break;
    case BO_LT:
    case BO_LTE:
    case BO_GT:
    case BO_GTE:
      fail (undefined_binop, "Comparison", type_string[complex]);
      break;
    case BO_POWER:
      return (power_vector_complex (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_2_vectors (l, r);
  raise_exception ();
}

ENTITY *
binop_vector_character (int op, VECTOR *l, VECTOR *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_vector_character (l, r));
    case BO_APPEND:
      return (append_vector_character (l, r));
    case BO_PILE:
      return (pile_vector_character (l, r));
    case BO_AND:
      return (logical_vector_character (l, r, and_character));
    case BO_OR:
      return (logical_vector_character (l, r, or_character));
    case BO_EQ:
      return (logical_vector_character (l, r, eq_character));
    case BO_NE:
      return (logical_vector_character (l, r, ne_character));
    case BO_LT:
      return (logical_vector_character (l, r, lt_character));
    case BO_LTE:
      return (logical_vector_character (l, r, lte_character));
    case BO_GT:
      return (logical_vector_character (l, r, gt_character));
    case BO_GTE:
      return (logical_vector_character (l, r, gte_character));
    case BO_SUB:
    case BO_MUL:
    case BO_PROD:
    case BO_DIV:
    case BO_MOD:
    case BO_POWER:
      fail (undefined_binop, "This", type_string[character]);
      break;
    default:
      fail (invalid_binop);
      break;
    }
  delete_2_vectors (l, r);
  raise_exception ();
}

ENTITY *
binop_vector_integer_character (int op, VECTOR *l, VECTOR *r)
{
  switch (op)
    {
    case BO_AND:
      return (logical_vector_integer (l, (VECTOR *) notnot_vector (r),
				      and_integer));
    case BO_OR:
      return (logical_vector_integer (l, (VECTOR *) notnot_vector (r),
				      or_integer));
    case BO_ADD:
    case BO_SUB:
    case BO_MUL:
    case BO_PROD:
    case BO_DIV:
    case BO_APPEND:
    case BO_PILE:
    case BO_MOD:
    case BO_EQ:
    case BO_NE:
    case BO_LT:
    case BO_LTE:
    case BO_GT:
    case BO_GTE:
    case BO_POWER:
    default:
      fail (invalid_binop);
      break;
    }
  delete_2_vectors (l, r);
  raise_exception ();
}

ENTITY *
binop_vector_real_character (int op, VECTOR *l, VECTOR *r)
{
  switch (op)
    {
    case BO_AND:
      return (logical_vector_real (l, (VECTOR *) notnot_vector (r),
				   and_real));
    case BO_OR:
      return (logical_vector_real (l, (VECTOR *) notnot_vector (r),
				   or_real));
    case BO_ADD:
    case BO_SUB:
    case BO_MUL:
    case BO_PROD:
    case BO_DIV:
    case BO_APPEND:
    case BO_PILE:
    case BO_MOD:
    case BO_EQ:
    case BO_NE:
    case BO_LT:
    case BO_LTE:
    case BO_GT:
    case BO_GTE:
    case BO_POWER:
    default:
      fail (invalid_binop);
      break;
    }
  delete_2_vectors (l, r);
  raise_exception ();
}

ENTITY *
binop_vector_complex_character (int op, VECTOR *l, VECTOR *r)
{
  switch (op)
    {
    case BO_AND:
      return (logical_vector_complex (l, (VECTOR *) notnot_vector (r),
				      and_complex));
    case BO_OR:
      return (logical_vector_complex (l, (VECTOR *) notnot_vector (r),
				      or_complex));
    case BO_ADD:
    case BO_SUB:
    case BO_MUL:
    case BO_PROD:
    case BO_DIV:
    case BO_APPEND:
    case BO_PILE:
    case BO_MOD:
    case BO_EQ:
    case BO_NE:
    case BO_LT:
    case BO_LTE:
    case BO_GT:
    case BO_GTE:
    case BO_POWER:
    default:
      fail (invalid_binop);
      break;
    }
  delete_2_vectors (l, r);
  raise_exception ();
}

ENTITY *
binop_vector_scalar_integer (int op, VECTOR *l, SCALAR *r)
{
  switch (op)
    {
    case BO_ADD:
      /* It's commutative. */
      return (add_scalar_vector_integer (r, l));
    case BO_SUB:
      return (subtract_vector_scalar_integer (l, r));
    case BO_MUL:
    case BO_PROD:
      /* It's commutative. */
      return (multiply_scalar_vector_integer (r, l));
    case BO_DIV:
      return (divide_vector_scalar_integer (l, r));
    case BO_APPEND:
      return (append_vector_scalar_integer (l, r));
    case BO_PILE:
      return (pile_vector_scalar_integer (l, r));
    case BO_AND:
      return (logical_scalar_vector_integer (r, l, and_integer));
    case BO_OR:
      return (logical_scalar_vector_integer (r, l, or_integer));
    case BO_EQ:
      return (logical_scalar_vector_integer (r, l, eq_integer));
    case BO_NE:
      return (logical_scalar_vector_integer (r, l, ne_integer));
    case BO_LT:
      return (logical_scalar_vector_integer (r, l, gt_integer));
    case BO_LTE:
      return (logical_scalar_vector_integer (r, l, gte_integer));
    case BO_GT:
      return (logical_scalar_vector_integer (r, l, lt_integer));
    case BO_GTE:
      return (logical_scalar_vector_integer (r, l, lte_integer));
    case BO_MOD:
      return (mod_vector_scalar_integer (l, r));
    case BO_POWER:
      return (power_vector_scalar_integer (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_vector (l);
  delete_scalar (r);
  raise_exception ();
}

ENTITY *
binop_vector_scalar_real (int op, VECTOR *l, SCALAR *r)
{
  switch (op)
    {
    case BO_ADD:
      /* Use commutativity. */
      return (add_scalar_vector_real (r, l));
    case BO_SUB:
      return (subtract_vector_scalar_real (l, r));
    case BO_MUL:
    case BO_PROD:
      /* It's commutative. */
      return (multiply_scalar_vector_real (r, l));
    case BO_DIV:
      return (divide_vector_scalar_real (l, r));
    case BO_APPEND:
      return (append_vector_scalar_real (l, r));
    case BO_PILE:
      return (pile_vector_scalar_real (l, r));
    case BO_AND:
      return (logical_scalar_vector_real (r, l, and_real));
    case BO_OR:
      return (logical_scalar_vector_real (r, l, or_real));
    case BO_EQ:
      return (logical_scalar_vector_real (r, l, eq_real));
    case BO_NE:
      return (logical_scalar_vector_real (r, l, ne_real));
    case BO_LT:
      return (logical_scalar_vector_real (r, l, gt_real));
    case BO_LTE:
      return (logical_scalar_vector_real (r, l, gte_real));
    case BO_GT:
      return (logical_scalar_vector_real (r, l, lt_real));
    case BO_GTE:
      return (logical_scalar_vector_real (r, l, lte_real));
    case BO_MOD:
      return (mod_vector_scalar_real (l, r));
    case BO_POWER:
      return (power_vector_scalar_real (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_vector (l);
  delete_scalar (r);
  raise_exception ();
}

ENTITY *
binop_vector_scalar_complex (int op, VECTOR *l, SCALAR *r)
{
  switch (op)
    {
    case BO_ADD:
      /* Use commutativity. */
      return (add_scalar_vector_complex (r, l));
    case BO_SUB:
      return (subtract_vector_scalar_complex (l, r));
    case BO_MUL:
    case BO_PROD:
      /* It's commutative. */
      return (multiply_scalar_vector_complex (r, l));
    case BO_DIV:
      return (divide_vector_scalar_complex (l, r));
    case BO_APPEND:
      return (append_vector_scalar_complex (l, r));
    case BO_PILE:
      return (pile_vector_scalar_complex (l, r));
    case BO_AND:
      return (logical_scalar_vector_complex (r, l, and_complex));
    case BO_OR:
      return (logical_scalar_vector_complex (r, l, or_complex));
    case BO_EQ:
      return (logical_scalar_vector_complex (r, l, eq_complex));
    case BO_NE:
      return (logical_scalar_vector_complex (r, l, ne_complex));
    case BO_MOD:
      fail (undefined_binop, "Modulus", type_string[complex]);
      break;
    case BO_LT:
    case BO_LTE:
    case BO_GT:
    case BO_GTE:
      fail (undefined_binop, "Comparison", type_string[complex]);
      break;
    case BO_POWER:
      return (power_vector_scalar_complex (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_vector (l);
  delete_scalar (r);
  raise_exception ();
}

ENTITY *
binop_vector_scalar_character (int op, VECTOR *l, SCALAR *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_vector_scalar_character (l, r));
    case BO_APPEND:
      return (append_vector_scalar_character (l, r));
    case BO_PILE:
      return (pile_vector_scalar_character (l, r));
    case BO_AND:
      return (logical_scalar_vector_character (r, l, and_character));
    case BO_OR:
      return (logical_scalar_vector_character (r, l, or_character));
    case BO_EQ:
      return (logical_scalar_vector_character (r, l, eq_character));
    case BO_NE:
      return (logical_scalar_vector_character (r, l, ne_character));
    case BO_LT:
      return (logical_scalar_vector_character (r, l, gt_character));
    case BO_LTE:
      return (logical_scalar_vector_character (r, l, gte_character));
    case BO_GT:
      return (logical_scalar_vector_character (r, l, lt_character));
    case BO_GTE:
      return (logical_scalar_vector_character (r, l, lte_character));
    case BO_SUB:
    case BO_MUL:
    case BO_PROD:
    case BO_DIV:
    case BO_MOD:
    case BO_POWER:
      fail (undefined_binop, "This", type_string[character]);
      break;
    default:
      fail (invalid_binop);
      break;
    }
  delete_vector (l);
  delete_scalar (r);
  raise_exception ();
}

ENTITY *
binop_vector_scalar_num_char (int op, VECTOR *l, SCALAR *r)
{
  switch (op)
    {
    case BO_AND:
      return (logical_scalar_vector_integer ((SCALAR *) notnot_scalar (r),
					     (VECTOR *) notnot_vector (l),
					     and_integer));
    case BO_OR:
      return (logical_scalar_vector_integer ((SCALAR *) notnot_scalar (r),
					     (VECTOR *) notnot_vector (l),
					     or_integer));
    default:
      fail (invalid_binop);
      break;
    }
  delete_vector (l);
  delete_scalar (r);
  raise_exception ();
}

ENTITY *
binop_vector_matrix_integer (int op, VECTOR *l, MATRIX *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_vector_matrix_integer (l, r));
    case BO_SUB:
      return (subtract_vector_matrix_integer (l, r));
    case BO_MUL:
      return (multiply_vector_matrix_integer (l, r));
    case BO_PROD:
      return (product_matrix_integer ((MATRIX *) vector_to_matrix (l), r));
    case BO_DIV:
      return (divide_vector_matrix_integer (l, r));
    case BO_APPEND:
      return (append_vector_matrix_integer (l, r));
    case BO_PILE:
      return (pile_vector_matrix_integer (l, r));
    case BO_AND:
      return (logical_vector_matrix_integer (l, r, and_integer));
    case BO_OR:
      return (logical_vector_matrix_integer (l, r, or_integer));
    case BO_EQ:
      return (logical_vector_matrix_integer (l, r, eq_integer));
    case BO_NE:
      return (logical_vector_matrix_integer (l, r, ne_integer));
    case BO_LT:
      return (logical_vector_matrix_integer (l, r, lt_integer));
    case BO_LTE:
      return (logical_vector_matrix_integer (l, r, lte_integer));
    case BO_GT:
      return (logical_vector_matrix_integer (l, r, gt_integer));
    case BO_GTE:
      return (logical_vector_matrix_integer (l, r, gte_integer));
    case BO_MOD:
      return (mod_vector_matrix_integer (l, r));
    case BO_POWER:
      return (power_vector_matrix_integer (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_vector (l);
  delete_matrix (r);
  raise_exception ();
}

ENTITY *
binop_vector_matrix_real (int op, VECTOR *l, MATRIX *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_vector_matrix_real (l, r));
    case BO_SUB:
      return (subtract_vector_matrix_real (l, r));
    case BO_MUL:
      return (multiply_vector_matrix_real (l, r));
    case BO_PROD:
      return (product_matrix_real ((MATRIX *) vector_to_matrix (l), r));
    case BO_DIV:
      return (divide_vector_matrix_real (l, r));
    case BO_APPEND:
      return (append_vector_matrix_real (l, r));
    case BO_PILE:
      return (pile_vector_matrix_real (l, r));
    case BO_AND:
      return (logical_vector_matrix_real (l, r, and_real));
    case BO_OR:
      return (logical_vector_matrix_real (l, r, or_real));
    case BO_EQ:
      return (logical_vector_matrix_real (l, r, eq_real));
    case BO_NE:
      return (logical_vector_matrix_real (l, r, ne_real));
    case BO_LT:
      return (logical_vector_matrix_real (l, r, lt_real));
    case BO_LTE:
      return (logical_vector_matrix_real (l, r, lte_real));
    case BO_GT:
      return (logical_vector_matrix_real (l, r, gt_real));
    case BO_GTE:
      return (logical_vector_matrix_real (l, r, gte_real));
    case BO_MOD:
      return (mod_vector_matrix_real (l, r));
    case BO_POWER:
      return (power_vector_matrix_real (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_vector (l);
  delete_matrix (r);
  raise_exception ();
}

ENTITY *
binop_vector_matrix_complex (int op, VECTOR *l, MATRIX *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_vector_matrix_complex (l, r));
    case BO_SUB:
      return (subtract_vector_matrix_complex (l, r));
    case BO_MUL:
      return (multiply_vector_matrix_complex (l, r));
    case BO_PROD:
      return product_matrix_complex ((MATRIX *) vector_to_matrix (l), r);
    case BO_DIV:
      return (divide_vector_matrix_complex (l, r));
    case BO_APPEND:
      return (append_vector_matrix_complex (l, r));
    case BO_PILE:
      return (pile_vector_matrix_complex (l, r));
    case BO_AND:
      return (logical_vector_matrix_complex (l, r, and_complex));
    case BO_OR:
      return (logical_vector_matrix_complex (l, r, or_complex));
    case BO_EQ:
      return (logical_vector_matrix_complex (l, r, eq_complex));
    case BO_NE:
      return (logical_vector_matrix_complex (l, r, ne_complex));
    case BO_MOD:
      fail (undefined_binop, "Modulus", type_string[complex]);
      break;
    case BO_POWER:
      return (power_vector_matrix_complex (l, r));
    case BO_LT:
    case BO_LTE:
    case BO_GT:
    case BO_GTE:
      fail (undefined_binop, "Comparison", type_string[complex]);
      break;
    default:
      fail (invalid_binop);
      break;
    }
  delete_vector (l);
  delete_matrix (r);
  raise_exception ();
}

ENTITY *
binop_vector_matrix_character (int op, VECTOR *l, MATRIX *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_vector_matrix_character (l, r));
    case BO_APPEND:
      return (append_vector_matrix_character (l, r));
    case BO_PILE:
      return (pile_vector_matrix_character (l, r));
    case BO_AND:
      return (logical_vector_matrix_character (l, r, and_character));
    case BO_OR:
      return (logical_vector_matrix_character (l, r, or_character));
    case BO_EQ:
      return (logical_vector_matrix_character (l, r, eq_character));
    case BO_NE:
      return (logical_vector_matrix_character (l, r, ne_character));
    case BO_LT:
      return (logical_vector_matrix_character (l, r, lt_character));
    case BO_LTE:
      return (logical_vector_matrix_character (l, r, lte_character));
    case BO_GT:
      return (logical_vector_matrix_character (l, r, gt_character));
    case BO_GTE:
      return (logical_vector_matrix_character (l, r, gte_character));
    case BO_SUB:
    case BO_MUL:
    case BO_PROD:
    case BO_DIV:
    case BO_MOD:
    case BO_POWER:
      fail (undefined_binop, "This", type_string[character]);
      break;
    default:
      fail (invalid_binop);
      break;
    }
  delete_vector (l);
  delete_matrix (r);
  raise_exception ();
}

ENTITY *
binop_vector_matrix_num_char (int op, VECTOR *l, MATRIX *r)
{
  switch (op)
    {
    case BO_AND:
      return (logical_vector_matrix_integer ((VECTOR *) notnot_vector (l),
					     (MATRIX *) notnot_matrix (r),
					     and_integer));
    case BO_OR:
      return (logical_vector_matrix_integer ((VECTOR *) notnot_vector (l),
					     (MATRIX *) notnot_matrix (r),
					     or_integer));
    default:
      fail (invalid_binop);
      break;
    }
  delete_vector (l);
  delete_matrix (r);
  raise_exception ();
}

ENTITY *
binop_vector_matrix_char_num (int op, VECTOR *l, MATRIX *r)
{
  switch (op)
    {
    case BO_AND:
      return (logical_vector_matrix_integer ((VECTOR *) notnot_vector (l),
					     (MATRIX *) notnot_matrix (r),
					     and_integer));
    case BO_OR:
      return (logical_vector_matrix_integer ((VECTOR *) notnot_vector (l),
					     (MATRIX *) notnot_matrix (r),
					     or_integer));
    default:
      fail (invalid_binop);
      break;
    }
  delete_vector (l);
  delete_matrix (r);
  raise_exception ();
}

ENTITY *
binop_matrix_integer (int op, MATRIX *l, MATRIX *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_matrix_integer (l, r));
    case BO_SUB:
      return (subtract_matrix_integer (l, r));
    case BO_MUL:
      return (multiply_matrix_integer (l, r));
    case BO_PROD:
      return (product_matrix_integer (l, r));
    case BO_DIV:
      return (divide_matrix_integer (l, r));
    case BO_APPEND:
      return (append_matrix_integer (l, r));
    case BO_PILE:
      return (pile_matrix_integer (l, r));
    case BO_AND:
      return (logical_matrix_integer (l, r, and_integer));
    case BO_OR:
      return (logical_matrix_integer (l, r, or_integer));
    case BO_EQ:
      return (logical_matrix_integer (l, r, eq_integer));
    case BO_NE:
      return (logical_matrix_integer (l, r, ne_integer));
    case BO_LT:
      return (logical_matrix_integer (l, r, lt_integer));
    case BO_LTE:
      return (logical_matrix_integer (l, r, lte_integer));
    case BO_GT:
      return (logical_matrix_integer (l, r, gt_integer));
    case BO_GTE:
      return (logical_matrix_integer (l, r, gte_integer));
    case BO_MOD:
      return (mod_matrix_integer (l, r));
    case BO_POWER:
      return (power_matrix_integer (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_2_matrices (l, r);
  raise_exception ();
}

ENTITY *
binop_matrix_real (int op, MATRIX *l, MATRIX *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_matrix_real (l, r));
    case BO_SUB:
      return (subtract_matrix_real (l, r));
    case BO_MUL:
      return (multiply_matrix_real (l, r));
    case BO_PROD:
      return (product_matrix_real (l, r));
    case BO_DIV:
      return (divide_matrix_real (l, r));
    case BO_APPEND:
      return (append_matrix_real (l, r));
    case BO_PILE:
      return (pile_matrix_real (l, r));
    case BO_AND:
      return (logical_matrix_real (l, r, and_real));
    case BO_OR:
      return (logical_matrix_real (l, r, or_real));
    case BO_EQ:
      return (logical_matrix_real (l, r, eq_real));
    case BO_NE:
      return (logical_matrix_real (l, r, ne_real));
    case BO_LT:
      return (logical_matrix_real (l, r, lt_real));
    case BO_LTE:
      return (logical_matrix_real (l, r, lte_real));
    case BO_GT:
      return (logical_matrix_real (l, r, gt_real));
    case BO_GTE:
      return (logical_matrix_real (l, r, gte_real));
    case BO_MOD:
      return (mod_matrix_real (l, r));
    case BO_POWER:
      return (power_matrix_real (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_2_matrices (l, r);
  raise_exception ();
}

ENTITY *
binop_matrix_complex (int op, MATRIX *l, MATRIX *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_matrix_complex (l, r));
    case BO_SUB:
      return (subtract_matrix_complex (l, r));
    case BO_MUL:
      return (multiply_matrix_complex (l, r));
    case BO_PROD:
      return (product_matrix_complex (l, r));
    case BO_DIV:
      return (divide_matrix_complex (l, r));
    case BO_APPEND:
      return (append_matrix_complex (l, r));
    case BO_PILE:
      return (pile_matrix_complex (l, r));
    case BO_AND:
      return (logical_matrix_complex (l, r, and_complex));
    case BO_OR:
      return (logical_matrix_complex (l, r, or_complex));
    case BO_EQ:
      return (logical_matrix_complex (l, r, eq_complex));
    case BO_NE:
      return (logical_matrix_complex (l, r, ne_complex));
    case BO_MOD:
      fail (undefined_binop, "Modulus", type_string[complex]);
      break;
    case BO_LT:
    case BO_LTE:
    case BO_GT:
    case BO_GTE:
      fail (undefined_binop, "Comparison", type_string[complex]);
      break;
    case BO_POWER:
      return (power_matrix_complex (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_2_matrices (l, r);
  raise_exception ();
}

ENTITY *
binop_matrix_character (int op, MATRIX *l, MATRIX *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_matrix_character (l, r));
    case BO_APPEND:
      return (append_matrix_character (l, r));
    case BO_PILE:
      return (pile_matrix_character (l, r));
    case BO_AND:
      return (logical_matrix_character (l, r, and_character));
    case BO_OR:
      return (logical_matrix_character (l, r, or_character));
    case BO_EQ:
      return (logical_matrix_character (l, r, eq_character));
    case BO_NE:
      return (logical_matrix_character (l, r, ne_character));
    case BO_LT:
      return (logical_matrix_character (l, r, lt_character));
    case BO_LTE:
      return (logical_matrix_character (l, r, lte_character));
    case BO_GT:
      return (logical_matrix_character (l, r, gt_character));
    case BO_GTE:
      return (logical_matrix_character (l, r, gte_character));
    case BO_SUB:
    case BO_MUL:
    case BO_PROD:
    case BO_DIV:
    case BO_MOD:
    case BO_POWER:
      fail (undefined_binop, "This", type_string[character]);
      break;
    default:
      fail (invalid_binop);
      break;
    }
  delete_2_matrices (l, r);
  raise_exception ();
}

ENTITY *
binop_matrix_num_char (int op, MATRIX *l, MATRIX *r)
{
  switch (op)
    {
    case BO_AND:
      return (logical_matrix_integer ((MATRIX *) notnot_matrix (l),
				      (MATRIX *) notnot_matrix (r),
				      and_integer));
    case BO_OR:
      return (logical_matrix_integer ((MATRIX *) notnot_matrix (l),
				      (MATRIX *) notnot_matrix (r),
				      or_integer));
    default:
      fail (invalid_binop);
      delete_2_matrices (l, r);
      raise_exception ();
    }
}

ENTITY *
binop_matrix_scalar_integer (int op, MATRIX *l, SCALAR *r)
{
  switch (op)
    {
    case BO_ADD:
      /* Use commutativity. */
      return (add_scalar_matrix_integer (r, l));
    case BO_SUB:
      return (subtract_matrix_scalar_integer (l, r));
    case BO_MUL:
    case BO_PROD:
      /* It's commutative. */
      return (multiply_scalar_matrix_integer (r, l));
    case BO_DIV:
      return (divide_matrix_scalar_integer (l, r));
    case BO_APPEND:
      return (append_matrix_scalar_integer (l, r));
    case BO_PILE:
      return (pile_matrix_scalar_integer (l, r));
    case BO_AND:
      return (logical_scalar_matrix_integer (r, l, and_integer));
    case BO_OR:
      return (logical_scalar_matrix_integer (r, l, or_integer));
    case BO_EQ:
      return (logical_scalar_matrix_integer (r, l, eq_integer));
    case BO_NE:
      return (logical_scalar_matrix_integer (r, l, ne_integer));
    case BO_LT:
      return (logical_scalar_matrix_integer (r, l, gt_integer));
    case BO_LTE:
      return (logical_scalar_matrix_integer (r, l, gte_integer));
    case BO_GT:
      return (logical_scalar_matrix_integer (r, l, lt_integer));
    case BO_GTE:
      return (logical_scalar_matrix_integer (r, l, lte_integer));
    case BO_MOD:
      return (mod_matrix_scalar_integer (l, r));
    case BO_POWER:
      return (power_matrix_scalar_integer (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_matrix (l);
  delete_scalar (r);
  raise_exception ();
}

ENTITY *
binop_matrix_scalar_real (int op, MATRIX *l, SCALAR *r)
{
  switch (op)
    {
    case BO_ADD:
      /* Use commutativity. */
      return (add_scalar_matrix_real (r, l));
    case BO_SUB:
      return (subtract_matrix_scalar_real (l, r));
    case BO_MUL:
    case BO_PROD:
      /* It's commutative. */
      return (multiply_scalar_matrix_real (r, l));
    case BO_DIV:
      return (divide_matrix_scalar_real (l, r));
    case BO_APPEND:
      return (append_matrix_scalar_real (l, r));
    case BO_PILE:
      return (pile_matrix_scalar_real (l, r));
    case BO_AND:
      return (logical_scalar_matrix_real (r, l, and_real));
    case BO_OR:
      return (logical_scalar_matrix_real (r, l, or_real));
    case BO_EQ:
      return (logical_scalar_matrix_real (r, l, eq_real));
    case BO_NE:
      return (logical_scalar_matrix_real (r, l, ne_real));
    case BO_LT:
      return (logical_scalar_matrix_real (r, l, gt_real));
    case BO_LTE:
      return (logical_scalar_matrix_real (r, l, gte_real));
    case BO_GT:
      return (logical_scalar_matrix_real (r, l, lt_real));
    case BO_GTE:
      return (logical_scalar_matrix_real (r, l, lte_real));
    case BO_MOD:
      return (mod_matrix_scalar_real (l, r));
    case BO_POWER:
      return (power_matrix_scalar_real (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_matrix (l);
  delete_scalar (r);
  raise_exception ();
}

ENTITY *
binop_matrix_scalar_complex (int op, MATRIX *l, SCALAR *r)
{
  switch (op)
    {
    case BO_ADD:
      /* Use commutativity. */
      return (add_scalar_matrix_complex (r, l));
    case BO_SUB:
      return (subtract_matrix_scalar_complex (l, r));
    case BO_MUL:
    case BO_PROD:
      /* It's commutative. */
      return (multiply_scalar_matrix_complex (r, l));
    case BO_DIV:
      return (divide_matrix_scalar_complex (l, r));
    case BO_APPEND:
      return (append_matrix_scalar_complex (l, r));
    case BO_PILE:
      return (pile_matrix_scalar_complex (l, r));
    case BO_AND:
      return (logical_scalar_matrix_complex (r, l, and_complex));
    case BO_OR:
      return (logical_scalar_matrix_complex (r, l, or_complex));
    case BO_EQ:
      return (logical_scalar_matrix_complex (r, l, eq_complex));
    case BO_NE:
      return (logical_scalar_matrix_complex (r, l, ne_complex));
    case BO_MOD:
      fail (undefined_binop, "Modulus", type_string[complex]);
      break;
    case BO_LT:
    case BO_LTE:
    case BO_GT:
    case BO_GTE:
      fail (undefined_binop, "Comparison", type_string[complex]);
      break;
    case BO_POWER:
      return (power_matrix_scalar_complex (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_matrix (l);
  delete_scalar (r);
  raise_exception ();
}

ENTITY *
binop_matrix_scalar_character (int op, MATRIX *l, SCALAR *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_matrix_scalar_character (l, r));
    case BO_APPEND:
      return (append_matrix_scalar_character (l, r));
    case BO_PILE:
      return (pile_matrix_scalar_character (l, r));
    case BO_AND:
      return (logical_scalar_matrix_character (r, l, and_character));
    case BO_OR:
      return (logical_scalar_matrix_character (r, l, or_character));
    case BO_EQ:
      return (logical_scalar_matrix_character (r, l, eq_character));
    case BO_NE:
      return (logical_scalar_matrix_character (r, l, ne_character));
    case BO_LT:
      return (logical_scalar_matrix_character (r, l, gt_character));
    case BO_LTE:
      return (logical_scalar_matrix_character (r, l, gte_character));
    case BO_GT:
      return (logical_scalar_matrix_character (r, l, lt_character));
    case BO_GTE:
      return (logical_scalar_matrix_character (r, l, lte_character));
    case BO_SUB:
    case BO_MUL:
    case BO_PROD:
    case BO_DIV:
    case BO_MOD:
    case BO_POWER:
      fail (undefined_binop, "This", type_string[character]);
      break;
    default:
      fail (invalid_binop);
      break;
    }
  delete_matrix (l);
  delete_scalar (r);
  raise_exception ();
}

ENTITY *
binop_matrix_scalar_num_char (int op, MATRIX *l, SCALAR *r)
{
  switch (op)
    {
    case BO_AND:
      return (logical_scalar_matrix_integer ((SCALAR *) notnot_scalar (r),
					     (MATRIX *) notnot_matrix (l),
					     and_integer));
    case BO_OR:
      return (logical_scalar_matrix_integer ((SCALAR *) notnot_scalar (r),
					     (MATRIX *) notnot_matrix (l),
					     or_integer));
    default:
      fail (invalid_binop);
      delete_matrix (l);
      delete_scalar (r);
      raise_exception ();
    }
}

ENTITY *
binop_matrix_vector_integer (int op, MATRIX *l, VECTOR *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_matrix_vector_integer (l, r));
    case BO_SUB:
      return (subtract_matrix_vector_integer (l, r));
    case BO_MUL:
      return (multiply_matrix_vector_integer (l, r));
    case BO_PROD:
      return (product_matrix_integer (l, (MATRIX *) vector_to_matrix (r)));
    case BO_DIV:
      return (divide_matrix_vector_integer (l, r));
    case BO_APPEND:
      return (append_matrix_integer (l, (MATRIX *) vector_to_matrix (r)));
    case BO_PILE:
      return (pile_matrix_vector_integer (l, r));
    case BO_AND:
      return (logical_matrix_vector_integer (l, r, and_integer));
    case BO_OR:
      return (logical_matrix_vector_integer (l, r, or_integer));
    case BO_EQ:
      return (logical_matrix_vector_integer (l, r, eq_integer));
    case BO_NE:
      return (logical_matrix_vector_integer (l, r, ne_integer));
    case BO_LT:
      return (logical_matrix_vector_integer (l, r, lt_integer));
    case BO_LTE:
      return (logical_matrix_vector_integer (l, r, lte_integer));
    case BO_GT:
      return (logical_matrix_vector_integer (l, r, gt_integer));
    case BO_GTE:
      return (logical_matrix_vector_integer (l, r, gte_integer));
    case BO_MOD:
      return (mod_matrix_vector_integer (l, r));
    case BO_POWER:
      return (power_matrix_vector_integer (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_matrix (l);
  delete_vector (r);
  raise_exception ();
}

ENTITY *
binop_matrix_vector_real (int op, MATRIX *l, VECTOR *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_matrix_vector_real (l, r));
    case BO_SUB:
      return (subtract_matrix_vector_real (l, r));
    case BO_MUL:
      return (multiply_matrix_vector_real (l, r));
    case BO_PROD:
      return (product_matrix_real (l, (MATRIX *) vector_to_matrix (r)));
    case BO_DIV:
      return (divide_matrix_vector_real (l, r));
    case BO_APPEND:
      return (append_matrix_real (l, (MATRIX *) vector_to_matrix (r)));
    case BO_PILE:
      return (pile_matrix_vector_real (l, r));
    case BO_AND:
      return (logical_matrix_vector_real (l, r, and_real));
    case BO_OR:
      return (logical_matrix_vector_real (l, r, or_real));
    case BO_EQ:
      return (logical_matrix_vector_real (l, r, eq_real));
    case BO_NE:
      return (logical_matrix_vector_real (l, r, ne_real));
    case BO_LT:
      return (logical_matrix_vector_real (l, r, lt_real));
    case BO_LTE:
      return (logical_matrix_vector_real (l, r, lte_real));
    case BO_GT:
      return (logical_matrix_vector_real (l, r, gt_real));
    case BO_GTE:
      return (logical_matrix_vector_real (l, r, gte_real));
    case BO_MOD:
      return (mod_matrix_vector_real (l, r));
    case BO_POWER:
      return (power_matrix_vector_real (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_matrix (l);
  delete_vector (r);
  raise_exception ();
}

ENTITY *
binop_matrix_vector_complex (int op, MATRIX *l, VECTOR *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_matrix_vector_complex (l, r));
    case BO_SUB:
      return (subtract_matrix_vector_complex (l, r));
    case BO_MUL:
      return (multiply_matrix_vector_complex (l, r));
    case BO_PROD:
      return (product_matrix_complex (l, (MATRIX *) vector_to_matrix (r)));
    case BO_DIV:
      return (divide_matrix_vector_complex (l, r));
    case BO_APPEND:
      return (append_matrix_complex (l, (MATRIX *) vector_to_matrix (r)));
    case BO_PILE:
      return (pile_matrix_vector_complex (l, r));
    case BO_AND:
      return (logical_matrix_vector_complex (l, r, and_complex));
    case BO_OR:
      return (logical_matrix_vector_complex (l, r, or_complex));
    case BO_EQ:
      return (logical_matrix_vector_complex (l, r, eq_complex));
    case BO_NE:
      return (logical_matrix_vector_complex (l, r, ne_complex));
    case BO_MOD:
      fail (undefined_binop, "Modulus", type_string[complex]);
      break;
    case BO_LT:
    case BO_LTE:
    case BO_GT:
    case BO_GTE:
      fail (undefined_binop, "Comparison", type_string[complex]);
      break;
    case BO_POWER:
      return (power_matrix_vector_complex (l, r));
    default:
      fail (invalid_binop);
      break;
    }
  delete_matrix (l);
  delete_vector (r);
  raise_exception ();
}

ENTITY *
binop_matrix_vector_character (int op, MATRIX *l, VECTOR *r)
{
  switch (op)
    {
    case BO_ADD:
      return (add_matrix_vector_character (l, r));
    case BO_APPEND:
      return (append_matrix_character (l, (MATRIX *) vector_to_matrix (r)));
    case BO_PILE:
      return (pile_matrix_vector_character (l, r));
    case BO_AND:
      return (logical_matrix_vector_character (l, r, and_character));
    case BO_OR:
      return (logical_matrix_vector_character (l, r, or_character));
    case BO_EQ:
      return (logical_matrix_vector_character (l, r, eq_character));
    case BO_NE:
      return (logical_matrix_vector_character (l, r, ne_character));
    case BO_LT:
      return (logical_matrix_vector_character (l, r, lt_character));
    case BO_LTE:
      return (logical_matrix_vector_character (l, r, lte_character));
    case BO_GT:
      return (logical_matrix_vector_character (l, r, gt_character));
    case BO_GTE:
      return (logical_matrix_vector_character (l, r, gte_character));
    case BO_SUB:
    case BO_MUL:
    case BO_PROD:
    case BO_DIV:
    case BO_MOD:
    case BO_POWER:
      fail (undefined_binop, "This", type_string[character]);
      break;
    default:
      fail (invalid_binop);
      break;
    }
  delete_matrix (l);
  delete_vector (r);
  raise_exception ();
}

ENTITY *
binop_matrix_vector_num_char (int op, MATRIX *l, VECTOR *r)
{
  switch (op)
    {
    case BO_AND:
      return (logical_matrix_vector_integer ((MATRIX *) notnot_matrix (l),
					     (VECTOR *) notnot_vector (r),
					     and_integer));
    case BO_OR:
      return (logical_matrix_vector_integer ((MATRIX *) notnot_matrix (l),
					     (VECTOR *) notnot_vector (r),
					     or_integer));
    default:
      fail (invalid_binop);
      break;
    }
  delete_matrix (l);
  delete_vector (r);
  raise_exception ();
}

ENTITY *
binop_matrix_vector_char_num (int op, MATRIX *l, VECTOR *r)
{
  switch (op)
    {
    case BO_AND:
      return (logical_matrix_vector_integer ((MATRIX *) notnot_matrix (l),
					     (VECTOR *) notnot_vector (r),
					     and_integer));
    case BO_OR:
      return (logical_matrix_vector_integer ((MATRIX *) notnot_matrix (l),
					     (VECTOR *) notnot_vector (r),
					     or_integer));
    default:
      fail (invalid_binop);
      break;
    }
  delete_matrix (l);
  delete_vector (r);
  raise_exception ();
}
