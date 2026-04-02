/*
   npsol.c -- Interface to NPSOL.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: npsol.c,v 1.3 2003/09/06 17:08:28 ksh Exp $";

#include "npsol.h"

#if HAVE_NPSOL

#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "function.h"
#include "cast.h"
#include "dense.h"
#include "datum.h"

static int PROTO (npsol_err, (int inform));
static int PROTO (confun,
		    (int *mode, int *ncnln, int *n, int *nrowj,
		 int *needc, REAL * x, REAL * c, REAL * cjac, int *nstate));
static int PROTO (objfun,
		    (int *mode, int *n, REAL * x, REAL * objf, REAL * objgrd,
		     int *nstate));

/*
 *  optimum = npsol( objective; start; constraints; options; state );
 *
 *      objective:
 *          Either a function that computes the objective value for the given
 *          point, or a table in which member `objf' is a function returning
 *          the objective value and member `objgrd' returns the gradients.
 *          If `objective' is a table, it may have an optional member `params'
 *          which is then passed as the second argument to `objf' and `objgrd'
 *          providing a means for passing additional parameters to those
 *          functions.
 *
 *      start:
 *          A vector that describes the starting point.
 *
 *      constraints:
 *          A table that may contain any or all of the following members:
 *
 *              side_constraints:
 *                  A matrix that specifies upper and lower constraints on
 *                  values of the variables.  This matrix has two columns,
 *                  the first column giving lower bounds and the second
 *                  column giving upper bounds.  If this matrix is not given,
 *                  then a default is provided using 1.0E10 as "infinity".
 *                  In that case, you should be aware that NPSOL will take
 *                  these values literally if you also use the "infinite_bound"
 *                  option with a value less than 1.0E10.
 *
 *              linear_constraints:
 *                  A table containing the following members:
 *
 *                      coefficients:
 *                          A matrix of the linear constraint coefficients.
 *                          The i-th row contains the coefficients for the
 *                          i-th linear constraint.
 *
 *                      bounds:
 *                          A matrix, like `side_constraints', that gives the
 *                          upper and lower bounds for the linear constraints.
 *                          If not given, defaults are provided as with
 *                          `side_constraints'.
 *
 *              nonlinear_constraints:
 *                  A table containing the following members:
 *
 *                      values:
 *                          Either a function that computes the vector of
 *                          nonlinear constraint function values at the given
 *                          point, or a table in which member `conf' is a
 *                          function returning the values and `congrd' is a
 *                          function returning the gradients.  Like `objectives',
 *                          a member called `params' may also be included.
 *
 *                      bounds:
 *                          A matrix, like `side_constraints', that gives the
 *                          upper and lower bounds for the nonlinear constraints.
 *                          If not given, defaults are provided as with
 *                          `side_constraints'.
 *
 *      options:
 *          A table containing any of the valid NPSOL options.  For example,
 *          { hessian = "yes" } specifies the corresponding option in NPSOL.
 *          Use the underscore character instead of blanks between words, as
 *          in { major_print_level = 1 }.
 *
 *      state:
 *          A table used only when the "warm start" option is requested.  It may
 *          be obtained from the return of a previous call to `npsol'.
 *
 *              ISTATE:
 *                  A vector describing the status of the constraints.
 *
 *              CLAMDA:
 *                  The Lagrange multiplier estimates for the nonlinear constraints.
 *
 *              R:
 *                  The factored Hessian of the Lagrangian function.
 *
 *      optimum:
 *          A table containing the information returned from NPSOL:
 *
 *              objective:
 *                  The value of the objective function at the final iterate.
 *
 *              solution:
 *                  The solution vector (or final estimate) for the problem.
 *
 *              inform:
 *                  The INFORM value (success/error indicator) returned by NPSOL.
 *
 *              iter:
 *                  The number of major iterations performed.
 *
 *              state:
 *                  The `state' table described above.
 *
 */

static char *objective_options[] =
{
  "objf",
  "objgrd",
  "params",
  NULL,
};
enum
  {
    OPT_OBJ_OBJF, OPT_OBJ_OBJGRD, OPT_OBJ_PARAMS
  };

static char *constraint_options[] =
{
  "side_constraints",
  "linear_constraints",
  "nonlinear_constraints",
  NULL,
};
enum
  {
    OPT_CON_SIDE, OPT_CON_LIN, OPT_CON_NONLIN
  };

static char *lin_con_options[] =
{
  "coefficients",
  "bounds",
  NULL,
};
enum
  {
    OPT_LIN_COEF, OPT_LIN_BOUNDS
  };

static char *nlin_con_options[] =
{
  "values",
  "bounds",
  NULL,
};
enum
  {
    OPT_NLIN_VALUES, OPT_NLIN_BOUNDS
  };

static char *values_options[] =
{
  "conf",
  "congrd",
  "params",
  NULL,
};
enum
  {
    OPT_VAL_CONF, OPT_VAL_CONGRD, OPT_VAL_PARAMS
  };

static char *state_options[] =
{
  "ISTATE",
  "CLAMDA",
  "R",
  NULL,
};
enum
  {
    OPT_STATE_ISTATE, OPT_STATE_CLAMDA, OPT_STATE_R
  };

/*
 * We call the FORTRAN routine `NPSOL', and it then calls
 * our `confun' and `objfun' functions.  Thus we have to
 * communicate through the following globals.
 */

static FUNCTION *objf_function;
static FUNCTION *objgrd_function;
static FUNCTION *conf_function;
static FUNCTION *congrd_function;

static ENTITY *npsol_obj_params;
static ENTITY *npsol_con_params;

static ENTITY *npsol_x_eid;

