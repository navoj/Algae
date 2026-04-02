/*
   sparse.c -- Sparse routines.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: sparse.c,v 1.7 2003/08/01 04:57:48 ksh Exp $";

#include "sparse.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "dense.h"
#include "cast.h"
#include "full.h"

ENTITY *
sparse_entity (ENTITY *ip)
{
  /*
   * This function converts matrices to more specialized forms.
   * A matrix in dense form will be converted to sparse or
   * sparse_upper, depending on its symmetry.  If you somehow
   * get a matrix that is sparse but has one of the symmetric
   * properties (symmetric or hermitian), it will be converted to
   * sparse_upper.
   */

  EASSERT (ip, 0, 0);

  switch (ip->class)
    {
    case scalar:
      return (ip);
    case vector:
      return (sparse_vector ((VECTOR *) ip));
    case matrix:
      return (sparse_matrix ((MATRIX *) ip));
    default:
      fail ("Can't make a %s entity sparse.",
	    class_string[ip->class]);
      delete_entity (ip);
      raise_exception ();
    }
}

ENTITY *
sparse_vector (VECTOR *ipv)
{
  /*
   * This routine converts a vector to `sparse' form.  A dense
   * vector is always converted, even if it has no zeros.  Vectors
   * that are already in `sparse' form will have any zeros removed.
   */

  int i, k, ne;
  VECTOR *opv;
  TYPE typ;
  static char msg[] = "Can't make a %s vector sparse.";

  EASSERT (ipv, vector, 0);

  typ = ipv->type;
  ne = ipv->ne;

  switch (ipv->density)
    {
    case dense:

      /* Make the new sparse vector that includes only the nonzeros. */

      opv = (VECTOR *) make_vector (ne, typ, sparse);

      if (ipv->eid != NULL)
	opv->eid = copy_entity (ipv->eid);

      /* Count the number of nonzeros */

      k = 1;
      switch (typ)
	{
	case integer:
	  for (i = 0; i < ne; i++)
	    {
	      if (ipv->a.integer[i] != 0)
		k++;
	    }
	  break;
	case real:
	  for (i = 0; i < ne; i++)
	    {
	      if (ipv->a.real[i] != (REAL) 0.0)
		k++;
	    }
	  break;
	case complex:
	  for (i = 0; i < ne; i++)
	    {
	      if (ipv->a.complex[i].real != 0.0 ||
		  ipv->a.complex[i].imag != 0.0)
		k++;
	    }
	  break;
	case character:
	  for (i = 0; i < ne; i++)
	    {
	      if (*ipv->a.character[i] != '\0')
		k++;
	    }
	  break;
	default:
	  wipeout (msg, type_string[typ]);
	}
      opv->nn = --k;

      /* If there are any nonzeros, copy them into the new vector. */
      /* If there are none, then we don't need `ja', or `a'. */

      if (k > 0)
	{
	  opv->ja = (int *) MALLOC (k * sizeof (int));
	  opv->a.ptr = E_MALLOC (k, typ);
	  k = 0;
	  switch (typ)
	    {
	    case integer:
	      for (i = 0; i < ne; i++)
		{
		  if (ipv->a.integer[i] != 0)
		    {
		      opv->ja[k] = i + 1;
		      opv->a.integer[k] = ipv->a.integer[i];
		      k++;
		    }
		}
	      break;
	    case real:
	      for (i = 0; i < ne; i++)
		{
		  if (ipv->a.real[i] != (REAL) 0.0)
		    {
		      opv->ja[k] = i + 1;
		      opv->a.real[k] = ipv->a.real[i];
		      k++;
		    }
		}
	      break;
	    case complex:
	      for (i = 0; i < ne; i++)
		{
		  if (ipv->a.complex[i].real != 0.0 ||
		      ipv->a.complex[i].imag != 0.0)
		    {
		      opv->ja[k] = i + 1;
		      opv->a.complex[k] = ipv->a.complex[i];
		      k++;
		    }
		}
	      break;
	    case character:
	      for (i = 0; i < ne; i++)
		{
		  if (ipv->a.character[i] &&
		      *ipv->a.character[i] != '\0')
		    {
		      opv->ja[k] = i + 1;
		      opv->a.character[k] = dup_char (ipv->a.character[i]);
		      k++;
		    }
		}
	      break;
	    default:
	      wipeout (msg, type_string[typ]);
	    }
	}
      delete_entity ((ENTITY *) ipv);
      break;
    case sparse:
      switch (typ)
	{
	case integer:
	  for (i = 0; i < ipv->nn; i++)
	    if (ipv->a.integer[i] == 0)
	      break;
	  break;
	case real:
	  for (i = 0; i < ipv->nn; i++)
	    if (ipv->a.real[i] == 0.0)
	      break;
	  break;
	case complex:
	  for (i = 0; i < ipv->nn; i++)
	    if (ipv->a.complex[i].real == 0.0 &&
		ipv->a.complex[i].imag == 0.0)
	      break;
	  break;
	case character:
	  for (i = 0; i < ipv->nn; i++)
	    if (*ipv->a.character[i] == '\0')
	      break;
	  break;
	default:
	  wipeout (msg, type_string[typ]);
	}
      if (i >= ipv->nn)
	{			/* If true, then no changes are needed. */
	  return ((ENTITY *) ipv);
	}
      opv = (VECTOR *) dup_vector (ipv);

      if (opv->nn == 0)
	break;

      switch (typ)
	{
	case integer:
	  for (i = 0; i < opv->nn; i++)
	    {
	      if (opv->a.integer[i] == 0)
		{
		  opv->ja[i] = 0;
		}
	    }
	  break;
	case real:
	  for (i = 0; i < opv->nn; i++)
	    {
	      if (opv->a.real[i] == (REAL) 0.0)
		{
		  opv->ja[i] = 0;
		}
	    }
	  break;
	case complex:
	  for (i = 0; i < opv->nn; i++)
	    {
	      if (opv->a.complex[i].real == (REAL) 0.0 &&
		  opv->a.complex[i].imag == (REAL) 0.0)
		{
		  opv->ja[i] = 0;
		}
	    }
	  break;
	case character:
	  for (i = 0; i < opv->nn; i++)
	    {
	      if (*opv->a.character[i] == '\0')
		{
		  opv->ja[i] = 0;
		  if (opv->a.character[i] != NULL_string)
		    FREE (opv->a.character[i]);
		}
	    }
	  break;
	default:
	  wipeout (msg, type_string[typ]);
	}
      opv = (VECTOR *) squeeze_vector (opv);
      break;
    default:
      wipeout (msg, density_string[ipv->density]);
    }
  return ((ENTITY *) opv);
}

