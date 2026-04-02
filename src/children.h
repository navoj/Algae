/*
 * children.h -- child processes
 *
 * Copyright (C) 1994-95  K. Scott Hunziker.
 * Copyright (C) 1990-94  The Boeing Company.
 *
 * See the file COPYING for license, warranty, and permission details.
 */

/* $Id: children.h,v 1.1.1.1 1996/04/17 05:56:16 ksh Exp $ */


#ifndef   CHILDREN_H
#define   CHILDREN_H

FILE *PROTO (get_pipe, (char *, int, int *));
int PROTO (wait_for, (int));
ENTITY *PROTO (bi_system, (ENTITY *));
ENTITY *PROTO (bi_cd, (int n, ENTITY * path));
ENTITY *PROTO (bi_getenv, (ENTITY * name));

#endif
