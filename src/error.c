/*
   error.c -- Error messages.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: error.c,v 1.1.1.1 1996/04/17 05:56:17 ksh Exp $";

#include <stdio.h>
#include <math.h>
#include "entity.h"
#include "ptypes.h"
#include "vargs.h"
#include "printf.h"

extern char *progname;
extern char *curr_line;
extern int curr_offset;

extern int curr_line_no;
extern char *curr_file;

void
yyerror (char *s)
{
  int i;

  p_error ("");
  xfprintf (stderr, "%s", curr_line);
  if (*curr_line && curr_line [strlen (curr_line) - 1] != '\n')
    xfprintf (stderr, "\n");
  for (i = 0; i < curr_offset; i++)
    if (curr_line[i] == '\t')
      xputc ('\t', stderr);
    else
      xputc (' ', stderr);

  xfprintf (stderr, "^ %s\n", s);
  raise_exception ();
}

#if HAVE_MATHERR

int
matherr (struct exception *x)
{
  switch (x->type)
    {
    case DOMAIN:
      fail ("%s: Argument out of domain.", x->name);
      raise_exception ();

    case SING:
      fail ("%s: Argument singularity.", x->name);
      raise_exception ();

    case OVERFLOW:
      fail ("%s: Floating point overflow.", x->name);
      raise_exception ();

    case UNDERFLOW:
      break;

    case TLOSS:
      fail ("%s: Total loss of significance.", x->name);
      raise_exception ();

    case PLOSS:
      warn ("%s: Partial loss of significance.", x->name);
      break;
    }

  return 1;
}

#endif
