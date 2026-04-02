/*
   fpe/apollo.c -- SIGFPE for Apollo.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.
   See the file COPYING for license, warranty, and permission details.
*/

static char rcsid[] =
  "$Id: apollo.c,v 1.1.1.1 1996/04/17 05:56:14 ksh Exp $";

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>

#if !SKIP_LOCAL_INCLUDES
#include "exception.h"
#include "message.h"
#endif

static void
catch_sigfpe( sig, code, scp )
  int sig;
  int code;
  struct sigcontext *scp;
{
    /* This function handles SIGFPE signals. */

    switch( code ) {

      case FPE_INTOVF_TRAP:

	fail( "Integer overflow." );
	break;

      case FPE_INTDIV_TRAP:

	fail( "Integer divide by zero." );
	break;

      case FPE_FLTOVF_FAULT:
      case FPE_FLTOVF_TRAP:

	fail( "Floating point overflow." );
	break;

      case FPE_FLTDIV_TRAP:
      case FPE_FLTDIV_FAULT:

	fail( "Floating point divide by zero." );
	break;

      case FPE_DECOVF_TRAP:

	/* I don't know what "decimal overflow" is! */

	fail( "Decimal overflow." );
	break;

      case FPE_SUBRNG_TRAP:

	/* I'm not sure what this is, but it sounds bad. */

	wipeout( "Subscript range trap." );
	break;

      default:

	/* Ignore other stuff, like underflow. */

	signal( SIGFPE, catch_sigfpe );
	return;
    }

    signal( SIGFPE, catch_sigfpe );
    raise_exception();
}

void
enable_fpe_traps()
{
    signal( SIGFPE, catch_sigfpe );
}
