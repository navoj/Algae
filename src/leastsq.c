/*
 * leastsq.c -- Solve overdetermined and underdetermined systems.
 *
 * Copyright (C) 1996-2003  K. Scott Hunziker.
 *
 * See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: leastsq.c,v 1.3 2003/09/30 05:01:30 ksh Exp $";

#include "leastsq.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "lapack.h"
#include "cast.h"
#include "dense.h"
#include "sparse.h"
#include "transpose.h"

ENTITY *
bi_leastsq (ENTITY *a, ENTITY *b)
{
  /*
   * This function solves full-rank overdetermined and underdetermined
   * linear systems using a QR or LQ factorization.  The inputs are the
   * coefficient array `A' and the RHS `B'.  The return value is the
   * solution array `X'.
   *
   * An underdetermined system is one for which `A' has fewer rows than
   * columns.  The rank of `A' must be equal to the number of rows of
   * `A'.  The return value is the minimum norm solution.
   *
   * An overdetermined system is one for which `A' has more rows than
   * columns.  This function treats a square `A' as if it were an
   * overdetermined system.  The rank of `A' must be equal to the number
   * of columns of `A'.  The return value is the solution to the least
   * squares problem
   *			minimize || B - A*X ||
   */

  CLASS b_class = b->class;
  ENTITY * volatile x = NULL;

  EASSERT (a, 0, 0);
  EASSERT (b, 0, 0);

  WITH_HANDLING
    {
      a = matrix_entity (EAT (a));

      if (b_class == vector)
        b = transpose_matrix ((MATRIX *) vector_to_matrix (EAT (b)));
      else
        b = matrix_entity (EAT (b));

      x = leastsq_matrix (EAT (a), EAT (b));

      if (b_class == vector)
        x = matrix_to_vector ((MATRIX *) x);
    }
  ON_EXCEPTION
    {
      delete_3_entities (a, b, x);
    }
  END_EXCEPTION;

  delete_2_entities (a, b);
  return x;
}

