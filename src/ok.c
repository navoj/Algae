/*
   ok.c -- Check an entity for internal consistency.

   Copyright (C) 1994-2001  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: ok.c,v 1.4 2001/07/21 18:52:40 ksh Exp $";

#include "ok.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "function.h"

int
ok_entity (ENTITY *ip)
{
  if (ip == NULL)
    {
      fail ("NULL entity pointer.");
      return (0);
    }

  if (ip->ref_count <= 0)
    {
      fail ("Bad \"ref_count\".");
      return (0);
    }

  switch (ip->class)
    {
    case scalar:
      switch (((SCALAR *) ip)->type)
	{
	case integer:
	case real:
	case complex:
	  break;
	case character:
	  if (((SCALAR *) ip)->v.character == NULL)
	    {
	      fail ("A scalar's character pointer is NULL.");
	      return (0);
	    }
	  break;
	default:
	  fail ("Bad \"%s\" type for scalar.",
		type_string[((SCALAR *) ip)->type]);
	  return (0);
	}

      if (((SCALAR *) ip)->stuff != NULL &&
	  !ok_entity (ENT (((SCALAR *) ip)->stuff)))
	return (0);

      break;
    case vector:
      switch (((VECTOR *) ip)->type)
	{
	case integer:
	case real:
	case complex:
	case character:
	  break;
	default:
	  fail ("Bad \"%s\" type for vector.",
		type_string[((VECTOR *) ip)->type]);
	  return (0);
	}

      switch (((VECTOR *) ip)->order)
	{
	case ordered:
	case not_ordered:
	  break;
	default:
	  fail ("Bad \"%s\" order for vector.",
		order_string[((VECTOR *) ip)->order]);
	  return (0);
	}

      if (((VECTOR *) ip)->eid != NULL &&
	  !ok_entity (((VECTOR *) ip)->eid))
	return (0);

      if (((VECTOR *) ip)->ne < 0)
	{
	  fail ("Bad vector dimension.");
	  return (0);
	}

      if (((VECTOR *) ip)->nn > 0 && ((VECTOR *) ip)->a.integer == NULL)
	{
	  fail ("Pointer to vector data is NULL.");
	  return (0);
	}

      switch (((VECTOR *) ip)->density)
	{
	case dense:
	  if (((VECTOR *) ip)->order != ordered)
	    {
	      fail ("Can't have an unordered, dense vector.");
	      return (0);
	    }
	  if (((VECTOR *) ip)->nn != ((VECTOR *) ip)->ne)
	    {
	      fail ("Invalid combination of \"ne\" and \"nn\" for a dense vector.");
	      return (0);
	    }
	  if (((VECTOR *) ip)->ja != NULL)
	    {
	      fail ("Sparse information stored for a dense vector.");
	      return (0);
	    }
	  break;
	case sparse:
	  if (((VECTOR *) ip)->nn > ((VECTOR *) ip)->ne)
	    {
	      fail ("Bad vector dimension.");
	      return (0);
	    }
	  if (((VECTOR *) ip)->nn > 0 && ((VECTOR *) ip)->ja == NULL)
	    {
	      fail ("No \"ja\" vector for vector.");
	      return (0);
	    }
	  break;
	default:
	  fail ("Bad density for vector.");
	  return (0);
	}

      if (((VECTOR *) ip)->stuff != NULL &&
	  !ok_entity (ENT (((VECTOR *) ip)->stuff)))
	return (0);

      break;
    case matrix:
      switch (((MATRIX *) ip)->symmetry)
	{
	case general:
	  break;
	case symmetric:
	  if (((MATRIX *) ip)->nr != ((MATRIX *) ip)->nc)
	    {
	      fail ("Bad symmetry for rectangular matrix.");
	      return (0);
	    }
	  break;
	case hermitian:
	  if (((MATRIX *) ip)->nr != ((MATRIX *) ip)->nc)
	    {
	      fail ("Bad symmetry for rectangular matrix.");
	      return (0);
	    }
	  if (((MATRIX *) ip)->type == character)
	    {
	      fail ("Character matrix has invalid symmetry.");
	      return (0);
	    }
	  if (((MATRIX *) ip)->type == integer ||
	      ((MATRIX *) ip)->type == real)
	    {
	      fail ("Real matrix has hermitian symmetry.");
	      return (0);
	    }
	  break;
	default:
	  fail ("Bad \"%s\" symmetry for matrix.",
		symmetry_string[((MATRIX *) ip)->symmetry]);
	  return (0);
	}

      switch (((MATRIX *) ip)->type)
	{
	case integer:
	case real:
	case complex:
	case character:
	  break;
	default:
	  fail ("Bad \"%s\" type for matrix.",
		type_string[((MATRIX *) ip)->type]);
	  return (0);
	}

      switch (((MATRIX *) ip)->order)
	{
	case ordered:
	case not_ordered:
	  break;
	default:
	  fail ("Bad \"%s\" order for matrix.",
		order_string[((MATRIX *) ip)->order]);
	  return (0);
	}

      if (((MATRIX *) ip)->rid != NULL &&
	  !ok_entity (((MATRIX *) ip)->rid))
	return (0);
      if (((MATRIX *) ip)->cid != NULL &&
	  !ok_entity (((MATRIX *) ip)->cid))
	return (0);

      if (((MATRIX *) ip)->nr < 0 || ((MATRIX *) ip)->nc < 0)
	{
	  fail ("Bad matrix dimensions.");
	  return (0);
	}

      if (((MATRIX *) ip)->nn > 0 && ((MATRIX *) ip)->a.integer == NULL)
	{
	  fail ("Pointer to matrix data is NULL.");
	  return (0);
	}

      switch (((MATRIX *) ip)->density)
	{
	case dense:
	  if (((MATRIX *) ip)->order != ordered)
	    {
	      fail ("Can't have an unordered, dense matrix.");
	      return (0);
	    }
	  if (((MATRIX *) ip)->nn != ((MATRIX *) ip)->nr * ((MATRIX *) ip)->nc)
	    {
	      fail ("Invalid combination of \"nr\", \"nc\", and \"nn\" for a dense matrix.");
	      return (0);
	    }
	  if (((MATRIX *) ip)->ia != NULL ||
	      ((MATRIX *) ip)->ja != NULL ||
	      ((MATRIX *) ip)->d.integer != NULL)
	    {
	      fail ("Sparse information stored for a dense matrix.");
	      return (0);
	    }
	  break;
	case sparse:
	  if ((double) ((MATRIX *) ip)->nn >
	      (double) ((MATRIX *) ip)->nr * (double) ((MATRIX *) ip)->nc)
	    {
	      fail ("Bad matrix dimensions.");
	      return (0);
	    }
	  if (((MATRIX *) ip)->nn > 0 && ((MATRIX *) ip)->ia == NULL)
	    {
	      fail ("No \"ia\" vector for matrix.");
	      return (0);
	    }
	  if (((MATRIX *) ip)->nn > 0 && ((MATRIX *) ip)->ia[0] != 1)
	    {
	      fail ("Bad \"ia\" vector for matrix.");
	      return (0);
	    }
	  if (((MATRIX *) ip)->nn > 0 && ((MATRIX *) ip)->ia[((MATRIX *) ip)->nr] != ((MATRIX *) ip)->nn + 1)
	    {
	      fail ("Bad \"ia\" vector for matrix.");
	      return (0);
	    }
	  if (((MATRIX *) ip)->nn > 0 && ((MATRIX *) ip)->ja == NULL)
	    {
	      fail ("No \"ja\" vector for matrix.");
	      return (0);
	    }
	  if (((MATRIX *) ip)->d.integer != NULL)
	    {
	      fail ("Diagonal data stored for \"sparse\" matrix.");
	      return (0);
	    }
	  break;
	case sparse_upper:
	  if (((MATRIX *) ip)->nr != ((MATRIX *) ip)->nc)
	    {
	      fail ("Rectangular matrix is stored in \"sparse_upper\" form.");
	      return (0);
	    }
	  if ((double) ((MATRIX *) ip)->nn >
	      (double) ((MATRIX *) ip)->nr * (double) ((MATRIX *) ip)->nc)
	    {
	      fail ("Bad matrix dimensions.");
	      return (0);
	    }
	  if (((MATRIX *) ip)->nn > 0 && ((MATRIX *) ip)->ia == NULL)
	    {
	      fail ("No \"ia\" vector for matrix.");
	      return (0);
	    }
	  if (((MATRIX *) ip)->nn > 0 && ((MATRIX *) ip)->ia[0] != 1)
	    {
	      fail ("Bad \"ia\" vector for matrix.");
	      return (0);
	    }
	  if (((MATRIX *) ip)->nn > 0 && ((MATRIX *) ip)->ja == NULL)
	    {
	      fail ("No \"ja\" vector for matrix.");
	      return (0);
	    }
	  break;
	default:
	  fail ("Bad density for matrix.");
	  return (0);
	}

      if (((MATRIX *) ip)->stuff != NULL &&
	  !ok_entity (ENT (((MATRIX *) ip)->stuff)))
	return (0);

      break;
    case table:
      if (((TABLE *) ip)->table_node == NULL)
	{
	  fail ("NULL table_node pointer.");
	  return (0);
	}
      if (((TABLE *) ip)->table_node->name == NULL)
	{
	  fail ("NULL table_node name.");
	  return (0);
	}
      if (((TABLE *) ip)->table_node->left == NULL)
	{
	  fail ("NULL table_node \"left\".");
	  return (0);
	}
      if (((TABLE *) ip)->table_node->right == NULL)
	{
	  fail ("NULL table_node \"right\".");
	  return (0);
	}
      break;
    case function:
      break;
    default:
      fail ("Bad \"%s\" class.", class_string[ip->class]);
      return (0);
    }
  return (1);
}
