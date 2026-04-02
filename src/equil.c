/*
   equil.c -- matrix equilibration

   Copyright (C) 2001-2003  K. Scott Hunziker.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: equil.c,v 1.5 2003/10/05 01:14:48 ksh Exp $";

#include "equil.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "transpose.h"
#include "lapack.h"
#include "dense.h"
#include "cast.h"
#include "full.h"

#if HAVE_LIBSUPERLU
#include "superlu.h"
#endif

ENTITY *
bi_equilibrate (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
    case vector:
      p = matrix_entity (p);	/* fall through */
    case matrix:
      return equilibrate_matrix ((MATRIX *) p);
    default:
      fail ("Can't apply \"equilibrate\" to a %s.", class_string[p->class]);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
equilibrate_matrix (MATRIX *p)
{
  /*
   * Find row and column scalings intended to equilibrate a matrix and
   * reduce its condition number.
   */

  ENTITY *result = NULL;
  int i, m, n, info;
  REAL rowcnd, colcnd, amax;
  REAL *r = NULL;
  REAL *c = NULL;
  ENTITY *e_r = NULL;
  ENTITY *e_rowcnd = NULL;

#if HAVE_LIBSUPERLU
  SuperMatrix a;
  NCformat v;
#endif

  EASSERT (p, matrix, 0);

  WITH_HANDLING
  {

    /* Gotta have elements. */

    if (!p->nr || !p->nc)
      {
	fail ("Matrix has no elements.");
	raise_exception ();
      }

    switch (p->density)
      {
      case sparse:
      case sparse_upper:

#if HAVE_LIBSUPERLU
	{
	  /* Need full storage. */

	  if (p->density == sparse_upper)
	    p = (MATRIX *) full_matrix (EAT (p));

	  /* Can't be all zeros. */

	  if (!p->nn)
	    {
	      fail ("Matrix elements are all zero.");
	      raise_exception ();
	    }

	  a.Stype = SLU_NC;

	  switch (p->type)
	    {
	    case integer:

	      p = (MATRIX *) cast_matrix_integer_real (EAT (p));
	      /* fall through */

	    case real:

	      a.Dtype = SLU_D;
	      break;

	    case complex:

	      a.Dtype = SLU_Z;
	      break;

	    default:

	      BAD_TYPE (p->type);
	      raise_exception ();
	    }

	  a.Mtype = SLU_GE;

	  /*
	   *  SuperLU wants compressed column storage, but we have it
	   *  in compressed row form.  We'll give it what we have, then
	   *  swap row and column outputs.
	   */

	  n = a.nrow = p->nc;
	  m = a.ncol = p->nr;
	  a.Store = &v;

	  v.nnz = p->nn;
	  v.nzval = (void *) p->a.ptr;

	  /* We're 1-based, SuperLU is 0-based. */

	  v.rowind = dup_mem (p->ja, p->nn * sizeof (int));
	  v.colptr = dup_mem (p->ia, (p->nr+1) * sizeof (int));
	  for (i=0; i<p->nn; i++) v.rowind[i]--;
	  for (i=0; i<p->nr+1; i++) v.colptr[i]--;

	  r = MALLOC (p->nr * sizeof(REAL));
	  c = MALLOC (p->nc * sizeof(REAL));

	  SIGINT_RAISE_ON ();
	  if (p->type == real)
	    dgsequ (&a, c, r, &colcnd, &rowcnd, &amax, &info);
	  else
	    zgsequ (&a, c, r, &colcnd, &rowcnd, &amax, &info);
	  SIGINT_RAISE_OFF ();

	  FREE (v.rowind);
	  FREE (v.colptr);

	  if (info < 0)
	    {
	      fail ("Bad argument, %s argument to %s.",
		    TH(-info), (p->type == real)? "dgsequ" : "zgsequ");
	      raise_exception ();
	    }
	}
	break;

#else
	/* Deal with dense only, for now. */

	p = (MATRIX *) dense_matrix (EAT (p));
	/* fall through */
#endif

      case dense:

	switch (p->type)
	  {
	  case integer:

	    p = (MATRIX *) cast_matrix_integer_real (EAT (p));
	    /* fall through */

	  case real:

	    switch (p->symmetry)
	      {
	      case general:

		m = p->nr;
		n = p->nc;
		r = MALLOC (m * sizeof(REAL));
		c = MALLOC (n * sizeof(REAL));

		SIGINT_RAISE_ON ();
		DGEEQU (&m, &n, p->a.real, &m,
			r, c, &rowcnd, &colcnd, &amax, &info);
		SIGINT_RAISE_OFF ();

		if (info < 0)
		  {
		    fail ("Bad argument to DGEEQU.");
		    raise_exception ();
		  }

		break;

	      case symmetric:

		/* must be positive definite */

		m = p->nr;
		r = MALLOC (m * sizeof(REAL));

		SIGINT_RAISE_ON ();
		DPOEQU (&m, p->a.real, &m, r, &rowcnd, &amax, &info);
		SIGINT_RAISE_OFF ();

		if (info < 0)
		  {
		    fail ("Bad argument to DPOEQU.");
		    raise_exception ();
		  }

		break;

	      default:

		BAD_SYMMETRY (p->symmetry);
		raise_exception ();
	      }

	    break;

	  case complex:

	    switch (p->symmetry)
	      {
	      case symmetric:

		/* Can't do this.  Convert to general symmetry. */

		assert (p->density == dense);
		p = (MATRIX *) dup_matrix (EAT (p));
		p->symmetry = general;
		/* fall through */

	      case general:

		m = p->nr;
		n = p->nc;
		r = MALLOC (m * sizeof(REAL));
		c = MALLOC (n * sizeof(REAL));

		SIGINT_RAISE_ON ();
		ZGEEQU (&m, &n, p->a.complex, &m,
			r, c, &rowcnd, &colcnd, &amax, &info);
		SIGINT_RAISE_OFF ();

		if (info < 0)
		  {
		    fail ("Bad argument to ZGEEQU.");
		    raise_exception ();
		  }

		break;

	      case hermitian:

		/* must be positive definite */

		m = p->nr;
		r = MALLOC (m * sizeof(REAL));

		SIGINT_RAISE_ON ();
		ZPOEQU (&m, p->a.complex, &m, r, &rowcnd, &amax, &info);
		SIGINT_RAISE_OFF ();

		if (info < 0)
		  {
		    fail ("Bad argument to ZPOEQU.");
		    raise_exception ();
		  }

		break;

	      default:

		BAD_SYMMETRY (p->symmetry);
		raise_exception ();
	      }

	    break;

	  default:

	    BAD_TYPE (p->type);
	    raise_exception ();
	  }

	break;

      default:

	BAD_DENSITY (p->density);
	raise_exception ();
      }

    if (info)
      {
	/* If nonzero, info has row or column info. */

	result = int_to_scalar ((info > m) ? m-info : info);
      }
    else
      {
	result = make_table ();
	e_r = gift_wrap_vector (m, real, r);
	e_rowcnd = real_to_scalar (rowcnd);

	result = replace_in_table (EAT (result),
				   copy_entity (e_r),
				   dup_char ("r"));
	result = replace_in_table (EAT (result),
				   copy_entity (e_rowcnd),
				   dup_char ("rowcnd"));
	result = replace_in_table (EAT (result),
				   real_to_scalar (amax),
				   dup_char ("amax"));
	if (c)
	  {
	    delete_2_entities (e_r, e_rowcnd);
	    e_r = e_rowcnd = NULL;

	    result = replace_in_table (EAT (result),
				       gift_wrap_vector (n, real, c),
				       dup_char ("c"));
	    result = replace_in_table (EAT (result),
				       real_to_scalar (colcnd),
				       dup_char ("colcnd"));
	  }
	else
	  {
	    result = replace_in_table (EAT (result),
				       EAT (e_r),
				       dup_char ("c"));
	    result = replace_in_table (EAT (result),
				       EAT (e_rowcnd),
				       dup_char ("colcnd"));
	  }
      }

  }
  ON_EXCEPTION
  {
    delete_matrix (p);
    delete_3_entities (result, e_r, e_rowcnd);
    TFREE (r);
    TFREE (c);
#if HAVE_LIBSUPERLU
    TFREE (v.rowind);
    TFREE (v.colptr);
#endif
  }
  END_EXCEPTION;

  delete_matrix (p);
  return ENT (result);
}
