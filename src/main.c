/*
   main.c -- Main program for Algae.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   This file is part of Algae.

   Algae is free software.  You can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Algae is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Algae; see the file LICENSE.  If not, write to the Free
   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   The copyright to major portions of Algae belongs to The Boeing
   Company.  The following permission notice and warranty disclaimer
   pertain to those portions of the code:

   Permission to use, copy, modify, and distribute this software
   and its documentation for any purpose is hereby granted,
   provided that the above copyright notice appear in all copies,
   that both the copyright notice and this permission notice and
   warranty disclaimer appear in supporting documentation, and that
   the names of Boeing or any of its entities not be used in
   advertising or publicity pertaining to distribution of the
   software without specific, written, prior permission.

   BOEING DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
   INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS,
   AND NONINFRINGEMENT.  IN NO EVENT SHALL BOEING BE LIABLE FOR ANY
   SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY
   DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA, OR PROFITS,
   WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE, OR OTHER TORTIOUS
   ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.
 */

static char rcsid[] =
"$Id: main.c,v 1.8 2003/08/01 04:57:47 ksh Exp $";

#include "main.h"
#include "algae.h"
#include <stdio.h>
#include <signal.h>
#include "exception.h"
#include "psr.h"
#include "code.h"
#include "file_io.h"
#include "entity.h"
#include "scalar.h"
#include "profile.h"
#include "printf.h"
#include "rc0.h"

void PROTO (enable_fpe_traps, (void));

int yydebug;
extern int PROTO (yyparse, (void));
extern void PROTO (yyrestart, ( FILE *input_file ));
extern char **file_args;
extern char **script_inputs;
extern int script_inputs_len;
extern FILE *yyin;

extern int skip_rc0, skip_rc1;

extern char *progname;

#if ENABLE_PROFILER
extern int profiling;		/* true for execution profiling */
#endif

int sigint_flag;		/* on if sigint hit */
int sigint_raise_exception_flag;
	/* on if sigint should raise exception */

/* The stack for algae_atexit */

#define ATEXIT_STACK_SIZE 8
static void (*atexit_stack[ATEXIT_STACK_SIZE]) ();
static void (**atexit_ptr) () = atexit_stack;

RETSIGTYPE
catch_sigint (int i)
{
  /*
   * On catching sigint, our preferred approach is to simply note that
   * we got it (by setting `sigint_flag') and then continue.  We poll
   * the flag periodically, when we can conveniently clean up and call
   * `raise_exception'.  Sometimes (like within external or system
   * code) we can't do any polling; in that case we set the flag
   * `sigint_raise_exception_flag' and then raise an exception
   * immediately here.
   */
   
  (void) signal (SIGINT, SIG_IGN);

  if (sigint_raise_exception_flag)
    raise_exception ();
  else
    sigint_flag = 1;
}

RETSIGTYPE
catch_sigquit (int i)
{
  /* Catching sigquit causes an immediate, clean exit. */
   
  (void) signal (SIGQUIT, SIG_IGN);
  algae_exit (1);
}

int
source (char *name)
{
  /*
   * Read commands from file `name'.  Returns zero on error.
   * Note that `name' is not FREE'd.
   */

  FILE *fp = fopen (name, "r");
  if (!fp)
    return (0);
  if (parse_and_exec (fp, name) < 0)
    {
      assert (exception_head == 0);
      fclose (fp);
      return (0);
    }
  assert (exception_head == 0);
  fclose (fp);
  return (1);
}

void
source_rc (void)
{
  char *name0, *name1;

  if (!skip_rc0)
    {
      name0 = getenv ("ALGAE_RC0");
      if (!name0 || !*name0)
	name0 = ALGAE_RC0_DEF;
      if (!source (name0))
	{
	  xfprintf (stderr, "%s: Problem reading startup file %s.\n",
		    progname, name0);
	  algae_exit (1);
	}
    }

  if (!skip_rc1)
    {
      name1 = getenv ("ALGAE_RC1");
      if (!name1 || !*name1)
	{
	  name1 = getenv ("HOME");
	  name1 = append_char (name1 ? name1 : NULL_string, "/.algae");
	}
      if (name1 != name0)
	source (name1);
    }

}

