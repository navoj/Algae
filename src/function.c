/*
   function.c -- user functions

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: function.c,v 1.8 2003/08/07 02:34:09 ksh Exp $";

#include "function.h"
#include "entity.h"
#include "table.h"
#define  BI_FUNCTION_INIT
#include "builtin.h"
#include "code.h"
#include "psr.h"
#include "file_io.h"
#include "ptypes.h"
#include "get.h"
#include "put.h"

#if HAVE_DLOPEN
#include <dlfcn.h>
#endif

static void PROTO (strip_code, (INST * start));

/*
 * This array describes the members of the
 * FUNCTION structure.  The fields are `name',
 * and `id'.  The entries must be in
 * alphabetical order, and there must be
 * exactly one entry for each member of the
 * FUNCTION_MEMBER enumeration except END_Function.
 */

MEMBER_ID function_member_names[] =
{
  {"class", FunctionClass},
  {"ilk", FunctionIlk},
};

FUNCTION_MEMBER
function_member_search (char *s)
{
  MEMBER_ID *m;

  assert (s != NULL);

  m = (MEMBER_ID *) bsearch (s, function_member_names, END_Function, sizeof (MEMBER_ID), member_cmp);
  return ((m == NULL) ? END_Function : m->id);
}

FUNCTION *
make_function (ILK ilk, void *p)
{
  FUNCTION *retval = (FUNCTION *) MALLOC (sizeof (FUNCTION));
  retval->entity.ref_count = 1;
  retval->entity.class = function;
  retval->ilk = ilk;
  retval->stuff = NULL;
  retval->funct_p = p;
  return retval;
}

void
DB_delete_function (FUNCTION *p, char *file, int line)
{
  /*
   * This is the DEBUG version of `delete_function'.  It decrements
   * the function's reference count, and frees it if it's unreferenced.
   */

  if (p)
    {

      if (--p->entity.ref_count < 0)
	{
	  wipeout ("A function's \"ref_count\" went below zero:  %s, %d.",
		   file, line);
	}

      if (p->entity.ref_count >= 1000 || debug_level > 1)
	{
	  inform ("function \"ref_count\" decrement:  %x, %d, %s, %d.",
		  p, p->entity.ref_count, file, line);
	}

      if (p->entity.ref_count == 0)
	free_function (p);
    }
}

void
free_function (FUNCTION *p)
{
  assert (p->entity.ref_count == 0);
  assert (p->ilk == builtin || p->ilk == user);

  delete_table (p->stuff);

  if (p->ilk == user)
    {
      FREE (((UF_BLOCK *) p->funct_p)->code);
      FREE (p->funct_p);
    }

  p->entity.class = undefined_class;
  FREE (p);
}

ENTITY *
dup_function (FUNCTION *p)
{
  FUNCTION *f;

  /* Duplicate a function. */

  if (p->entity.ref_count == 1)
    return (ENT (p));

  f = (FUNCTION *) dup_mem (p, sizeof (FUNCTION));

  if (f->ilk == user)
    {
      f->funct_p = dup_mem (f->funct_p, sizeof (UF_BLOCK));
      ((UF_BLOCK *) f->funct_p)->code =
	dup_mem (((UF_BLOCK *) f->funct_p)->code,
		 code_size (((UF_BLOCK *) f->funct_p)->code));
    }

  f->entity.ref_count = 1;
  if (p->stuff) f->stuff = (TABLE *) copy_table (p->stuff);

  if (debug_level > 1)
    inform ("Function created:  %x.", f);

  delete_function (p);
  return (ENT (f));
}

void
bi_init (void)
{
  /* load builtins into global symbol table */

  SYMTAB *stp;
  struct bi_init *p = builtin_init_data;

  while (p->name)
    {
      stp = insert (p->name);
      stp->scope = 0;
      stp->stval.datum = MALLOC (sizeof (DATUM));
      stp->stval.datum->type = D_ENTITY;
      stp->stval.datum->data.ptr =
	(void *) make_function (builtin, &p->bi);

      p++;
    }
}

/* From execute(), we get to here on an OP_CALL.
   This function merely routes to execute_bi
   or execute_uf
 */
