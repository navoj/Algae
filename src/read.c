/*
   read.c -- Algae's `read' function.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: read.c,v 1.7 2003/08/01 04:57:48 ksh Exp $";

#include "read.h"
#include "entity.h"
#include "datum.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "file_io.h"
#include "cast.h"
#include "transpose.h"
#include "dense.h"
#include "apply.h"

static char read_buf[READ_BUF_SIZE];

#define COMMENT		'#'	/* `readnum' takes this as a comment */
#define SPLIT_BUF_SIZE	20	/* size of the split buffer */

DATUM *this_many_read;

ENTITY *
bi_readnum (int n, ENTITY * volatile v, ENTITY *f)
{
  /*
   * This function reads numeric data from a file.  The vector `v'
   * specifies the form of the output.  If `v' is NULL or has zero
   * length, a scalar is returned.  If `v' has length one, then
   * a vector with `v[1]' elements is returned.  If `v' has two
   * elements, then a matrix is returned with `v[1]' rows and `v[2]'
   * columns.  The values are read from file `f', or from stdin
   * if `f' is NULL.  The global variable `this_many_read' has the
   * number of values read by the last call to `bi_read'; it is
   * accessible to the user through the variable `$read'.
   */

  FILE * volatile stream;

  WITH_HANDLING
  {
    stream = f ? find_file (entity_to_string (EAT (f)),
			    FILE_INPUT) : stdin;
    if (!stream)
      raise_exception ();
  }
  ON_EXCEPTION
  {
    delete_2_entities (f, v);
  }
  END_EXCEPTION;

  if (v)
    v = vector_entity (v);
  return (read_vector (stream, (VECTOR *) v));
}

ENTITY *
read_vector (FILE *stream, VECTOR *v)
{
  ENTITY *d = NULL;
  REAL * volatile r;
  int cnt;

  WITH_HANDLING
  {

    if (v)
      {
	v = (VECTOR *) cast_vector (
		     (VECTOR *) dense_vector ((VECTOR *) EAT (v)), integer);
	switch (v->ne)
	  {
	  case 0:
	    d = make_scalar (real);
	    cnt = 1;
	    r = &((SCALAR *) d)->v.real;
	    break;
	  case 1:
	    cnt = v->a.integer[0];
	    if (cnt < 0)
	      {
		fail ("Specified vector dimension is negative.");
		raise_exception ();
	      }
	    d = make_vector (cnt, real, dense);
	    r = ((VECTOR *) d)->a.real;
	    break;
	  case 2:
	    if (v->a.integer[0] < 0 || v->a.integer[1] < 0)
	      {
		fail ("Specified matrix dimension is negative.");
		raise_exception ();
	      }
	    d = make_matrix (v->a.integer[1], v->a.integer[0],
			     real, dense);
	    ((MATRIX *) d)->symmetry = general;
	    cnt = ((MATRIX *) d)->nn;
	    r = ((MATRIX *) d)->a.real;
	    break;
	  default:
	    fail ("Invalid dimension (%d) for \"readnum\".", v->ne);
	    raise_exception ();
	  }
      }
    else
      {
	d = make_scalar (real);
	cnt = 1;
	r = &((SCALAR *) d)->v.real;
      }

    /* We keep a count of the numbers read in `this_many_read'. */

    delete_datum (this_many_read);
    IVAL (this_many_read) = read_numbers (stream, cnt, r);
    this_many_read->type = D_INT;

    /*
     * Matrices are stored by columns, but we just read the data by
     * rows, so just transpose them.
     */

    if (d->class == matrix)
      d = transpose_matrix ((MATRIX *) EAT (d));

  }
  ON_EXCEPTION
  {
    delete_entity (d);
    delete_vector (v);
  }
  END_EXCEPTION;

  delete_vector (v);
  return (d);
}

/* strips floating point numbers out of a file */

