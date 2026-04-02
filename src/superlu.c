/*
   superlu.c -- interface to SuperLU package for solving sparse systems

   Copyright (C) 2002-2003  K. Scott Hunziker.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: superlu.c,v 1.3 2003/10/05 01:14:48 ksh Exp $";

#include "superlu.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "transpose.h"
#include "lapack.h"
#include "dense.h"
#include "cast.h"

static char *Stype_names[] =
{
  "SLU_NC",
  "SLU_NR",
  "SLU_SC",
  "SLU_SR",
  "SLU_NCP",
  "SLU_DN",
  NULL,
};

static char *Dtype_names[] =
{
  "SLU_S",
  "SLU_D",
  "SLU_C",
  "SLU_Z",
  NULL,
};

static char *Mtype_names[] =
{
  "SLU_GE",
  "SLU_TRLU",
  "SLU_TRUU",
  "SLU_TRL",
  "SLU_TRU",
  "SLU_SYL",
  "SLU_SYU",
  "SLU_HEL",
  "SLU_HEU",
  NULL,
};

/* Replace SuperLU's abort routine. */

void abort_and_exit (char *msg)
{
    fail (msg);
    raise_exception ();
}

#define RIT( t, v, n )	t = (TABLE *) replace_in_table (EAT(t), v, dup_char(n))
#define CS( c )		char_to_scalar (dup_char (c))
#define GV( n, t, v)	gift_wrap_vector (n, t, dup_mem (v, (n)*type_size[t]))


/* Convert a SuperMatrix to a table. */

ENTITY *
SuperLU_to_table (SuperMatrix *A)
{
  TABLE *t = NULL;

  WITH_HANDLING
  {
    t = (TABLE *) make_table ();
    RIT (t, CS (Stype_names[A->Stype]), "Stype");
    RIT (t, CS (Dtype_names[A->Dtype]), "Dtype");
    RIT (t, CS (Mtype_names[A->Mtype]), "Mtype");
    RIT (t, int_to_scalar (A->nrow), "nrow");
    RIT (t, int_to_scalar (A->ncol), "ncol");

    assert (A->Dtype == SLU_D || A->Dtype == SLU_Z);

    switch (A->Stype)
      {
      case SLU_NC:

	RIT (t, int_to_scalar (((NCformat *)(A->Store))->nnz), "nnz");
	RIT (t, GV (((NCformat *)(A->Store))->nnz,
				  (A->Dtype == SLU_D) ? real : complex,
				  ((NCformat *)(A->Store))->nzval), "nzval");
	RIT (t, GV (((NCformat *)(A->Store))->nnz,
				  integer,
				  ((NCformat *)(A->Store))->rowind), "rowind");
	RIT (t, GV (A->ncol+1, integer,
				  ((NCformat *)(A->Store))->colptr), "colptr");
	break;

      case SLU_NR:

	RIT (t, int_to_scalar (((NRformat *)(A->Store))->nnz), "nnz");
	RIT (t, GV (((NRformat *)(A->Store))->nnz,
				  (A->Dtype == SLU_D) ? real : complex,
				  ((NRformat *)(A->Store))->nzval), "nzval");
	RIT (t, GV (((NCformat *)(A->Store))->nnz, integer,
				  ((NRformat *)(A->Store))->colind), "colind");
	RIT (t, GV (A->nrow+1, integer,
				  ((NRformat *)(A->Store))->rowptr), "rowptr");
	break;

      case SLU_SC:

	RIT (t, int_to_scalar (((SCformat *)(A->Store))->nnz), "nnz");
	RIT (t, int_to_scalar (((SCformat *)(A->Store))->nsuper), "nsuper");
	RIT (t, GV (((SCformat *)(A->Store))->nzval_colptr[A->ncol],
				  (A->Dtype == SLU_D) ? real : complex,
				  ((SCformat *)(A->Store))->nzval), "nzval");
	RIT (t, GV (A->ncol+1, integer,
				  ((SCformat *)(A->Store))->nzval_colptr),
	     "nzval_colptr");
	RIT (t, GV (((SCformat *)(A->Store))->rowind_colptr[A->ncol], integer,
				  ((SCformat *)(A->Store))->rowind), "rowind");
	RIT (t, GV (A->ncol+1, integer,
				  ((SCformat *)(A->Store))->rowind_colptr),
	     "rowind_colptr");
	RIT (t, GV (A->ncol, integer,
				  ((SCformat *)(A->Store))->col_to_sup),
	     "col_to_sup");
	RIT (t, GV (((SCformat *)(A->Store))->nsuper+2, integer,
				  ((SCformat *)(A->Store))->sup_to_col),
	     "sup_to_col");
	break;

      default:

	fail ("Can't convert SuperLU type %d to table.", Stype_names[A->Stype]);
	raise_exception ();
      }
  }
  ON_EXCEPTION
  {
    delete_table (t);
  }
  END_EXCEPTION;

  return ENT (t);
}

