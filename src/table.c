/*
   table.c -- Tables

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: table.c,v 1.5 2003/08/01 04:57:48 ksh Exp $";

#include "table.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "get.h"
#include "put.h"
#include "file_io.h"
#include "print.h"
#include "vargs.h"
#include "binop.h"
#include "unop.h"
#include "printf.h"
#include "datum.h"

extern DATUM *term_width;
#define TERM_WIDTH datum_to_int( copy_datum( term_width, term_width ) )

static int PROTO (accept_members_node, (TABLE_NODE * tn, char **allowable_names));
static void PROTO (free_table_node, (TABLE_NODE * tn));
static TABLE_NODE *PROTO (dup_table_node, (TABLE_NODE * old_tn));
static int PROTO (put_table_node, (TABLE_NODE * tn, FILE * stream, struct ent_node *));
static TABLE_NODE *PROTO (get_table_node, (FILE * stream));

static ENTITY *PROTO (add_table_node, (TABLE * l, TABLE_NODE * tn));
static ENTITY *PROTO (subtract_table_node, (TABLE * l, TABLE_NODE * tn));
static void PROTO (lowercase_table_node, (TABLE_NODE * tn));
static char *PROTO (first_in_table_node, (TABLE_NODE * tn));
static int PROTO (visit_table_node, (TABLE_NODE * tn, PFI f, va_list arg));

/* The VMS compiler can't handle this structure initialization, so */
/* we have to initialize in create_table.  Not only that, it thinks */
/* that "static" means "automatic" for external variables and so   */
/* it repeatedly initializes them (which is wrong, wrong, wrong!). */

#ifndef VAXC
static TABLE_NODE table_tail =
{NULL_string, &table_tail, &table_tail, NULL};
#else
TABLE_NODE table_tail;
#endif

static char *msg_NULL_op = "%s operation on NULL.";

int
accept_members (TABLE *t, char **allowable_names)
{
  /*
   * This function checks table `t' to make sure that its members have
   * names that are in the `allowable_names' list.  It returns 0 iff
   * a member of `t' is disallowed.  The `allowable_names' array is
   * terminated by a NULL.
   */

  int r;

  EASSERT (t, table, 0);

  r = accept_members_node (t->table_node, allowable_names);
  delete_table (t);
  return (r);
}

static int
accept_members_node (TABLE_NODE *tn, char **allowable_names)
{
  char *c;

  if (tn != &table_tail)
    {
      if (!accept_members_node (tn->left, allowable_names) ||
	  !accept_members_node (tn->right, allowable_names))
	return (0);
      while (c = *allowable_names++)
	if (!strcmp (tn->name, c))
	  return (1);
      fail ("Invalid option:  \"%s\".", tn->name);
      return (0);
    }
  return (1);
}

ENTITY *
search_in_table (TABLE *head, char *name)
{
  /*
   * Looks for a member called `name' in table `head'.  Returns it
   * if found, otherwise returns NULL.
   */

  ENTITY *p;
  TABLE_NODE *node;
  int i;

  EASSERT (head, table, 0);
  assert (name != NULL);

  node = head->table_node;
  table_tail.name = name;

  while ((i = strcmp (name, node->name)) != 0)
    {
      node = (i < 0) ? node->left : node->right;
    }
  p = (node == &table_tail || node->entity == NULL) ? NULL : copy_entity (node->entity);
  delete_table (head);
  FREE_CHAR (name);
  return (p);
}

int
exists_in_table (TABLE *head, char *name)
{
  /* Returns 0 iff table `head' has no member called `name'. */

  TABLE_NODE *node;
  int i;

  EASSERT (head, table, 0);
  assert (name != NULL);

  node = head->table_node;
  table_tail.name = name;

  while ((i = strcmp (name, node->name)) != 0)
    {
      node = (i < 0) ? node->left : node->right;
    }
  delete_table (head);
  return (node != &table_tail);
}

