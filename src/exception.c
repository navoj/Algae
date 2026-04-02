/*
   exception.c -- Exception handling.

   Copyright (C) 1994-97  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: exception.c,v 1.2 1997/02/21 09:53:55 ksh Exp $";

#include "algae.h"
#include "exception.h"

/*
   exceptions are handled with a stack(linked list) of
   long jumps.

   exception_head points at the top of the stack
 */

EXCEPTION *exception_head;


void
exception_push (EXCEPTION *e)
{				/* add a new exception handler to the stack ,
				   struct EXCEPTION is in the caller's stack */
  e->check = e;			/* used to test list is not corrupted */
  e->link = exception_head;
  exception_head = e;
}

jmp_buf *
exception_pop (void)
{
  jmp_buf *retval;

  assert (exception_head != (EXCEPTION *) 0);
  assert (exception_head->check = exception_head);
  retval = (jmp_buf *) exception_head->jb;
  exception_head = exception_head->link;
  return retval;
}


void
raise_exception (void)
{
  assert (exception_head);
  longjmp (*exception_pop (), 1);
}


ENTITY *
bi_exception (void)
{
  raise_exception ();
}
