/*
   datum.c -- Functions for Algae data.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: datum.c,v 1.4 2003/08/07 04:48:19 ksh Exp $";

#include "ptypes.h"
#include "pmem.h"
#include "datum.h"
#include "algae.h"
#include "entity.h"
#include "scalar.h"
#include "print.h"
#include "test.h"
#include "get.h"
#include "put.h"
#include "file_io.h"

extern DATUM *num_digits;	/* number of significant digits to print */
#define DIGITS datum_to_int (num_digits)

extern int whitespace;	/* true prints whitespace */

/* TEMP */
extern int da_flag;
extern int dummy;


DATUM the_null;			/* NULL */

/* returns DATUM* initialized to D_NULL */
DATUM *
new_DATUM (void)
{
  DATUM *p = (DATUM *) MALLOC (sizeof (DATUM));

  p->type = D_NULL;
  return p;
}

DATUM *
new_INT (int val)
{
  DATUM *p = P_MEM (DATUM);

  p->type = D_INT;
  IVAL (p) = val;
  return p;
}


DATUM *
new_REAL (double val)
{
  DATUM *p = P_MEM (DATUM);

  p->type = D_REAL;
  RVAL (p) = val;
  return p;
}

#define WS(w, nw)	(whitespace ? w : nw)

void
datum_print (register DATUM *p)
{
  assert (p != NULL);
  switch (p->type)
    {
    case D_NULL:
      printf (WS ("\tNULL\n", "NULL"));
      break;

    case D_INT:
      printf (WS ("\t%d\n", "%d"), p->data.ival);
      break;

    case D_REAL:
      printf (WS ("\t%#.*g\n", "%#.*g"), DIGITS, p->data.rval);
      break;

    case D_ENTITY:
      print_entity (copy_entity (p->data.ptr), stdout);
      break;

    default:
      wipeout ("datum_print");
    }
  fflush (stdout);
}

DATUM *
copy_datum (register DATUM *target, register DATUM *source)
{
  switch (target->type = source->type)
    {
    case D_NULL:
      break;

    case D_INT:
      target->data.ival = source->data.ival;
      break;

    case D_REAL:
      target->data.rval = source->data.rval;
      break;

    case D_ENTITY:
      target->data.ptr =
	(void *) copy_entity ((ENTITY *) source->data.ptr);
      break;

    default:
      wipeout ("data_copy");
      break;
    }
  return target;
}


/* 
   make two successive data the same type
   casting `up'.
   Returns the common type.
 */

DATUM_TYPE
cast2_datum (DATUM *p)
{
  DATUM_TYPE retval;

  switch (TEST2DATUM (p))
    {
    CASE_any_NULL:
      retval = D_NULL;
      break;

    case INT_INT:
      retval = D_INT;
      break;

    case INT_REAL:
      p->data.rval = (double) p->data.ival;
      retval = p->type = D_REAL;
      break;

    case INT_ENTITY:
      p->data.ptr = int_to_scalar (p->data.ival);
      retval = p->type = D_ENTITY;
      break;

    case REAL_INT:
      (p + 1)->data.rval = (double) (p + 1)->data.ival;
      retval = (p + 1)->type = D_REAL;
      break;

    case REAL_REAL:
      retval = D_REAL;
      break;

    case REAL_ENTITY:
      p->data.ptr = real_to_scalar (p->data.rval);
      retval = p->type = D_ENTITY;
      break;

    case ENTITY_INT:
      (p + 1)->data.ptr = int_to_scalar ((p + 1)->data.ival);
      retval = (p + 1)->type = D_ENTITY;
      break;

    case ENTITY_REAL:
      (p + 1)->data.ptr = real_to_scalar ((p + 1)->data.rval);
      retval = (p + 1)->type = D_ENTITY;
      break;

    case ENTITY_ENTITY:
      retval = D_ENTITY;
      break;

    default:
      wipeout ("cast2_datum");
    }

  return retval;
}

/*
   cast two successive datum to type D_ENTITY 
   return is D_NULL or D_ENTITY 
 */