ENTITY *
make_table (void)
{
  /* Make an empty table. */

  TABLE *t;

#ifdef VAXC
  table_tail.name = NULL_string;
  table_tail.left = &table_tail;
  table_tail.right = &table_tail;
  table_tail.entity = NULL;
#endif

  t = (TABLE *) CALLOC (1, sizeof (TABLE));

  t->entity.ref_count = 1;
  t->entity.class = table;
  t->nm = 0;
  t->table_node = &table_tail;

  if (debug_level > 1)
    inform ("Table created:  %x.", t);

  return (ENT (t));
}

void
free_table (TABLE *head)
{
  /*
   * We assume that we've already gone through `delete'.
   */

  assert (head->entity.ref_count == 0);

  free_table_node (head->table_node);
  FREE (head);
}

static void
free_table_node (TABLE_NODE *tn)
{
  if (tn != &table_tail)
    {
      FREE_CHAR (tn->name);
      free_table_node (tn->left);
      free_table_node (tn->right);
      delete_entity (tn->entity);
      FREE (tn);
    }
}

void
print_table (TABLE *head, FILE *stream)
{
  /* Print the names of each member of a table. */

  int i, j, len, maxlen, nr, nc;
  char **names;

  EASSERT (head, table, 0);

  if (head->nm > 0)
    {

      /* Get a list of the names and suffices. */

      names = E_MALLOC (head->nm, character);
      get_table_names (head->table_node, names);

      /* Find the length of the longest one. */

      maxlen = 0;
      for (i = 0; i < head->nm; i++)
	{
	  if ((len = strlen (names[i])) > maxlen)
	    maxlen = len;
	}

      /*
       * Decide how many columns to use.  Each line begins with a tab
       * (assumed to be 8 characters), and each name is printed in a
       * field of `maxlen' characters.  The fields are separated by
       * two blanks.
       */

      nc = (TERM_WIDTH - 8) / (maxlen + 2);	/* number of columns on page */
      if (nc <= 0)
	nc = 1;
      nr = (head->nm - 1) / nc + 1;	/* number of lines */

      /* Now print them. */

      for (i = 0; i < nr; i++)
	{
	  xputc ('\t', stream);
	  for (j = i; j < head->nm; j += nr)
	    {
	      xfprintf (stream, "%-*s", maxlen + 2, names[j]);
	      FREE_CHAR (names[j]);
	    }
	  xputc ('\n', stream);
	}

      FREE (names);
    }

  delete_table (head);
}

char **
get_table_names (TABLE_NODE *tn, char **names)
{
  /*
   * This function adds `tn' and its children's names to the `names'
   * array.  It returns a pointer to the next spot in `names'.
   */

  if (tn != &table_tail)
    {
      names = get_table_names (tn->left, names);
      *(names++) = dup_char (tn->name);
      names = get_table_names (tn->right, names);
    }

  return (names);
}

ENTITY *
dup_table (TABLE *old)
{
  /*
   * In Algae, duplicating an entity means that we really make another
   * copy of it in memory (not just incrementing its reference count).
   */

  TABLE *new;

  EASSERT (old, table, 0);

  if (old->entity.ref_count == 1)
    return (ENT (old));

  new = (TABLE *) make_table ();
  new->nm = old->nm;
  new->table_node = dup_table_node (old->table_node);

  delete_table (old);
  return (ENT (new));
}

static TABLE_NODE *
dup_table_node (TABLE_NODE *old_tn)
{
  TABLE_NODE *new_tn;

  if (old_tn != &table_tail)
    {
      new_tn = (TABLE_NODE *) MALLOC (sizeof (TABLE_NODE));
      new_tn->name = dup_char (old_tn->name);
      new_tn->left = dup_table_node (old_tn->left);
      new_tn->right = dup_table_node (old_tn->right);
      new_tn->entity = (old_tn->entity == NULL) ? NULL : copy_entity (old_tn->entity);
    }
  else
    {
      new_tn = old_tn;
    }

  return (new_tn);
}

