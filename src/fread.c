/*
 * fread.c -- Read and write binary files.
 *
 * Copyright (C) 1994-2004  K. Scott Hunziker.
 *
 * See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: fread.c,v 1.9 2004/02/11 21:04:33 ksh Exp $";

#include <stdio.h>

#include "fread.h"
#include "get.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "file_io.h"
#include "cast.h"
#include "dense.h"

static char *invalid_type_msg = "%s: Invalid type specification.";
static char *invalid_msg = "%s: Invalid %s combination (%s and %s).";
static char *detour_msg = "Sorry, can't do this type yet.";

static void PROTO (parse_types,
		   (struct data_type *dt, TABLE *type, char *caller));
static ENTITY * PROTO (fread_entity,
                       (FILE *stream, struct data_type *dt, int len));
static void PROTO (fwrite_entity,
                   (FILE *stream, struct data_type *dt, VECTOR *v));

/*
 * The types and qualifiers are named here, in the same order as in
 * the DATA_TYPE enumeration in "fread.h".  The last entry,
 * corresponding to `dt_default', must be NULL.
 */

static char *type_list[] =
{
  "char",
  "int",
  "float",
  "double",
  "long",
  "short",
  "signed",
  "unsigned",
  "big",
  "little",
  "ieee",
  NULL
};

static void
parse_types (struct data_type *dt, TABLE *type, char *caller)
{
  char **t;
  
  dt->base = dt->sgn = dt->size = dt->endian = dt->ieee = dt_default;

  if (type)
    {
      if (!accept_members ((TABLE *) copy_table (type), type_list))
	{
	  delete_table (type);
	  raise_exception ();
	}

      for (t=type_list; *t; t++)
	  if (exists_in_table ((TABLE *) copy_table (type), *t))
	    {
	      switch (t-type_list)
		{

		case dt_char:
		case dt_int:
		case dt_float:
		case dt_double:

		  if (dt->base != dt_default)
		    {
		      fail (invalid_msg, caller,
			    "type", type_list[dt->base], *t);
		      raise_exception ();
		    }

		  dt->base = t-type_list;
		  break;

		case dt_long:
		case dt_short:

		  if (dt->size != dt_default)
		    {
		      fail (invalid_msg, caller,
			    "qualifier", type_list[dt->size], *t);
		      raise_exception ();
		    }

		  dt->size = t-type_list;
		  break;

		case dt_signed:
		case dt_unsigned:
		  
		  if (dt->sgn != dt_default)
		    {
		      fail (invalid_msg, caller,
			    "qualifier", type_list[dt->sgn], *t);
		      raise_exception ();
		    }

		  dt->sgn = t-type_list;
		  break;

		case dt_big:
		case dt_little:
		  
		  if (dt->endian != dt_default)
		    {
		      fail (invalid_msg, caller,
			    "qualifier", type_list[dt->endian], *t);
		      raise_exception ();
		    }

		  dt->endian = t-type_list;
		  break;

		case dt_ieee:

		  dt->ieee = t-type_list;
		  break;

		default:

		  wipeout ("parse_types");
		}
	    }
    }

  /* try to infer type */
      
  if (dt->base == dt_default)
    {
      if (dt->sgn != dt_default || dt->size != dt_default)
        dt->base = dt_int;
      else
        dt->base = dt_double;
    }

  delete_table (type);
}

