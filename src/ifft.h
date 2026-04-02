/*
   ifft.h -- Prototypes for `ifft.c'.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: ifft.h,v 1.5 2003/09/21 00:25:35 ksh Exp $ */

#ifndef IFFT_H
#define IFFT_H

#include "entity.h"

#if HAVE_FFTW
#include <fftw3.h>
extern ENTITY *PROTO (bi_ifftw, (int n, ENTITY * p, ENTITY *d));
extern ENTITY *PROTO (ifftw_vector, (VECTOR * p, int rigor));
extern ENTITY *PROTO (ifftw_matrix, (MATRIX * p, int dim, int rigor));
#endif

#if !HAVE_FFTW || USE_BOTH_FFT
extern void PROTO (CFFTI, (int *n, double *wsave));
extern void PROTO (CFFTB, (int *n, double *c, double *wsave));
extern ENTITY *PROTO (bi_ifft, (int n, ENTITY * p, ENTITY *d));
extern ENTITY *PROTO (ifft_vector, (VECTOR * p));
extern ENTITY *PROTO (ifft_matrix, (MATRIX * p, int dim));
#endif

#endif /* IFFT_H */
