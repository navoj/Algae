/*
   fpe/titan.c -- SIGFPE for Titan (a.k.a. Stardent, a.k.a. Ardent)

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.
   See the file COPYING for license, warranty, and permission details.
*/

static char rcsid[] =
  "$Id: titan.c,v 1.1.1.1 1996/04/17 05:56:14 ksh Exp $";

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>

#include <sys/signal.h>
#include <machine/mipsfpu.h>

#if !SKIP_LOCAL_INCLUDES
#include "exception.h"
#include "message.h"
#endif

static void
catch_sigfpe( sig, code, junk, scp )
  int sig;
  int code;
  int junk;
  struct sigcontext *scp;
{
    switch( code ) {

      case 0:

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
	}
	break;

      default:

	wipeout( "Invalid floating point exception." );

    }

    signal( SIGFPE, catch_sigfpe );
    raise_exception();
}

void
enable_fpe_traps()
{
    signal( SIGFPE, catch_sigfpe );

    SET_INPUT_IS_NAN();
    SET_DIVIDE_BY_ZERO();
    SET_INVALID_OPERAND();
    SET_ARITHMETIC_OVERFLOW();
}