static int npsol_is_active;	/* Recursion is a no-no. */

#define EVAL_STACK_SIZE 32

ENTITY *
bi_npsol (int n, ENTITY *objective, ENTITY *start, ENTITY *constraints, ENTITY *options, ENTITY *state)
{
  ENTITY *objf = NULL;
  ENTITY *objgrd = NULL;
  ENTITY *obj_params = NULL;
  ENTITY *side = NULL;
  ENTITY *linear_constraints = NULL;
  ENTITY *lin_coef = NULL;
  ENTITY *lin_bounds = NULL;
  ENTITY *nonlinear_constraints = NULL;
  ENTITY *values = NULL;
  ENTITY *conf = NULL;
  ENTITY *congrd = NULL;
  ENTITY *con_params = NULL;
  ENTITY *nlin_bounds = NULL;
  ENTITY *istate = NULL;
  ENTITY *clamda = NULL;
  ENTITY *r = NULL;

  EASSERT (objective, 0, 0);
  EASSERT (start, 0, 0);
  EASSERT (constraints, 0, 0);
  EASSERT (options, 0, 0);

  WITH_HANDLING
  {

    /* Get the objective functions. */

    if (objective)
      {
	switch (objective->class)
	  {
	  case function:
	    objf = EAT (objective);
	    break;
	  case table:
	    if (!OK_OPTIONS ((TABLE *) objective, objective_options))
	      raise_exception;

	    objf = search_in_table ((TABLE *) copy_table ((TABLE *) objective),
				dup_char (objective_options[OPT_OBJ_OBJF]));
	    if (objf && objf->class != function)
	      {
		fail ("Invalid %s entity in place of \"%s\" function.",
		class_string[objf->class], objective_options[OPT_OBJ_OBJF]);
		raise_exception ();
	      }
	    objgrd = search_in_table ((TABLE *) copy_table ((TABLE *) objective),
			      dup_char (objective_options[OPT_OBJ_OBJGRD]));
	    if (objgrd && objgrd->class != function)
	      {
		fail ("Invalid %s entity in place of \"%s\" function.",
		      class_string[objgrd->class], objective_options[OPT_OBJ_OBJGRD]);
		raise_exception ();
	      }
	    obj_params = search_in_table ((TABLE *) EAT (objective),
			      dup_char (objective_options[OPT_OBJ_PARAMS]));
	    break;
	  default:
	    fail ("Invalid %s entity given for \"npsol\" objective.",
		  class_string[objective->class]);
	    raise_exception;
	  }
      }

    /* Make sure `start' is a vector. */

    if (start)
      start = vector_entity (EAT (start));

    /* Pick apart constraints table. */

    if (constraints)
      {
	if (constraints->class != table)
	  {
	    fail ("Invalid %s entity in place of the constraints table.",
		  class_string[constraints->class]);
	    raise_exception;
	  }
	if (!OK_OPTIONS ((TABLE *) constraints, constraint_options))
	  raise_exception;

	side = search_in_table ((TABLE *) copy_table ((TABLE *) constraints),
				dup_char (constraint_options[OPT_CON_SIDE]));
	if (side)
	  side = matrix_entity (EAT (side));

	linear_constraints = search_in_table ((TABLE *) copy_table ((TABLE *) constraints),
				dup_char (constraint_options[OPT_CON_LIN]));
	if (linear_constraints)
	  {
	    if (linear_constraints->class != table)
	      {
		fail ("Invalid %s entity in place of the linear_constraints table.",
		      class_string[linear_constraints->class]);
		raise_exception ();
	      }
	    if (!OK_OPTIONS ((TABLE *) linear_constraints, lin_con_options))
	      raise_exception ();

	    lin_coef = search_in_table ((TABLE *) copy_table ((TABLE *) linear_constraints),
				  dup_char (lin_con_options[OPT_LIN_COEF]));
	    if (lin_coef)
	      lin_coef = matrix_entity (EAT (lin_coef));

	    lin_bounds = search_in_table ((TABLE *) copy_table ((TABLE *) linear_constraints),
				dup_char (lin_con_options[OPT_LIN_BOUNDS]));
	    if (lin_bounds)
	      lin_bounds = matrix_entity (EAT (lin_bounds));
	  }

	nonlinear_constraints = search_in_table ((TABLE *) copy_table ((TABLE *) constraints),
			     dup_char (constraint_options[OPT_CON_NONLIN]));
	if (nonlinear_constraints)
	  {
	    if (nonlinear_constraints->class != table)
	      {
		fail ("Invalid %s entity in place of the nonlinear_constraints table.",
		      class_string[nonlinear_constraints->class]);
		raise_exception ();
	      }
	    if (!OK_OPTIONS ((TABLE *) nonlinear_constraints, nlin_con_options))
	      raise_exception ();

	    values = search_in_table ((TABLE *) copy_table ((TABLE *) nonlinear_constraints),
			      dup_char (nlin_con_options[OPT_NLIN_VALUES]));
	    if (values)
	      {
		switch (values->class)
		  {
		  case function:
		    conf = EAT (values);
		    break;
		  case table:
		    if (!OK_OPTIONS ((TABLE *) values, values_options))
		      raise_exception ();
		    conf = search_in_table ((TABLE *) copy_table ((TABLE *) values),
				   dup_char (values_options[OPT_VAL_CONF]));
		    if (conf && conf->class != function)
		      {
			fail ("Invalid %s entity in place of \"%s\" function.",
			      class_string[conf->class], values_options[OPT_VAL_CONF]);
			raise_exception ();
		      }
		    congrd = search_in_table ((TABLE *) copy_table ((TABLE *) values),
				 dup_char (values_options[OPT_VAL_CONGRD]));
		    if (congrd && congrd->class != function)
		      {
			fail ("Invalid %s entity in place of \"%s\" function.",
			      class_string[congrd->class], values_options[OPT_VAL_CONGRD]);
			raise_exception ();
		      }
		    con_params = search_in_table ((TABLE *) EAT (values),
				 dup_char (values_options[OPT_VAL_PARAMS]));
		    break;
		  default:
		    fail ("Invalid %s entity given for nonlinear constraint values.",
			  class_string[values->class]);
		    raise_exception;
		  }
	      }

	    nlin_bounds = search_in_table ((TABLE *) copy_table ((TABLE *) nonlinear_constraints),
			      dup_char (nlin_con_options[OPT_NLIN_BOUNDS]));
	    if (nlin_bounds)
	      nlin_bounds = matrix_entity (EAT (nlin_bounds));
	  }
      }

    if (options && options->class != table)
      {
	fail ("Invalid %s entity given in place of options table.",
	      class_string[options->class]);
	raise_exception ();
      }

    if (state)
      {
	if (state->class != table)
	  {
	    fail ("Invalid %s entity in place of the state table.",
		  class_string[state->class]);
	    raise_exception ();
	  }
	if (!OK_OPTIONS ((TABLE *) state, state_options))
	  raise_exception ();

	istate = search_in_table ((TABLE *) copy_table ((TABLE *) state),
				dup_char (state_options[OPT_STATE_ISTATE]));
	if (istate)
	  istate = vector_entity (EAT (istate));

	clamda = search_in_table ((TABLE *) copy_table ((TABLE *) state),
				dup_char (state_options[OPT_STATE_CLAMDA]));
	if (clamda)
	  clamda = vector_entity (EAT (clamda));

	r = search_in_table ((TABLE *) copy_table ((TABLE *) state),
			     dup_char (state_options[OPT_STATE_R]));
	if (r)
	  r = matrix_entity (EAT (r));
      }

  }
  ON_EXCEPTION
  {
    delete_3_entities (objective, start, constraints);
    delete_2_entities (options, state);

    delete_3_entities (objf, objgrd, side);
    delete_2_entities (obj_params, con_params);
    delete_3_entities (linear_constraints, lin_coef, lin_bounds);
    delete_3_entities (nonlinear_constraints, values, conf);
    delete_2_entities (congrd, nlin_bounds);
    delete_2_entities (istate, clamda);
    delete_entity (r);
  }
  END_EXCEPTION;

  delete_3_entities (objective, constraints, state);
  delete_3_entities (linear_constraints, nonlinear_constraints, values);

  return (npsol_entity ((FUNCTION *) objf, (FUNCTION *) objgrd, (VECTOR *) start,
		(MATRIX *) side, (MATRIX *) lin_coef, (MATRIX *) lin_bounds,
	     (FUNCTION *) conf, (FUNCTION *) congrd, (MATRIX *) nlin_bounds,
		    (TABLE *) options, (VECTOR *) istate, (VECTOR *) clamda,
			(MATRIX *) r, obj_params, con_params));
}

