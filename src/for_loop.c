/*
   for_loop.c -- Algae `for' loops.

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: for_loop.c,v 1.2 1997/02/21 09:53:56 ksh Exp $";

#include "for_loop.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "partition.h"

void
init_for_loop (FOR_LOOP *f)
{
  EASSERT (f->list, vector, 0);

  f->type = f->list->type;
  f->last_elem = 0;
}

int
inc_for_loop (FOR_LOOP *f)
{
  assert (f->list->entity.class == vector);

  if (++f->last_elem > f->list->ne)
    return (0);

  switch (f->type)
    {

    case integer:
      f->value.integer =
	get_vector_element_integer (
				(VECTOR *) copy_vector ((VECTOR *) f->list),
				     f->last_elem);
      break;

    case real:
      f->value.real =
	get_vector_element_real (
				(VECTOR *) copy_vector ((VECTOR *) f->list),
				  f->last_elem);
      break;

    case complex:
      f->ep = complex_to_scalar (
				  get_vector_element_complex (
				(VECTOR *) copy_vector ((VECTOR *) f->list),
							     f->last_elem));
      break;

    case character:
      f->ep = char_to_scalar (
			       get_vector_element_character (
				(VECTOR *) copy_vector ((VECTOR *) f->list),
							      f->last_elem));
      break;

    default:
      BAD_TYPE (f->type);
      raise_exception ();
    }

  return (1);
}


/* on exception or function return pop the whole
   for_loop_stack passed here as linked list
 */

void
cleanup_for_loop_stack (FOR_LOOP *f)
{
  FOR_LOOP *tf;
  assert (f);

  do
    {
      tf = f;
      f = tf->link;

      delete_vector (tf->list);
      FREE (tf);
    }
  while (f);
}
