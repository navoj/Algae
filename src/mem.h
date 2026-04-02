/*
   mem.h -- memory management

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: mem.h,v 1.7 2003/12/10 04:38:37 ksh Exp $ */

#ifndef MEM_H
#define MEM_H	1

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef ardent	/* These are necessary for the poor brain-damaged Stardent. */
#include <unistd.h>
#include <sys/types.h>
#if HAVE_MALLOC_H
#include <malloc.h>
#endif
char *PROTO (calloc, ());
char *PROTO (getenv, (char *));
#endif

#if !defined(VAXC) && !defined(apollo) && !defined(__APPLE__)
#if HAVE_MALLOC_H
#include <malloc.h>
#endif
#endif

/* Use Doug Lea's dlmalloc package, if possible. */

#if USE_DLMALLOC

#define MALLOC( size )		malloc (size)
#define CALLOC( nobj, size )	calloc (nobj, size)
#define REALLOC( p, size )	realloc (p, size)
#define FREE( p )		free (p)

#define MALLOC_FAILURE_ACTION \
  fail ("Out of memory."); raise_exception ()

#if HAVE_MALLOC_H
#define HAVE_USR_INCLUDE_MALLOC_H 1
#endif

#else

/* Bypass our malloc wrappers if using dmalloc package. */

#if DMALLOC

#define MALLOC( size )		malloc (size)
#define CALLOC( nobj, size )	calloc (nobj, size)
#define REALLOC( p, size )	realloc (p, size)
#define FREE( p )		free (p)

#else

#define MALLOC( size )		algae_malloc (size)
#define CALLOC( nobj, size )	algae_calloc (nobj, size)
#define REALLOC( p, size )	algae_realloc (p, size)
#define FREE( p )		free (p)

extern void *PROTO (algae_malloc, (size_t size));
extern void *PROTO (algae_calloc, (size_t nobj, size_t size));
extern void *PROTO (algae_realloc, (void *p, size_t size));

#endif
#endif

#endif /* MEM_H */
