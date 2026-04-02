/*
   arpack.c -- Interface to the ARPACK routines for solution of
   eigenvalue problems with implicitly restarted Arnoldi methods.

   Copyright (C) 2002-2003  K. Scott Hunziker.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: arpack.c,v 1.3 2003/08/01 04:57:46 ksh Exp $";

#include "arpack.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "function.h"
#include "cast.h"
#include "dense.h"
#include "test.h"
#include "datum.h"

/*
 *  The iram function is an interface to the ARPACK routines,
 *  providing almost all of their functionality.  One thing missing is
 *  the user-supplied shift capability.
 *
 *  ARPACK solves three categories of problems:
 *	1.  real symmetric
 *	2.  real general (non-symmetric)
 *	3.  complex (regardless of symmetry)
 *
 *  The `options' argument is a table whose members provide the problem
 *  specifications.  These specifications vary according to the problem
 *  category, and so are described separately below.  However, the problem
 *  category itself is determined by the values of the `type' and `symmetry'
 *  members:
 *
 *	type:	   may be "real" (default) or "complex".
 *	symmetry:  may be "symmetric", "hermitian", or "general" (default).
 *
 *  
 *  result = iram (n; op_func; b_func; params; options);
 *
 *	n:
 *	    The dimension of the problem.  This should be an integer scalar,
 *	    or at least able to be cast to one.
 *
 *      op_func:
 *          A function performing the "w <- OP v" operation.  The
 *          operation required depends on the solution mode.  In the
 *          simplest case it is just the matrix-vector multiplication
 *          A*v.  This function is called with two arguments: `v' and
 *          `params'.  Argument `v' is the vector on which to operate.
 *          This is a real or complex vector (depending on the problem
 *          category), with length equal to the dimension of the
 *          eigenproblem.  The argument `params' is passed unchanged
 *          from the original call to `iram'.  It is intended to
 *          contain the coefficient arrays that define the problem.
 *
 *	b_func:
 *          A function performing the "w <- B v" operation.  It is not
 *          used at all for some modes, in which case it may be NULL.
 *          This function is called with two arguments: `v' and
 *          `params'.  Argument `v' is the vector on which to operate.
 *          This is a real or complex vector (depending on the problem
 *          category), with length equal to the dimension of the
 *          eigenproblem.  The argument `params' is passed unchanged
 *          from the original call to `iram_ds'.  It is intended to
 *          contain the coefficient arrays that define the problem.
 *          The matrix B must be hermitian.
 *
 *      params:
 *          A table intended to contain the coefficient arrays.  This table
 *	    is simply passed on to the `op_func' and `b_func' functions,
 *	    and is not referenced by `iram' at all.
 *
 *      options:
 *          A table containing problem specifications.  Details are given
 *	    below for each of the problem categories.
 */

#define EVAL_STACK_SIZE 32
#define SIT( t, n )	search_in_table ((TABLE *) copy_table (t), dup_char(n))
#define RIT( t, v, n )	t = (TABLE *) replace_in_table (EAT(t), v, dup_char(n))

ENTITY *
bi_iram (ENTITY *dim, ENTITY *op_func, ENTITY *b_func, ENTITY *params, ENTITY *options)
{
  ENTITY *type_e = NULL;
  ENTITY *sym_e = NULL;

  volatile TYPE type;
  char *type_s;
  volatile SYMMETRY sym;
  char *sym_s;

  WITH_HANDLING
  {
    /* Get type and symmetry. */

    if (options && options->class != table)
      {
	fail ("Options argument must be a table.");
	raise_exception ();
      }

    if (options && (type_e = SIT (options, "type")))
      {
	type_s = entity_to_string (EAT (type_e));
	if (!strcmp (type_s, "real"))
	  type = real;
	else if (!strcmp (type_s, "complex"))
	  type = complex;
	else
	  {
	    fail ("Bad type (%s) in iram.", type_s);
	    raise_exception ();
	  }
      }
    else
      type = real;

    if (options && (sym_e = SIT (options, "symmetry")))
      {
	sym_s = entity_to_string (EAT (sym_e));
	if (!strcmp (sym_s, "symmetric"))
	  sym = symmetric;
	else if (!strcmp (sym_s, "general") || !strcmp (sym_s, "hermitian"))
	  sym = general;
	else
	  {
	    fail ("Bad symmetry (%s) in iram.", sym_s);
	    raise_exception ();
	  }
      }
    else
      sym = general;
  }
  ON_EXCEPTION
  {
    delete_3_entities (dim, op_func, b_func);
    delete_2_entities (params, options);
    delete_2_entities (type_e, sym_e);
  }
  END_EXCEPTION;

  if (type == complex)
    return iram_zn_entity (dim, op_func, b_func, params, options);
  else
    if (sym == symmetric)
      return iram_ds_entity (dim, op_func, b_func, params, options);
    else
      /*
       * FIX ME!  The current version of ARPACK does not work with
       * the DLAHQR routine from version 3.0 of LAPACK.  This means
       * that the non-symmetric ARPACK code is unreliable.  For now,
       * we'll just use the complex code instead.  When ARPACK gets
       * updated, change the line below to call iram_dn_entity.
       */
      return iram_zn_entity (dim, op_func, b_func, params, options);
}

/*
 *  The `iram_ds_entity' function is an interface to the ARPACK
 *  routines dsaupd and dseupd.  This is for real, symmetric problems.
 *
 *  In addition to the the `type' and `symmetry' members described above,
 *  the `options' argument contains the following:
 *
 *	mode:
 *	    The solution mode, which must be 1, 3, 4, or 5.
 *          Mode 1 is the default.  This is IPARAM(7) from ARPACK.
 *
 *	bmat:
 *          The type of problem.  Must be "I" for standard or "G"
 *	    for generalized.  The default is "I".
 *
 *	which:
 *          Specifies which of the Ritz values of OP to compute.
 *	    Must be "LA", "SA", "LM", "SM", or "BE".  The default
 *	    is "LA".
 *
 *	nev:
 *          The number of eigenvalues to be computed.  Must be
 *	    greater than zero and less than the problem dimension.
 *	    The default is the lesser of 10 and n/2.
 *
 *	ncv:
 *          Controls the number of Lanczos vectors generated at
 *	    each iteration.  Must be less than or equal to n, and
 *	    must exceed nev.  The default is the lesser of 2*nev and n.
 *
 *	mxiter:
 *          The maximum number of Arnoldi update iterations allowed.
 *	    The default is 100.
 *
 *	basis:
 *          If "true", return an orthonormal (Lanczos) basis along
 *	    with other results.
 *
 *	vectors:
 *          If "true", return the eigenvectors along with other
 *	    results.
 *
 *	tol:
 *          The stopping criterion.  If `tol' is NULL or less than
 *	    or equal to zero, then machine precision is used.
 *
 *	resid:
 *          If present, this is an n-length vector that provides the
 *	    initial residual vector.  Otherwise, a random initial
 *	    vector is used.
 *
 *	shift:
 *          A real scalar providing the shift point for a spectral
 *	    transformation.  The default is 0.0.
 */