ENTITY *
execute_function (FUNCTION *fp, int num_args, DATUM *argp, int arg_space)
{
  ENTITY *r;

  switch (fp->ilk)
    {

    case builtin:

      /* We'll leak memory here if an exception occurs. */

      r = execute_bi ((BUILTIN *) fp->funct_p, num_args, argp);
      delete_function (fp);
      break;

    case user:

      /* We'll leak memory here if an exception occurs. */

      r = execute_uf ((UF_BLOCK *) fp->funct_p,
		      num_args, argp, arg_space,
                      (FUNCTION *) copy_function (fp));
      delete_function (fp);
      break;

    default:
      BAD_ILK (fp->ilk);
      delete_function (fp);
      raise_exception ();
    }

  return r;
}

/* executes a builtin function */

ENTITY *
execute_bi (BUILTIN *bp, int num_args, DATUM *argp)
{
#define MAX_BI_ARGS	99
  ENTITY *args[MAX_BI_ARGS];
  int i;
  static char too_many[] = "Too many arguments in \"execute_bi\".";

  if (bp->min_args > num_args)
    {
      fail ("At least %d argument%s expected in call.",
	    bp->min_args, PLURAL (bp->min_args));
      raise_exception ();
    }
  else if (bp->max_args < num_args)
    {
      fail ("At most %d argument%s expected in call.",
	    bp->max_args, PLURAL (bp->max_args));
      raise_exception ();
    }
  else if (num_args > MAX_BI_ARGS)
    {
      fail (too_many);
      raise_exception ();
    }

  for (i = 0; i < num_args; i++)
    {
      if ((args[i] = E_PTR (argp + i)) == NULL &&
	  !(1 << i & bp->arg_flags))
	{
	  fail ("NULL passed as %d%s argument.", i + 1, TH (i + 1));
	  raise_exception ();
	}
      argp[i].type = D_NULL;
    }

  for (i = num_args; i < bp->max_args; i++)
    {
      if (i >= 5)
	break;
      args[i] = NULL;
    }

  if (bp->max_args == bp->min_args)
    {
      switch (num_args)
	{
	case 0:
	  return ((ENTITY *(*)(void))bp->bi) ();
	case 1:
	  return ((ENTITY *(*)(ENTITY *))bp->bi) (args[0]);
	case 2:
	  return ((ENTITY *(*)(ENTITY *, ENTITY *))bp->bi) (args[0], args[1]);
	case 3:
	  return ((ENTITY *(*)(ENTITY *, ENTITY *, ENTITY *))bp->bi) (args[0], args[1], args[2]);
	case 4:
	  return ((ENTITY *(*)(ENTITY *, ENTITY *, ENTITY *, ENTITY *))bp->bi) (args[0], args[1], args[2],
				     args[3]);
	case 5:
	  return ((ENTITY *(*)(ENTITY *, ENTITY *, ENTITY *, ENTITY *, ENTITY *))bp->bi) (args[0], args[1], args[2],
				     args[3], args[4]);
	default:
	  wipeout (too_many);
	}
    }
  else
    {
      switch (bp->max_args)
	{
	case 1:
	  return ((ENTITY *(*)(int, ENTITY *))bp->bi) (num_args, args[0]);
	case 2:
	  return ((ENTITY *(*)(int, ENTITY *, ENTITY *))bp->bi) (num_args, args[0], args[1]);
	case 3:
	  return ((ENTITY *(*)(int, ENTITY *, ENTITY *, ENTITY *))bp->bi) (num_args, args[0], args[1],
				      args[2]);
	case 4:
	  return ((ENTITY *(*)(int, ENTITY *, ENTITY *, ENTITY *, ENTITY *))bp->bi) (num_args, args[0], args[1],
				      args[2], args[3]);
	case 5:
	  return ((ENTITY *(*)(int, ENTITY *, ENTITY *, ENTITY *, ENTITY *, ENTITY *))bp->bi) (num_args, args[0], args[1],
				      args[2], args[3], args[4]);
	default:
	  return ((ENTITY *(*)(int, ENTITY **))bp->bi) (num_args, args);
	}
    }
}

/* move a user function argument stack because it doesn't have
   room for all the locals
 */

static DATUM *
temp_stack (DATUM *old_base, int nargs, int targs)
{
  DATUM *new_base, *p, *q;

  new_base = MALLOC (targs * sizeof (DATUM));

  p = old_base;
  q = new_base;
  while (nargs)
    {
      nargs--;
      copy_datum (q, p);
      delete_datum (p);
      p++;
      q++;
    }
  return new_base;
}

