/*
 * fpe/rs6000-ibm-aix3.2.5.c -- SIGFPE for IBM RS6000 with AIX 3.2.5
 *
 * Copyright (C) 1995-2001  K. Scott Hunziker
 * See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
  "$Id: rs6000-ibm-aix3.2.5.c,v 1.3 2001/08/06 20:04:15 ksh Exp $";

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>

#include <fpxcp.h>
#include <fptrap.h>

#if !SKIP_LOCAL_INCLUDES
#include "exception.h"
#include "message.h"
#endif

void enable_fpe_traps ();

/* Signal handler for floating point exceptions. */

static void
catch_sigfpe (sig, code, scp)
     int sig;
     int code;
     struct sigcontext *scp;
{
  fp_sh_info_t flt_context;

  fp_sh_info (scp, &flt_context, FP_SH_INFO_SIZE);

  switch (flt_context.trap & !FP_INVALID)
    {
      case FP_DIV_BY_ZERO:

        fail ("Divide by zero.");
	break;

      case FP_OVERFLOW:

	fail ("Floating point overflow.");
	break;

      case FP_UNDERFLOW:

	fail ("Floating point underflow.");
	break;

      case FP_INEXACT:

	fail ("Floating point inexact.");
	break;

      default:

	if (flt_context.trap)
	  fail ("Invalid floating point operation.");
	else
	  fail ("Unknown floating point exception.");
	break;
    }

  enable_fpe_traps ();
  raise_exception ();
}

void
enable_fpe_traps ()
{
  struct sigaction act;

  act.sa_handler = (void (*)()) catch_sigfpe;	/* the signal handler */
  sigemptyset (&act.sa_mask);			/* no other signals blocked */
  act.sa_flags = 0;

  if (sigaction (SIGFPE, &act, NULL))
    wipeout ("Bad sigaction call.");

  fp_enable (TRP_INVALID | TRP_DIV_BY_ZERO | TRP_OVERFLOW);

  if (fp_trap (FP_TRAP_FASTMODE) == FP_TRAP_OFF)
    wipeout ("This platform does not support floating point trapping.");
}
