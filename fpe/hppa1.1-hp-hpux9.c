/*
   fpe/hppa1.1-hp-hpux9.c -- SIGFPE for HP-UX.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.
   See the file COPYING for license, warranty, and permission details.
*/

static char rcsid[] =
  "$Id: hppa1.1-hp-hpux9.c,v 1.1.1.1 1996/04/17 05:56:14 ksh Exp $";

/*
 * This fpe code works with 700 and 800 series machines, but probably
 * not with older machines.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
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
    switch( code ) {

      case 12:

	fail( "Floating point overflow." );
	break;

      case 13:

	fail( "Floating point conditional trap." );
	break;

      case 14:	/* What on earth is this? */

	fail( "Floating point assist exception trap." );
	break;

      case 22:	/* What on earth is this? */

	fail( "Floating point assist emulation trap." );
	break;

      default:

	fail( "Unknown floating point exception." );
	break;

    }

    signal( SIGFPE, catch_sigfpe );
    raise_exception();
}

void
enable_fpe_traps()
{
    fpsetdefaults();
    signal( SIGFPE, catch_sigfpe );
}
