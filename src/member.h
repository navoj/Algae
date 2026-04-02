/*
   member.h -- Prototypes for `member.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: member.h,v 1.1.1.1 1996/04/17 05:56:19 ksh Exp $ */

#ifndef MEMBER_H
#define MEMBER_H	1

#include "entity.h"

#define assign_member_table replace_in_table

extern ENTITY *PROTO (assign_member,
		      (ENTITY * head, ENTITY * new, char *name));
extern ENTITY *PROTO (assign_members,
		      (ENTITY * head, ENTITY * new, int cnt, char **memv));
extern ENTITY *PROTO (reference_member,
		      (ENTITY * head, char *name));
extern ENTITY *PROTO (reference_members,
		      (ENTITY * head, int cnt, char **memv));
extern ENTITY *PROTO (assign_member_scalar,
		      (SCALAR * head, ENTITY * new, char *name));
extern ENTITY *PROTO (assign_member_vector,
		      (VECTOR * head, ENTITY * new, char *name));
extern ENTITY *PROTO (assign_member_matrix,
		      (MATRIX * head, ENTITY * new, char *name));
extern ENTITY *PROTO (assign_member_function,
		      (FUNCTION * head, ENTITY * new, char *name));
extern ENTITY *PROTO (reference_member_scalar,
		      (SCALAR * head, char *name));
extern ENTITY *PROTO (reference_member_vector,
		      (VECTOR * head, char *name));
extern ENTITY *PROTO (reference_member_matrix,
		      (MATRIX * head, char *name));
extern ENTITY *PROTO (reference_member_function,
		      (FUNCTION * head, char *name));
extern ENTITY *PROTO (member_names,
		      (ENTITY * t));

#endif /* MEMBER_H */
