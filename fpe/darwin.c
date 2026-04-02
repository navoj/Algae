/*
   fpe/darwin.c -- SIGFPE code for Mac OS X

   This is from a patch by Matthias Neeracher.
*/

static char rcsid[] =
  "$Id: darwin.c,v 1.1 2003/09/06 17:06:52 ksh Exp $";

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <mach/mach.h>

#if !SKIP_LOCAL_INCLUDES
#include "exception.h"
#include "message.h"
#endif

/*
 * On Mach, we need a mindbogglingly complex setup for floating point errors.
 * Not the least of the hassles is that we have to do the whole thing from
 * a different thread.
 */
void * fpe_enabler(void * parent)
{
	mach_port_t		victim = (mach_port_t)parent;
	mach_msg_type_number_t	count;

	ppc_thread_state_t	ts;
	ppc_float_state_t	fs;

	/* First enable the right FP exception conditions */
	count = PPC_FLOAT_STATE_COUNT;
	thread_get_state(victim, PPC_FLOAT_STATE, (thread_state_t)&fs, &count);
	/* Enable VE OE ZE, Disable UE XE */
	fs.fpscr = (fs.fpscr & ~0x1FFFFF28) | 0x0D0;
	thread_set_state(victim, PPC_FLOAT_STATE, (thread_state_t)&fs, count);

	/* Now, enable FP exceptions as such */
	count = PPC_THREAD_STATE_COUNT;
	thread_get_state(victim, PPC_THREAD_STATE, (thread_state_t)&ts, &count);
	/* Set FE0 = FE1 = 1 */
	ts.srr1 |= 0x0900;
	thread_set_state(victim, PPC_THREAD_STATE, (thread_state_t)&ts, count);
	
	return 0;
}

static void
catch_sigfpe (sig)
  int sig;
{
  fail ("Floating point exception.");
  raise_exception ();
}

void
enable_fpe_traps ()
{
  pthread_t enabler;
  pthread_create (&enabler, NULL, fpe_enabler, mach_thread_self());
  pthread_join (enabler, NULL);

  signal (SIGFPE, catch_sigfpe);
}
