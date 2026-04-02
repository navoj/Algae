/*
   matrix.c -- MATRIX routines.

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: matrix.c,v 1.5 1997/04/24 06:01:03 ksh Exp $";

#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "get.h"
#include "put.h"
#include "file_io.h"
#include "binop.h"
#include "sparse.h"
#include "dense.h"

/*
 * This array describes the members of the
 * MATRIX structure.  The fields are `name',
 * and `id'.  The entries must be in
 * alphabetical order, and there must be
 * exactly one entry for each member of the
 * MATRIX_MEMBER enumeration except END_Matrix.
 */

MEMBER_ID matrix_member_names[] =
{
  {"cid", MatrixCid},
  {"class", MatrixClass},
  {"density", MatrixDensity},
  {"nc", MatrixNc},
  {"nn", MatrixNn},
  {"nr", MatrixNr},
  {"order", MatrixOrder},
  {"rid", MatrixRid},
  {"symmetry", MatrixSymmetry},
  {"type", MatrixType},
};

MATRIX_MEMBER
matrix_member_search (char *s)
{
  MEMBER_ID *m;

  assert (s != NULL);

  m = (MEMBER_ID *) bsearch (s, matrix_member_names, END_Matrix, sizeof (MEMBER_ID), member_cmp);

  return ((m == NULL) ? END_Matrix : m->id);
}

ENTITY *
bi_matrix (int n, ENTITY *p)
{
  /*
   * Convert `p' to matrix.  NULL becomes a zero-by-zero real
   * matrix.  Scalars become one-by-one matrices, vectors become
   * matrices with one row, and matrices are unchanged.
   */

  return p ? matrix_entity (p) : make_matrix (0, 0, real, dense);

}

ENTITY *
matrix_entity (ENTITY *p)
{
  /* Convert `p' to a matrix. */

  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (scalar_to_matrix ((SCALAR *) p));
    case vector:
      return (vector_to_matrix ((VECTOR *) p));
    case matrix:
      return (p);
    default:
      fail ("Can't convert a %s to a matrix.", class_string[p->class]);
      delete_entity (p);
      break;
    }
  raise_exception ();
}

ENTITY *
scalar_to_matrix (SCALAR *s)
{
  /* Convert `s' to matrix. */

  MATRIX *m;

  EASSERT (s, scalar, 0);

  m = (MATRIX *) form_matrix (1, 1, s->type, dense);
  m->symmetry = symmetric;

  switch (s->type)
    {
    case integer:
      m->a.integer[0] = s->v.integer;
      break;
    case real:
      m->a.real[0] = s->v.real;
      break;
    case complex:
      m->a.complex[0] = s->v.complex;
      break;
    case character:
      m->a.character[0] = dup_char (s->v.character);
      break;
    default:
      BAD_TYPE (s->type);
      delete_scalar (s);
      delete_matrix (m);
      raise_exception ();
    }

  if (s->stuff)
    m->stuff = (TABLE *) copy_table (s->stuff);

  delete_scalar (s);
  return (ENT (m));
}

ENTITY *
make_matrix (int nr, int nc, TYPE type, DENSITY density)
{
  /*
   * Makes a new matrix with `nr' rows and `nc' columns.  It's initialized
   * to zero (or "" if it's character) and marked as symmetric or hermitian
   * if it's square.
   *
   * NOTE: If the resulting matrix is square, then it is known to be
   * symmetric, even if you supply your own non-zero values on the
   * diagonal.  But if the matrix is complex, then it is hermitian
   * only by virtue of the fact that the diagonal values have zero
   * imaginary parts.  An easy way to screw up is to use this
   * function to form a complex array, then put in complex values.
   * That should be symmetric, not hermitian.
   */

  MATRIX *m;
  int i;

  m = (MATRIX *) form_matrix (nr, nc, type, density);

  m->symmetry = (nr == nc) ?
    ((type == complex) ? hermitian : symmetric) : general;

  switch (density)
    {
    case dense:
      if (m->nn)
	memset (m->a.ptr, 0, m->nn * type_size[type]);
      if (type == character)
	for (i = 0; i < m->nn; i++)
	  m->a.character[i] = NULL_string;
      break;
    case sparse:
    case sparse_upper:
      break;
    default:
      wipeout ("Bad density.");
    }

  if (debug_level > 1)
    inform ("Matrix created:  %x.", m);

  return (ENT (m));
}

