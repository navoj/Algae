
/*
   ifft.c -- Inverse Fourier Transform.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: ifft.c,v 1.7 2003/09/21 04:47:16 ksh Exp $";

#include "ifft.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "cast.h"
#include "dense.h"
#include "transpose.h"

static ENTITY *PROTO (modify_ifft_labels, (VECTOR * p));

static char *ifft_options[] =
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
    IFFT_OPT_ROW,
    IFFT_OPT_ESTIMATE,
    IFFT_OPT_MEASURE,
    IFFT_OPT_PATIENT,
    IFFT_OPT_EXHAUSTIVE,
    IFFT_OPT_FORGET,
  };

#if HAVE_FFTW

ENTITY *
bi_ifftw (int n, ENTITY *p, ENTITY *d)
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

  if (!OK_OPTIONS ((TABLE *) d, ifft_options)) raise_exception ();

  dim = !IN_TABLE (d, ifft_options[IFFT_OPT_ROW]);

  if (IN_TABLE (d, ifft_options[IFFT_OPT_FORGET]))
    fftw_forget_wisdom();

  rigor = FFTW_ESTIMATE;
  if (IN_TABLE (d, ifft_options[IFFT_OPT_MEASURE]))
    rigor = FFTW_MEASURE;
  if (IN_TABLE (d, ifft_options[IFFT_OPT_PATIENT]))
    rigor = FFTW_PATIENT;
  if (IN_TABLE (d, ifft_options[IFFT_OPT_EXHAUSTIVE]))
    rigor = FFTW_EXHAUSTIVE;

  WITH_HANDLING
    {
      switch (p->class)
	{
	case scalar:
	  s = ifftw_vector ((VECTOR *) scalar_to_vector ((SCALAR *) EAT(p)),
			    rigor);
	  break;

	case vector:
	  s = ifftw_vector ((VECTOR *) EAT(p), rigor);
	  break;

	case matrix:

	  /* IFFT of rows (dim=0); IFFT of cols (dim=1) */
	  s = ifftw_matrix ((MATRIX *) EAT(p), dim, rigor);
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
ifftw_vector (VECTOR *p, int rigor)
{
  COMPLEX *tmpc = NULL;
  REAL scale;
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
	p = (VECTOR *) cast_vector_real_complex (EAT (p));
        /* no break fall through */

      case complex:

	p = (VECTOR *) dup_vector (EAT (p));

	nn = p->nn;

	if (nn == 0)
	  goto out;		/* done if zero length */

	scale = 1.0 / (REAL) nn;

        /* Allocate space - transform done in place */
        tmpc = MALLOC (2*nn*sizeof(REAL));

        /* Create the fftw plan */
        plan = fftw_plan_dft_1d(nn, (fftw_complex *) tmpc,
				(fftw_complex *) tmpc, FFTW_BACKWARD, rigor);

        if (plan == NULL) {
	    fail ("FFTW failed to make a plan.");
	    raise_exception ();
        }

        /* copy with reorder */
        for (i=0,j=(nn-1)/2; j<nn; i++,j++) {
	  tmpc[i].real = p->a.complex[j].real;
          tmpc[i].imag = p->a.complex[j].imag;
        }
        for (i=(nn+2)/2,j=0; i<nn; i++,j++) {
	  tmpc[i].real = p->a.complex[j].real;
	  tmpc[i].imag = p->a.complex[j].imag;
        }

        /* Execute the transform */
        fftw_execute (plan);

        /* Copy and scale the data */
        for (i=0; i<nn; i++) {
            p->a.complex[i].real = scale * tmpc[i].real;
            p->a.complex[i].imag = scale * tmpc[i].imag;
        }

	break;

      default:
	fail ("Can't apply \"ifft\" to a %s vector.", type_string[p->type]);
	raise_exception ();
      }

    if (p->eid)
      p->eid = modify_ifft_labels ((VECTOR *) EAT (p->eid));

  }

out:				/* can't jump past the ON_EXCEPTION */

  ON_EXCEPTION
  {
    if (plan) fftw_destroy_plan (plan);
    TFREE (tmpc);
    delete_vector (p);
  }
  END_EXCEPTION;

  fftw_destroy_plan (plan);
  TFREE (tmpc);
  return ENT (p);
}

