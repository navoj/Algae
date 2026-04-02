/*
   fpe/ymp.c -- SIGFPE code

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.
   See the file COPYING for license, warranty, and permission details.
*/

static char rcsid[] =
  "$Id: ymp-cray-unicos.c,v 1.3 1997/02/21 09:53:45 ksh Exp $";

#ifndef CRAY
choke me
#endif

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#if !SKIP_LOCAL_INCLUDES
#include "exception.h"
#include "message.h"
#endif

static void
catch_sigfpe( sig )
     int sig;
{
  /*
   * This is the signal handler for floating point exceptions.  As far
   * as I can tell, the YMP doesn't give us any way to determine what
   * happened (overflow, divide by zero, etc.).
   */

  fail( "Floating point exception." );
  raise_exception();
}

void
enable_fpe_traps()
{
  /*
   * On the YMP, floating point exceptions are always "enabled".  We just
   * have to specify the action (that is, to call the signal handler).
   */

  struct sigaction act;

  act.sa_handler = catch_sigfpe;
  sigemptyset( &(act.sa_mask) );

  /*
   * I wouldn't have thought that any flags would need to be set.  I think
   * that what happens, though, is that without the NODEFER flag the
   * SIGFPE signal gets blocked when the handler is called and then never
   * gets unblocked.  Thus the first fpe gets handled just fine, but after
   * that all are ignored.  Probably this is because we're doing a long
   * jump out of the signal handler instead of returning.
   */

  act.sa_flags = SA_NODEFER;

  if ( sigaction( SIGFPE, &act, NULL ) )
    wipeout( "Bad sigaction call." );
}