ENTITY *
form_matrix (int nr, int nc, TYPE type, DENSITY density)
{
  /*
   * Forms a new matrix with `nr' rows and `nc' columns.  If dense,
   * memory for its data is allocated but not initialized.  (No memory
   * is allocated for data if it's sparse.)  Its symmetry is general.
   */

  MATRIX *m;

  assert (nr >= 0);
  assert (nc >= 0);

  m = (MATRIX *) CALLOC (1, sizeof (MATRIX));

  m->entity.ref_count = 1;
  m->entity.class = matrix;

  m->symmetry = general;

  m->type = type;
  m->order = ordered;
  m->density = density;

  m->nr = nr;
  m->nc = nc;

  switch (density)
    {

    case dense:

      if ((double) nr * (double) nc * type_size[type] > INT_MAX)
        {
          fail ("Out of memory.");
          raise_exception ();
        }
      m->nn = nr * nc;
      m->a.ptr = (m->nn) ? E_MALLOC (m->nn, type) : NULL;

      /*
       * For character type, set the first pointer to NULL so that
       * `free_matrix' knows there's junk there and it shouldn't
       * call FREE_CHAR on it.
       */

      if (m->nn && type == character)
	m->a.character[0] = NULL;

      break;

    case sparse:
    case sparse_upper:
      break;
    default:
      wipeout ("Bad density.");
    }

  if (debug_level > 1)
    inform ("Matrix created:  %x.", m);

  return (ENT (m));
}

ENTITY *
matrix_to_scalar (MATRIX *m)
{
  /* Convert `m' to scalar (only if it's one by one). */

  ENTITY *s;

  EASSERT (m, matrix, 0);

  /*  The matrix must have only 1 row and 1 column  */

  if (m->nr != 1 || m->nc != 1)
    {
      fail ("Can't convert to scalar a matrix with %d row%s and %d column%s.",
	    m->nr, PLURAL (m->nr), m->nc, PLURAL (m->nc));
      delete_matrix (m);
      raise_exception ();
    }
  else
    {
      switch (m->density)
	{
	case dense:
	  switch (m->type)
	    {
	    case integer:
	      s = int_to_scalar (m->a.integer[0]);
	      break;
	    case real:
	      s = real_to_scalar (m->a.real[0]);
	      break;
	    case complex:
	      s = complex_to_scalar (m->a.complex[0]);
	      break;
	    case character:
	      s = char_to_scalar (dup_char (m->a.character[0]));
	      break;
	    default:
	      BAD_TYPE (m->type);
	      delete_matrix (m);
	      raise_exception ();
	    }
	  break;
	case sparse:
	  /*
	   * It's a one-by-one matrix, so it has either 0 or 1 elements.  If
	   * it has 1 element, then that's the one we want.
	   */
	  if (m->nn > 0)
	    {
	      switch (m->type)
		{
		case integer:
		  s = int_to_scalar (m->a.integer[0]);
		  break;
		case real:
		  s = real_to_scalar (m->a.real[0]);
		  break;
		case complex:
		  s = complex_to_scalar (m->a.complex[0]);
		  break;
		case character:
		  s = char_to_scalar (dup_char (m->a.character[0]));
		  break;
		default:
		  BAD_TYPE (m->type);
		  delete_matrix (m);
		  raise_exception ();
		}
	    }
	  else
	    {
	      s = make_scalar (m->type);
	    }
	  break;
	case sparse_upper:
	  if (m->d.integer != NULL)
	    {
	      switch (m->type)
		{
		case integer:
		  s = int_to_scalar (m->d.integer[0]);
		  break;
		case real:
		  s = real_to_scalar (m->d.real[0]);
		  break;
		case complex:
		  s = complex_to_scalar (m->d.complex[0]);
		  break;
		case character:
		  s = char_to_scalar (dup_char (m->d.character[0]));
		  break;
		default:
		  BAD_TYPE (m->type);
		  delete_matrix (m);
		  raise_exception ();
		}
	    }
	  else
	    {
	      s = make_scalar (m->type);
	    }
	  break;
	default:
	  BAD_DENSITY (m->density);
	  delete_matrix (m);
	  raise_exception ();
	}
    }

  if (m->stuff)
    ((SCALAR *) s)->stuff = (TABLE *) copy_table (m->stuff);

  delete_matrix (m);
  return (s);
}

