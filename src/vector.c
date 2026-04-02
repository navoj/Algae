/*
   vector.c -- Functions for Algae vectors.

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: vector.c,v 1.4 2001/09/15 15:57:03 ksh Exp $";

#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "get.h"
#include "put.h"
#include "binop.h"
#include "file_io.h"
#include "dense.h"
#include "test.h"

/*
 * This array describes the members of the
 * VECTOR structure.  The fields are `name',
 * and `id'.  The entries must be in
 * alphabetical order, and there must be
 * exactly one entry for each member of the
 * VECTOR_MEMBER enumeration except END_Vector.
 */

MEMBER_ID vector_member_names[] =
{
  {"class", VectorClass},
  {"density", VectorDensity},
  {"eid", VectorEid},
  {"ne", VectorNe},
  {"nn", VectorNn},
  {"order", VectorOrder},
  {"type", VectorType}
};

VECTOR_MEMBER
vector_member_search (char *s)
{
  /*
   * Returns a pointer to the MEMBER_ID of a vector corresponding to the
   * name `s'.  It uses `bsearch', but that's kind of silly since we never
   * have enough members to justify it.
   */

  MEMBER_ID *m;

  assert (s != NULL);

  m = (MEMBER_ID *) bsearch (s, vector_member_names, END_Vector,
			     sizeof (MEMBER_ID), member_cmp);

  return ((m == NULL) ? END_Vector : m->id);
}

ENTITY *
bi_vector (int n, ENTITY *p)
{
  /* Convert `p' to a vector, or return one with zero length if p==NULL. */

  return (p ? vector_entity (p) : make_vector (0, real, dense));
}

