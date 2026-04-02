/*
   fpe/alpha-dec-osf3.0.c -- SIGFPE code

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.
   See the file COPYING for license, warranty, and permission details.
*/

static char rcsid[] =
  "$Id: alpha-dec-osf3.0.c,v 1.1.1.1 1996/04/17 05:56:14 ksh Exp $";

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>

#include <machine/fpu.h>

#if !SKIP_LOCAL_INCLUDES
#include "exception.h"
#include "message.h"
#endif

static void
catch_sigfpe( sig )
  int sig;
{
    switch( sig ) {

      case FPE_INTOVF_TRAP:
      case FPE_INTOVF_FAULT:

	fail( "Integer overflow." );
	break;

      case FPE_FLTOVF_TRAP:
      case FPE_FLTOVF_FAULT:

	fail( "Floating point overflow." );
	break;

      case FPE_FLTDIV_TRAP:
      case FPE_FLTDIV_FAULT:

	fail( "Divide by zero." );
	break;

      default:

	fail( "Floating point exception.", sig );
	break;
    }

    raise_exception();
}

void
enable_fpe_traps()
{
    unsigned long csr = ieee_get_fp_control();
    csr |= IEEE_TRAP_ENABLE_INV | IEEE_TRAP_ENABLE_DZE | IEEE_TRAP_ENABLE_OVF;
    ieee_set_fp_control( csr );

    signal( SIGFPE, catch_sigfpe );
}