/* Convert a table to a SuperMatrix.  Returns NULL on error. */

#define ST( t, n )	search_in_table ((TABLE *) copy_table (t), \
				dup_char (n))
#define RQ( e )		if (!(e)) { fail (invalid_msg); raise_exception (); }

static char *invalid_msg = "Invalid SuperLU matrix.";

static int
locate_string (char *u[], char *s)
{
  int i;

  for (i=0; *u[i] && strcmp (u[i], s); i++);

  return i;
}

SuperMatrix *
table_to_SuperLU (TABLE *t)
{
  ENTITY *e = NULL;
  SuperMatrix *A = MALLOC (sizeof (SuperMatrix));

  A->Store = NULL;

  WITH_HANDLING
  {
    RQ (e = ST (t, "Stype"));
    e = cast_scalar ((SCALAR *) scalar_entity (EAT (e)), character);
    A->Stype = locate_string (Stype_names, ((SCALAR *)e)->v.character);
    delete_entity (e); e = NULL;

    RQ (e = ST (t, "Dtype"));
    e = cast_scalar ((SCALAR *) scalar_entity (EAT (e)), character);
    A->Dtype = locate_string (Dtype_names, ((SCALAR *)e)->v.character);
    delete_entity (e); e = NULL;

    RQ (e = ST (t, "Mtype"));
    e = cast_scalar ((SCALAR *) scalar_entity (EAT (e)), character);
    A->Mtype = locate_string (Mtype_names, ((SCALAR *)e)->v.character);
    delete_entity (e); e = NULL;

    RQ (e = ST (t, "nrow"));
    e = cast_scalar ((SCALAR *) scalar_entity (EAT (e)), integer);
    A->nrow = ((SCALAR *)e)->v.integer;
    delete_entity (e); e = NULL;

    RQ (e = ST (t, "ncol"));
    e = cast_scalar ((SCALAR *) scalar_entity (EAT (e)), integer);
    A->ncol = ((SCALAR *)e)->v.integer;
    delete_entity (e); e = NULL;

    RQ (A->Dtype == SLU_D || A->Dtype == SLU_Z);
    RQ (A->nrow > 0 && A->ncol > 0);

    switch (A->Stype)
      {
      case SLU_NC:

	{
	  NCformat *Store = MALLOC (sizeof (NCformat));
	  A->Store = (void *) Store;

	  RQ (e = ST (t, "nnz"));
	  e = cast_scalar ((SCALAR *) scalar_entity (EAT (e)), integer);
	  Store->nnz = ((SCALAR *)e)->v.integer;
	  delete_entity (e); e = NULL;

	  RQ (e = ST (t, "nzval"));
	  if (A->Dtype == SLU_D)
	    {
	      e = cast_vector ((VECTOR *) dense_vector (
		       (VECTOR *) vector_entity (EAT (e))), real);
	      Store->nzval = dup_mem (((VECTOR *)e)->a.real,
				      ((VECTOR *)e)->ne*sizeof(REAL));
	    }
	  else
	    {
	      e = cast_vector ((VECTOR *) dense_vector (
		       (VECTOR *) vector_entity (EAT(e))), complex);
	      Store->nzval = dup_mem (((VECTOR *)e)->a.complex,
				      ((VECTOR *)e)->ne*sizeof(COMPLEX));
	    }
	  RQ (((VECTOR *)e)->ne == Store->nnz);
	  delete_entity (e); e = NULL;

	  RQ (e = ST (t, "rowind"));
	  e = cast_vector ((VECTOR *) dense_vector (
		       (VECTOR *) vector_entity (EAT (e))), integer);
	  Store->rowind = dup_mem (((VECTOR *)e)->a.integer,
				   ((VECTOR *)e)->ne*sizeof(int));
	  RQ (((VECTOR *)e)->ne == Store->nnz);
	  delete_entity (e); e = NULL;

	  RQ (e = ST (t, "colptr"));
	  e = cast_vector ((VECTOR *) dense_vector (
                       (VECTOR *) vector_entity (EAT (e))), integer);
	  Store->colptr = dup_mem (((VECTOR *)e)->a.integer,
				   ((VECTOR *)e)->ne*sizeof(int));
	  RQ (((VECTOR *)e)->ne == A->ncol+1);
	  delete_entity (e); e = NULL;
	}
	break;

      case SLU_NR:

	{
	  NRformat *Store = MALLOC (sizeof (NRformat));
	  A->Store = (void *) Store;

	  RQ (e = ST (t, "nnz"));
	  e = cast_scalar ((SCALAR *) scalar_entity (EAT (e)), integer);
	  Store->nnz = ((SCALAR *)e)->v.integer;
	  delete_entity (e); e = NULL;

	  RQ (e = ST (t, "nzval"));
	  if (A->Dtype == SLU_D)
	    {
	      e = cast_vector ((VECTOR *) dense_vector (
                           (VECTOR *) vector_entity (EAT (e))), real);
	      Store->nzval = dup_mem (((VECTOR *)e)->a.real,
				      ((VECTOR *)e)->ne*sizeof(REAL));
	    }
	  else
	    {
	      e = cast_vector ((VECTOR *) dense_vector (
                           (VECTOR *) vector_entity (EAT(e))), complex);
	      Store->nzval = dup_mem (((VECTOR *)e)->a.complex,
				      ((VECTOR *)e)->ne*sizeof(COMPLEX));
	    }
	  RQ (((VECTOR *)e)->ne == Store->nnz);
	  delete_entity (e); e = NULL;

	  RQ (e = ST (t, "colind"));
	  e = cast_vector ((VECTOR *) dense_vector (
                       (VECTOR *) vector_entity (EAT (e))), integer);
	  Store->colind = dup_mem (((VECTOR *)e)->a.integer,
				   ((VECTOR *)e)->ne*sizeof(int));
	  RQ (((VECTOR *)e)->ne == Store->nnz);
	  delete_entity (e); e = NULL;

	  RQ (e = ST (t, "rowptr"));
	  e = cast_vector ((VECTOR *) dense_vector (
                       (VECTOR *) vector_entity (EAT (e))), integer);
	  Store->rowptr = dup_mem (((VECTOR *)e)->a.integer,
				   ((VECTOR *)e)->ne*sizeof(int));
	  RQ (((VECTOR *)e)->ne == A->nrow+1);
	  delete_entity (e); e = NULL;
	}
	break;

      case SLU_SC:

	{
	  SCformat *Store = MALLOC (sizeof (SCformat));
	  A->Store = (void *) Store;

	  RQ (e = ST (t, "nnz"));
	  e = cast_scalar ((SCALAR *) scalar_entity (EAT (e)), integer);
	  Store->nnz = ((SCALAR *)e)->v.integer;
	  delete_entity (e); e = NULL;

	  RQ (e = ST (t, "nsuper"));
	  e = cast_scalar ((SCALAR *) scalar_entity (EAT (e)), integer);
	  Store->nsuper = ((SCALAR *)e)->v.integer;
	  delete_entity (e); e = NULL;

	  RQ (e = ST (t, "nzval_colptr"));
	  e = cast_vector ((VECTOR *) dense_vector (
                       (VECTOR *) vector_entity (EAT (e))), integer);
	  Store->nzval_colptr = dup_mem (((VECTOR *)e)->a.integer,
					 ((VECTOR *)e)->ne*sizeof(int));
	  RQ (((VECTOR *)e)->ne == A->ncol+1);
	  delete_entity (e); e = NULL;

	  RQ (e = ST (t, "nzval"));
	  if (A->Dtype == SLU_D)
	    {
	      e = cast_vector ((VECTOR *) dense_vector (
                           (VECTOR *) vector_entity (EAT (e))), real);
	      Store->nzval = dup_mem (((VECTOR *)e)->a.real,
				      ((VECTOR *)e)->ne*sizeof(REAL));
	    }
	  else
	    {
	      e = cast_vector ((VECTOR *) dense_vector (
                           (VECTOR *) vector_entity (EAT(e))), complex);
	      Store->nzval = dup_mem (((VECTOR *)e)->a.complex,
				      ((VECTOR *)e)->ne*sizeof(COMPLEX));
	    }
	  RQ (((VECTOR *)e)->ne == Store->nzval_colptr[A->ncol]);
	  delete_entity (e); e = NULL;

	  RQ (e = ST (t, "rowind_colptr"));
	  e = cast_vector ((VECTOR *) dense_vector (
                       (VECTOR *) vector_entity (EAT (e))), integer);
	  Store->rowind_colptr = dup_mem (((VECTOR *)e)->a.integer,
					  ((VECTOR *)e)->ne*sizeof(int));
	  RQ (((VECTOR *)e)->ne == A->ncol+1);
	  delete_entity (e); e = NULL;

	  RQ (e = ST (t, "rowind"));
	  e = cast_vector ((VECTOR *) dense_vector (
                       (VECTOR *) vector_entity (EAT (e))), integer);
	  Store->rowind = dup_mem (((VECTOR *)e)->a.integer,
				   ((VECTOR *)e)->ne*sizeof(int));
	  RQ (((VECTOR *)e)->ne == Store->rowind_colptr[A->ncol]);
	  delete_entity (e); e = NULL;

	  RQ (e = ST (t, "col_to_sup"));
	  e = cast_vector ((VECTOR *) dense_vector (
                       (VECTOR *) vector_entity (EAT (e))), integer);
	  Store->col_to_sup = dup_mem (((VECTOR *)e)->a.integer,
				       ((VECTOR *)e)->ne*sizeof(int));
	  RQ (((VECTOR *)e)->ne == A->ncol);
	  delete_entity (e); e = NULL;

	  RQ (e = ST (t, "sup_to_col"));
	  e = cast_vector ((VECTOR *) dense_vector (
                       (VECTOR *) vector_entity (EAT (e))), integer);
	  Store->sup_to_col = dup_mem (((VECTOR *)e)->a.integer,
				       ((VECTOR *)e)->ne*sizeof(int));
	  RQ (((VECTOR *)e)->ne == Store->nsuper+2);
	  delete_entity (e); e = NULL;
	}
	break;

      default:

	RQ (0);
      }
  }
  ON_EXCEPTION
  {
    delete_table (t);
    delete_entity (e);
    TFREE (A->Store);
    TFREE (A);
  }
  END_EXCEPTION;

  delete_table (t);

  return A;
}

