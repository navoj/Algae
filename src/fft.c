/*
   fft.c -- Fast Fourier Transform.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: fft.c,v 1.7 2003/09/21 04:47:16 ksh Exp $";

#include "fft.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "cast.h"
#include "dense.h"
#include "transpose.h"

static ENTITY *PROTO (modify_fft_labels, (VECTOR * p));

static char *fft_options[] =
{
  "row",
  "estimate",
  "measure",
  "patient",
  "exhaustive",
  "forget",
  NULL,
};
enum
  {
    FFT_OPT_ROW,
    FFT_OPT_ESTIMATE,
    FFT_OPT_MEASURE,
    FFT_OPT_PATIENT,
    FFT_OPT_EXHAUSTIVE,
    FFT_OPT_FORGET,
  };

#if HAVE_FFTW

static void PROTO (hermcpy, (COMPLEX *v, int x));

ENTITY *
bi_fftw (int n, ENTITY *p, ENTITY *d)
{
  int dim, rigor;
  ENTITY * volatile s = NULL;

  EASSERT (p, 0, 0);

  if (d != NULL && d->class != table)
    {
      fail ("Invalid options -- not a table.");
      delete_2_entities (p, d);
      raise_exception ();
    }

  if (!OK_OPTIONS ((TABLE *) d, fft_options)) raise_exception ();

  dim = !IN_TABLE (d, fft_options[FFT_OPT_ROW]);

  if (IN_TABLE (d, fft_options[FFT_OPT_FORGET]))
    fftw_forget_wisdom();

  rigor = FFTW_ESTIMATE;
  if (IN_TABLE (d, fft_options[FFT_OPT_MEASURE]))
    rigor = FFTW_MEASURE;
  if (IN_TABLE (d, fft_options[FFT_OPT_PATIENT]))
    rigor = FFTW_PATIENT;
  if (IN_TABLE (d, fft_options[FFT_OPT_EXHAUSTIVE]))
    rigor = FFTW_EXHAUSTIVE;

  WITH_HANDLING
    {
      switch (p->class)
	{
	case scalar:
	  s = fftw_vector ((VECTOR *) scalar_to_vector ((SCALAR *) EAT(p)),
			   rigor);
	  break;

	case vector:
	  s = fftw_vector ((VECTOR *) EAT(p), rigor);
	  break;

	case matrix:

	  /* FFT of rows (dim=0); FFT of cols (dim=1) */
	  s = fftw_matrix ((MATRIX *) EAT(p), dim, rigor);
	  break;

	default:
	  BAD_CLASS (p->class);
	  raise_exception ();
	}
    }
  ON_EXCEPTION
    {
      delete_2_entities (p, d);
    }
  END_EXCEPTION;

  delete_entity (d);
  return s;
}