#define EVAL_STACK_SIZE 32
#define SIT( t, n )	search_in_table ((TABLE *) copy_table (t), dup_char(n))
#define RIT( t, v, n )	t = (TABLE *) replace_in_table (EAT(t), v, dup_char(n))

ENTITY *
iram_ds_entity (ENTITY *dim, ENTITY *op_func, ENTITY *b_func, ENTITY *params, ENTITY *options)
{
  ENTITY *mode_e = NULL;
  ENTITY *bmat_e = NULL;
  ENTITY *which_e = NULL;
  ENTITY *nev_e = NULL;
  ENTITY *ncv_e = NULL;
  ENTITY *mxiter_e = NULL;
  ENTITY *tol_e = NULL;
  ENTITY *shift_e = NULL;
  ENTITY *resid_e = NULL;
  ENTITY *basis_e = NULL;
  ENTITY *vectors_e = NULL;
  MATRIX *v = NULL;
  MATRIX *vec = NULL;
  VECTOR *val = NULL;
  ENTITY *r = NULL;
  VECTOR *y;
  TABLE *ret = NULL;

  int n, mode, bmat, which, nev, ncv, mxiter, info, ido, lworkl;
  int basis, vectors;
  int iparam[11], ipntr[11];
  int *dummy;

  char *bmat_s;
  char *which_s;
  REAL tol, shift;
  REAL *workd = NULL;
  REAL *workl = NULL;
  int *select = NULL;

  DATUM exe_args[EVAL_STACK_SIZE];

  WITH_HANDLING
  {
    /* Check the dimension. */

    assert (dim);
    n = entity_to_int (EAT (dim));
    if (n < 2)
      {
	fail ("Problem dimension must exceed 1.");
	raise_exception ();
      }

    /* Check the OP function. */

    assert (op_func);
    if (op_func->class != function)
      {
	fail ("Second arg to iram must be a function.");
	raise_exception ();
      }

    /* Check the B function. */

    if (b_func && op_func->class != function)
      {
	fail ("Third arg to iram must be a function (or NULL).");
	raise_exception ();
      }

    /* Get the options. */

    if (options && options->class != table)
      {
	fail ("Options argument must be a table.");
	raise_exception ();
      }

    if (options && (mode_e = SIT (options, "mode")))
      {
	mode = entity_to_int (EAT (mode_e));
	if (mode < 1 || mode > 5)
	  {
	    fail ("Mode %d is invalid.", mode);
	    raise_exception ();
	  }

	/*
	 *  For the symmetric problem in mode 2, there are actually three
	 *  operations required:  y=inv(B)*A*x, y=B*x, and y=A*x.  We can't
	 *  do that, since we've only made provision for two user functions.
	 *  Just use the nonsymmetric code, instead.
	 */

	if (mode == 2)
	  {
	    fail ("Mode 2 is not supported for symmetric problems.");
	    raise_exception ();
	  }
      }
    else
      mode = 1;

    if (options && (bmat_e = SIT (options, "bmat")))
      {
	bmat_s = entity_to_string (EAT (bmat_e));
	if (!strcmp (bmat_s, "I"))
	  bmat = 1;
	else if (!strcmp (bmat_s, "G"))
	  {
	    bmat = 2;
	    if (!b_func)
	      {
		fail ("Generalized problem requested, but second function is NULL.");
		raise_exception ();
	      }
	  }
	else
	  {
	    fail ("Bad bmat value (%s).", bmat_s);
	    raise_exception ();
	  }
      }
    else
      bmat = 1;

    if (options && (which_e = SIT (options, "which")))
      {
	which_s = entity_to_string (EAT (which_e));
	if (!strcmp (which_s, "LA"))
	  which = 1;
	else if (!strcmp (which_s, "SA"))
	  which = 2;
	else if (!strcmp (which_s, "LM"))
	  which = 3;
	else if (!strcmp (which_s, "SM"))
	  which = 4;
	else if (!strcmp (which_s, "BE"))
	  which = 5;
	else
	  {
	    fail ("Bad which value (%s).", which_s);
	    raise_exception ();
	  }
      }
    else
      which = 1;

    if (options && (nev_e = SIT (options, "nev")))
      {
	nev = entity_to_int (EAT (nev_e));
	if (nev < 1 || nev >= n)
	  {
	    fail ("Invalid number of eigenvalues (%d).", nev);
	    raise_exception ();
	  }
      }
    else
      nev = (n > 20) ? 10 : n/2;

    if (options && (ncv_e = SIT (options, "ncv")))
      {
	ncv = entity_to_int (EAT (ncv_e));
	if (ncv < 1 || ncv > n)
	  {
	    fail ("Invalid ncv value (%d).", ncv);
	    raise_exception ();
	  }
	if (ncv <= nev)
	  {
	    fail ("Bad ncv value (%d); must exceed nev (%d).", ncv, nev);
	    raise_exception ();
	  }
      }
    else
      ncv = (2*nev > n) ? n : 2*nev;

    if (options && (mxiter_e = SIT (options, "mxiter")))
      {
	mxiter = entity_to_int (EAT (mxiter_e));
	if (mxiter < 1)
	  {
	    fail ("Invalid mxiter value (%d).", mxiter);
	    raise_exception ();
	  }
      }
    else
      mxiter = 100;

    if (options && (tol_e = SIT (options, "tol")))
      tol = entity_to_real (EAT (tol_e));
    else
      tol = 0.0;

    if (options && (shift_e = SIT (options, "shift")))
      shift = entity_to_real (EAT (shift_e));
    else
      shift = 0.0;

    if (options && (basis_e = SIT (options, "basis")))
      basis = test_entity (EAT (basis_e));
    else
      basis = 0;

    if (options && (vectors_e = SIT (options, "vectors")))
      vectors = test_entity (EAT (vectors_e));
    else
      vectors = 0;

    if (options && (resid_e = SIT (options, "resid")))
      {
	resid_e = dense_vector ((VECTOR *) vector_entity (EAT (resid_e)));
	resid_e = dup_vector ((VECTOR *) cast_vector (EAT (resid_e), real));
	if (((VECTOR *)resid_e)->ne != n)
	  {
	    fail ("Wrong length for resid vector; must equal problem dimension.");
	    raise_exception ();
	  }
	info = 1;
      }
    else
      {
	resid_e = form_vector (n, real, dense);
	info = 0;
      }

    /* Set up working arrays. */

    v = (MATRIX *) form_matrix (n, ncv, real, dense);

    iparam[0] = 1;		/* exact shifts */
    iparam[2] = mxiter;		/* max number of Arnoldi iterations */
    iparam[3] = 1;		/* blocksize */
    iparam[6] = mode;		/* solution mode */

    workd = MALLOC (3*n*sizeof(REAL));

    lworkl = ncv * (ncv + 8);
    workl = MALLOC (lworkl*sizeof(REAL));

    y = (VECTOR *) gift_wrap_vector (n, real, NULL);

    for (ido=0; ido != 99; )
      {
	RSAUPD (&ido, &bmat, &n, &which, &nev, &tol,
		((VECTOR *)resid_e)->a.real,
		&ncv, ((MATRIX *)v)->a.real, &n, iparam, ipntr, workd, workl,
		&lworkl, &info);

	switch (ido)
	  {
	  case 1:
	  case -1:	/* compute y <- OP * x */

	    y->a.real = workd + ipntr[0] - 1;
	    exe_args[0].data.ptr = copy_vector (y);
	    exe_args[0].type = D_ENTITY;
	    exe_args[1].data.ptr = params ? copy_entity (params) : NULL;
	    exe_args[1].type = D_ENTITY;

	    r = execute_function ((FUNCTION *) copy_function (op_func),
				  2, exe_args, EVAL_STACK_SIZE - 2);
	    r = dense_vector ((VECTOR *) vector_entity (EAT (r)));

	    if (((VECTOR *) r)->type == integer)
	      r = cast_vector_integer_real ((VECTOR *) EAT (r));

	    if (((VECTOR *) r)->type != real)
	      {
		fail ("Function returns %s; should be real.",
		      type_string[((VECTOR *) r)->type]);
		raise_exception ();
	      }
	    if (((VECTOR *) r)->ne != n)
	      {
		fail ("Function returns vector with incorrect length.");
		raise_exception ();
	      }

	    memcpy (workd + ipntr[1] - 1,
		    ((VECTOR *) r)->a.real, n*sizeof (REAL));

	    delete_entity (r); r = NULL;
	    break;

	  case 2:

	    y->a.real = workd + ipntr[0] - 1;
	    exe_args[0].data.ptr = copy_vector (y);
	    exe_args[0].type = D_ENTITY;
	    exe_args[1].data.ptr = params ? copy_entity (params) : NULL;
	    exe_args[1].type = D_ENTITY;

	    r = execute_function ((FUNCTION *) copy_function (b_func),
				  2, exe_args, EVAL_STACK_SIZE - 2);
	    r = dense_vector ((VECTOR *) vector_entity (EAT (r)));

	    if (((VECTOR *) r)->type == integer)
	      r = cast_vector_integer_real ((VECTOR *) EAT (r));

	    if (((VECTOR *) r)->type != real)
	      {
		fail ("Function returns %s; should be real.",
		      type_string[((VECTOR *) r)->type]);
		raise_exception ();
	      }
	    if (((VECTOR *) r)->ne != n)
	      {
		fail ("Function returns vector with incorrect length.");
		raise_exception ();
	      }

	    memcpy (workd + ipntr[1] - 1,
		    ((VECTOR *) r)->a.real, n*sizeof (REAL));

	    delete_entity (r); r = NULL;
	    break;

	  case 99:
	    /* normal exit */
	    break;

	  default:

	    fail ("Unknown return from DSAUPD.");
	    raise_exception ();
	  }
      }

    /* Deal with error returns. */

    switch (info)
      {
      case 0:
	/* normal exit */
	break;

      case 1:
	fail ("Iteration limit exceeded.");
	raise_exception ();

      case 3:
	fail ("No shifts could be applied during a cycle of the implicitly restarted Arnoldi iteration.  One possible fix is to increase the size of NCV relative to NEV.");
	raise_exception ();

      case -1:
      case -2:
      case -3:
      case -4:
      case -5:
      case -6:
      case -10:
      case -12:
	fail ("Input error to DSAUPD.  This is likely a bug.");
	raise_exception ();

      case -7:
	fail ("Length of WORKL is not sufficient.");
	raise_exception ();

      case -8:
	fail ("Error return from tridiagonal eigenvalue calculation.");
	raise_exception ();

      case -9:
	fail ("Starting vector is zero.");
	raise_exception ();

      case -11:
	fail ("Incompatible options.  Mode 1 cannot be used for generalized eigenvalue problems.");
	raise_exception ();

      case -13:
	fail ("NEV and WHICH are incompatible.");
	raise_exception ();

      case -9999:
	fail ("Could not build an Arnoldi factorization.  May be a bug.");
	raise_exception ();

      default:
	fail ("Unknown error from DSAUPD.");
	raise_exception ();
      }

    /* Now compute eigenvalues and vectors. */

    if (vectors && basis)
      vec = (MATRIX *) form_matrix (n, nev, real, dense);
    else
      vec = (MATRIX *) copy_matrix
	((MATRIX *) gift_wrap_matrix (n, nev, real, v->a.real));

    val = (VECTOR *) form_vector (nev, real, dense);
    select = MALLOC (ncv * sizeof(int));

    RSEUPD (&vectors, dummy, select, val->a.real, vec->a.real, &n, &shift,
	    &bmat, &n, &which, &nev, &tol, ((VECTOR *)resid_e)->a.real, &ncv,
	    v->a.real, &n, iparam, ipntr, workd, workl, &lworkl, &info);

    switch (info)
      {
      case 0:
	/* normal exit */
	break;

      case -1:
      case -2:
      case -3:
      case -5:
      case -6:
      case -10:
      case -15:
      case -16:
	fail ("Input error to DSEUPD.  This is likely a bug.");
	raise_exception ();

      case -7:
	fail ("Length of WORKL is not sufficient.");
	raise_exception ();

      case -8:
	fail ("Error return from tridiagonal eigenvalue calculation.");
	raise_exception ();

      case -9:
	fail ("Starting vector is zero.");
	raise_exception ();

      case -11:
	fail ("Incompatible options.  Mode 1 cannot be used for generalized eigenvalue problems.");
	raise_exception ();

      case -12:
	fail ("NEV and WHICH are incompatible.");
	raise_exception ();

      case -14:
	fail ("No eigenvalues found to sufficient accuracy.");
	raise_exception ();

      default:
	fail ("Unknown error from DSEUPD.");
	raise_exception ();
      }

    /* Free the work space. */

    y->a.real = NULL;
    delete_vector (y);

    FREE (EAT (workd));
    FREE (EAT (workl));
    FREE (EAT (select));

    /* Now put together the output table. */

    ret = (TABLE *) make_table ();
    RIT (ret, EAT (val), "values");
    if (vectors) RIT (ret, EAT (vec), "vectors");
    if (basis) RIT (ret, EAT (v), "basis");
    RIT (ret, EAT (resid_e), "resid");
    RIT (ret, int_to_scalar (iparam[2]), "iter");
    RIT (ret, int_to_scalar (iparam[4]), "nconv");
  }
  ON_EXCEPTION
  {
    TFREE (workd);
    TFREE (workl);
    TFREE (select);
    delete_3_entities (mode_e, bmat_e, which_e);
    delete_3_entities (nev_e, ncv_e, mxiter_e);
    delete_3_entities (tol_e, shift_e, resid_e);
    delete_3_entities (basis_e, vectors_e, r);
    delete_3_entities (dim, op_func, b_func);
    delete_2_entities (params, options);
    delete_2_matrices (v, vec);
    delete_vector (val);
    delete_table (ret);
  }
  END_EXCEPTION;

  delete_3_entities (op_func, b_func, params);
  delete_entity (options);

  return ENT (ret);
}

