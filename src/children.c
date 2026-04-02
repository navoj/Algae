/*
   children.c -- Child processes, pipes, wait, and system.

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: children.c,v 1.4 2001/08/31 16:20:22 ksh Exp $";

#include "algae.h"
#include <signal.h>
#include "entity.h"
#include "scalar.h"
#include "children.h"
#include "file_io.h"
#include "profile.h"

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

extern int restricted;

/* Some systems are going to take a lot of work. */

#if !NO_EXEC

#if !HAVE_FCNTL_H
#include <file.h>
#else
#include <fcntl.h>
#endif

#if !NO_FCNTL
#define CLOSE_ON_EXEC(fd)   while (fcntl(fd, F_SETFD, 1) == -1 && \
				   errno == EINTR)
	/* some unix os use ioctl() instead which will
	   require a different macro */
#else
#define CLOSE_ON_EXEC(fd)
#endif /* NO_FCNTL */

static void PROTO (add_to_child_list, (int, int));
static struct child *PROTO (remove_from_child_list, (int));

static char *shell = "/bin/sh";

#endif /* NO_EXEC */

/*
   returns a read or write descriptor on a pipe to 
   process "name".
 */

FILE *
get_pipe (char *name, int type, int *pid_ptr)
{
#if !NO_EXEC
  int the_pipe[2], local_fd, remote_fd;

  if (restricted)
    {
      fail ("Restricted -- no process creation.");
      return (FILE *) 0;
    }

  if (name[0] == 0)
    {
      fail ("cannot exec \"\".");
      return (FILE *) 0;
    }

  if (pipe (the_pipe) == -1)
    return (FILE *) 0;
  local_fd = the_pipe[type == PIPE_OUT];
  remote_fd = the_pipe[type == PIPE_IN];
  /* to keep output ordered correctly */
  fflush (stdout);
  fflush (stderr);

  IGNORE_SIGPROF;

  switch (*pid_ptr = fork ())
    {
    case -1:
      while (close (local_fd) && errno == EINTR) ;
      while (close (remote_fd) && errno == EINTR) ;
      CATCH_SIGPROF;
      return (FILE *) 0;

    case 0:
      while (close (local_fd) && errno == EINTR) ;
      while (close (type == PIPE_IN) && errno == EINTR) ;
      while (dup (remote_fd) < 0 && errno == EINTR) ;
      while (close (remote_fd) && errno == EINTR) ;
      STOP_PROFILE_TIMER;
      DEFAULT_SIGPROF;
      signal (SIGINT, SIG_IGN);	/* don't pass SIGINT to children */
      while (execl (shell, shell, "-c", name, (char *) 0) && errno == EINTR) ;
      fail ("failed to exec %s -c %s (%s)",
	    shell, name, strerror (errno));
      fflush (stderr);
      _exit (128);

    default:
      while (close (remote_fd) && errno == EINTR) ;
      /* we could deadlock if future child inherit the local fd ,
         set close on exec flag */
      CLOSE_ON_EXEC (local_fd);
      CATCH_SIGPROF;
      break;
    }

  {
    FILE *fp;
    fp = fdopen (local_fd, type == PIPE_IN ? "r" : "w");

    /* don't buffer out pipes */
    if (type == PIPE_OUT)
      setbuf (fp, (char *) 0);

    return fp;
  }
#else
  detour ("Pipes are not implemented on this system.");
  raise_exception ();
#endif /* NO_EXEC */
}


#if !NO_EXEC
/*------------ children ------------------*/

/* we need to wait for children at the end of output pipes to
   complete so we know any files they have created are complete */

/* sometimes wait() catches other processes other than the one we
   want those go on a linked linear list */

static struct child
{
  int pid;
  int exit_status;
  struct child *link;
}
 *child_list;

static void
add_to_child_list (int pid, int exit_status)
{
  register struct child *p =
  (struct child *) MALLOC (sizeof (struct child));

  p->pid = pid;
  p->exit_status = exit_status;
  p->link = child_list;
  child_list = p;
}

