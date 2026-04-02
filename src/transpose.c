/*
   transpose.c -- Transpose a matrix.

   Copyright (C) 1994-2002  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: transpose.c,v 1.3 2003/09/01 18:45:51 ksh Exp $";

#include "transpose.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "conjugate.h"
#include "thin.h"
#include "full.h"

static char undef_msg[] = "Transpose is undefined for %ss.";

ENTITY *
transpose_entity (ENTITY *p)
{
  /* This function returns the ordinary transpose of a matrix. */

  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return scalar_to_matrix ((SCALAR *) p);

    case vector:
      return transpose_matrix ((MATRIX *) vector_to_matrix ((VECTOR *) p));

    case matrix:
      return (transpose_matrix ((MATRIX *) p));

    case table:
    case function:
      fail (undef_msg, class_string[p->class]);
      break;

    default:
      BAD_CLASS (p->class);
      break;
    }

  delete_entity (p);
  raise_exception ();
}

ENTITY *
transpose_matrix (MATRIX *m)
{
  ENTITY *id;
  MATRIX *p;
  int i, j;

  EASSERT (m, matrix, 0);

  switch (m->density)
    {
    case dense:

      /* It's worth it to check for the easy case. */

      if (m->nr == 1 || m->nc == 1)
	{
	  p = (MATRIX *) dup_matrix (EAT (m));

	  i = p->nr;
	  p->nr = p->nc;
	  p->nc = i;

	  id = p->rid;
	  p->rid = p->cid;
	  p->cid = id;
	}
      else
	{
	  switch (m->symmetry)
	    {
	    case general:
	    case hermitian:

	      p = (MATRIX *) form_matrix (m->nc, m->nr, m->type, m->density);
	      p->symmetry = m->symmetry;
	      if (m->cid != NULL)
		p->rid = copy_entity (m->cid);
	      if (m->rid != NULL)
		p->cid = copy_entity (m->rid);

	      switch (m->type)
		{
		case integer:
		  for (i = 0; i < p->nr; i++)
		    {
		      for (j = 0; j < p->nc; j++)
			{
			  p->a.integer[i + p->nr * j] =
			    m->a.integer[j + m->nr * i];
			}
		    }
		  break;
		case real:
		  for (i = 0; i < p->nr; i++)
		    {
		      for (j = 0; j < p->nc; j++)
			{
			  p->a.real[i + p->nr * j] = m->a.real[j + m->nr * i];
			}
		    }
		  break;
		case complex:
		  for (i = 0; i < p->nr; i++)
		    {
		      for (j = 0; j < p->nc; j++)
			{
			  p->a.complex[i + p->nr * j] =
			    m->a.complex[j + m->nr * i];
			}
		    }
		  break;
		case character:
		  for (i = 0; i < p->nr; i++)
		    {
		      for (j = 0; j < p->nc; j++)
			{
			  p->a.character[i + p->nr * j] =
			    dup_char (m->a.character[j + m->nr * i]);
			}
		    }
		  break;
		default:
		  BAD_TYPE (m->type);
		  delete_matrix (m);
		  raise_exception ();
		}
	      delete_matrix (m);
	      break;

	    case symmetric:

	      if (m->rid || m->cid)
		{
		  p = (MATRIX *) dup_matrix (m);
		  id = p->rid;
		  p->rid = p->cid;
		  p->cid = id;
		}
	      else
		p = m;
	      break;

	    default:
	      BAD_SYMMETRY (m->symmetry);
	      delete_matrix (m);
	      raise_exception ();
	    }
	}
      break;
	  
    case sparse:
      switch (m->symmetry)
	{
	case general:
	case hermitian:

	  p = (MATRIX *) make_matrix (m->nc, m->nr, m->type, m->density);
	  p->symmetry = m->symmetry;
	  if (m->cid != NULL)
	    p->rid = copy_entity (m->cid);
	  if (m->rid != NULL)
	    p->cid = copy_entity (m->rid);

	  if (m->nn > 0)
	    {
	      p->nn = m->nn;
	      p->a.ptr = E_MALLOC (p->nn, p->type);
	      p->ia = (int *) MALLOC ((p->nr + 1) * sizeof (int));
	      p->ja = (int *) MALLOC (p->nn * sizeof (int));

	      switch (p->type)
		{
		case integer:
		  IGSTRN (m->ia, m->ja, m->a.integer,
			  &m->nr, &m->nc,
			  p->ia, p->ja, p->a.integer);
		  break;
		case real:
		  DGSTRN (m->ia, m->ja, m->a.real,
			  &m->nr, &m->nc,
			  p->ia, p->ja, p->a.real);
		  break;
		case complex:
		  ZGSTRN (m->ia, m->ja, m->a.complex,
			  &m->nr, &m->nc,
			  p->ia, p->ja, p->a.complex);
		  break;
		case character:
		  assert (sizeof (int) == sizeof (char *));
		  IGSTRN (m->ia, m->ja, m->a.integer,
			  &m->nr, &m->nc,
			  p->ia, p->ja, p->a.integer);
		  for (i = 0; i < p->nn; i++)
		    p->a.character[i] = dup_char (p->a.character[i]);
		  break;
		default:
		  BAD_TYPE (p->type);
		  delete_2_matrices (p, m);
		  raise_exception ();
		}
	    }
	  p->order = ordered;
	  delete_matrix (m);
	  break;

	case symmetric:

	  if (m->rid || m->cid)
	    {
	      p = (MATRIX *) dup_matrix (m);
	      id = p->rid;
	      p->rid = p->cid;
	      p->cid = id;
	    }
	  else
	    p = m;
	  break;

	default:
	  BAD_SYMMETRY (m->symmetry);
	  delete_matrix (m);
	  raise_exception ();
	}
      break;
    case sparse_upper:
      switch (m->symmetry)
	{
	case general:
	  p = (MATRIX *) transpose_matrix ((MATRIX *) full_matrix (m));
	  break;

	case symmetric:

	  if (m->rid || m->cid)
	    {
	      p = (MATRIX *) dup_matrix (m);
	      id = p->rid;
	      p->rid = p->cid;
	      p->cid = id;
	    }
	  else
	    p = m;
	  break;

	case hermitian:

	  p = (MATRIX *) conjugate_matrix (m);
	  if (m->rid || m->cid)
	    {
	      p = (MATRIX *) dup_matrix (m);
	      id = p->rid;
	      p->rid = p->cid;
	      p->cid = id;
	    }
	    
	  break;

	default:
	  BAD_SYMMETRY (m->symmetry);
	  delete_matrix (m);
	  raise_exception ();
	}
      break;
    default:
      BAD_DENSITY (m->density);
      delete_matrix (m);
      raise_exception ();
    }
  return (ENT (p));
}