ENTITY *
bi_fread (int n, ENTITY *fname, ENTITY *length, ENTITY *type)
{
  /*
   * This routine reads data from a binary file into a numeric array.
   *
   * The `fname' arg is the file name; if it's NULL we use stdin.
   *
   * The `length' arg gives the number of items to read.  If it's NULL,
   * the entire file is read.  Otherwise, it should be a non-negative
   * integer.
   *
   * The `type' arg is a table; its member names specify the
   * type of the data to read and, implicitly, the type of the
   * returned array.  These names may include any of the C types
   * and qualifiers "char", "int", "float", "double", "signed",
   * "unsigned", "short", and "long".  Combinations that are
   * disallowed in C (such as "unsigned double") are disallowed here,
   * too.  In addition, the qualifiers "big", "little", and "ieee" are
   * accepted.  If `type' is NULL or empty, "double" is assumed.
   */

  int len;
  struct data_type dt;
  ENTITY * volatile ret;

  WITH_HANDLING
    {
      FILE *stream = fname ? find_file (entity_to_string (EAT (fname)),
                                        FILE_INPUT) : stdin;

      if (stream)
        {
          if (type && type->class != table)
	    {
	      fail ("fread: Third arg is a %s; must be a table.",
		    class_string[type->class]);
	      raise_exception ();
	    }

          parse_types (&dt, EAT (type), "fread");

          if (length)
            {
              len = entity_to_int (EAT (length));
              if ( len < 0 )
                {
                  fail ("fread: Length is negative.");
                  raise_exception ();
                }
            }
          else
            {
	      long offset_current, long_len;
	      long offset_end = 0;
	      int i;

	      offset_current = ftell (stream);

	      if (fseek (stream, offset_end, SEEK_END))
		{
		  fail ("fread: Improper seek.");
		  raise_exception ();
		}
	      offset_end = ftell (stream);

	      switch (dt.base)
		{
		case dt_char:
		  long_len = (int) (offset_end-offset_current);
		  break;
		case dt_int:
		  switch (dt.size)
		    {
		    case dt_short:
		      long_len = (int) (offset_end-offset_current) /
			sizeof (short);
		      break;
		    case dt_long:
		      long_len = (int) (offset_end-offset_current) /
			sizeof (long);
		      break;
		    default:
		      long_len = (int) (offset_end-offset_current) /
			sizeof (int);
		      break;
		    }
		  break;
		case dt_float:
		  if (dt.ieee == dt_ieee)
		    long_len = (int) (offset_end-offset_current) / 4;
		  else
		    long_len = (int) (offset_end-offset_current) /
		      sizeof (float);
		  break;
		case dt_double:
		  if (dt.ieee == dt_ieee)
		    long_len = (int) (offset_end-offset_current) / 8;
		  else
		    long_len = (int) (offset_end-offset_current) /
		      sizeof (double);
		  break;
		}

	      i = ~0;
	      if (long_len & ~((long) i))
		{
		  fail ("fread: Maximum size exceeded.");
		  raise_exception ();
		}

	      if (fseek (stream, offset_current, SEEK_SET))
		{
		  fail ("fread: Improper seek.");
		  raise_exception ();
		}

	      len = (int) long_len;
            }

          ret = fread_entity (stream, &dt, len);
        }
      else
        {
          fail ("Can't read binary file.");
          raise_exception();
        }
    }
  ON_EXCEPTION
    {
      delete_3_entities (fname, type, length);
    }
  END_EXCEPTION;

  return ret;
}

#define READ_BINARY(AT, AN) \
  if (len) \
  { \
    AT *b = E_MALLOC (len, AN); \
    n = fread (b, sizeof (AT), len, stream); \
    if (n < len) \
      { \
        if (!feof (stream)) \
          { \
            fail (read_fail_msg, strerror (errno)); \
            clearerr (stream); \
            FREE (b); \
            raise_exception (); \
          } \
        clearerr (stream); \
        len = n; \
        if (n) \
          { \
            b = REALLOC (b, len * sizeof (AT)); \
          } else { \
            FREE (b); \
            b = NULL; \
          } \
      } \
    v = (VECTOR *) gift_wrap_vector (len, AN, b); \
  } else \
    v = (VECTOR *) form_vector (0, AN, dense);

#define CONVERT_BINARY(FT, AT, AN) \
  if (len) \
  { \
    FT *b = E_MALLOC (len, AN); \
    n = fread (b, sizeof (FT), len, stream); \
    if (n < len) \
      { \
        if (!feof (stream)) \
          { \
            fail (read_fail_msg, strerror (errno)); \
            clearerr (stream); \
            FREE (b); \
            raise_exception (); \
          } \
        clearerr (stream); \
        len = n; \
        if (n) \
          { \
            b = REALLOC (b, len * sizeof (AT)); \
          } else { \
            FREE (b); \
            b = NULL; \
          } \
      } \
    v = (VECTOR *) gift_wrap_vector (len, AN, b); \
    for (i=len-1; i>=0; i--) v->a.AN[i] = (AT) b[i]; \
  } else \
    v = (VECTOR *) form_vector (0, AN, dense);

