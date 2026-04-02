/*
   dynio.h -- Prototypes for `dynio.f'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
*/

/* $Id: dynio.h,v 1.1.1.1 1996/04/17 05:56:17 ksh Exp $ */

#ifndef DYNIO_H
#define DYNIO_H	1

#include "entity.h"

#if BINARY_FORMAT == CRAY_FLOAT

/*
 * Control word structures for blocked files.  See the blocked(5) man page.
 * We shouldn't have to define these here, since they're given in the
 * cosio.h file.  Unfortunately, for a reason that escapes me, you have
 * to know somebody to have access to the /usr/src/lib directory where
 * it resides.
 */

typedef struct {		/*	-- Block Control Word --	*/
    unsigned int m	:  4 ;	/* 0					*/
    unsigned int u1	:  7 ;	/* unused				*/
    unsigned int bdf	:  1 ;	/* bad data flag (currently unused)	*/
    unsigned int u2	: 19 ;	/* unused				*/
    unsigned int bn	: 24 ;	/* block number (modulo 2^24)		*/
    unsigned int fwi	:  9 ;	/* forward index			*/
} BCW;

typedef struct {		/*	-- Record Control Word --	*/
    unsigned int m	:  4 ;	/* { EOR=8, EOF=14, EOD=15 }		*/
    unsigned int ubc	:  6 ;	/* unused bit count			*/
    unsigned int tran	:  1 ;	/* transparent record field (unused)	*/
    unsigned int bdf	:  1 ;	/* bad data flag (unused)		*/
    unsigned int srs	:  1 ;	/* skip remainder of sector (unused)	*/
    unsigned int u1	:  7 ;	/* unused				*/
    unsigned int pf	: 20 ;	/* previous file index (unused)		*/
    unsigned int pri	: 15 ;	/* previous record index		*/
    unsigned int fwi	:  9 ;	/* forward index			*/
} RCW;

typedef union { BCW bcw; RCW rcw; } CW;

#endif	/* CRAY_FLOAT */

extern ENTITY *PROTO( bi_getdyn, ( int n, ENTITY *fp ) );
extern ENTITY *PROTO( bi_putdyn, ( int n, ENTITY *t, ENTITY *fname ) );

#endif				/* DYNIO_H */
