/*
   scalar.c -- Functions for scalars.

   Copyright (C) 1994-96  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: scalar.c,v 1.3 1996/08/29 06:01:38 ksh Exp $";

#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "get.h"
#include "put.h"
#include "file_io.h"

/*
 * This array describes the members of the
 * SCALAR structure.  The fields are `name',
 * and `id'.  The entries must be in
 * alphabetical order, and there must be
 * exactly one entry for each member of the
 * SCALAR_MEMBER enumeration except END_Scalar.
 */

MEMBER_ID scalar_member_names[] =
{
  {"class", ScalarClass},
  {"type", ScalarType},
};

SCALAR_MEMBER
scalar_member_search (char *s)
{
  MEMBER_ID *m;

  assert (s != NULL);

  m = (MEMBER_ID *) bsearch (s, scalar_member_names, END_Scalar,
			     sizeof (MEMBER_ID), member_cmp);

  return ((m == NULL) ? END_Scalar : m->id);
}

ENTITY *
bi_scalar (int n, ENTITY *p)
{
  /* Convert to a scalar, or return 0 if `p' is NULL. */

  return p ? scalar_entity (p) : int_to_scalar (0);
}

ENTITY *
scalar_entity (ENTITY *ip)
{
  /*
   * Convert an entity to scalar class.  The `ip' arg must point
   * to a valid entity.
   */

  EASSERT (ip, 0, 0);

  switch (ip->class)
    {
    case scalar:
      return ip;
    case vector:
      return vector_to_scalar ((VECTOR *) ip);
    case matrix:
      return matrix_to_scalar ((MATRIX *) ip);
    default:
      fail ("Can't convert a %s entity to a scalar.",
	    class_string[ip->class]);
      delete_entity (ip);
      raise_exception ();
    }
}

ENTITY *
make_scalar (TYPE type)
{
  /*
   * This routine makes a scalar with the given type.  Space for
   * the value is allocated and initialized with zero.
   */

  SCALAR *p;

  p = (SCALAR *) CALLOC (1, sizeof (SCALAR));
  p->entity.ref_count = 1;
  p->entity.class = scalar;
  p->type = type;
  if (type == character)
    p->v.character = NULL_string;
  p->stuff = NULL;

  if (debug_level > 1)
    inform ("Scalar created:  %x.", p);

  return ENT (p);
}

ENTITY *
int_to_scalar (int i)
{
  /* Turns an int into an integer scalar. */

  SCALAR *op;

  op = (SCALAR *) MALLOC (sizeof (SCALAR));
  op->entity.ref_count = 1;
  op->entity.class = scalar;
  op->type = integer;
  op->stuff = NULL;
  op->v.integer = i;

  if (debug_level > 1)
    inform ("Scalar created:  %x.", op);

  return ENT (op);
}

ENTITY *
real_to_scalar (REAL r)
{
  /* Turns a real value into a real scalar. */

  SCALAR *op;

  op = (SCALAR *) MALLOC (sizeof (SCALAR));
  op->entity.ref_count = 1;
  op->entity.class = scalar;
  op->type = real;
  op->v.real = r;
  op->stuff = NULL;

  if (debug_level > 1)
    inform ("Scalar created:  %x.", op);

  return ENT (op);
}

ENTITY *
complex_to_scalar (COMPLEX c)
{
  /* Turns a complex value into a complex scalar. */

  SCALAR *op;

  op = (SCALAR *) MALLOC (sizeof (SCALAR));
  op->entity.ref_count = 1;
  op->entity.class = scalar;
  op->type = complex;
  op->v.complex = c;
  op->stuff = NULL;

  if (debug_level > 1)
    inform ("Scalar created:  %x.", op);

  return (ENT (op));
}

ENTITY *
char_to_scalar (char *p)
{
  /*
   * Turns a string into a character scalar.  The string `p' is eaten,
   * so call `dup_char' on it first if you don't want to lose it.
   */

  SCALAR *op;

  op = (SCALAR *) MALLOC (sizeof (SCALAR));
  op->entity.ref_count = 1;
  op->entity.class = scalar;
  op->type = character;

  /* Try to point all zero-length strings to same place. */

  if (*p)
    {
      op->v.character = p;
    }
  else
    {
      op->v.character = NULL_string;
      FREE_CHAR (p);
    }

  op->stuff = NULL;

  if (debug_level > 1)
    inform ("Scalar created:  %x.", op);

  return ENT (op);
}

ENTITY *
dup_scalar (SCALAR *ips)
{
  SCALAR *ops;

  EASSERT (ips, scalar, 0);

  if (ips->entity.ref_count == 1)
    return ENT (ips);

  ops = (SCALAR *) dup_mem (ips, sizeof (SCALAR));
  ops->entity.ref_count = 1;
  if (ops->type == character)
    ops->v.character = dup_char (ops->v.character);
  ops->stuff = NULL;

  if (debug_level > 1)
    inform ("Scalar created:  %x.", ops);

  delete_scalar (ips);
  return ENT (ops);
}

