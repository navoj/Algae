/*
   init.c -- Initialization.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: init.c,v 1.13 2003/08/07 04:48:19 ksh Exp $";

#include "algae.h"

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#ifdef apollo
#include <apollo/time.h>
#endif
#endif

#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#include <signal.h>
#include <termios.h>

#if HAVE_MCHECK_H
#include <mcheck.h>
#endif

#include "ptypes.h"
#include "psr.h"
#include "code.h"
#include "parse.h"
#include "datum.h"
#include "rand.h"
#include "print.h"
#include "read.h"
#include "binop.h"
#include "scalar.h"
#include "vector.h"
#include "version.h"
#include "profile.h"
#include "printf.h"
#include "getopt.h"
#include "file_io.h"

void PROTO (enable_fpe_traps, (void));

int debug_level;
int stdin_is_interactive;
int whitespace = 1;	/* add whitespace with printing statements */
int restricted = 0;	/* if true, no process creation */

#if ENABLE_PROFILER
int profiling;			/* on when profiling */
#endif

#if HAVE_LIBREADLINE
int use_readline = 1;
static void PROTO (initialize_readline, (void));
static char *algae_histfile;
extern int PROTO (write_history, (char *));
extern int PROTO (history_truncate_file, (char *, int));
extern int PROTO (read_history, (char *));
#endif

int da_flag;
int skip_rc0, skip_rc1;
extern int yydebug;
char *progname;

/* We keep a list of pointers to the input scripts. */
char **script_inputs;
int script_inputs_len;

DATUM *process_id;		/* Initialized to the pid. */

char **file_args;
static char *no_args[] =
{"-", (char *) 0};
static int no_stdin_default = 0;	/* If no file specified, read stdin. */

static void PROTO (load_predefined_vars, (void));

/* Interface to getopt. */

static struct option longopts[] =
{
  {"debug", 1, NULL, 'd'},
  {"disassemble", 0, NULL, 'D'},
  {"help", 0, NULL, 'h'},
  {"interactive", 0, NULL, 'i'},
  {"nowhite", 0, NULL, 'n'},
  {"norc", 0, NULL, 's'},
  {"noreadline", 0, NULL, 'r'},
  {"nostartup", 0, NULL, 'S'},
  {"nostdin", 0, NULL, 'x'},
  {"profile", 0, NULL, 'p'},
  {"restrict", 0, NULL, 'R'},
  {"script", 1, NULL, 'e'},
  {"version", 0, NULL, 'V'},
  {NULL, 0, NULL, 0}
};

void
initialize (int argc, char **argv)
{
  int c, longind;

  {
    char *p = strrchr (argv[0], '/');
    progname = p ? p + 1 : argv[0];
  }

#if DEBUG
#if HAVE_MTRACE
  mtrace ();	/* To trace, set env. var. MALLOC_TRACE to file name. */
#endif
  debug_level = 1;
#endif

  load_predefined_vars ();	/* Must precede calls to `fail', etc. */
  enable_fpe_traps ();		/* floating point exceptions */

  /* get options: */

  while ((c = getopt_long (argc, argv, "d:De:hinprRsSVx",
			   longopts, &longind)) != EOF)
    {
      switch (c)
	{
        case 'x':		/* Don't read stdin by default. */

          no_stdin_default = 1;
          break;

        case 'R':		/* Restricted -- no process creation. */

          restricted = 1;
          break;

        case 'n':		/* No whitespace with printing statements. */

          whitespace = 0;
          break;
          
	case 'e':		/* Collect the input scripts. */

          script_inputs = script_inputs_len++ ?
            REALLOC (script_inputs, script_inputs_len * sizeof (char *)) :
            MALLOC (sizeof (char *));
          script_inputs [script_inputs_len-1] = optarg;
	  break;

	case 'i':		/* Use interactive mode for stdin. */

	  stdin_is_interactive = 1;
	  break;

	case 'p':		/* Turn on profiling. */

#if ENABLE_PROFILER
	  profiling = 1;
	  initialize_profiler ();
#else
	  xfprintf (stderr, "%s: execution profiler disabled\n", progname);
	  algae_exit (1);
#endif
	  break;

	case 'r':		/* Don't use readline. */

#if HAVE_LIBREADLINE
	  use_readline = 0;
#endif
	  break;

	case 's':		/* Don't read user's ".algae" file. */

	  skip_rc1 = 1;
	  break;

	case 'S':		/* Don't read any startup files. */

	  skip_rc0 = skip_rc1 = 1;
	  break;

	case 'D':		/* Disassemble. */

	  da_flag = 1;
	  break;

	case 'V':		/* The version and date. */

	  printf (VERSION_STRING);
	  break;

	case 'd':		/* Set debug level. */

	  debug_level = atoi (optarg);
	  break;

	case 'h':

	  xfprintf (stderr,
		    "%s: usage: %s [-DhinprRsSVx] [-d num] [-e script] [file ...] [-]\n",
		    progname, progname);
	  algae_exit (0);

	default:

	  algae_exit (1);
	}
    }

  argc -= optind;
  argv += optind;

  file_args = argc ? argv : no_args + no_stdin_default;

  if (debug_level > 2)
    yydebug = 1;

  rand_init ();			/* random number seed */
  bi_init ();			/* builtin functions */
  set_stderr ();

#if HAVE_LIBREADLINE
  if (use_readline)
    initialize_readline ();
#endif

  /* Have to initialize FORTRAN I/O on some machines. */

#ifdef FORTRAN_INIT
  FORTRAN_INIT
#endif

}

