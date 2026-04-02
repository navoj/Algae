/*
   printf.c -- The Algae `fprintf' and `sprintf' functions.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
*/

static char rcsid[] =
  "$Id: printf.c,v 1.4 2003/09/06 17:08:28 ksh Exp $";

/*
 * The `do_printf' routine is from Mike Brennan's mawk
 * (an implementation of the AWK programming language).
 */

#include "printf.h"
#include "entity.h"
#include "scalar.h"
#include "file_io.h"
#include "cast.h"
#include "vargs.h"

static void PROTO (null_arg, (int cnt, char *who));
static void PROTO (arg_error,
		  (char *few_or_many, char *who, char *format ));
static void PROTO (bad_conversion,
		  (int cnt, char *who, char *format));
static char *PROTO (do_printf,
		   (void *fp, char *format, unsigned int argcnt,
		    ENTITY **ep));

#if NO_PROTOS
typedef int (*PRINTER) ();
#else
typedef int (*PRINTER) (void *, char *, ...);
#endif

/*---------- types and defs for doing printf and sprintf----*/
#define  PF_C		0
#define  PF_S		1
#define  PF_D		2   /* int conversion */
#define  PF_LD		3   /* long int */
#define  PF_F		4   /* float conversion */

/* for switch on number of '*' and type */
#define  AST(num,type)  (5*(num)+(type))

/*-------------------------------------------------------*/

void
xfprintf VA_ALIST2 (FILE *stream, char *format)

  /* fprintf, but with error checking */

  va_list arg;
  VA_START2 (arg, FILE *, stream, char *, format);

  if (vfprintf (stream, format, arg) < 0)
    {
      WRITE_FAIL (stream);
      raise_exception ();
    }

  va_end (arg);
}

void
xsprintf VA_ALIST2 (char *str, char *format)

  /* Use only for writing to printf_buf. */
  /* We're wide open to buffer overflows without snprintf. */

#if HAVE_VSNPRINTF
  size_t n;
#endif
  va_list arg;
  VA_START2 (arg, char *, str, char *, format);

#if HAVE_VSNPRINTF
  n = PRINTF_BUF_SIZE - (str - printf_buf);
  if (vsnprintf (str, n, format, arg) >= n)
    {
      fail ("Overflow in sprintf buffer.");
      raise_exception ();
    }
#else
  vsprintf (str, format, arg);
#endif

  va_end (arg);
}

void
xputc (int c, FILE *stream)
{
  if (putc (c, stream) == EOF)
    {
      WRITE_FAIL (stream);
      raise_exception ();
    }
}

void
xfputs (char *s, FILE *stream)
{
  if (fputs (s, stream) == EOF)
    {
      WRITE_FAIL (stream);
      raise_exception ();
    }
}

static void
null_arg (int cnt, char *who)
{
  fail ("NULL as %d%s argument to %s.", cnt, TH (cnt), who);
  raise_exception ();
}

static void
arg_error (char * few_or_many, char * who, char * format)
{
  fail ("Too %s arguments passed to %s(\"%s\").",
	few_or_many, who, format);
  raise_exception ();
}

static void
bad_conversion (int cnt, char * who, char * format)
{
  fail ("Improper conversion (number %d) in %s(\"%s\").", 
	cnt, who, format);
  raise_exception ();
}

