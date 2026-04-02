/*
   print.c -- Print entities.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: print.c,v 1.10 2003/09/06 18:42:59 ksh Exp $";

#include "print.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "function.h"
#include "file_io.h"
#include "full.h"
#include "partition.h"
#include "cast.h"
#include "printf.h"
#include "psr.h"

static char col_msg[] = "\tColumns %d through %d ...\n";
static char elem_msg[] = "\tElements %d through %d ...\n";

DATUM *prompt_string;

DATUM *term_width;
#define TERM_WIDTH datum_to_int (term_width)

extern int whitespace;		/* true prints trailing newlines */

DATUM *num_digits;		/* number of significant digits to print */
#define DIGITS datum_to_int (num_digits)

ENTITY *
bi_print (int n, ENTITY *p, ENTITY *fname)
{
  FILE * volatile stream;

  assert (n > 0);
  EASSERT (p, 0, 0);

  WITH_HANDLING
  {
    switch (n)
      {
      case 1:
	stream = stdout;
	break;
      case 2:
	EASSERT (fname, 0, 0);
	fname = bi_scalar (1, EAT (fname));
	if (((SCALAR *) fname)->type != character)
	  {
	    fail ("File name for \"print\" must be character.");
	    raise_exception ();
	  }
	stream = find_file (dup_char (((SCALAR *) fname)->v.character), FILE_OUTPUT);
	break;
      default:
	wipeout ("Wrong number of args to \"bi_print\".");
      }
    if (stream)
      print_entity (EAT (p), stream);
  }
  ON_EXCEPTION
  {
    delete_2_entities (p, fname);
  }
  END_EXCEPTION;

  delete_entity (fname);
  return (stream ? int_to_scalar (0) : NULL);
}

void
print_entity (ENTITY *p, FILE *stream)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      print_scalar ((SCALAR *) p, stream);
      break;
    case vector:
      print_vector ((VECTOR *) p, stream);
      break;
    case matrix:
      print_matrix ((MATRIX *) p, stream);
      break;
    case table:
      print_table ((TABLE *) p, stream);
      break;
    case function:
      print_function ((FUNCTION *) p, stream);
      break;
    default:
      BAD_CLASS (p->class);
      delete_entity (p);
      raise_exception ();
    }
}

#define WS(w, nw)	(whitespace ? w : nw)

void
print_scalar (SCALAR *p, FILE *stream)
{
  int digits = DIGITS;

  EASSERT (p, scalar, 0);

  switch (p->type)
    {
    case integer:
      xfprintf (stream, WS ("\t%d\n", "%d"), p->v.integer);
      break;
    case real:
      xfprintf (stream, WS ("\t%#.*g\n", "%#.*g"), digits, p->v.real);
      break;
    case complex:
      if (p->v.complex.real == 0 && p->v.complex.imag == 0)
	{
	  sprintf (printf_buf, "0");
	}
      else
	{
#if HAVE_SNPRINTF
	  if (snprintf (printf_buf, PRINTF_BUF_SIZE, "%#.*g + %#.*g*i",
			digits, p->v.complex.real,
			digits, p->v.complex.imag) >= PRINTF_BUF_SIZE)
	    {
	      fail ("Overflow in sprintf buffer.");
	      raise_exception ();
	    }
#else
	  sprintf (printf_buf, "%#.*g + %#.*g*i",
		   digits, p->v.complex.real,
		   digits, p->v.complex.imag);
#endif
	}
      xfprintf (stream, WS ("\t%s\n", "%s"), printf_buf);
      break;
    case character:
      xfprintf (stream, WS ("\t\"%s\"\n", "%s"), p->v.character);
      break;
    default:
      BAD_TYPE (p->type);
      delete_scalar (p);
      raise_exception ();
    }
  delete_scalar (p);
}