ENTITY *
ifftw_matrix (p, dim, rigor)
     MATRIX *p;
     int dim;  /* IFFT of rows (dim=0); IFFT of cols (dim=1) */
     int rigor;
{
  COMPLEX *tmpc = NULL;
  REAL scale;
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
        /* no break fall through */

      case complex:

	p = (MATRIX *) dup_matrix (EAT (p));

        if ( dim == 0 )   /* Take the IFFT of each row */
        {
          nr = p->nr;     /* Number of rows */
          nn = p->nc;     /* Number to columns (elements per row) */

	  if (nn == 0)
	    goto out;     /* Done if columns have zero length */

	  scale = 1.0 / (REAL) nn;

	  if (nr)
	    {
	      /* Allocate space - transform done in place */
	      tmpc = MALLOC (2 * nn * sizeof(REAL));

	      /* Create the fftw plan */
	      plan = fftw_plan_dft_1d (nn, (fftw_complex *) tmpc,
				       (fftw_complex *) tmpc, FFTW_BACKWARD,
				       rigor);

	      if (plan == NULL) {
		fail ("FFTW failed to make a plan.");
		raise_exception ();
	      }

	      for (l = 0; l < nr; l++)
		{
		  /* copy with reorder */
		  for (i=0,j=l+((nn-1)/2)*nr; i<=nn/2; i++,j+=nr) {
		    tmpc[i].real = p->a.complex[j].real;
		    tmpc[i].imag = p->a.complex[j].imag;
		  }
		  for (i=(nn+2)/2,j=l; i<nn; i++,j+=nr) {
		    tmpc[i].real = p->a.complex[j].real;
		    tmpc[i].imag = p->a.complex[j].imag;
		  }

		  /* Execute the transform */
		  fftw_execute(plan);

		  /* copy and scale */
		  for (i=0; i<nn; i++) {
		    p->a.complex[l+i*nr].real = scale * tmpc[i].real;
		    p->a.complex[l+i*nr].imag = scale * tmpc[i].imag;
		  }

		} /* l - done loop over each row */
	    }

          if (p->cid)
            p->cid = modify_ifft_labels ((VECTOR *) EAT (p->cid));
        }
        else              /* Take the IFFT of each column */
        {
          nc = p->nc;     /* Number of columns */
          nn = p->nr;     /* Number to rows (elements per column) */

	  if (nn == 0)
	    goto out;     /* Done if rows have zero length */

	  scale = 1.0 / (REAL) nn;

	  if (nc)
	    {
	      /* Allocate space - transform done in place */
	      tmpc = MALLOC (2 * nn * sizeof(REAL));

	      /* Create the fftw plan */
	      plan = fftw_plan_dft_1d (nn, (fftw_complex *) tmpc,
				       (fftw_complex *) tmpc, FFTW_BACKWARD,
				       rigor);

	      if (plan == NULL) {
		fail ("FFTW failed to make a plan.");
		raise_exception ();
	      }

	      for (l = 0; l < nc; l++)
		{
		  /* copy with reorder */
		  for (i=0,j=(nn-1)/2+l*nn; i<=nn/2; i++,j++) {
		    tmpc[i].real = p->a.complex[j].real;
		    tmpc[i].imag = p->a.complex[j].imag;
		  }
		  for (i=(nn+2)/2,j=l*nn; i<nn; i++,j++) {
		    tmpc[i].real = p->a.complex[j].real;
		    tmpc[i].imag = p->a.complex[j].imag;
		  }

		  /* Execute the transform */
		  fftw_execute(plan);

		  /* copy and scale */
		  for (i=0; i<nn; i++) {
		    p->a.complex[i+l*nn].real = scale * tmpc[i].real;
		    p->a.complex[i+l*nn].imag = scale * tmpc[i].imag;
		  }

		} /* l - done loop over each column */
	    }

          if (p->rid)
            p->rid = modify_ifft_labels ((VECTOR *) EAT (p->rid));

        }
        break;

      default:
	fail ("Can't apply \"ifft\" to a %s matrix.", type_string[p->type]);
	raise_exception ();
      }
  }

out:				/* can't jump past the ON_EXCEPTION */

  ON_EXCEPTION
  {
    if (plan) fftw_destroy_plan (plan);
    TFREE (tmpc);
    delete_matrix (p);
  }
  END_EXCEPTION;

  fftw_destroy_plan (plan);
  TFREE (tmpc);
  return ENT (p);
}
#endif

#if !HAVE_FFTW || USE_BOTH_FFT

