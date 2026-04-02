/*
   fpe/alpha-unknown-freebsd4.8.c -- SIGFPE code

   Copyright (C) 2003  K. Scott Hunziker.
   See the file COPYING for license, warranty, and permission details.
*/

static char rcsid[] =
  "$Id: alpha-unknown-freebsd4.8.c,v 1.1 2003/08/09 18:13:15 ksh Exp $";

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>

#include <assert.h>

#include <ieeefp.h>
#include <machine/fpu.h>

#if !SKIP_LOCAL_INCLUDES	/* only used during configuration */
#include "exception.h"
#include "message.h"
#endif

static void
catch_sigfpe (sig, code, v)
     int sig;
     siginfo_t *code;
     void *v;
{
  assert (sig == SIGFPE);

  fail ("Floating point exception.");
  raise_exception();
}

void
enable_fpe_traps()
{
  struct sigaction act;

  act.sa_sigaction = catch_sigfpe;	/* the signal handler */
  if (sigemptyset (&(act.sa_mask)))	/* no other signals blocked */
    wipeout ("sigemptyset");
  act.sa_flags = SA_SIGINFO;		/* want 3 args for handler */

  if (sigaction (SIGFPE, &act, NULL))	/* specify handler */
    wipeout ("Bad sigaction call.");

  if (sigaddset (&(act.sa_mask), SIGFPE))	/* unblock SIGFPE */
    wipeout ("sigaddset");
  if (sigprocmask (SIG_UNBLOCK, &(act.sa_mask), NULL))
    wipeout ("sigprocmask");

  /* enable traps */

  (void) fpsetmask( FP_X_INV | FP_X_OFL | FP_X_DZ );
}