int
read_numbers (FILE *fp, int cnt, double *dvec)
{
  char xbuff[128];
  char *bp;
  int number_flag, dot_flag;

  int n, c;

  for (n = cnt, number_flag = 0; n > 0; n--)
    {

    restart:

      dot_flag = 0;
      bp = xbuff;

    reswitch:

      switch (c = getc (fp))
	{

	default:

	  goto reswitch;

	case COMMENT:

	  while (1)
	    if ((c = getc (fp)) == '\n')
	      goto reswitch;
	    else if (c == EOF)
	      break;		/* and fall thru */

	case EOF:

	  if (ferror (fp) && errno == EINTR)
	    {
	      clearerr (fp);
	      goto reswitch;
	    }
	  return cnt - n;

	case '.':

	  dot_flag = 1;
	  /* fall thru */

	case '-':
	case '+':
	  *bp++ = c;
	  goto get_number;


	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':

	  number_flag = 1;
	  *bp++ = c;

	get_number:

	  while (1)
	    {
	      c = getc (fp);
	      if (c == EOF && ferror (fp) && errno == EINTR)
		{
		  clearerr (fp);
		  continue;
		}
	      if (!isdigit (c)) break;
	      *bp++ = c;
	      number_flag = 1;
	    }

	  if (!dot_flag && c == '.')
	    {
	      *bp++ = c;
	      while (1)
		{
		  c = getc (fp);
		  if (c == EOF && ferror (fp) && errno == EINTR)
		    {
		      clearerr (fp);
		      continue;
		    }
		  if (!isdigit (c)) break;
		  *bp++ = c;
		  number_flag = 1;
		}
	    }

	  if (!number_flag)
	    {
	      ungetc (c, fp);
	      goto restart;
	    }

	  /* get the exponent */

	  if (c == 'e' || c == 'E')
	    {
	      *bp++ = c;
	      while ((c = getc (fp)) == EOF && ferror (fp) && errno == EINTR)
		clearerr (fp);
	      if (c == '-' || c == '+')
		*bp++ = c;
	      else
		ungetc (c, fp);

	      while (1)
		{
		  c = getc (fp);
		  if (c == EOF && ferror (fp) && errno == EINTR)
		    {
		      clearerr (fp);
		      continue;
		    }
		  if (!isdigit (c)) break;
		  *bp++ = c;
		}
	    }

	  ungetc (c, fp);
	  if (bp - xbuff > 127)
	    {
	      xbuff[127] = 0;
	      fail (" Numeric input is too long: %s.", xbuff);
	      algae_exit (1);
	    }
	  *bp = 0;
	  errno = 0;
	  *dvec++ = atof (xbuff);
	  if (errno)
	    {
	      fail ("Value out of range: %s.", xbuff);
	      raise_exception ();
	    }

	  number_flag = 0;	/* reset */
	}
    }
  return cnt;
}

ENTITY *
bi_read (int n, ENTITY *f)
{
  /* Read a line from file `f'. */

  FILE *stream;
  char *c;
  int len, tlen;

  /* Open the file. */

  stream = f ? find_file (entity_to_string (f), FILE_INPUT) : stdin;
  if (!stream)
    raise_exception ();

  /* Read into buffer. */

  if (!fgets (read_buf, READ_BUF_SIZE, stream))
    return NULL;

  if ((len = strlen (read_buf)) == 0)
    return char_to_scalar (NULL_string);

  /* Remove trailing newline. */

  if (read_buf[len - 1] == '\n')
    read_buf[--len] = '\0';

  /* Tuck the string away. */

  c = MALLOC (len + 1);
  memcpy (c, read_buf, len + 1);

  /* Unless we filled the buffer, we're done. */

  if (len < READ_BUF_SIZE - 1)
    return char_to_scalar (c);

  /* OK, there's more waiting for us. */

  tlen = len;

  do
    {

      /* Read more into buffer. */

      if (!fgets (read_buf, READ_BUF_SIZE, stream) ||
	  (len = strlen (read_buf)) == 0)
	break;

      /* Remove trailing newline. */

      if (read_buf[len - 1] == '\n')
	read_buf[--len] = '\0';

      /* Add this to the string we've already got. */

      c = REALLOC (c, len + tlen + 1);
      memcpy (c + tlen, read_buf, len + 1);

      tlen += len;

    }
  while (len == READ_BUF_SIZE - 1);

  return char_to_scalar (c);

}