ENTITY *
vector_entity (ENTITY *p)
{
  /* Convert `p' to a vector. */

  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (scalar_to_vector ((SCALAR *) p));
    case vector:
      return (p);
    case matrix:
      return (matrix_to_vector ((MATRIX *) p));
    default:
      fail ("Can't convert a %s to a vector.",
	    class_string[p->class]);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
vector_to_scalar (VECTOR *v)
{
  /* Convert `v' to scalar (only if it's length is one). */

  ENTITY *s;

  EASSERT (v, vector, 0);

  /*  The vector must have only one element. */

  if (v->ne != 1)
    {
      fail ("Vector has %d elements---can't convert to scalar.", v->ne);
      delete_vector (v);
      raise_exception ();
    }

  switch (v->density)
    {
    case dense:
      switch (v->type)
	{
	case integer:
	  s = int_to_scalar (v->a.integer[0]);
	  break;
	case real:
	  s = real_to_scalar (v->a.real[0]);
	  break;
	case complex:
	  s = complex_to_scalar (v->a.complex[0]);
	  break;
	case character:
	  s = char_to_scalar (dup_char (v->a.character[0]));
	  break;
	default:
	  BAD_TYPE (v->type);
	  delete_vector (v);
	  raise_exception ();
	}
      break;
    case sparse:
      /*
       * It's a one-by-one vector, so it has either 0 or 1 elements.  If
       * it has 1 element, then that's the one we want.
       */
      if (v->nn > 0)
	{
	  switch (v->type)
	    {
	    case integer:
	      s = int_to_scalar (v->a.integer[0]);
	      break;
	    case real:
	      s = real_to_scalar (v->a.real[0]);
	      break;
	    case complex:
	      s = complex_to_scalar (v->a.complex[0]);
	      break;
	    case character:
	      s = char_to_scalar (dup_char (v->a.character[0]));
	      break;
	    default:
	      BAD_TYPE (v->type);
	      delete_vector (v);
	      raise_exception ();
	    }
	}
      else
	{
	  s = make_scalar (v->type);
	}
      break;
    default:
      fail ("Can't convert a %s vector to a scalar.",
	    density_string[v->density]);
      delete_vector (v);
      raise_exception ();
    }

  if (v->stuff)
    ((SCALAR *) s)->stuff = (TABLE *) copy_table (v->stuff);

  delete_vector (v);
  return (s);
}

ENTITY *
dup_vector (VECTOR *old)
{
  /*
   * In Algae, duplicating an entity means that we really make another
   * copy of it in memory (not just incrementing its reference count).
   * The `eid' member comes along, but nothing in `stuff'.
   */

  VECTOR *new;
  int i;

  EASSERT (old, vector, 0);

  if (old->entity.ref_count == 1)
    return (ENT (old));

  new = (VECTOR *) form_vector (old->ne, old->type, old->density);
  if (old->eid)
    new->eid = copy_entity (old->eid);
  new->nn = old->nn;

  if (old->ja != NULL)
    new->ja = (int *) dup_mem (old->ja, old->nn * sizeof (int));
  if (old->a.ptr != NULL)
    {
      if (new->a.ptr == NULL)
	new->a.ptr = E_MALLOC (old->nn, old->type);
      memcpy (new->a.ptr, old->a.ptr, old->nn * type_size[old->type]);
      if (new->type == character)
	for (i = 0; i < old->nn; i++)
	  new->a.character[i] = dup_char (new->a.character[i]);
    }

  delete_vector (old);
  return (ENT (new));
}

ENTITY *
scalar_to_vector (SCALAR *s)
{
  /* Convert the scalar `s' to a vector. */

  VECTOR *v;

  EASSERT (s, scalar, 0);

  v = (VECTOR *) form_vector (1, s->type, dense);
  switch (s->type)
    {
    case integer:
      v->a.integer[0] = s->v.integer;
      break;
    case real:
      v->a.real[0] = s->v.real;
      break;
    case complex:
      v->a.complex[0] = s->v.complex;
      break;
    case character:
      v->a.character[0] = dup_char (s->v.character);
      break;
    default:
      BAD_TYPE (s->type);
      delete_scalar (s);
      raise_exception ();
    }

  if (s->stuff)
    v->stuff = (TABLE *) copy_table (s->stuff);
  delete_scalar (s);
  return (ENT (v));
}

ENTITY *
make_vector (int ne, TYPE type, DENSITY density)
{
  /*
   * Makes a new vector with length `ne'.  It's initialized to zero (or
   * "" for character type).
   */

  VECTOR *v;
  int i;

  assert (ne >= 0);

  v = (VECTOR *) form_vector (ne, type, density);

  switch (density)
    {
    case dense:
      if (v->nn)
	memset (v->a.ptr, 0, v->nn * type_size[type]);
      if (type == character)
	for (i = 0; i < v->nn; i++)
	  v->a.character[i] = NULL_string;
      break;
    case sparse:
      break;
    default:
      wipeout ("Can't make a %s vector.", density_string[density]);
    }

  if (debug_level > 1)
    inform ("Vector created:  %x.", v);

  return (ENT (v));
}

ENTITY *
form_vector (int ne, TYPE type, DENSITY density)
{
  /*
   * Makes a new vector with length `ne'.  If dense, memory for
   * its data is allocated but not initialized.  (No memory is
   * allocated for data if it's sparse.)
   */

  VECTOR *v;

  assert (ne >= 0);

  v = (VECTOR *) CALLOC (1, sizeof (VECTOR));

  v->entity.ref_count = 1;
  v->entity.class = vector;

  v->type = type;
  v->order = ordered;
  v->density = density;
  v->ne = ne;

  switch (density)
    {
    case dense:
      v->nn = ne;
      v->a.ptr = (v->nn) ? E_MALLOC (v->nn, type) : NULL;
      if (v->nn && type == character)
	v->a.character[0] = NULL;
      break;
    case sparse:
      break;
    default:
      wipeout ("Can't make a %s vector.", density_string[density]);
    }

  if (debug_level > 1)
    inform ("Vector created:  %x.", v);

  return (ENT (v));
}

void
free_vector (VECTOR *p)
{
  int i;

  assert (p->entity.ref_count == 0);

  delete_table (p->stuff);
  delete_entity (p->eid);

  if (p->ja)
    FREE (p->ja);
  if (p->a.ptr)
    {
      if (p->type == character)
	{
	  for (i = 0; i < p->nn; i++)
	    FREE_CHAR (p->a.character[i]);
	}
      FREE (p->a.ptr);
    }

  /* Just to make it harder to use it again inadvertently. */
  p->entity.class = undefined_class;

  FREE (p);
}

void
DB_delete_vector (VECTOR *p, char *file, int line)
{
  /*
   * This is the DEBUG version of `delete_vector'.  It decrements
   * the vector's reference count, and frees it if it is unreferenced.
   */

  if (p)
    {

      if (--p->entity.ref_count < 0)
	{
	  wipeout ("A vector's \"ref_count\" went below zero: %s, %d.",
		   file, line);
	}

      if (p->entity.ref_count >= 1000 || debug_level > 1)
	{
	  inform ("vector \"ref_count\" decrement:  %x, %d, %s, %d.",
		  p, p->entity.ref_count, file, line);
	}

      if (p->entity.ref_count == 0)
	free_vector (p);

    }
}

int
put_vector (VECTOR *s, FILE *stream, struct ent_node *ent_tree)
{
  /* Write vector `s' out in binary form to file `stream'. */

  size_t size;
  int i, isize;

  EASSERT (s, vector, 0);

  assert (s->order == ordered);	/* unordered vectors are obsolete */

  if (!WRITE_INT (&s->type, stream) ||
      !WRITE_INT (&s->density, stream) ||
      !WRITE_INT (&s->ne, stream) ||
      !WRITE_INT (&s->nn, stream))
    goto err;

  if (s->a.ptr)
    {

      i = 1;			/* values follow */
      if (!WRITE_INT (&i, stream))
	goto err;

      switch (s->type)
	{

	case integer:
	  if (!WRITE_INTS (s->a.integer, s->nn, stream))
	    goto err;
	  break;

	case real:
	  if (!WRITE_DOUBLES (s->a.real, s->nn, stream))
	    goto err;
	  break;

	case complex:
	  if (!WRITE_DOUBLES (s->a.real, 2 * s->nn, stream))
	    goto err;
	  break;

	case character:
	  for (i = 0; i < s->nn; i++)
	    {
	      size = strlen (s->a.character[i]);
	      if (size > (size_t) INT_MAX)
		{
		  warn ("String too large.");
		  goto err;
		}
	      isize = (int) size;
	      if (!WRITE_INT (&isize, stream))
		goto err;
	      if (size > 0 &&
		  fwrite (s->a.character[i], 1, size, stream) < size)
		{
		  WRITE_WARN (stream);
		  goto err;
		}
	    }
	  break;

	default:
	  BAD_TYPE (s->type);
	  delete_vector (s);
	  raise_exception ();
	}

    }
  else
    {

      i = 0;			/* no values follow */
      if (!WRITE_INT (&i, stream))
	goto err;

    }

  if (s->ja)
    {
      i = 1;			/* ja follows */
      if (!WRITE_INT (&i, stream) ||
	  !WRITE_INTS (s->ja, s->nn, stream))
	goto err;
    }
  else
    {
      i = 0;			/* no ja */
      if (!WRITE_INT (&i, stream))
	goto err;
    }

  if (s->eid)
    {
      i = 1;			/* eid follows */
      if (!WRITE_INT (&i, stream) ||
	  !put_entity (copy_entity (s->eid), stream, ent_tree))
	goto err;
    }
  else
    {
      i = 0;			/* no eid */
      if (!WRITE_INT (&i, stream))
	goto err;
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

  delete_vector (s);
  return 1;

err:
  delete_vector (s);
  return 0;
}

ENTITY *
get_vector (FILE *stream, int ver)
{
  /* Read a vector from binary file `stream'. */

  VECTOR *s;
  int i, size;
  static char *warn_msg = "Invalid vector in file.";

  s = (VECTOR *) CALLOC (1, sizeof (VECTOR));
  s->entity.ref_count = 1;
  s->entity.class = vector;
  s->order = ordered;

  if (!READ_INT (&s->type, stream) ||
      !READ_INT (&s->density, stream) ||
      !READ_INT (&s->ne, stream) ||
      !READ_INT (&s->nn, stream))
    {
      FREE (s);
      return NULL;
    }

  if (!READ_INT (&i, stream))
    {				/* values follow? */
      FREE (s);
      return NULL;
    }

  if (i)
    {

      if (s->nn < 1)
	{
	  warn (warn_msg);
	  FREE (s);
	  return NULL;
	}

      switch (s->type)
	{

	case integer:
	  s->a.integer = MALLOC (s->nn * sizeof (int));
	  if (!READ_INTS (s->a.integer, s->nn, stream))
	    goto err;
	  break;

	case real:
	  s->a.real = MALLOC (s->nn * sizeof (double));
	  if (!READ_DOUBLES (s->a.real, s->nn, stream))
	    goto err;
	  break;

	case complex:
	  s->a.real = MALLOC (2 * s->nn * sizeof (double));
	  if (!READ_DOUBLES (s->a.real, 2 * s->nn, stream))
	    goto err;
	  break;

	case character:
	  s->a.character = MALLOC (s->nn * sizeof (char *));
	  for (i = 0; i < s->nn; i++)
	    s->a.character[i] = NULL_string;
	  for (i = 0; i < s->nn; i++)
	    {
	      if (!READ_INT (&size, stream))
		goto err;
	      if (size > 0)
		{
		  s->a.character[i] = (char *) MALLOC (size + 1);
		  if (fread (s->a.character[i], 1, size, stream) <
		      (size_t) size)
		    {
		      READ_WARN (stream);
		      goto err;
		    }
		  s->a.character[i][size] = '\0';
		}
	    }
	  break;

	default:
	  warn (warn_msg);
	  goto err;
	}

    }
  else
    {
      s->a.ptr = NULL;
    }

  if (!READ_INT (&i, stream))
    goto err;			/* ja follows? */
  if (i)
    {
      s->ja = (int *) MALLOC (s->nn * sizeof (int));
      if (!READ_INTS (s->ja, s->nn, stream))
	goto err;
    }

  if (!READ_INT (&i, stream))
    goto err;			/* eid follows */
  if (i && !(s->eid = get_entity (stream)))
    goto err;

  if (!READ_INT (&i, stream))
    goto err;			/* stuff follows */
  if (i && !(s->stuff = (TABLE *)
	     (ver ? get_entity (stream) : get_table (stream, ver))))
    goto err;

  if (!ok_entity (ENT (s)))
    goto err;

  return apt_vector (s);

err:
  delete_vector (s);
  return (NULL);
}

int
match_vectors (VECTOR *l, VECTOR *r)
{
  /*
   * Tests equality of two vectors.  BOTH `l' AND `r' MUST BE
   * NON-NULL AND ARE NOT DELETED.
   */

  int i;

  EASSERT (l, vector, 0);
  EASSERT (r, vector, 0);

  if (l->ne != r->ne)
    return 0;

  /* Check them right here if dense and same type. */

  if (l->type == r->type &&
      l->density == dense &&
      r->density == dense)
    {

      switch (l->type)
	{

	case integer:

	  for (i = 0; i < l->ne; i++)
	    if (l->a.integer[i] != r->a.integer[i])
	      return 0;
	  break;

	case real:

	  for (i = 0; i < l->ne; i++)
	    if (l->a.real[i] != r->a.real[i])
	      return 0;
	  break;

	case complex:

	  for (i = 0; i < l->ne; i++)
	    if (l->a.complex[i].real != r->a.complex[i].real ||
		l->a.complex[i].imag != r->a.complex[i].imag)
	      return 0;
	  break;

	case character:

	  for (i = 0; i < l->ne; i++)
	    if (strcmp (l->a.character[i], r->a.character[i]))
	      return 0;
	  break;

	default:

	  BAD_TYPE (l->type);
	  delete_2_vectors (l, r);
	  raise_exception ();
	}

      return 1;

    }
  else
    {
      assert (l->type != r->type);
      if (l->type == character || r->type == character) return 0;

      return !test_vector ((VECTOR *)
		     binop_vector_vector (BO_NE, (VECTOR *) copy_vector (l),
					  (VECTOR *) copy_vector (r)));
    }
}

ENTITY *
gift_wrap_vector (int ne, TYPE type, void *data)
{
  /* Wrap a vector entity around the given data. */

  VECTOR *v;

  assert (ne >= 0);

  v = (VECTOR *) CALLOC (1, sizeof (VECTOR));

  v->entity.ref_count = 1;
  v->entity.class = vector;

  v->type = type;
  v->order = ordered;
  v->density = dense;
  v->nn = v->ne = ne;

  v->a.ptr = (v->nn) ? data : NULL;

  if (debug_level > 1)
    inform ("Vector created:  %x.", v);

  return (ENT (v));
}