ENTITY *
execute_uf (UF_BLOCK *ufp, int nargs, DATUM *argp, int arg_space, FUNCTION *fp)
{
  ENTITY *retval;
  int j;
  int stack_moved_flag;

  if (ufp->cargs < nargs)
    {
      fail ("user function called with too many arguments \
(%d -- %d expected)", nargs, ufp->cargs);
      delete_function (fp);
      raise_exception ();
    }

  if (ufp->targs - nargs > arg_space)
    {
      /* there is not enough space for locals so move the frame */
      stack_moved_flag = 1;
      argp = temp_stack (argp, nargs, ufp->targs);
    }
  else
    stack_moved_flag = 0;

  /* null out unused args and locals as datums */

  for (j = nargs + 1; j <= ufp->targs; j++)
    argp[j - 1].type = D_NULL;

  retval = execute (ufp->code, argp, fp);

  /* cleanup all arguments params and locals */
  for (j = 0; j < ufp->targs; j++)
    delete_datum (argp + j);

  if (stack_moved_flag)
    FREE (argp);

  return retval;
}

int
code_size (INST *start)
{
  /* Return size (in bytes) of user code. */

  register INST *cp = start;

  while (1)
    {

      POLL_SIGINT ();

      switch (cp->op)
	{

	case OP_HALT:

	  return (cp - start + 1) * sizeof (INST);

	case OP_PUSHS:
	case OP_PUSHS2E:
	case OP_PUSHC:
	case OP_PUSHA:
	case OP_PUSHI:
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

	  cp += 2;
	  break;

	default:

	  if (cp->op < 0 || cp->op >= NUM_OP_CODES)
	    {
	      fail ("Unknown opcode 0x%x.\n", cp->op);
	      raise_exception ();
	    }

	  cp++;

	}
    }
}

static void
strip_code (INST *start)
{
  /* Strip OP_LINE and OP_FILE from code. */

  register INST *cp = start;

  while (1)
    {

      POLL_SIGINT ();

      switch (cp->op)
	{

	case OP_HALT:

	  return;

	case OP_PUSHS:
	case OP_PUSHS2E:
	case OP_PUSHC:
	case OP_PUSHA:
	case OP_PUSHI:
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
	case OP_VEIL:

	  cp += 2;
	  break;

	case OP_LINE:
	case OP_FILE:

	  cp++->op = OP_NOP;
	  cp++->op = OP_NOP;
	  break;

	default:

	  if (cp->op < 0 || cp->op >= NUM_OP_CODES)
	    {
	      fail ("Unknown opcode 0x%x.\n", cp->op);
	      raise_exception ();
	    }

	  cp++;

	}
    }
}

ENTITY *
bi_strip (ENTITY *p)
{
  /* Strip file and line number info from a function. */

  if (p->class == function)
    {

      if (((FUNCTION *) p)->ilk == user)
	{

	  p = dup_function ((FUNCTION *) p);
	  strip_code (((UF_BLOCK *) (((FUNCTION *) p)->funct_p))->code);

	}

    }
  else
    {

      fail ("Can't strip a %s.", class_string[p->class]);
      delete_entity (p);
      raise_exception ();

    }

  return p;
}

int
put_function (FUNCTION *f, FILE *stream, struct ent_node *ent_tree)
{
  int i, size;
  char *name = NULL;
  struct bi_init *b = builtin_init_data;

  /* Write function `f' in binary form to file `stream'. */

  if (!WRITE_INT (&f->ilk, stream))
    goto err;

  /* Write any additional members. */

  if (f->stuff)
    {
      i = 1;			/* stuff follows */
      if (!WRITE_INT (&i, stream) ||
	  !put_entity ( copy_table (f->stuff), stream, ent_tree))
	goto err;
    }
  else
    {
      i = 0;			/* no stuff */
      if (!WRITE_INT (&i, stream))
	goto err;
    }

  /* Different handling for builtins and user functions. */

  switch (f->ilk)
    {

    case builtin:

      /* Write the BUILTIN structure. */

      if (!WRITE_INT (&((BUILTIN *) f->funct_p)->min_args, stream) ||
	  !WRITE_INT (&((BUILTIN *) f->funct_p)->max_args, stream) ||
	  !WRITE_INT (&((BUILTIN *) f->funct_p)->arg_flags, stream))
	goto err;

      /* Look up name of function and write it. */

      while ((name = b->name) != NULL)
	{
	  if ((b++)->bi.bi == ((BUILTIN *) f->funct_p)->bi)
	    {
	      size = strlen (name);
	      assert (size > 0);
	      if (!WRITE_INT (&size, stream))
		goto err;
	      if (fwrite (name, 1, size, stream) < (size_t) size)
		{
		  WRITE_WARN (stream);
		  goto err;
		}
	      break;
	    }
	}
      assert (name);

      break;

    case user:

      /* Write the UF_BLOCK structure. */

      if (!WRITE_INT (&((UF_BLOCK *) f->funct_p)->cargs, stream) ||
	  !WRITE_INT (&((UF_BLOCK *) f->funct_p)->targs, stream))
	goto err;

      /* Write Algae code. */

      if (!put_code (((UF_BLOCK *) f->funct_p)->code, stream, ent_tree))
	{
	  WRITE_WARN (stream);
	  goto err;
	}

      break;

    default:

      BAD_ILK (f->ilk);
      delete_function (f);
      raise_exception ();

    }

  delete_function (f);
  return 1;

err:
  delete_function (f);
  return 0;
}

