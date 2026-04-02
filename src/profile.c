/*
   profile.c -- execution profiling

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: profile.c,v 1.2 1997/02/21 09:54:04 ksh Exp $";

#include <signal.h>

#include <stdio.h>
#include "algae.h"
#include "psr.h"
#include "profile.h"
#include "printf.h"

#if ENABLE_PROFILER

extern char *progname;

#define LINE_CHUNKS 100		/* allocate this many lines at a time */

static struct profile prof_tail =
{"", NULL, 0, &prof_tail};
static struct profile *prof_files = &prof_tail;
static struct profile *prof_curr_file = &prof_tail;
static unsigned int *prof_curr_line;

RETSIGTYPE
catch_sigprof (int sig)
{
  (*prof_curr_line)++;
#if RESET_SIGNAL_HANDLER
  CATCH_SIGPROF;
#endif
}

void
initialize_profiler (void)
{
  struct itimerval value;

  value.it_interval.tv_sec = 0;
  value.it_interval.tv_usec = PROFILE_INTERVAL;
  value.it_value.tv_sec = 0;
  value.it_value.tv_usec = PROFILE_INTERVAL;

  IGNORE_SIGPROF;

  if (setitimer (ITIMER_PROF, &value, NULL))
    wipeout ("Error setting profile timer.");

  prof_update_file ("stdin");
  CATCH_SIGPROF;

  algae_atexit (write_profile);
}

void
stop_profile_timer (void)
{
  struct itimerval value;

  value.it_interval.tv_sec = 0;
  value.it_interval.tv_usec = 0;
  value.it_value.tv_sec = 0;
  value.it_value.tv_usec = 0;

  if (setitimer (ITIMER_PROF, &value, NULL))
    wipeout ("Error resetting profile timer.");
}

void
write_profile (void)
{
  FILE *fp;
  int i;
  struct profile *p = prof_files;

  stop_profile_timer ();
  IGNORE_SIGPROF;

  if (!(fp = fopen ("algae.out", "w")))
    {
      xfprintf (stderr, "%s: Can't open \"algae.out\".\n", progname);
    }
  else
    {
      while (p != &prof_tail)
	{
	  xfprintf (fp, "%s\n", p->name);
	  for (i = 0; i < p->length; i++)
	    {
	      if (p->count[i])
		xfprintf (fp, "\t%d\t%d\n", i + 1, p->count[i]);
	    }
	  p = p->next;
	}
    }
}

static struct profile *
lookup_filename (char *name, struct profile *p)
{
  /* The name must exist!  Put it in prof_tail before coming here. */

  while (strcmp (p->name, name))
    p = p->next;

  return p;
}

static struct profile *
add_filename (char *name, struct profile *p)
{
  struct profile *q = MALLOC (sizeof (struct profile));
  q->name = dup_char (name);
  q->count = CALLOC (LINE_CHUNKS, sizeof (int));
  q->length = LINE_CHUNKS;
  q->next = p;

  return q;
}

void
prof_update_file (char *name)
{
  /* Lookup the file name. */

  prof_tail.name = name;
  prof_curr_file = lookup_filename (name, prof_files);

  /* If this is a new one for us, add it to the list. */

  if (prof_curr_file == &prof_tail)
    prof_files = prof_curr_file = add_filename (name, prof_files);

  prof_update_line (1);
}

void
prof_update_line (int line)
{
  struct profile *p = prof_curr_file;

  if (line > p->length)
    {
      p->count = REALLOC (p->count, (p->length + LINE_CHUNKS) * sizeof (int));
      memset (p->count + p->length, 0, LINE_CHUNKS * sizeof (int));
      p->length += LINE_CHUNKS;
    }

  prof_curr_line = p->count + line - 1;
}

#endif /* ENABLE_PROFILER */
