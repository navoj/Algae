/*
   dense.c -- Convert to dense storage.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: dense.c,v 1.1.1.1 1996/04/17 05:56:16 ksh Exp $";

#include "dense.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"

ENTITY *
dense_entity (ENTITY *ip)
{
  /* Convert entity to dense storage. */

  EASSERT (ip, 0, 0);

  switch (ip->class)
    {
    case scalar:
      return (ip);
    case vector:
      return (dense_vector ((VECTOR *) ip));
    case matrix:
      return (dense_matrix ((MATRIX *) ip));
    default:
      fail ("Can't make a %s entity dense.", class_string[ip->class]);
      delete_entity (ip);
      raise_exception ();
    }
}

ENTITY *
dense_matrix (MATRIX *ipm)
{
  /* Convert a matrix to dense storage. */

  int i, n, nr;
  MATRIX *opm;

  EASSERT (ipm, matrix, 0);

  switch (ipm->density)
    {
    case dense:
      return (ENT (ipm));
    case sparse:
    case sparse_upper:
      nr = ipm->nr;
      opm = (MATRIX *) make_matrix (ipm->nr, ipm->nc, ipm->type, dense);
      opm->symmetry = ipm->symmetry;
      if (ipm->rid != NULL)
	opm->rid = copy_entity (ipm->rid);
      if (ipm->cid != NULL)
	opm->cid = copy_entity (ipm->cid);

      if (ipm->nn > 0)
	{
	  switch (ipm->type)
	    {
	    case integer:
	      for (i = 0; i < nr; i++)
		{
		  POLL_SIGINT ();
		  for (n = ipm->ia[i] - 1; n < ipm->ia[i + 1] - 1; n++)
		    {
		      opm->a.integer[i + nr * (ipm->ja[n] - 1)] =
			ipm->a.integer[n];
		    }
		}
	      break;
	    case real:
	      for (i = 0; i < nr; i++)
		{
		  POLL_SIGINT ();
		  for (n = ipm->ia[i] - 1; n < ipm->ia[i + 1] - 1; n++)
		    {
		      opm->a.real[i + nr * (ipm->ja[n] - 1)] = ipm->a.real[n];
		    }
		}
	      break;
	    case complex:
	      for (i = 0; i < nr; i++)
		{
		  POLL_SIGINT ();
		  for (n = ipm->ia[i] - 1; n < ipm->ia[i + 1] - 1; n++)
		    {
		      opm->a.complex[i + nr * (ipm->ja[n] - 1)] =
			ipm->a.complex[n];
		    }
		}
	      break;
	    case character:
	      for (i = 0; i < nr; i++)
		{
		  POLL_SIGINT ();
		  for (n = ipm->ia[i] - 1; n < ipm->ia[i + 1] - 1; n++)
		    {
		      opm->a.character[i + nr * (ipm->ja[n] - 1)] =
			dup_char (ipm->a.character[n]);
		    }
		}
	      break;
	    default:
	      BAD_TYPE (ipm->type);
	      delete_2_matrices (ipm, opm);
	      raise_exception ();
	    }
	}
      if (ipm->density == sparse_upper)
	{
	  switch (ipm->type)
	    {
	    case integer:
	      for (i = 0; i < nr; i++)
		{
		  POLL_SIGINT ();
		  if (ipm->nn > 0)
		    {
		      for (n = ipm->ia[i] - 1; n < ipm->ia[i + 1] - 1; n++)
			{
			  opm->a.integer[nr * i + (ipm->ja[n] - 1)] =
			    ipm->a.integer[n];
			}
		    }
		}
	      if (ipm->d.integer != NULL)
		{
		  for (i = 0; i < nr; i++)
		    {
		      opm->a.integer[i * (nr + 1)] = ipm->d.integer[i];
		    }
		}
	      break;
	    case real:
	      for (i = 0; i < nr; i++)
		{
		  POLL_SIGINT ();
		  if (ipm->nn > 0)
		    {
		      for (n = ipm->ia[i] - 1; n < ipm->ia[i + 1] - 1; n++)
			{
			  opm->a.real[nr * i + (ipm->ja[n] - 1)] = ipm->a.real[n];
			}
		    }
		}
	      if (ipm->d.real != NULL)
		{
		  for (i = 0; i < nr; i++)
		    {
		      opm->a.real[i * (nr + 1)] = ipm->d.real[i];
		    }
		}
	      break;
	    case complex:
	      for (i = 0; i < nr; i++)
		{
		  POLL_SIGINT ();
		  if (ipm->nn > 0)
		    {
		      for (n = ipm->ia[i] - 1; n < ipm->ia[i + 1] - 1; n++)
			{
			  opm->a.complex[nr * i + (ipm->ja[n] - 1)] =
			    ipm->a.complex[n];
			}
		    }
		}
	      if (ipm->d.complex != NULL)
		{
		  for (i = 0; i < nr; i++)
		    {
		      opm->a.complex[i * (nr + 1)] = ipm->d.complex[i];
		    }
		}
	      break;
	    case character:
	      for (i = 0; i < nr; i++)
		{
		  POLL_SIGINT ();
		  if (ipm->nn > 0)
		    {
		      for (n = ipm->ia[i] - 1; n < ipm->ia[i + 1] - 1; n++)
			{
			  opm->a.character[nr * i + (ipm->ja[n] - 1)] =
			    dup_char (ipm->a.character[n]);
			}
		    }
		}
	      if (ipm->d.character != NULL)
		{
		  for (i = 0; i < nr; i++)
		    {
		      opm->a.character[i * (nr + 1)] =
			dup_char (ipm->d.character[i]);
		    }
		}
	      break;
	    default:
	      BAD_TYPE (ipm->type);
	      delete_2_matrices (ipm, opm);
	      raise_exception ();
	    }
	}
      break;
    default:
      BAD_DENSITY (ipm->density);
      delete_matrix (ipm);
      raise_exception ();
    }
  delete_matrix (ipm);
  return (ENT (opm));
}