ENTITY *
leastsq_matrix (MATRIX *a, MATRIX *b)
{
  int over, itrans, lwork, info;
  REAL ws;
  MATRIX * volatile x = NULL;
  MATRIX *bx = NULL;
  ENTITY * volatile rid = NULL;
  ENTITY * volatile cid = NULL;
  void * volatile work = NULL;

  EASSERT (a, matrix, 0);
  EASSERT (b, matrix, 0);

  WITH_HANDLING
    {
      /* Check the arguments. */

      over = a->nr >= a->nc;

      if (a->nr != b->nr)
        {
          fail ("The dimensions don't match.");
          raise_exception ();
        }

      if (a->type == character || b->type == character)
        {
          fail ("Invalid character type for the least squares problem.");
          raise_exception ();
        }

      if (!MATCH_VECTORS (a->rid, b->rid))
        {
          fail ("Labels don't match.");
          raise_exception ();
        }

      if (a->cid) rid = copy_entity (a->cid);
      if (b->cid) cid = copy_entity (b->cid);

      /* Convert integer to real. */

      if (a->type == integer)
        a = (MATRIX *) cast_matrix_integer_real ((MATRIX *) EAT (a));
      if (b->type == integer)
        b = (MATRIX *) cast_matrix_integer_real ((MATRIX *) EAT (b));
      AUTO_CAST_MATRIX (a, b);

      /* For now, we have no sparse solvers, so convert to dense. */

      a = (MATRIX *) dense_matrix (EAT (a));

      /* OK, solve it. */

      switch (a->density)
        {
        case dense:

          b = (MATRIX *) dense_matrix ((MATRIX *) EAT (b));
          a = (MATRIX *) dup_matrix ((MATRIX *) EAT (a));

          switch (a->type)
            {

            case real:

              if (over)
                {
                  bx = (MATRIX *) dup_matrix ((MATRIX *) EAT (b));
                }
              else
                {
                  int i, j, m, n;
                  m = a->nr;
                  n = a->nc;
                  bx = (MATRIX *) form_matrix (n, b->nc, a->type, dense);
                  for (j=0; j<b->nc; j++)
                    for (i=0; i<m; i++)
                      bx->a.real[j*n+i] = b->a.real[j*m+i];
                  delete_matrix (b);
                  b = NULL;
                }

              itrans = 'N';

              /* Call once to get the best work size. */

              lwork = -1;

              RGELS (&itrans, &a->nr, &a->nc, &bx->nc, a->a.real, &a->nr,
                     bx->a.real, &bx->nr, &ws, &lwork, &info);

              lwork = (int) ws;
              work = E_MALLOC (lwork, real);

              SIGINT_RAISE_ON ();
              RGELS (&itrans, &a->nr, &a->nc, &bx->nc, a->a.real, &a->nr,
                     bx->a.real, &bx->nr, work, &lwork, &info);
              SIGINT_RAISE_OFF ();

              FREE (work);
              work = NULL;

              if (info)
                {
                  fail ("Illegal value in %d%s argument to DGELS.  Please report this bug.",
                        -info, TH (-info));
                  raise_exception ();
                }

              if (over)
                {
                  int i, j, m, n;
                  m = a->nr;
                  n = a->nc;
                  x = (MATRIX *) form_matrix (n, bx->nc, a->type, dense);
                  for (j=0; j<bx->nc; j++)
                    for (i=0; i<n; i++)
                      x->a.real[j*n+i] = bx->a.real[j*m+i];
                  delete_matrix (bx);
                  bx = NULL;
                }
              else
                {
                  x = (MATRIX *) dup_matrix ((MATRIX *) EAT (bx));
                }

              break;
              
            case complex:

              if (over)
                {
                  bx = (MATRIX *) dup_matrix ((MATRIX *) EAT (b));
                }
              else
                {
                  int i, j, m, n;
                  m = a->nr;
                  n = a->nc;
                  bx = (MATRIX *) form_matrix (n, b->nc, a->type, dense);
                  for (j=0; j<b->nc; j++)
                    for (i=0; i<m; i++)
                      bx->a.complex[j*n+i] = b->a.complex[j*m+i];
                  delete_matrix (b);
                  b = NULL;
                }

              itrans = 'N';

              /* Call once to get the best work size. */

              lwork = -1;

              XGELS (&itrans, &a->nr, &a->nc, &bx->nc, a->a.complex, &a->nr,
                     bx->a.complex, &bx->nr, &ws, &lwork, &info);

              lwork = (int) ws;
              work = E_MALLOC (lwork, complex);

              SIGINT_RAISE_ON ();
              XGELS (&itrans, &a->nr, &a->nc, &bx->nc, a->a.complex, &a->nr,
                     bx->a.complex, &bx->nr, work, &lwork, &info);
              SIGINT_RAISE_OFF ();

              FREE (work);
              work = NULL;

              if (info)
                {
                  fail ("Illegal value in %d%s argument to DGELS.  Please report this bug.",
                        -info, TH (-info));
                  raise_exception ();
                }

              if (over)
                {
                  int i, j, m, n;
                  m = a->nr;
                  n = a->nc;
                  x = (MATRIX *) form_matrix (n, bx->nc, a->type, dense);
                  for (j=0; j<bx->nc; j++)
                    for (i=0; i<n; i++)
                      x->a.complex[j*n+i] = bx->a.complex[j*m+i];
                  delete_matrix (bx);
                  bx = NULL;
                }
              else
                {
                  x = (MATRIX *) dup_matrix ((MATRIX *) EAT (bx));
                }

              break;

            default:
              BAD_TYPE (a->type);
              raise_exception ();
            }
          break;

        case sparse:
        case sparse_upper:

          detour ("Can't yet do %s least squares.",
                  density_string[a->density]);
          raise_exception ();

        default:

          BAD_DENSITY (a->density);
          raise_exception ();
        }
    }
  ON_EXCEPTION
    {
      if (work != NULL) FREE (work);
      delete_2_matrices (a, b);
      delete_2_matrices (x, bx);
      delete_2_entities (rid, cid);
    }
  END_EXCEPTION;

  assert (!x->rid);
  assert (!x->cid);
  x->rid = rid;
  x->cid = cid;

  delete_3_matrices (a, b, bx);

  return ENT (x);
}