ENTITY *
get_function (FILE *stream, int ver)
{
  /* Read a function in binary form from `stream'. */

  FUNCTION *f;
  int i, size;
  char *name;
  struct bi_init *b = builtin_init_data;
  ENTITY *(*bi_ptr) () = NULL;
  static char *warn_msg = "Invalid function in file.";

  /* Read the FUNCTION structure itself. */

  f = (FUNCTION *) CALLOC (1, sizeof (FUNCTION));
  f->entity.ref_count = 1;
  f->entity.class = function;

  if (!READ_INT (&f->ilk, stream))
    {
      FREE (f);
      return NULL;
    }

  /* Read additional members, if they exist. */

  if (!READ_INT (&i, stream) ||
      i && !(f->stuff = (TABLE *)
	     (ver ? get_entity (stream) : get_table (stream, ver))))
    {
      FREE (f);
      return NULL;
    }

  /* Different handling for builtins and user functions. */

  switch (f->ilk)
    {

    case builtin:

      /* Read the BUILTIN structure. */

      f->funct_p = MALLOC (sizeof (BUILTIN));

      if (!READ_INT (&((BUILTIN *) f->funct_p)->min_args, stream) ||
	  !READ_INT (&((BUILTIN *) f->funct_p)->max_args, stream) ||
	  !READ_INT (&((BUILTIN *) f->funct_p)->arg_flags, stream))
	{
	  FREE (f->funct_p);
	  delete_table (f->stuff);
	  FREE (f);
	  return NULL;
	}

      /* Read function name. */

      if (!READ_INT (&size, stream))
	{
	  FREE (f->funct_p);
	  delete_table (f->stuff);
	  FREE (f);
	  return NULL;
	}

      if (size < 1)
	{
	  warn (warn_msg);
	  FREE (f->funct_p);
	  delete_table (f->stuff);
	  FREE (f);
	  return NULL;
	}

      name = MALLOC (size + 1);
      if (fread (name, 1, size, stream) < (size_t) size)
	{
	  READ_WARN (stream);
	  FREE (f->funct_p);
	  delete_table (f->stuff);
	  FREE (f);
	  FREE (name);
	  return NULL;
	}
      name[size] = '\0';

      /* Look up pointer to function. */

      while ((bi_ptr = b->bi.bi) != NULL)
	{
	  if (!strcmp ((b++)->name, name))
	    {
	      ((BUILTIN *) f->funct_p)->bi = bi_ptr;
	      break;
	    }
	}

      FREE (name);

      if (!bi_ptr)
	{
	  warn ("File references unknown builtin function.");
	  FREE (f->funct_p);
	  delete_table (f->stuff);
	  FREE (f);
	  return NULL;
	}

      if (!ok_entity (ENT (f)))
	{
	  FREE (f->funct_p);
	  delete_table (f->stuff);
	  FREE (f);
	  return NULL;
	}

      break;

    case user:

      /* Read the UF_BLOCK structure. */

      f->funct_p = MALLOC (sizeof (UF_BLOCK));

      if (!READ_INT (&((UF_BLOCK *) f->funct_p)->cargs, stream) ||
	  !READ_INT (&((UF_BLOCK *) f->funct_p)->targs, stream))
	{
	  FREE (f->funct_p);
	  delete_table (f->stuff);
	  FREE (f);
	  return NULL;
	}

      /* Read the Algae code. */

      if (!(((UF_BLOCK *) f->funct_p)->code = get_code (stream, f)))
	{
	  FREE (f->funct_p);
	  delete_table (f->stuff);
	  FREE (f);
	  return NULL;
	}

      if (!ok_entity (ENT (f)))
	{
	  FREE (f->funct_p);
	  delete_table (f->stuff);
	  FREE (f);
	  return NULL;
	}

      break;

    default:

      BAD_ILK (f->ilk);
      delete_table (f->stuff);
      FREE (f);
      raise_exception ();

    }

  return ENT (f);

}