ENTITY *
npsol_entity (FUNCTION *objf, FUNCTION *objgrd, VECTOR *start, MATRIX *side, MATRIX *lin_coef, MATRIX *lin_bounds, FUNCTION *conf, FUNCTION *congrd, MATRIX *nlin_bounds, TABLE *options, VECTOR *istate, VECTOR *clamda, MATRIX *r, ENTITY *obj_params, ENTITY *con_params)
{
  TABLE *optimum = NULL;
  VECTOR *bl = NULL;
  VECTOR *bu = NULL;
  SCALAR *inform = NULL;
  SCALAR *iter = NULL;
  VECTOR *c = NULL;
  SCALAR *objective = NULL;
  MATRIX *cjac = NULL;
  VECTOR *ogrd = NULL;
  VECTOR *x = NULL;
  TABLE *state = NULL;

  int i;
  int ndv, nrowa, nrowuj, nrowr;
  int nclin = 0;
  int ncnln = 0;
  REAL bigbnd = 1.0e10;
  int leniw, lenw;

  int *iw = NULL;
  REAL *w = NULL;

  WITH_HANDLING
  {

    /* Make sure we're not in a recursive call. */

    if (npsol_is_active)
      {
	fail ("Can't make recursive call to NPSOL.");
	raise_exception ();
      }
    npsol_is_active = 1;

    objf_function = objgrd_function = NULL;
    conf_function = congrd_function = NULL;
    npsol_obj_params = npsol_con_params = NULL;
    npsol_x_eid = NULL;


    /* Set the NPSOL options. */

    set_npsol_options ((TABLE *) EAT (options));

    /* Check the rest of the input. */

    if (!objf)
      {
	fail ("No objective function supplied.");
	raise_exception ();
      }

    start = (VECTOR *) dense_vector (start);
    if (start->type == integer)
      start = (VECTOR *) cast_vector_integer_real (EAT (start));
    if (start->type != real)
      {
	fail ("Invalid %s type for the \"start\" vector.",
	      type_string[start->type]);
	raise_exception;
      }
    ndv = start->ne;
    x = (VECTOR *) dup_vector ((VECTOR *) EAT (start));

    if (side)
      {
	if (side->type == integer)
	  {
	    side = (MATRIX *) cast_matrix_integer_real ((MATRIX *) EAT (side));
	  }
	if (side->type != real)
	  {
	    fail ("Invalid %s type for \"%s\".",
		  type_string[side->type], constraint_options[OPT_CON_SIDE]);
	    raise_exception;
	  }
	side = (MATRIX *) dense_matrix ((MATRIX *) EAT (side));
	if (side->nr != ndv)
	  {
	    fail ("The \"start\" vector has %d element%s, but the \"%s\" matrix has %d row%s.",
		  ndv, PLURAL (ndv), constraint_options[OPT_CON_SIDE],
		  side->nr, PLURAL (side->nr));
	    raise_exception;
	  }
	if (side->nc != 2)
	  {
	    fail ("The \"%s\" matrix has %d column%s, not the required 2.",
	     constraint_options[OPT_CON_SIDE], side->nc, PLURAL (side->nc));
	    raise_exception ();
	  }
      }

    if (lin_coef)
      {
	if (lin_coef->nc != ndv)
	  {
	    fail ("The problem has %d variable%s, but the linear constraint coefficient matrix has %d column%s.",
		  ndv, PLURAL (ndv), lin_coef->nc, PLURAL (lin_coef->nc));
	    raise_exception ();
	  }
	if (lin_coef->type == integer)
	  {
	    lin_coef = (MATRIX *) cast_matrix_integer_real ((MATRIX *) EAT (lin_coef));
	  }
	if (lin_coef->type != real)
	  {
	    fail ("Invalid %s type for the linear constraint coefficients.",
		  type_string[lin_coef->type]);
	    raise_exception;
	  }
	lin_coef = (MATRIX *) dense_matrix ((MATRIX *) EAT (lin_coef));
	nclin = lin_coef->nr;
      }

    if (lin_bounds)
      {
	if (!lin_coef)
	  {
	    fail ("Bounds for linear constraints were given, but not the coefficients.");
	    raise_exception ();
	  }
	if (lin_bounds->nr != nclin)
	  {
	    fail ("The linear constraint bounds matrix has %d row%s, but the coefficient matrix has %d row%s.",
	    lin_bounds->nr, PLURAL (lin_bounds->nr), nclin, PLURAL (nclin));
	    raise_exception ();
	  }
	if (lin_bounds->nc != 2)
	  {
	    fail ("The linear constraint bounds matrix has %d column%s, not the required 2.",
		  lin_bounds->nc, PLURAL (lin_bounds->nc));
	    raise_exception ();
	  }
	if (lin_bounds->type == integer)
	  {
	    lin_bounds = (MATRIX *) cast_matrix_integer_real ((MATRIX *) EAT (lin_bounds));
	  }
	if (lin_bounds->type != real)
	  {
	    fail ("Invalid %s type for the linear constraint bounds.",
		  type_string[lin_bounds->type]);
	    raise_exception;
	  }
	lin_bounds = (MATRIX *) dense_matrix ((MATRIX *) EAT (lin_bounds));
      }

    if (nlin_bounds)
      {
	if (!conf)
	  {
	    fail ("Bounds for nonlinear constraints were given, but not the constraints function.");
	    raise_exception ();
	  }
	if (nlin_bounds->nc != 2)
	  {
	    fail ("The nonlinear constraint bounds matrix has %d column%s, not the required 2.",
		  nlin_bounds->nc, PLURAL (nlin_bounds->nc));
	    raise_exception ();
	  }
	if (nlin_bounds->type == integer)
	  {
	    nlin_bounds = (MATRIX *) cast_matrix_integer_real ((MATRIX *) EAT (nlin_bounds));
	  }
	if (nlin_bounds->type != real)
	  {
	    fail ("Invalid %s type for the nonlinear constraint bounds.",
		  type_string[nlin_bounds->type]);
	    raise_exception;
	  }
	nlin_bounds = (MATRIX *) dense_matrix ((MATRIX *) EAT (nlin_bounds));
	ncnln = nlin_bounds->nr;
      }

    if (!lin_coef)
      lin_coef = (MATRIX *) make_matrix (1, 1, real, dense);

    if (istate)
      {
	if (istate->type != integer)
	  {
	    fail ("Invalid %s type for \"%s\".",
		type_string[istate->type], state_options[OPT_STATE_ISTATE]);
	    raise_exception;
	  }
	istate = (VECTOR *) dense_vector ((VECTOR *) EAT (istate));
	if (istate->ne != ndv + nclin + ncnln)
	  {
	    fail ("The \"%s\" vector has %d element%s; it should have %d element%s.",
		  state_options[OPT_STATE_ISTATE],
		  istate->ne, PLURAL (istate->ne), ndv + nclin + ncnln, PLURAL (ndv + nclin + ncnln));
	    raise_exception;
	  }
      }
    else
      {
	istate = (VECTOR *) make_vector (ndv + nclin + ncnln, integer, dense);
      }

    if (clamda)
      {
	if (clamda->type != real)
	  {
	    fail ("Invalid %s type for \"%s\".",
		type_string[clamda->type], state_options[OPT_STATE_CLAMDA]);
	    raise_exception;
	  }
	clamda = (VECTOR *) dense_vector ((VECTOR *) EAT (clamda));
	if (clamda->ne != ndv + nclin + ncnln)
	  {
	    fail ("The \"%s\" vector has %d element%s; it should have %d element%s.",
		  state_options[OPT_STATE_CLAMDA],
		  clamda->ne, PLURAL (clamda->ne), ndv + nclin + ncnln, PLURAL (ndv + nclin + ncnln));
	    raise_exception;
	  }
      }
    else
      {
	clamda = (VECTOR *) make_vector (ndv + nclin + ncnln, real, dense);
      }

    if (r)
      {
	if (r->type != real)
	  {
	    fail ("Invalid %s type for \"%s\".",
		  type_string[r->type], state_options[OPT_STATE_R]);
	    raise_exception;
	  }
	r = (MATRIX *) dense_matrix ((MATRIX *) EAT (r));
	if (r->nr != ndv)
	  {
	    fail ("The \"%s\" matrix has %d row%s; it should have %d row%s.",
		  state_options[OPT_STATE_R],
		  r->nr, PLURAL (r->nr), ndv, PLURAL (ndv));
	    raise_exception;
	  }
	if (r->nc != ndv)
	  {
	    fail ("The \"%s\" matrix has %d row%s; it should have %d row%s.",
		  state_options[OPT_STATE_R],
		  r->nc, PLURAL (r->nc), ndv, PLURAL (ndv));
	    raise_exception;
	  }
      }
    else
      {
	r = (MATRIX *) make_matrix (ndv, ndv, real, dense);
      }

    /* Set up BL and BU. */

    bl = (VECTOR *) make_vector (ndv + nclin + ncnln, real, dense);
    bu = (VECTOR *) make_vector (ndv + nclin + ncnln, real, dense);

    if (side)
      {
	memcpy (bl->a.real, side->a.real, ndv * sizeof (REAL));
	memcpy (bu->a.real, side->a.real + ndv, ndv * sizeof (REAL));
      }
    else
      {
	for (i = 0; i < ndv; i++)
	  bl->a.real[i] = -bigbnd;
	for (i = 0; i < ndv; i++)
	  bu->a.real[i] = bigbnd;
      }

    if (nclin)
      {
	if (lin_bounds)
	  {
	    memcpy (bl->a.real + ndv, lin_bounds->a.real, nclin * sizeof (REAL));
	    memcpy (bu->a.real + ndv, lin_bounds->a.real + nclin, nclin * sizeof (REAL));
	  }
	else
	  {
	    for (i = ndv; i < ndv + nclin; i++)
	      bl->a.real[i] = -bigbnd;
	    for (i = ndv; i < ndv + nclin; i++)
	      bu->a.real[i] = bigbnd;
	  }
      }

    if (ncnln)
      {
	if (nlin_bounds)
	  {
	    memcpy (bl->a.real + ndv + nclin, nlin_bounds->a.real, ncnln * sizeof (REAL));
	    memcpy (bu->a.real + ndv + nclin, nlin_bounds->a.real + ncnln, ncnln * sizeof (REAL));
	  }
	else
	  {
	    for (i = ndv + nclin; i < ndv + nclin + ncnln; i++)
	      bl->a.real[i] = -bigbnd;
	    for (i = ndv + nclin; i < ndv + nclin + ncnln; i++)
	      bu->a.real[i] = bigbnd;
	  }
      }

    /*
     * Put functions and parameters in globals so `objfun' and `confun'
     * can use them.
     */

    objf_function = (FUNCTION *) EAT (objf);
    objgrd_function = (FUNCTION *) EAT (objgrd);
    conf_function = (FUNCTION *) EAT (conf);
    congrd_function = (FUNCTION *) EAT (congrd);

    npsol_obj_params = EAT (obj_params);
    npsol_con_params = EAT (con_params);

    if (x->eid)
      npsol_x_eid = copy_entity (x->eid);

    /* Set up some return stuff. */

    inform = (SCALAR *) int_to_scalar (0);
    iter = (SCALAR *) int_to_scalar (0);
    c = (VECTOR *) make_vector (ncnln, real, dense);
    objective = (SCALAR *) real_to_scalar (0.0);
    cjac = (MATRIX *) make_matrix (ncnln, ndv, real, dense);
    ogrd = (VECTOR *) make_vector (ndv, real, dense);

    /* Malloc workspace. */

    leniw = 3 * ndv + nclin + 2 * ncnln;
    iw = E_MALLOC (leniw, integer);
    lenw = 20 * ndv;
    if (nclin || ncnln)
      lenw += 2 * ndv * ndv;
    if (nclin)
      lenw += 11 * nclin;
    if (ncnln)
      lenw += ndv * nclin + 2 * ndv * ncnln + 21 * ncnln;
    w = E_MALLOC (lenw, real);

    /* Make sure dimensions are greater than zero. */

    nrowa = (nclin > 0) ? nclin : 1;
    if (lin_coef->nn == 0)
      {
	delete_matrix (lin_coef);
	lin_coef = NULL;
	lin_coef = (MATRIX *) make_matrix (1, 1, real, dense);
      }
    nrowuj = (ncnln > 0) ? ncnln : 1;
    if (cjac->nn == 0)
      {
	delete_matrix (cjac);
	cjac = NULL;
	cjac = (MATRIX *) make_matrix (1, 1, real, dense);
      }
    nrowr = (ndv > 0) ? ndv : 1;
    if (r->nn == 0)
      {
	delete_matrix (r);
	r = NULL;
	r = (MATRIX *) make_matrix (1, 1, real, dense);
      }

    /* Call NPSOL */

    NPSOL (&ndv, &nclin, &ncnln, &nrowa, &nrowuj, &nrowr, lin_coef->a.real,
	   bl->a.real, bu->a.real, confun, objfun, &inform->v.integer,
	   &iter->v.integer, istate->a.integer, c->a.real, cjac->a.real,
	   clamda->a.real, &objective->v.real, ogrd->a.real, r->a.real,
	   x->a.real, iw, &leniw, w, &lenw);
    setbuf (stderr, NULL);	/* FORTRAN changes stderr buffering. */

    if (npsol_err (inform->v.integer) > 1)
      raise_exception ();

    /* Free the workspace. */

    FREE (EAT (iw));
    FREE (EAT (w));

    /* Now put together the output table. */

    state = (TABLE *) replace_in_table ((TABLE *) make_table (),
					EAT (istate), dup_char ("ISTATE"));
    state = (TABLE *) replace_in_table ((TABLE *) EAT (state),
					EAT (clamda), dup_char ("CLAMDA"));
    state = (TABLE *) replace_in_table ((TABLE *) EAT (state),
					EAT (r), dup_char ("R"));

    optimum = (TABLE *) replace_in_table ((TABLE *) make_table (),
				   EAT (objective), dup_char ("objective"));
    optimum = (TABLE *) replace_in_table ((TABLE *) EAT (optimum),
					  EAT (x), dup_char ("solution"));
    optimum = (TABLE *) replace_in_table ((TABLE *) EAT (optimum),
					  EAT (inform), dup_char ("inform"));
    optimum = (TABLE *) replace_in_table ((TABLE *) EAT (optimum),
					  EAT (iter), dup_char ("iter"));
    optimum = (TABLE *) replace_in_table ((TABLE *) EAT (optimum),
					  EAT (state), dup_char ("state"));

  }
  ON_EXCEPTION
  {
    TFREE (iw);
    TFREE (w);
    delete_2_functions (objf, objgrd);
    delete_vector (start);
    delete_3_matrices (side, lin_coef, lin_bounds);
    delete_2_functions (conf, congrd);
    delete_matrix (nlin_bounds);
    delete_table (options);
    delete_matrix (cjac);
    delete_3_vectors (clamda, istate, ogrd);
    delete_matrix (r);
    delete_2_vectors (bl, bu);
    delete_2_entities (obj_params, con_params);
    delete_2_functions (objf_function, objgrd_function);
    delete_2_functions (conf_function, congrd_function);
    delete_2_entities (npsol_obj_params, npsol_con_params);
    delete_entity (npsol_x_eid);
    delete_2_scalars (inform, iter);
    delete_2_vectors (c, x);
    delete_scalar (objective);
    delete_2_tables (optimum, state);

    npsol_is_active = 0;
  }
  END_EXCEPTION;

  delete_2_functions (objf, objgrd);
  delete_vector (start);
  delete_3_matrices (side, lin_coef, lin_bounds);
  delete_2_functions (conf, congrd);
  delete_matrix (nlin_bounds);
  delete_table (options);
  delete_matrix (cjac);
  delete_3_vectors (clamda, istate, ogrd);
  delete_matrix (r);
  delete_2_vectors (bl, bu);
  delete_2_entities (obj_params, con_params);
  delete_2_functions (objf_function, objgrd_function);
  delete_2_functions (conf_function, congrd_function);
  delete_2_entities (npsol_obj_params, npsol_con_params);
  delete_entity (npsol_x_eid);
  delete_2_scalars (inform, iter);
  delete_vector (c);
  delete_scalar (objective);

  npsol_is_active = 0;

  return (ENT (optimum));
}