ENTITY *
fftw_vector (VECTOR *p, int rigor)
{
  REAL *tmpd = NULL;
  COMPLEX *tmpc = NULL;
  VECTOR *c = NULL;
  fftw_plan plan = NULL;
  int i, j, nn;

  EASSERT (p, vector, 0);

  WITH_HANDLING
  {

    p = (VECTOR *) dense_vector (EAT (p));

    switch (p->type)
      {
      case integer:
	p = (VECTOR *) cast_vector_integer_real (EAT (p));
        /* no break fall through */

      case real:
	c = (VECTOR *) cast_vector_real_complex ((VECTOR *) copy_vector (p));

	nn = p->nn;

	if (nn == 0)
	  goto out;		/* done if zero length */

	if (rigor == FFTW_ESTIMATE)
	  {
	    /* Create the fftw plan */
	    plan = fftw_plan_dft_r2c_1d (nn, p->a.real,
					 (fftw_complex *) c->a.real, rigor);

	    if (plan == NULL) {
	      fail ("FFTW failed to make a plan.");
	      raise_exception ();
	    }

	    /* Execute the transform */
	    fftw_execute (plan);
	  }
	else
	  {
	    tmpd = MALLOC (nn * sizeof (REAL));

	    /* Create the fftw plan */
	    plan = fftw_plan_dft_r2c_1d (nn, tmpd,
					 (fftw_complex *) c->a.real, rigor);

	    if (plan == NULL) {
	      fail ("FFTW failed to make a plan.");
	      raise_exception ();
	    }

	    memcpy (tmpd, p->a.real, nn * sizeof (REAL));

	    /* Execute the transform */
	    fftw_execute (plan);

	    FREE (EAT (tmpd));
	  }

        /* Hermitian copy and reorder frequencies */
        hermcpy (c->a.complex, nn);

        break;

      case complex:

	c = (VECTOR *) dup_vector (EAT (p));

	nn = c->nn;

	if (nn == 0)
	  goto out;		/* done if zero length */

	tmpc = MALLOC (2 * nn * sizeof (REAL));

        /* Create the fftw plan */
        plan = fftw_plan_dft_1d(nn, (fftw_complex *) tmpc,
				(fftw_complex *) tmpc, FFTW_FORWARD, rigor);

        if (plan == NULL) {
	    fail ("FFTW failed to make a plan.");
	    raise_exception ();
        }

        /* Copy the data. */
	memcpy (tmpc, c->a.complex, 2*nn*sizeof(REAL));

        /* Execute the transform */
        fftw_execute (plan);

        /* copy with reorder */
        for (i=0,j=(nn-1)/2; j<nn; i++,j++) {
	  c->a.complex[j].real = tmpc[i].real;
	  c->a.complex[j].imag = tmpc[i].imag;
        }
        for (i=(nn+2)/2,j=0; i<nn; i++,j++) {
	  c->a.complex[j].real = tmpc[i].real;
	  c->a.complex[j].imag = tmpc[i].imag;
        }

	FREE (EAT (tmpc));

	break;

      default:
	fail ("Can't apply \"fft\" to a %s vector.", type_string[p->type]);
	raise_exception ();
      }

    if (c->eid)
      c->eid = modify_fft_labels ((VECTOR *) EAT (c->eid));
  }

out:				/* can't jump past the ON_EXCEPTION */

  ON_EXCEPTION
  {
    if (plan != NULL) fftw_destroy_plan (plan);
    TFREE (tmpc);
    TFREE (tmpd);
    delete_2_vectors (p, c);
  }
  END_EXCEPTION;

  fftw_destroy_plan (plan);
  delete_vector (p);
  return ENT (c);
}

