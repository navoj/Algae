
/*
   for_loop.h -- For loops.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: for_loop.h,v 1.1.1.1 1996/04/17 05:56:17 ksh Exp $ */

#ifndef FOR_LOOP_H
#define FOR_LOOP_H	1

#include "entity.h"
#include "ptypes.h"
#include "vector_st.h"

/* structure to save state of loop

   for( var in vector)
 */


typedef struct f_l
  {
    struct f_l *link;		/* for a linked stack */

    /* execute() uses these fields */
    DATUM *var_address;
    VECTOR *list;		/* vector goes here name is list for
				   historical reasons */

    /* code that walks vector communicates to execute() in
       these fields */
/*----------------------------*/
    TYPE type;			/* holds the current value of var  */
    VALUE value;		/* if type <= real, store in value */
    ENTITY *ep;			/* else stored in ep , type < 0 if done */
/*--------------------------*/
    /* state info goes here , used by walk functions */
    int last_elem;
  }
FOR_LOOP;


void PROTO (init_for_loop, (FOR_LOOP *));
int PROTO (inc_for_loop, (FOR_LOOP *));
void PROTO (cleanup_for_loop_stack, (FOR_LOOP *));

#define CLEANUP_FOR_LOOP_STACK() \
	if( for_loop_stack ) \
	   cleanup_for_loop_stack(for_loop_stack) ; else


/* notes:

   for loops are coded as follows:

   ifl
   relative jmp to fl
   --
   .
   body
   .
   --
   fl
   relative jmp to start of body
   kfl
   --

 */

#endif /* FOR_LOOP_H */
