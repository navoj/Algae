/*
   code.c -- Utility routines for code generation.

   Copyright (C) 1994-2002  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: code.c,v 1.5 2003/08/07 02:34:09 ksh Exp $";

#include "algae.h"
#include "entity.h"
#include "ptypes.h"
#include "code.h"
#include "mem.h"

static void PROTO (jmp_error_clear, (void));
static void PROTO (bc_error_clear, (void));
static void PROTO (enlarge_code_block, (void));


CODE_BLOCK the_code_block;

/* this is now a function so it can check for possible code
   overflow */
void
code2 (int op, void *ptr)
{
  if (code_index > code_danger)
    enlarge_code_block ();
  INST_PTR (code_index)->op = op;
  INST_PTR (code_index + 1)->ptr = ptr;
  code_index += 2;
}

void
icode2 (int op, int op2)
{
  if (code_index > code_danger)
    enlarge_code_block ();
  INST_PTR (code_index)->op = op;
  INST_PTR (code_index + 1)->op = op2;
  code_index += 2;
}


/* code for += */

void
code_dup_id (int index, int op)
{
  int cnt = code_index - index;

  (void) memcpy (INST_PTR (code_index), INST_PTR (index),
		 cnt * sizeof (INST));

  op = code_index + op - index;

  if (INST_PTR (op)->op == OP_PUSHA)
    INST_PTR (op)->op = OP_PUSHI;
  else if (INST_PTR (op)->op == OP_LPUSHA)
    INST_PTR (op)->op = OP_LPUSHI;
  else
    {
      assert (INST_PTR (op)->op == OP_PUSHA_SYM);
      INST_PTR (op)->op = OP_PUSHI_SYM;
    }

  code_index += cnt;
}

void
code_id_address (SYMTAB *p)
{
  if (code_index > code_danger)
    enlarge_code_block ();

  if (p->scope == 0)
    {
      INST_PTR (code_index)->op = OP_PUSHA;
      INST_PTR (code_index + 1)->ptr = p->stval.datum;
    }
  else if (p->scope == 1)
    {
      INST_PTR (code_index)->op = OP_LPUSHA;
      INST_PTR (code_index + 1)->op = p->stval.offset;
    }
  else
    {
      detour ("scoped functions later");
      raise_exception ();
    }
  code_index += 2;
}

void
code_veil_address (SYMTAB *p)
{
  if (code_index > code_danger)
    enlarge_code_block ();

  INST_PTR (code_index)->op = OP_VEIL;
  INST_PTR (code_index + 1)->ptr = p;
  code_index += 2;
}

/*--------- back patching of jumps -------------------*/

/*
   jumps that need to be back patched are maintained as a
   stack on linked lists of these
 */

typedef struct jmp
{
  struct jmp *link;
  OFFSET source;		/* where to patch */
}
JMP;

static JMP *jmp_top;

/*
   code a jump. If target is -1, it goes on the 
   jump stack to be back patched later
 */

void
code_jmp (int jtype, OFFSET target)
{
  INST *cp = code_base + code_index;

  code_index += 2;

  cp->op = jtype;

  if (target != -1)		/* jmp can be coded now */
    {
      cp[1].op = target - code_index + 1;
    }
  else
    /* jmp will need to be back patched */
    {
      JMP *p = (JMP *) MALLOC (sizeof (JMP));

      p->source = code_index - 1;
      p->link = jmp_top;
      jmp_top = p;
    }
}


void
patch_jmp (OFFSET target)
{
  JMP *p = jmp_top;

  assert (jmp_top);		/*stack is not empty */

  /* source of jmp is on the jump stack */
  jmp_top = p->link;
  INST_PTR (p->source)->op = target - p->source;
  FREE (p);
}


/* on error, jmp stack needs to be cleared */

static void
jmp_error_clear (void)
{
  JMP *p = jmp_top;
  JMP *q;

  jmp_top = (JMP *) 0;
  while (p)
    {
      q = p;
      p = p->link;
      FREE (q);
    }
}

/* Start a new jump stack, for a new parser invocation. */

void *
jmp_top_push (void)
{
  JMP *p = jmp_top;

  jmp_top = (JMP *) 0;
  return (void *) p;
}

/* Restore the jump stack. */

void
jmp_top_pop (void *p)
{
  jmp_top = (JMP *) p;
}

/*------- break and continues ----------------*/

typedef struct bc
{
  struct bc *link;
  int type;			/* 'B' or 'C' or 0 */
  OFFSET where;			/* source */
}
BC;

static BC *bc_top;

void
BC_new (void)
{				/* this marks the start of a new loop */
  (void) BC_insert (0, 0);
}

/*
   returns true, if we are in fact in a loop
 */

int
BC_insert (int type, OFFSET where)
{
  if (type == 0 || bc_top)
    {
      BC *p = (BC *) MALLOC (sizeof (BC));
      p->type = type;
      p->where = where;
      p->link = bc_top;
      bc_top = p;
      return 1;
    }
  return 0;
}


/*
   We've seen the end of a loop.
   Patch all break/continues
 */
void
BC_clear (OFFSET b_off, OFFSET c_off)
{
  BC *p, *q;
  OFFSET off;

  assert (bc_top);

  p = bc_top;

  while (p->type)
    {
      off = p->type == 'B' ? b_off : c_off;
      INST_PTR (p->where)->op = off - p->where;

      q = p;
      p = p->link;
      FREE (q);
    }
  /* remove the mark node */
  bc_top = p->link;
  FREE (p);
}


static void
bc_error_clear (void)
{
  BC *p, *q;

  p = bc_top;
  bc_top = (BC *) 0;

  while (p)
    {
      q = p;
      p = p->link;
      FREE (q);
    }
}

/* Start a new jump stack, for a new parser invocation. */

void *
bc_top_push (void)
{
  BC *p = bc_top;

  bc_top = (BC *) 0;
  return (void *) p;
}

/* Restore the jump stack. */

void
bc_top_pop (void *p)
{
  bc_top = (BC *) p;
}


/*----------------------------------------------------*/


/* reset after error ; if flag is on this is a 
   debug check and both stacks should be empty */

void
reset_jmp (int flag)
{

  if (flag)
    {
      assert (!jmp_top);
      assert (!bc_top);
    }
  else
    {
      jmp_error_clear ();
      bc_error_clear ();
    }
}

/* ------- malloc() and movement of code ------------ */


void
new_code_block (void)
{
  code_base = MALLOC (CODE_SIZE * sizeof (INST));
  code_index = 0;
  code_limit = CODE_SIZE;
  code_danger = code_limit - CODE_SAFETY;
}


static void
enlarge_code_block (void)
{
  if (code_index >= code_limit)
    wipeout ("code segment overflow");

  code_limit += CODE_SIZE;
  code_danger = code_limit - CODE_SAFETY;
  code_base = REALLOC (code_base, code_limit * sizeof (INST));
}


void
shrink_code_block (void)
{
  code_danger = code_limit = code_index;
  code_base = REALLOC (code_base, code_index * sizeof (INST));
}