static void
load_predefined_vars (void)
{
  SYMTAB *stp;

  stp = insert ("$beep");
  stp->scope = 0;
  vociferant = stp->stval.datum = new_DATUM ();
  stp->stval.datum->type = D_INT;
  IVAL (stp->stval.datum) = 0;

  /*
   * $program: The current program name.  This entity points to the same
   * data as the global variable `progname'.  The reference count is
   * increased by one so we never try to free it.
   */

  stp = insert ("$program");
  stp->scope = 0;
  stp->stval.datum = new_DATUM ();
  stp->stval.datum->type = D_ENTITY;
  stp->stval.datum->data.ptr = copy_scalar (char_to_scalar (progname));

  /* Get the window (terminal) width. */

  stp = insert ("$term_width");
  stp->scope = 0;
  term_width = stp->stval.datum = new_DATUM ();
  stp->stval.datum->type = D_INT;
  {
    struct winsize w;
    while (ioctl (1, TIOCGWINSZ, &w) < 0)
      {
	/*
	 * Call to get window size failed.  If it was interrupted, try
	 * again; otherwise just use default.
	 */

	if (errno != EINTR)
	  {
	    w.ws_col = 80;
	    break;
	  }
      }
    IVAL (stp->stval.datum) = w.ws_col;
  }

  stp = insert ("$read");
  stp->scope = 0;
  this_many_read = stp->stval.datum = new_DATUM ();
  stp->stval.datum->type = D_INT;
  IVAL (stp->stval.datum) = 0;

  stp = insert ("$digits");
  stp->scope = 0;
  num_digits = stp->stval.datum = new_DATUM ();
  stp->stval.datum->type = D_INT;
  IVAL (stp->stval.datum) = 4;

  stp = insert ("$pid");
  stp->scope = 0;
  process_id = stp->stval.datum = new_DATUM ();
  stp->stval.datum->type = D_INT;
  IVAL (stp->stval.datum) = getpid ();

  stp = insert ("$prompt");
  stp->scope = 0;
  prompt_string = stp->stval.datum = new_DATUM ();
  stp->stval.datum->type = D_ENTITY;
  stp->stval.datum->data.ptr =
    binop_scalar_character (BO_APPEND,
			    (SCALAR *) char_to_scalar (dup_char ("> ")),
			    (SCALAR *) char_to_scalar (dup_char ("  ")));
}

#if HAVE_LIBREADLINE

extern void PROTO (using_history, (void));

void
algae_write_history (void)
{
  write_history (algae_histfile);
  history_truncate_file (algae_histfile, 500);
}

static void
initialize_readline (void)
{
  using_history ();

  /*
   * Read the history file.  Use the file named by the environment
   * variable ALGAE_HISTFILE or else the default "~/.algae_history".
   * On error, continue without warning.
   */

  {
    char *name = getenv ("ALGAE_HISTFILE");

    if (!name || !*name)
      {
        name = getenv ("HOME");
        name = append_char (name ? name : NULL_string, "/.algae_history");
      }
    read_history (name);

    algae_histfile = name;
  }

  /* Write and truncate the history file at exit. */

  algae_atexit (algae_write_history);
}

#endif