ENTITY *
delete_from_table (TABLE *head, char *name)
{
  /* Removes member called `name' from table `head'. */

  int i;
  TABLE_NODE *parent = NULL;
  TABLE_NODE *child, *target, *node;

  EASSERT (head, table, 0);

  head = (TABLE *) dup_table (head);
  node = head->table_node;
  table_tail.name = name;

  while ((i = strcmp (name, node->name)) != 0)
    {
      parent = node;
      node = (i < 0) ? parent->left : parent->right;
    }

  target = node;
  if (target->right == &table_tail)
    {
      node = target->left;
    }
  else if (target->right->left == &table_tail)
    {
      node = target->right;
      node->left = target->left;
    }
  else
    {
      child = target->right;
      while (child->left->left != &table_tail)
	{
	  child = child->left;
	}
      node = child->left;
      child->left = node->right;
      node->left = target->left;
      node->right = target->right;
    }

  if (parent == NULL)
    {
      head->table_node = node;
    }
  else
    {
      if (strcmp (name, parent->name) < 0)
	{
	  parent->left = node;
	}
      else
	{
	  parent->right = node;
	}
    }

  if (target != &table_tail)
    {
      delete_entity (target->entity);
      head->nm--;
      FREE (target);
    }

  FREE_CHAR (name);
  return (ENT (head));
}

ENTITY *
replace_in_table (TABLE *head, ENTITY *new, char *name)
{
  /*
   * This function adds the entity `new' to the table `head' and
   * gives it the name `name'.  All three arguments are deleted.
   * If there's already a member called `name', then it's replaced.
   */

  TABLE_NODE *parent = NULL;
  TABLE_NODE *node;
  int i;

  EASSERT (head, table, 0);

  head = (TABLE *) dup_table (head);
  node = head->table_node;
  table_tail.name = name;

  while ((i = strcmp (name, node->name)) != 0)
    {
      parent = node;
      node = (i < 0) ? parent->left : parent->right;
    }

  if (node == &table_tail)
    {
      node = (TABLE_NODE *) MALLOC (sizeof (TABLE_NODE));
      node->name = name;
      node->left = &table_tail;
      node->right = &table_tail;
      if (parent == NULL)
	{
	  head->table_node = node;
	}
      else
	{
	  if (strcmp (node->name, parent->name) < 0)
	    {
	      parent->left = node;
	    }
	  else
	    {
	      parent->right = node;
	    }
	}
      head->nm++;
    }
  else
    {
      delete_entity (node->entity);
      FREE_CHAR (name);
    }

  node->entity = new;

  return (ENT (head));
}

void
DB_delete_table (TABLE *p, char *file, int line)
{
  /*
   * This is the DEBUG version of `delete_table'.  It decrements
   * the table's reference count, and frees it if it's unreferenced.
   */

  if (p)
    {

      if (--p->entity.ref_count < 0)
	{
	  wipeout ("A table's \"ref_count\" went below zero:  %s, %d.",
		   file, line);
	}

      if (p->entity.ref_count >= 1000 || debug_level > 1)
	{
	  inform ("\"ref_count\" decrement for table:  %x, %d, %s, %d.",
		  p, p->entity.ref_count, file, line);
	}

      if (p->entity.ref_count == 0)
	free_table (p);
    }
}

int
put_table (TABLE *t, FILE *stream, struct ent_node *ent_tree)
{
  /* Write table `t' out in binary form to file `stream'. */

  int i;

  EASSERT (t, table, 0);

  if (!WRITE_INT (&t->nm, stream))
    goto err;

  i = t->table_node != &table_tail;
  if (!WRITE_INT (&i, stream))
    goto err;

  if (i && !put_table_node (t->table_node, stream, ent_tree))
    {
      WRITE_WARN (stream);
      goto err;
    }

  delete_table (t);
  return 1;

err:
  delete_table (t);
  return 0;
}