/*
 *  The `iram_dn_entity' function is an interface to the ARPACK
 *  routines dnaupd and dneupd.  This is for real, general problems.
 *
 *  Although ARPACK allows complex shifts for this problem, that capability
 *  is not supported here -- use `iram_zn_entity' instead.
 *
 *  In addition to the the `type' and `symmetry' members described above,
 *  the `options' argument contains the following:
 *
 *	mode:
 *	    The solution mode, which must be 1, 2, or 3.
 *          Mode 1 is the default.  This is IPARAM(7) from ARPACK.
 *
 *	bmat:
 *          The type of problem.  Must be "I" for standard or "G"
 *	    for generalized.  The default is "I".
 *
 *	which:
 *          Specifies which of the Ritz values of OP to compute.
 *	    Must be "LM", "SM", "LR", "SR", "LI", or "SI".  The default
 *	    is "LR".
 *
 *	nev:
 *          The number of eigenvalues to be computed.  Must be
 *	    greater than zero and less than the problem dimension
 *	    minus one.  The default is the lesser of 10 and n/2.
 *
 *	ncv:
 *          Controls the number of Arnoldi vectors generated at
 *	    each iteration.  Must be less than or equal to n, and
 *	    must exceed nev by at least two.  The default is the lesser
 *	    of 2*nev+1 and n.
 *
 *	mxiter:
 *          The maximum number of Arnoldi update iterations allowed.
 *	    The default is 100.
 *
 *	basis:
 *          If "true", returns, along with other results, an orthonormal
 *	    basis for the associated approximate invariant subspace.
 *
 *	vectors:
 *          If "true", return the eigenvectors along with the other
 *	    results.
 *
 *	tol:
 *          The stopping criterion.  If `tol' is NULL or less than
 *	    or equal to zero, then machine precision is used.
 *
 *	resid:
 *          If present, this is an n-length vector that provides the
 *	    initial residual vector.  Otherwise, a random initial
 *	    vector is used.
 *
 *	shift:
 *          A real scalar providing the shift point for a spectral
 *	    transformation.  The default is 0.0.
 */