DATUM_TYPE
cast2_to_entity (DATUM *p)
{
  DATUM_TYPE retval = D_ENTITY;

  /* don't switch type field until successful */

  switch (TEST2DATUM (p))
    {
    CASE_any_NULL:
      retval = D_NULL;
      break;

    case INT_INT:
      p->data.ptr = int_to_scalar (p->data.ival);
      p->type = D_ENTITY;

      (p + 1)->data.ptr = int_to_scalar ((p + 1)->data.ival);
      (p + 1)->type = D_ENTITY;
      break;

    case INT_REAL:
      p->data.ptr = int_to_scalar (p->data.ival);
      p->type = D_ENTITY;

      (p + 1)->data.ptr = real_to_scalar ((p + 1)->data.rval);
      (p + 1)->type = D_ENTITY;
      break;

    case INT_ENTITY:
      p->data.ptr = int_to_scalar (p->data.ival);
      p->type = D_ENTITY;
      break;

    case REAL_INT:
      p->data.ptr = real_to_scalar (p->data.rval);
      p->type = D_ENTITY;

      (p + 1)->data.ptr = int_to_scalar ((p + 1)->data.ival);
      (p + 1)->type = D_ENTITY;
      break;

    case REAL_REAL:
      p->data.ptr = real_to_scalar (p->data.rval);
      p->type = D_ENTITY;

      (p + 1)->data.ptr = real_to_scalar ((p + 1)->data.rval);
      (p + 1)->type = D_ENTITY;
      break;

    case REAL_ENTITY:
      p->data.ptr = real_to_scalar (p->data.rval);
      p->type = D_ENTITY;
      break;

    case ENTITY_INT:
      (p + 1)->data.ptr = int_to_scalar ((p + 1)->data.ival);
      (p + 1)->type = D_ENTITY;
      break;

    case ENTITY_REAL:
      (p + 1)->data.ptr = real_to_scalar ((p + 1)->data.rval);
      (p + 1)->type = D_ENTITY;
      break;

    case ENTITY_ENTITY:
      break;

    default:
      wipeout ("cast2_to_entity");
    }

  return retval;
}


int
cast1_to_entity (DATUM *dp)
{
  switch (dp->type)
    {
    case D_NULL:
      return D_NULL;

    case D_INT:
      dp->data.ptr = int_to_scalar (dp->data.ival);
      break;

    case D_REAL:
      dp->data.ptr = real_to_scalar (dp->data.rval);
      break;
    }
  /* change type last in case of exception */
  return dp->type = D_ENTITY;
}


int
datum_test (DATUM *dp)
{
  switch (dp->type)
    {
    case D_NULL:
      return 0;

    case D_INT:
      return dp->data.ival != 0;

    case D_REAL:
      return dp->data.rval != 0.0;

    case D_ENTITY:
      return test_entity (E_PTR (dp));	/* `dp' is deleted. */

    default:
      wipeout ("datum_test");
    }
}

#if DEBUG

void
DB_delete_datum (DATUM *p)
{
  if (p->type == D_ENTITY)
    delete_entity (E_PTR (p));
  p->type = D_NULL;
}

#endif

int
datum_to_int (DATUM *dp)
{
  /*
   * This routine grabs an int from a DATUM.  "I call 'em if I
   * see 'em, and if I don't see 'em I make 'em up."  If it has
   * problems, it just returns zero.
   */

  switch (dp->type)
    {
    case D_NULL:
      return (0);
    case D_INT:
      return (dp->data.ival);
    case D_REAL:
      return ((int) floor (dp->data.rval + 0.5));
    case D_ENTITY:
      switch (E_PTR (dp)->class)
	{
	case scalar:
	  switch (((SCALAR *) E_PTR (dp))->type)
	    {
	    case integer:
	      return (((SCALAR *) E_PTR (dp))->v.integer);
	    case real:
	      return ((int) floor (((SCALAR *) E_PTR (dp))->v.real + 0.5));
	    case complex:
	      return ((int) floor (((SCALAR *) E_PTR (dp))->v.complex.real + 0.5));
	    default:
	      return (0);
	    }
	default:
	  return (0);
	}
    default:
      wipeout ("datum_to_int");
    }
}

int
put_datum (DATUM *d, FILE *stream, struct ent_node *ent_tree)
{
  /*
   * Write a datum to a binary file.  Returns 0 on error, 1 otherwise.
   * The DATUM `d' is not deleted.
   */

  if (!WRITE_INT (&d->type, stream))
    return 0;

  switch (d->type)
    {

    case D_NULL:
      break;

    case D_INT:
      if (!WRITE_INT (&d->data.ival, stream))
	return 0;
      break;

    case D_REAL:
      if (!WRITE_DOUBLE (&d->data.rval, stream))
	return 0;
      break;

    default:
      assert (d->type == D_ENTITY);
      if (!put_entity (copy_entity (d->data.ptr), stream, ent_tree))
	return 0;
    }

  return 1;
}

DATUM *
get_datum (FILE *stream)
{
  /* Read a datum from a binary file. */

  DATUM *d = new_DATUM ();

  if (!READ_INT (&d->type, stream))
    {
      FREE (d);
      return NULL;
    }

  switch (d->type)
    {

    case D_NULL:
      break;

    case D_INT:
      if (!READ_INT (&d->data.ival, stream))
	{
	  FREE (d);
	  return NULL;
	}
      break;

    case D_REAL:
      if (!READ_DOUBLE (&d->data.rval, stream))
	{
	  FREE (d);
	  return NULL;
	}
      break;

    default:
      if (d->type != D_ENTITY ||
	  !(d->data.ptr = get_entity (stream)))
	{
	  warn ("Invalid datum in file.");
	  FREE (d);
	  return NULL;
	}
    }

  return d;
}
