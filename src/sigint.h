/*
   sigint.h

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: sigint.h,v 1.1.1.1 1996/04/17 05:56:22 ksh Exp $ */

#ifndef  SIGINT_H
#define  SIGINT_H	1

#include <signal.h>

extern int sigint_flag;
extern int sigint_raise_exception_flag;

RETSIGTYPE PROTO (catch_sigint, (int));

#endif