ENTITY *
iram_dn_entity (ENTITY *dim, ENTITY *op_func, ENTITY *b_func, ENTITY *params, ENTITY *options)
{
  ENTITY *mode_e = NULL;
  ENTITY *bmat_e = NULL;
  ENTITY *which_e = NULL;
  ENTITY *nev_e = NULL;
  ENTITY *ncv_e = NULL;
  ENTITY *mxiter_e = NULL;
  ENTITY *tol_e = NULL;
  ENTITY *shift_e = NULL;
  ENTITY *resid_e = NULL;
  ENTITY *basis_e = NULL;
  ENTITY *vectors_e = NULL;
  MATRIX *v = NULL;
  MATRIX *vec = NULL;
  VECTOR *val = NULL;
  VECTOR * volatile vali = NULL;
  ENTITY *r = NULL;
  VECTOR *y;
  TABLE *ret = NULL;

  int n, mode, bmat, which, nev, ncv, mxiter, info, ido, lworkl, i, j;
  int basis, vectors, keep_nev;
  int iparam[11], ipntr[14];
  int *dummy;

  char *bmat_s;
  char *which_s;
  REAL tol, shift, shifti;
  REAL *workd = NULL;
  REAL *workl = NULL;
  REAL *workev = NULL;
  int *select = NULL;

  DATUM exe_args[EVAL_STACK_SIZE];

  WITH_HANDLING
  {
    /* Check the dimension. */

    assert (dim);
    n = entity_to_int (EAT (dim));
    if (n < 3)
      {
	fail ("Problem dimension must exceed 2.");
	raise_exception ();
      }

    /* Check the OP function. */

    assert (op_func);
    if (op_func->class != function)
      {
	fail ("Second arg to iram must be a function.");
	raise_exception ();
      }

    /* Check the B function. */

    if (b_func && op_func->class != function)
      {
	fail ("Third arg to iram must be a function (or NULL).");
	raise_exception ();
      }

    /* Get the options. */

    if (options && options->class != table)
      {
	fail ("Options argument must be a table.");
	raise_exception ();
      }

    if (options && (mode_e = SIT (options, "mode")))
      {
	mode = entity_to_int (EAT (mode_e));
	if (mode < 1 || mode > 5)
	  {
	    fail ("Mode %d is invalid.", mode);
	    raise_exception ();
	  }
	if (mode > 3)
	  {
	    fail ("Mode %d is not supported.", mode);
	    raise_exception ();
	  }
      }
    else
      mode = 1;

    if (options && (bmat_e = SIT (options, "bmat")))
      {
	bmat_s = entity_to_string (EAT (bmat_e));
	if (!strcmp (bmat_s, "I"))
	  bmat = 1;
	else if (!strcmp (bmat_s, "G"))
	  {
	    bmat = 2;
	    if (!b_func)
	      {
		fail ("Generalized problem requested, but second function is NULL.");
		raise_exception ();
	      }
	  }
	else
	  {
	    fail ("Bad bmat value (%s).", bmat_s);
	    raise_exception ();
	  }
      }
    else
      bmat = 1;

    if (options && (which_e = SIT (options, "which")))
      {
	which_s = entity_to_string (EAT (which_e));
	if (!strcmp (which_s, "LM"))
	  which = 1;
	else if (!strcmp (which_s, "SM"))
	  which = 2;
	else if (!strcmp (which_s, "LR"))
	  which = 3;
	else if (!strcmp (which_s, "SR"))
	  which = 4;
	else if (!strcmp (which_s, "LI"))
	  which = 5;
	else if (!strcmp (which_s, "SI"))
	  which = 6;
	else
	  {
	    fail ("Bad which value (%s).", which_s);
	    raise_exception ();
	  }
      }
    else
      which = 1;

    if (options && (nev_e = SIT (options, "nev")))
      {
	nev = entity_to_int (EAT (nev_e));
	if (nev < 1 || nev >= n-1)
	  {
	    fail ("Invalid number of eigenvalues (%d).", nev);
	    raise_exception ();
	  }
      }
    else
      nev = (n > 20) ? 10 : n/2;

    keep_nev = nev;

    if (options && (ncv_e = SIT (options, "ncv")))
      {
	ncv = entity_to_int (EAT (ncv_e));
	if (ncv < 1 || ncv > n)
	  {
	    fail ("Invalid ncv value (%d).", ncv);
	    raise_exception ();
	  }
	if (2 > ncv-nev)
	  {
	    fail ("Bad ncv value (%d).", ncv);
	    raise_exception ();
	  }
      }
    else
      ncv = (2*nev >= n) ? n : 2*nev + 1;

    if (options && (mxiter_e = SIT (options, "mxiter")))
      {
	mxiter = entity_to_int (EAT (mxiter_e));
	if (mxiter < 1)
	  {
	    fail ("Invalid mxiter value (%d).", mxiter);
	    raise_exception ();
	  }
      }
    else
      mxiter = 100;

    if (options && (tol_e = SIT (options, "tol")))
      tol = entity_to_real (EAT (tol_e));
    else
      tol = 0.0;

    shifti = 0.0;
    if (options && (shift_e = SIT (options, "shift")))
      shift = entity_to_real (EAT (shift_e));
    else
      shift = 0.0;

    if (options && (basis_e = SIT (options, "basis")))
      basis = test_entity (EAT (basis_e));
    else
      basis = 0;

    if (options && (vectors_e = SIT (options, "vectors")))
      vectors = test_entity (EAT (vectors_e));
    else
      vectors = 0;

    if (options && (resid_e = SIT (options, "resid")))
      {
	resid_e = dense_vector ((VECTOR *) vector_entity (EAT (resid_e)));
	resid_e = dup_vector ((VECTOR *) cast_vector (EAT (resid_e), real));
	if (((VECTOR *)resid_e)->ne != n)
	  {
	    fail ("Wrong length for resid vector; must equal problem dimension.");
	    raise_exception ();
	  }
	info = 1;
      }
    else
      {
	resid_e = form_vector (n, real, dense);
	info = 0;
      }

    /* Set up working arrays. */

    v = (MATRIX *) form_matrix (n, ncv, real, dense);

    iparam[0] = 1;		/* exact shifts */
    iparam[2] = mxiter;		/* max number of Arnoldi iterations */
    iparam[3] = 1;		/* blocksize */
    iparam[6] = mode;		/* solution mode */

    workd = MALLOC (3*n*sizeof(REAL));

    lworkl = 3 * ncv * (ncv + 2);
    workl = MALLOC (lworkl*sizeof(REAL));

    y = (VECTOR *) gift_wrap_vector (n, real, NULL);

    for (ido=0; ido != 99; )
      {
	RNAUPD (&ido, &bmat, &n, &which, &nev, &tol,
		((VECTOR *)resid_e)->a.real,
		&ncv, ((MATRIX *)v)->a.real, &n, iparam, ipntr, workd, workl,
		&lworkl, &info);

	switch (ido)
	  {
	  case 1:
	  case -1:	/* compute y <- OP * x */

	    y->a.real = workd + ipntr[0] - 1;
	    exe_args[0].data.ptr = copy_vector (y);
	    exe_args[0].type = D_ENTITY;
	    exe_args[1].data.ptr = params ? copy_entity (params) : NULL;
	    exe_args[1].type = D_ENTITY;

	    r = execute_function ((FUNCTION *) copy_function (op_func),
				  2, exe_args, EVAL_STACK_SIZE - 2);
	    r = dense_vector ((VECTOR *) vector_entity (EAT (r)));

	    if (((VECTOR *) r)->type == integer)
	      r = cast_vector_integer_real ((VECTOR *) EAT (r));

	    if (((VECTOR *) r)->type != real)
	      {
		fail ("Function returns %s; should be real.",
		      type_string[((VECTOR *) r)->type]);
		raise_exception ();
	      }
	    if (((VECTOR *) r)->ne != n)
	      {
		fail ("Function returns vector with incorrect length.");
		raise_exception ();
	      }

	    memcpy (workd + ipntr[1] - 1,
		    ((VECTOR *) r)->a.real, n*sizeof (REAL));

	    delete_entity (r); r = NULL;
	    break;

	  case 2:

	    y->a.real = workd + ipntr[0] - 1;
	    exe_args[0].data.ptr = copy_vector (y);
	    exe_args[0].type = D_ENTITY;
	    exe_args[1].data.ptr = params ? copy_entity (params) : NULL;
	    exe_args[1].type = D_ENTITY;

	    r = execute_function ((FUNCTION *) copy_function (b_func),
				  2, exe_args, EVAL_STACK_SIZE - 2);
	    r = dense_vector ((VECTOR *) vector_entity (EAT (r)));

	    if (((VECTOR *) r)->type == integer)
	      r = cast_vector_integer_real ((VECTOR *) EAT (r));

	    if (((VECTOR *) r)->type != real)
	      {
		fail ("Function returns %s; should be real.",
		      type_string[((VECTOR *) r)->type]);
		raise_exception ();
	      }
	    if (((VECTOR *) r)->ne != n)
	      {
		fail ("Function returns vector with incorrect length.");
		raise_exception ();
	      }

	    memcpy (workd + ipntr[1] - 1,
		    ((VECTOR *) r)->a.real, n*sizeof (REAL));

	    delete_entity (r); r = NULL;
	    break;

	  case 99:
	    /* normal exit */
	    break;

	  default:

	    fail ("Unknown return from DNAUPD.");
	    raise_exception ();
	  }
      }

    /* Deal with error returns. */

    switch (info)
      {
      case 0:
	/* normal exit */
	break;

      case 1:
	fail ("Iteration limit exceeded.");
	raise_exception ();

      case 3:
	fail ("No shifts could be applied during a cycle of the implicitly restarted Arnoldi iteration.  One possible fix is to increase the size of NCV relative to NEV.");
	raise_exception ();

      case -1:
      case -2:
      case -3:
      case -4:
      case -5:
      case -6:
      case -10:
      case -12:
	fail ("Input error to DNAUPD.  This is likely a bug.");
	raise_exception ();

      case -7:
	fail ("Length of WORKL is not sufficient.");
	raise_exception ();

      case -8:
	fail ("Error return from LAPACK eigenvalue calculation.");
	raise_exception ();

      case -9:
	fail ("Starting vector is zero.");
	raise_exception ();

      case -11:
	fail ("Incompatible options.  Mode 1 cannot be used for generalized eigenvalue problems.");
	raise_exception ();

      case -9999:
	fail ("Could not build an Arnoldi factorization.  May be a bug.");
	raise_exception ();

      default:
	fail ("Unknown error from DNAUPD.");
	raise_exception ();
      }

    /* Now compute eigenvalues and vectors. */

    workev = MALLOC (3*ncv*sizeof(REAL));

    vec = (MATRIX *) form_matrix (n, keep_nev+1, real, dense);
    val = (VECTOR *) form_vector (keep_nev+1, real, dense);
    vali = (VECTOR *) form_vector (keep_nev+1, real, dense);
    select = MALLOC (ncv * sizeof(int));

    RNEUPD (&vectors, dummy, select, val->a.real, vali->a.real, vec->a.real,
	    &n, &shift, &shifti, workev, &bmat, &n, &which, &nev, &tol,
	    ((VECTOR *)resid_e)->a.real, &ncv, v->a.real, &n, iparam,
	    ipntr, workd, workl, &lworkl, &info);

    switch (info)
      {
      case 0:
	/* normal exit */
	break;

      case 1:
	fail ("Internal error in DNEUPD.  This is likely an ARPACK bug.");
	raise_exception ();

      case -1:
      case -2:
      case -3:
      case -5:
      case -6:
      case -10:
      case -12:
      case -13:
      case -15:
	fail ("Input error to DNEUPD.  This is likely a bug.");
	raise_exception ();

      case -7:
	fail ("Length of WORKL is not sufficient.");
	raise_exception ();

      case -8:
	fail ("Error return from LAPACK routine DLAHQR.");
	raise_exception ();

      case -9:
	fail ("Error return from LAPACK routine DTREVC.");
	raise_exception ();

      case -11:
	fail ("Incompatible options.  Mode 1 cannot be used for generalized eigenvalue problems.");
	raise_exception ();

      case -14:
	fail ("No eigenvalues found to sufficient accuracy.");
	raise_exception ();

      default:
	fail ("Unknown error from DNEUPD.");
	raise_exception ();
      }

    /* Free the work space. */

    y->a.real = NULL;
    delete_vector (y);

    FREE (EAT (workd));
    FREE (EAT (workl));
    FREE (EAT (select));
    FREE (EAT (workev));

    /* We'll return complex eigenvalues and eigenvectors. */

    val = (VECTOR *) cast_vector_real_complex (EAT (val));

    if (vectors)
      {
	vec = (MATRIX *) cast_matrix_real_complex (EAT (vec));

	for (j=0; j<keep_nev; j++)
	  if (vali->a.real[j])
	    {
	      assert ((val->a.complex[j].real == val->a.complex[j+1].real)
		      || j >= keep_nev-1);
	      assert ((vali->a.real[j] == - vali->a.real[j+1])
		      || j >= keep_nev-1);

	      val->a.complex[j].imag = vali->a.real[j];
	      val->a.complex[j+1].imag = vali->a.real[j+1];

	      for (i=j*n; i<(j+1)*n; i++)
		{
		  vec->a.complex[i].imag = vec->a.complex[i+n].real;
		  vec->a.complex[i+n].imag = - vec->a.complex[i+n].real;
		  vec->a.complex[i+n].real = vec->a.complex[i].real;
		}

	      j++;
	    }
      }
    else
      {
	for (j=0; j<keep_nev; j++)
	  if (vali->a.real[j])
	    {
	      assert ((val->a.complex[j].real == val->a.complex[j+1].real)
		      || j >= keep_nev-1);
	      assert ((vali->a.real[j] == - vali->a.real[j+1])
		      || j>= keep_nev-1);

	      val->a.complex[j].imag = vali->a.real[j];
	      val->a.complex[j+1].imag = vali->a.real[j+1];

	      j++;
	    }
      }

    /* We gave them some extra space. */

    val->ne--; val->nn--;
    vec->nc--; vec->nn -= n;

    delete_vector (vali); vali = NULL;

    /* Now put together the output table. */

    ret = (TABLE *) make_table ();
    RIT (ret, EAT (val), "values");
    if (vectors) RIT (ret, EAT (vec), "vectors");
    if (basis) RIT (ret, EAT (v), "basis");
    RIT (ret, EAT (resid_e), "resid");
    RIT (ret, int_to_scalar (iparam[2]), "iter");
    RIT (ret, int_to_scalar (iparam[4]), "nconv");
  }
  ON_EXCEPTION
  {
    TFREE (workd);
    TFREE (workl);
    TFREE (select);
    delete_3_entities (mode_e, bmat_e, which_e);
    delete_3_entities (nev_e, ncv_e, mxiter_e);
    delete_3_entities (tol_e, shift_e, resid_e);
    delete_3_entities (basis_e, vectors_e, r);
    delete_3_entities (dim, op_func, b_func);
    delete_2_entities (params, options);
    delete_2_matrices (v, vec);
    delete_2_vectors (val, vali);
    delete_table (ret);
  }
  END_EXCEPTION;

  delete_3_entities (op_func, b_func, params);
  delete_2_matrices (v, vec);
  delete_entity (options);

  return ENT (ret);
}