ENTITY *
sparse_matrix (MATRIX *ipm)
{
  /*
   * This routine converts a matrix to `sparse' or `sparse_upper'
   * form, according to its symmetry.  A dense matrix is always
   * converted, even if it has no zeros.  Matrices that are already
   * in `sparse' or `sparse_upper' form will have any zeros removed.
   * Matrices in `sparse' form will be converted to `sparse_upper'
   * if possible.
   */

  int i, j, k, nr, nc;
  size_t obj_size;
  MATRIX *opm;
  TYPE typ;
  static char msg[] = "Can't make a %s matrix sparse.";

  EASSERT (ipm, matrix, 0);

  typ = ipm->type;
  nr = ipm->nr;
  nc = ipm->nc;
  obj_size = type_size[typ];

  switch (ipm->density)
    {
    case dense:

      /* Make the new sparse matrix that includes only the nonzeros. */

      opm = (MATRIX *) make_matrix (nr, nc, typ, sparse);
      opm->symmetry = ipm->symmetry;

      if (ipm->rid != NULL)
	opm->rid = copy_entity (ipm->rid);
      if (ipm->cid != NULL)
	opm->cid = copy_entity (ipm->cid);

      opm->ia = (int *) MALLOC ((nr + 1) * sizeof (int));

      /* Count the number of nonzeros */

      switch (ipm->symmetry)
	{
	case general:
	  k = 1;
	  opm->ia[0] = 1;

	  switch (typ)
	    {
	    case integer:
	      for (i = 0; i < nr; i++)
		{
		  for (j = 0; j < nc; j++)
		    {
		      if (ipm->a.integer[i + j * nr] != 0)
			k++;
		    }
		  opm->ia[i + 1] = k;
		}
	      break;
	    case real:
	      for (i = 0; i < nr; i++)
		{
		  for (j = 0; j < nc; j++)
		    {
		      if (ipm->a.real[i + j * nr] != (REAL) 0.0)
			k++;
		    }
		  opm->ia[i + 1] = k;
		}
	      break;
	    case complex:
	      for (i = 0; i < nr; i++)
		{
		  for (j = 0; j < nc; j++)
		    {
		      if (ipm->a.complex[i + j * nr].real != 0.0 ||
			  ipm->a.complex[i + j * nr].imag != 0.0)
			k++;
		    }
		  opm->ia[i + 1] = k;
		}
	      break;
	    case character:
	      for (i = 0; i < nr; i++)
		{
		  for (j = 0; j < nc; j++)
		    {
		      if (*ipm->a.character[i + j * nr] != '\0')
			k++;
		    }
		  opm->ia[i + 1] = k;
		}
	      break;
	    default:
	      wipeout (msg, type_string[typ]);
	    }
	  opm->nn = --k;

	  /* If there are any nonzeros, copy them into the new matrix. */
	  /* If there are none, then we don't need `ia', `ja', or `a'. */

	  if (k > 0)
	    {
	      opm->ja = (int *) MALLOC (k * sizeof (int));
	      opm->a.ptr = E_MALLOC (k, typ);

	      switch (typ)
		{
		case integer:
		  for (i = 0; i < nr; i++)
		    {
		      k = opm->ia[i];
		      for (j = 0; j < nc; j++)
			{
			  if (ipm->a.integer[i + j * nr] != 0)
			    {
			      opm->ja[k - 1] = j + 1;
			      opm->a.integer[k - 1] = ipm->a.integer[i + j * nr];
			      k++;
			    }
			}
		    }
		  break;
		case real:
		  for (i = 0; i < nr; i++)
		    {
		      k = opm->ia[i];
		      for (j = 0; j < nc; j++)
			{
			  if (ipm->a.real[i + j * nr] != (REAL) 0.0)
			    {
			      opm->ja[k - 1] = j + 1;
			      opm->a.real[k - 1] = ipm->a.real[i + j * nr];
			      k++;
			    }
			}
		    }
		  break;
		case complex:
		  for (i = 0; i < nr; i++)
		    {
		      k = opm->ia[i];
		      for (j = 0; j < nc; j++)
			{
			  if (ipm->a.complex[i + j * nr].real != 0.0 ||
			      ipm->a.complex[i + j * nr].imag != 0.0)
			    {
			      opm->ja[k - 1] = j + 1;
			      opm->a.complex[k - 1] = ipm->a.complex[i + j * nr];
			      k++;
			    }
			}
		    }
		  break;
		case character:
		  for (i = 0; i < nr; i++)
		    {
		      k = opm->ia[i];
		      for (j = 0; j < nc; j++)
			{
			  if (*ipm->a.character[i + j * nr] != '\0')
			    {
			      opm->ja[k - 1] = j + 1;
			      opm->a.character[k - 1] =
				dup_char (ipm->a.character[i + j * nr]);
			      k++;
			    }
			}
		    }
		  break;
		default:
		  wipeout (msg, type_string[typ]);
		}
	    }
	  else
	    {
	      FREE (opm->ia);
	      opm->ia = NULL;
	    }
	  break;
	case symmetric:
	case hermitian:

	  opm->density = sparse_upper;
	  if (nr)
	    opm->d.integer = (int *) MALLOC (nr * obj_size);

	  k = 1;
	  opm->ia[0] = 1;

	  switch (typ)
	    {
	    case integer:
	      for (i = 0; i < nr; i++)
		{
		  opm->d.integer[i] = ipm->a.integer[i * (1 + nr)];
		  for (j = i + 1; j < nc; j++)
		    {
		      if (ipm->a.integer[i + j * nr] != 0)
			k++;
		    }
		  opm->ia[i + 1] = k;
		}
	      break;
	    case real:
	      for (i = 0; i < nr; i++)
		{
		  opm->d.real[i] = ipm->a.real[i * (1 + nr)];
		  for (j = i + 1; j < nc; j++)
		    {
		      if (ipm->a.real[i + j * nr] != (REAL) 0.0)
			k++;
		    }
		  opm->ia[i + 1] = k;
		}
	      break;
	    case complex:
	      for (i = 0; i < nr; i++)
		{
		  opm->d.complex[i] = ipm->a.complex[i * (1 + nr)];
		  for (j = i + 1; j < nc; j++)
		    {
		      if (ipm->a.complex[i + j * nr].real != 0.0 ||
			  ipm->a.complex[i + j * nr].imag != 0.0)
			k++;
		    }
		  opm->ia[i + 1] = k;
		}
	      break;
	    case character:
	      for (i = 0; i < nr; i++)
		{
		  opm->d.character[i] =
		    dup_char (ipm->a.character[i * (1 + nr)]);
		  for (j = i + 1; j < nc; j++)
		    {
		      if (*ipm->a.character[i + j * nr] != '\0')
			k++;
		    }
		  opm->ia[i + 1] = k;
		}
	      break;
	    default:
	      wipeout (msg, type_string[typ]);
	    }
	  opm->nn = --k;

	  if (k > 0)
	    {
	      opm->ja = (int *) MALLOC (k * sizeof (int));
	      opm->a.ptr = E_MALLOC (k, typ);

	      switch (typ)
		{
		case integer:
		  for (i = 0; i < nr; i++)
		    {
		      k = opm->ia[i];
		      for (j = i + 1; j < nc; j++)
			{
			  if (ipm->a.integer[i + j * nr] != 0)
			    {
			      opm->ja[k - 1] = j + 1;
			      opm->a.integer[k - 1] = ipm->a.integer[i + j * nr];
			      k++;
			    }
			}
		    }
		  break;
		case real:
		  for (i = 0; i < nr; i++)
		    {
		      k = opm->ia[i];
		      for (j = i + 1; j < nc; j++)
			{
			  if (ipm->a.real[i + j * nr] != (REAL) 0.0)
			    {
			      opm->ja[k - 1] = j + 1;
			      opm->a.real[k - 1] = ipm->a.real[i + j * nr];
			      k++;
			    }
			}
		    }
		  break;
		case complex:
		  for (i = 0; i < nr; i++)
		    {
		      k = opm->ia[i];
		      for (j = i + 1; j < nc; j++)
			{
			  if (ipm->a.complex[i + j * nr].real != 0.0 ||
			      ipm->a.complex[i + j * nr].imag != 0.0)
			    {
			      opm->ja[k - 1] = j + 1;
			      opm->a.complex[k - 1] = ipm->a.complex[i + j * nr];
			      k++;
			    }
			}
		    }
		  break;
		case character:
		  for (i = 0; i < nr; i++)
		    {
		      k = opm->ia[i];
		      for (j = i + 1; j < nc; j++)
			{
			  if (*ipm->a.character[i + j * nr] != '\0')
			    {
			      opm->ja[k - 1] = j + 1;
			      opm->a.character[k - 1] =
				dup_char (ipm->a.character[i + j * nr]);
			      k++;
			    }
			}
		    }
		  break;
		default:
		  wipeout (msg, type_string[typ]);
		}
	    }
	  else
	    {
	      FREE (opm->ia);
	      opm->ia = NULL;
	    }
	  break;
	default:
	  wipeout (msg, symmetry_string[ipm->symmetry]);
	}
      break;
    case sparse:

      /*
       * If the symmetry is such that we can store the matrix in
       * `sparse_upper' form, then we'll convert it.  If not, then
       * we'll first check to see if there are any zeros to begin
       * with.
       */

      switch (ipm->symmetry)
	{
	case general:
	  switch (typ)
	    {
	    case integer:
	      for (i = 0; i < ipm->nn; i++)
		if (ipm->a.integer[i] == 0)
		  break;
	      break;
	    case real:
	      for (i = 0; i < ipm->nn; i++)
		if (ipm->a.real[i] == 0.0)
		  break;
	      break;
	    case complex:
	      for (i = 0; i < ipm->nn; i++)
		if (ipm->a.complex[i].real == 0.0 &&
		    ipm->a.complex[i].imag == 0.0)
		  break;
	      break;
	    case character:
	      for (i = 0; i < ipm->nn; i++)
		if (*ipm->a.character[i] == '\0')
		  break;
	      break;
	    default:
	      wipeout (msg, type_string[typ]);
	    }
	  if (i >= ipm->nn)
	    {			/* If true, then no changes are needed. */
	      return ((ENTITY *) ipm);
	    }
	  opm = (MATRIX *) dup_matrix ((MATRIX *) EAT (ipm));
	  break;
	case symmetric:
	case hermitian:
	  opm = (MATRIX *) dup_matrix ((MATRIX *) EAT (ipm));
	  opm->density = sparse_upper;
	  if (opm->nn > 0)
	    opm->d.integer = (int *) CALLOC (nr, obj_size);
	  break;
	default:
	  wipeout (msg, symmetry_string[ipm->symmetry]);
	}

      if (opm->nn == 0)
	break;

      switch (typ)
	{
	case integer:
	  if (opm->density == sparse_upper)
	    {
	      for (i = 0; i < nr; i++)
		{
		  for (j = opm->ia[i]; j < opm->ia[i + 1]; j++)
		    {
		      if (opm->ja[j - 1] - 1 < i)
			{
			  opm->ja[j - 1] = 0;
			}
		      else if (opm->ja[j - 1] - 1 == i)
			{
			  opm->ja[j - 1] = 0;
			  opm->d.integer[i] = opm->a.integer[j - 1];
			}
		    }
		}
	    }
	  for (i = 0; i < opm->nn; i++)
	    {
	      if (opm->a.integer[i] == 0)
		{
		  opm->ja[i] = 0;
		}
	    }
	  break;
	case real:
	  if (opm->density == sparse_upper)
	    {
	      for (i = 0; i < nr; i++)
		{
		  for (j = opm->ia[i]; j < opm->ia[i + 1]; j++)
		    {
		      if (opm->ja[j - 1] - 1 < i)
			{
			  opm->ja[j - 1] = 0;
			}
		      else if (opm->ja[j - 1] - 1 == i)
			{
			  opm->ja[j - 1] = 0;
			  opm->d.real[i] = opm->a.real[j - 1];
			}
		    }
		}
	    }
	  for (i = 0; i < opm->nn; i++)
	    {
	      if (opm->a.real[i] == (REAL) 0.0)
		{
		  opm->ja[i] = 0;
		}
	    }
	  break;
	case complex:
	  if (opm->density == sparse_upper)
	    {
	      for (i = 0; i < nr; i++)
		{
		  for (j = opm->ia[i]; j < opm->ia[i + 1]; j++)
		    {
		      if (opm->ja[j - 1] - 1 < i)
			{
			  opm->ja[j - 1] = 0;
			}
		      else if (opm->ja[j - 1] - 1 == i)
			{
			  opm->ja[j - 1] = 0;
			  opm->d.complex[i] = opm->a.complex[j - 1];
			}
		    }
		}
	    }
	  for (i = 0; i < opm->nn; i++)
	    {
	      if (opm->a.complex[i].real == (REAL) 0.0 &&
		  opm->a.complex[i].imag == (REAL) 0.0)
		{
		  opm->ja[i] = 0;
		}
	    }
	  break;
	case character:
	  if (opm->density == sparse_upper)
	    {
	      for (i = 0; i < nr; i++)
		{
		  for (j = opm->ia[i]; j < opm->ia[i + 1]; j++)
		    {
		      if (opm->ja[j - 1] - 1 < i)
			{
			  opm->ja[j - 1] = 0;
			  if (opm->a.character[j - 1] != NULL_string)
			    FREE (opm->a.character[j - 1]);
			}
		      else if (opm->ja[j - 1] - 1 == i)
			{
			  opm->ja[j - 1] = 0;
			  opm->d.character[i] = opm->a.character[j - 1];
			}
		    }
		}
	    }
	  for (i = 0; i < opm->nn; i++)
	    {
	      if (*opm->a.character[i] == '\0')
		{
		  opm->ja[i] = 0;
		  if (opm->a.character[i] != NULL_string)
		    FREE (opm->a.character[i]);
		}
	    }
	  break;
	default:
	  wipeout (msg, type_string[typ]);
	}
      opm = (MATRIX *) squeeze_matrix (opm);
      break;
    case sparse_upper:

      /* First see if any changes are needed. */

      if (ipm->nn == 0)
	return ((ENTITY *) ipm);

      switch (typ)
	{
	case integer:
	  for (i = 0; i < ipm->nn; i++)
	    if (ipm->a.integer[i] == 0)
	      break;
	  break;
	case real:
	  for (i = 0; i < ipm->nn; i++)
	    if (ipm->a.real[i] == 0.0)
	      break;
	  break;
	case complex:
	  for (i = 0; i < ipm->nn; i++)
	    if (ipm->a.complex[i].real == 0.0 &&
		ipm->a.complex[i].imag == 0.0)
	      break;
	  break;
	case character:
	  for (i = 0; i < ipm->nn; i++)
	    if (*ipm->a.character[i] == '\0')
	      break;
	  break;
	default:
	  wipeout (msg, type_string[typ]);
	}
      if (i >= ipm->nn)
	{			/* If true, then no changes are needed. */
	  return ((ENTITY *) ipm);
	}

      opm = (MATRIX *) dup_matrix ((MATRIX *) EAT (ipm));

      switch (typ)
	{
	case integer:
	  for (i = 0; i < opm->nn; i++)
	    {
	      if (opm->a.integer[i] == 0)
		{
		  opm->ja[i] = 0;
		}
	    }
	  break;
	case real:
	  for (i = 0; i < opm->nn; i++)
	    {
	      if (opm->a.real[i] == (REAL) 0.0)
		{
		  opm->ja[i] = 0;
		}
	    }
	  break;
	case complex:
	  for (i = 0; i < opm->nn; i++)
	    {
	      if (opm->a.complex[i].real == (REAL) 0.0 &&
		  opm->a.complex[i].imag == (REAL) 0.0)
		{
		  opm->ja[i] = 0;
		}
	    }
	case character:
	  for (i = 0; i < opm->nn; i++)
	    {
	      if (*opm->a.character[i] == '\0')
		{
		  opm->ja[i] = 0;
		  if (opm->a.character[i] != NULL_string)
		    FREE (opm->a.character[i]);
		}
	    }
	  break;
	default:
	  wipeout (msg, type_string[typ]);
	}
      opm = (MATRIX *) squeeze_matrix (opm);
      break;
    default:
      wipeout (msg, density_string[ipm->density]);
    }

  if (opm->nn == 0 && nr == nc)
    {
      assert (((ENTITY *) opm)->ref_count == 1);
      if (typ == complex)
	{
	  opm->symmetry = hermitian;
	}
      else
	{
	  opm->symmetry = symmetric;
	}
    }

  delete_matrix (ipm);
  return ((ENTITY *) opm);
}

