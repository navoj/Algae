/*
   execute.c -- Execute the byte-compiled code.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: execute.c,v 1.6 2003/08/07 02:34:09 ksh Exp $";

#include <stdio.h>
#include <setjmp.h>
#include <ctype.h>
#include "algae.h"
#include "exception.h"
#include "sigint.h"
#include "psr.h"
#include "binop.h"
#include "unop.h"

#include "ptypes.h"
#include "code.h"
#include "partition.h"
#include "generate.h"
#include "table.h"
#include "assign.h"
#include "member.h"
#include "function.h"
#include "builtin.h"
#include "for_loop.h"
#include "vector.h"
#include "divide.h"
#include "scalar.h"
#include "real.h"
#include "profile.h"
#include "debug_algae.h"

#if ENABLE_PROFILER
extern int profiling;		/* on when profiling */
#endif

#define  EVAL_STACK_SIZE	64
#define  EXCEPTION_STACK_SIZE	8
#define  CDP_STACK_SIZE		8
#define  VEIL_STACK_SIZE	16

#if DEBUG
#define  inc_sp()  if( ++sp == eval_stack + EVAL_STACK_SIZE )\
		   { fail( "eval stack overflow") ; algae_exit(1) ; }\
		   else
#define  inc_ep()  if( ++exp_ptr == EXCEPTION_STACK_SIZE )\
		   { fail( "exception stack overflow") ; algae_exit(1) ; }\
		   else
#define  inc_cp()  if( ++cdp_ptr == cdp_stack + CDP_STACK_SIZE )\
		   { fail( "code pointer stack overflow") ; algae_exit(1) ; }\
		   else
#define  inc_vp()  if( vp += 2, vp >= veil_stack + VEIL_STACK_SIZE )\
		   { fail( "veil stack overflow") ; algae_exit(1) ; }\
		   else
#else
#define  inc_sp()	(sp++)
#define  inc_ep()	(exp_ptr++)
#define  inc_cp()	(cdp_ptr++)
#define  inc_vp()	(vp+=2, vp)
#endif

#define  RT_ABORT(s)  { fail( s ) ; raise_exception() ; }


/* macro -- used to call binop_entity() to operate on two adjacent
   DATUM on the eval_stack
   sets the datum types to D_NULL before the call
   and adjusts the datum type of the left operand after the call */
#define  EBINOP(op,sp) do { sp->type = (sp+1)->type = D_NULL ;\
			    sp->data.ptr = (void*)\
			      binop_entity(op,E_PTR(sp),E_PTR(sp+1)) ;\
			    sp->type = D_ENTITY ;\
			  } while(0)

#define EUNOP( op, sp )	do { sp->type = D_NULL;\
			     sp->data.ptr = (void *)\
			       unop_entity( op, E_PTR( sp ) );\
			     sp->type = D_ENTITY;\
			   } while(0)