/*
 *  The `iram_zn_entity' function is an interface to the ARPACK
 *  routines znaupd and zneupd.  This is for complex problems, regardless
 *  of symmetry.
 *
 *  In addition to the the `type' and `symmetry' members described above,
 *  the `options' argument contains the following:
 *
 *	mode:
 *	    The solution mode, which must be 1, 2, or 3.
 *          Mode 1 is the default.  This is IPARAM(7) from ARPACK.
 *
 *	bmat:
 *          The type of problem.  Must be "I" for standard or "G"
 *	    for generalized.  The default is "I".
 *
 *	which:
 *          Specifies which of the Ritz values of OP to compute.
 *	    Must be "LM", "SM", "LR", "SR", "LI", or "SI".  The default
 *	    is "LR".
 *
 *	nev:
 *          The number of eigenvalues to be computed.  Must be
 *	    greater than zero and less than the problem dimension
 *	    minus one.  The default is the lesser of 10 and n/2.
 *
 *	ncv:
 *          Controls the number of Arnoldi vectors generated at
 *	    each iteration.  Must be less than or equal to n, and
 *	    must exceed nev by at least one.  The default is the lesser
 *	    of 2*nev and n.
 *
 *	mxiter:
 *          The maximum number of Arnoldi update iterations allowed.
 *	    The default is 100.
 *
 *	basis:
 *          If "true", returns, along with other results, an orthonormal
 *	    basis for the associated approximate invariant subspace.
 *
 *	vectors:
 *          If "true", return the eigenvectors along with the other
 *	    results.
 *
 *	tol:
 *          The stopping criterion.  If `tol' is NULL or less than
 *	    or equal to zero, then machine precision is used.
 *
 *	resid:
 *          If present, this is an n-length vector that provides the
 *	    initial residual vector.  Otherwise, a random initial
 *	    vector is used.
 *
 *	shift:
 *          A real scalar providing the shift point for a spectral
 *	    transformation.  The default is 0.0.
 */

