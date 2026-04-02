/*
   message.c -- Print messages.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: message.c,v 1.4 2003/09/06 17:08:28 ksh Exp $";

#include "algae.h"
#include "scalar.h"
#include "datum.h"
#include "print.h"
#include "vargs.h"
#include "psr.h"

#define TERM_WIDTH datum_to_int( copy_datum( term_width, term_width ) )

#if HAVE_STDARG_H
static int PROTO (algae_fprintf, (FILE *, char *,...));
static int PROTO (algae_vfprintf,
		    (FILE * stream, char *format, va_list arg));
#else
static algae_fprintf ();
static algae_vfprintf ();
#endif

char printf_buf[PRINTF_BUF_SIZE];
extern char *progname;

DATUM *vociferant;
#define BEEP ( datum_test( copy_datum(vociferant,vociferant) ) ? "\007" : "" )

ENTITY *
bi_file (void)
{
  return char_to_scalar (dup_char (curr_file));
}

ENTITY *
bi_line (void)
{
  return int_to_scalar (curr_line_no);
}

void
p_error VA_ALIST (char *fmt)

     /* parse errors */

     char *cf = NULL;
     va_list ap;
     VA_START (ap, char *, fmt);

     /* don't overflow the print buffer */

     if (strlen (curr_file) > 128)
       {
         cf = MALLOC (128);
         memcpy (cf, curr_file, 124);
         memcpy (cf+124, "...", 4);
       }
     
     algae_fprintf (stderr, "%s: %s: line %d: parse error%s: ",
		   progname, cf ? cf : curr_file, curr_line_no, BEEP);
     algae_vfprintf (stderr, fmt, ap);
     algae_fprintf (stderr, "\n");

     TFREE (cf);
     va_end (ap);
}

void
wipeout VA_ALIST (char *fmt)

     /* Total wipeout.  Splash one user. */

     va_list ap;

     VA_START (ap, char *, fmt);

     fprintf (stderr, "%s: %s: line %d: wipeout: ",
              progname, curr_file, curr_line_no);
     vfprintf (stderr, fmt, ap);
     putc ('\n', stderr);

     va_end (ap);
     abort ();
}

void
detour VA_ALIST (char *fmt)

     /* Code that's supposed to work, but doesn't yet. */

     char *cf = NULL;
     va_list ap;
     VA_START (ap, char *, fmt);

     /* don't overflow the print buffer */

     if (strlen (curr_file) > 128)
       {
         cf = MALLOC (128);
         memcpy (cf, curr_file, 124);
         memcpy (cf+124, "...", 4);
       }
     
     algae_fprintf (stderr, "%s: %s: line %d: run time detour%s: ",
		   progname, cf ? cf : curr_file, curr_line_no, BEEP);
     algae_vfprintf (stderr, fmt, ap);
     algae_fprintf (stderr, "\n");

     TFREE (cf);
     va_end (ap);
}

void
fail VA_ALIST (char *fmt)

     /* A run-time error. */

     char *cf = NULL;
     va_list ap;
     VA_START (ap, char *, fmt);

     /* don't overflow the print buffer */

     if (strlen (curr_file) > 128)
       {
         cf = MALLOC (128);
         memcpy (cf, curr_file, 124);
         memcpy (cf+124, "...", 4);
       }
     
     algae_fprintf (stderr, "%s: %s: line %d: run time error%s: ",
		   progname, cf ? cf : curr_file, curr_line_no, BEEP);
     algae_vfprintf (stderr, fmt, ap);
     algae_fprintf (stderr, "\n");

     TFREE (cf);
     va_end (ap);
}

void
warn VA_ALIST (char *fmt)

     /* A warning message. */

     char *cf = NULL;
     va_list ap;
     VA_START (ap, char *, fmt);

     /* don't overflow the print buffer */

     if (strlen (curr_file) > 128)
       {
         cf = MALLOC (128);
         memcpy (cf, curr_file, 124);
         memcpy (cf+124, "...", 4);
       }
     
     algae_fprintf (stderr, "%s: %s: line %d: run time warning%s: ",
		   progname, cf ? cf : curr_file, curr_line_no, BEEP);
     algae_vfprintf (stderr, fmt, ap);
     algae_fprintf (stderr, "\n");

     TFREE (cf);
     va_end (ap);
}