int
put_code (INST *start, FILE *stream, struct ent_node *ent_tree)
{
  register INST *cp = start;

  /* Write user code to a binary file. */

  int i;
  char *s;

  /* Write code size (in bytes). */

  /*
   * Note that its size when written may be different than its
   * size in memory.  The file consists of 4-byte integers, but
   * an INST may have a different size.
   */

  i = code_size (cp) / sizeof (INST) * 4;

  assert (i > 0);
  if (!WRITE_INT (&i, stream))
    return 0;

  while (1)
    {

      POLL_SIGINT ();

      switch (cp->op)
	{

	case OP_HALT:
	  if (!WRITE_INT (&(cp++->op), stream))
	    return 0;
	  return 1;

	case OP_PUSHS:		/* string follows */
	case OP_PUSHS2E:
	case OP_FILE:
	  if (!WRITE_INT (&(cp++->op), stream))
	    return 0;
	  i = strlen (cp->ptr);
	  if (!WRITE_INT (&i, stream))
	    return 0;
	  if (i > 0 && fwrite (cp->ptr, 1, i, stream) < (size_t) i)
	    {
	      WRITE_WARN (stream);
	      return 0;
	    }
	  cp++;
	  break;

	case OP_PUSHC:		/* datum follows */

	  /*
	   * If the datum we're about to push is a D_ENTITY, then not
	   * only does it have to be a function but it also must be a
	   * "self" reference, since we're already in a function.
	   * If so, do an OP_PUSHI followed by the string "self".  Since
	   * `get_code' knows that the parser would never allow that
	   * ("self" is a keyword), it'll know what to do.
	   */

	  if (((DATUM *) cp[1].ptr)->type == D_ENTITY)
	    {
#if DEBUG
	      {
		FUNCTION *f = (FUNCTION *) E_PTR ((DATUM *) cp[1].ptr);
		assert (f->entity.class == function);
		assert (f->ilk == user);
		assert (((UF_BLOCK *) f->funct_p)->code == start);
	      }
#endif
	      i = OP_PUSHI;
	      if (!WRITE_INT (&i, stream))
		return 0;
	      s = "self";
	      i = strlen (s);
	      if (!WRITE_INT (&i, stream))
		return 0;
	      if (fwrite (s, 1, i, stream) < (size_t) i)
		{
		  WRITE_WARN (stream);
		  return 0;
		}
	      cp += 2;

	    }
	  else
	    {

	      if (!WRITE_INT (&(cp++->op), stream) ||
		  !put_datum (cp++->ptr, stream, ent_tree))
		return 0;

	    }
	  break;

	case OP_PUSHA:		/* address follows */
	case OP_PUSHI:
	  if (!WRITE_INT (&(cp++->op), stream))
	    return 0;
	  s = address_to_id (cp++->ptr);
	  i = strlen (s);
	  if (!WRITE_INT (&i, stream))
	    return 0;
	  if (i > 0 && fwrite (s, 1, i, stream) < (size_t) i)
	    {
	      WRITE_WARN (stream);
	      return 0;
	    }
	  break;

	case OP_LPUSHI:	/* integer follows */
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
	case OP_VEIL:
	  if (!WRITE_INT (&(cp++->op), stream))
	    return 0;
	  i = cp++->op;
	  if (!WRITE_INT (&i, stream))
	    return 0;
	  break;

	default:
	  if (!WRITE_INT (&(cp++->op), stream))
	    return 0;
	  if (cp[-1].op < 0 || cp[-1].op >= NUM_OP_CODES)
	    wipeout ("Unknown opcode 0x%x.\n", cp[-1].op);

	}
    }
}