ENTITY *
iram_zn_entity (ENTITY *dim, ENTITY *op_func, ENTITY *b_func, ENTITY *params, ENTITY *options)
{
  ENTITY *mode_e = NULL;
  ENTITY *bmat_e = NULL;
  ENTITY *which_e = NULL;
  ENTITY *nev_e = NULL;
  ENTITY *ncv_e = NULL;
  ENTITY *mxiter_e = NULL;
  ENTITY *tol_e = NULL;
  ENTITY *shift_e = NULL;
  ENTITY *resid_e = NULL;
  ENTITY *basis_e = NULL;
  ENTITY *vectors_e = NULL;
  MATRIX *v = NULL;
  MATRIX *vec = NULL;
  VECTOR *val = NULL;
  ENTITY *r = NULL;
  VECTOR *y;
  TABLE *ret = NULL;

  int n, mode, bmat, which, nev, ncv, mxiter, info, ido, lworkl;
  int basis, vectors, keep_nev;
  int iparam[11], ipntr[14];
  int *dummy;

  char *bmat_s;
  char *which_s;
  REAL tol;
  COMPLEX shift;
  COMPLEX *workd = NULL;
  COMPLEX *workl = NULL;
  REAL *rwork = NULL;
  COMPLEX *workev = NULL;
  int *select = NULL;

  DATUM exe_args[EVAL_STACK_SIZE];

  WITH_HANDLING
  {
    /* Check the dimension. */

    assert (dim);
    n = entity_to_int (EAT (dim));
    if (n < 3)
      {
	fail ("Problem dimension must exceed 2.");
	raise_exception ();
      }

    /* Check the OP function. */

    assert (op_func);
    if (op_func->class != function)
      {
	fail ("Second arg to iram must be a function.");
	raise_exception ();
      }

    /* Check the B function. */

    if (b_func && op_func->class != function)
      {
	fail ("Third arg to iram must be a function (or NULL).");
	raise_exception ();
      }

    /* Get the options. */

    if (options && options->class != table)
      {
	fail ("Options argument must be a table.");
	raise_exception ();
      }

    if (options && (mode_e = SIT (options, "mode")))
      {
	mode = entity_to_int (EAT (mode_e));
	if (mode < 1 || mode > 3)
	  {
	    fail ("Mode %d is invalid.", mode);
	    raise_exception ();
	  }
      }
    else
      mode = 1;

    if (options && (bmat_e = SIT (options, "bmat")))
      {
	bmat_s = entity_to_string (EAT (bmat_e));
	if (!strcmp (bmat_s, "I"))
	  bmat = 1;
	else if (!strcmp (bmat_s, "G"))
	  {
	    bmat = 2;
	    if (!b_func)
	      {
		fail ("Generalized problem requested, but second function is NULL.");
		raise_exception ();
	      }
	  }
	else
	  {
	    fail ("Bad bmat value (%s).", bmat_s);
	    raise_exception ();
	  }
      }
    else
      bmat = 1;

    if (options && (which_e = SIT (options, "which")))
      {
	which_s = entity_to_string (EAT (which_e));
	if (!strcmp (which_s, "LM"))
	  which = 1;
	else if (!strcmp (which_s, "SM"))
	  which = 2;
	else if (!strcmp (which_s, "LR"))
	  which = 3;
	else if (!strcmp (which_s, "SR"))
	  which = 4;
	else if (!strcmp (which_s, "LI"))
	  which = 5;
	else if (!strcmp (which_s, "SI"))
	  which = 6;
	else
	  {
	    fail ("Bad which value (%s).", which_s);
	    raise_exception ();
	  }
      }
    else
      which = 1;

    if (options && (nev_e = SIT (options, "nev")))
      {
	nev = entity_to_int (EAT (nev_e));
	if (nev < 1 || nev >= n-1)
	  {
	    fail ("Invalid number of eigenvalues (%d).", nev);
	    raise_exception ();
	  }
      }
    else
      nev = (n > 20) ? 10 : n/2;

    keep_nev = nev;

    if (options && (ncv_e = SIT (options, "ncv")))
      {
	ncv = entity_to_int (EAT (ncv_e));
	if (ncv < 1 || ncv > n)
	  {
	    fail ("Invalid ncv value (%d).", ncv);
	    raise_exception ();
	  }
	if (1 > ncv-nev)
	  {
	    fail ("Bad ncv value (%d).", ncv);
	    raise_exception ();
	  }
      }
    else
      ncv = (2*nev > n) ? n : 2*nev;

    if (options && (mxiter_e = SIT (options, "mxiter")))
      {
	mxiter = entity_to_int (EAT (mxiter_e));
	if (mxiter < 1)
	  {
	    fail ("Invalid mxiter value (%d).", mxiter);
	    raise_exception ();
	  }
      }
    else
      mxiter = 100;

    if (options && (tol_e = SIT (options, "tol")))
      tol = entity_to_real (EAT (tol_e));
    else
      tol = 0.0;

    if (options && (shift_e = SIT (options, "shift")))
      shift = entity_to_complex (EAT (shift_e));
    else
      {
	shift.real = 0.0;
	shift.imag = 0.0;
      }

    if (options && (basis_e = SIT (options, "basis")))
      basis = test_entity (EAT (basis_e));
    else
      basis = 0;

    if (options && (vectors_e = SIT (options, "vectors")))
      vectors = test_entity (EAT (vectors_e));
    else
      vectors = 0;

    if (options && (resid_e = SIT (options, "resid")))
      {
	resid_e = dense_vector ((VECTOR *) vector_entity (EAT (resid_e)));
	resid_e = dup_vector ((VECTOR *) cast_vector (EAT (resid_e), complex));
	if (((VECTOR *)resid_e)->ne != n)
	  {
	    fail ("Wrong length for resid vector; must equal problem dimension.");
	    raise_exception ();
	  }
	info = 1;
      }
    else
      {
	resid_e = form_vector (n, complex, dense);
	info = 0;
      }

    /* Set up working arrays. */

    v = (MATRIX *) form_matrix (n, ncv, complex, dense);

    iparam[0] = 1;		/* exact shifts */
    iparam[2] = mxiter;		/* max number of Arnoldi iterations */
    iparam[3] = 1;		/* blocksize */
    iparam[6] = mode;		/* solution mode */

    workd = MALLOC (3*n*sizeof(COMPLEX));
    rwork = MALLOC (ncv*sizeof(REAL));

    lworkl = ncv * (3 * ncv + 5);
    workl = MALLOC (lworkl*sizeof(COMPLEX));

    y = (VECTOR *) gift_wrap_vector (n, complex, NULL);

    for (ido=0; ido != 99; )
      {
	XNAUPD (&ido, &bmat, &n, &which, &nev, &tol,
		((VECTOR *)resid_e)->a.complex,
		&ncv, ((MATRIX *)v)->a.complex, &n, iparam, ipntr, workd,
		workl, &lworkl, rwork, &info);

	switch (ido)
	  {
	  case 1:
	  case -1:	/* compute y <- OP * x */

	    y->a.complex = workd + ipntr[0] - 1;
	    exe_args[0].data.ptr = copy_vector (y);
	    exe_args[0].type = D_ENTITY;
	    exe_args[1].data.ptr = params ? copy_entity (params) : NULL;
	    exe_args[1].type = D_ENTITY;

	    r = execute_function ((FUNCTION *) copy_function (op_func),
				  2, exe_args, EVAL_STACK_SIZE - 2);
	    r = dense_vector ((VECTOR *) vector_entity (EAT (r)));
	    r = cast_vector (EAT (r), complex);

	    if (((VECTOR *) r)->ne != n)
	      {
		fail ("Function returns vector with incorrect length.");
		raise_exception ();
	      }

	    memcpy (workd + ipntr[1] - 1,
		    ((VECTOR *) r)->a.complex, n*sizeof (COMPLEX));

	    delete_entity (r); r = NULL;
	    break;

	  case 2:

	    y->a.complex = workd + ipntr[0] - 1;
	    exe_args[0].data.ptr = copy_vector (y);
	    exe_args[0].type = D_ENTITY;
	    exe_args[1].data.ptr = params ? copy_entity (params) : NULL;
	    exe_args[1].type = D_ENTITY;

	    r = execute_function ((FUNCTION *) copy_function (b_func),
				  2, exe_args, EVAL_STACK_SIZE - 2);
	    r = dense_vector ((VECTOR *) vector_entity (EAT (r)));
	    r = cast_vector (EAT (r), complex);

	    if (((VECTOR *) r)->ne != n)
	      {
		fail ("Function returns vector with incorrect length.");
		raise_exception ();
	      }

	    memcpy (workd + ipntr[1] - 1,
		    ((VECTOR *) r)->a.complex, n*sizeof (COMPLEX));

	    delete_entity (r); r = NULL;
	    break;

	  case 99:
	    /* normal exit */
	    break;

	  default:

	    fail ("Unknown return from ZNAUPD.");
	    raise_exception ();
	  }
      }

    /* Deal with error returns. */

    switch (info)
      {
      case 0:
	/* normal exit */
	break;

      case 1:
	fail ("Iteration limit exceeded.");
	raise_exception ();

      case 3:
	fail ("No shifts could be applied during a cycle of the implicitly restarted Arnoldi iteration.  One possible fix is to increase the size of NCV relative to NEV.");
	raise_exception ();

      case -1:
      case -2:
      case -3:
      case -4:
      case -5:
      case -6:
      case -10:
      case -12:
	fail ("Input error to ZNAUPD.  This is likely a bug.");
	raise_exception ();

      case -7:
	fail ("Length of WORKL is not sufficient.");
	raise_exception ();

      case -8:
	fail ("Error return from LAPACK eigenvalue calculation.");
	raise_exception ();

      case -9:
	fail ("Starting vector is zero.");
	raise_exception ();

      case -11:
	fail ("Incompatible options.  Mode 1 cannot be used for generalized eigenvalue problems.");
	raise_exception ();

      case -9999:
	fail ("Could not build an Arnoldi factorization.  May be a bug.");
	raise_exception ();

      default:
	fail ("Unknown error from ZNAUPD.");
	raise_exception ();
      }

    /* Now compute eigenvalues and vectors. */

    workev = MALLOC (2*ncv*sizeof(COMPLEX));

    vec = (MATRIX *) form_matrix (n, keep_nev, complex, dense);
    val = (VECTOR *) form_vector (keep_nev+1, complex, dense);
    select = MALLOC (ncv * sizeof(int));

    XNEUPD (&vectors, dummy, select, val->a.complex, vec->a.complex,
	    &n, &shift, workev, &bmat, &n, &which, &nev, &tol,
	    ((VECTOR *)resid_e)->a.complex, &ncv, v->a.complex, &n, iparam,
	    ipntr, workd, workl, &lworkl, rwork, &info);

    switch (info)
      {
      case 0:
	/* normal exit */
	break;

      case 1:
	fail ("Internal error in ZNEUPD.  This is likely an ARPACK bug.");
	raise_exception ();

      case -1:
      case -2:
      case -3:
      case -5:
      case -6:
      case -10:
      case -12:
      case -13:
      case -15:
	fail ("Input error to ZNEUPD.  This is likely a bug.");
	raise_exception ();

      case -7:
	fail ("Length of WORKL is not sufficient.");
	raise_exception ();

      case -8:
	fail ("Error return from LAPACK.  \"This should never happen.\"");
	raise_exception ();

      case -9:
	fail ("Error return from LAPACK routine ZTREVC.");
	raise_exception ();

      case -11:
	fail ("Incompatible options.  Mode 1 cannot be used for generalized eigenvalue problems.");
	raise_exception ();

      case -14:
	fail ("No eigenvalues found to sufficient accuracy.");
	raise_exception ();

      default:
	fail ("Unknown error from DSEUPD.");
	raise_exception ();
      }

    /* Free the work space. */

    y->a.complex = NULL;
    delete_vector (y);

    FREE (EAT (workd));
    FREE (EAT (rwork));
    FREE (EAT (workl));
    FREE (EAT (select));
    FREE (EAT (workev));

    /* We gave them some extra space. */

    val->ne--; val->nn--;

    /* Now put together the output table. */

    ret = (TABLE *) make_table ();
    RIT (ret, EAT (val), "values");
    if (vectors) RIT (ret, EAT (vec), "vectors");
    if (basis) RIT (ret, EAT (v), "basis");
    RIT (ret, EAT (resid_e), "resid");
    RIT (ret, int_to_scalar (iparam[2]), "iter");
    RIT (ret, int_to_scalar (iparam[4]), "nconv");
  }
  ON_EXCEPTION
  {
    TFREE (workd);
    TFREE (rwork);
    TFREE (workl);
    TFREE (select);
    delete_3_entities (mode_e, bmat_e, which_e);
    delete_3_entities (nev_e, ncv_e, mxiter_e);
    delete_3_entities (tol_e, shift_e, resid_e);
    delete_3_entities (basis_e, vectors_e, r);
    delete_3_entities (dim, op_func, b_func);
    delete_2_entities (params, options);
    delete_2_matrices (v, vec);
    delete_vector (val);
    delete_table (ret);
  }
  END_EXCEPTION;

  delete_3_entities (op_func, b_func, params);
  delete_2_matrices (v, vec);
  delete_entity (options);

  return ENT (ret);
}
