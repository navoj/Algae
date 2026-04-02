/*
 * pmem.c -- parser memory
 *
 * Copyright (C) 1994-97  K. Scott Hunziker.
 * Copyright (C) 1990-94  The Boeing Company.
 *
 * See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: pmem.c,v 1.3 2001/08/26 05:47:58 ksh Exp $";

/*
 * The parser needs mem that's thrown away after execution.
 *
 * It gets it here.
 *
 * pmem() in turn gets it in big chunks from malloc()
 */

#include "algae.h"
#include "ptypes.h"
#include "code.h"
#include "pmem.h"

struct pm active_pmem;
   /* pmem info for current invocation of parse_and_exec()
      is stored here */


void *
xpmem (int blocks)
{
  void *retval;

  if (scope_level > 0)
    return MALLOC (blocks * BLOCK_SIZE);
  /* user functions don't use pmem */

  assert (blocks != 0);

  if (blocks > amt_avail)
    {
      union pblock *p = MALLOC (sizeof (union pblock));

      p->link = pblock_list;
      pblock_list = p;
      avail = &p->blocks[1];
      amt_avail = PG_SIZE - 1;
      num_pages++;

    }

  assert (blocks <= amt_avail);

  retval = (void *) avail;
  avail += blocks;
  amt_avail -= blocks;
  return retval;
}


/*
   free all but one page and reset avail and amount avail
   for that page
 */

void
reset_pmem (void)
{
  union pblock *p;


  if (num_pages)
    {

      while (num_pages > 1)
	{
	  num_pages--;
	  p = pblock_list;
	  pblock_list = p->link;
	  FREE (p);
	}
      avail = &pblock_list->blocks[1];
      amt_avail = PG_SIZE - 1;
    }
  else
    assert (avail == 0 && amt_avail == 0);
}


void
free_all_pmem (void)
{
  reset_pmem ();

  if (pblock_list)
    {
      FREE (pblock_list);
      pblock_list = (union pblock *) 0;
    }
}