void
free_scalar (SCALAR *p)
{
  /*
   * Called by `delete_scalar' to free memory used by a scalar.  The
   * ref_count must be zero.
   */

  assert (p->entity.ref_count == 0);

  delete_table (p->stuff);
  if (p->type == character && p->v.character != NULL_string)
    FREE (p->v.character);

  /* Just to make it harder to use it again inadvertently. */
  p->entity.class = undefined_class;
  FREE (p);
}

void
DB_delete_scalar (SCALAR *p, char *file, int line)
{
  /*
   * This is the DEBUG version of `delete_scalar'.  It decrements
   * the scalar's reference count and frees it if it is unreferenced.
   * OK to pass a NULL pointer for `p'.
   */

  if (p)
    {
      if (--p->entity.ref_count < 0)
	{
	  wipeout ("A scalar's \"ref_count\" went below zero:  %s, %d.",
		   file, line);
	}

      if (p->entity.ref_count >= 1000 || debug_level > 1)
	{
	  inform ("scalar \"ref_count\" decrement:  %x, %d, %s, %d.",
		  p, p->entity.ref_count, file, line);
	}

      if (p->entity.ref_count == 0)
	free_scalar (p);
    }
}

int
put_scalar (SCALAR *s, FILE *stream, struct ent_node *ent_tree)
{
  /* Write scalar `s' out in binary form to file `stream'. */

  int i, size;

  EASSERT (s, scalar, 0);

  if (!WRITE_INT (&s->type, stream))
    goto err;

  switch (s->type)
    {
    case integer:
      if (!WRITE_INT (&s->v.integer, stream))
	goto err;
      break;

    case real:
      if (!WRITE_DOUBLE (&s->v.real, stream))
	goto err;
      break;

    case complex:
      if (!WRITE_DOUBLE (&s->v.complex.real, stream) ||
	  !WRITE_DOUBLE (&s->v.complex.imag, stream))
	goto err;
      break;

    case character:
      size = strlen (s->v.character);
      if (!WRITE_INT (&size, stream))
	goto err;
      if (size > 0 && fwrite (s->v.character, 1, size, stream) <
	  (size_t) size)
	{
	  WRITE_WARN (stream);
	  goto err;
	}
      break;

    default:
      BAD_TYPE (s->type);
      delete_scalar (s);
      raise_exception ();
    }

  if (s->stuff)
    {
      i = 1;			/* stuff follows */
      if (!WRITE_INT (&i, stream) ||
	  !put_entity (copy_table (s->stuff), stream, ent_tree))
	goto err;
    }
  else
    {
      i = 0;			/* no stuff */
      if (!WRITE_INT (&i, stream))
	goto err;
    }

  delete_scalar (s);
  return 1;

err:
  delete_scalar (s);
  return 0;
}

ENTITY *
get_scalar (FILE *stream, int ver)
{
  /* Read a scalar from the binary file `stream'. */

  SCALAR *s;
  int i, size;

  s = (SCALAR *) CALLOC (1, sizeof (SCALAR));
  s->entity.ref_count = 1;
  s->entity.class = scalar;

  if (!READ_INT (&s->type, stream))
    {
      FREE (s);
      return NULL;
    }

  switch (s->type)
    {
    case integer:
      if (!READ_INT (&s->v.integer, stream))
	goto err;
      break;

    case real:
      if (!READ_DOUBLE (&s->v.real, stream))
	goto err;
      break;

    case complex:
      if (!READ_DOUBLE (&s->v.complex.real, stream) ||
	  !READ_DOUBLE (&s->v.complex.imag, stream))
	goto err;
      break;

    case character:
      if (!READ_INT (&size, stream))
	goto err;
      if (size > 0)
	{
	  s->v.character = (char *) MALLOC (size + 1);
	  if (fread (s->v.character, 1, size, stream) < (size_t) size)
	    {
	      READ_WARN (stream);
	      goto err;
	    }
	  s->v.character[size] = '\0';
	}
      else
	{
	  s->v.character = NULL_string;
	}
      break;

    default:
      warn ("Invalid scalar type in file.");
      goto err;
    }

  if (!READ_INT (&i, stream))
    goto err;			/* stuff follows? */
  if (i && !(s->stuff = (TABLE *)
	     (ver ? get_entity (stream) : get_table (stream, ver))))
    goto err;

  if (!ok_entity (ENT (s)))
    goto err;

  return ENT (s);

err:
  delete_scalar (s);
  return NULL;
}