void
print_matrix (MATRIX *p, FILE *stream)
{
  EASSERT (p, matrix, 0);

  if (p->density == sparse_upper)
    p = (MATRIX *) full_matrix (p);

  switch (p->type)
    {
    case integer:
      switch (p->density)
	{
	case dense:
	  print_matrix_integer_dense (p, stream);
	  break;
	case sparse:
	  print_matrix_integer_sparse (p, stream);
	  break;
	default:
	  BAD_DENSITY (p->density);
	  delete_matrix (p);
	  raise_exception ();
	}
      break;
    case real:
      switch (p->density)
	{
	case dense:
	  print_matrix_real_dense (p, stream);
	  break;
	case sparse:
	  print_matrix_real_sparse (p, stream);
	  break;
	default:
	  BAD_DENSITY (p->density);
	  delete_matrix (p);
	  raise_exception ();
	}
      break;
    case complex:
      switch (p->density)
	{
	case dense:
	  print_matrix_complex_dense (p, stream);
	  break;
	case sparse:
	  print_matrix_complex_sparse (p, stream);
	  break;
	default:
	  BAD_DENSITY (p->density);
	  delete_matrix (p);
	  raise_exception ();
	}
      break;
    case character:
      switch (p->density)
	{
	case dense:
	  print_matrix_character_dense (p, stream);
	  break;
	case sparse:
	  print_matrix_character_sparse (p, stream);
	  break;
	default:
	  BAD_DENSITY (p->density);
	  delete_matrix (p);
	  raise_exception ();
	}
      break;
    default:
      BAD_TYPE (p->type);
      delete_matrix (p);
      raise_exception ();
    }
}

void
print_matrix_character_dense (MATRIX *p, FILE *stream)
{
  char **b;
  int m, n, maxlen, len;
  int i, j, ii, nn, inc, istart, iend;

  EASSERT (p, matrix, character);
  assert (p->density == dense);

  WITH_HANDLING
  {
    b = p->a.character;
    m = p->nr;
    n = p->nc;

    /* Determine length of the longest string. */

    maxlen = 0;
    for (i = 0; i < p->nn; i++)
      {
	if ((len = strlen (b[i])) > maxlen)
	  maxlen = len;
      }

    /* Number of columns per line */
    inc = (TERM_WIDTH > 0) ? (TERM_WIDTH - 12) / (maxlen + 4) : n;

    if (inc <= 0)
      inc = 1;
    nn = n / inc;
    if (n % inc > 0)
      nn++;

    for (i = 0; i < nn; i++)
      {
	POLL_SIGINT ();
	istart = i * inc;
	if ((i + 1) * inc < n)
	  {
	    iend = (i + 1) * inc;
	  }
	else
	  {
	    iend = n;
	  }
	if (istart < iend && nn > 1)
	  {
	    xfprintf (stream, col_msg, istart + 1, iend);
	  }
	for (j = 0; j < m; j++)
	  {
	    POLL_SIGINT ();
	    xfprintf (stream, (i == 0) ? "\t[  " : "\t   ");
	    for (ii = istart; ii < iend; ii++)
	      {
		xfprintf (stream, " \"%s\"%*s", b[m * ii + j],
			  maxlen - strlen (b[m * ii + j]) + 1, " ");
		fflush (stdout);
	      }
	    xfprintf (stream, (i == nn - 1) ? "]\n" : "\n");
	  }
      }
  }
  ON_EXCEPTION
  {
    delete_matrix (p);
  }
  END_EXCEPTION;

  delete_matrix (p);
}