ENTITY *
dup_matrix (MATRIX *old)
{
  /*
   * In Algae, duplicating an entity means that we really make another
   * copy of it in memory (not just incrementing its reference count).
   */

  MATRIX *new;
  int i;

  EASSERT (old, matrix, 0);

  if (old->entity.ref_count == 1)
    return (ENT (old));

  new = (MATRIX *) form_matrix (old->nr, old->nc, old->type, old->density);

  new->symmetry = old->symmetry;

  if (old->rid)
    new->rid = copy_entity (old->rid);
  if (old->cid)
    new->cid = copy_entity (old->cid);

  new->nn = old->nn;

  if (old->ia)
    new->ia = (int *) dup_mem (old->ia, (old->nr + 1) * sizeof (int));

  if (old->ja)
    new->ja = (int *) dup_mem (old->ja, old->nn * sizeof (int));

  if (old->a.integer)
    {
      if (new->a.integer == NULL)
	new->a.integer = (int *) E_MALLOC (old->nn, old->type);
      memcpy (new->a.integer, old->a.integer,
	      old->nn * type_size[old->type]);
      if (old->type == character)
	for (i = 0; i < old->nn; i++)
	  new->a.character[i] = dup_char (new->a.character[i]);
    }

  if (old->d.integer)
    {
      new->d.integer = (int *) dup_mem (old->d.integer,
					old->nr * type_size[old->type]);
      if (old->type == character)
	for (i = 0; i < old->nr; i++)
	  new->d.character[i] = dup_char (new->d.character[i]);
    }

  if (debug_level > 1)
    inform ("Matrix created:  %x.", new);

  delete_matrix (old);
  return (ENT (new));
}

ENTITY *
matrix_to_vector (MATRIX *m)
{
  /* Convert matrix `m' to a vector (if it's a single row or column). */

  VECTOR *v;
  int i, j;

  EASSERT (m, matrix, 0);

  /* Make sure it's a single row or column. */

  if (m->nr != 1 && m->nc != 1)
    {
      fail ("Can't convert a %d by %d matrix into a vector.",
            m->nr, m->nc);
      delete_matrix (m);
      raise_exception ();
    }

  if (m->density == sparse_upper)
    {

      /* If it's sparse_upper, then it has to be a 1x1 matrix. */

      assert (m->nr == 1 && m->nc == 1);
      v = (VECTOR *) make_vector (1, m->type, dense);
      if (m->d.ptr != NULL)
	memcpy (v->a.ptr, m->d.ptr,
		type_size[m->type]);
      if (v->type == character)
	v->a.character[0] = dup_char (v->a.character[0]);

    }
  else if (m->entity.ref_count == 1)
    {

      /* If it has a ref_count of 1, we'll take a shortcut. */

      v = (VECTOR *) CALLOC (1, sizeof (VECTOR));
      if (debug_level > 1)
	inform ("Vector created:  %x.", v);
      v->entity.ref_count = 1;
      v->entity.class = vector;
      v->type = m->type;
      v->order = m->order;
      v->density = m->density;
      v->nn = m->nn;

      /*
       * For sparse, if `m' was a row, then `ja' is just what we want.
       * Otherwise, we have to fill in `ja' with the element numbers.
       */

      v->ja = m->ja;
      if (v->ja && m->nr != 1)
	{
	  for (i = j = 0; i < m->nr; i++)
	    {
	      if (m->ia[i + 1] > m->ia[i])
		v->ja[j++] = i + 1;
	    }
	}
      m->ja = NULL;

      /*
       * We don't have to worry about cleaning up `m->ia'---it's done
       * in `delete_matrix' below.
       */

      v->a = m->a;
      m->a.ptr = NULL;

    }
  else
    {

      /* Here's the general case. */

      v = (VECTOR *) form_vector ((m->nr != 1) ? m->nr :
				  m->nc, m->type, m->density);
      if (m->nn > 0)
	{
	  v->nn = m->nn;
	  if (m->density == sparse)
	    {
	      if (m->nr == 1)
		{
		  v->ja = (int *) dup_mem (m->ja, m->nn * sizeof (int));
		}
	      else
		{
		  v->ja = (int *) MALLOC (v->nn * sizeof (int));
		  for (i = j = 0; i < m->nr; i++)
		    {
		      if (m->ia[i + 1] > m->ia[i])
			v->ja[j++] = i + 1;
		    }
		}
	      v->a.ptr = dup_mem (m->a.ptr, m->nn * type_size[m->type]);
	    }
	  else
	    {
	      assert (m->density == dense);
	      memcpy (v->a.ptr, m->a.ptr, m->nn * type_size[m->type]);
	    }
	  if (v->type == character)
	    {
	      for (i = 0; i < v->nn; i++)
		v->a.character[i] = dup_char (v->a.character[i]);
	    }
	}
    }

  if (m->nr != 1)
    {
      v->ne = m->nr;
      if (m->rid != NULL)
	v->eid = copy_entity (m->rid);
    }
  else
    {
      v->ne = m->nc;
      if (m->cid != NULL)
	v->eid = copy_entity (m->cid);
    }

  if (m->stuff)
    v->stuff = (TABLE *) copy_table (m->stuff);

  delete_matrix (m);
  return (apt_vector (v));
}

