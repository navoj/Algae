/*
   get.h -- Prototypes for `get.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: get.h,v 1.1.1.1 1996/04/17 05:56:18 ksh Exp $ */

#ifndef GET_H
#define GET_H	1

#include "entity.h"

#if BINARY_FORMAT == IEEE_LITTLE_ENDIAN

#define READ_INTS( ptr, n, s ) \
  ( errno = 0, \
    ( fread_int_reverse( (char *) (ptr), n, s ) < (n) ) ? \
         ( READ_WARN( s ), 0 ) : 1 )

#define READ_DOUBLES( ptr, n, s ) \
  ( errno = 0, \
    ( fread_double_reverse( (char *) (ptr), n, s ) < (n) ) ? \
         ( READ_WARN( s ), 0 ) : 1 )

#elif BINARY_FORMAT == CRAY_FLOAT

#define READ_INTS( ptr, n, s ) \
  ( errno = 0, \
    ( fread_int_cray( (char *) (ptr), n, s ) < (n) ) ? \
         ( READ_WARN( s ), 0 ) : 1 )

#define READ_DOUBLES( ptr, n, s ) \
  ( errno = 0, \
    ( fread_double_cray( (char *) (ptr), n, s ) < (n) ) ? \
         ( READ_WARN( s ), 0 ) : 1 )

extern size_t PROTO (fread_int_cray,
		       (char *ptr, size_t n, FILE * stream));
extern size_t PROTO (fread_double_cray,
		       (char *ptr, size_t n, FILE * stream));
extern size_t PROTO (fread_int_cray_reverse,
		       (char *ptr, size_t n, FILE * stream));
extern size_t PROTO (fread_double_cray_reverse,
		       (char *ptr, size_t n, FILE * stream));

#else

#define READ_INTS( ptr, n, s ) \
  ( errno = 0, \
    ( fread( ptr, 4, n, s ) < (n) ) ? \
         ( READ_WARN( s ), 0 ) : 1 )

#define READ_DOUBLES( ptr, n, s ) \
  ( errno = 0, \
    ( fread( ptr, 8, n, s ) < (n) ) ? \
         ( READ_WARN( s ), 0 ) : 1 )

#endif

#define READ_INT( ptr, s )	READ_INTS( ptr, 1, s )
#define READ_DOUBLE( ptr, s )	READ_DOUBLES( ptr, 1, s )

extern ENTITY *PROTO (bi_get, (int n, ENTITY * fname));
extern ENTITY *PROTO (get_entity, (FILE * stream));

extern size_t PROTO (fread_int,
		       (char *ptr, size_t n, FILE * stream));
extern size_t PROTO (fread_double,
		       (char *ptr, size_t n, FILE * stream));
extern size_t PROTO (fread_int_reverse,
		       (char *ptr, size_t n, FILE * stream));
extern size_t PROTO (fread_double_reverse,
		       (char *ptr, size_t n, FILE * stream));

#endif /* GET_H */