void
print_matrix_character_sparse (MATRIX *p, FILE *stream)
{
  int i, j, ii, nk, n, inc, istart, iend, nr, nc;
  int len, maxlen;

  EASSERT (p, matrix, character);
  assert (p->density == sparse);

  WITH_HANDLING
  {

    if (p->order != ordered)
      {
	BAD_ORDER (p->order);
	raise_exception ();
      }

    nr = p->nr;
    nc = p->nc;

    maxlen = 0;
    for (i = 0; i < p->nn; i++)
      {
	if ((len = strlen (p->a.character[i])) > maxlen)
	  maxlen = len;
      }

    /* number of columns per line */
    inc = (TERM_WIDTH > 0) ? (TERM_WIDTH - 12) / (maxlen + 4) : nc;

    if (inc <= 0)
      inc = 1;
    nk = nc / inc;
    if (nc % inc > 0)
      nk++;

    for (i = 0; i < nk; i++)
      {
	POLL_SIGINT ();
	istart = i * inc;
	if ((i + 1) * inc < nc)
	  {
	    iend = (i + 1) * inc;
	  }
	else
	  {
	    iend = nc;
	  }
	if (istart < iend && nk > 1)
	  {
	    xfprintf (stream, col_msg, istart + 1, iend);
	  }
	for (j = 0; j < nr; j++)
	  {
	    POLL_SIGINT ();
	    if (p->nn > 0)
	      {
		n = p->ia[j] - 1;
		for (ii = 0; ii < istart; ii++)
		  {
		    if (p->ja[n] == ii + 1 && n + 1 < p->ia[j + 1])
		      n++;
		  }
		xfprintf (stream, (i == 0) ? "\t[  " : "\t   ");
		for (ii = istart; ii < iend; ii++)
		  {
		    if (p->ja[n] == ii + 1 && n + 1 < p->ia[j + 1])
		      {
			xfprintf (stream, " \"%s\"%*s", p->a.character[n],
			      maxlen - strlen (p->a.character[n]) + 1, " ");
			fflush (stdout);
			n++;
		      }
		    else
		      {
			xfprintf (stream, "  .%*s", maxlen + 1, " ");
		      }
		  }
		xfprintf (stream, (i == nk - 1) ? "]\n" : "\n");
	      }
	    else
	      {
		xfprintf (stream, (i == 0) ? "\t[  " : "\t   ");
		for (ii = istart; ii < iend; ii++)
		  {
		    xfprintf (stream, "  .%*s", maxlen + 1, " ");
		  }
		xfprintf (stream, (i == nk - 1) ? "]\n" : "\n");
	      }
	  }
      }
  }
  ON_EXCEPTION
  {
    delete_matrix (p);
  }
  END_EXCEPTION;

  delete_matrix (p);
}

void
print_matrix_integer_dense (MATRIX *p, FILE *stream)
{
  int *b;
  int m, n;
  int i, j, ii, nn, inc, istart, iend;

  EASSERT (p, matrix, integer);
  assert (p->density == dense);

  WITH_HANDLING
  {
    b = p->a.integer;
    m = p->nr;
    n = p->nc;

    /* number of columns per line */
    inc = (TERM_WIDTH > 0) ? (TERM_WIDTH - 12) / 11 : n;

    if (inc <= 0)
      inc = 1;
    nn = n / inc;
    if (n % inc > 0)
      nn++;

    for (i = 0; i < nn; i++)
      {
	POLL_SIGINT ();
	istart = i * inc;
	if ((i + 1) * inc < n)
	  {
	    iend = (i + 1) * inc;
	  }
	else
	  {
	    iend = n;
	  }
	if (istart < iend && nn > 1)
	  {
	    xfprintf (stream, col_msg, istart + 1, iend);
	  }
	for (j = 0; j < m; j++)
	  {
	    POLL_SIGINT ();
	    xfprintf (stream, (i == 0) ? "\t[  " : "\t   ");
	    for (ii = istart; ii < iend; ii++)
	      {
		xfprintf (stream, " %9d ", b[m * ii + j]);
		fflush (stdout);
	      }
	    xfprintf (stream, (i == nn - 1) ? "]\n" : "\n");
	  }
      }
  }
  ON_EXCEPTION
  {
    delete_matrix (p);
  }
  END_EXCEPTION;

  delete_matrix (p);
}

