/*
   mem.c -- MALLOC Package

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: mem.c,v 1.5 2003/12/10 04:38:37 ksh Exp $";

#include "algae.h"
#include "mem.h"

#if ! USE_DLMALLOC

void *
algae_malloc (size_t size)
{
  void *p;

  assert (size > 0);

  p = malloc (size);
  if (p == NULL)
    {
      fail ("Out of memory.");
      raise_exception ();
    }
  return (p);
}

void *
algae_calloc (size_t nobj, size_t size)
{
  void *p;

  assert (nobj > 0);
  assert (size > 0);

  p = calloc (nobj, size);
  if (p == NULL)
    {
      fail ("Out of memory.");
      raise_exception ();
    }
  return (p);
}

void *
algae_realloc (void *p, size_t size)
{
  assert (size > 0);

  p = realloc (p, size);
  if (p == NULL)
    {
      fail ("Out of memory.");
      raise_exception ();
    }
  return (p);
}

#endif /* ! USE_DLMALLOC */
