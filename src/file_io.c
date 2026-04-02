/*
   file_io.c -- file associations

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: file_io.c,v 1.3 1997/02/21 09:53:56 ksh Exp $";

#include "algae.h"
#include "file_io.h"
#include "entity.h"
#include "scalar.h"
#include "children.h"
#include "main.h"

#if FILE_NOT_FCNTL
#include <file.h>
#else
#include <fcntl.h>
#endif

static void PROTO (close_out_pipes, (void));

/* open files are stored on lists of these */
typedef struct file
  {
    struct file *link;
    FILE *fp;
    int mode;
    int pid;			/* for pipes */
    char *name;
  }
FILENODE;

static FILENODE *file_list;

/* Tuck away some error messages. */

char *read_fail_msg = "Can't read file (%s).";
char *write_fail_msg = "Can't write file (%s).";
char *eof_msg = "unexpected end of file";

/* The filename "/dev/stderr" is associated with
   FILE *stderr by convention */

void
set_stderr (void)
{
  char *s = "/dev/stderr";

  file_list = MALLOC (sizeof (FILENODE));
  file_list->link = (FILENODE *) 0;
  file_list->fp = stderr;
  file_list->mode = FILE_OUTPUT;
  file_list->name = strcpy (MALLOC (strlen (s) + 1), s);

  algae_atexit (close_out_pipes);
}

/*
   find a stored fileptr for name
   or open name

   storage is linked linear list with move to
   the front

 */

FILE *
find_file (char *name, int mode)
{
  FILENODE *p = file_list;
  FILENODE *q = (FILENODE *) 0;
  FILE *fp;
  int pid;

  assert (name != (char *) 0);

  while (p)
    if (strcmp (p->name, name) == 0)
      {
	/* found it */
	FREE_CHAR (name);

	if ((p->mode & IO_MASK) != mode)
	  {
	    fail ("%s is already open for %s", p->name,
		  mode == FILE_INPUT ? "output" : "input");
	    return (FILE *) 0;
	  }

	if (!q)
	  return p->fp;		/* at front */
	else
	  {			/* move to the front */
	    q->link = p->link;
	    goto put_at_front;
	  }
      }
    else
      {
	q = p;
	p = p->link;
      }


  /* not there */

  if (name[0] == '!')
    mode |= PIPE_FLAG;

  if ((mode & IO_MASK) == FILE_INPUT)
    {
      fp = name[0] == '!' ? get_pipe (name + 1, PIPE_IN, &pid)
	: efopen (name, "r");
    }
  else if (name[0] == '+')
    fp = efopen (name + 1, "a");

  else if (name[0] == '!')
    fp = get_pipe (name + 1, PIPE_OUT, &pid);
  else
    fp = efopen (name, "w");

  if (!fp)
    {
      FREE_CHAR (name);
      return fp;
    }

  p = MALLOC (sizeof (FILENODE));
  p->name = name;
  p->mode = mode;
  p->pid = pid;
  p->fp = fp;

put_at_front:

  p->link = file_list;
  file_list = p;
  return p->fp;
}

/*
   close a file by name
   return 0 if really an open file else -1

   if file is pipe return exit status of pipe process

   name is READONLY(not duped)
 */

int
close_file (char *name)
{
  FILENODE *p = file_list;
  FILENODE *q = (FILENODE *) 0;
  int retval;

  assert (name != (char *) 0);

  while (p)
    if (strcmp (p->name, name) == 0)	/* found */
      {
	/* delete */
	if (q)
	  q->link = p->link;
	else
	  file_list = p->link;

	if (p->fp != stdin)
	  (void) fclose (p->fp);

	if (p->mode & PIPE_FLAG)
	  retval = wait_for (p->pid);
	else
	  retval = 0;

	FREE_CHAR (p->name);
	FREE (p);

	return retval;
      }
    else
      {
	q = p;
	p = p->link;
      }

  return -1;
}

static void
close_out_pipes (void)
{

  FILENODE *p = file_list;

  while (p)
    {
      if (p->mode == PIPE_OUT)
	{
	  (void) fclose (p->fp);
	  (void) wait_for (p->pid);
	}
      p = p->link;
    }

#if DEBUG			/* delete the list for malloc check */
  {
    FILENODE *q;

    p = file_list;
    while (p)
      {
	if (p->mode != PIPE_OUT && p->fp != stderr)
	  fclose (p->fp);
	FREE_CHAR (p->name);
	q = p;
	p = p->link;
	FREE (q);
      }
  }
#endif
}

ENTITY *
bi_close (ENTITY *fname)
{
  int r;
  SCALAR *fn = (SCALAR *) scalar_entity (fname);

  if (fn->type == character)
    r = close_file (fn->v.character);
  else
    r = -1;

  delete_scalar (fn);
  return r == -1 ? NULL : int_to_scalar (r);
}

ENTITY *
bi_source (ENTITY *fname)
{
  /* Read commands from file `fname'. */

  FILE *fp;
  SCALAR *fn = (SCALAR *) scalar_entity (fname);
  int r = -1;

  if (fn->type == character)
    fp = find_file (dup_char (fn->v.character), FILE_INPUT);
  /* find_file() used so that  "!command" works */
  else
    fp = (FILE *) 0;

  if (fp)
    {
      /*
       * This leaks memory!  The file name is dup'ed, but never gets
       * freed.  I need to clean up file name handling so that the
       * names are stored within the user function structures.
       */
      r = parse_and_exec (fp, dup_char (fn->v.character));
      (void) close_file (fn->v.character);
    }

  delete_scalar (fn);
  return r == -1 ? NULL : int_to_scalar (r);
}

FILE *
efopen (char *name, char *mode)
{
  FILE *fp;

  if (mode[0] == 'r' && name[0] == '-' && name[1] == 0)
    fp = stdin;
  else if (!(fp = fopen (name, mode)))
    warn ("Can't open %s (%s).", name, strerror (errno));

  return fp;
}

ENTITY *
bi_exec (ENTITY *cmd)
{
  /* Parse and execute the given string. */

  char *c = entity_to_string (cmd);
  /*
   * This leaks memory!  The file name is dup'ed, but never gets
   * freed.  I need to clean up file name handling so that the
   * names are stored within the user function structures.
   */
  int r = parse_and_exec (NULL, c);
  return int_to_scalar (r);
}