static ENTITY *
fread_entity (FILE *stream, struct data_type *dt, int len)
{
  VECTOR *v = NULL;
  
  int i, n;

  switch (dt->base)
    {

    case dt_char:

      if (dt->size != dt_default ||
	  dt->endian != dt_default ||
	  dt->ieee != dt_default)
        {
          fail (invalid_type_msg, "fread");
          raise_exception ();
        }

      switch (dt->sgn)
        {

        case dt_signed:

          CONVERT_BINARY (signed char, int, integer);
          break;

        case dt_unsigned:

          CONVERT_BINARY (unsigned char, int, integer);
          break;

        default:

          CONVERT_BINARY (char, int, integer);
          break;
        }
      break;

    case dt_int:

      if (dt->ieee != dt_default)
        {
          fail (invalid_type_msg, "fread");
          raise_exception ();
        }

      if (dt->size == dt_long)
        {
          detour (detour_msg);
          raise_exception();
        }
          
      if (dt->size == dt_short)
	{
	  if (dt->endian == dt_default
	            || dt->endian == dt_big && WORDS_BIGENDIAN
	      || dt->endian == dt_little && !WORDS_BIGENDIAN)
	    {
	      CONVERT_BINARY (short, int, integer);
	    }
	  else
	    {
	      if (len)
		{
		  short *b = E_MALLOC (len, integer);
		  n = fread (b, sizeof (short), len, stream);
		  if (n < len)
		    {
		      if (!feof (stream))
			{
			  fail (read_fail_msg, strerror (errno));
			  clearerr (stream);
			  FREE (b);
			  raise_exception ();
			}
		      clearerr (stream);
		      len = n;
		      if (n)
			{
			  b = REALLOC (b, len * sizeof (int));
			} else {
			  FREE (b);
			  b = NULL;
			}
		    }
		  if (n)
		    {
		      char t;
		      char *p = (char *) b;
		      for (i=0; i<2*n; i+=2)
			{
			  t = p[i];
			  p[i] = p[i+1];
			  p[i+1] = t;
			}
		    }
		  v = (VECTOR *) gift_wrap_vector (len, integer, b);
		  for (i=len-1; i>=0; i--) v->a.integer[i] = (int) b[i];
		} else
		  v = (VECTOR *) form_vector (0, integer, dense);
	    }
	}
      else if (dt->sgn == dt_unsigned)
	{
	  CONVERT_BINARY (unsigned int, double, real);
	}
      else
	{
	  READ_BINARY (int, integer);
	}

      break;

    case dt_float:

      if (dt->sgn != dt_default || dt->size == dt_short)
        {
          fail (invalid_type_msg, "fread");
          raise_exception();
        }

      if (dt->size != dt_default ||
	  dt->endian != dt_default ||
	  dt->ieee != dt_default)
        {
          detour (detour_msg);
          raise_exception();
        }

      CONVERT_BINARY (float, double, real);

#if HAVE_ISNAN
      for (i=0; i<len; i++)
	if (isnan (v->a.real[i]))
	  {
	    fail ("fread: Invalid floating point data in file.");
	    delete_vector (v);
	    raise_exception ();
	  }
#endif

      break;

    case dt_double:

      if (dt->sgn != dt_default || dt->size == dt_short)
        {
          fail (invalid_type_msg, "fread");
          raise_exception();
        }

      if (dt->size != dt_default ||
	  dt->endian != dt_default ||
	  dt->ieee != dt_default)
        {
          detour (detour_msg);
          raise_exception();
        }

      READ_BINARY (double, real);

#if HAVE_ISNAN
      for (i=0; i<len; i++)
	if (isnan (v->a.real[i]))
	  {
	    fail ("fread: Invalid floating point data in file.");
	    delete_vector (v);
	    raise_exception ();
	  }
#endif

      break;

    default:

      detour (detour_msg);
      raise_exception ();
    }

  return ENT(v);
}

