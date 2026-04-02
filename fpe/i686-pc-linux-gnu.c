/*
   fpe/i686-pc-linux-gnu.c -- SIGFPE for Linux on a Pentium

   Copyright (C) 2001-2  K. Scott Hunziker.
   See the file COPYING for license, warranty, and permission details.
*/

static char rcsid[] =
  "$Id: i686-pc-linux-gnu.c,v 1.5 2002/07/30 22:29:42 ksh Exp $";

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>

#include <assert.h>
#include <fenv.h>

#if !SKIP_LOCAL_INCLUDES
#include "exception.h"
#include "message.h"
#endif

void enable_fpe_traps ();
int feenableexcept (int EXCEPTS);

/* Signal handler for floating point exceptions. */

static void
catch_sigfpe (int sig, siginfo_t *code, void *v)
{
  assert (sig == SIGFPE);

  switch (code->si_code)
    {
    case FPE_INTDIV:
      fail ("Integer divide by zero.");
      break;
    case FPE_INTOVF:
      fail ("Integer overflow.");
      break;
    case FPE_FLTDIV:
      fail ("Floating point divide by zero.");
      break;
    case FPE_FLTOVF:
      fail ("Floating point overflow.");
      break;
    case FPE_FLTUND:
      fail ("Floating point underflow.");
      break;
    case FPE_FLTRES:
      fail ("Floating point inexact result.");
      break;
    case FPE_FLTINV:
      fail ("Invalid floating point operation.");
      break;
    case FPE_FLTSUB:
      fail ("Floating point subscript out of range.");
      break;
    default:
      fail ("Unknown floating point exception.");
      break;
    }

  enable_fpe_traps ();
  raise_exception ();
}

/* This is swiped from glibc-2.2.2/sysdeps/i386/fpu/feenablxcpt.c */

#if !HAVE_FEENABLEEXCEPT
int
feenableexcept (int excepts)
{
  unsigned short int new_exc, old_exc;

  /* Get the current control word.  */
  __asm__ ("fstcw %0" : "=m" (*&new_exc));

  excepts &= FE_ALL_EXCEPT;
  old_exc = (~new_exc) & FE_ALL_EXCEPT;

  new_exc &= ~excepts;
  __asm__ ("fldcw %0" : : "m" (*&new_exc));

  return old_exc;
}
#endif

void
enable_fpe_traps ()
{
  struct sigaction act;

  act.sa_sigaction = catch_sigfpe;	/* the signal handler */
  sigemptyset (&(act.sa_mask));		/* no other signals blocked */
  act.sa_flags = SA_SIGINFO;		/* want 3 args for handler */

  if (sigaction (SIGFPE, &act, NULL))	/* specify handler */
    wipeout ("Bad sigaction call.");

  /*
   * The feenableexcept function is new for glibc 2.2.  See its
   * description in the man page for fenv(3).
   */

  (void) feenableexcept (FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW);
}
