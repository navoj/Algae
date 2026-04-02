/*
   da.c -- disassemble code

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: da.c,v 1.7 2003/12/13 01:35:10 ksh Exp $";

#include "entity.h"
#include "scalar.h"
#include "ptypes.h"
#include "code.h"
#include "for_loop.h"
#include  <stdio.h>
#include "function.h"
#include "exception.h"
#include "printf.h"
#include "table.h"

static struct
  {
    int op;
    char *name;
  }
simple_op_table[] =
{
  OP_ASSIGN, "assign",
    OP_POP, "pop",
    OP_PRINT, "print",
    OP_ADD, "add",
    OP_SUB, "sub",
    OP_MUL, "mul",
    OP_DIV, "div",
    OP_MOD, "mod",
    OP_PROD, "prod",
    OP_UMINUS, "uminus",
    OP_NOT, "not",
    OP_TRANS, "trans",
    OP_POWER, "pow",
    OP_PILE, "pile",
    OP_APPEND, "append",
    OP_MATRIX, "matrix",
    OP_VSEQ, "vseq",
    OP_PUSH_NULL, "push null",
    OP_EQ, "eq",
    OP_NE, "ne",
    OP_LT, "lt",
    OP_GT, "gt",
    OP_GTE, "gte",
    OP_LTE, "lte",
    OP_AND, "and",
    OP_OR, "or",
    OP_RET, "ret",
    OP_MK_TABLE, "mk_table",
    OP_R_TABLE, "r_table",
    OP_ID_TABLE, "id_table",
    OP_SYMTAB, "symtab",
    OP_E2STR, "e2str",
    OP_KFL, "kfl",
    OP_NOP, "nop",
    OP_PUSHA_SYM, "pusha_sym",
    OP_PUSHI_SYM, "pushi_sym",
    OP_SELF, "self",
    OP_TRY, "try",
    OP_CATCH, "catch",
    0, (char *) 0,
};


void
da (INST *start, FILE *fp)
{
  DATUM *dp;
  register INST *cp = start;
  int i;

  while (1)
    {
      POLL_SIGINT ();

      /* print the relative code address (label) */
      xfprintf (fp, "%03d ", cp - start);

      switch (cp++->op)
	{
	case OP_HALT:
	  xfprintf (fp, "halt\n");
	  return;

	case OP_PUSHS:
	  xfprintf (fp, "pushs\t\"%s\"\n", cp++->ptr);
	  break;

	case OP_PUSHS2E:
	  xfprintf (fp, "pushs2e\t\"%s\"\n", cp++->ptr);
	  break;

	case OP_PUSHC:
	  dp = (DATUM *) cp++->ptr;
	  switch (dp->type)
	    {
	    case D_INT:
	      xfprintf (fp, "pushc\t%d\n", dp->data.ival);
	      break;

	    case D_REAL:
	      xfprintf (fp, "pushc\t%.6g\n", dp->data.rval);
	      break;

	    case D_ENTITY:

	      assert (E_PTR (dp)->class == function);
	      xfprintf (fp, "pushc\t%s\n",
			class_string[E_PTR (dp)->class]);

	      break;

	    case D_NULL:
	      xfprintf (fp, "pushc\tNULL\n");
	      break;

	    default:
	      xfprintf (fp, "pushc\tWEIRD\n");
	      break;
	    }
	  break;

	case OP_PUSHA:
	  xfprintf (fp, "pusha\t%s\n", address_to_id (cp++->ptr));
	  break;

	case OP_PUSHI:
	  xfprintf (fp, "pushi\t%s\n", address_to_id (cp++->ptr));
	  break;

	case OP_LPUSHI:
	  xfprintf (fp, "lpushi\t%03d\n", cp++->op);
	  break;

	case OP_LPUSHA:
	  xfprintf (fp, "lpusha\t%03d\n", cp++->op);
	  break;

	case OP_TRY:
	  xfprintf (fp, "try\t\t%03d\n", cp + cp->op - start);
	  cp++;
	  break;

	case OP_CATCH:
	  xfprintf (fp, "catch\t%03d\n", cp + cp->op - start);
	  cp++;
	  break;

	case OP_JMP:
	  xfprintf (fp, "jmp\t\t%03d\n", cp + cp->op - start);
	  cp++;
	  break;

	case OP_JZ:
	  xfprintf (fp, "jz\t\t%03d\n", cp + cp->op - start);
	  cp++;
	  break;

	case OP_JNZ:
	  xfprintf (fp, "jnz\t%03d\n", cp + cp->op - start);
	  cp++;
	  break;

	case OP_REF_MEMB:
	  xfprintf (fp, "ref_memb\t%d\n", cp++->op);
	  break;

	case OP_ASG_MEMB:
	  xfprintf (fp, "asg_memb\t%d\n", cp++->op);
	  break;

	case OP_P1D:
	  xfprintf (fp, "p1d  \t%d\n", cp++->op);
	  break;

	case OP_P2D:
	  xfprintf (fp, "p2d  \t%d\n", cp++->op);
	  break;

	case OP_ASSIGN_SV:
	  xfprintf (fp, "asg_sv\t%d\n", cp++->op);
	  break;

	case OP_ASSIGN_SM:
	  xfprintf (fp, "asg_sm\t%d\n", cp++->op);
	  break;

	case OP_CALL:
	  xfprintf (fp, "call\t%d\n", cp++->op);
	  break;

	case OP_IFL:
	  {
	    int s = cp + cp->op - start;
	    cp++;
	    xfprintf (fp, "ifl\t%03d\n", s);
	  }
	  break;

	case OP_FL:		/* for loop */
	  {
	    int s = cp + cp->op - start;
	    cp++;
	    xfprintf (fp, "fl\t%03d\n", s);
	  }
	  break;

	case OP_LINE:		/* line number */
	  xfprintf (fp, "line\t%d\n", cp++->op);
	  break;

	case OP_FILE:		/* file name */
	  xfprintf (fp, "file\t%s\n", cp++->ptr);
	  break;

	case OP_VEIL:
	  xfprintf (fp, "veil\t%s\n", address_to_id (cp++->ptr));
	  break;

	default:
	  for (i = 0;
	       simple_op_table[i].op &&
	       simple_op_table[i].op != cp[-1].op;
	       i++
	    );

	  if (simple_op_table[i].op)
	    xfprintf (fp, "%s\n", simple_op_table[i].name);
	  else
	    xfprintf (fp, "unknown opcode 0x%x\n", cp[-1].op);
	  break;
	}
    }
}