ENTITY *
squeeze_matrix (MATRIX *p)
{
  /*
   * Sometimes we need to delete elements out of a sparse matrix.
   * We do this by setting the appropriate elements in "ja" to zero.
   * This is not a standard form for sparse matrices, though, and
   * this routine exists to "squeeze" the deleted elements out of
   * the matrix.  It's OK, too, if the pointer to the data is NULL.
   */

  /*
   * NOTE:  For `character' type matrices, `FREE' is not called.
   *        To delete an element, you should call FREE in
   *        addition to setting the element of `ja' to zero.
   */

  int i, j, k, m;
  size_t obj_size;

  assert (p->entity.ref_count == 1);
  assert (p->density == sparse || p->density == sparse_upper);

  obj_size = type_size[p->type];

  if (p->nn > 0)
    {
      k = 0;
      m = 1;
      switch (p->type)
	{
	case integer:
	  for (i = 0; i < p->nr; i++)
	    {
	      if (p->a.integer != NULL)
		{
		  for (j = m; j < p->ia[i + 1]; j++)
		    {
		      if (p->ja[j - 1] != 0)
			{
			  p->ja[k] = p->ja[j - 1];
			  p->a.integer[k++] = p->a.integer[j - 1];
			}
		    }
		  m = p->ia[i + 1];
		  p->ia[i + 1] = k + 1;
		}
	      else
		{
		  for (j = m; j < p->ia[i + 1]; j++)
		    {
		      if (p->ja[j - 1] != 0)
			{
			  p->ja[k] = p->ja[j - 1];
			  k++;
			}
		    }
		  m = p->ia[i + 1];
		  p->ia[i + 1] = k + 1;
		}
	    }
	  p->nn = p->ia[i] - 1;
	  break;
	case real:
	  for (i = 0; i < p->nr; i++)
	    {
	      if (p->a.real != NULL)
		{
		  for (j = m; j < p->ia[i + 1]; j++)
		    {
		      if (p->ja[j - 1] != 0)
			{
			  p->ja[k] = p->ja[j - 1];
			  p->a.real[k++] = p->a.real[j - 1];
			}
		    }
		  m = p->ia[i + 1];
		  p->ia[i + 1] = k + 1;
		}
	      else
		{
		  for (j = m; j < p->ia[i + 1]; j++)
		    {
		      if (p->ja[j - 1] != 0)
			{
			  p->ja[k] = p->ja[j - 1];
			  k++;
			}
		    }
		  m = p->ia[i + 1];
		  p->ia[i + 1] = k + 1;
		}
	    }
	  p->nn = p->ia[i] - 1;
	  break;
	case complex:
	  for (i = 0; i < p->nr; i++)
	    {
	      if (p->a.complex != NULL)
		{
		  for (j = m; j < p->ia[i + 1]; j++)
		    {
		      if (p->ja[j - 1] != 0)
			{
			  p->ja[k] = p->ja[j - 1];
			  p->a.complex[k++] = p->a.complex[j - 1];
			}
		    }
		  m = p->ia[i + 1];
		  p->ia[i + 1] = k + 1;
		}
	      else
		{
		  for (j = m; j < p->ia[i + 1]; j++)
		    {
		      if (p->ja[j - 1] != 0)
			{
			  p->ja[k] = p->ja[j - 1];
			  k++;
			}
		    }
		  m = p->ia[i + 1];
		  p->ia[i + 1] = k + 1;
		}
	    }
	  p->nn = p->ia[i] - 1;
	  break;
	case character:
	  for (i = 0; i < p->nr; i++)
	    {
	      if (p->a.integer != NULL)
		{
		  for (j = m; j < p->ia[i + 1]; j++)
		    {
		      if (p->ja[j - 1] != 0)
			{
			  p->ja[k] = p->ja[j - 1];
			  p->a.character[k++] = p->a.character[j - 1];
			}
		    }
		  m = p->ia[i + 1];
		  p->ia[i + 1] = k + 1;
		}
	      else
		{
		  for (j = m; j < p->ia[i + 1]; j++)
		    {
		      if (p->ja[j - 1] != 0)
			{
			  p->ja[k] = p->ja[j - 1];
			  k++;
			}
		    }
		  m = p->ia[i + 1];
		  p->ia[i + 1] = k + 1;
		}
	    }
	  p->nn = p->ia[i] - 1;
	  break;
	default:
	  wipeout ("Can't handle %s type in routine \"squeeze_matrix\".",
		   type_string[p->type]);
	}

      if (p->nn > 0)
	{
	  p->ja = REALLOC (p->ja, p->nn * sizeof (int));
	  if (p->a.ptr != NULL)
	    p->a.ptr = REALLOC (p->a.ptr, p->nn * obj_size);
	}
      else
	{
	  TFREE (p->ia);
	  p->ia = NULL;
	  TFREE (p->ja);
	  p->ja = NULL;
	  TFREE (p->a.ptr);
	  p->a.ptr = NULL;
	}
    }
  return (ENT (p));
}

