/*
   fpe/fpe.c -- generic SIGFPE

   Copyright (C) 2003  K. Scott Hunziker.
   See the file COPYING for license, warranty, and permission details.
*/

static char rcsid[] =
  "$Id: fpe.c,v 1.1 2003/08/09 18:13:15 ksh Exp $";

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>

#include <assert.h>

/*** floating point ***/
#if HAVE_FENV_H
#include <fenv.h>
#endif
#if HAVE_MACHINE_FPU_H
#include <machine/fpu.h>
#endif
#if HAVE_IEEEFP_H
#include <ieeefp.h>
#endif
#if HAVE_SIGINFO_H
#include <siginfo.h>
#endif
#if HAVE_UCONTEXT_H
#include <ucontext.h>
#endif
/**********************/

#if !SKIP_LOCAL_INCLUDES	/* only used during configuration */
#include "exception.h"
#include "message.h"
#endif

void enable_fpe_traps (void);
#if HAVE_FEENABLEEXCEPT
extern int feenableexcept (int excepts);
#endif

/* Signal handler for floating point exceptions. */

#if HAVE_SIGACTION
static void
catch_sigfpe (int sig, siginfo_t *code, void *v)
{
  assert (sig == SIGFPE);

  switch (code->si_code)
    {
    case FPE_FLTDIV:
      fail ("Floating point divide by zero.");
      break;
    case FPE_FLTOVF:
      fail ("Floating point overflow.");
      break;
    case FPE_FLTRES:
      fail ("Floating point inexact.");
      break;
    case FPE_FLTINV:
      fail ("Floating point invalid.");
      break;
    default:
      fail ("Floating point exception (%d).", code->si_code);
      break;
    }

  raise_exception ();
}
#else
static RETSIGTYPE
catch_sigfpe (int sig)
{
  assert (sig == SIGFPE);

  fail ("Floating point exception.");
  raise_exception ();
}
#endif

void
enable_fpe_traps (void)
{
#if HAVE_SIGACTION
  struct sigaction act;

  act.sa_sigaction = catch_sigfpe;	/* the signal handler */
  if (sigemptyset (&(act.sa_mask)))	/* no other signals blocked */
    wipeout ("sigemptyset");
  act.sa_flags = SA_SIGINFO;		/* want 3 args for handler */

  if (sigaction (SIGFPE, &act, NULL))	/* specify handler */
    wipeout ("Bad sigaction call.");

  if (sigemptyset (&(act.sa_mask)))	/* unblock SIGFPE */
    wipeout ("sigemptyset");
  if (sigprocmask (SIG_SETMASK, &(act.sa_mask), NULL))
    wipeout ("sigprocmask");
#else
  (void) signal (SIGFPE, catch_sigfpe);
#endif

  /* enable traps */

#if HAVE_FPSETMASK || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
  (void) fpsetmask( FP_X_INV | FP_X_OFL | FP_X_DZ );
#elif HAVE_FEENABLEEXCEPT
  (void) feenableexcept (FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW);
#elif HAVE_FPSETDEFAULTS
  (void) fpsetdefaults();
#else
  choke me
#endif
}