void
da_function (UF_BLOCK *ufp, FILE *fp)
{
  xfprintf (fp, "FUNCTION at 0x%x (%d,%d)\n", ufp,
	    ufp->cargs, ufp->targs);
  da (ufp->code, fp);
}

void
var_ref_function (UF_BLOCK *u, FILE *fp)
{
  /*
   * Print lists of all the global variables referenced
   * or assigned in the given user function.
   */

  TABLE *tr = (TABLE *) make_table();
  TABLE *ta = (TABLE *) make_table();
  register INST *cp = u->code;

  while (1)
    {
      switch (cp++->op)
        {

        case OP_HALT:
          xfprintf (fp, "\tglobal variables referenced:\n");
          print_table (tr, fp);
          xfprintf (fp, "\tglobal variables assigned:\n");
          print_table (ta, fp);
          return;

        case OP_PUSHI:
          tr = (TABLE *) replace_in_table
            (tr, NULL, dup_char (address_to_id (cp++->ptr)));
          break;

        case OP_SYMTAB:
        case OP_PUSHI_SYM:
          tr = (TABLE *) replace_in_table
            (tr, NULL, dup_char ("$$"));
          break;

	case OP_PUSHA:
          ta = (TABLE *) replace_in_table
            (ta, NULL, dup_char (address_to_id (cp++->ptr)));
          break;

        case OP_PUSHA_SYM:
          ta = (TABLE *) replace_in_table
            (ta, NULL, dup_char ("$$"));
          break;

        case OP_PUSHS:
	case OP_PUSHS2E:
	case OP_PUSHC:
	case OP_LPUSHI:
	case OP_LPUSHA:
	case OP_JMP:
	case OP_JZ:
	case OP_JNZ:
	case OP_REF_MEMB:
	case OP_ASG_MEMB:
	case OP_P1D:
	case OP_P2D:
	case OP_ASSIGN_SV:
	case OP_ASSIGN_SM:
	case OP_CALL:
	case OP_IFL:
	case OP_FL:
	case OP_LINE:
	case OP_FILE:
	case OP_VEIL:

	  cp++;
	  break;

	default:

	  if (cp->op < 0 || cp->op >= NUM_OP_CODES)
	    {
	      fail ("Unknown opcode 0x%x.\n", cp->op);
	      raise_exception ();
	    }
        }
    }
}