static int
put_table_node (TABLE_NODE *tn, FILE *stream, struct ent_node *ent_tree)
{
  int size, i;

  assert (tn != &table_tail);

  size = strlen (tn->name);
  if (!WRITE_INT (&size, stream))
    return 0;
  if (size > 0 &&
      fwrite (tn->name, 1, size, stream) < (size_t) size)
    {
      WRITE_WARN (stream);
      return 0;
    }

  i = tn->entity != NULL;
  if (!WRITE_INT (&i, stream) ||
      i && !put_entity (copy_entity (tn->entity), stream, ent_tree))
    return 0;

  i = tn->left != &table_tail;
  if (!WRITE_INT (&i, stream) ||
      i && !put_table_node (tn->left, stream, ent_tree))
    return 0;

  i = tn->right != &table_tail;
  if (!WRITE_INT (&i, stream) ||
      i && !put_table_node (tn->right, stream, ent_tree))
    return 0;

  return 1;
}

ENTITY *
get_table (FILE *stream, int ver)
{
  /* Read a table from binary file `stream'. */

  TABLE *t;
  int i;

  t = (TABLE *) CALLOC (1, sizeof (TABLE));
  t->entity.ref_count = 1;
  t->entity.class = table;

  if (!READ_INT (&t->nm, stream) || !READ_INT (&i, stream))
    goto err;

  if (t->nm)
    {
      if (!(t->table_node = get_table_node (stream)))
	goto err;
    }
  else
    {
      t->table_node = &table_tail;
    }

  return ENT (t);

err:

  FREE (t);
  return NULL;
}

static TABLE_NODE *
get_table_node (FILE *stream)
{
  TABLE_NODE *tn;
  int size, i;

  tn = (TABLE_NODE *) CALLOC (1, sizeof (TABLE_NODE));
  tn->left = tn->right = &table_tail;

  if (!READ_INT (&size, stream))
    {
      FREE (tn);
      return NULL;
    }
  if (size > 0)
    {
      tn->name = MALLOC (size + 1);
      if (fread (tn->name, 1, size, stream) < (size_t) size)
	{
	  READ_WARN (stream);
	  FREE (tn->name);
	  FREE (tn);
	  return NULL;
	}
      tn->name[size] = '\0';
    }
  else
    {
      tn->name = NULL_string;
    }

  if (!READ_INT (&i, stream) ||
      i && !(tn->entity = get_entity (stream)))
    {
      FREE_CHAR (tn->name);
      FREE (tn);
      return NULL;
    }

  if (!READ_INT (&i, stream) ||
      i && !(tn->left = get_table_node (stream)))
    {
      delete_entity (tn->entity);
      FREE_CHAR (tn->name);
      FREE (tn);
      return NULL;
    }

  if (!READ_INT (&i, stream) ||
      i && !(tn->right = get_table_node (stream)))
    {
      delete_entity (tn->entity);
      free_table_node (tn->left);
      FREE_CHAR (tn->name);
      FREE (tn);
      return NULL;
    }

  return tn;
}

ENTITY *
add_table (TABLE *l, TABLE *r)
{
  EASSERT (l, table, 0);
  EASSERT (r, table, 0);

  l = (TABLE *) add_table_node (l, r->table_node);
  delete_table (r);
  return (ENT (l));
}

static ENTITY *
add_table_node (TABLE *l, TABLE_NODE *tn)
{
  if (tn != &table_tail)
    {
      l = (TABLE *) add_table_node (l, tn->left);
      l = (TABLE *) replace_in_table (l, (tn->entity != NULL) ?
				      copy_entity (tn->entity) : NULL,
				      dup_char (tn->name));
      l = (TABLE *) add_table_node (l, tn->right);
    }
  return (ENT (l));
}