ENTITY *
bi_split (int na, ENTITY *str, ENTITY *sep)
{
  /* Split a character string into fields. */

  VECTOR *vec;
  char *w = " \t\n";		/* the default separators */
  char *s, *p;
  char **c, **v;
  char *split_buf[SPLIT_BUF_SIZE];
  int len;
  int n = 0;

  EASSERT (str, 0, 0);

  /* Get the string. */

  str = cast_scalar ((SCALAR *) scalar_entity (str), character);
  str = dup_scalar ((SCALAR *) str);
  s = ((SCALAR *) str)->v.character;
  len = strlen (s);

  /* Get the separators, if given. */

  if (sep)
    {
      sep = cast_scalar ((SCALAR *) scalar_entity (sep), character);
      w = ((SCALAR *) sep)->v.character;
    }

  /* Read all tokens into split buffer, expanding it if necessary. */

  p = s;
  v = c = split_buf;
  while (s = strtok (p, w))
    {
      if (n++ == SPLIT_BUF_SIZE)
	{
	  v = MALLOC (sizeof (char *) * ((len + 1) / 2));
	  memcpy (v, split_buf, sizeof (char *) * SPLIT_BUF_SIZE);
	  c = v + SPLIT_BUF_SIZE;
	}
      p = NULL;
      *c++ = dup_char (s);
    }

  /* Copy pointers to character vector. */

  vec = (VECTOR *) form_vector (n, character, dense);
  memcpy (vec->a.character, v, sizeof (char *) * n);

  /* Free expanded buffer, if needed. */

  if (v != split_buf)
    FREE (v);
  delete_2_scalars ((SCALAR *) str, (SCALAR *) sep);

  /* Return the new vector. */

  return ENT (vec);
}

ENTITY *
bi_substr (int n, ENTITY *s, ENTITY *i, ENTITY *j)
{
  /*
   * Return the substring of string `s', starting at the `i'th character,
   * of length `j'.  If `j' is omitted, then the suffix of `s', starting
   * at the `i'th character, is returned.
   */

  int start, end;
  char *c;
  ENTITY * volatile r;

  WITH_HANDLING
    {
      s = cast_scalar ((SCALAR *) scalar_entity (EAT (s)), character);
      i = cast_scalar ((SCALAR *) scalar_entity (EAT (i)), integer);

      start = ((SCALAR *)i)->v.integer;

      if (start < 1)
	{
	  fail ("substr: Invalid starting index (%d).", start);
	  raise_exception ();
	}

      c = ((SCALAR *)s)->v.character;
      end = strlen (c);
      if (start > end) start = end + 1;
      c += start - 1;

      if (j)
	{
	  int len;
	  char *m;
	  j = cast_scalar ((SCALAR *) scalar_entity (EAT (j)), integer);
	  len = ((SCALAR *)j)->v.integer;
	  if (len < 0)
	    {
	      fail ("substr: Invalid length (%d).", len);
	      raise_exception ();
	    }
	  if (len > end-start+1) len = end-start+1;
	  if (len)
	    {
	      m = memcpy (MALLOC (len+1), c, len);
	      m[len] = '\0';
	    }
	  else
	    {
	      m = NULL_string;
	    }
	  r = char_to_scalar (m);
	}
      else
	{
	  r = char_to_scalar (dup_char (c));
	}
    }
  ON_EXCEPTION
    {
      delete_3_entities (s, i, j);
    }
  END_EXCEPTION;

  delete_3_entities (s, i, j);

  return r;
}