ENTITY *
squeeze_vector (VECTOR *p)
{
  /*  Same idea as for `squeeze_matrix'. */

  /*
   * NOTE:  For `character' type vectors, `FREE' is not called.
   *        To delete an element, you should call FREE in
   *        addition to setting the element of `ja' to zero
   *        prior to calling `squeeze_vector'.
   */

  int j, k;
  size_t obj_size;

  assert (p->entity.ref_count == 1);
  assert (p->density == sparse);

  obj_size = type_size[p->type];

  if (p->nn > 0)
    {
      k = 0;
      switch (p->type)
	{
	case integer:
	  if (p->a.integer != NULL)
	    {
	      for (j = 0; j < p->nn; j++)
		{
		  if (p->ja[j] != 0)
		    {
		      p->ja[k] = p->ja[j];
		      p->a.integer[k++] = p->a.integer[j];
		    }
		}
	    }
	  else
	    {
	      for (j = 0; j < p->nn; j++)
		{
		  if (p->ja[j] != 0)
		    {
		      p->ja[k] = p->ja[j];
		      k++;
		    }
		}
	    }
	  break;
	case real:
	  if (p->a.real != NULL)
	    {
	      for (j = 0; j < p->nn; j++)
		{
		  if (p->ja[j] != 0)
		    {
		      p->ja[k] = p->ja[j];
		      p->a.real[k++] = p->a.real[j];
		    }
		}
	    }
	  else
	    {
	      for (j = 0; j < p->nn; j++)
		{
		  if (p->ja[j] != 0)
		    {
		      p->ja[k] = p->ja[j];
		      k++;
		    }
		}
	    }
	  break;
	case complex:
	  if (p->a.complex != NULL)
	    {
	      for (j = 0; j < p->nn; j++)
		{
		  if (p->ja[j] != 0)
		    {
		      p->ja[k] = p->ja[j];
		      p->a.complex[k++] = p->a.complex[j];
		    }
		}
	    }
	  else
	    {
	      for (j = 0; j < p->nn; j++)
		{
		  if (p->ja[j] != 0)
		    {
		      p->ja[k] = p->ja[j];
		      k++;
		    }
		}
	    }
	  break;
	case character:
	  if (p->a.integer != NULL)
	    {
	      for (j = 0; j < p->nn; j++)
		{
		  if (p->ja[j] != 0)
		    {
		      p->ja[k] = p->ja[j];
		      p->a.character[k++] = p->a.character[j];
		    }
		}
	    }
	  else
	    {
	      for (j = 0; j < p->nn; j++)
		{
		  if (p->ja[j] != 0)
		    {
		      p->ja[k] = p->ja[j];
		      k++;
		    }
		}
	    }
	  break;
	default:
	  wipeout ("Can't handle %s type in routine \"squeeze_vector\".",
		   type_string[p->type]);
	}

      p->nn = k;
      if (p->nn > 0)
	{
	  p->ja = REALLOC (p->ja, p->nn * sizeof (int));
	  if (p->a.ptr != NULL)
	    p->a.ptr = REALLOC (p->a.ptr, p->nn * obj_size);
	}
      else
	{
	  TFREE (p->ja);
	  p->ja = NULL;
	  TFREE (p->a.ptr);
	  p->a.ptr = NULL;
	}
    }
  return (ENT (p));
}