ENTITY *
subtract_table (TABLE *l, TABLE *r)
{
  EASSERT (l, table, 0);
  EASSERT (r, table, 0);

  l = (TABLE *) subtract_table_node (l, r->table_node);
  delete_table (r);
  return (ENT (l));
}

static ENTITY *
subtract_table_node (TABLE *l, TABLE_NODE *tn)
{
  if (tn != &table_tail)
    {
      l = (TABLE *) subtract_table_node (l, tn->left);
      l = (TABLE *) delete_from_table (l, dup_char (tn->name));
      l = (TABLE *) subtract_table_node (l, tn->right);
    }
  return (ENT (l));
}

ENTITY *
lowercase_table (TABLE *head)
{
  EASSERT (head, table, 0);

  head = (TABLE *) dup_table (head);
  lowercase_table_node (head->table_node);
  return (ENT (head));
}

static void
lowercase_table_node (TABLE_NODE *tn)
{
  char *c;

  if (tn != &table_tail)
    {
      lowercase_table_node (tn->left);
      for (c = tn->name; *c; c++)
	*c = tolower (*c);
      lowercase_table_node (tn->right);
    }
}

char *
first_in_table (TABLE *head)
{
  char *c;

  EASSERT (head, table, 0);

  c = first_in_table_node (head->table_node);
  delete_table (head);
  return (c);
}

static char *
first_in_table_node (TABLE_NODE *tn)
{
  char *c;

  if (tn != &table_tail)
    {
      return ((c = first_in_table_node (tn->left)) ? c : dup_char (tn->name));
    }
  return (NULL);
}

int
visit_table 
VA_ALIST2 (TABLE * t, PFI f)

    /* Apply function `f' to each member of table `t'. */

     int r;
     va_list arg;

VA_START2 (arg, TABLE *, t, PFI, f);

EASSERT (t, table, 0);

WITH_HANDLING
{
  r = visit_table_node (t->table_node, f, arg);
}
ON_EXCEPTION
{
  va_end (arg);
  delete_table (t);
}
END_EXCEPTION;

va_end (arg);
delete_table (t);

return r;
}

static int
visit_table_node (TABLE_NODE *tn, PFI f, va_list arg)
{
  /*  If the current entity is NULL, just skip it. */

  return (tn == &table_tail ||
	  ((!tn->entity ||
	   (*f) (dup_char (tn->name), copy_entity (tn->entity), arg)) &&
	   visit_table_node (tn->left, f, arg) &&
	   visit_table_node (tn->right, f, arg)));
}

static void
binop_table_other_node (int op, TABLE_NODE *tn, ENTITY *r)
{
  if (tn != &table_tail)
    {
      if (!tn->entity)
	{
	  fail (msg_NULL_op, "Binary");
	  raise_exception ();
	}

      POLL_SIGINT ();
      binop_table_other_node (op, tn->left, r);
      tn->entity = binop_entity (op, EAT (tn->entity), copy_entity (r));
      binop_table_other_node (op, tn->right, r);
    }
}

ENTITY *
binop_table_other (int op, TABLE *l, ENTITY *r)
{
  /*
   * A binary operation between a table and a non-table.  We simply
   * map the operation to each member of the table.
   */

  if (l->nm)
    {
      WITH_HANDLING
      {
	l = (TABLE *) dup_table (EAT (l));
	binop_table_other_node (op, l->table_node, r);
      }
      ON_EXCEPTION
      {
	delete_table (l);
	delete_entity (r);
      }
      END_EXCEPTION;
    }

  delete_entity (r);
  return ENT (l);
}

static void
binop_other_table_node (int op, ENTITY *l, TABLE_NODE *tn)
{
  if (tn != &table_tail)
    {
      if (!tn->entity)
	{
	  fail (msg_NULL_op, "Binary");
	  raise_exception ();
	}

      POLL_SIGINT ();
      binop_other_table_node (op, l, tn->left);
      tn->entity = binop_entity (op, copy_entity (l), EAT (tn->entity));
      binop_other_table_node (op, l, tn->right);
    }
}