ENTITY *
dense_vector (VECTOR *ipv)
{
  /* Convert a vector from sparse to dense form. */

  int n;
  VECTOR *opv;

  EASSERT (ipv, vector, 0);

  switch (ipv->density)
    {
    case dense:
      return (ENT (ipv));	/* Already dense. */
    case sparse:
      opv = (VECTOR *) make_vector (ipv->ne, ipv->type, dense);
      /* Get the labels. */
      if (ipv->eid != NULL)
	opv->eid = copy_entity (ipv->eid);

      /*
       * `make_vector' gave us values appropriate for the elements not
       * stored, so just go through and get the ones that are stored.
       */

      if (ipv->nn > 0)
	{
	  switch (ipv->type)
	    {
	    case integer:
	      for (n = 0; n < ipv->nn; n++)
		{
		  opv->a.integer[ipv->ja[n] - 1] = ipv->a.integer[n];
		}
	      break;
	    case real:
	      for (n = 0; n < ipv->nn; n++)
		{
		  opv->a.real[ipv->ja[n] - 1] = ipv->a.real[n];
		}
	      break;
	    case complex:
	      for (n = 0; n < ipv->nn; n++)
		{
		  opv->a.complex[ipv->ja[n] - 1] = ipv->a.complex[n];
		}
	      break;
	    case character:
	      for (n = 0; n < ipv->nn; n++)
		{
		  opv->a.character[ipv->ja[n] - 1] =
		    dup_char (ipv->a.character[n]);
		}
	      break;
	    default:
	      BAD_TYPE (ipv->type);
	      delete_2_vectors (ipv, opv);
	      raise_exception ();
	    }
	}
      break;
    default:
      BAD_DENSITY (ipv->density);
      delete_vector (ipv);
      raise_exception ();
    }
  delete_vector (ipv);
  return (ENT (opv));
}

ENTITY *
apt_vector (VECTOR *v)
{
  /*
   * This function checks for "fat" sparse vectors; that is, vectors
   * that are stored in sparse form but have so many nonzeros that
   * they ought to be stored dense.  If `v' is dense, or if we
   * decide that it isn't "fat", then it's returned unchanged.
   * Otherwise we convert it to dense form.
   */

  EASSERT (v, vector, 0);

  return (fat_vector (v->ne, v->nn, v->type, v->density)) ?
    dense_vector (v) : ENT (v);
}

#define FAT_VECTOR	1.5

int
fat_vector (int ne, int nn, TYPE type, DENSITY density)
{
  /*
   * This function checks for "fat" sparse vectors; that is, vectors that
   * are stored in sparse form but have so many nonzeros that they ought
   * to be stored dense.  Naturally, we can't really know for sure how
   * many that is without knowing exactly what the vector is to be used
   * for.  Instead, we'll use the simple-minded approach of basing it on
   * the amount of memory each one takes.
   *
   * The macro `FAT_VECTOR' is used to make our decision.  It represents
   * the storage penalty we're willing to pay to store it dense.  If it's
   * 1, for example, then we'll convert to dense only if it saves space.
   * A value of 2 would mean that we'll convert to dense even if it takes
   * twice as much space.
   */

  return (density == sparse &&
	  nn * (type_size[type] + sizeof (int)) * FAT_VECTOR >
	    (double) ne * type_size[type]);
}

ENTITY *
apt_matrix (MATRIX *m)
{
  /*
   * This function checks for "fat" sparse matrices; that is,
   * matrices that are stored in sparse form but have so many
   * nonzeros that they ought to be stored dense.  If `m' is dense,
   * or if we decide that it isn't "fat", then it's returned
   * unchanged.  Otherwise we convert it to dense form.
   */

  EASSERT (m, matrix, 0);

  return (fat_matrix (m->nr, m->nc, m->nn, m->type, m->density)) ?
    dense_matrix (m) : ENT (m);
}

#define FAT_MATRIX	1.5

int
fat_matrix (int nr, int nc, int nn, TYPE type, DENSITY density)
{
  /*
   * This function checks for "fat" sparse matrices; that is, matrices that
   * are stored in sparse form but have so many nonzeros that they ought
   * to be stored dense.  Naturally, we can't really know for sure how
   * many that is without knowing exactly what the matrix is to be used
   * for.  Instead, we'll use the simple-minded approach of basing it on
   * the amount of memory each one takes.
   *
   * The macro `FAT_MATRIX' is used to make our decision.  It represents
   * the storage penalty we're willing to pay to store it dense.  If it's
   * 1, for example, then we'll convert to dense only if it saves space.
   * A value of 2 would mean that we'll convert to dense even if it takes
   * twice as much space.
   */

  if (density != dense)
    {
      if (density == sparse)
	{
	  if ((nn * (type_size[type] + sizeof (int)) +
	       nr * sizeof (int)) * FAT_MATRIX >
	        (double) nr * (double) nc * type_size[type])
	      return 1;
	}
      else
	{
	  assert (density == sparse_upper);
	  if ((nn + nr) * (type_size[type] + sizeof (int)) *
	      FAT_MATRIX >
	        (double) nr * (double) nc * type_size[type])
	      return 1;
	}
    }

  return 0;
}
