/*
   exception.h -- Exception handling.

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: exception.h,v 1.3 1997/02/21 09:53:55 ksh Exp $ */

#ifndef  EXCEPTION_H
#define  EXCEPTION_H	1

#include <setjmp.h>
#include <signal.h>
#include "algae.h"
#include "sigint.h"

typedef struct except
  {
    struct except *link;
    jmp_buf jb;
    struct except *check;
  }
EXCEPTION;

extern EXCEPTION *exception_head;



#define  WITH_HANDLING  { EXCEPTION  my ;\
		if ( setjmp(my.jb) == 0 ) { exception_push(&my) ;


#define  ON_EXCEPTION   (void) exception_pop() ; } else {
#define  EXCEPTION_BASE	ON_EXCEPTION

#define  END_EXCEPTION   raise_exception(); } }

#define  END_EXCEPTION_BASE  ; }}


void PROTO (exception_push, (EXCEPTION *));
jmp_buf *PROTO (exception_pop, (void));
void PROTO (raise_exception, (void));
ENTITY *PROTO (bi_exception, (void));


/* handling of SIGINT  */


#define POLL_SIGINT()   if( sigint_flag ) raise_exception() ; else

#define SIGINT_RAISE_ON()  do {\
	(void) signal(SIGINT, SIG_IGN) ;\
	POLL_SIGINT() ;\
	sigint_raise_exception_flag = 1 ;\
	(void) signal(SIGINT, catch_sigint) ; } while(0)

#define SIGINT_RAISE_OFF()  (sigint_raise_exception_flag = 0)
#endif /* EXCEPTION_H */