INST *
get_code (FILE *stream, FUNCTION *parent_function)
{
  /* Read user code from a binary file. */

  INST *cp, *start;
  int i, size;
  char *s;
  static char *warn_msg = "Invalid user code in file.";

  /* read code size */

  if (!READ_INT (&size, stream))
    return NULL;

  if (size < 1)
    {
      warn (warn_msg);
      return NULL;
    }
  size /= 4;			/* convert to units of INST's */
  start = cp = MALLOC (size * sizeof (INST));

  while (cp - start < size)
    {

      POLL_SIGINT ();

      /* read op */

      if (!READ_INT (&cp->op, stream))
	{
	  FREE (start);
	  return NULL;
	}

#ifdef apollo
      {
	int op = cp++->op;	/* avoid a bug on old apollos */
	switch (op)
#else
      switch (cp++->op)
#endif
	{

	case OP_HALT:
	  if (cp - start != size)
	    {
	      warn (warn_msg);
	      FREE (start);
	      return NULL;
	    }
	  return start;

	case OP_PUSHS:		/* string follows */
	case OP_PUSHS2E:
	case OP_FILE:

	  /* string length */

	  if (!READ_INT (&i, stream))
	    {
	      FREE (start);
	      return NULL;
	    }

	  /* read string */

	  if (i > 0)
	    {
	      cp->ptr = MALLOC (i + 1);
	      if (fread (cp->ptr, 1, i, stream) < (size_t) i)
		{
		  READ_WARN (stream);
		  FREE (start);
		  return NULL;
		}
	      ((char *) cp->ptr)[i] = '\0';
	    }
	  else
	    {
	      cp->ptr = NULL_string;
	    }
	  cp++;
	  break;

	case OP_PUSHC:		/* datum follows */
	  if (!(cp++->ptr = get_datum (stream)))
	    {
	      FREE (start);
	      return NULL;
	    }
	  break;

	case OP_PUSHA:		/* address follows */
	case OP_PUSHI:

	  /* length of name */

	  if (!READ_INT (&i, stream))
	    {
	      FREE (start);
	      return NULL;
	    }
	  if (i < 1)
	    {
	      warn (warn_msg);
	      FREE (start);
	      return NULL;
	    }
	  else
	    {

	      /* read the name */

	      SYMTAB *stp;
	      s = MALLOC (i + 1);
	      if (fread (s, 1, i, stream) < (size_t) i)
		{
		  READ_WARN (stream);
		  FREE (s);
		  FREE (start);
		  return NULL;
		}
	      s[i] = '\0';

	      /* If the name is "self", then we have a self reference. */

	      if (!strcmp (s, "self"))
		{

		  cp[-1].op = OP_PUSHC;
		  cp->ptr = new_DATUM ();
		  ((DATUM *) cp->ptr)->data.ptr = parent_function;
		  ((DATUM *) cp->ptr)->type = D_ENTITY;
		  cp++;

		}
	      else
		{

		  /* find (or create) the address */

		  stp = find (s, CREATE);	/* swallows s */
		  if (stp->scope == NO_SCOPE)
		    {
		      stp->scope = 0;
		      stp->stval.datum = new_DATUM ();
		    }
		  cp++->ptr = stp->stval.datum;

		}
	    }
	  break;

	case OP_LPUSHI:	/* integer follows */
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
	  if (!READ_INT (&(cp++->op), stream))
	    {
	      FREE (start);
	      return NULL;
	    }
	  break;

	default:
	  if (cp[-1].op < 0 || cp[-1].op >= NUM_OP_CODES)
	    {
	      warn (warn_msg);
	      FREE (start);
	      return NULL;
	    }

	}
#ifdef apollo
    }
#endif
}

warn (warn_msg);
FREE (start);
return NULL;
}

ENTITY *
bi_builtin (ENTITY *obj, ENTITY *sym)
{
  /* Create a new builtin function from a shared object. */

#if HAVE_DLOPEN

  void *handle;
  ENTITY *(*fptr) ();
  BUILTIN *p;

  if (!(handle = dlopen (entity_to_string (obj), RTLD_NOW)))
    {
      fail ("Can't open shared object:\n%s\n", dlerror ());
      delete_entity (sym);
      raise_exception ();
    }

  if (!(fptr = (ENTITY * (*)())dlsym (handle, entity_to_string (sym))))
    {
      fail ("Can't obtain symbol:\n%s\n", dlerror ());
      raise_exception ();
    }

  p = MALLOC (sizeof (BUILTIN));
  p->min_args = 0;
  p->max_args = 5;	/* more than 5, and we'd have to pass the arg array */
  p->arg_flags = ~0;	/* any or all args may be zero */
  p->bi = fptr;

  return ENT (make_function (builtin, p));

#else

  fail ("Not configured to use shared objects.");
  raise_exception ();

#endif

}