ENTITY *
bi_fwrite (int n, ENTITY *fname, ENTITY *v, ENTITY *type)
{
  /*
   * This routine writes data to a binary file from a numeric array.
   *
   * The `fname' arg is the file name; if it's NULL we use stdout.
   *
   * The `v' arg gives the vector to write.
   *
   * The `type' arg is a table; its member's names specify the
   * type of the data to write.  These names may include any of the
   * C types and qualifiers "char", "int", "float", "double",
   * "signed", "unsigned", "short", and "long".  Combinations that are
   * disallowed in C (such as "unsigned double") are disallowed here,
   * too.  In addition, the qualifiers "big", "little", and "ieee" are
   * accepted.  If `type' is NULL or empty, "double" is assumed.
   */

  struct data_type dt;

  WITH_HANDLING
    {
      FILE *stream = fname ? find_file (entity_to_string (EAT (fname)),
                                        FILE_OUTPUT) : stdout;
      if (!stream)
        {
          fail ("Can't write binary file.");
          raise_exception();
        }

      if (type && type->class != table)
	{
	  fail ("fwrite: Third arg is a %s; must be a table.",
		class_string[type->class]);
	  raise_exception ();
	}

      parse_types (&dt, EAT (type), "fwrite");

      v = dense_vector ((VECTOR *) vector_entity (EAT (v)));
      fwrite_entity (stream, &dt, EAT (v));
    }
  ON_EXCEPTION
    {
      delete_3_entities (fname, v, type);
    }
  END_EXCEPTION;

  return int_to_scalar(1);
}

#define WRITE_BINARY(p,s)	n = fwrite (p, s, v->ne, stream); \
				if (n < v->ne) \
				{ \
				  WRITE_FAIL (stream); \
				  delete_vector (v); \
				  raise_exception(); \
				}

static void
fwrite_entity (FILE *stream, struct data_type *dt, VECTOR *v)
{
  int i, n;

  switch (dt->base)
    {

    case dt_char:

      /* only sign qualifiers are accepted */
      
      if (dt->size != dt_default ||
	  dt->endian != dt_default ||
	  dt->ieee != dt_default)
        {
          fail (invalid_type_msg, "fwrite");
          raise_exception ();
        }

      v = (VECTOR *) cast_vector (EAT (v), integer);
      n = v->ne;

      switch (dt->sgn)
        {

        case dt_signed:
          {
            signed char *b = (signed char *) v->a.integer;
            for (i=0; i<n; i++) b[i] = (signed char) v->a.integer[i];
            WRITE_BINARY (v->a.integer, sizeof (signed char));
          }
          break;

        case dt_unsigned:
          {
            unsigned char *b = (unsigned char *) v->a.integer;
            for (i=0; i<n; i++) b[i] = (unsigned char) v->a.integer[i];
            WRITE_BINARY (v->a.integer, sizeof (unsigned char));
          }
          break;

        default:
          {
            char *b = (char *) v->a.integer;
            for (i=0; i<n; i++) b[i] = (char) v->a.integer[i];
            WRITE_BINARY (v->a.integer, sizeof (char));
          }
        }

      break;

    case dt_int:

      if (dt->ieee != dt_default)
        {
          fail (invalid_type_msg, "fwrite");
          raise_exception ();
        }

      if (dt->size == dt_long || dt->endian != dt_default)
        {
          detour (detour_msg);
          raise_exception();
        }

      v = (VECTOR *) cast_vector (EAT (v), integer);

      if (dt->size == dt_short)
	{
	  int *ip;
	  short *sp;
	  v = (VECTOR *) dup_vector (EAT (v));
	  ip = v->a.integer;
	  sp = (short *) v->a.integer;

	  for (i=0; i<v->ne; i++) sp[i] = (short) ip[i];
	  WRITE_BINARY (sp, sizeof (short));
	}
      else
	{
	  WRITE_BINARY (v->a.integer, sizeof (int));
	}

      break;

    case dt_float:

      if (dt->sgn != dt_default)
        {
          fail (invalid_type_msg, "fwrite");
          raise_exception();
        }

      if (dt->size != dt_default ||
	  dt->endian != dt_default ||
	  dt->ieee != dt_default)
        {
          detour (detour_msg);
          raise_exception();
        }

      v = (VECTOR *) dup_vector ((VECTOR *) cast_vector (EAT (v), real));

      {
        float *f = (float *) v->a.real;
        for (i=0; i<v->ne; i++) f[i] = v->a.real[i];
      }
      
      WRITE_BINARY (v->a.real, sizeof (float));

      break;

    case dt_double:

      if (dt->sgn != dt_default)
        {
          fail (invalid_type_msg, "fwrite");
          raise_exception();
        }

      if (dt->size != dt_default ||
	  dt->endian != dt_default ||
	  dt->ieee != dt_default)
        {
          detour (detour_msg);
          raise_exception();
        }

      v = (VECTOR *) cast_vector (EAT (v), real);
      WRITE_BINARY (v->a.real, sizeof (REAL));

      break;

    default:

      detour (detour_msg);
      raise_exception ();
    }

  delete_vector (v);
}
