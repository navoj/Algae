/*
   table_st.h -- The `table' structure.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: table_st.h,v 1.1.1.1 1996/04/17 05:56:22 ksh Exp $ */

#ifndef TABLE_ST_H
#define TABLE_ST_H	1

#include "entity.h"

typedef struct table_node
  {
    char *name;
    struct table_node *left;
    struct table_node *right;
    ENTITY *entity;
  }
TABLE_NODE;

struct table
  {
    ENTITY entity;
    int nm;			/* Number of members. */
    TABLE_NODE *table_node;
  };

#endif /* TABLE_ST_H  */