static char *
do_printf (void * fp, char * format, unsigned int argcnt, ENTITY ** volatile ep)
{
  /*
   * This calls fprintf (when `fp' is a valid file pointer) or sprintf (when
   * `fp' is NULL).  The pointer `ep' points to the first of `argcnt' ENTITY
   * pointers.  On exit, the `format' string is freed and all the ENTITY's in
   * `ep' are deleted.
   */

  char save;
  char *p;
  char *retval;
  volatile unsigned int arg_count = argcnt;
  ENTITY ** volatile ep_start = ep;
  char * volatile q = format;
  char * volatile target;
  int l_flag , h_flag;  /* seen %ld or %hd  */
  int ast_cnt;
  int ast[2];
  long lval;
  int ival;  /* caters to MSDOS */
  int num_conversion = 0; /* for error messages */
  char *who; /*ditto*/
  int pf_type;  /* conversion type */
  PRINTER printer; /* pts at xfprintf() or sprintf() */
  char *printf_limit = printf_buf + PRINTF_BUF_SIZE;
  
  if (fp == NULL)	/* doing sprintf */
    {		
      target = printf_buf;
      printer = (PRINTER) xsprintf;
      who = "sprintf";
    }
  else			/* doing printf */
    {
      target = (char *) fp;	/* will never change */
      printer = (PRINTER) xfprintf;
      who = "fprintf";
    }
  
  WITH_HANDLING
    {
      while (1)
	{
	  if (fp)
	    {
	      while (*q != '%')
		{
		  if (*q == 0)	/* done */
		    {
		      retval = NULL; /* Success, even if args remain. */
		      goto done;
		    }
		  else
		    {
		      xputc (*q, (FILE *) fp);
		      q++;
		    }
		}
	    }
	  else
	    {
	      while (*q != '%')
		{
		  if (*q == 0)	/* done */
		    {
		      if (target >= printf_limit)
			{
			  /* damaged, hope this works */
			  wipeout ("Overflow in sprintf buffer.");
			}
		      else	/* really done */
			{
			  *target = '\0';
			  retval = dup_char (printf_buf);
			  goto done;
			}
		    }
		  else
		    {
		      *target++ = *q++;
		      if (target - printf_buf >= PRINTF_BUF_SIZE)
			{
			  fail ("Overflow in sprintf buffer.");
			  raise_exception ();
			}
		    }
		}
	    }
	  num_conversion++;

	  if (* ++q == '%')	/* %% */
	    {
	      if (fp)
		{
		  xputc (*q, (FILE *) fp);
		}
	      else
		{
		  *target++ = *q;
		  if (target - printf_buf >= PRINTF_BUF_SIZE)
		    {
		      fail ("Overflow in sprintf buffer.");
		      raise_exception ();
		    }
		}
	      q++;
	      continue;
	    }
	  
	  /* mark the '%' with p */
	  p = q-1;
	  
	  /* eat the flags */
	  while (*q == '-' || *q == '+' || *q == ' ' ||
		 *q == '#' || *q == '0')  q++;
	  
	  ast_cnt = 0;
	  if (*q == '*')
	    {
	      if (*ep == NULL) null_arg (arg_count - argcnt + 1, who);
	      *ep = cast_scalar ((SCALAR *) scalar_entity (EAT (*ep)),
				 integer);
	      ast[ast_cnt++] = ((SCALAR *)*ep++)->v.integer;
	      argcnt--; q++;
	    }
	  else
	    {
	      while (isdigit (*q)) q++;
	    }
	  
	  if (*q == '.')	/* have precision */
	    {
	      q++;
	      if (*q == '*')
		{
		  if (*ep == NULL)
		    null_arg (arg_count - argcnt + 1, who);
		  *ep = cast_scalar ((SCALAR *) scalar_entity (EAT (*ep)),
				     integer);
		  ast[ast_cnt++] = ((SCALAR *)*ep++)->v.integer;
		  argcnt--; q++;
		}
	      else
		while (isdigit (*q)) q++;
	    }
	  
	  if (argcnt <= 0) arg_error ("few", who, format);
	  l_flag = h_flag = 0;
	    
	  if (*q == 'l')
	    {
	      q++;
	      l_flag = 1;
	    }
	  
#if HAVE_PRINTF_HD
	  else
	    if (*q == 'h')
	      {
		q++;
		h_flag = 1;
	      }
#endif
	  
	  switch (*q++)
	    {
	    case 's':

	      if (l_flag + h_flag) 
		bad_conversion (num_conversion, who, format);
	      if (*ep == NULL) null_arg (arg_count - argcnt + 1, who);
	      *ep = cast_scalar ((SCALAR *) scalar_entity (EAT (*ep)),
				 character );
	      pf_type = PF_S;
	      break;
		  
	    case 'c':

	      if (l_flag + h_flag)
		bad_conversion (num_conversion, who, format);
	      if (*ep == NULL) null_arg (arg_count - argcnt + 1, who);
	      *ep = scalar_entity (EAT (*ep));
	      switch (((SCALAR *)*ep)->type)
		{
		case integer:

		  ival = ((SCALAR *)*ep)->v.integer;
		  break;
			
		case real:
		  
		  ival = (int) ((SCALAR *)*ep)->v.real;
		  break;
			
		case character:

		  ival = ((SCALAR *)*ep)->v.character[0];
		  break;

		default:
		  
		  BAD_TYPE (((SCALAR *)*ep)->type);
		  raise_exception ();
		}
		  
	      pf_type = PF_C;
	      break;
		  
	    case 'd':
	    case 'o':
	    case 'x':
	    case 'X':
	    case 'i':
	    case 'u':

	      if (*ep == NULL) null_arg (arg_count - argcnt + 1, who);
	      *ep = cast_scalar ((SCALAR *) scalar_entity (EAT (*ep)),
				 integer);
	      lval = (long) ((SCALAR *)*ep)->v.integer;
#if HAVE_PRINTF_HD
	      if ( h_flag ) lval &= 0xffff;
#endif
	      pf_type = l_flag ? PF_LD : PF_D;
	      break;
		  
	    case 'e':
	    case 'g':
	    case 'f':
	    case 'E':
	    case 'G':

	      if ( h_flag + l_flag )
		bad_conversion (num_conversion, who, format);
	      if (*ep == NULL) null_arg (arg_count - argcnt + 1, who);
	      *ep = cast_scalar ((SCALAR *) scalar_entity (EAT (*ep)),
				 real);
	      pf_type = PF_F;
	      break;
		  
	    default:

	      bad_conversion (num_conversion, who, format);
	    }
	    
	  save = *q;
	  *q = 0;
	    
	  /* ready to call printf() */
	  switch (AST (ast_cnt, pf_type))
	    {
	    case AST(0, PF_C ):
	      (*printer) (target, p, ival);
	      break;
	      
	    case AST(1, PF_C ):
	      (*printer) (target, p, ast[0], ival);
	      break;
	      
	    case AST(2, PF_C ):
	      (*printer) (target, p, ast[0], ast[1], ival);
	      break;
	      
	    case AST(0, PF_S):
	      (*printer) (target, p, ((SCALAR *)*ep)->v.character);
	      break;
		  
	    case AST(1, PF_S):
	      (*printer) (target, p, ast[0],
			  ((SCALAR *)*ep)->v.character);
	      break;
	      
	    case AST(2, PF_S):
	      (*printer) (target, p, ast[0], ast[1],
			  ((SCALAR *)*ep)->v.character);
	      break;
		  
	    case AST(0, PF_D):
	      (*printer) (target, p, (int) lval);
	      break;
	      
	    case AST(1, PF_D):
	      (*printer) (target, p, ast[0], (int) lval);
	      break;
	      
	    case AST(2, PF_D):
	      (*printer) (target, p, ast[0], ast[1], (int) lval);
	      break;
	      
	    case AST(0, PF_LD):
	      (*printer) (target, p,  lval);
	      break;
		  
	    case AST(1, PF_LD):
	      (*printer) (target, p, ast[0],  lval);
	      break;
		  
	    case AST(2, PF_LD):
	      (*printer) (target, p, ast[0], ast[1],  lval);
	      break;
	      
	    case AST(0, PF_F):
	      (*printer) (target, p,  ((SCALAR *)*ep)->v.real);
	      break;
	      
	    case AST(1, PF_F):
	      (*printer) (target, p, ast[0],  ((SCALAR *)*ep)->v.real);
	      break;
		  
	    case AST(2, PF_F):
	      (*printer) (target, p, ast[0], ast[1],
			  ((SCALAR *)*ep)->v.real);
	      break;
	    }
	  if (fp == NULL) while (*target) target++;
	  *q = save;
	  argcnt--;
	  ep++;
	}
    done:
      FREE_CHAR (format);
    }
  ON_EXCEPTION
    {
      while (arg_count--)
	{
	  delete_entity (*ep_start);
	  *ep_start = NULL;
	  ep_start++;
	}
      FREE_CHAR (format);
    }
  END_EXCEPTION;
  
  while (arg_count--)
    {
      delete_entity (*ep_start);
      *ep_start = NULL;
      ep_start++;
    }
  return retval;
}