ENTITY *
bi_atof (ENTITY *p)
{
  /* Convert character string to a real. */

  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return atof_scalar ((SCALAR *) p);
    case vector:
      return atof_vector ((VECTOR *) p);
    case matrix:
      return atof_matrix ((MATRIX *) p);
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
atof_scalar (SCALAR *s)
{
  SCALAR *r;

  EASSERT (s, scalar, 0);

  s = (SCALAR *) cast_scalar (s, character);
  r = (SCALAR *) make_scalar (real);

  errno = 0;
  r->v.real = atof (s->v.character);
  if (errno && r->v.real != 0.0)
    {
      delete_2_scalars (r, s);
      math_error ();
    }

  delete_scalar (s);
  return ENT (r);
}

ENTITY *
atof_vector (VECTOR *v)
{
  VECTOR *r;
  int i;

  EASSERT (v, vector, 0);

  v = (VECTOR *) cast_vector (v, character);
  r = (VECTOR *) form_vector (v->ne, real, v->density);
  if (v->eid) r->eid = copy_entity (v->eid);
  if (v->ja) r->ja = dup_mem (v->ja, v->nn * sizeof (int));

  r->nn = v->nn;
  if (v->a.character)
    {
      if (!r->a.real) r->a.real = E_MALLOC (r->nn, real);
      for (i=0; i<r->nn; i++)
        {
          errno = 0;
          r->a.real[i] = atof (v->a.character[i]);
          if (errno && r->a.real[i] != 0.0)
            {
              delete_2_vectors (r, v);
              math_error ();
            }
        }
    }

  delete_vector (v);
  return ENT (r);
}

ENTITY *
atof_matrix (MATRIX *m)
{
  MATRIX *r;
  int i;

  EASSERT (m, matrix, 0);

  m = (MATRIX *) cast_matrix (m, character);
  r = (MATRIX *) form_matrix (m->nr, m->nc, real, m->density);
  r->symmetry = m->symmetry;
  if (m->rid) r->rid = copy_entity (m->rid);
  if (m->cid) r->cid = copy_entity (m->cid);
  if (m->ia) r->ia = dup_mem (m->ia, (m->nr+1) * sizeof (int));
  if (m->ja) r->ja = dup_mem (m->ja, m->nn * sizeof (int));

  r->nn = m->nn;
  if (m->a.character)
    {
      if (!r->a.real) r->a.real = E_MALLOC (r->nn, real);
      for (i=0; i<r->nn; i++)
        {
          errno = 0;
          r->a.real[i] = atof (m->a.character[i]);
          if (errno && r->a.real[i] != 0.0)
            {
              delete_2_matrices (r, m);
              math_error ();
            }
        }
    }

  delete_matrix (m);
  return ENT (r);
}

ENTITY *
bi_dice (ENTITY *s)
{
  /*
   * This function takes a character string and "dices" it up into
   * a vector of individual characters.
   */

  VECTOR *v;
  int i;
  char *c;

  s = cast_scalar ((SCALAR *) scalar_entity (s), character);
  c = ((SCALAR *) s)->v.character;

  v = (VECTOR *) form_vector (strlen (c), character, dense);

  for (i = 0; i < v->ne; i++)
    {
      v->a.character[i] = MALLOC (2);
      v->a.character[i][0] = c[i];
      v->a.character[i][1] = '\0';
    }

  delete_scalar ((SCALAR *) s);
  return ENT (v);
}

char *
tolower_character (char *s)
{
  char *c;
  
  for (c=s; *c; c++) *c = tolower (*c);

  return s;
}

ENTITY *
bi_tolower (ENTITY *e)
{
  e = cast_entity (e, character);

  switch (e->class)
    {
    case scalar:
      e = dup_scalar ((SCALAR *) e);
      ((SCALAR *)e)->v.character =
        tolower_character (((SCALAR *)e)->v.character);
      return e;
    case vector:
      return apply_vector_character_character (tolower_character,
                                               (VECTOR *) e);
    case matrix:
      return apply_matrix_character_character (tolower_character,
                                               (MATRIX *) e);
    default:
      BAD_CLASS (e->class);
      delete_entity (e);
      raise_exception ();
    }
}

char *
toupper_character (char *s)
{
  char *c;
  
  for (c=s; *c; c++) *c = toupper (*c);

  return s;
}

ENTITY *
bi_toupper (ENTITY *e)
{
  e = cast_entity (e, character);

  switch (e->class)
    {
    case scalar:
      e = dup_scalar ((SCALAR *) e);
      ((SCALAR *)e)->v.character =
        toupper_character (((SCALAR *)e)->v.character);
      return e;
    case vector:
      return apply_vector_character_character (toupper_character,
                                               (VECTOR *) e);
    case matrix:
      return apply_matrix_character_character (toupper_character,
                                               (MATRIX *) e);
    default:
      BAD_CLASS (e->class);
      delete_entity (e);
      raise_exception ();
    }
}

ENTITY *
bi_char (ENTITY *e)
{
  int i;
  char *s;
  VECTOR *v = (VECTOR *) vector_entity (e);

  v = (VECTOR *) cast_vector (v, integer);
  v = (VECTOR *) dense_vector (v);

  s = MALLOC (v->ne+1);
  for (i=0; i<v->ne; i++) s[i] = (char) v->a.integer[i];
  s[i] = '\0';

  delete_vector (v);
  return char_to_scalar (s);
}