ENTITY *
fftw_matrix (p, dim, rigor)
     MATRIX *p;
     int dim;  /* FFT of rows (dim=0); FFT of cols (dim=1) */
     int rigor;
{
  REAL *tmpd = NULL;
  COMPLEX *tmpc = NULL;
  fftw_plan plan = NULL;
  int i, j, l, nr, nc, nn;

  EASSERT (p, matrix, 0);

  WITH_HANDLING
  {
    p = (MATRIX *) dense_matrix (EAT (p));

    switch (p->type)
      {
      case integer:
	p = (MATRIX *) cast_matrix_integer_real (EAT (p));
        /* no break fall through */

      case real:
	p = (MATRIX *) cast_matrix_real_complex (EAT (p));

        if ( dim == 0 )   /* Take the FFT of each row */
        {
          nr = p->nr;     /* Number of rows */
          nn = p->nc;     /* Number to columns (elements per row) */

	  if (nn == 0)
	    goto out;     /* Done if columns have zero length */

	  if (nr)
	    {
	      /* Allocate space - transform not done in place */
	      tmpc = MALLOC (2 * nn * sizeof(REAL));
	      tmpd = MALLOC (nn * sizeof(REAL));

	      /* Create the fftw plan */
	      plan = fftw_plan_dft_r2c_1d(nn, tmpd,
					  (fftw_complex *) tmpc, rigor);

	      if (plan == NULL) {
		fail ("FFTW failed to make a plan.");
		raise_exception ();
	      }

	      for (l = 0; l < nr; l++)
		{
		  /* Copy the data */
		  for (i=0; i<nn; i++) {
		    tmpd[i] = p->a.complex[l+i*nr].real;
		  }

		  /* Execute the transform */
		  fftw_execute(plan);

		  /* Hermitian copy and reorder frequencies */
		  hermcpy((COMPLEX *) tmpc, nn);

		  /* copy the data */
		  for (i=0; i<nn; i++) {
		    p->a.complex[l+i*nr].real = tmpc[i].real;
		    p->a.complex[l+i*nr].imag = tmpc[i].imag;
		  }

		} /* l - done loop over each row */

	      /* Destroy plan and tmpc arrays */
	      FREE (EAT (tmpd));
	      FREE (EAT (tmpc));
	    }

          if (p->cid)
            p->cid = modify_fft_labels ((VECTOR *) EAT (p->cid));
        }
        else              /* Take the FFT of each column */
        {
          nc = p->nc;     /* Number of columns */
          nn = p->nr;     /* Number to rows (elements per column) */

	  if (nn == 0)
	    goto out;     /* Done if rows have zero length */

	  if (nc)
	    {
	      /* Allocate space - transform not done in place */
	      tmpc = MALLOC (2 * nn * sizeof(REAL));
	      tmpd = MALLOC (nn * sizeof(REAL));

	      /* Create the fftw plan */
	      plan = fftw_plan_dft_r2c_1d(nn, tmpd,
					  (fftw_complex *) tmpc, rigor);

	      if (plan == NULL) {
		fail ("FFTW failed to make a plan.");
		raise_exception ();
	      }

	      for (l = 0; l < nc; l++)
		{
		  /* Copy the data */
		  for (i=0; i<nn; i++) {
		    tmpd[i] = p->a.complex[i+l*nn].real;
		  }

		  /* Execute the transform */
		  fftw_execute(plan);

		  /* Hermitian copy and reorder frequencies */
		  hermcpy((COMPLEX *) tmpc, nn);

		  /* copy the data */
		  for (i=0; i<nn; i++) {
		    p->a.complex[i+l*nn].real = tmpc[i].real;
		    p->a.complex[i+l*nn].imag = tmpc[i].imag;
		  }

		} /* l - done loop over each column */

	      FREE (EAT (tmpd));
	      FREE (EAT (tmpc));
	    }

          if (p->rid)
            p->rid = modify_fft_labels ((VECTOR *) EAT (p->rid));

        }

        break;

      case complex:

	p = (MATRIX *) dup_matrix (EAT (p));

        if ( dim == 0 )   /* Take the FFT of each row */
        {
          nr = p->nr;     /* Number of rows */
          nn = p->nc;     /* Number to columns (elements per row) */

	  if (nn == 0)
	    goto out;     /* Done if columns have zero length */

	  if (nr)
	    {
	      /* Allocate space - transform done in place */
	      tmpc = MALLOC (2 * nn * sizeof(REAL));

	      /* Create the fftw plan */
	      plan = fftw_plan_dft_1d (nn, (fftw_complex *) tmpc,
				       (fftw_complex *) tmpc, FFTW_FORWARD,
				       rigor);

	      if (plan == NULL) {
		fail ("FFTW failed to make a plan.");
		raise_exception ();
	      }

	      for (l = 0; l < nr; l++)
		{
		  /* Copy the data */
		  for (i=0; i<nn; i++) {
		    tmpc[i].real = p->a.complex[l+i*nr].real;
		    tmpc[i].imag = p->a.complex[l+i*nr].imag;
		  }

		  /* Execute the transform */
		  fftw_execute(plan);

		  /* copy with reorder */
		  for (i=0,j=l+((nn-1)/2)*nr; i<=nn/2; i++,j+=nr) {
		    p->a.complex[j].real = tmpc[i].real;
		    p->a.complex[j].imag = tmpc[i].imag;
		  }
		  for (i=(nn+2)/2,j=l; i<nn; i++,j+=nr) {
		    p->a.complex[j].real = tmpc[i].real;
		    p->a.complex[j].imag = tmpc[i].imag;
		  }

		} /* l - done loop over each row */

	      FREE (EAT (tmpc));
	    }

          if (p->cid)
            p->cid = modify_fft_labels ((VECTOR *) EAT (p->cid));
        }
        else              /* Take the FFT of each column */
        {
          nc = p->nc;     /* Number of columns */
          nn = p->nr;     /* Number to rows (elements per column) */

	  if (nn == 0)
	    goto out;     /* Done if rows have zero length */

	  if (nc)
	    {
	      /* Allocate space - transform done in place */
	      tmpc = MALLOC (2 * nn * sizeof(REAL));

	      /* Create the fftw plan */
	      plan = fftw_plan_dft_1d (nn, (fftw_complex *) tmpc,
				       (fftw_complex *) tmpc, FFTW_FORWARD,
				       rigor);

	      if (plan == NULL) {
		fail ("FFTW failed to make a plan.");
		raise_exception ();
	      }

	      for (l = 0; l < nc; l++)
		{
		  /* Copy the data */
		  for (i=0; i<nn; i++) {
		    tmpc[i].real = p->a.complex[i+l*nn].real;
		    tmpc[i].imag = p->a.complex[i+l*nn].imag;
		  }

		  /* Execute the transform */
		  fftw_execute(plan);

		  /* copy with reorder */
		  for (i=0,j=(nn-1)/2+l*nn; i<=nn/2; i++,j++) {
		    p->a.complex[j].real = tmpc[i].real;
		    p->a.complex[j].imag = tmpc[i].imag;
		  }
		  for (i=(nn+2)/2,j=l*nn; i<nn; i++,j++) {
		    p->a.complex[j].real = tmpc[i].real;
		    p->a.complex[j].imag = tmpc[i].imag;
		  }

		} /* l - done loop over each column */

	      FREE (EAT (tmpc));
	    }

          if (p->rid)
            p->rid = modify_fft_labels ((VECTOR *) EAT (p->rid));

        }
        break;

      default:
	fail ("Can't apply \"fft\" to a %s matrix.", type_string[p->type]);
	raise_exception ();
      }
  }

out:				/* can't jump past the ON_EXCEPTION */

  ON_EXCEPTION
  {
    if (plan) fftw_destroy_plan (plan);
    TFREE (tmpc);
    TFREE (tmpd);
    delete_matrix (p);
  }
  END_EXCEPTION;

  fftw_destroy_plan (plan);
  return ENT (p);
}

