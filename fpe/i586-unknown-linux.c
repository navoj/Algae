/*
   fpe/i586-unknown-linux.c -- SIGFPE for Linux on a Pentium

   Copyright (C) 1996  K. Scott Hunziker.
   See the file COPYING for license, warranty, and permission details.
*/

static char rcsid[] =
  "$Id: i586-unknown-linux.c,v 1.2 1996/05/10 05:33:18 ksh Exp $";

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>

#include <fpu_control.h>

#if !SKIP_LOCAL_INCLUDES
#include "exception.h"
#include "message.h"
#endif

void enable_fpe_traps ();

/*
 * Signal handler for floating point exceptions.  Unfortunately, there
 * seems to be no way to determine the cause (overflow, divide by zero,
 * etc.) of the exception.
 */

static void
catch_sigfpe (sig)
  int sig;
{
  fail ("Floating point exception.");
  enable_fpe_traps ();
  raise_exception ();
}

void
enable_fpe_traps ()
{
  struct sigaction act;

  act.sa_handler = catch_sigfpe;		/* the signal handler */
  sigemptyset( &(act.sa_mask) );		/* no other signals blocked */
  act.sa_flags = 0;

  if ( sigaction( SIGFPE, &act, NULL ) )	/* specify handler */
    wipeout( "Bad sigaction call." );

  sigaddset (&(act.sa_mask), SIGFPE);		/* unblock SIGFPE */
  sigprocmask (SIG_UNBLOCK, &(act.sa_mask), NULL);

  __setfpucw (_FPU_DEFAULT & ~_FPU_MASK_IM & ~_FPU_MASK_ZM & ~_FPU_MASK_OM);
}
