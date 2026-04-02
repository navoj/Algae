/*
   diag.c -- The `diag' function.

   Copyright (C) 1994-2001  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: diag.c,v 1.5 2001/10/26 05:28:34 ksh Exp $";

#include "diag.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "dense.h"
#include "cast.h"

ENTITY *
bi_diag (ENTITY *p)
{
  EASSERT (p, 0, 0);

  switch (p->class)
    {
    case scalar:
      return (p);
    case vector:
      return (diag_vector ((VECTOR *) p));
    case matrix:
      return (diag_matrix ((MATRIX *) p));
    default:
      fail ("Can't apply \"diag\" to a %s entity.",
	    class_string[p->class]);
      delete_entity (p);
      raise_exception ();
    }
}

ENTITY *
diag_matrix (MATRIX *p)
{
  VECTOR *v;
  int i, k, nrp;

  EASSERT (p, matrix, 0);

  /*
   * The sparse code below depends on the vector being initialized
   * to zero.
   */

  v = (VECTOR *) make_vector ((p->nr < p->nc) ? p->nr : p->nc,
			      p->type, dense);

  if (v->ne == p->nc && p->cid != NULL)
    {
      v->eid = copy_entity (p->cid);
    }
  else if (v->ne == p->nr && p->rid != NULL)
    {
      v->eid = copy_entity (p->rid);
    }

  switch (p->density)
    {
    case dense:
      nrp = p->nr + 1;
      switch (p->type)
	{
	case integer:
	  for (i = 0; i < v->nn; i++)
	    v->a.integer[i] = p->a.integer[nrp * i];
	  break;
	case real:
	  for (i = 0; i < v->nn; i++)
	    v->a.real[i] = p->a.real[nrp * i];
	  break;
	case complex:
	  for (i = 0; i < v->nn; i++)
	    v->a.complex[i] = p->a.complex[nrp * i];
	  break;
	case character:
	  for (i = 0; i < v->nn; i++)
	    v->a.character[i] = dup_char (p->a.character[nrp * i]);
	  break;
	default:
	  BAD_TYPE (p->type);
	  delete_vector (v);
	  delete_matrix (p);
	  raise_exception ();
	}
      break;
    case sparse:
      if (p->nn)
        {
          switch (p->type)
            {
            case integer:
              for (i = 0; i < v->nn; i++)
                {
                  for (k = p->ia[i]; k < p->ia[i + 1]; k++)
                    {
                      if (p->ja[k - 1] == i + 1)
                        {
                          v->a.integer[i] = p->a.integer[k - 1];
                          break;
                        }
                    }
                }
              break;
            case real:
              for (i = 0; i < v->nn; i++)
                {
                  for (k = p->ia[i]; k < p->ia[i + 1]; k++)
                    {
                      if (p->ja[k - 1] == i + 1)
                        {
                          v->a.real[i] = p->a.real[k - 1];
                          break;
                        }
                    }
                }
              break;
            case complex:
              for (i = 0; i < v->nn; i++)
                {
                  for (k = p->ia[i]; k < p->ia[i + 1]; k++)
                    {
                      if (p->ja[k - 1] == i + 1)
                        {
                          v->a.complex[i] = p->a.complex[k - 1];
                          break;
                        }
                    }
                }
              break;

            case character:

              if (p->entity.ref_count == 1)
                {
                  for (i = 0; i < v->nn; i++)
                    for (k = p->ia[i]; k < p->ia[i + 1]; k++)
                      if (p->ja[k - 1] == i + 1)
                        {
                          v->a.character[i] = p->a.character[k - 1];
                          p->a.character[k - 1] = NULL_string;
                          break;
                        }
                }
              else
                {
                  for (i = 0; i < v->nn; i++)
                    for (k = p->ia[i]; k < p->ia[i + 1]; k++)
                      if (p->ja[k - 1] == i + 1)
                        {
                          v->a.character[i] =
                            dup_char (p->a.character[k - 1]);
                          break;
                        }
                }
              break;
            default:
              BAD_TYPE (p->type);
              delete_vector (v);
              delete_matrix (p);
              raise_exception ();
            }
        }
      break;

    case sparse_upper:

      if (p->d.ptr != NULL)
	{
	  if (p->type == character)
	    {
	      if (p->entity.ref_count == 1)
		for (i = 0; i < v->ne; i++)
		  {
		    v->a.character[i] = p->d.character[i];
		    p->d.character[i] = NULL_string;
		  }
	      else
		for (i = 0; i < v->ne; i++)
		  v->a.character[i] = dup_char (p->d.character[i]);
	    }
	  else
	    {
	      memcpy (v->a.ptr, p->d.ptr, v->nn * type_size[p->type]);
	    }
	}
      break;

    default:
      BAD_DENSITY (p->density);
      delete_vector (v);
      delete_matrix (p);
      raise_exception ();
    }

  /* If matrix was complex hermitian, result is real. */

  if (p->type == complex && p->symmetry == hermitian)
    v = (VECTOR *) cast_vector_complex_real ((VECTOR *) v);

  delete_matrix (p);
  return ENT (v);
}

ENTITY *
diag_vector (VECTOR *p)
{
  MATRIX *m;

  EASSERT (p, vector, 0);

  m = (MATRIX *) make_matrix (p->ne, p->ne, p->type, sparse_upper);
  if (p->eid != NULL)
    {
      m->rid = copy_entity (p->eid);
      m->cid = copy_entity (p->eid);
    }

  if (p->ne > 0)
    {
      m->d.ptr = E_MALLOC (m->nr, m->type);
      p = (VECTOR *) dense_vector (p);
      if (m->type == character)
	{
	  int i;

	  if (p->entity.ref_count == 1)
	    {
	      for (i = 0; i < p->ne; i++)
		{
		  m->d.character[i] = p->a.character[i];
		  p->a.character[i] = NULL_string;
		}
	    }
	  else
	    {
	      for (i = 0; i < p->ne; i++)
		m->d.character[i] = dup_char (p->a.character[i]);
	    }
	}
      else
	{
	  memcpy (m->d.ptr, p->a.ptr, m->nr * type_size[m->type]);
	}
      m->symmetry = symmetric;
    }

  delete_vector (p);
  return (apt_matrix (m));
}