void hermcpy ( COMPLEX *v, int x )
{
    /* Hermetian copy and frequency shift converts:
     * [( 1, 0),( 2, 2),( 3, 3),( 4, 0),( x, x),( x, x)] to
     * [( 3,-3),( 2,-2),( 1, 0),( 2, 2),( 3, 3),( 4, 0)]
     * or
     * [( 1, 0),( 2, 2),( 3, 3),( 4, 4),( x, x),( x, x), ( x, x)] to
     * [( 4,-4),( 3,-3),( 2,-2),( 1, 0),( 2, 2),( 3, 3), ( 4, 4)]
     */

    int i, xx, xm;

    xx = x/2;

    for (i=xx, xm=x-1; i>=0; i--,xm--)
      {
	v[xm].real = v[i].real;
	v[xm].imag = v[i].imag;
      }
    for (i=0,xm=x-1-!(x%2); i<(x-1)/2; i++,xm--)
      {
	v[i].real =  v[xm].real;
	v[i].imag = -v[xm].imag;
      }
}
#endif

#if !HAVE_FFTW || USE_BOTH_FFT

ENTITY *
bi_fft (int n, ENTITY *p, ENTITY *d)
{
  int dim = 1;
  ENTITY * volatile s = NULL;

  EASSERT (p, 0, 0);

  if (d != NULL && d->class != table)
    {
      fail ("Invalid options -- not a table.");
      delete_2_entities (p, d);
      raise_exception ();
    }

  if (!OK_OPTIONS ((TABLE *) d, fft_options)) raise_exception ();

  dim = !IN_TABLE (d, fft_options[FFT_OPT_ROW]);

  WITH_HANDLING
    {
      switch (p->class)
	{
	case scalar:
	  s = fft_vector ((VECTOR *) scalar_to_vector ((SCALAR *) EAT(p)));
	  break;

	case vector:
	  s = fft_vector ((VECTOR *) EAT(p));
	  break;

	case matrix:

	  /* FFT of rows (dim=0); FFT of cols (dim=1) */
	  s = fft_matrix ((MATRIX *) EAT(p), dim);
	  break;

	default:
	  BAD_CLASS (p->class);
	  raise_exception ();
	}
    }
  ON_EXCEPTION
    {
      delete_2_entities (p, d);
    }
  END_EXCEPTION;

  delete_entity (d);
  return s;
}

