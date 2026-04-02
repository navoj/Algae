/*
   fpe/sparc-sun-solaris2.3.c -- SIGFPE for Sun SPARC with Solaris 2.3

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.
   See the file COPYING for license, warranty, and permission details.
*/

static char rcsid[] =
  "$Id: sparc-sun-solaris2.3.c,v 1.1.1.1 1996/04/17 05:56:14 ksh Exp $";

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>

#include <floatingpoint.h>
#include <siginfo.h>
#include <ucontext.h>

#if !SKIP_LOCAL_INCLUDES
#include "exception.h"
#include "message.h"
#endif

/*
 * Signal handler for floating point exceptions.  This includes code
 * for integer overflow and divide by zero, even though there doesn't
 * appear to be any way to trigger them.
 */

static sigfpe_handler_type
catch_sigfpe( sig, code, scp )
  sigfpe_code_type sig;
  siginfo_t *code;
  ucontext_t *scp;
{
    switch( code->si_code ) {

      case FPE_FLTDIV:

	fail( "Divide by zero." );
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

      default:

	fail( "Unknown floating point exception." );
	break;

    }

    if ( sigrelse( SIGFPE ) )
        warn( "Can't release SIGFPE hold." );
    raise_exception();
}

void
enable_fpe_traps()
{
    if ( ieee_handler( "set", "common", catch_sigfpe ) )
        warn( "IEEE floating point exception trapping is not supported." );
}