int
main (int argc, char **argv)
{
  FILE *fp;

  initialize (argc, argv);

  (void) signal (SIGINT, catch_sigint);
  (void) signal (SIGQUIT, catch_sigquit);
  (void) signal (SIGPIPE, SIG_IGN);	/* Ignore broken pipes. */

  /*
   * Unfortunately, flex thinks that if yyin is NULL the first time it's
   * called then you really meant stdin.  We have to get all dressed up
   * to go nowhere, just so flex doesn't change our yyin.  This is only
   * a concern if we have input scripts to deal with.
   */

  if (script_inputs)
    {
      fp = fopen ("/dev/null", "r");
      if (!fp)
	{
	  fprintf (stderr, "%s: Can't open \"/dev/null\".\n", progname);
	  algae_exit (1);
	}
      if (parse_and_exec (fp, "/dev/null") < 0) algae_exit (1);
      fclose (fp);
    }

  /* Run the startup code. */

  source_rc ();

  /* Now run each command line script ("-e" option) in order. */

  if (script_inputs)
    {
      char **s = script_inputs;
      int i = script_inputs_len;
    
      while (i--) if (parse_and_exec (NULL, *s++)) algae_exit (1);
      FREE (script_inputs);
    }

  /* Now we source each file given on the command line. */

  while (*file_args)
    {
      fp = ((*file_args)[0] == '-' && (*file_args)[1] == 0) ?
	stdin : fopen (*file_args, "r");
      if (!fp)
	{
	  fprintf (stderr, "%s: Can't open source file %s (%s).\n",
		   progname, *file_args, strerror (errno));
	  algae_exit (1);
	}
      if (parse_and_exec (fp, *file_args) < 0) algae_exit (1);
      assert (exception_head == 0);
      if (fp != stdin) fclose (fp);
      file_args++;
    }
  algae_exit (0);
}

/*
 * Parse and execute an input stream.  If `fp' is NULL, then the input
 * is read from the character string pointed to by `file_name'.  Otherwise,
 * `fp' is a pointer to the input file and `file_name' gives its name.
 * The `file_name' arg must point to a null terminated character string.
 * The memory to which `file_name' points is not freed.  The function
 * returns 0 on success, -1 if a parse error occurs, and -2 if a run time
 * error occurs.
 */

int
parse_and_exec (FILE *fp, char *file_name)
{
  volatile int done = 0;
  int retval;

  if (fp == stdin) file_name = "stdin";

  push_parser_scanner_state (fp, file_name);

  PROF_UPDATE_FILE (file_name);	/* for profiler */

  do
    {
      WITH_HANDLING
      {
	retval = 0;
	if (interactive)
	  yyparse ();
	else if (yyparse () != 0)
	  retval = -1;
	else
	  {
	    code1 (OP_HALT);
	    if (da_flag)
	      da (code_base, stderr);
	    execute (code_base, NULL, NULL);
	  }

	done = 1;
      }
      EXCEPTION_BASE
      {

	signal (SIGINT, SIG_IGN);

	fflush (stdout);
	xputc ('\n', stderr);
	fflush (stderr);

	/* FORTRAN sometimes changes stderr buffering, so reset it. */

	setbuf (stderr, NULL);
	retval = -2;

	if (interactive)
	  yyrestart (yyin);
	else
	  done = 1;

	sigint_flag = 0;
	sigint_raise_exception_flag = 0;
	signal (SIGINT, catch_sigint);
      }
      END_EXCEPTION_BASE;

      enable_fpe_traps ();
    }
  while (!done);

  pop_parser_scanner_state ();
  return retval;
}

int
algae_atexit (void (*fcn)())
{
  /*
   * This replaces the ANSI `atexit', which isn't always around.
   * The static array `atexit_stack' keeps the function pointers.
   */

  assert (atexit_ptr - atexit_stack < ATEXIT_STACK_SIZE);

  *(atexit_ptr++) = fcn;

  return 1;
}

void
algae_exit (int status)
{
  /*
   * Call `exit', but first call functions registered with `algae_atexit'.
   */

  assert (atexit_ptr - atexit_stack <= ATEXIT_STACK_SIZE);

  while (atexit_ptr-- > atexit_stack)
    (*atexit_ptr) ();

  exit (status);
}

ENTITY *
bi_exit (int n, ENTITY *s)
{
  /*
   * Here's how users call exit.  If `s' is NULL, we return exit
   * code 0.  If `s' is a non-character scalar, then it's converted
   * to an integer and returned.  Otherwise we return 1.
   */

  int status = 1;

  if (!s)
    {
      status = 0;
    }
  else
    {
      if (s->class == scalar && ((SCALAR *) s)->type != character)
	status = entity_to_int (s);
    }

  algae_exit (status);
}

/*
 * Linux ELF systems have a problem with unresolved
 * references from the Fortran libraries.
 */

#if linux

void
MAIN__ (void)
{
  wipeout ("The Fortran main program is missing.");
}

#if 0

/* I don't think we need this any more. */
void
etime_ (void)
{
  wipeout ("The etime_ routine is missing.");
}
#endif

#endif
