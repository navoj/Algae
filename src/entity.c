/*
   entity.c -- Miscellaneous routines for entities.

   Copyright (C) 1994-2002  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: entity.c,v 1.5 2002/07/30 22:05:17 ksh Exp $";

#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "function.h"
#include "cast.h"

#define ENUM_STRNG_INIT		1
#include "enum_strng.h"

/*
 * When debugging, a message is printed if the number of references
 * to an entity exceeds DB_REF_THRESHOLD.
 */

#ifndef DB_REF_THRESHOLD
#define DB_REF_THRESHOLD 1000
#endif

char NULL_string[1] = "";	/* Most NULL strings point here. */

void *
eat (void **p)
{
  void *e;

  e = *p;
  *p = NULL;
  return (e);
}

ENTITY *
DB_copy_entity (ENTITY *p, char *file, int line)
{
  EASSERT (p, 0, 0);

  if (++p->ref_count >= DB_REF_THRESHOLD || debug_level > 1)
    inform ("%s \"ref_count\" increment:  %x, %d, %s, %d.",
	    class_string[p->class], p, p->ref_count, file, line);

  return (p);
}

void
DB_delete_entity (ENTITY *p, char *file, int line)
{
  /*
   * This is the DEBUG version of `delete_entity'.  It decrements
   * the entity's reference count, and frees it if it is unreferenced.
   */

  if (p)
    {

      if (--p->ref_count < 0)
	{
	  wipeout ("A %s's \"ref_count\" went below zero:  %s, %d.",
		   class_string[p->class], file, line);
	}

      if (p->ref_count >= DB_REF_THRESHOLD || debug_level > 1)
	{
	  inform ("%s \"ref_count\" decrement:  %x, %d, %s, %d.",
		  class_string[p->class], p, p->ref_count, file, line);
	}

      if (p->ref_count == 0)
	free_entity (p);

    }
}

void
free_entity (ENTITY *p)
{
  assert (p != NULL);
  assert (p->ref_count == 0);

  switch (p->class)
    {
    case scalar:
      free_scalar ((SCALAR *) p);
      break;
    case vector:
      free_vector ((VECTOR *) p);
      break;
    case matrix:
      free_matrix ((MATRIX *) p);
      break;
    case table:
      free_table ((TABLE *) p);
      break;
    case function:
      free_function ((FUNCTION *) p);
      break;
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
dup_entity (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (dup_scalar ((SCALAR *) p));
    case vector:
      return (dup_vector ((VECTOR *) p));
    case matrix:
      return (dup_matrix ((MATRIX *) p));
    case table:
      return (dup_table ((TABLE *) p));
    case function:
      return (dup_function ((FUNCTION *) p));
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

char *
dup_char (char *p)
{
  assert (p != NULL);

  if (*p == '\0')
    return (NULL_string);

  return (strcpy (MALLOC (strlen (p) + 1), p));
}

char *
append_char (char *l, char *r)
{
  char *p;

  assert (l != NULL && r != NULL);
  p = MALLOC (strlen (l) + strlen (r) + 1);
  return (strcat (strcpy (p, l), r));
}

void *
dup_mem (void *p, int len)
{
  assert (p != NULL);

  return (memcpy (MALLOC (len), p, len));
}

ENTITY *
not_NULL (ENTITY *p)
{
  if (p == NULL)
    raise_exception ();
  return (p);
}

int
member_cmp (const void *keyval, const void *dat)
{
  return (strcmp ((char *) keyval, ((MEMBER_ID *) dat)->name));
}

ENTITY *
bi_class (int n, ENTITY *p)
{
  char *c;

  if (p)
    {
      c = class_string[p->class];
      delete_entity (p);
    }
  else
    {
      c = "NULL";
    }

  return char_to_scalar (dup_char (c));
}

char *
th (int i)
{
  if (i < 0)
    {
      return ("");
    }
  else if (i > 3 && i < 21)
    {
      return ("th");
    }
  else
    {
      switch (i % 10)
	{
	case 1:
	  return ("st");
	case 2:
	  return ("nd");
	case 3:
	  return ("rd");
	default:
	  return ("th");
	}
    }
}

int
entity_to_int (ENTITY *e)
{
  /* Convert an entity to an integer */

  int i;

  e = cast_scalar ((SCALAR *) scalar_entity (e), integer);
  i = ((SCALAR *) e)->v.integer;

  delete_scalar ((SCALAR *) e);
  return (i);
}

REAL
entity_to_real (ENTITY *e)
{
  /* Convert an entity to a real. */

  REAL r;

  e = cast_scalar ((SCALAR *) scalar_entity (e), real);
  r = ((SCALAR *) e)->v.real;

  delete_scalar ((SCALAR *) e);
  return r;
}

COMPLEX
entity_to_complex (ENTITY *e)
{
  /* Convert an entity to a complex. */

  COMPLEX r;

  e = cast_scalar ((SCALAR *) scalar_entity (e), complex);
  r.real = ((SCALAR *) e)->v.complex.real;
  r.imag = ((SCALAR *) e)->v.complex.imag;

  delete_scalar ((SCALAR *) e);
  return r;
}

char *
entity_to_string (ENTITY *e)
{
  /* Convert an entity to a character string. */

  char *s;

  e = cast_scalar ((SCALAR *) scalar_entity (e), character);

  /*
   * We'll try to take a little shortcut here.  If the ref_count
   * is 1, we'll just steal the entity's copy of the string
   * instead of malloc'ing another.
   */

  if (e->ref_count == 1)
    {
      s = ((SCALAR *) e)->v.character;
      ((SCALAR *) e)->v.character = NULL_string;
    }
  else
    {
      s = dup_char (((SCALAR *) e)->v.character);
    }

  delete_scalar ((SCALAR *) e);
  return (s);
}

void *
memmove_forward (void *s, void *t, size_t n)
{
  /*
   * Copy `n' characters from `t' to `s', and return `s'.  This
   * works if `t' and `s' overlap, provided that s>=t.
   */

  assert (s && t && n > 0);

  while (n--)
    ((char *) s)[n] = ((char *) t)[n];

  return (s);
}
