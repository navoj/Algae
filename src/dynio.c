/*
   dynio.c -- DYNASTY file I/O.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: dynio.c,v 1.3 2003/08/01 04:57:47 ksh Exp $";

#include "dynio.h"
#include "algae.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "file_io.h"
#include "vargs.h"
#include "dense.h"

typedef struct
  {
    int mid;
    int nrec;
    char aname[80];
    int nr;
    int nc;
    int ln;
    int ntype;
    int buf[20];
  }
DYNASTY_HEADER;

static int PROTO (putdyn_entity, (char *aname, ENTITY * x, va_list arg));
static void *PROTO (get_record, (FILE * fp));
static void *PROTO (get_record_or_eof, (FILE * fp));
static void PROTO (put_record, (void *p, int n, FILE * fp));
static int PROTO (dynasty_ntype, (ENTITY * e));
static TYPE PROTO (dynasty_type, (int ntype));

#if BINARY_FORMAT == CRAY_FLOAT
#define DYNASTY_EOT 4994303008731832352
#else
#define DYNASTY_EOT 1162826784
#endif

#define FREAD( ptr, size, nobj, stream ) \
       if ( fread( ptr, size, nobj, stream ) < (size_t) nobj ) { \
           READ_FAIL( stream ); \
	   raise_exception(); \
       } else;
#define FSEEK( stream, offset, origin ) \
       if ( fseek( stream, offset, origin ) ) { \
           READ_FAIL( stream ); \
	   raise_exception(); \
       } else;
#define FWRITE( ptr, size, nobj, stream ) \
       if ( fwrite( ptr, size, nobj, stream ) < (size_t) nobj ) { \
           WRITE_FAIL( stream ); \
	   raise_exception(); \
       } else;

#if BINARY_FORMAT == CRAY_FLOAT
static int block_count, word_count, forward_index, unused_bits, starting_block;
#endif

ENTITY *
bi_getdyn (int n, ENTITY *fp)
{
  int i, j;
  MATRIX *op = NULL;
  VECTOR *rlab = NULL;
  VECTOR *clab = NULL;
  TABLE *t;

  int nr, nc, ntype;
  TYPE type, rtype, ctype;
  char *cspace;
  char aname[81];
  char *name = NULL;
  FILE *stream;
  DYNASTY_HEADER *head = NULL;

  EASSERT (fp, 0, 0);

  t = (TABLE *) make_table ();

  WITH_HANDLING
  {

    /* Open the file. */

    stream = fp ?
      find_file (entity_to_string (EAT (fp)), FILE_INPUT) : stdin;
    if (!stream)
      raise_exception ();

    /* Loop over each matrix. */

#if BINARY_FORMAT == CRAY_FLOAT
    forward_index = 0;