ENTITY *
fft_vector (VECTOR *p)
{
  REAL *tmp;
  REAL s[4];
  int i, j, k, nn;

  EASSERT (p, vector, 0);

  WITH_HANDLING
  {

    p = (VECTOR *) dense_vector ((VECTOR *) EAT (p));
    p = (VECTOR *) dup_vector (EAT (p));

    switch (p->type)
      {
      case integer:
	p = (VECTOR *) cast_vector_integer_real (p);	/* fall through */
      case real:

	/* The RFFTF routine gives kind of screwy results (sine and */
	/* cosine coefficients) which can't be used for convolution */
	/* in the usual way.  Let's just convert to complex instead. */

	p = (VECTOR *) cast_vector_real_complex ((VECTOR *) EAT (p));

	nn = p->nn;
	if (nn == 0)
	  goto out;		/* done if zero length */

	tmp = (REAL *) CALLOC (4 * nn + 15, sizeof (REAL));

	CFFTI (&nn, tmp);
	CFFTF (&nn, p->a.real, tmp);
	FREE (tmp);

	/* Reorder for increasing frequency from -1/2t to +1/2t */

	j = 2 * ((nn - 1) / 2);
	k = (nn % 2) ? 2 : 4;
	for (i = j; i < j + k; i++)
	  s[i - j] = p->a.real[i];
	for (i = 0; i < j; i++)
	  {
	    p->a.real[2 * nn - j + i - k] = p->a.real[i];
	    p->a.real[i] = p->a.real[2 * nn - j + i];
	  }
	for (i = 2 * nn - k; i < 2 * nn; i++)
	  p->a.real[i] = s[i - 2 * nn + k];
	break;

      case complex:

	nn = p->nn;
	if (nn == 0)
	  goto out;		/* done if zero length */

	tmp = (REAL *) CALLOC (4 * nn + 15, sizeof (REAL));

	CFFTI (&nn, tmp);
	CFFTF (&nn, p->a.real, tmp);
	FREE (tmp);

	/* Reorder for increasing frequency from -1/2t to +1/2t */

	j = 2 * ((nn - 1) / 2);
	k = (nn % 2) ? 2 : 4;
	for (i = j; i < j + k; i++)
	  s[i - j] = p->a.real[i];
	for (i = 0; i < j; i++)
	  {
	    p->a.real[2 * nn - j + i - k] = p->a.real[i];
	    p->a.real[i] = p->a.real[2 * nn - j + i];
	  }
	for (i = 2 * nn - k; i < 2 * nn; i++)
	  p->a.real[i] = s[i - 2 * nn + k];
	break;

      default:
	fail ("Can't apply \"fft\" to a %s vector.",
	      type_string[p->type]);
	raise_exception ();
      }

    if (p->eid)
      p->eid = modify_fft_labels ((VECTOR *) EAT (p->eid));

  }

out:				/* can't jump past the ON_EXCEPTION */

  ON_EXCEPTION
  {
    delete_vector (p);
  }
  END_EXCEPTION;

  return (ENT (p));
}

