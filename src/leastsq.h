/*
 * leastsq.h -- Prototypes for `leastsq.c'.
 *
 * Copyright (C) 1996  K. Scott Hunziker.
 *
 * See the file COPYING for license, warranty, and permission details.
 */

/* $Id: leastsq.h,v 1.1 1996/08/30 06:38:41 ksh Exp $ */

#ifndef LEASTSQ_H
#define LEASTSQ_H

#include "entity.h"

extern ENTITY *PROTO (bi_leastsq, (ENTITY *a, ENTITY *b));
extern ENTITY *PROTO (leastsq_matrix, (MATRIX *a, MATRIX *b));

#endif /* LEASTSQ_H */