ENTITY *
execute (INST *cdp, DATUM *fp, FUNCTION *current_function)
{
  DATUM eval_stack[EVAL_STACK_SIZE];
  register DATUM *sp = eval_stack - 1;

  FOR_LOOP * volatile for_loop_stack = (FOR_LOOP *) 0;

  /* This stack is for global variable veils. */

  DATUM *veil_stack[VEIL_STACK_SIZE];
  DATUM ** volatile vp = veil_stack - 2;

  /* These stacks are for TRY and CATCH. */

  EXCEPTION exception_stack[EXCEPTION_STACK_SIZE];
  int exp_ptr = -1;

  INST *cdp_stack[CDP_STACK_SIZE];
  INST **cdp_ptr = cdp_stack - 1;

  /*
   * Save the line and file, so we can restore the global values
   * when we're done here.
   */

  int line = curr_line_no;
  char *file = curr_file;

  CATCH_SIGPROF;		/* start catching profiler signals */

  (void) memset (eval_stack, 0, sizeof (eval_stack));

  WITH_HANDLING
  {

    while (1)
#ifdef apollo
      {
	int op = cdp++->op;	/* avoid a nasty bug on old apollo */
	switch (op)
#else
      switch (cdp++->op)
#endif
	{
	case OP_HALT:

	  assert (sp == eval_stack - 1);
	  assert (!for_loop_stack);

	  sp++;
	  sp->type = D_NULL;
	  goto out;

	case OP_RET:

	  assert (sp == eval_stack);

	  /* we might have returned in the middle of a for_loop(s) */

	  CLEANUP_FOR_LOOP_STACK ();

	  /* for return inside try */

	  exp_ptr = -1;
	  cdp_ptr = cdp_stack - 1;
	  exception_head = &my;

	  goto out;


	case OP_PRINT:

	  datum_print (sp);
	  /* fall thru */

	case OP_POP:

	  delete_datum (sp);
	  sp--;
	  break;

	case OP_PUSHC:
	case OP_PUSHI:

	  inc_sp ();
	  copy_datum (sp, cdp++->ptr);
	  break;

	case OP_LPUSHI:

	  inc_sp ();
	  copy_datum (sp, fp + cdp++->op);
	  break;

	case OP_PUSHS:		/* push string */
	  {
	    char *s = (char *) cdp++->ptr;
	    int len = strlen (s) + 1;

	    inc_sp ();
	    sp->type = D_NULL;	/* nothing to free */
	    PTR (sp) = (void *) strcpy (MALLOC (len), s);
	  }
	  break;

	case OP_PUSHS2E:
	  {
	    char *s = (char *) cdp++->ptr;

	    s = strcpy (MALLOC (strlen (s) + 1), s);
	    inc_sp ();
	    sp->type = D_ENTITY;
	    PTR (sp) = (void *) char_to_scalar (s);
	  }
	  break;

	case OP_PUSHA:

	  inc_sp ();
	  sp->type = D_NULL;	/* used only when clearing stack */
	  sp->data.ptr = cdp++->ptr;
	  break;

	case OP_LPUSHA:

	  inc_sp ();
	  sp->type = D_NULL;
	  PTR (sp) = (void *) (fp + cdp++->op);
	  break;

	case OP_PUSH_NULL:
	  inc_sp ();
	  sp->type = D_ENTITY;
	  sp->data.ptr = (void *) 0;
	  break;

	case OP_ADD:

	  switch (cast2_datum (--sp))
	    {
	    case D_NULL:
	      RT_ABORT ("Add operation on NULL.");
	      break;

	    case D_INT:
	      IVAL (sp) += (sp + 1)->data.ival;
	      break;

	    case D_REAL:
	      RVAL (sp) += RVAL (sp + 1);
	      break;

	    case D_ENTITY:
	      EBINOP (BO_ADD, sp);
	      break;
	    }
	  break;

	case OP_SUB:

	  switch (cast2_datum (--sp))
	    {
	    case D_NULL:
	      RT_ABORT ("Subtract operation on NULL.");
	      break;

	    case D_INT:
	      IVAL (sp) -= (sp + 1)->data.ival;
	      break;

	    case D_REAL:
	      RVAL (sp) -= RVAL (sp + 1);
	      break;

	    case D_ENTITY:
	      EBINOP (BO_SUB, sp);
	      break;
	    }
	  break;

	case OP_MUL:

	  switch (cast2_datum (--sp))
	    {
	    case D_NULL:
	      RT_ABORT ("Multiply operation on NULL.");
	      break;

	    case D_INT:
	      IVAL (sp) *= (sp + 1)->data.ival;
	      break;

	    case D_REAL:
	      RVAL (sp) *= RVAL (sp + 1);
	      break;

	    case D_ENTITY:
	      EBINOP (BO_MUL, sp);
	      break;
	    }
	  break;

	case OP_PROD:

	  switch (cast2_datum (--sp))
	    {
	    case D_NULL:
	      RT_ABORT ("Product operation on NULL.");
	      break;

	    case D_INT:
	      IVAL (sp) *= (sp + 1)->data.ival;
	      break;

	    case D_REAL:
	      RVAL (sp) *= RVAL (sp + 1);
	      break;

	    case D_ENTITY:
	      EBINOP (BO_PROD, sp);
	      break;
	    }
	  break;

	case OP_DIV:

	  switch (cast2_datum (--sp))
	    {
	    case D_NULL:
	      RT_ABORT ("Divide operation on NULL.");
	      break;

	    case D_INT:

	      if (IVAL (sp + 1) == 0)
		{
		  RT_ABORT ("Division by zero.");
		}

	      {
		div_t result;

		result = div (IVAL (sp), IVAL (sp + 1));
		if (result.rem != 0)
		  {
		    /* convert to real */
		    sp->type = D_REAL;
		    RVAL (sp) = (double) IVAL (sp);
		    RVAL (sp + 1) = (double) IVAL (sp + 1);
		    RVAL (sp) /= RVAL (sp + 1);
		  }
		else
		  IVAL (sp) = result.quot;


	      }
	      break;

	    case D_REAL:
	      RVAL (sp) /= RVAL (sp + 1);
	      break;

	    case D_ENTITY:
	      EBINOP (BO_DIV, sp);
	      break;
	    }
	  break;

	case OP_MOD:

	  switch (cast2_datum (--sp))
	    {
	    case D_NULL:
	      RT_ABORT ("Modulus operation on NULL.");
	      break;

	    case D_INT:
	      if (IVAL (sp + 1) == 0)
		RT_ABORT ("Division by zero.");
	      IVAL (sp) %= IVAL (sp + 1);
	      break;

	    case D_REAL:
	      RVAL (sp) =
		fmod (RVAL (sp), RVAL (sp + 1));
	      break;

	    case D_ENTITY:
	      EBINOP (BO_MOD, sp);
	      break;
	    }
	  break;

	case OP_UMINUS:

	  switch (sp->type)
	    {
	    case D_NULL:
	      RT_ABORT ("Negation of NULL.");

	    case D_INT:
	      IVAL (sp) = -IVAL (sp);
	      break;

	    case D_REAL:
	      RVAL (sp) = -RVAL (sp);
	      break;

	    case D_ENTITY:
	      EUNOP (UO_NEG, sp);
	      break;
	    }
	  break;

	case OP_NOT:

	  switch (sp->type)
	    {
	    case D_NULL:
	      RT_ABORT ("Not operation on NULL.");

	    case D_INT:
	      IVAL (sp) = !IVAL (sp);
	      break;

	    case D_REAL:
	      IVAL (sp) = !RVAL (sp);
	      sp->type = D_INT;
	      break;

	    case D_ENTITY:
	      EUNOP (UO_NOT, sp);
	      break;
	    }
	  break;

	case OP_TRANS:

	  if (cast1_to_entity (sp) == D_NULL)
	    RT_ABORT ("Transpose operation on NULL.");

	  EUNOP (UO_TRANS, sp);
	  break;

	case OP_POWER:

	  switch (cast2_datum (--sp))
	    {
	    case D_NULL:
	      RT_ABORT ("Power operation on NULL.");
	      break;

	    case D_INT:
	      if (IVAL (sp + 1) < 0)
		{
		  if (IVAL (sp) == 0)
		    RT_ABORT ("Invalid power operation (1/0).");
		  RVAL (sp) = pow ((REAL) IVAL (sp), (REAL) IVAL (sp + 1));
		  sp->type = D_REAL;
		}
	      else
		{
		  if (IVAL (sp) == 0 && IVAL (sp + 1) == 0)
		    RT_ABORT ("Undefined power operation (0^0).");
		  IVAL (sp) = (int) round (pow ((REAL) IVAL (sp),
						(REAL) IVAL (sp + 1)));
		}
	      break;

	    case D_REAL:
	      if (RVAL (sp) < 0 && ((int) RVAL (sp + 1)) != RVAL (sp + 1))
		{
		  if (cast2_to_entity (sp) == D_NULL)
		    RT_ABORT ("Power operation on NULL.");
		  EBINOP (BO_POWER, sp);
		}
	      else
		{
		  if (RVAL (sp) == 0.0 && RVAL (sp + 1) <= 0.0)
		    {
		      if (RVAL (sp + 1) == 0.0)
			fail ("Undefined power operation (0^0).");
		      else
			fail ("Invalid power operation (1/0).");

		      raise_exception ();
		    }
		  RVAL (sp) = pow (RVAL (sp), RVAL (sp + 1));
		}
	      break;

	    case D_ENTITY:
	      EBINOP (BO_POWER, sp);
	      break;
	    }
	  break;

	case OP_ASSIGN:
	  /* assign sp[0] to *sp[-1] */

	  sp--;
	  assign_datum (D_PTR (sp), sp + 1);
	  copy_datum (sp, sp + 1);
	  delete_datum (sp + 1);
	  break;

	case OP_PILE:
	  if (cast2_to_entity (--sp) == D_NULL)
	    RT_ABORT ("Pile on NULL.");

	  EBINOP (BO_PILE, sp);
	  break;

	case OP_MATRIX:
	  /* convert value at top of stack to matrix */
	  if (cast1_to_entity (sp) == D_NULL)
	    RT_ABORT ("Matrix operation on NULL.");

	  sp->type = D_NULL;
	  sp->data.ptr = (void *) bi_matrix (1, E_PTR (sp));
	  sp->type = D_ENTITY;
	  break;


	case OP_APPEND:
	  if (cast2_to_entity (--sp) == D_NULL)
	    RT_ABORT ("Append operation on NULL.");

	  EBINOP (BO_APPEND, sp);
	  break;


	case OP_VSEQ:
	  if (cast1_to_entity (sp) == D_NULL ||
	      (sp -= 2, cast2_to_entity (sp)) == D_NULL)
	    RT_ABORT ("Generate on NULL.");

	  sp->type = (sp + 1)->type = (sp + 2)->type = D_NULL;
	  sp->data.ptr =
	    generate_entity (E_PTR (sp), E_PTR (sp + 1), E_PTR (sp + 2));
	  sp->type = D_ENTITY;
	  break;


	case OP_P1D:
	  /*
	   * compute `a.b.c...[e]'
	   * sp[0] holds `e'
	   * sp[-1] through sp[-memcnt] hold strings `b', `c', etc.
	   * sp[-1-memcnt] holds entity `a'
	   */

	  {
	    int memcnt = cdp++->op;

	    if (memcnt != 0)
	      {
		char *memv[32];
		int i;

		sp -= memcnt + 1;
		for (i = 0; i < memcnt; i++)
		  memv[i] = (char *) PTR (sp + i + 1);

		if (cast1_to_entity (sp) == D_NULL)
		  {
		    for (i = 0; i < memcnt; i++)
		      FREE_CHAR (memv[i]);
		    RT_ABORT ("Member reference on NULL.");
		  }

		sp->type = D_NULL;
		PTR (sp) =
		  (void *) reference_members (E_PTR (sp), memcnt, memv);
		if (PTR (sp))
		  sp->type = D_ENTITY;

		/* rearrange stack */

		sp[1] = sp[memcnt + 1];
		(sp + memcnt + 1)->type = D_NULL;
		sp++;
	      }

	    sp--;
	    if (cast2_to_entity (sp) == D_NULL)
	      RT_ABORT ("Partition on NULL.");
	    sp->type = (sp + 1)->type = D_NULL;
	    PTR (sp) = partition_1d (E_PTR (sp), E_PTR (sp + 1));
	    sp->type = D_ENTITY;
	  }
	  break;


	case OP_ASSIGN_SV:

	  /*
	   * execute `V.a.b...c.d[e]=f'
	   * sp[0] has `f'
	   * sp[-1] has `e'
	   * sp[-2] through sp[-1-memcnt] has strings "a", "b", ..., "c", "d"
	   * sp[-2-memcnt] has address of `V'
	   */

	  {
	    DATUM * volatile V;
	    ENTITY *vec = NULL;	/* holds V.a.b...c.d */
	    ENTITY *head = NULL;	/* holds V */
	    ENTITY *tmp = NULL;
	    int i;
	    int memcnt = cdp++->op;
	    DATUM * volatile p = sp - memcnt - 1;
	    char *memv[32];
	    char *assign_on_null = "Assign subvector on NULL.";

	    WITH_HANDLING
	    {

	      sp--;
	      if (cast2_to_entity (sp) == D_NULL)
		RT_ABORT (assign_on_null);

	      /*
	       * Make a copy of `V' to use as `head'; keep the
	       * original in `V'.
	       */

	      sp -= memcnt + 1;
	      V = D_PTR (sp);
	      copy_datum (sp, V);
	      if (cast1_to_entity (sp) == D_NULL)
		RT_ABORT (assign_on_null);
	      head = E_PTR (sp);

	      /* Put copy of `f' at top of stack for return. */

	      copy_datum (sp, sp + 2 + memcnt);

	      if (memcnt)
		{

		  /* Get the vector. */
		  /* (Duplicate id strings---we'll need them later.) */

		  for (i = 0; i < memcnt; i++)
		    memv[i] = dup_char ((char *) PTR (p + i));

		  vec = reference_members (copy_entity (head),
					   memcnt, memv);
		  if (!vec)
		    {
		      fail (assign_on_null);
		      raise_exception ();
		    }

		  /* Now make the assignment to the vector in `vec'. */

		  /*
		   * This gets messy.  The problem is that the
		   * `assign_subvector' function takes a dangerous
		   * shortcut, modifying `vec' even if it sees two
		   * references to it.  (It does that because, in the
		   * case where memcnt==0, one reference to head is
		   * kept in `V' and is used again only if we raise
		   * an exception.)  Here we'll make an extra copy
		   * of `vec' to keep us out of trouble.
		   */

		  tmp = copy_entity (vec);

		  (sp + 1 + memcnt)->type = (sp + 2 + memcnt)->type = D_NULL;
		  vec = assign_subvector (EAT (vec), E_PTR (sp + 1 + memcnt),
					  E_PTR (sp + 2 + memcnt));

		  /* Get rid of the extra copy. */

		  delete_entity (tmp);
		  EAT (tmp);

		  /* Put `vec' where it belongs in `head'. */

		  for (i = 0; i < memcnt; i++)
		    memv[i] = (char *) PTR (p + i);
		  head = assign_members (EAT (head), EAT (vec),
					 memcnt, memv);

		}
	      else
		{

		  /*
		   * There were no "a", "b", etc. strings; it's
		   * just V[e]=f.
		   */

		  (sp + 1)->type = (sp + 2)->type = D_NULL;
		  head = assign_subvector (EAT (head), E_PTR (sp + 1),
					   E_PTR (sp + 2));

		}

	    }
	    ON_EXCEPTION
	    {
	      for (i = 0; i < memcnt; i++)
		FREE_CHAR ((char *) PTR (p + i));
	      delete_3_entities (head, vec, tmp);
	    }
	    END_EXCEPTION;

	    delete_datum (V);
	    V->type = D_ENTITY;
	    PTR (V) = head;
	  }
	  break;


	case OP_P2D:
	  /*
	   * compute `a.b.c...[e1;e2]'
	   * sp[0] holds `e2'
	   * sp[-1] holds `e1'
	   * sp[-2] through sp[-1-memcnt] hold strings `b', `c', etc.
	   * sp[-2-memcnt] holds entity `a'
	   */

	  {
	    int flag = 0;
	    int memcnt = cdp++->op;

	    if (memcnt != 0)
	      {
		char *memv[32];
		int i;

		sp -= memcnt + 2;
		for (i = 0; i < memcnt; i++)
		  memv[i] = (char *) PTR (sp + i + 1);

		if (cast1_to_entity (sp) == D_NULL)
		  {
		    for (i = 0; i < memcnt; i++)
		      FREE_CHAR (memv[i]);
		    RT_ABORT ("Member reference on NULL.");
		  }

		sp->type = D_NULL;
		PTR (sp) =
		  (void *) reference_members (E_PTR (sp), memcnt, memv);
		if (PTR (sp))
		  sp->type = D_ENTITY;

		/* rearrange stack */

		sp[1] = sp[memcnt + 1];
		(sp + memcnt + 1)->type = D_NULL;
		sp[2] = sp[memcnt + 2];
		(sp + memcnt + 2)->type = D_NULL;
		sp += 2;
	      }

	    if (cast1_to_entity (sp) == D_NULL)
	      flag = 1;
	    else
	      {
		sp -= 2;
		if (cast2_to_entity (sp) == D_NULL)
		  flag = 1;
	      }

	    if (flag)
	      RT_ABORT ("Partition on NULL.");

	    sp->type = (sp + 1)->type = (sp + 2)->type = D_NULL;
	    sp->data.ptr = partition_2d (E_PTR (sp),
					 E_PTR (sp + 1), E_PTR (sp + 2));
	    sp->type = D_ENTITY;
	  }
	  break;


	case OP_ASSIGN_SM:

	  /*
	   * execute `V.a.b...c.d[e1;e2]=f'
	   * sp[0] has `f'
	   * sp[-1] has `e2'
	   * sp[-2] has `e1'
	   * sp[-3] through sp[-2-memcnt] has strings "a", "b", ..., "c", "d"
	   * sp[-3-memcnt] has address of `V'
	   */

	  {
	    DATUM * volatile V;
	    ENTITY *mat = NULL;
	    ENTITY *head = NULL;
	    ENTITY *tmp = NULL;
	    int i;
	    int memcnt = cdp++->op;
	    DATUM * volatile p = sp - memcnt - 2;
	    char *memv[32];
	    char *assign_on_null = "Assign submatrix on NULL.";

	    WITH_HANDLING
	    {
	      if (cast1_to_entity (sp) == D_NULL)
		RT_ABORT (assign_on_null);
	      sp -= 2;
	      if (cast2_to_entity (sp) == D_NULL)
		RT_ABORT (assign_on_null);

	      /*
	       * Make a copy of `V' to use as `head'; keep the
	       * original in `V'.
	       */

	      sp -= memcnt + 1;
	      V = D_PTR (sp);
	      copy_datum (sp, V);
	      if (cast1_to_entity (sp) == D_NULL)
		RT_ABORT ("Assign submatrix on NULL.");
	      head = E_PTR (sp);

	      /* Put copy of `f' at top of stack for return. */

	      copy_datum (sp, sp + 3 + memcnt);

	      if (memcnt)
		{

		  /* Get the matrix. */
		  /* (Duplicate id strings---we'll need them later.) */

		  for (i = 0; i < memcnt; i++)
		    memv[i] = dup_char ((char *) PTR (p + i));

		  mat = reference_members (copy_entity (head),
					   memcnt, memv);
		  if (!mat)
		    {
		      fail (assign_on_null);
		      raise_exception ();
		    }

		  /* Now make the assignment to the matrix in `mat'. */

		  /*
		   * This gets messy.  The problem is that the
		   * `assign_submatrix' function takes a dangerous
		   * shortcut, modifying `mat' even if it sees two
		   * references to it.  (It does that because, in the
		   * case where memcnt==0, one reference to head is
		   * kept in `V' and is used again only if we raise
		   * an exception.)  Here we'll make an extra copy
		   * of `mat' to keep us out of trouble.
		   */

		  tmp = copy_entity (mat);

		  (sp + 1 + memcnt)->type = (sp + 2 + memcnt)->type =
		    (sp + 3 + memcnt)->type = D_NULL;
		  mat = assign_submatrix (EAT (mat), E_PTR (sp + 1 + memcnt),
					  E_PTR (sp + 2 + memcnt),
					  E_PTR (sp + 3 + memcnt));

		  /* Get rid of the extra copy. */

		  delete_entity (tmp);
		  EAT (tmp);

		  /* Now put `mat' where it belongs in `head'. */

		  for (i = 0; i < memcnt; i++)
		    memv[i] = (char *) PTR (p + i);
		  head = assign_members (EAT (head), EAT (mat),
					 memcnt, memv);

		}
	      else
		{

		  /*
		   * There were no "a", "b", etc. strings; it's
		   * just V[e1;e2]=f.
		   */

		  (sp + 1)->type = (sp + 2)->type = (sp + 3)->type = D_NULL;
		  head = assign_submatrix (EAT (head), E_PTR (sp + 1),
					   E_PTR (sp + 2), E_PTR (sp + 3));
		}
	    }
	    ON_EXCEPTION
	    {
	      for (i = 0; i < memcnt; i++)
		FREE_CHAR ((char *) PTR (p + i));
	      delete_3_entities (head, mat, tmp);
	    }
	    END_EXCEPTION;

	    delete_datum (V);
	    V->type = D_ENTITY;
	    PTR (V) = head;
	  }
	  break;

	case OP_TRY:

	  if (sigint_flag) raise_exception ();

	  {
	    inc_ep ();
	    if (!setjmp (exception_stack[exp_ptr].jb))
	      {
		exception_push (&exception_stack[exp_ptr]);
		inc_cp ();
		*cdp_ptr = cdp++;
	      }
	    else
	      {
		/* cleanup the stack */
		for (sp = eval_stack; sp < eval_stack + EVAL_STACK_SIZE; sp++)
		  delete_datum (sp);
		sp = eval_stack - 1;

		exp_ptr--;
		cdp = *cdp_ptr--;
		cdp += cdp->op;
	      }
	  }
	  break;

	case OP_VEIL:

	  /* put global on veil stack */

	  inc_vp ();
	  *vp = cdp++->ptr;
	  *(vp+1) = new_DATUM ();
	  (*(vp+1))->type = ((SYMTAB *)(*vp))->stval.datum->type;
	  (*(vp+1))->data = ((SYMTAB *)(*vp))->stval.datum->data;
	  copy_datum (((SYMTAB *)(*vp))->stval.datum, *(vp+1));
	  break;

	case OP_CATCH:

	  exp_ptr--;
	  cdp_ptr--;
	  (void) exception_pop ();
	  cdp += cdp->op;
	  break;

	case OP_JMP:

	  if (sigint_flag)
	    raise_exception ();
	  cdp += cdp->op;
	  break;

	case OP_JZ:
	  if (sigint_flag)
	    raise_exception ();
	  if (datum_test (sp--))
	    cdp++;		/* `datum_test' deletes datum. */
	  else
	    cdp += cdp->op;
	  break;

	case OP_JNZ:
	  if (sigint_flag)
	    raise_exception ();
	  if (!datum_test (sp--))
	    cdp++;		/* `datum_test' deletes datum. */
	  else
	    cdp += cdp->op;
	  break;

	case OP_EQ:

	  switch (cast2_datum (--sp))
	    {
	    case D_NULL:
	      {
		int i = sp->type == (sp + 1)->type;

		/* One of them might be an entity. */
		delete_datum (sp);
		delete_datum (sp + 1);

		sp->type = D_INT;
		IVAL (sp) = i;
	      }
	      break;

	    case D_INT:
	      IVAL (sp) = IVAL (sp) == (sp + 1)->data.ival;
	      break;

	    case D_REAL:
	      IVAL (sp) = RVAL (sp) == RVAL (sp + 1);
	      sp->type = D_INT;
	      break;

	    case D_ENTITY:
	      EBINOP (BO_EQ, sp);
	      break;
	    }
	  break;

	case OP_NE:

	  switch (cast2_datum (--sp))
	    {
	    case D_NULL:
	      {
		int i = sp->type != (sp + 1)->type;

		/* One of them might be an entity. */
		delete_datum (sp);
		delete_datum (sp + 1);

		sp->type = D_INT;
		IVAL (sp) = i;
	      }
	      break;

	    case D_INT:
	      IVAL (sp) = IVAL (sp) != (sp + 1)->data.ival;
	      break;

	    case D_REAL:
	      IVAL (sp) = RVAL (sp) != RVAL (sp + 1);
	      sp->type = D_INT;
	      break;

	    case D_ENTITY:
	      EBINOP (BO_NE, sp);
	      break;
	    }
	  break;

	case OP_LT:

	  switch (cast2_datum (--sp))
	    {
	    case D_NULL:
	      RT_ABORT ("Less-than operation on NULL.");
	      break;

	    case D_INT:
	      IVAL (sp) = IVAL (sp) < (sp + 1)->data.ival;
	      break;

	    case D_REAL:
	      IVAL (sp) = RVAL (sp) < RVAL (sp + 1);
	      sp->type = D_INT;
	      break;

	    case D_ENTITY:
	      EBINOP (BO_LT, sp);
	      break;
	    }
	  break;

	case OP_LTE:

	  switch (cast2_datum (--sp))
	    {
	    case D_NULL:
	      RT_ABORT ("Less than or equal operation on NULL.");
	      break;

	    case D_INT:
	      IVAL (sp) = IVAL (sp) <= (sp + 1)->data.ival;
	      break;

	    case D_REAL:
	      IVAL (sp) = RVAL (sp) <= RVAL (sp + 1);
	      sp->type = D_INT;
	      break;

	    case D_ENTITY:
	      EBINOP (BO_LTE, sp);
	      break;
	    }
	  break;

	case OP_GT:

	  switch (cast2_datum (--sp))
	    {
	    case D_NULL:
	      RT_ABORT ("Greater than operation on NULL.");
	      break;

	    case D_INT:
	      IVAL (sp) = IVAL (sp) > (sp + 1)->data.ival;
	      break;

	    case D_REAL:
	      IVAL (sp) = RVAL (sp) > RVAL (sp + 1);
	      sp->type = D_INT;
	      break;

	    case D_ENTITY:
	      EBINOP (BO_GT, sp);
	      break;
	    }
	  break;

	case OP_GTE:

	  switch (cast2_datum (--sp))
	    {
	    case D_NULL:
	      RT_ABORT ("Greater than or equal op on NULL.");
	      break;

	    case D_INT:
	      IVAL (sp) = IVAL (sp) >= (sp + 1)->data.ival;
	      break;

	    case D_REAL:
	      IVAL (sp) = RVAL (sp) >= RVAL (sp + 1);
	      sp->type = D_INT;
	      break;

	    case D_ENTITY:
	      EBINOP (BO_GTE, sp);
	      break;
	    }
	  break;

	case OP_AND:

	  switch (cast2_datum (--sp))
	    {
	    case D_NULL:
	      RT_ABORT ("Logical and on NULL.");
	      break;

	    case D_INT:
	      IVAL (sp) = IVAL (sp) && (sp + 1)->data.ival;
	      break;

	    case D_REAL:
	      IVAL (sp) = RVAL (sp) && RVAL (sp + 1);
	      sp->type = D_INT;
	      break;

	    case D_ENTITY:
	      EBINOP (BO_AND, sp);
	      break;
	    }
	  break;

	case OP_OR:

	  switch (cast2_datum (--sp))
	    {
	    case D_NULL:
	      RT_ABORT ("Logical or on NULL.");
	      break;

	    case D_INT:
	      IVAL (sp) = IVAL (sp) || (sp + 1)->data.ival;
	      break;

	    case D_REAL:
	      IVAL (sp) = RVAL (sp) || RVAL (sp + 1);
	      sp->type = D_INT;
	      break;

	    case D_ENTITY:
	      EBINOP (BO_OR, sp);
	      break;
	    }
	  break;

	  /* table ops */
	case OP_MK_TABLE:
	  inc_sp ();
	  sp->type = D_ENTITY;
	  PTR (sp) = make_table ();
	  break;

	case OP_ID_TABLE:
	  /*
	   * This is a hack that may go away if datums go away.  It
	   * resolves {X} at run time ;
	   * setting things up to fall thru to OP_R_TABLE
	   */
	  {
	    SYMTAB *p;

	    inc_sp ();
	    PTR (sp) = PTR (sp - 1);

	    if ((p = find (PTR (sp), NO_CREATE)) &&
		p->scope == 0)
	      {
		copy_datum (sp - 1, p->stval.datum);
	      }
	    else
	      {
		assert (!p || p->scope == NO_SCOPE);
		(sp - 1)->type = D_NULL;
	      }
	  }
	  /* FALL THRU */

	case OP_R_TABLE:
	  /*
	   * sp[0] has an identifier
	   * sp[-1]  has an expression
	   * sp[-2]  has a table 
	   */

	  sp -= 2;
	  /* sp[1] NULL is OK */
	  if (cast1_to_entity (sp + 1) == D_NULL)
	    PTR (sp + 1) = (void *) 0;
	  else
	    (sp + 1)->type = D_NULL;
	  sp->type = D_NULL;
	  PTR (sp) = (void *) assign_member_table (
		 (TABLE *) PTR (sp), E_PTR (sp + 1), (char *) PTR (sp + 2));
	  sp->type = D_ENTITY;
	  break;


	case OP_E2STR:

	  /*
	   * Convert an entity to string(character) for
	   * member into a table.
	   */

	  if (cast1_to_entity (sp) == D_NULL)
	    {
	      fail ("Illegal table member.");
	      raise_exception ();
	    }

	  sp->type = D_NULL;
	  PTR (sp) = entity_to_string (E_PTR (sp));

	  break;

	case OP_REF_MEMB:
	  /*
	   * top of stack has n = cdp.op strings
	   * next down is a table
	   * put  table.id.id...  at top of stack
	   */

	  {
	    char *memv[32];
	    int cnt = cdp++->op;
	    int i;

	    sp -= cnt;
	    if (cast1_to_entity (sp) == D_NULL)
	      {
		for (i = 1; i <= cnt; i++)
		  FREE (PTR (sp + i));
		RT_ABORT ("Member reference on NULL.");
	      }

	    sp->type = D_NULL;

	    if (cnt > 1)
	      {
		for (i = 1; i <= cnt; i++)
		  memv[i - 1] = (char *) PTR (sp + i);

		PTR (sp) = (void *) reference_members (
						     E_PTR (sp), cnt, memv);
	      }
	    else
	      PTR (sp) = (void *) reference_member (
					 E_PTR (sp), (char *) PTR (sp + 1));

	    if (PTR (sp) != NULL)
	      sp->type = D_ENTITY;
	    break;
	  }

	case OP_ASG_MEMB:
	  {
	    int cnt = cdp++->op;
	    int i;
	    char *memv[32];
	    DATUM *t;
	    ENTITY *t_temp;

	    sp -= cnt + 1;
	    t = D_PTR (sp);
	    (void) copy_datum (sp, t);

	    if (cast1_to_entity (sp) == D_NULL)
	      {
		for (i = 1; i <= cnt; i++)
		  FREE_CHAR (PTR (sp + i));	/*free all ids */
		RT_ABORT ("Member assignment to NULL.");
	      }

	    t_temp = E_PTR (sp);
	    /* put expected result into sp */
	    copy_datum (sp, sp + cnt + 1);

	    /* cast sp+cnt+1 to entity, NULL is OK */
	    if (cast1_to_entity (sp + cnt + 1) == D_NULL)
	      PTR (sp + cnt + 1) = (void *) 0;
	    else
	      (sp + cnt + 1)->type = D_NULL;

	    if (cnt == 1)
	      {			/* usual case */

		/*
		 * We've been promised by `assign_member' that, if
		 * `t_temp' is a table, it won't raise an exception.
		 * Knowing that, we can go ahead and delete `t'
		 * now and maybe avoid an unnecessary duplication of
		 * a table.
		 */

		if (t_temp->class == table)
		  delete_datum (t);

		t_temp = assign_member (t_temp, E_PTR (sp + 2),
					(char *) PTR (sp + 1));
	      }
	    else
	      {

		for (i = 1; i <= cnt; i++)
		  memv[i - 1] = (char *) PTR (sp + i);
		t_temp = assign_members (t_temp, E_PTR (sp + cnt + 1),
					 cnt, memv);
	      }
	    /* it worked !! */
	    delete_datum (t);
	    t->type = D_ENTITY;
	    PTR (t) = t_temp;
	  }
	  break;

	case OP_CALL:

	  {
	    int i;
	    FUNCTION *func;
	    int num_args = cdp++->op;
	    int arg_space	/* for user function locals */
	    = eval_stack + (EVAL_STACK_SIZE - 1) - sp;

	    if (arg_space < 0)
	      {
		/* this is unlikely */
		fail ("Eval stack overflow from user function parameters.");
		raise_exception ();
	      }

	    sp -= num_args;	/*sp now pts at callee */

	    if (sp->type != D_ENTITY ||
		E_PTR (sp)->class != function)
	      {
		RT_ABORT ("Call to non-function.");
	      }
	    else
	      func = (FUNCTION *) D_PTR (sp);

	    for (i = 1; i <= num_args; i++)
	      if (cast1_to_entity (sp + i) == D_NULL)
		PTR (sp + i) = NULL;

	    sp->type = D_NULL;
	    PTR (sp) = execute_function (
					  (FUNCTION *) sp->data.ptr,
					  num_args, sp + 1, arg_space);

	    if (PTR (sp) != NULL)
	      sp->type = D_ENTITY;
	  }
	  break;

	case OP_IFL:		/* init a FOR LOOP:   for ( i in V ) */

	  {
	    FOR_LOOP *flp = MALLOC (sizeof (FOR_LOOP));

	    flp->var_address = D_PTR (sp);
	    sp--;

	    /* stack it now -- in case of exception */
	    flp->list = (VECTOR *) 0;
	    flp->link = for_loop_stack;
	    for_loop_stack = flp;

	    if (cast1_to_entity (sp) == D_NULL)
	      {
		RT_ABORT ("for ( var in NULL )");
	      }

	    /* The "list" is a vector that we're going to step through. */

	    if (E_PTR (sp)->class != vector)
	      {
		sp->type = D_NULL;
		PTR (sp) = vector_entity (E_PTR (sp));
		sp->type = D_ENTITY;
	      }

	    /*
	     * We're transfering entity pointer for V in 
	     * datum stack into FOR_LOOP structure.
	     * No reference_counts need to be adjusted.
	     */

	    flp->list = (VECTOR *) E_PTR (sp);
	    sp->type = D_NULL;
	    sp--;

	    init_for_loop (flp);

	    cdp += cdp->op;
	  }
	  break;

	case OP_FL:

	  {
	    FOR_LOOP *flp = for_loop_stack;
	    DATUM *dp;

	    POLL_SIGINT ();
	    if (inc_for_loop (flp))
	      {
		/* load var */
		dp = flp->var_address;
		delete_datum (dp);
		if (flp->type > real)
		  {
		    dp->type = D_ENTITY;
		    PTR (dp) = flp->ep;
		    flp->ep = (ENTITY *) 0;
		  }
		else if (flp->type == real)
		  {
		    dp->type = D_REAL;
		    RVAL (dp) = flp->value.real;
		  }
		else
		  {		/* must be int */
		    dp->type = D_INT;
		    IVAL (dp) = flp->value.integer;
		  }

		cdp += cdp->op;
	      }
	    else		/* go to KFL */
	      cdp++;

	  }
	  break;

	case OP_KFL:

	  /* pop the for loop stack */
	  {
	    FOR_LOOP *flp = for_loop_stack;

	    for_loop_stack = flp->link;
	    delete_vector (flp->list);
	    FREE (flp);
	  }
	  break;

	case OP_SYMTAB:
	  inc_sp ();
	  sp->type = D_ENTITY;
	  PTR (sp) = symbols_to_table ();
	  break;

	case OP_PUSHI_SYM:

	  /* Push global symbol named by string at top of stack. */

	  {
	    SYMTAB *stp;

	    sp->type = D_NULL;

	    stp = find ((char *) PTR (sp), NO_CREATE);
	    FREE_CHAR (PTR (sp));

	    if (stp && !stp->scope)
	      copy_datum (sp, stp->stval.datum);

	    break;
	  }

	case OP_PUSHA_SYM:

	  /* Push address of symbol named by string at top of stack. */

	  {
	    SYMTAB *stp;

	    stp = find ((char *) PTR (sp), CREATE);
	    if (stp->scope == NO_SCOPE)
	      {
		stp->scope = 0;
		stp->stval.datum = new_DATUM ();
	      }
	    assert (stp->scope == 0);

	    sp->type = D_NULL;
	    sp->data.ptr = stp->stval.datum;

	    break;
	  }

	case OP_LINE:

	  /* update the line number */

	  curr_line_no = cdp++->op;
	  PROF_UPDATE_LINE (curr_line_no);	/* for profiler */

	  /* Check debugger breakpoints / stepping */
	  if (debugger_enabled)
	    debugger_check (curr_file, curr_line_no);

	  break;

	case OP_FILE:

	  /* update the file name */

	  curr_file = cdp++->ptr;
	  PROF_UPDATE_FILE (curr_file);		/* for profiler */
	  break;

	case OP_NOP:

	  /* no op */
	  break;

	case OP_SELF:

	  /* push reference to self */

	  inc_sp();
	  sp->type = D_ENTITY;
	  PTR(sp) = copy_function( current_function );
	  break;

	default:
	  wipeout ("bad op code");

	}
#ifdef apollo
      }
#endif

  out:;				/* This label must be here.  CANNOT jump over ON_EXCEPTION. */

  }
  ON_EXCEPTION
    {

      /* reset the file and line */
      curr_file = file;
      PROF_UPDATE_FILE (curr_file);
      curr_line_no = line;
      PROF_UPDATE_LINE (curr_line_no);

      /* cleanup the stack */
      for (sp = eval_stack; sp < eval_stack + EVAL_STACK_SIZE; sp++)
	delete_datum (sp);

      /* unveil globals */
      for (; vp >= veil_stack; vp-=2)
	{
	  delete_datum (((SYMTAB *)(*vp))->stval.datum);
	  ((SYMTAB *)(*vp))->stval.datum->type = (*(vp+1))->type;
	  ((SYMTAB *)(*vp))->stval.datum->data = (*(vp+1))->data;
	  FREE (*(vp+1));
	}

      CLEANUP_FOR_LOOP_STACK ();

      delete_function( current_function );

    }
  END_EXCEPTION;

  assert (sp == eval_stack);
  assert (exp_ptr == -1);
  assert (cdp_ptr == cdp_stack - 1);

  /* reset the file and line */
  curr_file = file;
  PROF_UPDATE_FILE (curr_file);
  curr_line_no = line;
  PROF_UPDATE_LINE (curr_line_no);

  /* unveil globals */
  for (; vp >= veil_stack; vp-=2)
    {
      delete_datum (((SYMTAB *)(*vp))->stval.datum);
      ((SYMTAB *)(*vp))->stval.datum->type = (*(vp+1))->type;
      ((SYMTAB *)(*vp))->stval.datum->data = (*(vp+1))->data;
      FREE (*(vp+1));
    }

  delete_function( current_function );

  return cast1_to_entity (sp) == D_NULL ? NULL : E_PTR (sp);

}
