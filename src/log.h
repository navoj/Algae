/*
   log.h -- Prototypes for `log.c'.

   Copyright (C) 1994-2001  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: log.h,v 1.3 2001/08/09 03:19:29 ksh Exp $ */

#ifndef LOG_H
#define LOG_H	1

#include "entity.h"

#if LAX_LOG
extern double PROTO (xlog, (double x));
#else
#define xlog log
#endif

extern COMPLEX PROTO (log_complex, (COMPLEX z));
extern ENTITY *PROTO (bi_log, (ENTITY * p));
extern ENTITY *PROTO (log_scalar, (SCALAR * p));
extern ENTITY *PROTO (log_vector, (VECTOR * p));
extern ENTITY *PROTO (log_matrix, (MATRIX * p));

#endif /* LOG_H */