void
inform VA_ALIST (char *fmt)

     /* A non-threatening message. */

     char *cf = NULL;
     va_list ap;
     VA_START (ap, char *, fmt);

     /* don't overflow the print buffer */

     if (strlen (curr_file) > 128)
       {
         cf = MALLOC (128);
         memcpy (cf, curr_file, 124);
         memcpy (cf+124, "...", 4);
       }
     
     algae_fprintf (stderr, "%s: %s: line %d: run time info: ",
		   progname, cf ? cf : curr_file, curr_line_no);
     algae_vfprintf (stderr, fmt, ap);
     algae_fprintf (stderr, "\n");

     TFREE (cf);
     va_end (ap);
}

static int
algae_fprintf VA_ALIST2 (FILE * stream, char *format)

     /*
      * Like fprintf, but break lines to accommodate terminal width.
      * Returns the number of characters written.  This uses 'algae_vfprintf',
      * below, which uses a buffer.  If you write too much, you'll overflow
      * the buffer.
      */

     int i;
     va_list arg;

     VA_START2 (arg, FILE *, stream, char *, format);

     i = algae_vfprintf (stream, format, arg);
     va_end (arg);

     return i;
}

static int
algae_vfprintf (FILE *stream, char *format, va_list arg)
{
  /*
   * Like vfprintf, but break lines to accommodate terminal width.
   * Returns the number of characters written.  This uses a buffer
   * for temporary storage.  If you write too much, you'll overflow
   * the buffer.
   */

  int i;
  static int t;
  static char tag[] = "No Step!";
  char *marker, *pointer, *c;
  int width = TERM_WIDTH;

#if HAVE_VSNPRINTF
  if ((i = vsnprintf (printf_buf, PRINTF_BUF_SIZE, format, arg))
      >= PRINTF_BUF_SIZE)
    {
      fail ("Overflow in sprintf buffer.");
      raise_exception ();
    }
#else
  /*
   * We put a tag at the end, hoping to nab anyone who writes beyond it.
   * We could use the output of `vsprintf' instead of calling `strlen',
   * but we don't because `vsprintf' works differently on some machines.
   */
  sprintf (printf_buf + PRINTF_BUF_SIZE - sizeof (tag) - 1, tag);
  vsprintf (printf_buf, format, arg);
  i = strlen (printf_buf);
  if (strcmp (printf_buf + PRINTF_BUF_SIZE - sizeof (tag) - 1, tag))
    wipeout ("Overflow in print buffer.");
#endif

  /*
   * `pointer' is the beginning of the line.
   * `marker' marks the last whitespace found.
   */

  c = pointer = printf_buf;
  marker = t ? printf_buf : NULL;

  do
    {
      if (isspace (*c) || !*c)
	{
	  /* Found whitespace, should we break the line? */

	  if (t >= width && width > 0 || *c == '\n')
	    {
	      if (t >= width && width > 0)
		{
		  /* The line's too long, so let's break it. */

		  if (marker == NULL)
		    {
		      *c++ = '\0';	/* No good place to break it. */
		    }
		  else if (marker == printf_buf)
		    {
		      pointer = "";	/* Break it at the beginning. */
		      c = printf_buf;
		    }
		  else
		    {
		      *marker = '\0';	/* Break it at `marker'. */
		      c = marker + 1;
		    }
		}
	      else
		{
		  /* It's just a newline. */

		  *c++ = '\0';
		}

	      fputs (pointer, stream);
	      putc ('\n', stream);

	      /* Now restart at the beginning of the line. */

	      while (isspace (*c))
		c++;
	      pointer = c;
	      marker = NULL;
	      t = 0;

	      continue;
	    }
	  else
	    {
	      /* Don't break it, just mark this spot */

	      t += (*c == '\t') ? 8 : (isprint (*c)) ? 1 : 0;
	      marker = c++;
	    }
	}
      else
	{
	  if (isprint (*c++))
	    t++;
	}
    }
  while (c - printf_buf <= i);

  fputs (pointer, stream);

  return (i);
}

void
math_error (void)
{
  /*
   * This function exists only because the strings given by `strerror'
   * for math errors are typically stupid.  For example, `log(0)' gives
   * "argument too large" on our DECstation.
   */

  switch (errno)
    {

    case EDOM:

      fail ("Argument out of domain.");
      break;

    case ERANGE:

      fail ("Value out of range.");
      break;

    default:

      wipeout ("Unknown math error.");

    }

  errno = 0;
  raise_exception ();
}