void
free_SuperLU (SuperMatrix *A)
{
  if (A)
    {
      if (A->Store)
	{
	  switch (A->Stype)
	    {
	    case SLU_NC:

	      TFREE (((NCformat *)(A->Store))->nzval);
	      TFREE (((NCformat *)(A->Store))->rowind);
	      TFREE (((NCformat *)(A->Store))->colptr);
	      break;

	    case SLU_NR:

	      TFREE (((NRformat *)(A->Store))->nzval);
	      TFREE (((NRformat *)(A->Store))->colind);
	      TFREE (((NRformat *)(A->Store))->rowptr);
	      break;

	    case SLU_SC:

	      TFREE (((SCformat *)(A->Store))->nzval);
	      TFREE (((SCformat *)(A->Store))->nzval_colptr);
	      TFREE (((SCformat *)(A->Store))->rowind);
	      TFREE (((SCformat *)(A->Store))->rowind_colptr);
	      TFREE (((SCformat *)(A->Store))->col_to_sup);
	      TFREE (((SCformat *)(A->Store))->sup_to_col);
	      break;

	    case SLU_NCP:

	      TFREE (((NCPformat *)(A->Store))->nzval);
	      TFREE (((NCPformat *)(A->Store))->rowind);
	      TFREE (((NCPformat *)(A->Store))->colbeg);
	      TFREE (((NCPformat *)(A->Store))->colend);
	      break;

	    default:

	      fail ("Invalid SuperLU type.");
	      raise_exception ();
	    }

	  FREE (A->Store);
	}
      FREE (A);
    }

  return;
}
