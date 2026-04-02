/*
 * get.c -- Read entities from files.
 *
 * Copyright (C) 1994-97  K. Scott Hunziker.
 * Copyright (C) 1990-94  The Boeing Company.
 *
 * See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: get.c,v 1.3 1997/06/06 03:47:52 ksh Exp $";

#include <stdio.h>
#include "get.h"
#include "put.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "function.h"
#include "file_io.h"

/* Keep a list of pointers to the entities we've read. */

static ENTITY **ent_list;
static int ent_length;		/* length of ent_list */
static int seq_num;

ENTITY *
bi_get (int n, ENTITY *fname)
{
  /*
   * This routine reads an entity from a file.  The file is in Algae's
   * "native" binary format, which is essentially a mirror of the C
   * structures.  Someday we'll have to formalize this...
   *
   * If no args are given (n==0), then we read from stdin.
   */

  ENTITY *ret;

  FILE *stream = fname ? find_file (entity_to_string (fname),
				    FILE_INPUT) : stdin;

  if (stream)
    {
      seq_num = 0;
      ent_length = 100;
      ent_list = MALLOC (ent_length * sizeof (ENTITY *));
      ret = get_entity (stream);
      FREE (ent_list);
    }
  else
    {
      ret = NULL;
    }

  return ret;
}

ENTITY *
get_entity (FILE *stream)
{
  /* Read an entity from `stream'.  (See `bi_get'.) */

  ENTITY *ret;
  CLASS c;
  char header[6];
  int ver, seq, this_seq;

  /* Check the header and version. */

  header[5] = '\0';
  if (fread (header, 1, 5, stream) < 5)
    {
      READ_WARN (stream);
      return NULL;
    }

  if (strcmp (header, "\177Alki"))
    {
      /* Maybe it's ancient? */

      if (header[0] == '\0' &&
	  header[1] == '\0' &&
	  header[2] == '\014' &&
	  header[3] == '\373')
	{
	  char s[3];
	  if (fread (s, 1, 3, stream) < 3)
	    {
	      READ_WARN (stream);
	      return NULL;
	    }
	  ver = 0;
	  c = (CLASS) s[2];
	}
      else
	{
	  warn ("Not a valid Algae binary file.");
	  return NULL;
	}
    }
  else
    {
      char v;
      if (fread (&v, 1, 1, stream) < 1)
	{
	  READ_WARN (stream);
	  return NULL;
	}

      ver = (int) v;
      if (ver > (char) FILE_FORMAT)
	{
	  warn ("Incompatible binary file version.");
	  return NULL;
	}

      /*
       * Read the reference tag.  If it's zero, read the entity and keep
       * its pointer on the entity list.  If the tag is not zero, then it's
       * a reference to the previously read entity with that sequence
       * number.
       */

      if (!READ_INT (&seq, stream))
	return NULL;
      if (seq < 0 || seq > seq_num)
	{
	  warn ("Invalid reference in binary file.");
	  return NULL;
	}
      if (seq)
	return copy_entity (ent_list[seq - 1]);

      if (!READ_INT (&c, stream))
	return NULL;
    }

  this_seq = seq_num++;

  if (c >= NUM_CLASS)
    {
      warn ("Invalid entity in file.");
      return NULL;
    }

  switch (c)
    {
    case scalar:
      ret = get_scalar (stream, ver);
      break;
    case vector:
      ret = get_vector (stream, ver);
      break;
    case matrix:
      ret = get_matrix (stream, ver);
      break;
    case table:
      ret = get_table (stream, ver);
      break;
    case function:
      ret = get_function (stream, ver);
      break;
    default:
      BAD_CLASS (c);
      return NULL;
    }

  if (seq_num >= ent_length)
    {
      ent_length += 100;
      ent_list = REALLOC (ent_list, ent_length * sizeof (ENTITY *));
    }

  return ent_list[this_seq] = ret;
}

size_t
fread_int (char *ptr, size_t n, FILE *stream)
{
  return fread (ptr, 4, n, stream);
}

size_t
fread_int_reverse (char *ptr, size_t n, FILE *stream)
{
  size_t w = 0;
  char t;

  assert (sizeof (int) == 4);
  assert (ptr != NULL);
  assert (n > 0);

  do
    {
      if (fread (ptr, 4, 1, stream) < 1)
        return w;
      t = ptr[0]; ptr[0] = ptr[3]; ptr[3] = t;
      t = ptr[1]; ptr[1] = ptr[2]; ptr[2] = t;
    }
  while (ptr += 4, ++w < n);

  return w;
}

size_t
fread_double (char *ptr, size_t n, FILE *stream)
{
  return fread (ptr, 8, n, stream);
}

size_t
fread_double_reverse (char *ptr, size_t n, FILE *stream)
{
  char t;
  size_t w = 0;

  assert (sizeof (double) == 8);
  assert (ptr != NULL);
  assert (n > 0);

  do
    {
      if (fread (ptr, 8, 1, stream) < 1)
	return w;
      t = ptr[0]; ptr[0] = ptr[7]; ptr[7] = t;
      t = ptr[1]; ptr[1] = ptr[6]; ptr[6] = t;
      t = ptr[2]; ptr[2] = ptr[5]; ptr[5] = t;
      t = ptr[3]; ptr[3] = ptr[4]; ptr[4] = t;
    }
  while (ptr += 8, ++w < n);

  return w;
}

#if BINARY_FORMAT == CRAY_FLOAT

static char *cray_format_error = "Error converting from IEEE to Cray format.";

size_t
fread_int_cray (char *ptr, size_t n, FILE *stream)
{
  int type = 1;
  int bitoff = 0;
  int num;
  char *buf = MALLOC (n * 4);

  num = fread (buf, 4, n, stream);
  if (IEG2CRAY (&type, &num, buf, &bitoff, ptr))
    {
      fail (cray_format_error);
      FREE (buf);
      raise_exception ();
    }

  FREE (buf);
  return num;
}

size_t
fread_double_cray (char *ptr, size_t n, FILE *stream)
{
  int type = 8;
  int bitoff = 0;
  int num;
  void *buf = MALLOC (n * 8);

  num = fread (buf, 8, n, stream);
  if (IEG2CRAY (&type, &num, buf, &bitoff, ptr))
    {
      fail (cray_format_error);
      FREE (buf);
      raise_exception ();
    }

  FREE (buf);
  return num;
}

size_t
fread_int_cray_reverse (char *ptr, size_t n, FILE *stream)
{
  int type = 1;
  int bitoff = 0;
  int num;
  char *buf = MALLOC (n * 4);

  num = fread (buf, 4, n, stream);
  reverse_words (buf, 4, n);
  if (IEG2CRAY (&type, &num, buf, &bitoff, ptr))
    {
      fail (cray_format_error);
      FREE (buf);
      raise_exception ();
    }

  FREE (buf);
  return num;
}

size_t
fread_double_cray_reverse (char *ptr, size_t n, FILE *stream)
{
  int type = 8;
  int bitoff = 0;
  int num;
  void *buf = MALLOC (n * 8);

  num = fread (buf, 8, n, stream);
  reverse_words (buf, 4, n);
  if (IEG2CRAY (&type, &num, buf, &bitoff, ptr))
    {
      fail (cray_format_error);
      FREE (buf);
      raise_exception ();
    }

  FREE (buf);
  return num;
}

#endif