#endif

    while (1)
      {
	/* Read the DYNASTY header. */

	if (!(head = get_record_or_eof (stream)) ||
	    head->mid == DYNASTY_EOT)
	  break;

	/* Check the dimensions. */

	nr = head->nr;
	nc = head->nc;
	ntype = head->ntype;

	if (nr <= 0)
	  {
	    fail ("Can't have %d rows.", nr);
	    raise_exception ();
	  }
	if (nc <= 0)
	  {
	    fail ("Can't have %d columns.", nc);
	    raise_exception ();
	  }

	/* Read the row labels, if any. */

	if (head->buf[0])
	  {

	    rtype = dynasty_type (head->buf[10]);

	    if (rtype == character)
	      {

		rlab = (VECTOR *) form_vector (nr, rtype, dense);
		cspace = get_record (stream);
		for (i = 0; i < nr; i++)
		  {

		    /* strip trailing blanks */

		    int k = head->buf[10];
		    while (k && cspace[i * head->buf[10] + k - 1] == ' ')
		      k--;

		    /* now copy it */

		    rlab->a.character[i] = MALLOC (k + 1);
		    memcpy (rlab->a.character[i],
			    cspace + i * head->buf[10], k);
		    rlab->a.character[i][k] = '\0';
		  }
		FREE (cspace);

	      }
	    else
	      {

		void *v = get_record (stream);

		rlab = (VECTOR *) form_vector (nr, rtype, dense);

		if (head->buf[10] == -1)
		  {
		    for (i = 0; i < nr; i++)
		      {
			rlab->a.real[i] = (REAL) * (((float *) v) + i);
		      }
		  }
		else if (head->buf[10] == -6)
		  {
		    for (i = 0; i < 2 * nr; i++)
		      {
			rlab->a.real[i] = (REAL) * (((float *) v) + i);
		      }
		  }
		else
		  {
		    memcpy (rlab->a.integer, v, nr * type_size[rtype]);
		  }

		FREE (v);
	      }

	  }

	/* Get the column labels, if any. */

	if (head->buf[1])
	  {

	    ctype = dynasty_type (head->buf[11]);

	    if (ctype == character)
	      {

		clab = (VECTOR *) form_vector (nc, ctype, dense);
		cspace = get_record (stream);
		for (i = 0; i < nc; i++)
		  {

		    /* strip trailing blanks */

		    int k = head->buf[11];
		    while (k && cspace[i * head->buf[11] + k - 1] == ' ')
		      k--;

		    /* now copy it */

		    clab->a.character[i] = MALLOC (k + 1);
		    memcpy (clab->a.character[i],
			    cspace + i * head->buf[11], k);
		    clab->a.character[i][k] = '\0';
		  }
		FREE (cspace);

	      }
	    else
	      {

		void *v = get_record (stream);

		clab = (VECTOR *) form_vector (nc, ctype, dense);

		if (head->buf[11] == -1)
		  {
		    for (i = 0; i < nc; i++)
		      {
			clab->a.real[i] = (REAL) * (((float *) v) + i);
		      }
		  }
		else if (head->buf[11] == -6)
		  {
		    for (i = 0; i < 2 * nc; i++)
		      {
			clab->a.real[i] = (REAL) * (((float *) v) + i);
		      }
		  }
		else
		  {
		    memcpy (clab->a.integer, v, nc * type_size[ctype]);
		  }

		FREE (v);
	      }

	  }

	/* Now read the data. */

	type = dynasty_type (ntype);
	op = (MATRIX *) form_matrix (nr, nc, type, dense);

	if (type == character)
	  {

	    int k;

	    for (j = 0; j < nc; j++)
	      {

		cspace = get_record (stream);

		for (i = 0; i < nr; i++)
		  {

		    /* strip trailing blanks */

		    k = ntype;
		    while (k && cspace[i * ntype + k - 1] == ' ')
		      k--;

		    /* now copy it */

		    op->a.character[i + j * nr] = MALLOC (k + 1);
		    memcpy (op->a.character[i + j * nr], cspace + i * ntype, k);
		    op->a.character[i + j * nr][k] = '\0';
		  }
		FREE (cspace);
	      }

	  }
	else
	  {

	    void *v;

	    for (j = 0; j < nc; j++)
	      {

		v = get_record (stream);

		if (ntype == -1)
		  {		/* single precision */
		    for (i = 0; i < nr; i++)
		      {
			op->a.real[i + j * nr] = (REAL) * (((float *) v) + i);
		      }
		  }
		else if (ntype == -6)
		  {		/* single precision complex */
		    for (i = 0; i < 2 * nr; i++)
		      {
			op->a.real[i + 2 * j * nr] = (REAL) * (((float *) v) + i);
		      }
		  }
		else
		  {
		    memcpy ((char *) op->a.ptr + j * nr * type_size[type],
			    v, nr * type_size[type]);
		  }

		FREE (v);
	      }
	  }

	/* Create the name. */

	memcpy (aname, head->aname, 80);
	aname[80] = '\0';
	name = aname;
	while (isspace (*name))
	  name++;
	if (*name == '[' && (name = strtok (++name, " ,]")))
	  {
	    name = dup_char (name);
	  }
	else
	  {
	    sprintf (aname, "%d", head->mid);
	    name = dup_char (aname);
	  }

	TFREE (head);
	op->rid = EAT (rlab);
	op->cid = EAT (clab);

	/* Add to table. */

	t = (TABLE *) replace_in_table (EAT (t), EAT (op), name);

      }
  }
  ON_EXCEPTION
  {
    TFREE (head);
    delete_entity (fp);
    delete_matrix (op);
    delete_2_vectors (rlab, clab);
    delete_table (t);
  }
  END_EXCEPTION;

  return (ENT (t));
}

