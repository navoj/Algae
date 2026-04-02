/*
   algae.h -- Visible variables and function declarations.

   Copyright (C) 1994-2001  K. Scott Hunziker.
   Copyright (C) 1994  The Boeing Company.

   This file is part of Algae.

   Algae is free software.  You can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Algae is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Algae; see the file LICENSE.  If not, write to the Free
   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   The copyright to major portions of Algae belongs to The Boeing
   Company.  The following permission notice and warranty disclaimer
   pertain to those portions of the code:

   Permission to use, copy, modify, and distribute this software
   and its documentation for any purpose is hereby granted,
   provided that the above copyright notice appear in all copies,
   that both the copyright notice and this permission notice and
   warranty disclaimer appear in supporting documentation, and that
   the names of Boeing or any of its entities not be used in
   advertising or publicity pertaining to distribution of the
   software without specific, written, prior permission.

   BOEING DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
   INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS,
   AND NONINFRINGEMENT.  IN NO EVENT SHALL BOEING BE LIABLE FOR ANY
   SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY
   DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA, OR PROFITS,
   WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE, OR OTHER TORTIOUS
   ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.
 */

/* $Id: algae.h,v 1.4 2002/04/24 19:14:01 ksh Exp $ */

#ifndef  ALGAE_H
#define  ALGAE_H	1

#if HAVE_CONFIG_H
#include <config.h>
#endif

/* Needed on AIX. */

#if HAVE_STANDARDS_H
#include <standards.h>
#endif

#if STDC_HEADERS
#include <string.h>
#else
#if !HAVE_STRCHR
#define strchr index
#define strrchr rindex
#endif
char *strchr (), *strrchr (), *strtok (), *strcpy (), *strcat ();
#if !HAVE_MEMCPY
#define memcpy(d, s, n) bcopy ((s), (d), (n))
#define memmove(d, s, n) bcopy ((s), (d), (n))
#endif
#endif

#if HAVE_UNISTD_H
#include <sys/types.h>
#include <unistd.h>
#endif

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if !DEBUG
#define  NDEBUG			/* for assert() */
#endif

#if NO_PROTOS
#define  PROTO(name,args)	name()
#else
#define  PROTO(name,args)	name args
#endif

#define IEEE_LITTLE_ENDIAN	0
#define IEEE_BIG_ENDIAN		1
#define VAX_D_FLOAT		2	/* not implemented */
#define VAX_G_FLOAT		3	/* not implemented */
#define CRAY_FLOAT		4

#ifndef BINARY_FORMAT
#if defined(CRAY) || defined(CRAY2)
#define BINARY_FORMAT	CRAY_FLOAT
#else
#define BINARY_FORMAT	WORDS_BIGENDIAN
#endif
#endif

#include <stdio.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#else
double atof ();
double strtod ();
#endif

#if HAVE_MEMORY_H
#include <memory.h>
#endif

#if !HAVE_STRERROR
extern int sys_nerr;
extern char *sys_errlist[];
#define strerror(err)	( ( err < sys_nerr && err > 0 ) ? \
                          sys_errlist[err] : "unknown error" )
#endif

#include <limits.h>

#if HAVE_FLOAT_H
/*
 * For some reason, these floating-point constants are often included
 * in <limits.h>.  That's wrong!
 */
#undef FLT_MAX
#undef FLT_MIN
#undef DBL_MAX
#undef DBL_MIN

#include <float.h>
#endif

#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>

#include "mem.h"
#include "fortran.h"
#include "message.h"


typedef double REAL;

typedef struct
  {
    REAL real;
    REAL imag;
  }
COMPLEX;

typedef struct datum DATUM;
typedef struct scalar SCALAR;
typedef struct table TABLE;
typedef struct vector VECTOR;
typedef struct matrix MATRIX;
typedef struct function FUNCTION;
typedef struct entity ENTITY;

#include "exception.h"

#define E_MALLOC( n, t )	((t == character) ? \
                                 CALLOC (n, type_size[t]) : \
                                 (((double) n * type_size[t] > INT_MAX) && \
                                  (fail ("Out of memory."), \
                                   raise_exception (), type_size), \
                                  MALLOC ((n)*type_size[t])))
#define E_CALLOC( n, t )	CALLOC( n, type_size[t] )

#if HAVE_MATHERR
#define CHECK_MATH()
#else
#define CHECK_MATH()	if (errno) {math_error ();} else;
#endif

#define EASSERT( p, c, t ) \
    { assert( p != NULL ); \
      assert( ((ENTITY *)p)->class < NUM_CLASS-1 ); \
      assert( debug_level < 1 || ok_entity( (ENTITY *) p ) ); \
      assert( !(c) || ( ((ENTITY *)p)->class == c ) ); \
      assert( !(t) || ( ( ((ENTITY *)p)->class == scalar ) ? \
		              ((SCALAR *)p)->type == t : \
		              ( ((ENTITY *)p)->class == vector ) ? \
		                  ((VECTOR *)p)->type == t : \
		                  ( ((ENTITY *)p)->class == matrix ) ? \
		                      ((MATRIX *)p)->type == t : 0 ) ); }

extern char NULL_string[];
extern int debug_level;

#define PRINTF_BUF_SIZE 2048
extern char printf_buf[];

#ifdef NEED_FPRINTF_DECLARATOR
extern int PROTO (fprintf, (FILE * stream, char *format,...));
#endif

#if DMALLOC
#include <dmalloc.h>
#endif

extern int PROTO (algae_atexit, (void (*fcn) ()));
extern void PROTO (algae_exit, (int status));

#endif /* ALGAE_H  */