ENTITY *
hermitian_transpose_entity (ENTITY *p)
{
  /* This function returns the hermitian transpose of a matrix. */

  EASSERT (p, 0, 0);

  switch (p->class)
    {

    case scalar:
      return hermitian_transpose_matrix (
				(MATRIX *) scalar_to_matrix ((SCALAR *) p));

    case vector:
      return hermitian_transpose_matrix (
				(MATRIX *) vector_to_matrix ((VECTOR *) p));

    case matrix:
      return (hermitian_transpose_matrix ((MATRIX *) p));

    case table:
      fail (undef_msg, class_string[p->class]);
      break;

    default:
      BAD_CLASS (p->class);
      break;
    }

  delete_entity (p);
  raise_exception ();
}

ENTITY *
hermitian_transpose_matrix (MATRIX *m)
{
  EASSERT (m, matrix, 0);

  switch (m->type)
    {
    case integer:
    case real:
    case character:
      return (transpose_matrix (m));
    case complex:
      switch (m->symmetry)
	{
	case general:
	  return (conjugate_matrix ((MATRIX *) transpose_matrix (m)));
	case symmetric:
	case hermitian:
	  {
	    ENTITY *id;

	    m = (MATRIX *) conjugate_matrix (m);
	    if (m->rid || m->cid)
	      {
		m = (MATRIX *) dup_matrix (m);
		id = m->rid;
		m->rid = m->cid;
		m->cid = id;
	      }

	    return ENT (m);
	  }
	default:
	  BAD_SYMMETRY (m->symmetry);
	  delete_matrix (m);
	  raise_exception ();
	}
    default:
      BAD_TYPE (m->type);
      delete_matrix (m);
      raise_exception ();
    }
}