#if BINARY_FORMAT == CRAY_FLOAT

static void
write_cray_eof (FILE *stream)
{
  RCW rcw;
  BCW bcw;

  bcw.m = 0;
  bcw.u1 = bcw.bdf = bcw.u2 = 0;
  rcw.m = 8;
  rcw.tran = rcw.bdf = rcw.srs = rcw.u1 = 0;

  /* Write EOR. */

  if (word_count == 512)
    {
      bcw.bn = ++block_count;
      bcw.fwi = 0;
      FWRITE (&bcw, 8, 1, stream);
      word_count = 0;
    }
  rcw.ubc = unused_bits;
  rcw.pf = block_count;
  rcw.pri = block_count - starting_block;
  rcw.fwi = 0;
  FWRITE (&rcw, 8, 1, stream);
  word_count++;
  unused_bits = 0;

  /* Write EOF. */

  if (word_count == 512)
    {
      bcw.bn = ++block_count;
      bcw.fwi = 0;
      FWRITE (&bcw, 8, 1, stream);
      word_count = 0;
    }
  rcw.m = 14;
  rcw.ubc = 0;
  rcw.pf = block_count;
  rcw.pri = block_count - starting_block;
  FWRITE (&rcw, 8, 1, stream);
  word_count++;
  if (word_count == 512)
    {
      bcw.bn = ++block_count;
      bcw.fwi = 0;
      FWRITE (&bcw, 8, 1, stream);
      word_count = 0;
    }
  rcw.m = 15;
  rcw.pf = block_count;
  rcw.pri = block_count - starting_block;
  FWRITE (&rcw, 8, 1, stream);
}

#endif