void
print_matrix_integer_sparse (MATRIX *p, FILE *stream)
{
  int i, j, ii, nk, n, inc, istart, iend, nr, nc;

  EASSERT (p, matrix, integer);
  assert (p->density == sparse);

  WITH_HANDLING
  {

    if (p->order != ordered)
      {
	BAD_ORDER (p->order);
	raise_exception ();
      }

    nr = p->nr;
    nc = p->nc;

    /* number of columns per line */
    inc = (TERM_WIDTH > 0) ? (TERM_WIDTH - 12) / 11 : nc;

    if (inc <= 0)
      inc = 1;
    nk = nc / inc;
    if (nc % inc > 0)
      nk++;

    for (i = 0; i < nk; i++)
      {
	POLL_SIGINT ();
	istart = i * inc;
	if ((i + 1) * inc < nc)
	  {
	    iend = (i + 1) * inc;
	  }
	else
	  {
	    iend = nc;
	  }
	if (istart < iend && nk > 1)
	  {
	    xfprintf (stream, col_msg, istart + 1, iend);
	  }
	for (j = 0; j < nr; j++)
	  {
	    POLL_SIGINT ();
	    if (p->nn > 0)
	      {
		n = p->ia[j] - 1;
		for (ii = 0; ii < istart; ii++)
		  {
		    if (p->ja[n] == ii + 1 && n + 1 < p->ia[j + 1])
		      n++;
		  }
		xfprintf (stream, (i == 0) ? "\t[  " : "\t   ");
		for (ii = istart; ii < iend; ii++)
		  {
		    if (p->ja[n] == ii + 1 && n + 1 < p->ia[j + 1])
		      {
			xfprintf (stream, " %9d ", p->a.integer[n]);
			fflush (stdout);
			n++;
		      }
		    else
		      {
			xfprintf (stream, "         . ");
		      }
		  }
		xfprintf (stream, (i == nk - 1) ? "]\n" : "\n");
	      }
	    else
	      {
		xfprintf (stream, (i == 0) ? "\t[  " : "\t   ");
		for (ii = istart; ii < iend; ii++)
		  {
		    xfprintf (stream, "         . ");
		  }
		xfprintf (stream, (i == nk - 1) ? "]\n" : "\n");
	      }
	  }
      }
  }
  ON_EXCEPTION
  {
    delete_matrix (p);
  }
  END_EXCEPTION;

  delete_matrix (p);
}

void
print_matrix_real_dense (MATRIX *p, FILE *stream)
{
  REAL *b;
  int m, n;
  int i, j, ii, nn, inc, istart, iend;
  int digits = DIGITS;

  EASSERT (p, matrix, real);

  WITH_HANDLING
  {

    b = p->a.real;
    m = p->nr;
    n = p->nc;

    /* number of columns per line */
    inc = (TERM_WIDTH > 0) ? (TERM_WIDTH - 12) / (digits + 9) : n;

    if (inc <= 0)
      inc = 1;
    nn = n / inc;
    if (n % inc > 0)
      nn++;

    for (i = 0; i < nn; i++)
      {
	POLL_SIGINT ();
	istart = i * inc;
	if ((i + 1) * inc < n)
	  {
	    iend = (i + 1) * inc;
	  }
	else
	  {
	    iend = n;
	  }
	if (istart < iend && nn > 1)
	  {
	    xfprintf (stream, col_msg, istart + 1, iend);
	  }
	for (j = 0; j < m; j++)
	  {
	    POLL_SIGINT ();
	    xfprintf (stream, (i == 0) ? "\t[  " : "\t   ");
	    for (ii = istart; ii < iend; ii++)
	      {
		xfprintf (stream, " %#*.*g ", digits + 7, digits, b[m * ii + j]);
		fflush (stdout);
	      }
	    xfprintf (stream, (i == nn - 1) ? "]\n" : "\n");
	  }
      }
  }
  ON_EXCEPTION
  {
    delete_matrix (p);
  }
  END_EXCEPTION;

  delete_matrix (p);
}