ENTITY *
vector_to_matrix (VECTOR *v)
{
  /* Convert vector `v' to a matrix (a single row). */

  MATRIX *m;
  int i;

  EASSERT (v, vector, 0);

  if (v->entity.ref_count == 1)
    {
      m = (MATRIX *) CALLOC (1, sizeof (MATRIX));
      if (debug_level > 1)
	inform ("Matrix created:  %x.", m);
      m->entity.ref_count = 1;
      m->entity.class = matrix;
      m->symmetry = (v->ne == 1) ? symmetric : general;
      m->type = v->type;
      m->order = v->order;
      m->density = v->density;
      if (v->eid != NULL)
	m->cid = copy_entity (v->eid);
      m->nr = 1;
      m->nc = v->ne;
      m->nn = v->nn;
      if (v->density == sparse && v->nn > 0)
	{
	  m->ia = (int *) CALLOC (2, sizeof (int));
	  m->ia[0] = 1;
	  m->ia[1] = 1 + m->nn;
	}
      m->ja = v->ja;
      v->ja = NULL;
      m->a = v->a;
      v->a.ptr = NULL;
    }
  else
    {
      m = (MATRIX *) make_matrix (1, v->ne, v->type, v->density);
      m->symmetry = (v->ne == 1) ? symmetric : general;
      if (v->eid != NULL)
	m->cid = copy_entity (v->eid);
      if (v->nn > 0)
	{
	  m->nn = v->nn;
	  if (v->density == sparse)
	    {
	      m->ia = (int *) CALLOC (2, sizeof (int));
	      m->ia[0] = 1;
	      m->ia[1] = 1 + m->nn;
	      m->ja = (int *) dup_mem (v->ja, m->nn * sizeof (int));
	      m->a.ptr = dup_mem (v->a.ptr, m->nn * type_size[v->type]);
	    }
	  else
	    {
	      memcpy (m->a.ptr, v->a.ptr, m->nn * type_size[v->type]);
	    }
	  if (m->type == character)
	    {
	      for (i = 0; i < m->nn; i++)
		m->a.character[i] = dup_char (m->a.character[i]);
	    }
	}
    }

  if (v->stuff)
    m->stuff = (TABLE *) copy_table (v->stuff);

  delete_vector (v);
  return (apt_matrix (m));
}

void
free_matrix (MATRIX *p)
{
  /* Destroy the matrix, and free up the space it occupied. */

  int i;

  assert (p->entity.ref_count == 0);

  delete_table (p->stuff);
  delete_entity (p->rid);
  delete_entity (p->cid);

  if (p->ia)
    FREE (p->ia);
  if (p->ja)
    FREE (p->ja);

  if (p->a.ptr)
    {

      /*
       * For character type, the understanding is that the pointers are
       * all valid (and we can call FREE_CHAR on them, provided that
       * p->a.character[0] is not NULL.
       */

      if (p->type == character && p->a.character[0])
	{
	  for (i = 0; i < p->nn; i++)
	    FREE_CHAR (p->a.character[i]);
	}

      FREE (p->a.ptr);
    }

  if (p->d.ptr)
    {

      /* Same as above for character type, but with p->d.character[0]. */

      if (p->type == character && p->d.character[0])
	{
	  for (i = 0; i < p->nr; i++)
	    FREE_CHAR (p->d.character[i]);
	}

      FREE (p->d.integer);
    }

  /* Just to make it harder to use it again inadvertently. */

  p->entity.class = undefined_class;
  FREE (p);
}

