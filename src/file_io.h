/*
   file_io.h -- File handling.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: file_io.h,v 1.1.1.1 1996/04/17 05:56:17 ksh Exp $ */

#ifndef FILE_IO_H
#define FILE_IO_H	1

#include "entity.h"

#define  FILE_INPUT	0
#define  FILE_OUTPUT	1

#define  PIPE_FLAG	2
#define  PIPE_OUT	(PIPE_FLAG|FILE_OUTPUT)
#define  PIPE_IN	(PIPE_FLAG|FILE_INPUT)
#define  IO_MASK	1

extern char *read_fail_msg, *write_fail_msg, *eof_msg;

#define READ_FAIL( fp ) \
   do { fail( read_fail_msg, feof(fp) ? eof_msg : strerror(errno) ); \
	clearerr( fp ); } while (0)
#define READ_WARN( fp ) \
  ( warn( read_fail_msg, feof(fp) ? eof_msg : strerror(errno) ), \
    clearerr( fp ) )

#define WRITE_FAIL( fp ) \
  do { fail( write_fail_msg, strerror(errno) ); clearerr (fp); } while (0)
#define WRITE_WARN( fp ) \
  ( warn( write_fail_msg, strerror(errno) ), clearerr (fp) )

void PROTO (set_stderr, (void));
FILE *PROTO (find_file, (char *name, int mode));
ENTITY *PROTO (bi_close, (ENTITY * fname));
ENTITY *PROTO (bi_source, (ENTITY * fname));
FILE *PROTO (efopen, (char *name, char *mode));
int PROTO (close_file, (char *name));

#endif /* FILE_IO_H */