void
print_matrix_real_sparse (MATRIX *p, FILE *stream)
{
  int i, j, ii, nk, n, inc, istart, iend, nr, nc;
  int digits = DIGITS;

  EASSERT (p, matrix, real);
  assert (p->density == sparse);

  WITH_HANDLING
  {

    if (p->order != ordered)
      {
	BAD_ORDER (p->order);
	raise_exception ();
      }

    nr = p->nr;
    nc = p->nc;

    /* number of columns per line */
    inc = (TERM_WIDTH > 0) ? (TERM_WIDTH - 12) / (digits + 9) : nc;

    if (inc <= 0)
      inc = 1;
    nk = nc / inc;
    if (nc % inc > 0)
      nk++;

    for (i = 0; i < nk; i++)
      {
	POLL_SIGINT ();
	istart = i * inc;
	if ((i + 1) * inc < nc)
	  {
	    iend = (i + 1) * inc;
	  }
	else
	  {
	    iend = nc;
	  }
	if (istart < iend && nk > 1)
	  {
	    xfprintf (stream, col_msg, istart + 1, iend);
	  }
	for (j = 0; j < nr; j++)
	  {
	    POLL_SIGINT ();
	    if (p->nn > 0)
	      {
		n = p->ia[j] - 1;
		for (ii = 0; ii < istart; ii++)
		  {
		    if (p->ja[n] == ii + 1 && n + 1 < p->ia[j + 1])
		      n++;
		  }
		xfprintf (stream, (i == 0) ? "\t[  " : "\t   ");
		for (ii = istart; ii < iend; ii++)
		  {
		    if (p->ja[n] == ii + 1 && n + 1 < p->ia[j + 1])
		      {
			xfprintf (stream, " %#*.*g ",
				  digits + 7, digits, p->a.real[n]);
			fflush (stdout);
			n++;
		      }
		    else
		      {
			xfprintf (stream, " %*s ", digits + 7, ".");
		      }
		  }
		xfprintf (stream, (i == nk - 1) ? "]\n" : "\n");
	      }
	    else
	      {
		xfprintf (stream, (i == 0) ? "\t[  " : "\t   ");
		for (ii = istart; ii < iend; ii++)
		  {
		    xfprintf (stream, " %*s ", digits + 7, ".");
		  }
		xfprintf (stream, (i == nk - 1) ? "]\n" : "\n");
	      }
	  }
      }
  }
  ON_EXCEPTION
  {
    delete_matrix (p);
  }
  END_EXCEPTION;

  delete_matrix (p);
}

void
print_matrix_complex_dense (MATRIX *p, FILE *stream)
{
  COMPLEX *b;
  int m, n;
  int i, j, ii, nn, inc, istart, iend;
  int digits = DIGITS;

  EASSERT (p, matrix, complex);
  assert (p->density == dense);

  WITH_HANDLING
  {

    b = p->a.complex;
    m = p->nr;
    n = p->nc;

    /* number of columns per line */
    inc = (TERM_WIDTH > 0) ? (TERM_WIDTH - 12) / (2 * digits + 21) : n;

    if (inc <= 0)
      inc = 1;
    nn = n / inc;
    if (n % inc > 0)
      nn++;

    for (i = 0; i < nn; i++)
      {
	POLL_SIGINT ();
	istart = i * inc;
	if ((i + 1) * inc < n)
	  {
	    iend = (i + 1) * inc;
	  }
	else
	  {
	    iend = n;
	  }
	if (istart < iend && nn > 1)
	  {
	    xfprintf (stream, col_msg, istart + 1, iend);
	  }
	for (j = 0; j < m; j++)
	  {
	    POLL_SIGINT ();
	    xfprintf (stream, (i == 0) ? "\t[  " : "\t   ");
	    for (ii = istart; ii < iend; ii++)
	      {
		if (b[m * ii + j].real == 0 && b[m * ii + j].imag == 0)
		  {
		    sprintf (printf_buf, "0  ");
		  }
		else
		  {
#if HAVE_SNPRINTF
		    if (snprintf (printf_buf, PRINTF_BUF_SIZE,
				  "%#.*g + %#.*g*i",
				  digits, b[m * ii + j].real,
				  digits, b[m * ii + j].imag)
			>= PRINTF_BUF_SIZE)
		      {
			fail ("Overflow in sprintf buffer.");
			raise_exception ();
		      }
#else
		    sprintf (printf_buf, "%#.*g + %#.*g*i",
			     digits, b[m * ii + j].real,
			     digits, b[m * ii + j].imag);
#endif
		  }
		xfprintf (stream, " %*s ", 2 * digits + 19, printf_buf);
	      }
	    xfprintf (stream, (i == nn - 1) ? "]\n" : "\n");
	  }
      }
  }
  ON_EXCEPTION
  {
    delete_matrix (p);
  }
  END_EXCEPTION;

  delete_matrix (p);
}

