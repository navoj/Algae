/*
   put.c -- Write entities to files.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: put.c,v 1.4 2003/08/01 04:57:48 ksh Exp $";

#include <stdio.h>
#include "put.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "function.h"
#include "file_io.h"

/*
 * We keep a binary tree containing pointers to entities that we've
 * written.  Before we write another one, we check to see if we've
 * already written it.
 */

/* The leaves of the tree all point here. */
static struct ent_node ent_tail =
{NULL, 0, &ent_tail, &ent_tail};

/* Count of the number of entities we've written. */
static int ent_count;

static void
free_ent_tree (struct ent_node *ent_tree)
{
  /* Free all the nodes of the entity tree. */

  if (ent_tree != &ent_tail)
    {
      free_ent_tree (ent_tree->left);
      free_ent_tree (ent_tree->right);
      FREE (ent_tree);
    }
}

ENTITY *
bi_put (int n, ENTITY *p, ENTITY *fname)
{
  /*
   * This routine writes an entity to a file.  The file is in Algae's
   * "native" binary format, which is essentially a mirror of the C
   * structures.  Someday we'll have to formalize this...
   *
   * If `fname' is not given, then we write to stdout.  If `fname'
   * can't be taken as a scalar we'll raise an exception.  Otherwise
   * we return NULL on error and 1 otherwise.
   */

  FILE * volatile stream;
  int ret;
  struct ent_node * volatile ent_head;

  EASSERT (p, 0, 0);

  WITH_HANDLING
  {
    stream = fname ? find_file (entity_to_string (EAT (fname)),
				FILE_OUTPUT) : stdout;

    /* Entity tree must start with a NULL node. */

    ent_head = MALLOC (sizeof (struct ent_node));
    ent_head->entity = NULL;
    ent_head->left = ent_head->right = &ent_tail;
    ent_count = 0;
  }
  ON_EXCEPTION
  {
    delete_2_entities (p, fname);
  }
  END_EXCEPTION;

  if (!stream)
    {
      delete_entity (p);
      return NULL;
    }

  ret = put_entity (p, stream, ent_head);
  free_ent_tree (ent_head);

  return ret ? int_to_scalar (1) : NULL;
}

static int
check_ent_tree (ENTITY *p, struct ent_node *ent_tree)
{
  /*
   * Check for the pointer `p' in the entity tree `ent_tree'.  Returns
   * its sequence number if it's found; otherwise, it adds it to the
   * tree and returns 0;
   */

  struct ent_node *parent;

  ent_tail.entity = p;
  while (p != ent_tree->entity)
    {
      parent = ent_tree;
      ent_tree = (p < ent_tree->entity) ? ent_tree->left : ent_tree->right;
    }

  if (ent_tree == &ent_tail)
    {
      struct ent_node *e = MALLOC (sizeof (struct ent_node));
      e->entity = p;
      e->seq = ++ent_count;
      e->left = e->right = &ent_tail;
      if (p < parent->entity)
	parent->left = e;
      else
	parent->right = e;
      return 0;
    }

  return ent_tree->seq;
}

int
put_entity (ENTITY *p, FILE *stream, struct ent_node *ent_tree)
{
  char ver = (char) FILE_FORMAT;
  int seq;

  /*
   * Write entity `p' to `stream'.  Returns 0 iff it couldn't do it.
   */

  EASSERT (p, 0, 0);

  /* Write the magic string and version number. */

  if (fwrite ("\177Alki", 1, 5, stream) < 5 ||
      fwrite (&ver, 1, 1, stream) < 1)
    {
      WRITE_WARN (stream);
      delete_entity (p);
      return 0;
    }

  /*
   * Check to see if we've already written this entity.  (If it has
   * only a single reference count, then we don't need to check.)
   */

  if (p->ref_count && (seq = check_ent_tree (p, ent_tree)))
    {
      /* already written -- just write its sequence number */

      if (!WRITE_INT (&seq, stream))
	{
	  delete_entity (p);
	  return 0;
	}
    }
  else
    {
      /* write 0 for the sequence number, then the rest of the entity */

      seq = 0;
      if (!WRITE_INT (&seq, stream))
	{
	  delete_entity (p);
	  return 0;
	}

      if (!WRITE_INT (&p->class, stream))
	{
	  delete_entity (p);
	  return 0;
	}

      switch (p->class)
	{
	case scalar:
	  return put_scalar ((SCALAR *) p, stream, ent_tree);
	case vector:
	  return put_vector ((VECTOR *) p, stream, ent_tree);
	case matrix:
	  return put_matrix ((MATRIX *) p, stream, ent_tree);
	case table:
	  return put_table ((TABLE *) p, stream, ent_tree);
	case function:
	  return put_function ((FUNCTION *) p, stream, ent_tree);
	default:
	  BAD_CLASS (p->class);
	  delete_entity (p);
	  return 0;
	}
    }

  return 1;
}

size_t
fwrite_int_reverse (char *ptr, size_t n, FILE *stream)
{
  char t[4];
  size_t w = 0;

  assert (sizeof (int) == 4);
  assert (ptr != NULL);
  assert (n > 0);

  do
    {
      t[0] = ptr[3]; t[1] = ptr[2]; t[2] = ptr[1]; t[3] = ptr[0];

      if (fwrite (t, 4, 1, stream) < 1)
	return w;
    }
  while (ptr += 4, ++w < n);

  return w;
}

size_t
fwrite_double_reverse (char *ptr, size_t n, FILE *stream)
{
  char t[8];
  size_t w = 0;

  assert (sizeof (double) == 8);
  assert (ptr != NULL);
  assert (n > 0);

  do
    {
      t[0] = ptr[7]; t[1] = ptr[6]; t[2] = ptr[5]; t[3] = ptr[4];
      t[4] = ptr[3]; t[5] = ptr[2]; t[6] = ptr[1]; t[7] = ptr[0];

      if (fwrite (t, 8, 1, stream) < 1)
	return w;
    }
  while (ptr += 8, ++w < n);

  return w;
}

#if BINARY_FORMAT == CRAY_FLOAT

static char *cray_format_error = "Error converting from Cray to IEEE format.";

size_t
fwrite_int_cray (char *ptr, size_t n, FILE *stream)
{
  int type = 1;
  int bitoff = 0;
  int num = n;
  char *buf = MALLOC (num * 4);

  num = n;
  if (CRAY2IEG (&type, &num, buf, &bitoff, ptr))
    {
      fail (cray_format_error);
      FREE (buf);
      raise_exception ();
    }
  num = fwrite (buf, 4, n, stream);

  FREE (buf);
  return num;
}

size_t
fwrite_double_cray (char *ptr, size_t n, FILE *stream)
{
  int type = 8;
  int bitoff = 0;
  int num;
  char *buf = MALLOC (n * 8);

  num = n;
  if (CRAY2IEG (&type, &num, buf, &bitoff, ptr))
    {
      fail (cray_format_error);
      FREE (buf);
      raise_exception ();
    }
  num = fwrite (buf, 8, n, stream);

  FREE (buf);
  return num;
}

#endif