ENTITY *
sparse_to_band (MATRIX *m)
{
  /*
   * This routine converts a square real or complex matrix into the
   * "band" form used by LAPACK.  The number of superdiagonals is `ku'
   * and the number of subdiagonals is `kl';  both are saved in the
   * `stuff' table.  The matrix returned has `ku+1+kl' rows.  If `m'
   * is symmetric, then `kl' is zero.
   */

  MATRIX *b;
  int i, k, tu, tl, kk;
  int ku = 0;
  int kl = 0;

  EASSERT (m, matrix, 0);
  assert (m->nr == m->nc);
  assert (m->type != integer);
  assert (m->type != character);
  assert (m->order == ordered);

  /* Convert to sparse, if necessary, and sparse_upper if possible. */

  m = (MATRIX *) sparse_matrix (m);

  /* Determine the number of diagonals. */

  if (m->nn)
    {
      for (i = 0; i < m->nr; i++)
	{
	  if (m->ia[i + 1] != m->ia[i])
	    {			/* nonzeros this row? */
	      tu = m->ja[m->ia[i + 1] - 2] - i - 1;	/* height above diag */
	      if (tu > ku)
		ku = tu;
	      tl = i - m->ja[m->ia[i] - 1] + 1;		/* depth below diag */
	      if (tl > kl)
		kl = tl;
	    }
	}
    }

  /* Form the "band" matrix. */

  kk = ku + kl + 1;
  b = (MATRIX *) make_matrix (kk, m->nc, m->type, dense);
  b->symmetry = general;
  if (m->cid)
    b->cid = copy_entity (m->cid);

  b->stuff = (TABLE *) replace_in_table ((TABLE *) make_table (),
					 int_to_scalar (ku),
					 dup_char ("ku"));
  b->stuff = (TABLE *) replace_in_table (b->stuff,
					 int_to_scalar (kl),
					 dup_char ("kl"));

  /* Now copy the nonzeros. */

  if (m->type == real)
    {
      if (m->nn)
	{
	  for (i = 0; i < m->nr; i++)
	    {
	      for (k = m->ia[i] - 1; k < m->ia[i + 1] - 1; k++)
		{
		  b->a.real[m->ja[k] * (kk - 1) - kl + i] = m->a.real[k];
		}
	    }
	}
      if (m->d.real)
	{
	  for (i = 0; i < m->nr; i++)
	    {
	      b->a.real[i * kk + ku] = m->d.real[i];
	    }
	}
    }
  else
    {				/* complex */
      if (m->nn)
	{
	  for (i = 0; i < m->nr; i++)
	    {
	      for (k = m->ia[i] - 1; k < m->ia[i + 1] - 1; k++)
		{
		  b->a.complex[m->ja[k] * (kk - 1) - kl + i] = m->a.complex[k];
		}
	    }
	}
      if (m->d.complex)
	{
	  for (i = 0; i < m->nr; i++)
	    {
	      b->a.complex[i * kk + ku] = m->d.complex[i];
	    }
	}
    }

  delete_matrix (m);

  return ENT (b);
}

static char *mksparse_specs[] =
{
  "shape",
  "rows",
  "cols",
  "values",
  NULL,
};
enum
  {
    MKS_SHAPE, MKS_ROWS, MKS_COLS, MKS_VALUES
  };