ENTITY *
bi_ifft (int n, ENTITY *p, ENTITY *d)
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

  if (!OK_OPTIONS ((TABLE *) d, ifft_options)) raise_exception ();

  dim = !IN_TABLE (d, ifft_options[IFFT_OPT_ROW]);

  WITH_HANDLING
    {
      switch (p->class)
	{
	case scalar:
	  s = ifft_vector ((VECTOR *) scalar_to_vector ((SCALAR *) EAT(p)));
	  break;

	case vector:
	  s = ifft_vector ((VECTOR *) EAT(p));
	  break;

	case matrix:

	  /* IFFT of rows (dim=0); IFFT of cols (dim=1) */
	  s = ifft_matrix ((MATRIX *) EAT(p), dim);
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
ifft_vector (VECTOR *p)
{
  REAL *tmp;
  REAL r, s[4];
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
	p = (VECTOR *) cast_vector_real_complex ((VECTOR *) EAT (p));

	nn = p->nn;
	if (nn == 0)
	  goto out;

	/*
	 * The input is assumed to have data in increasing frequency
	 * from -1/2t to +1/2t.  The frequency spacing of the data
	 * (df) must be constant.  If there are nn points of data,
	 * then the first point is at -dt*((nn-1)/2) and the last
	 * is at dt*(nn/2).  Here we have to reorder it so that it
	 * begins at zero frequency (like CFFTB likes it).
	 */

	j = 2 * ((nn - 1) / 2);
	k = (nn % 2) ? 2 : 4;
	for (i = 2 * nn - k; i < 2 * nn; i++)
	  s[i - 2 * nn + k] = p->a.real[i];
	for (i = j - 1; i >= 0; i--)
	  {
	    p->a.real[2 * nn - j + i] = p->a.real[i];
	    p->a.real[i] = p->a.real[2 * nn - j + i - k];
	  }
	for (i = j; i < j + k; i++)
	  p->a.real[i] = s[i - j];

	tmp = (REAL *) CALLOC (4 * nn + 15, sizeof (REAL));
	CFFTI (&nn, tmp);
	CFFTB (&nn, p->a.real, tmp);
	FREE (tmp);
	r = 1.0 / (REAL) nn;
	for (i = 0; i < 2 * nn; i++)
	  p->a.real[i] *= r;
	break;

      case complex:

	nn = p->nn;
	if (nn == 0)
	  goto out;

	/* The input is assumed to have data in increasing frequency */
	/* from -1/2t to +1/2t.  The frequency spacing of the data */
	/* (df) must be constant.  If there are nn points of data, */
	/* then the first point is at -dt*((nn-1)/2) and the last */
	/* is at dt*(nn/2).  Here we have to reorder it so that it */
	/* begins at zero frequency (like CFFTB likes it). */

	j = 2 * ((nn - 1) / 2);
	k = (nn % 2) ? 2 : 4;
	for (i = 2 * nn - k; i < 2 * nn; i++)
	  s[i - 2 * nn + k] = p->a.real[i];
	for (i = j - 1; i >= 0; i--)
	  {
	    p->a.real[2 * nn - j + i] = p->a.real[i];
	    p->a.real[i] = p->a.real[2 * nn - j + i - k];
	  }
	for (i = j; i < j + k; i++)
	  p->a.real[i] = s[i - j];

	tmp = (REAL *) CALLOC (4 * nn + 15, sizeof (REAL));
	CFFTI (&nn, tmp);
	CFFTB (&nn, p->a.real, tmp);
	FREE (tmp);

	r = 1.0 / (REAL) nn;
	for (i = 0; i < 2 * nn; i++)
	  p->a.real[i] *= r;
	break;

      default:
	fail ("Can't apply \"ifft\" to a %s matrix.",
	      type_string[p->type]);
	raise_exception ();
      }

    if (p->eid != NULL)
      p->eid = modify_ifft_labels ((VECTOR *) EAT (p->eid));
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
ifft_matrix (MATRIX *p, int dim)
{
  REAL *tmp;
  REAL r, s[4];
  int i, j, k, m, nn;

  EASSERT (p, matrix, 0);

  /* IFFT of rows (dim==0) or cols (dim==1) of matrix. */

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
	p = (MATRIX *) cast_matrix_real_complex ((MATRIX *) EAT (p));

	nn = dim ? p->nr : p->nc;
	if (nn == 0)
	  goto out;

	/*
	 * The input is assumed to have data in increasing frequency
	 * from -1/2t to +1/2t.  The frequency spacing of the data
	 * (df) must be constant.  If there are nn points of data,
	 * then the first point is at -dt*((nn-1)/2) and the last
	 * is at dt*(nn/2).  Here we have to reorder it so that it
	 * begins at zero frequency (like CFFTB likes it).
	 */

	tmp = (REAL *) CALLOC (4 * nn + 15, sizeof (REAL));

	/* CFFTB has no stride option, so must transpose if doing rows. */

	if (!dim)
	  p = (MATRIX *) transpose_matrix ((MATRIX *) EAT (p));

	CFFTI (&nn, tmp);

	j = 2 * ((nn - 1) / 2);
	k = (nn % 2) ? 2 : 4;

	for (m=0; m<2*p->nc; m+=2)
	  {
	    for (i = 2 * nn - k; i < 2 * nn; i++)
	      s[i - 2 * nn + k] = p->a.real[i+m*nn];
	    for (i = j - 1; i >= 0; i--)
	      {
		p->a.real[(2+m) * nn - j + i] = p->a.real[i+m*nn];
		p->a.real[i+m*nn] = p->a.real[(2+m) * nn - j + i - k];
	      }
	    for (i = j; i < j + k; i++)
	      p->a.real[i+m*nn] = s[i - j];

	    CFFTB (&nn, p->a.real+m*nn, tmp);
	  }

	FREE (tmp);

	if (!dim)
	  p = (MATRIX *) transpose_matrix ((MATRIX *) EAT (p));

	r = 1.0 / (REAL) nn;
	for (i = 0; i < 2 * p->nn; i++)
	  p->a.real[i] *= r;

	break;

      case complex:

	nn = dim ? p->nr : p->nc;
	if (nn == 0)
	  goto out;

	/* The input is assumed to have data in increasing frequency */
	/* from -1/2t to +1/2t.  The frequency spacing of the data */
	/* (df) must be constant.  If there are nn points of data, */
	/* then the first point is at -dt*((nn-1)/2) and the last */
	/* is at dt*(nn/2).  Here we have to reorder it so that it */
	/* begins at zero frequency (like CFFTB likes it). */

	tmp = (REAL *) CALLOC (4 * nn + 15, sizeof (REAL));

	/* CFFTB has no stride option, so must transpose if doing rows. */

	if (!dim)
	  p = (MATRIX *) transpose_matrix ((MATRIX *) EAT (p));

	CFFTI (&nn, tmp);

	j = 2 * ((nn - 1) / 2);
	k = (nn % 2) ? 2 : 4;

	for (m=0; m<2*p->nc; m+=2)
	  {
	    for (i = 2 * nn - k; i < 2 * nn; i++)
	      s[i - 2 * nn + k] = p->a.real[i+m*nn];
	    for (i = j - 1; i >= 0; i--)
	      {
		p->a.real[(2+m) * nn - j + i] = p->a.real[i+m*nn];
		p->a.real[i+m*nn] = p->a.real[(2+m) * nn - j + i - k];
	      }
	    for (i = j; i < j + k; i++)
	      p->a.real[i+m*nn] = s[i - j];

	    CFFTB (&nn, p->a.real+m*nn, tmp);
	  }

	FREE (tmp);

	if (!dim)
	  p = (MATRIX *) transpose_matrix ((MATRIX *) EAT (p));

	r = 1.0 / (REAL) nn;
	for (i = 0; i < 2 * p->nn; i++)
	  p->a.real[i] *= r;
	break;

      default:
	fail ("Can't apply \"ifft\" to a %s matrix.", type_string[p->type]);
	raise_exception ();
      }

    if (dim)
      {
	if (p->rid)
	  p->rid = modify_ifft_labels ((VECTOR *) EAT (p->rid));
      }
    else
      {
	if (p->cid)
	  p->cid = modify_ifft_labels ((VECTOR *) EAT (p->cid));
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
modify_ifft_labels (VECTOR *p)
{
  /*
   * The vector in `p' contains frequency-domain abscissas for a vector
   * that is to be transformed with the inverse FFT.  The return value
   * is a vector of the corresponding temporal or spatial coordinates.
   */

  REAL dt, dif, d, f, df;
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
	f = p->a.real[n - 1] - p->a.real[0];
	df = f / (REAL) (n - 1);

	/* Go through the samples to make sure that they are evenly spaced. */

	dif = 0;
	for (i = 1; i < n; i++)
	  {
	    d = fabs (p->a.real[i] - p->a.real[i - 1] - df);
	    if (d > dif)
	      dif = d;
	  }
	dif /= df;
	if (dif > SAMPLE_ERR)
	  {
	    fail ("Unevenly spaced samples in \"ifft\".");
	    raise_exception ();
	  }
	else if (dif > SAMPLE_WARN)
	  {
	    warn ("Unevenly spaced samples in \"ifft\".");
	  }

	/* Now compute the new "time" labels. */

	dt = 8.0 * atan (1.0) / (n * df);
	for (i = 0; i < n; i++)
	  p->a.real[i] = dt * i;
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