void
print_matrix_complex_sparse (MATRIX *p, FILE *stream)
{
  int i, j, ii, nk, n, inc, istart, iend, nr, nc;
  int digits = DIGITS;

  EASSERT (p, matrix, complex);
  assert (p->density == sparse);

  WITH_HANDLING
  {

    if (p->order != ordered)
      {
	BAD_ORDER (p->order);
	raise_exception ();
      }

    nr = p->nr;
    nc = p->nc;

    /* number of columns per line */
    inc = (TERM_WIDTH > 0) ? (TERM_WIDTH - 12) / (2 * digits + 21) : nc;

    if (inc <= 0)
      inc = 1;
    nk = nc / inc;
    if (nc % inc > 0)
      nk++;

    for (i = 0; i < nk; i++)
      {
	POLL_SIGINT ();
	istart = i * inc;
	if ((i + 1) * inc < nc)
	  {
	    iend = (i + 1) * inc;
	  }
	else
	  {
	    iend = nc;
	  }
	if (istart < iend && nk > 1)
	  {
	    xfprintf (stream, col_msg, istart + 1, iend);
	  }
	for (j = 0; j < nr; j++)
	  {
	    POLL_SIGINT ();
	    if (p->nn > 0)
	      {
		n = p->ia[j] - 1;
		for (ii = 0; ii < istart; ii++)
		  {
		    if (p->ja[n] == ii + 1 && n + 1 < p->ia[j + 1])
		      n++;
		  }
		xfprintf (stream, (i == 0) ? "\t[  " : "\t   ");
		for (ii = istart; ii < iend; ii++)
		  {
		    if (p->ja[n] == ii + 1 && n + 1 < p->ia[j + 1])
		      {
#if HAVE_SNPRINTF
			if (snprintf (printf_buf, PRINTF_BUF_SIZE,
				      "%#.*g + %#.*g*i",
				      digits, p->a.complex[n].real,
				      digits, p->a.complex[n].imag)
			    >= PRINTF_BUF_SIZE)
			  {
			    fail ("Overflow in sprintf buffer.");
			    raise_exception ();
			  }
#else
			sprintf (printf_buf, "%#.*g + %#.*g*i",
				 digits, p->a.complex[n].real,
				 digits, p->a.complex[n].imag);
#endif
			n++;
		      }
		    else
		      {
#if HAVE_SNPRINTF
			if (snprintf (printf_buf, PRINTF_BUF_SIZE, ".")
			    >= PRINTF_BUF_SIZE)
			  {
			    fail ("Overflow in sprintf buffer.");
			    raise_exception ();
			  }
#else
			sprintf (printf_buf, ".");
#endif
		      }
		    xfprintf (stream, " %*s ", 2 * digits + 19, printf_buf);
		  }
		xfprintf (stream, (i == nk - 1) ? "]\n" : "\n");
	      }
	    else
	      {
		xfprintf (stream, (i == 0) ? "\t[  " : "\t   ");
		for (ii = istart; ii < iend; ii++)
		  {
		    xfprintf (stream, " %*s ", 2 * digits + 19, ".");
		  }
		xfprintf (stream, (i == nk - 1) ? "]\n" : "\n");
	      }
	  }
      }
  }
  ON_EXCEPTION
  {
    delete_matrix (p);
  }
  END_EXCEPTION;

  delete_matrix (p);
}

void
print_vector (VECTOR *p, FILE *stream)
{
  EASSERT (p, vector, 0);

  switch (p->density)
    {
    case dense:
      print_vector_dense (p, stream);
      break;
    case sparse:
      print_vector_sparse (p, stream);
      break;
    default:
      BAD_DENSITY (p->density);
      delete_vector (p);
      raise_exception ();
    }
}

