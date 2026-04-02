/*
   filter.c -- digital filter

   Copyright (C) 1995-2003  K. Scott Hunziker.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: filter.c,v 1.4 2003/11/30 07:47:12 ksh Exp $";

#include "filter.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "dense.h"
#include "blas.h"
#include "cast.h"

ENTITY *
bi_filter (int n, ENTITY *b, ENTITY *a, ENTITY *x, ENTITY *z)
{
  /* A "direct form II transposed" digital filter. */

  WITH_HANDLING
    {
      b = dense_vector ((VECTOR *) vector_entity (EAT (b)));
      a = dense_vector ((VECTOR *) vector_entity (EAT (a)));
      x = dense_vector ((VECTOR *) vector_entity (EAT (x)));

      if ( z ) z = dense_vector ((VECTOR *) vector_entity (EAT (z)));
    }
  ON_EXCEPTION
    {
      delete_2_entities (b, a);
      delete_2_entities (x, z);
    }
  END_EXCEPTION;

  return filter_vector ((VECTOR *) b, (VECTOR *) a,
			(VECTOR *) x, (VECTOR *) z);
}

static char *arg_type_msg =
  "filter: Bad type for %s argument -- must be real.";

#define REQ_REAL(x,ord) \
    switch (x->type) \
      { \
      case integer: \
	x = (VECTOR *) cast_vector_integer_real (EAT (x)); \
	break; \
      case real: \
	break; \
      default: \
	fail (arg_type_msg, ord); \
	raise_exception (); \
      }

ENTITY *
filter_vector (VECTOR *b, VECTOR *a, VECTOR *x, VECTOR *z)
{
  VECTOR *y = NULL;
  TABLE *t = NULL;
  REAL *ap, *bp, *xp, *yp, *zp;
  REAL a0, s, z0;
  int i, k, n, an, bn, xn, nm;

  EASSERT (b, vector, 0);
  EASSERT (a, vector, 0);
  EASSERT (x, vector, 0);

  WITH_HANDLING
  {
    REQ_REAL (b, "first");
    REQ_REAL (a, "second");
    REQ_REAL (x, "third");

    a0 = a->a.real[0];
    if (!a0)
      {
	fail ("filter: First term in `A' must not be zero.");
	raise_exception();
      }
    s = 1.0 / a0;

    /*
     * It's convenient to make numerator and denominator vectors the
     * same length.  We'll go to a bit of effort here to use dup_vector
     * if we can, since it's a big win if the vector's ref_count is 1.
     */

    an = a->ne;
    bn = b->ne;
    n = (an > bn) ? an : bn;
    ap = a->a.real;
    bp = b->a.real;

    if (an == n)
      {
	int j = 1;
	a = (VECTOR *) dup_vector (EAT (a));
	DSCAL (&n, &s, a->a.real, &j);
      }
    else
      {
	VECTOR *aa;
	REAL *aap;

	aa = (VECTOR *) make_vector (n, real, dense);
	aap = aa->a.real;
	for (i=1; i<an; i++) aap[i] = ap[i] * s;

	delete_vector (a);
	EAT (a);
	a = aa;
      }
    ap = a->a.real;

    if (bn == n)
      {
	int j = 1;
	b = (VECTOR *) dup_vector (EAT (b));
	DSCAL (&n, &s, b->a.real, &j);
      }
    else
      {
	VECTOR *bb;
	REAL *bbp;

	bb = (VECTOR *) make_vector (n, real, dense);
	bbp = bb->a.real;
	for (i=0; i<bn; i++) bbp[i] = bp[i] * s;

	delete_vector (b);
	EAT (b);
	b = bb;
      }
    bp = b->a.real;
	
    xn = x->ne;
    y = (VECTOR *) form_vector (xn, real, dense);

    nm = n - 1;
    if ( z )
      {
	REQ_REAL (z, "fourth");
	if (z->ne != n)
	  {
	    fail ("filter: Fourth arg has wrong length -- must be the same length as the longer of the first and second args.");
	    raise_exception();
	  }
	z = (VECTOR *) dup_vector (EAT (z));
	zp = z->a.real;
	for (i=0; i<nm; i++) zp[i] = zp[i+1];
	zp[nm] = 0.0;
      }
    else
      {
	z = (VECTOR *) make_vector (n, real, dense);
	zp = z->a.real;
      }

    xp = x->a.real;
    yp = y->a.real;
    for (i=0; i<xn; i++)
      {
        POLL_SIGINT ();
	z0 = yp[i] = bp[0] * xp[i] + zp[0];
	for (k=1; k<n; k++)
	  zp[k-1] = bp[k] * xp[i] - ap[k] * z0 + zp[k];
      }

    for (i=nm; i>0; i--) zp[i] = zp[i-1];
    zp[0] = z0;

    t = (TABLE *) make_table ();
    t = (TABLE *) replace_in_table (EAT (t), EAT (y), dup_char ("y"));
    t = (TABLE *) replace_in_table (EAT (t), EAT (z), dup_char ("z"));

  }
  ON_EXCEPTION
  {
    delete_3_vectors (b, a, x);
    delete_2_vectors (y, z);
    delete_table (t);
  }
  END_EXCEPTION;

  delete_3_vectors (b, a, x);
  return ENT (t);
}
