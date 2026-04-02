/*
   sort.c -- Sorting routines.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: sort.c,v 1.3 1997/05/14 05:22:16 ksh Exp $";

#include "sort.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "dense.h"
#include "generate.h"
#include "partition.h"


static void *isort_ptr;	 /* for communication with comparison functions */

ENTITY *
bi_sort (ENTITY *p)
{
  /* Sort a vector. */

  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (sort_vector ((VECTOR *) scalar_to_vector ((SCALAR *) p), 0));
    case vector:
      return (sort_vector ((VECTOR *) p, 0));
    case matrix:
      return (sort_vector ((VECTOR *) matrix_to_vector ((MATRIX *) p), 0));
    default:
      fail ("Can't sort a %s entity.", class_string[p->class]);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
bi_isort (ENTITY *p)
{
  /* Sort vector, return index. */

  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (sort_vector ((VECTOR *) scalar_to_vector ((SCALAR *) p), 1));
    case vector:
      return (sort_vector ((VECTOR *) p, 1));
    case matrix:
      return (sort_vector ((VECTOR *) matrix_to_vector ((MATRIX *) p), 1));
    default:
      fail ("Can't sort a %s entity.", class_string[p->class]);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
sort_vector (VECTOR *p, int ind)
{
  /*
   * This function sorts vector `p' in increasing order.  If `ind' is
   * zero, the sorted vector is returned.  Otherwise, a vector of the
   * corresponding indices is returned.
   */

  VECTOR *index;

  EASSERT (p, vector, 0);

  p = (VECTOR *) dup_entity (dense_vector (p));
  index = (VECTOR *) ((p->ne > 0) ?
                      generate_integer (1, p->ne, 1) :
                      make_vector (0, integer, dense));
    
  switch (p->type)
    {
    case integer:
      isort_vector_integer (p->ne, p->a.integer, index->a.integer);
      break;
    case real:
      isort_vector_real (p->ne, p->a.real, index->a.integer);
      break;
    case complex:
      isort_vector_complex (p->ne, p->a.complex, index->a.integer);
      break;
    case character:
      isort_vector_character (p->ne, p->a.character, index->a.integer);
      break;
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  p = (VECTOR *) partition_vector_vector (p, (VECTOR *) copy_vector (index));

  if (ind)
    {
      delete_vector (p);
      return (ENT (index));
    }
  else
    {
      if (p->eid == NULL)
        p->eid = ENT (index);
      else
        delete_vector (index);

      return ENT (p);
    }
}

static int
compare_int_indirect (const void *a, const void *b)
{
  return ((int *)isort_ptr)[*((int *)a)-1] -
    ((int *)isort_ptr)[*((int *)b)-1];
}

static int
compare_real_indirect (const void *a, const void *b)
{
  REAL p = ((REAL *)isort_ptr)[*((int *)a)-1] -
    ((REAL *)isort_ptr)[*((int *)b)-1];
  return (p < 0) ? -1 : (p > 0) ? 1 : 0;
}

static int
compare_complex_indirect (const void *a, const void *b)
{
  REAL p;
  p = (((COMPLEX *)isort_ptr)[*((int *)a)-1]).real -
    (((COMPLEX *)isort_ptr)[*((int *)b)-1]).real;
  if (!p)
    p = (((COMPLEX *)isort_ptr)[*((int *)a)-1]).imag -
      (((COMPLEX *)isort_ptr)[*((int *)b)-1]).imag;
  return (p < 0) ? -1 : (p > 0) ? 1 : 0;
}

static int
compare_char_indirect (const void *a, const void *b)
{
  return strcmp (((char **)isort_ptr)[*((int *)a)-1],
                 ((char **)isort_ptr)[*((int *)b)-1]);
}

void
isort_vector_integer (int right, int b[], int x[])
{
  /*
   * Sort integer vector into increasing order.
   */

  isort_ptr = b;
  qsort (x, right, sizeof (int), compare_int_indirect);
}

void
isort_vector_real (int right, REAL b[], int x[])
{
  isort_ptr = b;
  qsort (x, right, sizeof (int), compare_real_indirect);
}

void
isort_vector_complex (int right, COMPLEX b[], int x[])
{
  isort_ptr = b;
  qsort (x, right, sizeof (int), compare_complex_indirect);
}

void
isort_vector_character (int right, char *b[], int x[])
{
  isort_ptr = b;
  qsort (x, right, sizeof (int), compare_char_indirect);
}

int
compare_integer (const void *a, const void *b)
{
  return *((int *) a) - *((int *) b);
}
