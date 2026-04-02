
/*
   rand.c -- Random numbers.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: rand.c,v 1.1.1.1 1996/04/17 05:56:21 ksh Exp $";

#include <time.h>
#include "rand.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "zero.h"
#include "dense.h"
#include "cast.h"
#include "partition.h"

/*
 * Prefer to use drand48, otherwise use random, or rand as a last resort.
 */

#if HAVE_DRAND48

#if !HAVE_DRAND48_DECL
double PROTO (drand48, (void));
void PROTO (srand48, (long seed));
#endif
#define RANDOM		drand48
#define SRANDOM(s)	srand48( (long) (s) )
#define RANDOM_NUMBER	RANDOM()

#else /* not HAVE_DRAND48 */

/*
 * The largest number returned by the random number generator is
 * RANDOM_MAX.  If we're using `rand' it's RAND_MAX, but if we're
 * using `random' it's 2^31-1.
 */
#ifndef RANDOM_MAX
#if !HAVE_RANDOM
#define RANDOM_MAX	RAND_MAX
#else
#define RANDOM_MAX	2147483647.0
#endif
#endif

#if !HAVE_RANDOM

#define RANDOM	rand
#define SRANDOM	srand

#else /* HAVE_RANDOM */

#define RANDOM	random
#define SRANDOM	srandom

#endif /* HAVE_RANDOM */

/* 0 <= RANDOM_NUMBER <= 1 */
#define RANDOM_NUMBER (((double)RANDOM())/(double)RANDOM_MAX)

#endif /* not HAVE_DRAND48 */

int
rand_init (void)
{
  /* Initialize the random number seed based on the time of day. */

  int s = (int) time (NULL);
  SRANDOM (s);
  return (s);
}

ENTITY *
bi_srand (int n, ENTITY *s)
{
  /*
   * This function sets the seed for the random number generator.
   * If no argument is given, the seed is based on the time of day.
   */

  int i;

  if (n == 0)
    {
      i = rand_init ();
    }
  else
    {
      EASSERT (s, 0, 0);
      assert (n == 1);
      i = entity_to_int (s);
      SRANDOM (i);
    }

  return (int_to_scalar (i));
}

ENTITY *
bi_rand (int n, ENTITY *shape)
{
  /*
   * Generate random real numbers uniformly distributed between
   * zero and one.  The class of entity returned depends on the
   * number of arguments.  If `shape' is NULL, a scalar is returned.
   * If `shape' is has 1 element, a vector with `shape[1]' elements is
   * returned.  If `shape' has 2 elements, a matrix with `shape[1]' rows
   * and `shape[2]' columns is returned.
   */


  if (!shape)
    {

      /* No shape, so return scalar. */

      return (real_to_scalar (RANDOM_NUMBER));

    }
  else
    {

      shape = cast_vector ((VECTOR *) vector_entity (shape), integer);

      switch (((VECTOR *) shape)->ne)
	{
	case 0:
	  delete_vector ((VECTOR *) shape);
	  return (real_to_scalar (RANDOM_NUMBER));
	case 1:
	  {
	    int i;
	    int ne = get_vector_element_integer ((VECTOR *) shape, 1);
	    VECTOR *r;

	    if (ne < 0)
	      {
		fail ("Specified dimension is negative.");
		raise_exception ();
	      }

	    r = (VECTOR *) form_vector (ne, real, dense);
	    for (i = 0; i < r->nn; i++)
	      r->a.real[i] = RANDOM_NUMBER;
	    return (ENT (r));
	  }
	case 2:
	  {
	    int i;
	    MATRIX *r;
	    int nr = get_vector_element_integer ((VECTOR *) copy_vector ((VECTOR *) shape), 1);
	    int nc = get_vector_element_integer ((VECTOR *) shape, 2);

	    if (nr < 0 || nc < 0)
	      {
		fail ("Specified dimension is negative.");
		raise_exception ();
	      }

	    r = (MATRIX *) form_matrix (nr, nc, real, dense);
	    r->symmetry = general;
	    for (i = 0; i < r->nn; i++)
	      r->a.real[i] = RANDOM_NUMBER;
	    return (ENT (r));
	  }
	default:
	  fail ("Too many (%d) dimensions specified in \"rand\".",
		((VECTOR *) shape)->ne);
	  delete_vector ((VECTOR *) shape);
	  raise_exception ();
	}
    }
}