ENTITY *
bi_putdyn (int n, ENTITY *t, ENTITY *fname)
{
  FILE * volatile stream;
  int num = 1;
  ENTITY *r;

  EASSERT (t, 0, 0);

  WITH_HANDLING
  {

    /* Open the file. */

    stream = fname ?
      find_file (entity_to_string (EAT (fname)), FILE_OUTPUT) : stdout;

    if (t->class != table)
      {
	fail ("Not a table.");
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_2_entities (t, fname);
  }
  END_EXCEPTION;

  if (!stream)
    {
      delete_entity (t);
      return NULL;
    }

#if BINARY_FORMAT == CRAY_FLOAT
  block_count = -1;		/* negative means initialize */
#endif

  r = visit_table ((TABLE *) t, putdyn_entity, stream, &num) ?
    int_to_scalar (1) : NULL;

#if BINARY_FORMAT == CRAY_FLOAT
  write_cray_eof (stream);
#endif

  return r;
}

static int
putdyn_entity (char *aname, ENTITY *x, va_list arg)
{
  MATRIX * volatile m = NULL;
  VECTOR * volatile rlab = NULL;
  VECTOR * volatile clab = NULL;
  int i;
  DYNASTY_HEADER *h = NULL;

  FILE *stream;
  int *num;

  stream = va_arg (arg, FILE *);
  num = va_arg (arg, int *);

  EASSERT (x, 0, 0);

  WITH_HANDLING
  {
    /* Make sure `x' is a dense matrix. */

    m = (MATRIX *) dense_matrix ((MATRIX *) matrix_entity (EAT (x)));

    /* Can't have zero rows or columns. */

    if (m->nr <= 0 || m->nc <= 0)
      {
	fail ("Matrix has no elements.");
	raise_exception ();
      }

    /* Fill in the header. */

    h = (DYNASTY_HEADER *) CALLOC (1, sizeof (DYNASTY_HEADER));
    h->mid = *num;
    h->nrec = m->nc + (m->rid != NULL) + (m->cid != NULL);
    h->aname[0] = '[';
    strncpy (h->aname + 1, aname, 79);
    h->aname[strlen (aname) + 1] = ']';
    for (i = strlen (aname) + 2; i < 80; i++)
      h->aname[i] = ' ';
    TFREE (aname);
    h->nr = m->nr;
    h->nc = m->nc;
    h->ln = (*num)++;
    h->ntype = dynasty_ntype (ENT (m));
    h->buf[2] = 1;

    /* Take care of labels. */

    if (m->rid)
      {
	h->buf[0] = 1;
	rlab =
	  (VECTOR *) dense_vector ((VECTOR *) copy_vector ((VECTOR *) m->rid));
	h->buf[10] = dynasty_ntype (ENT (rlab));
      }
    if (m->cid)
      {
	h->buf[1] = 1;
	clab =
	  (VECTOR *) dense_vector ((VECTOR *) copy_vector ((VECTOR *) m->cid));
	h->buf[11] = dynasty_ntype (ENT (clab));
      }

    /* Write the DYNASTY header. */

    put_record (h, sizeof (DYNASTY_HEADER), stream);

    /* Write row labels. */

    if (rlab)
      {
	if (h->buf[10] <= 0)
	  {
	    put_record (rlab->a.ptr, rlab->nn * type_size[rlab->type],
			stream);
	  }
	else
	  {
	    int kk;
	    int k = h->buf[10];
	    char *cspace = MALLOC (k * rlab->nn);
	    assert (rlab->type == character);
	    memset (cspace, ' ', k * rlab->nn);
	    for (i = 0; i < rlab->nn; i++)
	      {
		kk = strlen (rlab->a.character[i]);
		if (kk > 0)
		  {
		    memcpy (cspace + i * k, rlab->a.character[i], kk);
		  }
	      }
	    put_record (cspace, k * rlab->nn, stream);
	    FREE (cspace);
	  }
      }

    /* Write column labels. */

    if (clab)
      {
	if (h->buf[11] <= 0)
	  {
	    put_record (clab->a.ptr, clab->nn * type_size[clab->type],
			stream);
	  }
	else
	  {
	    int kk;
	    int k = h->buf[11];
	    char *cspace = MALLOC (k * clab->nn);
	    assert (clab->type == character);
	    memset (cspace, ' ', k * clab->nn);
	    for (i = 0; i < clab->nn; i++)
	      {
		kk = strlen (clab->a.character[i]);
		if (kk > 0)
		  {
		    memcpy (cspace + i * k, clab->a.character[i], kk);
		  }
	      }
	    put_record (cspace, k * clab->nn, stream);
	    FREE (cspace);
	  }
      }

    /* Write the data. */

    if (h->ntype <= 0)
      {
	for (i = 0; i < h->nc; i++)
	  {
	    put_record ((char *) m->a.ptr + i * h->nr * type_size[m->type],
			m->nr * type_size[m->type], stream);
	  }
      }
    else
      {
	int j, kk;
	int k = h->ntype;
	char *cspace = MALLOC (k * m->nr);
	assert (m->type == character);
	for (j = 0; j < h->nc; j++)
	  {
	    memset (cspace, ' ', k * m->nr);
	    for (i = 0; i < m->nr; i++)
	      {
		kk = strlen (m->a.character[i + j * h->nr]);
		if (kk > 0)
		  {
		    memcpy (cspace + i * k, m->a.character[i + j * h->nr], kk);
		  }
	      }
	    put_record (cspace, k * m->nr, stream);
	  }
	FREE (cspace);
      }
  }
  ON_EXCEPTION
  {
    delete_entity (x);
    delete_matrix (m);
    delete_2_vectors (rlab, clab);
    TFREE (h);
  }
  END_EXCEPTION;

  delete_matrix (m);
  delete_2_vectors (rlab, clab);
  FREE (h);

  return 1;
}

static void *
get_record (FILE *stream)
{
  void *x = get_record_or_eof (stream);
  if (!x)
    {
      fail ("Can't read file (unexpected end of file).");
      raise_exception ();
    }
  return x;
}

static void *
get_record_or_eof (FILE *stream)
{
  /*
   * This function reads a record in a FORTRAN unformatted, sequential
   * file.  When this function is called, the current position in the
   * file should be at the start of a record.  This function then reads
   * that record and leaves the current position at the beginning of the
   * next record.  Returns NULL if at EOF.
   */

  void *x = NULL;

#if BINARY_FORMAT == CRAY_FLOAT

  /* Read a Cray FORTRAN binary. */

  CW cw;
  size_t x_size = 0;

  for (;;)
    {
      if (forward_index)
	{
	  x_size += forward_index;
	  x = REALLOC (x, x_size * sizeof (CW));
	  FREAD ((CW *) x + x_size - forward_index, 8, forward_index, stream);
	}

      /* Next word is a control word. */

      FREAD (&cw.bcw, sizeof (CW), 1, stream);

      switch (cw.bcw.m)
	{
	case 0:		/* BCW */

	  if (debug_level > 1)
	    inform ("Read block control word #%d.", cw.bcw.bn);
	  forward_index = cw.bcw.fwi;
	  break;

	case 8:		/* EOR */

	  if (debug_level > 1)
	    inform ("Read EOR.");
	  forward_index = cw.rcw.fwi;
	  return x;

	case 14:		/* EOF */
	case 15:		/* EOD */

	  if (debug_level > 1)
	    inform ("Read EOF or EOD.");
	  TFREE (x);
	  return NULL;

	default:

	  fail ("Improper blocking in FORTRAN file.");
	  raise_exception ();
	}
    }

#else

  int length, tail;

  /*
   * This works for systems that have the length of the record (an int)
   * at both the front and back of the data.
   */

  if (fread (&length, sizeof (int), 1, stream) < 1)
    {
      if (feof (stream))
	{
	  return NULL;
	}
      else
	{
	  fail ("Can't read file (%s).", strerror (errno));
	  raise_exception ();
	}
    }

  if (length)
    {
#if FORTRAN_BINARY_BY_WORDS
      length *= 4;
#endif /* FORTRAN_BINARY_BY_WORDS */
      x = MALLOC (length);
      FREAD (x, 1, length, stream);
#if FORTRAN_BINARY_BY_WORDS
      length /= 4;
#endif /* FORTRAN_BINARY_BY_WORDS */
    }
  else
    x = MALLOC (1);

  FREAD (&tail, sizeof (int), 1, stream);

  if (length != tail)
    {
      fail ("Bad record in file.");
      raise_exception ();
    }

#endif

  return x;
}

static void
put_record (void *p, int n, FILE *stream)
{
  /*
   * This function writes a record in a FORTRAN unformatted,
   * sequential file.
   */

#if BINARY_FORMAT == CRAY_FLOAT

  double *d = p;
  BCW bcw;
  RCW rcw;
  int m;
  double dummy = 0.0;

  bcw.m = 0;
  bcw.u1 = bcw.bdf = bcw.u2 = 0;
  rcw.m = 8;
  rcw.tran = rcw.bdf = rcw.srs = rcw.u1 = 0;

  /* Must write EOR, unless this is the first record. */

  if (block_count >= 0)
    {
      if (word_count == 512)
	{
	  bcw.bn = ++block_count;
	  bcw.fwi = 0;
	  FWRITE (&bcw, 8, 1, stream);
	  word_count = 1;
	}
      rcw.ubc = unused_bits;
      rcw.pf = block_count;
      rcw.pri = block_count - starting_block;
      word_count++;
      m = (int) ceil ((double) n / (double) 8);
      if (m + word_count > 511)
	m = 512 - word_count;
      rcw.fwi = m;
      FWRITE (&rcw, 8, 1, stream);
      unused_bits = 0;
    }
  else
    {
      block_count = -1;
      starting_block = 0;
      word_count = 512;
    }

  for (;;)
    {
      if (word_count == 512)
	{
	  m = (int) ceil ((double) n / (double) 8);
	  if (m > 511)
	    m = 511;

	  bcw.bn = ++block_count;
	  bcw.fwi = m;
	  FWRITE (&bcw, 8, 1, stream);
	  word_count = 1;
	}
      else if (n > 7)
	{
	  m = n / 8;
	  if (m + word_count > 511)
	    m = 512 - word_count;
	  FWRITE (d, 8, m, stream);
	  word_count += m;
	  n -= 8 * m;
	  d += m;
	}
      else if (n > 0)
	{
	  FWRITE (d, 1, n, stream);
	  FWRITE (&dummy, 1, 8 - n, stream);
	  unused_bits = 64 - 8 * n;
	  word_count++;
	  n = 0;
	}
      else
	break;
    }

#else

#if FORTRAN_BINARY_BY_WORDS
  char z = '\0';
  int actual = n;
  int length = (n / 4) * 4;
  if (n % 4)
    length += 4;
  n = length / 4;
  FWRITE (&n, sizeof (int), 1, stream);
  FWRITE (p, actual, 1, stream);
  while (actual++ < length)
    FWRITE (&z, 1, 1, stream);
  FWRITE (&n, sizeof (int), 1, stream);
#else
  FWRITE (&n, sizeof (int), 1, stream);
  FWRITE (p, n, 1, stream);
  FWRITE (&n, sizeof (int), 1, stream);
#endif

#endif
}

static TYPE
dynasty_type (int ntype)
{
  switch (ntype)
    {
    case 0:			/* I1 */
      return integer;
    case -1:			/* R1 */
    case -2:			/* R2 */
      return real;
    case -6:			/* Z2 */
    case -7:			/* Z4 */
      return complex;
    default:
      if (ntype > 0)
	{			/* Cn */
	  return character;
	}
      else
	{
	  fail ("Can't read this DYNASTY format.");
	  raise_exception ();
	}
    }
}

static int
dynasty_ntype (ENTITY *e)
{
  switch (e->class)
    {
    case vector:
      switch (((VECTOR *) e)->type)
	{
	case integer:
	  return 0;
	case real:
	  return -2;
	case complex:
	  return -7;
	case character:
	  {
	    int i, kk;
	    int k = 1;
	    assert (((VECTOR *) e)->density == dense);
	    for (i = 0; i < ((VECTOR *) e)->nn; i++)
	      {
		kk = strlen (((VECTOR *) e)->a.character[i]);
		if (kk > k)
		  k = kk;
	      }
	    return k;
	  }
	default:
	  wipeout ("Bad type.");
	}
    case matrix:
      switch (((MATRIX *) e)->type)
	{
	case integer:
	  return 0;
	case real:
	  return -2;
	case complex:
	  return -7;
	case character:
	  {
	    int i, kk;
	    int k = 1;
	    assert (((MATRIX *) e)->density == dense);
	    for (i = 0; i < ((MATRIX *) e)->nn; i++)
	      {
		kk = strlen (((MATRIX *) e)->a.character[i]);
		if (kk > k)
		  k = kk;
	      }
	    return k;
	  }
	default:
	  wipeout ("Bad type.");
	}
    default:
      wipeout ("Bad class.");
    }
}