void
DB_delete_matrix (MATRIX *p, char *file, int line)
{
  /*
   * This is the DEBUG version of `delete_matrix'.  It decrements
   * the matrix's reference count, and frees it if it is unreferenced.
   */

  if (p)
    {

      if (--p->entity.ref_count < 0)
	{
	  wipeout ("A matrix's \"ref_count\" went below zero:  %s, %d.",
		   file, line);
	}

      if (p->entity.ref_count >= 1000 || debug_level > 1)
	{
	  inform ("matrix \"ref_count\" decrement:  %x, %d, %s, %d.",
		  p, p->entity.ref_count, file, line);
	}

      if (p->entity.ref_count == 0)
	free_matrix (p);

    }
}

int
put_matrix (MATRIX *s, FILE *stream, struct ent_node *ent_tree)
{
  /* Write matrix `s' out in binary form to file `stream'. */

  int size, i;

  EASSERT (s, matrix, 0);

  assert (s->order == ordered);	/* unordered is obsolete */

  if (!WRITE_INT (&s->type, stream) ||
      !WRITE_INT (&s->density, stream) ||
      !WRITE_INT (&s->symmetry, stream) ||
      !WRITE_INT (&s->nr, stream) ||
      !WRITE_INT (&s->nc, stream) ||
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
	      if (!WRITE_INT (&size, stream))
		goto err;
	      if (size > 0 &&
		  fwrite (s->a.character[i], 1, size, stream) <
		  (size_t) size)
		{
		  WRITE_WARN (stream);
		  goto err;
		}
	    }
	  break;

	default:
	  BAD_TYPE (s->type);
	  delete_matrix (s);
	  raise_exception ();
	}

    }
  else
    {

      i = 0;			/* no values */
      if (!WRITE_INT (&i, stream))
	goto err;

    }

  if (s->d.ptr)
    {

      i = 1;			/* diagonal values follow */
      if (!WRITE_INT (&i, stream))
	goto err;

      switch (s->type)
	{

	case integer:
	  if (!WRITE_INTS (s->d.integer, s->nr, stream))
	    goto err;
	  break;

	case real:
	  if (!WRITE_DOUBLES (s->d.real, s->nr, stream))
	    goto err;
	  break;

	case complex:
	  if (!WRITE_DOUBLES (s->d.real, 2 * s->nr, stream))
	    goto err;
	  break;

	case character:
	  for (i = 0; i < s->nr; i++)
	    {
	      size = strlen (s->d.character[i]);
	      if (!WRITE_INT (&size, stream))
		goto err;
	      if (size > 0 &&
		  fwrite (s->d.character[i], 1, size, stream) <
		  (size_t) size)
		{
		  WRITE_WARN (stream);
		  goto err;
		}
	    }
	  break;
	}

    }
  else
    {

      i = 0;			/* no diagonal values */
      if (!WRITE_INT (&i, stream))
	goto err;

    }

  if (s->ia)
    {
      i = 1;			/* ia follows */
      if (!WRITE_INT (&i, stream) ||
	  !WRITE_INTS (s->ia, s->nr + 1, stream))
	goto err;
    }
  else
    {
      i = 0;			/* no ia */
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

  if (s->rid)
    {
      i = 1;			/* rid follows */
      if (!WRITE_INT (&i, stream) ||
	  !put_entity (copy_entity (s->rid), stream, ent_tree))
	goto err;
    }
  else
    {
      i = 0;			/* no rid */
      if (!WRITE_INT (&i, stream))
	goto err;
    }

  if (s->cid)
    {
      i = 1;			/* cid follows */
      if (!WRITE_INT (&i, stream) ||
	  !put_entity (copy_entity (s->cid), stream, ent_tree))
	goto err;
    }
  else
    {
      i = 0;			/* no cid */
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

  delete_matrix (s);
  return 1;

err:
  delete_matrix (s);
  return 0;
}

ENTITY *
get_matrix (FILE *stream, int ver)
{
  /* Read a vector from binary file `stream'. */

  MATRIX *s;
  int size, i;
  static char *warn_msg = "Invalid matrix in file.";

  s = (MATRIX *) CALLOC (1, sizeof (MATRIX));
  s->entity.ref_count = 1;
  s->entity.class = matrix;
  s->order = ordered;

  if (!READ_INT (&s->type, stream) ||
      !READ_INT (&s->density, stream) ||
      !READ_INT (&s->symmetry, stream) ||
      !READ_INT (&s->nr, stream) ||
      !READ_INT (&s->nc, stream) ||
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
	  s->a.real = MALLOC (s->nn * sizeof (REAL));
	  if (!READ_DOUBLES (s->a.real, s->nn, stream))
	    goto err;
	  break;

	case complex:
	  s->a.real = MALLOC (2 * s->nn * sizeof (REAL));
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
    goto err;			/* diagonal values follow? */

  if (i)
    {

      if (s->nr < 1)
	{
	  warn (warn_msg);
	  goto err;
	}

      switch (s->type)
	{

	case integer:
	  s->d.integer = MALLOC (s->nr * sizeof (int));
	  if (!READ_INTS (s->d.integer, s->nr, stream))
	    goto err;
	  break;

	case real:
	  s->d.real = MALLOC (s->nr * sizeof (REAL));
	  if (!READ_DOUBLES (s->d.real, s->nr, stream))
	    goto err;
	  break;

	case complex:
	  s->d.real = MALLOC (2 * s->nr * sizeof (REAL));
	  if (!READ_DOUBLES (s->d.real, 2 * s->nr, stream))
	    goto err;
	  break;

	case character:
	  s->d.character = MALLOC (s->nr * sizeof (char *));
	  for (i = 0; i < s->nr; i++)
	    s->d.character[i] = NULL_string;
	  for (i = 0; i < s->nr; i++)
	    {
	      if (!READ_INT (&size, stream))
		goto err;
	      if (size > 0)
		{
		  s->d.character[i] = (char *) MALLOC (size + 1);
		  if (fread (s->d.character[i], 1, size, stream) <
		      (size_t) size)
		    {
		      READ_WARN (stream);
		      goto err;
		    }
		  s->d.character[i][size] = '\0';
		}
	    }
	  break;

	}

    }
  else
    {
      s->d.ptr = NULL;
    }

  if (!READ_INT (&i, stream))
    goto err;			/* ia follows? */
  if (i)
    {
      s->ia = (int *) MALLOC ((s->nr + 1) * sizeof (int));
      if (!READ_INTS (s->ia, s->nr + 1, stream))
	goto err;
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
    goto err;			/* rid follows */
  if (i && !(s->rid = get_entity (stream)))
    goto err;

  if (!READ_INT (&i, stream))
    goto err;			/* cid follows */
  if (i && !(s->cid = get_entity (stream)))
    goto err;

  if (!READ_INT (&i, stream))
    goto err;			/* stuff follows */
  if (i && !(s->stuff = (TABLE *)
	     (ver ? get_entity (stream) : get_table (stream, ver))))
    goto err;

  if (!ok_entity (ENT (s)))
    goto err;

  return apt_matrix (s);

err:
  delete_matrix (s);
  return NULL;
}

ENTITY *
gift_wrap_matrix (int nr, int nc, TYPE type, void *data)
{
  /*
   * This function creates a dense MATRIX structure to point to
   * the given array.
   */

  MATRIX *m;

  assert (nr >= 0);
  assert (nc >= 0);

  m = (MATRIX *) CALLOC (1, sizeof (MATRIX));

  m->entity.ref_count = 1;
  m->entity.class = matrix;

  m->symmetry = general;

  m->type = type;
  m->order = ordered;
  m->density = dense;
  m->nr = nr;
  m->nc = nc;

  if ((double) nr * nc > INT_MAX)
    {
      fail ("Integer overflow.  Matrix dimensions too big.");
      raise_exception ();
    }
  m->nn = nr * nc;
  m->a.ptr = (m->nn) ? data : NULL;

  if (debug_level > 1)
    inform ("Matrix created:  %x.", m);

  return (ENT (m));
}
