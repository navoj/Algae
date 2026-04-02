/*
   fpe/sparc-sun-solaris2.8.c -- SIGFPE code

   Copyright (C) 1994-2001  K. Scott Hunziker.
   See the file COPYING for license, warranty, and permission details.
*/

static char rcsid[] =
  "$Id: sparc-sun-solaris2.8.c,v 1.2 2001/12/07 21:36:05 ksh Exp $";

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>

#include <ieeefp.h>
#include <ucontext.h>

#if !SKIP_LOCAL_INCLUDES
#include "exception.h"
#include "message.h"
#endif

void enable_fpe_traps();

/*
 * Signal handler for floating point exceptions.  This includes code
 * for integer overflow and divide by zero, even though there doesn't
 * appear to be any way to trigger them.
 */

static void
catch_sigfpe( sig, info, cntxt )
  int sig;
  siginfo_t *info;
  ucontext_t *cntxt;
{
  switch( info->si_code )
  {
    case FPE_INTDIV:

      fail( "Integer divide by zero." );
      break;

    case FPE_INTOVF:

      fail( "Integer overflow." );
      break;

    case FPE_FLTDIV:

      fail( "Floating point divide by zero." );
      break;

    case FPE_FLTOVF:

      fail( "Floating point overflow." );
      break;

    case FPE_FLTUND:

      fail( "Floating point underflow." );
      break;

    case FPE_FLTRES:

      fail( "Floating point inexact." );
      break;

    case FPE_FLTINV:

      fail( "Invalid floating point operation." );
      break;

    case FPE_FLTSUB:

      fail( "Subscript out of range." );
      break;

    default:

      fail( "Unrecognized SIGFPE." );
      break;
  }

  enable_fpe_traps();

  raise_exception();
}

void
enable_fpe_traps()
{
  struct sigaction act;

  act.sa_handler = catch_sigfpe;		/* the signal handler */
  sigemptyset( &(act.sa_mask) );		/* no other signals blocked */
  act.sa_flags = SA_SIGINFO;			/* provide info to handler */

  if ( sigaction( SIGFPE, &act, NULL ) )	/* specify handler */
    wipeout( "Bad sigaction call." );

  sigaddset (&(act.sa_mask), SIGFPE);		/* unblock SIGFPE */
  sigprocmask (SIG_UNBLOCK, &(act.sa_mask), NULL);

  fpsetmask( FP_X_INV | FP_X_OFL | FP_X_DZ );	/* enable traps */
}