ENTITY *
bi_mksparse (int nn, ENTITY *t)
{
  /*
   * This function returns a sparse matrix with dimensions and
   * elements given by members of the table `t'.  The `shape' member
   * provides the dimensions of the matrix.  If it doesn't exist, the
   * minimum size is used which will include all of the given
   * elements.  The `rows', `cols', and `values' vectors must all have
   * the same length.  Their corresponding elements specify a single
   * element in the resulting sparse matrix.  If an element is given
   * more than once, its value is the sum of the given values.
   */

  volatile int nr,  nc;
  int ne, i, j, maxr, maxc, s, n;
  int * volatile r, *c, *p;
  int *ia = NULL;
  int *ja = NULL;
  int *ip = NULL;

  VECTOR *shape = NULL;
  VECTOR *rows = NULL;
  VECTOR *cols = NULL;
  VECTOR *values = NULL;

  MATRIX *m = NULL;

  /* If no specs at all, return a 0x0 real, sparse matrix. */

  if (!t) return make_matrix (0, 0, real, sparse);

  WITH_HANDLING
  {
    /* Be picky -- no members other than what we recognize. */

    if (t->class != table)
      {
	fail ("Argument to mksparse must be a table.");
	raise_exception ();
      }

    if (!OK_OPTIONS ((TABLE *) t, mksparse_specs)) raise_exception ();

    shape = (VECTOR *) search_in_table ((TABLE *) copy_table ((TABLE *) t),
					dup_char (mksparse_specs[MKS_SHAPE]));
    rows = (VECTOR *) search_in_table ((TABLE *) copy_table ((TABLE *) t),
				       dup_char (mksparse_specs[MKS_ROWS]));
    cols = (VECTOR *) search_in_table ((TABLE *) copy_table ((TABLE *) t),
				       dup_char (mksparse_specs[MKS_COLS]));
    values =
      (VECTOR *) search_in_table ((TABLE *) copy_table ((TABLE *) t),
				  dup_char (mksparse_specs[MKS_VALUES]));

    delete_table ((TABLE *) t);
    t = NULL;

    if (shape)
      {
	shape = (VECTOR *) vector_entity (EAT (shape));
	shape = (VECTOR *) dense_vector (EAT (shape));
	shape = (VECTOR *) cast_vector (EAT (shape), integer);

	if (shape->ne == 2)
	  {
	    nr = shape->a.integer[0];
	    if (nr < 0)
	      {
		fail ("Specified row dimension is negative.");
		raise_exception ();
	      }
	    nc = shape->a.integer[1];
	    if (nc < 0)
	      {
		fail ("Specified column dimension is negative.");
		raise_exception ();
	      }
	  }
	else
	  {
	    fail ("Invalid matrix dimensions specified in mksparse.");
	    raise_exception ();
	  }
      }

    if (rows)
      {
	rows = (VECTOR *) vector_entity (EAT (rows));
	rows = (VECTOR *) dense_vector (EAT (rows));
	rows = (VECTOR *) cast_vector (EAT (rows), integer);
	r = rows->a.integer;
      }
    else
      {
	fail ("No row numbers specified.");
	raise_exception ();
      }

    if (cols)
      {
	cols = (VECTOR *) vector_entity (EAT (cols));
	cols = (VECTOR *) dense_vector (EAT (cols));
	cols = (VECTOR *) cast_vector (EAT (cols), integer);
	cols = (VECTOR *) dup_vector ((VECTOR *) EAT (cols));
	c = cols->a.integer;
      }
    else
      {
	fail ("No column numbers specified.");
	raise_exception ();
      }

    if (values)
      {
	values = (VECTOR *) vector_entity (EAT (values));
	values = (VECTOR *) dense_vector (EAT (values));
      }
    else
      {
	fail ("No values specified.");
	raise_exception ();
      }

    if (values->type == character)
      {
	fail ("Invalid type in mksparse.");
	raise_exception ();
      }

    /* Check dimensions. */

    ne = values->ne;

    if (rows->ne != ne || cols->ne != ne)
      {
	fail ("Index and value dimensions do not match.");
	raise_exception ();
      }

    maxr = maxc = 0;
    for (i=0; i<ne; i++)
      {
	if (r[i] > maxr) maxr = r[i];
	if (c[i] > maxc) maxc = c[i];

	if (r[i] < 1 || c[i] < 1)
	  {
	    fail ("Row or column index is not positive.");
	    raise_exception ();
	  }
      }

    if (shape)
      {
	for (i=0; i<ne; i++)
	  if (r[i] > nr || c[i] > nc)
	    {
	      fail ("Row or column index exceeds specification.");
	      raise_exception ();
	    }
      }
    else
      {
	nr = maxr;
	nc = maxc;
      }

    if (ne)
      {

	/* Set up row indices. */

	ia = CALLOC (nr+1, sizeof (int));
	for (i=0; i<ne; i++) ia[r[i]]++;
	for (s=1,i=0; i<=nr; i++) s = (ia[i] += s);

	/* Set up indirect element pointers. */

	ip = CALLOC (nr, sizeof (int));	/* Count elements in each row. */
	ja = MALLOC (ne * sizeof (int));
	for (i=0; i<ne; i++)
	  {
	    j = r[i] - 1;
	    ja[ia[j]-1+ip[j]] = i+1;
	    ip[j]++;
	  }
	FREE (ip);
	ip = NULL;

	/* Sort elements by column number within each row. */

	for (i=0; i<nr; i++)
	  {
	    n = ia[i+1] - ia[i];
	    if (n>1)
	      {
		p = ja + ia[i] - 1;
		isort_vector_integer (n, c, p);
	    
		/* Deal with duplicates.  Mark column as zero, delete later. */

		for (j=1; j<n; j++)
		  if (c[p[j]-1] == c[p[j-1]-1]) c[p[j-1]-1] = 0;
	      }
	  }

	/* Fill out sorted values and column numbers. */

	switch (values->type)
	  {
	  case integer:
	    {
	      int w = 0;
	      int *v = (int *) MALLOC (ne * type_size[integer]);
	      for (i=0; i<ne; i++)
		{
		  j = ja[i] - 1;
		  if (c[j] == 0)
		    w += values->a.integer[j];
		  else
		    {
		      v[i] = w + values->a.integer[j];
		      w = 0;
		    }
		  ja[i] = c[j];
		}
	      m = (MATRIX *) form_matrix (nr, nc, integer, sparse);
	      m->ia = ia;
	      m->ja = ja;
	      m->a.integer = v;
	      m->nn = ne;
	    }
	    break;
	  case real:
	    {
	      REAL w = 0.0;
	      REAL *v = (REAL *) MALLOC (ne * type_size[real]);
	      for (i=0; i<ne; i++)
		{
		  j = ja[i] - 1;
		  if (c[j] == 0)
		    w += values->a.real[j];
		  else
		    {
		      v[i] = w + values->a.real[j];
		      w = 0;
		    }
		  ja[i] = c[j];
		}
	      m = (MATRIX *) form_matrix (nr, nc, real, sparse);
	      m->ia = ia;
	      m->ja = ja;
	      m->a.real = v;
	      m->nn = ne;
	    }
	    break;
	  case complex:
	    {
	      COMPLEX w;
	      COMPLEX *v = (COMPLEX *) MALLOC (ne * type_size[complex]);
	      w.real = w.imag = 0.0;
	      for (i=0; i<ne; i++)
		{
		  j = ja[i] - 1;
		  if (c[j] == 0)
		    {
		      w.real += values->a.complex[j].real;
		      w.imag += values->a.complex[j].imag;
		    }
		  else
		    {
		      v[i].real = w.real + values->a.complex[j].real;
		      v[i].imag = w.imag + values->a.complex[j].imag;
		      w.real = w.imag = 0.0;
		    }
		  ja[i] = c[j];
		}
	      m = (MATRIX *) form_matrix (nr, nc, complex, sparse);
	      m->ia = ia;
	      m->ja = ja;
	      m->a.complex = v;
	      m->nn = ne;
	    }
	    break;
	  default:
	    wipeout ("Bad type.");
	  }

	ia = ja = NULL;
	m = (MATRIX *) squeeze_matrix (EAT (m));
      }
    else
      m = (MATRIX *) make_matrix (nr, nc, values->type, sparse);

  }
  ON_EXCEPTION
  {
    delete_entity (t);
    delete_3_vectors (shape, rows, cols);
    delete_vector (values);
    delete_matrix (m);
    TFREE (ia);
    TFREE (ja);
    TFREE (ip);
  }
  END_EXCEPTION;

  delete_3_vectors (shape, rows, cols);
  delete_vector (values);

  return ENT (m);
}

