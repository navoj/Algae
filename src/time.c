/*
   time.c -- Return user time.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: time.c,v 1.2 2002/05/09 18:37:54 ksh Exp $";

#include "time.h"
#include "entity.h"
#include "datum.h"
#include "scalar.h"
#include "fortran.h"

#ifdef VAXC
#include <time.h>
#else
#include <sys/types.h>
#include <sys/times.h>
#include <sys/param.h>
#endif

#ifndef CLK_TCK
#ifdef HZ
#define CLK_TCK	HZ
#else
#define CLK_TCK sysconf(_SC_CLK_TCK)
#endif
#endif

ENTITY *
bi_time (void)
{
  /*
   * This function returns the user time, in seconds, for the
   * current process.
   */

#ifdef VAXC
  tbuffer_t t;
#else
  struct tms t;
#endif

  times (&t);

#ifdef VAXC
  return (real_to_scalar ((REAL) t.proc_user_time / 100.0));
#else
  return (real_to_scalar ((REAL) t.tms_utime / CLK_TCK));
#endif

}

#if !HAVE_ETIME
float
ETIME_FUNCTION (float tarray[])
{
  /*
   *  I'm not sure what the Fortran standards situation is with this
   *  function.  Many folks assume that it's an intrinsic function,
   *  but it's missing on many systems.
   */

#ifdef VAXC
  tbuffer_t t;
#else
  struct tms t;
#endif

  times (&t);

#ifdef VAXC
  tarray[0] = (REAL) t.proc_user_time / 100.0;
  tarray[1] = (REAL) t.proc_system_time / 100.0;
#else
  tarray[0] = (REAL) t.tms_utime / CLK_TCK;
  tarray[1] = (REAL) t.tms_stime / CLK_TCK;
#endif

  return tarray[0] + tarray[1];
}
#endif
