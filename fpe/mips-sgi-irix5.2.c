/*
   fpe/mips-sgi-irix5.2.c -- SIGFPE for SGI.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.
   See the file COPYING for license, warranty, and permission details.
*/

static char rcsid[] =
  "$Id: mips-sgi-irix5.2.c,v 1.1.1.1 1996/04/17 05:56:14 ksh Exp $";

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>

#include <sigfpe.h>
#include <sys/fpu.h>

#if !SKIP_LOCAL_INCLUDES
#include "exception.h"
#include "message.h"
#endif

static void
catch_sigtrap( sig, code, scp )
  int sig;
  int code;
  struct sigcontext *scp;
{
    switch ( code ) {

      case BRK_OVERFLOW:

	fail( "Integer overflow." );
	break;

      case BRK_DIVZERO:

	fail( "Integer divide by zero." );
	break;

      case BRK_MULOVF:

	fail( "Multiply overflow." );
	break;

      default:

	wipeout( "Caught SIGTRAP (%d).", code );
    }

    signal( SIGTRAP, catch_sigtrap );
    raise_exception();
}

static void
catch_sigfpe( sig, code, scp )
  int sig;
  int code;
  struct sigcontext *scp;
{
    union fpc_csr csr;
    csr.fc_word = scp->sc_fpc_csr;

    if		( csr.fc_struct.se_invalid )
      {
	  fail( "Invalid floating point operation." );
      }
    else if	( csr.fc_struct.se_divide0 )
      {
	  fail( "Divide by zero." );
      }
    else if	( csr.fc_struct.se_overflow )
      {
	  fail( "Floating point overflow." );
      }
    else if	( csr.fc_struct.se_underflow )
      {
	  fail( "Floating point underflow." );
      }
    else if	( csr.fc_struct.se_inexact )
      {
	  fail( "Floating point inexact." );
      }
    else
      {
	  fail( "Unknown floating point exception." );
      }

    signal( SIGFPE, catch_sigfpe );
    raise_exception();
}

void
enable_fpe_traps()
{
    handle_sigfpes( _ON,
		    _EN_OVERFL | _EN_DIVZERO | _EN_INVALID,
		    catch_sigfpe,
		    _ABORT_ON_ERROR,
		    0 );

    signal( SIGFPE, catch_sigfpe );
    signal( SIGTRAP, catch_sigtrap );
}