ENTITY *
bi_exsparse (ENTITY *x)
{
  /*
   * This function disassembles a sparse matrix, returning it in its
   * "coordinate" form, suitable for reassembly by the `mksparse' function.
   */

  int ne, i, j;

  VECTOR *shape = NULL;
  VECTOR *rows = NULL;
  VECTOR *cols = NULL;
  VECTOR *values = NULL;

  MATRIX *m = NULL;
  TABLE *t = NULL;

  WITH_HANDLING
  {
    m = (MATRIX *) matrix_entity (EAT (x));
    if (m->density == dense) m = (MATRIX *) sparse_matrix (EAT (m));
    m = (MATRIX *) full_matrix (EAT (m));

    if (m->type == character)
      {
	fail ("Invalid type in exsparse.");
	raise_exception ();
      }

    ne = m->nn;

    t = (TABLE *) make_table ();
    shape = (VECTOR *) make_vector (2, integer, dense);
    rows = (VECTOR *) make_vector (ne, integer, dense);
    cols = (VECTOR *) make_vector (ne, integer, dense);
    values = (VECTOR *) make_vector (ne, m->type, dense);

    shape->a.integer[0] = m->nr;
    shape->a.integer[1] = m->nc;

    for (i=0; i<m->nr; i++)
      for (j=m->ia[i]; j<m->ia[i+1]; j++)
	rows->a.integer[j-1] = i+1;

    memcpy (cols->a.integer, m->ja, ne*sizeof(int));
    memcpy (values->a.ptr, m->a.ptr, ne*type_size[m->type]);

    t = (TABLE *) replace_in_table ((TABLE *) EAT (t),
				    EAT (shape),
				    dup_char (mksparse_specs[MKS_SHAPE]));
    t = (TABLE *) replace_in_table ((TABLE *) EAT (t),
				    EAT (rows),
				    dup_char (mksparse_specs[MKS_ROWS]));
    t = (TABLE *) replace_in_table ((TABLE *) EAT (t),
				    EAT (cols),
				    dup_char (mksparse_specs[MKS_COLS]));
    t = (TABLE *) replace_in_table ((TABLE *) EAT (t),
				    EAT (values),
				    dup_char (mksparse_specs[MKS_VALUES]));
  }
  ON_EXCEPTION
  {
    delete_entity (x);
    delete_table (t);
    delete_3_vectors (shape, rows, cols);
    delete_vector (values);
    delete_matrix (m);
  }
  END_EXCEPTION;

  delete_matrix (m);

  return ENT (t);
}

ENTITY *
triu_entity (int i, ENTITY *p, ENTITY *s)
{
  /*
   * This function returns the upper triangular part of the matrix `p'.
   * The scalar `s' determines the cutoff diagonal.  If `s' is zero,
   * the main diagonal and above are included.  If `s' is positive, the
   * cutoff diagonal is moved up by that many diagonals.  If `s' is
   * negative, the cutoff moves down.
   */

  int n;

  EASSERT (p, 0, 0);

  WITH_HANDLING
    {
      p = matrix_entity (EAT (p));

      if (s)
	{
	  s = scalar_entity (EAT (s));
	  n = ((SCALAR *)s)->v.integer;
	}
      else
	n = 0;
    }
  ON_EXCEPTION
    {
      delete_2_entities (p, s);
    }
  END_EXCEPTION;

  delete_entity (s);
  return triu_matrix ((MATRIX *)p, n);
}