static struct child *
remove_from_child_list (int pid)
{
  register struct child *p = child_list;
  struct child *q = (struct child *) 0;

  while (p)
    if (p->pid == pid)
      {
	if (q)
	  q->link = p->link;
	else
	  child_list = p->link;
	break;
      }
    else
      {
	q = p;
	p = p->link;
      }

  return p;			/* null return if not in the list */
}
#endif /* NO_EXEC */

/* wait for a specific child to complete and return its 
   exit status.  Exit status is transformed as with
   "/bin/sh" */

int
wait_for (int pid)
{
#if !NO_EXEC
  int exit_status;
  struct child *p;
  int id;

  /* see if an earlier wait() caught our child */
  if (p = remove_from_child_list (pid))
    {
      exit_status = p->exit_status;
      FREE (p);
    }
  else				/* need to really wait */
    while ((id = wait (&exit_status)) != pid)
      {
	if (id == -1)
	  {
	    if (errno == EINTR) continue;
	    return 0;
	  }
	else
	  {			/* we got the exit status of another child
				   put it on the child list and try again */
	    add_to_child_list (id, exit_status);
	  }
      }

  /* make standard /bin/sh conversion of exit status */
  return (exit_status & 0xff)
    ? ((exit_status & 0xff) + 128)
    : ((exit_status & 0xff00) >> 8);
#else
  return 0;
#endif /* NO_EXEC */
}

ENTITY *
bi_system (ENTITY *command)
{
#if !NO_EXEC
  int retval;
  int pid;
  SCALAR *cmd = (SCALAR *) scalar_entity (command);

  if (restricted)
    {
      fail ("Restricted -- no process creation.");
      retval = -1;
    }
  else if (cmd->type != character)
    {
      fail ("Non character scalar passed to \"system\".");
      retval = -1;
    }
  else
    {
      fflush (stdout);
      fflush (stderr);
      /* keeps output correctly ordered */

      IGNORE_SIGPROF;

      switch (pid = fork ())
	{
	case -1:
	  fail ("Unable to create a new process (%s).", strerror (errno));
	  retval = -1;
	  break;

	case 0:

	  STOP_PROFILE_TIMER;
	  DEFAULT_SIGPROF;
	  while (execl (shell, shell, "-c", cmd->v.character, (char *) 0) &&
		 errno == EINTR) ;
	  /* exec failed!! -- unexpected */
	  fail ("exec of %s failed! (%s)", shell, strerror (errno));
	  fflush (stderr);
	  _exit (127);

	default:
	  retval = wait_for (pid);
	  break;
	}

      CATCH_SIGPROF;
    }

  delete_scalar (cmd);
  if (retval == -1)
    raise_exception ();

  return int_to_scalar (retval);
#else
  detour ("The \"system\" function is not implemented on this system.");
  raise_exception ();
#endif /* NO_EXEC */
}

ENTITY *
bi_cd (int n, ENTITY *path)
{
  /* change working directory */

  char *s;

  if (path)
    {
      SCALAR *p = (SCALAR *) scalar_entity (path);

      if (p->type != character)
	{
	  fail ("Non character scalar passed to \"cd\".");
	  delete_scalar (p);
	  raise_exception ();
	}

      s = dup_char (p->v.character);
      delete_scalar (p);

    }
  else
    {

      s = getenv ("HOME");
      s = s ? dup_char (s) : NULL_string;

    }

  while (chdir (s))
    if (errno != EINTR)
      {
	fail ("Can't change directory to \"%s\" (%s).", s, strerror (errno));
	FREE_CHAR (s);
	raise_exception ();
      }

  FREE_CHAR (s);
  return int_to_scalar (1);
}

ENTITY *
bi_getenv (ENTITY *name)
{
  /* get value of an environment variable */

  char *s;
  SCALAR *p = (SCALAR *) scalar_entity (name);

  if (p->type != character)
    {
      fail ("Invalid type (%s) in \"getenv\".", type_string[p->type]);
      delete_scalar (p);
      raise_exception ();
    }

  s = getenv (p->v.character);

  delete_scalar (p);
  return s ? char_to_scalar (dup_char (s)) : NULL;
}