void
set_npsol_options (TABLE *options)
{
  static char list[] = "list";
  static char nolist[] = "nolist";
  static char defaults[] = "defaults";
  static char major_print_level[] = "major print level = 0";
#define NPSOL_OPTION_LENGTH	60
  char npsol_option_buffer[NPSOL_OPTION_LENGTH];

  ENTITY *e;
  char *s, *t;

  EASSERT (options, table, 0);

  /*
   * This routine sets the NPSOL options based on the contents of the
   * input table.  The "nolist" option is only effective if it is given
   * first, so we look for it explicitly.  We reset the defaults, whether
   * the "defaults" option is given or not, so the options are not
   * cumulative like they normally are in NPSOL.
   */

  options = (TABLE *) lowercase_table (options);

  if (IN_TABLE (options, list))
    {
      if (IN_TABLE (options, nolist))
	{
	  fail ("Can't specify both \"list\" and \"nolist\" options.");
	  raise_exception ();
	}
      options = (TABLE *) delete_from_table (options, dup_char (list));
    }
  else
    {
      NPOPTN (nolist, (long int) strlen (nolist));
      options = (TABLE *) delete_from_table (options, dup_char (nolist));
    }

  NPOPTN (defaults, (long int) strlen (defaults));
  options = (TABLE *) delete_from_table (options, dup_char (defaults));

  NPOPTN (major_print_level, (long int) strlen (major_print_level));

  while (s = first_in_table ((TABLE *) copy_table (options)))
    {
      e = search_in_table ((TABLE *) copy_table (options), dup_char (s));
      options = (TABLE *) delete_from_table (options, dup_char (s));

      if (strlen (s) >= NPSOL_OPTION_LENGTH)
	goto too_long;
      strcpy (npsol_option_buffer, s);

      t = npsol_option_buffer;
      while (*(t++))
	if (*t == '_')
	  *t = ' ';

      if (e)
	{

	  if (e->class != scalar)
	    {
	      fail ("Invalid %s entity for NPSOL option \"%s\".",
		    class_string[e->class], s);
	      delete_entity (e);
	      delete_table (options);
	      FREE_CHAR (s);
	      raise_exception ();
	    }

	  if (strlen (s) + 3 + 20 >= NPSOL_OPTION_LENGTH)
	    goto too_long;
	  strcpy (npsol_option_buffer + strlen (s), " = ");

	  switch (((SCALAR *) e)->type)
	    {
	    case integer:
#if HAVE_SNPRINTF
	      snprintf (npsol_option_buffer + strlen (s) + 3,
			NPSOL_OPTION_LENGTH, "%d", ((SCALAR *) e)->v.integer);
#else
	      sprintf (npsol_option_buffer + strlen (s) + 3,
		       "%d", ((SCALAR *) e)->v.integer);
#endif
	      break;
	    case real:
#if HAVE_SNPRINTF
	      snprintf (npsol_option_buffer + strlen (s) + 3,
			NPSOL_OPTION_LENGTH, "%g", ((SCALAR *) e)->v.real);
#else
	      sprintf (npsol_option_buffer + strlen (s) + 3,
		       "%g", ((SCALAR *) e)->v.real);
#endif
	      break;
	    case character:
#if HAVE_SNPRINTF
	      if (snprintf (npsol_option_buffer + strlen (s) + 3,
			    NPSOL_OPTION_LENGTH, "%s",
			    ((SCALAR *) e)->v.character)
		  >= NPSOL_OPTION_LENGTH)
		goto too_long;
#else
	      if (strlen (s) + 3 + strlen (((SCALAR *) e)->v.character)
		  >= NPSOL_OPTION_LENGTH)
		goto too_long;
	      sprintf (npsol_option_buffer + strlen (s) + 3,
		       "%s", ((SCALAR *) e)->v.character);
#endif
	      break;
	    default:
	      fail ("Invalid %s type for an NPSOL option.",
		    type_string[((SCALAR *) e)->type]);
	      delete_entity (e);
	      delete_table (options);
	      FREE_CHAR (s);
	      raise_exception ();
	    }

	  delete_entity (e);
	}

      NPOPTN (npsol_option_buffer, (long int) strlen (npsol_option_buffer));
      if (npsol_err (0) > 1)
	{
	  fail ("Problem with NPSOL option \"%s\".", s);
	  delete_table (options);
	  FREE_CHAR (s);
	  raise_exception ();
	}
    }

  delete_table (options);
  return;

too_long:
  fail ("NPSOL option \"%s\" is too long.", s);
  delete_entity (e);
  delete_table (options);
  FREE_CHAR (s);
  raise_exception ();
}

