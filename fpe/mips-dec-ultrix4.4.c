/*
   fpe/ultrix.c -- SIGFPE for Ultrix 4.2

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.
   See the file COPYING for license, warranty, and permission details.
*/

static char rcsid[] =
  "$Id: mips-dec-ultrix4.4.c,v 1.1.1.1 1996/04/17 05:56:14 ksh Exp $";

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>

#include <mips/cpu.h>
#include <mips/fpu.h>

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

      default:

	wipeout( "Caught SIGTRAP (%d).", code );
    }

    raise_exception();
}

static void
catch_sigfpe( sig, code, scp )
  int sig;
  int code;
  struct sigcontext *scp;
{
    switch( code ) {

      case EXC_OV:

	fail( "Integer overflow." );
	break;

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
	break;
    }

    raise_exception();
}

void
enable_fpe_traps()
{
    union fpc_csr csr;
    csr.fc_word = get_fpc_csr();
    csr.fc_struct.en_invalid = 1;
    csr.fc_struct.en_divide0 = 1;
    csr.fc_struct.en_overflow = 1;
    set_fpc_csr( (int) csr.fc_word );

    signal( SIGFPE, catch_sigfpe );
    signal( SIGTRAP, catch_sigtrap );
}