ENTITY *
bi_fprintf (volatile int n, ENTITY ** volatile args)
{
  /*
   * Works pretty much like C's fprintf.  The first arg
   * is the file name (NULL gets you stdout).
   */
  
  char * volatile format = NULL_string;
  FILE * volatile stream;
    
  assert (n > 1);
    
  WITH_HANDLING
    {
      if (args[0])
	{
	  args[0] = scalar_entity (EAT (args[0]));
	  if (((SCALAR *)args[0])->type != character)
	    {
	      fail ("File name for \"fprintf\" must be character.");
	      raise_exception ();
	    }
	  stream = find_file (dup_char (((SCALAR *)args[0])->v.character),
			      FILE_OUTPUT);
	  if (!stream) raise_exception ();
	  delete_entity (args[0]);
	  args[0] = NULL;
	}
      else
	{
	  stream = stdout;
	}

      if (!args[1]) null_arg (2, "fprintf");
      args[1] = scalar_entity (EAT (args[1]));
      if (((SCALAR *)args[1])->type != character)
	{
	  fail ("Format string for \"fprintf\" must be character.");
	  raise_exception ();
	}
	format = dup_char (((SCALAR *)args[1])->v.character);
      delete_entity (args[1]);
      args[1] = NULL;
    }
  ON_EXCEPTION
    {
      while (n--)
	{
	  delete_entity (*args);
	  args++;  /* Keep separate, `delete_entity' may be a macro. */
	}
	FREE_CHAR (format);
    }
  END_EXCEPTION;
    
  do_printf (stream, format, (unsigned int) (n-2), args+2);
  return int_to_scalar(0);
}