ENTITY *
triu_matrix (MATRIX *p, int n)
{
  int i, j, k, rs, ce, nr, nc;

  EASSERT (p, matrix, 0);

  /* If n is sufficently negative, no change needed. */

  if (n < 2 - p->nr)
    return ENT (p);

  /* OK, changes needed. */

  p = (MATRIX *) dup_matrix (EAT (p));
  nr = p->nr;
  nc = p->nc;

  /* Figure out starting row and ending column to zero. */

  rs = (n > 0) ? 1 : 2 - n;
  ce = (n > nc-nr) ? nc : nr + n - 1;

  switch (p->density)
    {
    case dense:

      switch (p->type)
	{
	case integer:

	  for (j=0; j<n-1 && j<ce; j++)
	    for (i=0; i<nr; i++)
	      p->a.integer[j*nr+i] = 0;
	  for (; j<ce; j++)
	    for (i=rs++-1; i<nr; i++)
	      p->a.integer[j*nr+i] = 0;
	  break;

	case real:

	  for (j=0; j<n-1 && j<ce; j++)
	    for (i=0; i<nr; i++)
	      p->a.real[j*nr+i] = 0.0;
	  for (; j<ce; j++)
	    for (i=rs++-1; i<nr; i++)
	      p->a.real[j*nr+i] = 0.0;
	  break;

	case complex:

	  for (j=0; j<n-1 && j<ce; j++)
	    for (i=0; i<nr; i++)
	      {
		k = j*nr+i;
		p->a.complex[k].real = 0.0;
		p->a.complex[k].imag = 0.0;
	      }
	  for (; j<ce; j++)
	    for (i=rs++-1; i<nr; i++)
	      {
		k = j*nr+i;
		p->a.complex[k].real = 0.0;
		p->a.complex[k].imag = 0.0;
	      }
	  break;

	case character:

	  for (j=0; j<n-1 && j<ce; j++)
	    for (i=0; i<nr; i++)
	      {
		k = j*nr+i;
		TFREE (p->a.character[k]);
		p->a.character[k] = NULL;
	      }
	  for (; j<ce; j++)
	    for (i=rs++-1; i<nr; i++)
	      {
		k = j*nr+i;
		TFREE (p->a.character[k]);
		p->a.character[k] = NULL;
	      }

	default:
	  BAD_TYPE (p->type);
	  delete_matrix (p);
	  raise_exception ();
	}

      break;

    case sparse_upper:

      if (n >= 1)
	{
	  /* Take what we have; fall through if necessary. */

	  p->density = sparse;
	  TFREE (p->d.ptr); p->d.ptr = NULL;

	  if (n == 1) break;
	}

      else
	{
	  /* Convert to full, then fall through. */

	  p = (MATRIX *) full_matrix (p);
	}

    case sparse:

      assert (p->density == sparse);

      /* We'll zero the elements first, then compress them out. */

      if (p->nn)
	{
	  switch (p->type)
	    {
	    case integer:

	      for (i=rs-1; i<nr; i++)
		for (k=p->ia[i]; k<p->ia[i+1]; k++)
		  if (p->ja[k-1] <= i + n)
		    p->a.integer[k-1] = 0;
	      break;

	    case real:

	      for (i=rs-1; i<nr; i++)
		for (k=p->ia[i]; k<p->ia[i+1]; k++)
		  if (p->ja[k-1] <= i + n)
		    p->a.real[k-1] = 0.0;
	      break;

	    case complex:

	      for (i=rs-1; i<nr; i++)
		for (k=p->ia[i]; k<p->ia[i+1]; k++)
		  if (p->ja[k-1] <= i + n)
		    {
		      p->a.complex[k-1].real = 0.0;
		      p->a.complex[k-1].imag = 0.0;
		    }
	      break;

	    case character:

	      for (i=rs-1; i<nr; i++)
		for (k=p->ia[i]; k<p->ia[i+1]; k++)
		  if (p->ja[k-1] <= i + n)
		    {
		      TFREE (p->a.character[k-1]);
		      p->a.character[k-1] = NULL;
		    }
	      break;

	    default:
	      BAD_TYPE (p->type);
	      delete_matrix (p);
	      raise_exception ();
	    }

	  if (nr == nc && (!p->nn || n >= nc))
	    p->symmetry = (p->type == complex) ? hermitian : symmetric;
	  else
	    p->symmetry = general;

	  p = (MATRIX *) sparse_matrix (p);
	}
      break;

    default:
      BAD_DENSITY (p->density);
      delete_matrix (p);
      raise_exception ();
    }

  /* Get the correct symmetry. */

  if (nr == nc && (!p->nn || n >= nc))
    p->symmetry = (p->type == complex) ? hermitian : symmetric;
  else
    p->symmetry = general;

  return ENT (p);
}

ENTITY *
tril_entity (int i, ENTITY *p, ENTITY *s)
{
  /*
   * This function returns the lower triangular part of the matrix `p'.
   * The scalar `s' determines the cutoff diagonal.  If `s' is zero,
   * the main diagonal and below are included.  If `s' is positive, the
   * cutoff diagonal is moved up by that many diagonals.  If `s' is
   * negative, the cutoff moves down.
   */

  int n;

  EASSERT (p, 0, 0);

  WITH_HANDLING
    {
      p = matrix_entity (EAT (p));

      if (s)
	{
	  s = scalar_entity (EAT (s));
	  n = ((SCALAR *)s)->v.integer;
	}
      else
	n = 0;
    }
  ON_EXCEPTION
    {
      delete_2_entities (p, s);
    }
  END_EXCEPTION;

  delete_entity (s);
  return tril_matrix ((MATRIX *)p, n);
}

ENTITY *
tril_matrix (MATRIX *p, int n)
{
  int i, j, k, re, cs, nr, nc;

  EASSERT (p, matrix, 0);

  /* If n is sufficently positive, no change needed. */

  if (n > p->nc - 2)
    return ENT (p);

  /* OK, changes needed. */

  p = (MATRIX *) dup_matrix (EAT (p));
  nr = p->nr;
  nc = p->nc;

  /* Figure out starting column and ending row to zero. */

  re = (n < nc-nr) ? nr : nc - n - 1;
  cs = (n < 0) ? 1 : n + 2;

  switch (p->density)
    {
    case dense:

      switch (p->type)
	{
	case integer:

	  for (j=cs-1; j<nc; j++)
	    for (i=0; i<j-n && i<nr; i++)
	      p->a.integer[j*nr+i] = 0;
	  break;

	case real:

	  for (j=cs-1; j<nc; j++)
	    for (i=0; i<j-n && i<nr; i++)
	      p->a.real[j*nr+i] = 0;
	  break;

	case complex:

	  for (j=cs-1; j<nc; j++)
	    for (i=0; i<j-n && i<nr; i++)
	      {
		k = j*nr+i;
		p->a.complex[k].real = 0.0;
		p->a.complex[k].imag = 0.0;
	      }
	  break;

	case character:

	  for (j=cs-1; j<nc; j++)
	    for (i=0; i<j-n && i<nr; i++)
	      {
		k = j*nr+i;
		TFREE (p->a.character[k]);
		p->a.character[k] = NULL;
	      }

	default:
	  BAD_TYPE (p->type);
	  delete_matrix (p);
	  raise_exception ();
	}

      break;

    case sparse_upper:

      /* Convert to full, then fall through. */

      p = (MATRIX *) full_matrix (p);

    case sparse:

      assert (p->density == sparse);

      /* We'll zero the elements first, then compress them out. */

      if (p->nn)
	{
	  switch (p->type)
	    {
	    case integer:

	      for (i=0; i<nr; i++)
		for (k=p->ia[i]; k<p->ia[i+1]; k++)
		  if (p->ja[k-1] > i + n + 1)
		    p->a.integer[k-1] = 0;
	      break;

	    case real:

	      for (i=0; i<nr; i++)
		for (k=p->ia[i]; k<p->ia[i+1]; k++)
		  if (p->ja[k-1] > i + n + 1)
		    p->a.real[k-1] = 0.0;
	      break;

	    case complex:

	      for (i=0; i<nr; i++)
		for (k=p->ia[i]; k<p->ia[i+1]; k++)
		  if (p->ja[k-1] > i + n + 1)
		    {
		      p->a.complex[k-1].real = 0.0;
		      p->a.complex[k-1].imag = 0.0;
		    }
	      break;

	    case character:

	      for (i=0; i<nr; i++)
		for (k=p->ia[i]; k<p->ia[i+1]; k++)
		  if (p->ja[k-1] > i + n + 1)
		    {
		      TFREE (p->a.character[k-1]);
		      p->a.character[k-1] = NULL;
		    }
	      break;

	    default:
	      BAD_TYPE (p->type);
	      delete_matrix (p);
	      raise_exception ();
	    }

	  if (nr == nc && (!p->nn || n <= -nr))
	    p->symmetry = (p->type == complex) ? hermitian : symmetric;
	  else
	    p->symmetry = general;

	  p = (MATRIX *) sparse_matrix (p);
	}
      break;

    default:
      BAD_DENSITY (p->density);
      delete_matrix (p);
      raise_exception ();
    }

  /* Get the correct symmetry. */

  if (nr == nc && (!p->nn || n <= -nr))
    p->symmetry = (p->type == complex) ? hermitian : symmetric;
  else
    p->symmetry = general;

  return ENT (p);
}
