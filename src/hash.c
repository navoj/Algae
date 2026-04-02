/*
   hash.c -- Symbol table functions.

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: hash.c,v 1.2 1997/02/21 09:53:57 ksh Exp $";

#include "algae.h"
#include "psr.h"
#include "code.h"
#include "mem.h"
#include "function.h"
#include "table.h"
#include "datum.h"


static void PROTO (restore_ids, (void));


unsigned
hash (char *s)
{
  register unsigned char *u = (unsigned char *) s;
  register unsigned h = 0;

  while (*u)
    h += h + *u++;
  return h;
}


#define  HASH_PRIME    149

typedef struct hash
{
  struct hash *link;
  SYMTAB symtab;
}
HASHNODE;

static HASHNODE *hash_table[HASH_PRIME];

/* insert --
   puts a name in the symbol table that is
   guaranteed not to already be in the table.
   Also name need not be duplicated.
   Used for initialization.
 */

SYMTAB *
insert (char *name)
{
  register HASHNODE *p;
  unsigned h;

  h = hash (name) % HASH_PRIME;
  p = MALLOC (sizeof (HASHNODE));

  p->symtab.name = name;
  p->symtab.scope = 0;
  p->link = hash_table[h];
  hash_table[h] = p;
  return &p->symtab;
}

ENTITY *
symbols_to_table (void)
{
  HASHNODE **p = hash_table;
  HASHNODE *q;
  DATUM d;
  ENTITY *e;
  TABLE *t = (TABLE *) make_table ();

  while (p < hash_table + HASH_PRIME)
    {
      q = *p++;
      while (q)
	{
	  if (q->symtab.scope == 0)
	    {
	      e = (!q->symtab.stval.datum ||
		   cast1_to_entity (copy_datum (&d, q->symtab.stval.datum))
		   == D_NULL) ?
		NULL : copy_entity (E_PTR (&d));
	      t = (TABLE *) replace_in_table (t, e, dup_char (q->symtab.name));
	      delete_datum (&d);
	    }
	  q = q->link;
	}
    }

  return (ENT (t));
}

/*
   find --
   searches for name in the symbol table.

   If name is not used and CREATE, then name is freed

   Creates an entry if create_flag is on.
 */

SYMTAB *
find (char *name, int create_flag)
{
  register HASHNODE *p;		/*searchs for name */
  HASHNODE *q = (HASHNODE *) 0;	/* trails p */
  unsigned h;

  p = hash_table[h = hash (name) % HASH_PRIME];

  while (p)
    if (strcmp (p->symtab.name, name) == 0)	/* found */
      {
	if (create_flag)
	  FREE (name);
	if (!q)
	  return &p->symtab;
	else
	  {
	    q->link = p->link;
	    goto found;
	  }
      }
    else
      {
	q = p;
	p = p->link;
      }

  if (!create_flag)
    {
      return (SYMTAB *) 0;
    }
  /* not in the table make a new node */
  p = MALLOC (sizeof (HASHNODE));
  p->symtab.name = name;
  p->symtab.scope = NO_SCOPE;
  p->symtab.stval.datum = NULL;

found:
  p->link = hash_table[h];
  hash_table[h] = p;

  return &p->symtab;
}

/*
   This is only used by the disassembler
 */

char *
address_to_id (DATUM *dp)
{
  register HASHNODE *p;
  register int i;

  for (i = 0; i < HASH_PRIME; i++)
    {
      p = hash_table[i];
      while (p)
	if (p->symtab.stval.datum == dp)
	  return p->symtab.name;
	else
	  p = p->link;
    }

  wipeout ("address_to_id");
  /* phoney */
  return (char *) 0;
}


typedef struct scope
{
  struct scope *link;		/* links as stack */
  UF_BLOCK *ufp;		/* saved active_function */
  CODE_BLOCK code_block;	/*saved code */
}
SCOPE;


int scope_level;
static SCOPE *scope_stack;
static HASHNODE *save_list;


/* the parser calls this function, when it sees the start of
   a function definition to change scope and stack
   scope state
 */

void
push_scope (UF_BLOCK *ufp)
{
  SCOPE *p = MALLOC (sizeof (SCOPE));
  HASHNODE *q = MALLOC (sizeof (HASHNODE));

  p->code_block = the_code_block;
  new_code_block ();

  p->ufp = ufp;
  p->link = scope_stack;
  scope_stack = p;
  scope_level++;

  /* mark the save list */
  q->symtab.name = (char *) 0;
  q->link = save_list;
  save_list = q;
}


UF_BLOCK *
pop_scope (void)
{
  SCOPE *p = scope_stack;
  UF_BLOCK *retval = p->ufp;

  the_code_block = p->code_block;

  scope_stack = p->link;
  scope_level--;
  restore_ids ();

  FREE (p);
  return retval;
}

void
cleanup_scope_stack (void)
{
  /* this needs to be done after an exception */
  UF_BLOCK *ufp;

  while (scope_stack)
    {
      ufp = pop_scope ();
      if (ufp)
	{
	  FREE (ufp->code);
	  FREE (ufp);
	}
    }
  scope_level = 0;
}

/* a local name has hidden a name with lower scope.
   The name is saved on the save_list */

void
save_id (SYMTAB *p)
{
  HASHNODE *pp = MALLOC (sizeof (HASHNODE));

  pp->symtab = *p;
  pp->link = save_list;
  save_list = pp;
}


/* restore one-level of names in the symbol table */
static void
restore_ids (void)
{
  HASHNODE *p = save_list, *hold;
  SYMTAB *q;

  /* pop down to the marker */
  while (p->symtab.name)
    {
      q = find (p->symtab.name, NO_CREATE);
      assert (q);
      *q = p->symtab;

      hold = p;
      p = p->link;
      FREE (hold);
    }
  /* remove the marker */
  save_list = p->link;
  FREE (p);
}
