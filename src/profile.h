/*
   profile.h

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: profile.h,v 1.2 2003/08/01 04:57:48 ksh Exp $ */

#ifndef  SIGPROF_H
#define  SIGPROF_H	1

/* Can't run the profiler without setitimer(3C). */

#if !HAVE_SETITIMER
#undef ENABLE_PROFILER
#define ENABLE_PROFILER 0
#endif

#if !ENABLE_PROFILER

#define PROF_UPDATE_LINE(n)	/* empty */
#define PROF_UPDATE_FILE(n)	/* empty */
#define IGNORE_SIGPROF		/* empty */
#define CATCH_SIGPROF		/* empty */
#define DEFAULT_SIGPROF		/* empty */
#define STOP_PROFILE_TIMER	/* empty */

#else

struct profile
  {
    char *name;
    unsigned int *count;
    int length;
    struct profile *next;
  };

#if HAVE_SIGACTION && HAVE_SA_RESTART
#define CATCH_SIGPROF		 { \
				   struct sigaction act; \
				   act.sa_handler = catch_sigprof; \
				   sigemptyset (&act.sa_mask); \
				   act.sa_flags = SA_RESTART; \
				   if (sigaction (SIGPROF, &act, NULL)) \
				     wipeout ("Bad sigaction call."); \
				 }
#else
#if HAVE_SIGSET
#define CATCH_SIGPROF		sigset (SIGPROF, catch_sigprof)
#else
#define CATCH_SIGPROF		signal (SIGPROF, catch_sigprof)
#endif
#endif

#define IGNORE_SIGPROF		signal (SIGPROF, SIG_IGN)
#define DEFAULT_SIGPROF		signal (SIGPROF, SIG_DFL)

#define STOP_PROFILE_TIMER	stop_profile_timer ()

#define PROF_UPDATE_LINE( n )	if (profiling) { \
				    IGNORE_SIGPROF; \
				    prof_update_line (n); \
				    CATCH_SIGPROF; \
				} else ;

#define PROF_UPDATE_FILE( n )	if (profiling) { \
				    IGNORE_SIGPROF; \
				    prof_update_file (n); \
				    CATCH_SIGPROF; \
				} else ;

RETSIGTYPE PROTO (catch_sigprof, (int sig));
void PROTO (initialize_profiler, (void));
void PROTO (stop_profile_timer, (void));
void PROTO (prof_update_file, (char *name));
void PROTO (prof_update_line, (int line));
void PROTO (write_profile, (void));

#endif /* ENABLE_PROFILER */

#endif