static int
confun (int *mode, int *ncnln, int *n, int *nrowj, int *needc, REAL *x, REAL *c, REAL *cjac, int *nstate)
{
  DATUM exe_args[EVAL_STACK_SIZE];
  ENTITY *c_ent = NULL;
  ENTITY *cjac_ent = NULL;
  VECTOR *x_vector = NULL;

  assert (*ncnln == *nrowj);

  WITH_HANDLING
  {

    x_vector = (VECTOR *) form_vector (*n, real, dense);
    memcpy (x_vector->a.real, x, *n * sizeof (REAL));
    if (npsol_x_eid)
      x_vector->eid = copy_entity (npsol_x_eid);

    /* Need constraint values? */

    if ((*mode == 0 || *mode == 2) && conf_function)
      {

	exe_args[0].data.ptr = copy_vector (x_vector);
	exe_args[0].type = D_ENTITY;
	if (npsol_con_params)
	  {
	    exe_args[1].data.ptr = copy_entity (npsol_con_params);
	    exe_args[1].type = D_ENTITY;
	    c_ent = execute_function ((FUNCTION *) copy_function (conf_function),
				      2, exe_args, EVAL_STACK_SIZE - 2);
	  }
	else
	  {
	    exe_args[1].data.ptr = NULL;
	    exe_args[1].type = D_NULL;
	    c_ent = execute_function ((FUNCTION *) copy_function (conf_function),
				      1, exe_args, EVAL_STACK_SIZE - 2);
	  }

	c_ent = dense_vector ((VECTOR *) vector_entity (EAT (c_ent)));

	if (((VECTOR *) c_ent)->type == integer)
	  c_ent = cast_vector_integer_real ((VECTOR *) EAT (c_ent));

	if (((VECTOR *) c_ent)->type != real)
	  {
	    fail ("The \"conf\" function returns %s; should be real.",
		  type_string[((VECTOR *) c_ent)->type]);
	    raise_exception ();
	  }
	if (((VECTOR *) c_ent)->ne != *ncnln)
	  {
	    fail ("There %s %d constraint%s, but the \"conf\" function returns %d value%s.",
		  (*ncnln == 1) ? "is" : "are", *ncnln, PLURAL (*ncnln),
		  ((VECTOR *) c_ent)->ne, PLURAL (((VECTOR *) c_ent)->ne));
	    raise_exception ();
	  }

	memcpy (c, ((VECTOR *) c_ent)->a.real, *ncnln * sizeof (REAL));

	delete_entity (c_ent);
	c_ent = NULL;
      }

    if ((*mode == 1 || *mode == 2) && congrd_function)
      {

	exe_args[0].data.ptr = copy_vector (x_vector);
	exe_args[0].type = D_ENTITY;
	if (npsol_con_params)
	  {
	    exe_args[1].data.ptr = copy_entity (npsol_con_params);
	    exe_args[1].type = D_ENTITY;
	    cjac_ent = execute_function ((FUNCTION *) copy_function (congrd_function),
					 2, exe_args, EVAL_STACK_SIZE - 2);
	  }
	else
	  {
	    exe_args[1].data.ptr = NULL;
	    exe_args[1].type = D_NULL;
	    cjac_ent = execute_function ((FUNCTION *) copy_function (congrd_function),
					 1, exe_args, EVAL_STACK_SIZE - 2);
	  }

	cjac_ent = dense_matrix ((MATRIX *) matrix_entity (EAT (cjac_ent)));
	if (((MATRIX *) cjac_ent)->type == integer)
	  cjac_ent = cast_matrix_integer_real ((MATRIX *) EAT (cjac_ent));
	if (((MATRIX *) cjac_ent)->type != real)
	  {
	    fail ("The \"congrd\" function returns %s; should be real.",
		  type_string[((MATRIX *) cjac_ent)->type]);
	    raise_exception ();
	  }
	if (((MATRIX *) cjac_ent)->nr != *ncnln)
	  {
	    fail ("There %s %d constraint%s, but the \"congrd\" function returns %d row%s.",
		  (*ncnln == 1) ? "is" : "are", *ncnln, PLURAL (*ncnln),
	     ((MATRIX *) cjac_ent)->nr, PLURAL (((MATRIX *) cjac_ent)->nr));
	    raise_exception ();
	  }
	if (((MATRIX *) cjac_ent)->nc != *n)
	  {
	    fail ("There %s %d variable%s, but the \"congrd\" function returns %d column%s.",
		  (*n == 1) ? "is" : "are", *n, PLURAL (*n),
	     ((MATRIX *) cjac_ent)->nc, PLURAL (((MATRIX *) cjac_ent)->nc));
	    raise_exception ();
	  }

	memcpy (cjac, ((MATRIX *) cjac_ent)->a.real, *ncnln * *n * sizeof (REAL));

	delete_entity (cjac_ent);
	cjac_ent = NULL;
      }
  }
  ON_EXCEPTION
  {
    delete_2_entities (c_ent, cjac_ent);
    delete_vector (x_vector);
  }
  END_EXCEPTION;

  delete_vector (x_vector);

  return (0);
}

