/*
   member.c -- Functions for member operations.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: member.c,v 1.2 2003/08/01 04:57:47 ksh Exp $";

#include "member.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "function.h"
#include "sort.h"
#include "append.h"

extern MEMBER_ID scalar_member_names[];
extern MEMBER_ID vector_member_names[];
extern MEMBER_ID matrix_member_names[];
extern MEMBER_ID function_member_names[];

ENTITY *
assign_member (ENTITY *head, ENTITY *new, char *name)
{
  /*
   * Assigns `new' as a member of `head' called `name'.
   * Watch it---all args are deleted.
   *
   * WARNING:  We've made a promise to the `execute' function that
   *           we won't raise an exception for any reason when `head'
   *           is a table.
   */

  EASSERT (head, 0, 0);

  switch (head->class)
    {
    case scalar:
      return (assign_member_scalar ((SCALAR *) head, new, name));
    case vector:
      return (assign_member_vector ((VECTOR *) head, new, name));
    case matrix:
      return (assign_member_matrix ((MATRIX *) head, new, name));
    case table:
      return (replace_in_table ((TABLE *) head, new, name));
    case function:
      return (assign_member_function ((FUNCTION *) head, new, name));
    default:
      BAD_CLASS (head->class);
      delete_2_entities (head, new);
      FREE_CHAR (name);
      raise_exception ();
    }
}

ENTITY *
assign_members (ENTITY *head, ENTITY * volatile new, int cnt, char ** volatile memv)
{
  ENTITY *p = NULL;

  if (head == NULL)
    {
      fail ("Member assignment in NULL entity.");
      delete_2_entities (head, new);
      while (cnt-- > 0)
	{
	  FREE_CHAR (*memv);
	  memv++;
	}
      raise_exception ();
    }

  EASSERT (head, 0, 0);
  assert (cnt > 0);

  if (cnt > 1)
    {

      p = reference_member (copy_entity (head), dup_char (*memv));

      WITH_HANDLING
      {
	new = assign_members (p, new, cnt - 1, memv + 1);
      }
      ON_EXCEPTION
      {
	delete_entity (head);
	FREE_CHAR (*memv);
      }
      END_EXCEPTION;
    }

  return (assign_member (head, new, *memv));
}

ENTITY *
reference_member (ENTITY *head, char *name)
{
  /*
   * Returns the member called `name' of entity `head'.  The
   * string `name' is FREE'd.
   */

  EASSERT (head, 0, 0);

  switch (head->class)
    {
    case scalar:
      return (reference_member_scalar ((SCALAR *) head, name));
    case vector:
      return (reference_member_vector ((VECTOR *) head, name));
    case matrix:
      return (reference_member_matrix ((MATRIX *) head, name));
    case table:
      return (search_in_table ((TABLE *) head, name));
    case function:
      return (reference_member_function ((FUNCTION *) head, name));
    default:
      BAD_CLASS (head->class);
      delete_entity (head);
      FREE_CHAR (name);
      raise_exception ();
    }
}

ENTITY *
reference_members (ENTITY *head, volatile int cnt, char ** volatile memv)
{
  /*
   * This function handles an expression like `a.b.c', returning
   * `c' which is a member of `b' which is a member of `a'.  The arg
   * `head' is the first entity (`a').  The arg `cnt' gives the
   * number of members referenced (2 in `a.b.c').  The arg `memv'
   * contains the names of the members ("b" and "c" in `a.b.c').
   * The character strings in `memv' are all FREE'd.
   */

  char ** volatile p = memv;
  int c = cnt;

  EASSERT (head, 0, 0);
  assert (cnt > 0);

  WITH_HANDLING
  {
    while (c-- > 0)
      {
	if (head == NULL)
	  {
	    fail ("Member reference in NULL entity.");
	    raise_exception ();
	  }
	head = reference_member (EAT (head), EAT (*p));
	p++;
      }
  }
  ON_EXCEPTION
  {
    delete_entity (head);
    while (cnt-- > 0)
      {
	if (*memv)
	  FREE_CHAR (*memv);
	memv++;
      }
  }
  END_EXCEPTION;

  return (head);
}

ENTITY *
assign_member_scalar (SCALAR *head, ENTITY *new, char *name)
{
  TABLE *stuff;

  EASSERT (head, scalar, 0);
  assert (name != NULL);

  switch (scalar_member_search (name))
    {
    case ScalarClass:
    case ScalarType:
      fail ("Assignment to scalar member \"%s\" is not allowed.", name);
      delete_2_entities (ENT (head), ENT (new));
      FREE_CHAR (name);
      raise_exception ();
    default:
      stuff = (TABLE *) (head->stuff ? copy_entity (head->stuff) :
			 make_table ());
      head = (SCALAR *) dup_scalar (head);
      head->stuff = (TABLE *) replace_in_table (stuff, new, name);
      break;
    }
  return (ENT (head));
}