ENTITY *
fft_matrix (MATRIX *p, int dim)
{
  REAL *tmp;
  REAL s[4];
  int i, j, k, m, nn;

  EASSERT (p, matrix, 0);

  /* FFT of rows (dim==0) or cols (dim==1) of matrix. */

  WITH_HANDLING
  {

    p = (MATRIX *) dense_matrix ((MATRIX *) EAT (p));
    p = (MATRIX *) dup_matrix (EAT (p));
    p->symmetry = general;

    switch (p->type)
      {
      case integer:
	p = (MATRIX *) cast_matrix_integer_real (p);	/* fall through */

      case real:

	/* The RFFTF routine gives kind of screwy results (sine and */
	/* cosine coefficients) which can't be used for convolution */
	/* in the usual way.  Let's just convert to complex instead. */

	p = (MATRIX *) cast_matrix_real_complex ((MATRIX *) EAT (p));

	nn = dim ? p->nr : p->nc;
	if (nn == 0)
	  goto out;		/* done if zero length */

	tmp = (REAL *) CALLOC (4 * nn + 15, sizeof (REAL));

	/* CFFTF has no stride option, so must transpose if doing rows. */

	if (!dim)
	  p = (MATRIX *) transpose_matrix ((MATRIX *) EAT (p));

	CFFTI (&nn, tmp);

	j = 2 * ((nn - 1) / 2);
	k = (nn % 2) ? 2 : 4;

	for (m=0; m<2*p->nc; m+=2)
	  {
	    CFFTF (&nn, p->a.real+m*nn, tmp);

	    /* Reorder for increasing frequency from -1/2t to +1/2t */

	    for (i = j; i < j + k; i++)
	      s[i - j] = p->a.real[i+m*nn];
	    for (i = 0; i < j; i++)
	      {
		p->a.real[(2+m) * nn - j + i - k] = p->a.real[i+m*nn];
		p->a.real[i+m*nn] = p->a.real[(2+m) * nn - j + i];
	      }
	    for (i = 2 * nn - k; i < 2 * nn; i++)
	      p->a.real[i+m*nn] = s[i - 2 * nn + k];
	  }

	FREE (tmp);

	if (!dim)
	  p = (MATRIX *) transpose_matrix ((MATRIX *) EAT (p));

	break;

      case complex:

	nn = dim ? p->nr : p->nc;
	if (nn == 0)
	  goto out;		/* done if zero length */

	tmp = (REAL *) CALLOC (4 * nn + 15, sizeof (REAL));

	/* CFFTF has no stride option, so must transpose if doing rows. */

	if (!dim)
	  p = (MATRIX *) transpose_matrix ((MATRIX *) EAT (p));

	CFFTI (&nn, tmp);

	j = 2 * ((nn - 1) / 2);
	k = (nn % 2) ? 2 : 4;

	for (m=0; m<2*p->nc; m+=2)
	  {
	    CFFTF (&nn, p->a.real+m*nn, tmp);

	    /* Reorder for increasing frequency from -1/2t to +1/2t */

	    for (i = j; i < j + k; i++)
	      s[i - j] = p->a.real[i+m*nn];
	    for (i = 0; i < j; i++)
	      {
		p->a.real[(2+m) * nn - j + i - k] = p->a.real[i+m*nn];
		p->a.real[i+m*nn] = p->a.real[(2+m) * nn - j + i];
	      }
	    for (i = 2 * nn - k; i < 2 * nn; i++)
	      p->a.real[i+m*nn] = s[i - 2 * nn + k];
	  }

	FREE (tmp);

	if (!dim)
	  p = (MATRIX *) transpose_matrix ((MATRIX *) EAT (p));

	break;

      default:
	fail ("Can't apply \"fft\" to a %s matrix.", type_string[p->type]);
	raise_exception ();
      }

    if (dim)
      {
	if (p->rid)
	  p->rid = modify_fft_labels ((VECTOR *) EAT (p->rid));
      }
    else
      {
	if (p->cid)
	  p->cid = modify_fft_labels ((VECTOR *) EAT (p->cid));
      }

  }

out:				/* can't jump past the ON_EXCEPTION */

  ON_EXCEPTION
  {
    delete_matrix (p);
  }
  END_EXCEPTION;

  return ENT (p);
}
#endif

#define SAMPLE_WARN 1.0e-6
#define SAMPLE_ERR 1.0e-3

static ENTITY *
modify_fft_labels (VECTOR *p)
{
  /*
   * The given vector contains abscissas corresponding to the ordinates
   * in another vector that is to be transformed with the FFT.  The result
   * of this routine is a vector of frequency-domain abscissas.
   */

  REAL dt, dif, t, d, f, df;
  int n, i;

  switch (p->type)
    {
    case integer:
      p = (VECTOR *) cast_vector_integer_real (p);
      break;
    case real:
      break;
    case complex:
    case character:
      delete_vector (p);
      return (NULL);
    default:
      BAD_TYPE (p->type);
      delete_vector (p);
      raise_exception ();
    }
  assert (p->type == real);

  p = (VECTOR *) dup_vector ((VECTOR *) dense_vector ((VECTOR *) EAT (p)));

  WITH_HANDLING
  {
    n = p->nn;
    if (n > 1)
      {
	t = p->a.real[n - 1] - p->a.real[0];
	dt = t / (REAL) (n - 1);

	/* Go through the samples to make sure that they are evenly spaced. */

	dif = 0;
	for (i = 1; i < n; i++)
	  {
	    d = fabs (p->a.real[i] - p->a.real[i - 1] - dt);
	    if (d > dif)
	      dif = d;
	  }
	dif /= dt;
	if (dif > SAMPLE_ERR)
	  {
	    fail ("Unevenly spaced samples in \"fft\".");
	    raise_exception ();
	  }
	else if (dif > SAMPLE_WARN)
	  {
	    warn ("Unevenly spaced samples in \"fft\".");
	  }

	/* Now compute the new "frequency" labels. */

	f = 4.0 * atan (1.0) / dt;
	df = 2.0 * f / (REAL) n;
	for (i = 0; i < n; i++)
	  p->a.real[n - i - 1] = df * ((n / 2) - i);
      }
    else
      if (n == 1) p->a.real[0] = 0.0;
  }
  ON_EXCEPTION
  {
    delete_vector (p);
  }
  END_EXCEPTION;

  return (ENT (p));
}
