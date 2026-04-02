/*
   fpe/sun.c -- SIGFPE for Sun OS 4.1

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.
   See the file COPYING for license, warranty, and permission details.
*/

static char rcsid[] =
  "$Id: sparc-sun-sunos4.1.c,v 1.1.1.1 1996/04/17 05:56:14 ksh Exp $";

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>

#include <floatingpoint.h>

#if !SKIP_LOCAL_INCLUDES
#include "exception.h"
#include "message.h"
#endif

static void
catch_sigfpe( sig, code, scp, addr )
  int sig;
  int code;
  struct sigcontext *scp;
  char *addr;
{
    switch( code ) {

      case FPE_INTOVF_TRAP:

	fail( "Integer overflow." );
	break;

      case FPE_INTDIV_TRAP:

	fail( "Integer divide by zero." );
	break;

      case FPE_FLTDIV_TRAP:

	fail( "Divide by zero." );
	break;

      case FPE_FLTOVF_TRAP:

	fail( "Floating point overflow." );
	break;

      case FPE_FLTUND_TRAP:

	fail( "Floating point underflow." );
	break;

      case FPE_FLTINEX_TRAP:

	fail( "Floating point inexact." );
	break;

      case FPE_FLTOPERR_TRAP:

	fail( "Invalid floating point operation." );
	break;

      default:

	fail( "Unknown floating point exception." );
	break;

    }

    raise_exception();
}

void
enable_fpe_traps()
{
    ieee_handler( "set", "common", catch_sigfpe );
    sigfpe( FPE_INTOVF_TRAP, catch_sigfpe );
    sigfpe( FPE_INTDIV_TRAP, catch_sigfpe );
}