void
print_vector_dense (VECTOR *p, FILE *stream)
{
  int n;
  int i, ii, nn, inc, istart, iend;
  int maxlen = 0;
  int digits = DIGITS;

  EASSERT (p, vector, 0);
  assert (p->density == dense);

  WITH_HANDLING
  {

    n = p->ne;

    if (n)
      {
        if (TERM_WIDTH > 0)
          {
            switch (p->type)
              {
              case integer:
                inc = (TERM_WIDTH - 10) / 11;
                break;
              case real:
                inc = (TERM_WIDTH - 10) / (digits + 9);
                break;
              case complex:
                inc = (TERM_WIDTH - 10) / (2 * digits + 21);
                break;
              case character:
                {
                  int len;
                  for (i = 0; i < p->nn; i++)
                    {
                      if ((len = strlen (p->a.character[i])) > maxlen)
                        maxlen = len;
                    }
                  inc = (TERM_WIDTH - 10) / (maxlen + 4);
                }
              break;
              default:
                BAD_TYPE (p->type);
                raise_exception ();
              }
            if (inc <= 0)
              inc = 1;
          }
        else
          {
            inc = n;
          }

        nn = n / inc;
        if (n % inc > 0)
          nn++;

        for (i = 0; i < nn; i++)
          {
            POLL_SIGINT ();
            istart = i * inc;
            if ((i + 1) * inc < n)
              {
                iend = (i + 1) * inc;
              }
            else
              {
                iend = n;
              }
            if (istart < iend && nn > 1)
              {
                xfprintf (stream, elem_msg, istart + 1, iend);
              }
            xfprintf (stream, (i == 0) ? "\t( " : "\t  ");
            switch (p->type)
              {
              case integer:
                for (ii = istart; ii < iend - 1; ii++)
                  {
                    xfprintf (stream, " %9d,", p->a.integer[ii]);
                  }
                if (iend > 0)
                  xfprintf (stream, " %9d", p->a.integer[iend - 1]);
                break;
              case real:
                for (ii = istart; ii < iend - 1; ii++)
                  {
                    xfprintf (stream, " %#*.*g,",
                              digits + 7, digits, p->a.real[ii]);
                  }
                if (iend > 0)
                  xfprintf (stream, " %#*.*g",
                            digits + 7, digits,
                            p->a.real[iend - 1]);
                break;
              case complex:
                for (ii = istart; ii < iend - 1; ii++)
                  {
#if HAVE_SNPRINTF
                    if (snprintf (printf_buf, PRINTF_BUF_SIZE,
				  "%#.*g + %#.*g*i",
				  digits, p->a.complex[ii].real,
				  digits, p->a.complex[ii].imag)
			>= PRINTF_BUF_SIZE)
		      {
			fail ("Overflow in sprintf buffer.");
			raise_exception ();
		      }
#else
                    sprintf (printf_buf, "%#.*g + %#.*g*i",
                             digits, p->a.complex[ii].real,
                             digits, p->a.complex[ii].imag);
#endif
                    xfprintf (stream, " %*s,", 2 * digits + 19, printf_buf);
                  }
                if (iend > 0)
                  {
#if HAVE_SNPRINTF
                    if (snprintf (printf_buf, PRINTF_BUF_SIZE,
				  "%#.*g + %#.*g*i",
				  digits, p->a.complex[iend - 1].real,
				  digits, p->a.complex[iend - 1].imag)
			>= PRINTF_BUF_SIZE)
		      {
			fail ("Overflow in sprintf buffer.");
			raise_exception ();
		      }
#else
                    sprintf (printf_buf, "%#.*g + %#.*g*i",
                             digits, p->a.complex[iend - 1].real,
                             digits, p->a.complex[iend - 1].imag);
#endif
                    xfprintf (stream, " %*s", 2 * digits + 19, printf_buf);
                  }
                break;
              case character:
                for (ii = istart; ii < iend - 1; ii++)
                  {
                    xfprintf (stream, " \"%s\"%*s,", p->a.character[ii],
                              maxlen - strlen (p->a.character[ii]), "");
                  }
                if (iend > 0)
                  xfprintf (stream, " \"%s\"",
                            p->a.character[iend - 1]);
                break;
              default:
                BAD_TYPE (p->type);
                raise_exception ();
              }
            xfprintf (stream, (i == nn - 1) ? " )\n" : ",\n");
          }
      }
  }
  ON_EXCEPTION
  {
    delete_vector (p);
  }
  END_EXCEPTION;

  delete_vector (p);
}