ENTITY *
assign_member_vector (VECTOR * volatile head, ENTITY *new, char *name)
{
  TABLE *stuff;

  EASSERT (head, vector, 0);
  assert (name != NULL);

  switch (vector_member_search (name))
    {
    case VectorEid:
      if (head->eid == new)
	{
	  delete_entity (new);
	}
      else
	{
	  if (new != NULL)
	    {
	      if (new->class != vector)
		{
		  WITH_HANDLING
		  {
		    new = vector_entity (EAT (new));
		  }
		  ON_EXCEPTION
		  {
		    delete_2_entities (ENT (head), new);
		    FREE_CHAR (name);
		  }
		  END_EXCEPTION;
		}
	      if (((VECTOR *) new)->ne != head->ne)
		{
		  fail ("Inconsistent dimensions for vector label.  Vector has %d element%s, but label has %d element%s.",
			head->ne, PLURAL (head->ne), ((VECTOR *) new)->ne, PLURAL (((VECTOR *) new)->ne));
		  delete_2_entities (ENT (head), new);
		  FREE_CHAR (name);
		  raise_exception ();
		}
	    }
	  stuff = (TABLE *) (head->stuff ?
			     copy_entity (head->stuff) : NULL);
	  head = (VECTOR *) dup_vector (head);
	  head->stuff = stuff;
	  delete_entity (head->eid);
	  head->eid = new;
	}
      FREE_CHAR (name);
      break;
    case VectorClass:
    case VectorType:
    case VectorDensity:
    case VectorNe:
    case VectorNn:
    case VectorOrder:
      fail ("Assignment to vector member \"%s\" is not allowed.", name);
      delete_2_entities (ENT (head), ENT (new));
      FREE_CHAR (name);
      raise_exception ();
    default:
      stuff = (TABLE *) (head->stuff ?
			 copy_entity (head->stuff) : make_table ());
      head = (VECTOR *) dup_vector (head);
      head->stuff = (TABLE *) replace_in_table (stuff, new, name);
      break;
    }
  return (ENT (head));
}

ENTITY *
assign_member_matrix (MATRIX * volatile head, ENTITY *new, char *name)
{
  TABLE *stuff;

  EASSERT (head, matrix, 0);
  assert (name != NULL);

  switch (matrix_member_search (name))
    {
    case MatrixRid:
      if (head->rid == new)
	{			/* Nothing to be done? */
	  delete_entity (new);
	}
      else
	{
	  if (new != NULL)
	    {
	      if (new->class != vector)
		{
		  WITH_HANDLING
		  {
		    new = vector_entity (EAT (new));
		  }
		  ON_EXCEPTION
		  {
		    delete_2_entities (ENT (head), new);
		    FREE_CHAR (name);
		  }
		  END_EXCEPTION;
		}
	      if (((VECTOR *) new)->ne != head->nr)
		{
		  fail ("Inconsistent dimensions for matrix label.  Matrix has %d row%s, but label has %d element%s.",
			head->nr, PLURAL (head->nr), ((VECTOR *) new)->ne, PLURAL (((VECTOR *) new)->ne));
		  delete_2_entities (ENT (head), new);
		  FREE_CHAR (name);
		  raise_exception ();
		}
	    }
	  stuff = (TABLE *) (head->stuff ?
			     copy_entity (head->stuff) : NULL);
	  head = (MATRIX *) dup_matrix (head);
	  head->stuff = stuff;
	  delete_entity (head->rid);
	  head->rid = new;
	}
      FREE_CHAR (name);
      break;
    case MatrixCid:
      if (head->cid == new)
	{			/* Nothing to be done? */
	  delete_entity (new);
	}
      else
	{
	  if (new != NULL)
	    {
	      if (new->class != vector)
		{
		  WITH_HANDLING
		  {
		    new = vector_entity (EAT (new));
		  }
		  ON_EXCEPTION
		  {
		    delete_2_entities (ENT (head), new);
		    FREE_CHAR (name);
		  }
		  END_EXCEPTION;
		}
	      if (((VECTOR *) new)->ne != head->nc)
		{
		  fail ("Inconsistent dimensions for matrix label.  Matrix has %d column%s, but label has %d element%s.",
			head->nc, PLURAL (head->nc), ((VECTOR *) new)->ne, PLURAL (((VECTOR *) new)->ne));
		  delete_2_entities (ENT (head), new);
		  FREE_CHAR (name);
		  raise_exception ();
		}
	    }
	  stuff = (TABLE *) (head->stuff ?
			     copy_entity (head->stuff) : NULL);
	  head = (MATRIX *) dup_matrix (head);
	  head->stuff = stuff;
	  delete_entity (head->cid);
	  head->cid = new;
	}
      FREE_CHAR (name);
      break;
    case MatrixClass:
    case MatrixType:
    case MatrixSymmetry:
    case MatrixDensity:
    case MatrixNr:
    case MatrixNc:
    case MatrixNn:
    case MatrixOrder:
      fail ("Assignment to matrix member \"%s\" is not allowed.", name);
      delete_2_entities (ENT (head), ENT (new));
      FREE_CHAR (name);
      raise_exception ();
    default:
      stuff = (TABLE *) (head->stuff ?
			 copy_entity (head->stuff) : make_table ());
      head = (MATRIX *) dup_matrix (head);
      head->stuff = (TABLE *) replace_in_table (stuff, new, name);
      break;
    }
  return (ENT (head));
}

