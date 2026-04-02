

/*
   pmem.h

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: pmem.h,v 1.2 2001/08/26 05:47:58 ksh Exp $ */

#ifndef  PMEM_H
#define  PMEM_H		1

#include "mem.h"

typedef double pm_align_t;

#define BLOCK_SIZE  sizeof(pm_align_t)
#define PG_SIZE	256

union pblock
  {
    union pblock *link;
    pm_align_t blocks[PG_SIZE];
  };

struct pm
  {
    pm_align_t *_avail;
    int _amt_avail;
    int _num_pages;
    union pblock *_pblock_list;
  };

extern struct pm active_pmem;

#define  avail	active_pmem._avail
#define  amt_avail  active_pmem._amt_avail
#define  num_pages  active_pmem._num_pages
#define  pblock_list  active_pmem._pblock_list

void *PROTO (xpmem, (int));
void PROTO (reset_pmem, (void));
void PROTO (free_all_pmem, (void));

#define  pmem(x)  xpmem(((x)+BLOCK_SIZE-1)/BLOCK_SIZE)
#define  P_MEM(x)  pmem(sizeof(x))

#endif