ENTITY *
binop_other_table (int op, ENTITY *l, TABLE *r)
{
  /*
   * A binary operation between a non-table and a table.  We simply
   * map the operation to each member of the table.
   */

  if (r->nm)
    {
      WITH_HANDLING
      {
	r = (TABLE *) dup_table (EAT (r));
	binop_other_table_node (op, l, r->table_node);
      }
      ON_EXCEPTION
      {
	delete_entity (l);
	delete_table (r);
      }
      END_EXCEPTION;
    }

  delete_entity (l);
  return ENT (r);
}

static void
unop_table_node (int op, TABLE_NODE *tn)
{
  if (tn != &table_tail)
    {
      POLL_SIGINT ();
      unop_table_node (op, tn->left);

      if (!tn->entity)
	{
	  switch (op)
	    {
	    case UO_NOT:
	      tn->entity = int_to_scalar (1);
	      break;

	    case UO_NEG:
	      fail (msg_NULL_op, "Negate");
	      raise_exception ();

	    case UO_TRANS:
	      fail (msg_NULL_op, "Transpose");
	      raise_exception ();

	    default:
	      wipeout ("Bad unop.");
	      break;
	    }
	}
      else
	{
	  tn->entity = unop_entity (op, EAT (tn->entity));
	  unop_table_node (op, tn->right);
	}
    }
}

ENTITY *
unop_table (int op, TABLE *t)
{
  /*
   * A unary operation on a table.  We simply map the operation to
   * each member of the table.
   */

  if (t->nm)
    {
      WITH_HANDLING
      {
	t = (TABLE *) dup_table (EAT (t));
	unop_table_node (op, t->table_node);
      }
      ON_EXCEPTION
      {
	delete_table (t);
      }
      END_EXCEPTION;
    }

  return ENT (t);
}

static void
map_table_node (TABLE_NODE *tn, PFE f, int n, ENTITY **args)
{
  if (tn != &table_tail)
    {
      int i;

      POLL_SIGINT ();
      map_table_node (tn->left, f, n, args);

      /* Copy the args, unless they're NULL. */

      for (i = 0; i < n; i++)
	if (args[i])
	  copy_entity (args[i]);

      switch (n)
	{
	case 0:
	  tn->entity = ((ENTITY *(*)(ENTITY *))f) (EAT (tn->entity));
	  break;
	case 1:
	  tn->entity = ((ENTITY *(*)(ENTITY *, ENTITY *))f) (EAT (tn->entity),
			     args[0]);
	  break;
	case 2:
	  tn->entity = ((ENTITY *(*)(ENTITY *, ENTITY *, ENTITY *))f) (EAT (tn->entity),
			     args[0], args[1]);
	  break;
	case 3:
	  tn->entity = ((ENTITY *(*)(ENTITY *, ENTITY *, ENTITY *, ENTITY *))f) (EAT (tn->entity),
			     args[0], args[1], args[2]);
	  break;
	case 4:
	  tn->entity = ((ENTITY *(*)(ENTITY *, ENTITY *, ENTITY *, ENTITY *, ENTITY *))f) (EAT (tn->entity),
			     args[0], args[1], args[2], args[3]);
	  break;
	default:
	  wipeout ("Too many args.");
	}

      map_table_node (tn->right, f, n, args);
    }
}

ENTITY *
map_table (TABLE *t, PFE f, volatile int n, ENTITY **args)
{
  /* Apply function `f' (with `n' args) to each member of table `t'. */

  EASSERT (t, table, 0);

  WITH_HANDLING
  {
    t = (TABLE *) dup_table (EAT (t));
    map_table_node (t->table_node, f, n, args);
  }
  ON_EXCEPTION
  {
    delete_table (t);
    while (n--)
      delete_entity (args[n]);
  }
  END_EXCEPTION;

  while (n--)
    delete_entity (args[n]);
  return ENT (t);
}