void
print_vector_sparse (VECTOR *p, FILE *stream)
{
  int i;
  int digits = DIGITS;

  EASSERT (p, vector, 0);
  assert (p->density == sparse);

  WITH_HANDLING
  {
    switch (p->type)
      {
      case integer:
	for (i = 0; i < p->nn; i++)
	  {
	    POLL_SIGINT ();
	    xfprintf (stream, "\t( %d )\t%d\n", p->ja[i], p->a.integer[i]);
	  }
	break;
      case real:
	for (i = 0; i < p->nn; i++)
	  {
	    POLL_SIGINT ();
	    xfprintf (stream, "\t( %d )\t%#.*g\n",
		      p->ja[i], digits, p->a.real[i]);
	  }
	break;
      case complex:
	for (i = 0; i < p->nn; i++)
	  {
	    POLL_SIGINT ();
	    xfprintf (stream, "\t( %d )\t%#.*g + %#.*g*i\n",
		      p->ja[i], digits, p->a.complex[i].real,
		      digits, p->a.complex[i].imag);
	  }
	break;
      case character:
	for (i = 0; i < p->nn; i++)
	  {
	    POLL_SIGINT ();
	    xfprintf (stream, "\t( %d )\t\"%s\"\n", p->ja[i],
		      p->a.character[i]);
	  }
	break;
      default:
	BAD_TYPE (p->type);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_vector (p);
  }
  END_EXCEPTION;

  delete_vector (p);
}

void
print_function (FUNCTION *p, FILE *stream)
{
  if (p->ilk == user)
    {
      extern int da_flag;

      if (da_flag)
	da_function (p->funct_p, stream);
      else
	xfprintf (stream, "\t(user function)\n");

      /* Report the global variables used. */

      var_ref_function (p->funct_p, stream);
    }
  else
    xfprintf (stream, "\t(built-in function)\n");

  delete_function (p);
}

char *
get_prompt (int i)
{
  /*
   * This routine returns the prompt.  There are actually multiple prompts
   * (0, 1, 2, ...), specified by the argument `i'.  The global variable
   * `print_string' is supposed to contain a character vector that
   * has the strings in it.  If we can't make sense of it, then we
   * return a NULL string.  The user can change it through the
   * `$prompt' variable.  No malloc is done -- don't call free on the
   * pointer returned.
   */

  char *c = NULL;

#define PPTR prompt_string->data.ptr

  if (prompt_string->type == D_ENTITY)
    {
      switch (ENT (PPTR)->class)
	{
	case scalar:
	  if (i == 0 && ((SCALAR *) PPTR)->type == character)
	    {
	      c = dup_char (((SCALAR *) PPTR)->v.character);
	    }
	  break;
	case vector:
	  if (((VECTOR *) PPTR)->type == character &&
	      ((VECTOR *) PPTR)->ne > i)
	    {
	      c = get_vector_element_character (
			   (VECTOR *) copy_vector ((VECTOR *) PPTR), i + 1);
	    }
	  break;
	case matrix:
	  if (((MATRIX *) PPTR)->type == character)
	    {
	      if (((MATRIX *) PPTR)->nr == 1)
		{
		  if (((MATRIX *) PPTR)->nc > i)
		    {
		      c = get_matrix_element_character (
			(MATRIX *) copy_matrix ((MATRIX *) PPTR), 1, i + 1);
		    }
		}
	      else if (((MATRIX *) PPTR)->nc == 1)
		{
		  if (((MATRIX *) PPTR)->nr > i)
		    {
		      c = get_matrix_element_character (
			(MATRIX *) copy_matrix ((MATRIX *) PPTR), i + 1, 1);
		    }
		}
	    }
	  break;
	}
    }

  return c ? c : NULL_string;
}

ENTITY *
bi_digits (int n, ENTITY *prec)
{
  /*
   * If `prec' is not NULL, it specifies `digits', the number of
   * significant digits to print.  The value of `digits' is returned.
   */

  if (prec)
    {

      int d;

      /* Convert to an integer scalar. */

      prec = cast_scalar ((SCALAR *) scalar_entity (prec), integer);
      d = ((SCALAR *) prec)->v.integer;

      /* Set the number of digits. */

      delete_datum (num_digits);
      IVAL (num_digits) = d;
      num_digits->type = D_INT;

    }
  else
    {

      prec = int_to_scalar (DIGITS);

    }

  return prec;
}