ENTITY *
bi_sprintf (volatile int n, ENTITY ** volatile args)
{
  /*
   * Works like C's sprintf, except that the resulting string
   * is returned as a scalar (no address is given in the args).
   */
    
  char * volatile format = NULL_string;
    
  assert (n > 0);
    
  WITH_HANDLING
    {
      if (!args[0]) null_arg (1, "sprintf");
      args[0] = scalar_entity (EAT (args[0]));
      if (((SCALAR *)args[0])->type != character)
	{
	  fail ("Format string for \"sprintf\" must be character.");
	  raise_exception ();
	}
      format = dup_char (((SCALAR *)args[0])->v.character);
      delete_entity (args[0]);
      args[0] = NULL;
    }
  ON_EXCEPTION
    {
      while (n--)
	{
	  delete_entity (*args);
	  args++;  /* Keep separate, `delete_entity' may be a macro. */
	}
      FREE_CHAR (format);
    }
  END_EXCEPTION;
    
  return char_to_scalar (do_printf (NULL, format, (unsigned int) (n-1),
				    args+1));
}

ENTITY *
bi_printf (volatile int n, ENTITY ** volatile args)
{
  /*
   * Works pretty much like C's printf.
   */
    
  char * volatile format = NULL_string;

  assert (n > 0);
    
  WITH_HANDLING
    {
      if (!args[0]) null_arg (1, "printf");
      args[0] = scalar_entity (EAT (args[0]));
      if (((SCALAR *)args[0])->type != character)
	{
	  fail ("Format string for \"printf\" must be character.");
	  raise_exception ();
	}
	format = dup_char (((SCALAR *)args[0])->v.character);
      delete_entity (args[0]);
      args[0] = NULL;
    }
  ON_EXCEPTION
    {
      while (n--)
	{
	  delete_entity (*args);
	  args++;  /* Keep separate, `delete_entity' may be a macro. */
	}
      FREE_CHAR (format);
    }
  END_EXCEPTION;

  do_printf (stdout, format, (unsigned int) (n-1), args+1);
  return int_to_scalar(0);
}
