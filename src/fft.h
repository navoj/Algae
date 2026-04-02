/*
   fft.h -- Prototypes for `fft.c'.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: fft.h,v 1.5 2003/09/21 00:25:35 ksh Exp $ */

#ifndef FFT_H
#define FFT_H

#include "entity.h"

#if HAVE_FFTW
#include <fftw3.h>
extern ENTITY *PROTO (bi_fftw, (int n, ENTITY * p, ENTITY *d));
extern ENTITY *PROTO (fftw_vector, (VECTOR * p, int rigor));
extern ENTITY *PROTO (fftw_matrix, (MATRIX * p, int dim, int rigor));
#endif

#if !HAVE_FFTW || USE_BOTH_FFT
extern void PROTO (CFFTI, (int *n, REAL * wsave));
extern void PROTO (CFFTF, (int *n, REAL * c, REAL * wsave));
extern ENTITY *PROTO (bi_fft, (int n, ENTITY * p, ENTITY *d));
extern ENTITY *PROTO (fft_vector, (VECTOR * p));
extern ENTITY *PROTO (fft_matrix, (MATRIX * p, int dim));
#endif

#endif /* FFT_H */