static int
objfun (int *mode, int *n, REAL *x, REAL *objf, REAL *objgrd, int *nstate)
{
  DATUM exe_args[EVAL_STACK_SIZE];
  ENTITY *objf_ent = NULL;
  ENTITY *objgrd_ent = NULL;
  VECTOR *x_vector = NULL;

  WITH_HANDLING
  {

    x_vector = (VECTOR *) form_vector (*n, real, dense);
    memcpy (x_vector->a.real, x, *n * sizeof (REAL));
    if (npsol_x_eid)
      x_vector->eid = copy_entity (npsol_x_eid);

    /* Need constraint values? */

    if ((*mode == 0 || *mode == 2) && objf_function)
      {

	exe_args[0].data.ptr = copy_vector (x_vector);
	exe_args[0].type = D_ENTITY;
	if (npsol_obj_params)
	  {
	    exe_args[1].data.ptr = copy_entity (npsol_obj_params);
	    exe_args[1].type = D_ENTITY;
	    objf_ent = execute_function ((FUNCTION *) copy_entity (ENT (objf_function)),
					 2, exe_args, 0);
	  }
	else
	  {
	    exe_args[1].data.ptr = NULL;
	    exe_args[1].type = D_NULL;
	    objf_ent = execute_function ((FUNCTION *) copy_entity (ENT (objf_function)),
					 1, exe_args, 0);
	  }

	objf_ent = bi_scalar (1, EAT (objf_ent));
	if (((SCALAR *) objf_ent)->type == integer)
	  objf_ent = cast_scalar_integer_real ((SCALAR *) EAT (objf_ent));
	if (((SCALAR *) objf_ent)->type != real)
	  {
	    fail ("The \"objf\" function returns %s; should be real.",
		  type_string[((SCALAR *) objf_ent)->type]);
	    raise_exception ();
	  }

	*objf = ((SCALAR *) objf_ent)->v.real;

	delete_entity (objf_ent);
	objf_ent = NULL;
      }

    if ((*mode == 1 || *mode == 2) && objgrd_function)
      {

	exe_args[0].data.ptr = copy_vector (x_vector);
	exe_args[0].type = D_ENTITY;
	if (npsol_obj_params)
	  {
	    exe_args[1].data.ptr = copy_entity (npsol_obj_params);
	    exe_args[1].type = D_ENTITY;
	    objgrd_ent = execute_function ((FUNCTION *) copy_entity (ENT (objgrd_function)),
					   2, exe_args, 0);
	  }
	else
	  {
	    exe_args[1].data.ptr = NULL;
	    exe_args[1].type = D_NULL;
	    objgrd_ent = execute_function ((FUNCTION *) copy_entity (ENT (objgrd_function)),
					   1, exe_args, 0);
	  }

	objgrd_ent = dense_vector ((VECTOR *) vector_entity (EAT (objgrd_ent)));
	if (((VECTOR *) objgrd_ent)->type == integer)
	  objgrd_ent = cast_vector_integer_real ((VECTOR *) EAT (objgrd_ent));
	if (((VECTOR *) objgrd_ent)->type != real)
	  {
	    fail ("The \"objgrd\" function returns %s; should be real.",
		  type_string[((VECTOR *) objgrd_ent)->type]);
	    raise_exception ();
	  }
	if (((VECTOR *) objgrd_ent)->ne != *n)
	  {
	    fail ("There %s %d variable%s, but the \"objgrd\" function returns %d element%s.",
		  (*n == 1) ? "is" : "are", *n, PLURAL (*n),
		  ((VECTOR *) objgrd_ent)->ne, PLURAL (((VECTOR *) objgrd_ent)->ne));
	    raise_exception ();
	  }

	memcpy (objgrd, ((VECTOR *) objgrd_ent)->a.real, *n * sizeof (REAL));

	delete_entity (objgrd_ent);
	objgrd_ent = NULL;
      }
  }
  ON_EXCEPTION
  {
    delete_2_entities (objf_ent, objgrd_ent);
    delete_vector (x_vector);
  }
  END_EXCEPTION;

  delete_vector (x_vector);
  return (0);
}

static int
npsol_err (int inform)
{
  switch (NPOPTE ())
    {
    case 0:
      break;
    case 1:
      warn ("Warnings were issued by NPSOL.");
      return (1);
    case 2:
      fail ("Error reported by NPSOL.");
      return (2);
    case 3:
      wipeout ("Somebody screwed up in NPSOL.");
    }

  if (inform == 9)
    {
      fail ("Invalid input to NPSOL.");
      return (2);
    }

  return (0);
}

#endif /* HAVE_NPSOL */