ENTITY *
assign_member_function (FUNCTION *head, ENTITY *new, char *name)
{
  TABLE *stuff;

  EASSERT (head, function, 0);
  assert (name != NULL);

  switch (function_member_search (name))
    {
    case FunctionClass:
    case FunctionIlk:
      fail ("Assignment to function member \"%s\" is not allowed.", name);
      delete_2_entities (ENT (head), ENT (new));
      FREE_CHAR (name);
      raise_exception ();
    default:
      stuff = (TABLE *) (head->stuff ?
			 copy_entity (head->stuff) : make_table ());
      head = (FUNCTION *) dup_function (head);
      head->stuff = (TABLE *) replace_in_table (stuff, new, name);
      break;
    }
  return (ENT (head));
}

ENTITY *
reference_member_scalar (SCALAR *head, char *name)
{
  ENTITY *p;

  EASSERT (head, scalar, 0);
  assert (name != NULL);

  switch (scalar_member_search (name))
    {
    case ScalarClass:
      p = char_to_scalar (dup_char (class_string[scalar]));
      FREE_CHAR (name);
      break;
    case ScalarType:
      p = char_to_scalar (dup_char (type_string[head->type]));
      FREE_CHAR (name);
      break;
    default:
      if (head->stuff)
	{
	  p = search_in_table ((TABLE *) copy_table (head->stuff), name);
	}
      else
	{
	  FREE_CHAR (name);
	  p = NULL;
	}
    }
  delete_scalar (head);
  return (p);
}

ENTITY *
reference_member_vector (VECTOR *head, char *name)
{
  ENTITY *p;

  EASSERT (head, vector, 0);
  assert (name != NULL);

  switch (vector_member_search (name))
    {
    case VectorClass:
      p = char_to_scalar (dup_char (class_string[vector]));
      FREE_CHAR (name);
      break;
    case VectorEid:
      p = (head->eid == NULL) ? NULL : copy_entity (head->eid);
      FREE_CHAR (name);
      break;
    case VectorDensity:
      p = char_to_scalar (dup_char (density_string[head->density]));
      FREE_CHAR (name);
      break;
    case VectorNe:
      p = int_to_scalar (head->ne);
      FREE_CHAR (name);
      break;
    case VectorNn:
      p = int_to_scalar (head->nn);
      FREE_CHAR (name);
      break;
    case VectorOrder:
      p = char_to_scalar (dup_char (order_string[head->order]));
      FREE_CHAR (name);
      break;
    case VectorType:
      p = char_to_scalar (dup_char (type_string[head->type]));
      FREE_CHAR (name);
      break;
    default:
      if (head->stuff)
	{
	  p = search_in_table ((TABLE *) copy_table (head->stuff), name);
	}
      else
	{
	  FREE_CHAR (name);
	  p = NULL;
	}
    }
  delete_vector (head);
  return (p);
}

