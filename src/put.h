/*
   put.h -- Prototypes for `put.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: put.h,v 1.1.1.1 1996/04/17 05:56:21 ksh Exp $ */

#ifndef PUT_H
#define PUT_H	1

#include "entity.h"

/* We write a magic number and version with every entity. */

#define OLD_MAGIC_NUMBER	06373
#define FILE_FORMAT		1

#if BINARY_FORMAT == IEEE_LITTLE_ENDIAN

#define WRITE_INTS( ptr, n, s ) \
  ( errno = 0, \
    ( fwrite_int_reverse( (char *) (ptr), n, s ) < (n) ) ? \
         ( WRITE_WARN( s ), 0 ) : 1 )

#define WRITE_DOUBLES( ptr, n, s ) \
  ( errno = 0, \
    ( fwrite_double_reverse( (char *) (ptr), n, s ) < (n) ) ? \
         ( WRITE_WARN( s ), 0 ) : 1 )

#elif BINARY_FORMAT == CRAY_FLOAT

#define WRITE_INTS( ptr, n, s ) \
  ( errno = 0, \
    ( fwrite_int_cray( (char *) (ptr), n, s ) < (n) ) ? \
         ( WRITE_WARN( s ), 0 ) : 1 )

#define WRITE_DOUBLES( ptr, n, s ) \
  ( errno = 0, \
    ( fwrite_double_cray( (char *) (ptr), n, s ) < (n) ) ? \
         ( WRITE_WARN( s ), 0 ) : 1 )

extern size_t PROTO (fwrite_int_cray,
		       (char *ptr, size_t n, FILE * stream));
extern size_t PROTO (fwrite_double_cray,
		       (char *ptr, size_t n, FILE * stream));

#else

#define WRITE_INTS( ptr, n, s ) \
  ( errno = 0, \
    ( fwrite( ptr, 4, n, s ) < (n) ) ? \
         ( WRITE_WARN( s ), 0 ) : 1 )

#define WRITE_DOUBLES( ptr, n, s ) \
  ( errno = 0, \
    ( fwrite( ptr, 8, n, s ) < (n) ) ? \
         ( WRITE_WARN( s ), 0 ) : 1 )

#endif

#define WRITE_INT( ptr, s )	WRITE_INTS( ptr, 1, s )
#define WRITE_DOUBLE( ptr, s )	WRITE_DOUBLES( ptr, 1, s )

struct ent_node
  {
    ENTITY *entity;
    int seq;
    struct ent_node *left;
    struct ent_node *right;
  };

extern ENTITY *PROTO (bi_put, (int n, ENTITY * p, ENTITY * fname));
extern int PROTO (put_entity, (ENTITY * p, FILE * stream, struct ent_node *));

extern size_t PROTO (fwrite_int_reverse,
		       (char *ptr, size_t n, FILE * stream));
extern size_t PROTO (fwrite_double_reverse,
		       (char *ptr, size_t n, FILE * stream));

#endif /* PUT_H */