ENTITY *
reference_member_matrix (MATRIX *head, char *name)
{
  ENTITY *p;

  EASSERT (head, matrix, 0);
  assert (name != NULL);

  switch (matrix_member_search (name))
    {
    case MatrixClass:
      p = char_to_scalar (dup_char (class_string[matrix]));
      FREE_CHAR (name);
      break;
    case MatrixRid:
      p = (head->rid == NULL) ? NULL : copy_entity (head->rid);
      FREE_CHAR (name);
      break;
    case MatrixCid:
      p = (head->cid == NULL) ? NULL : copy_entity (head->cid);
      FREE_CHAR (name);
      break;
    case MatrixDensity:
      p = char_to_scalar (dup_char (density_string[head->density]));
      FREE_CHAR (name);
      break;
    case MatrixSymmetry:
      p = char_to_scalar (dup_char (symmetry_string[head->symmetry]));
      FREE_CHAR (name);
      break;
    case MatrixNr:
      p = int_to_scalar (head->nr);
      FREE_CHAR (name);
      break;
    case MatrixNc:
      p = int_to_scalar (head->nc);
      FREE_CHAR (name);
      break;
    case MatrixNn:
      p = int_to_scalar (head->nn);
      FREE_CHAR (name);
      break;
    case MatrixOrder:
      p = char_to_scalar (dup_char (order_string[head->order]));
      FREE_CHAR (name);
      break;
    case MatrixType:
      p = char_to_scalar (dup_char (type_string[head->type]));
      FREE_CHAR (name);
      break;
    default:
      if (head->stuff)
	{
	  p = search_in_table ((TABLE *) copy_table (head->stuff), name);
	}
      else
	{
	  FREE_CHAR (name);
	  p = NULL;
	}
    }
  delete_matrix (head);
  return (p);
}

ENTITY *
reference_member_function (FUNCTION *head, char *name)
{
  ENTITY *p;

  EASSERT (head, function, 0);
  assert (name != NULL);

  switch (function_member_search (name))
    {
    case FunctionClass:
      p = char_to_scalar (dup_char (class_string[function]));
      FREE_CHAR (name);
      break;
    case FunctionIlk:
      p = char_to_scalar (dup_char (ilk_string[head->ilk]));
      FREE_CHAR (name);
      break;
    default:
      if (head->stuff)
	{
	  p = search_in_table ((TABLE *) copy_table (head->stuff), name);
	}
      else
	{
	  FREE_CHAR (name);
	  p = NULL;
	}
    }
  delete_entity (ENT (head));
  return (p);
}

ENTITY *
member_names (ENTITY *t)
{
  /*
   * This function returns a vector containing the names of
   * the members of the given entity.
   */

  int i;
  VECTOR *names, *v;

  EASSERT (t, 0, 0);

  switch (t->class)
    {

    case scalar:
      names = (VECTOR *) form_vector (END_Scalar, character, dense);
      for (i = 0; i < END_Scalar; i++)
	{
	  names->a.character[i] = dup_char (scalar_member_names[i].name);
	}
      if (((SCALAR *) t)->stuff)
	{
	  v = (VECTOR *) member_names (copy_table (((SCALAR *) t)->stuff));
	  names = (VECTOR *) append_vector_character (names, v);
	}
      names = (VECTOR *) sort_vector (names, 0);
      break;

    case vector:
      names = (VECTOR *) form_vector (END_Vector, character, dense);
      for (i = 0; i < END_Vector; i++)
	{
	  names->a.character[i] = dup_char (vector_member_names[i].name);
	}
      if (((VECTOR *) t)->stuff)
	{
	  v = (VECTOR *) member_names (copy_table (((VECTOR *) t)->stuff));
	  names = (VECTOR *) append_vector_character (names, v);
	}
      names = (VECTOR *) sort_vector (names, 0);
      break;

    case matrix:
      names = (VECTOR *) form_vector (END_Matrix, character, dense);
      for (i = 0; i < END_Matrix; i++)
	{
	  names->a.character[i] = dup_char (matrix_member_names[i].name);
	}
      if (((MATRIX *) t)->stuff)
	{
	  v = (VECTOR *) member_names (copy_table (((MATRIX *) t)->stuff));
	  names = (VECTOR *) append_vector_character (names, v);
	}
      names = (VECTOR *) sort_vector (names, 0);
      break;

    case function:
      names = (VECTOR *) form_vector (END_Function, character, dense);
      for (i = 0; i < END_Function; i++)
	{
	  names->a.character[i] = dup_char (function_member_names[i].name);
	}
      if (((FUNCTION *) t)->stuff)
	{
	  v = (VECTOR *) member_names (copy_table (((FUNCTION *) t)->stuff));
	  names = (VECTOR *) append_vector_character (names, v);
	}
      names = (VECTOR *) sort_vector (names, 0);
      break;

    case table:
      names = (VECTOR *) form_vector (((TABLE *) t)->nm,
				      character, dense);
      if (names->ne > 0)
	{
	  get_table_names (((TABLE *) t)->table_node,
			   names->a.character);
	}
      break;

    default:
      BAD_CLASS (t->class);
      delete_entity (t);
      raise_exception ();
    }

  delete_entity (t);
  return (ENT (names));
}
