/*
   cram.c -- cram an array

   Copyright (C) 2001-03  K. Scott Hunziker.

   See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: cram.c,v 1.4 2003/08/01 04:57:47 ksh Exp $";

#include "cram.h"
#include "entity.h"
#include "datum.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "cast.h"
#include "transpose.h"
#include "dense.h"
#include "form.h"
#include "partition.h"
#include "test.h"
#include "full.h"

ENTITY *
bi_cram (ENTITY *shape, ENTITY *v)
{
  /*
   * This function returns an entity with the given `shape' that
   * contains the elements of `v'.  If `shape' is NULL or has zero
   * length, a scalar is returned.  If `shape' has length one, then
   * a vector with `shape[1]' elements is returned.  If `shape' has two
   * elements, then a matrix is returned with `shape[1]' rows and
   * `shape[2]' columns.  If `v' is a matrix, its elements are used
   * by rows.  If `v' doesn't contain enough elements to form the
   * result, it is padded with zeros or null strings.
   *
   * This function is identical to `bi_form', except that the result
   * of `bi_cram' may be sparse.
   */

  ENTITY * volatile r = NULL;

  WITH_HANDLING
  {

    if (shape)
      shape = vector_entity (EAT (shape));

    switch (v->class)
      {

      case scalar:
	r = bi_cram_scalar (EAT (shape), EAT (v));
	break;

      case vector:
	r = bi_cram_vector (EAT (shape), EAT (v));
	break;

      case matrix:
	r = bi_cram_matrix (EAT (shape), EAT (v));
	break;

      default:
	fail ("Can't use a %s in \"cram\".", class_string[v->class]);
	raise_exception ();

      }

  }
  ON_EXCEPTION
  {
    delete_3_entities (shape, v, r);
  }
  END_EXCEPTION;

  return r;
}

ENTITY *
bi_cram_scalar (VECTOR *shape, SCALAR *v)
{
  ENTITY * volatile d = NULL;
  int cnt, i, ne;

  WITH_HANDLING
  {
    if (shape)
      {
	shape = (VECTOR *) dense_vector (EAT (shape));
	shape = (VECTOR *) cast_vector (EAT (shape), integer);
	ne = shape->ne;
      }
    else
      ne = 0;

    switch (ne)
      {
      case 0:
	d = copy_scalar (v);
	break;

      case 1:
	cnt = shape->a.integer[0];
	if (cnt < 0)
	  {
	    fail ("Specified vector dimension is negative.");
	    raise_exception ();
	  }

	/* Should this be sparse? */

	if (fat_vector (cnt, 1, v->type, sparse))
	  d = bi_form_scalar (EAT (shape), EAT (v));
	else
	  {
	    d = make_vector (cnt, v->type, sparse);
	    if (test_scalar ((SCALAR *) copy_scalar (v)))
	      {
		((VECTOR *) d)->a.ptr = E_MALLOC (1, v->type);
		((VECTOR *) d)->ja = (int *) MALLOC (sizeof (int));
		((VECTOR *) d)->ja[0] = 1;
		((VECTOR *) d)->nn = 1;
		memcpy (((VECTOR *) d)->a.ptr, &v->v.integer,
			type_size[v->type]);
		if (((VECTOR *) d)->type == character)
		  ((VECTOR *) d)->a.character[0] =
		    dup_char (((VECTOR *) d)->a.character[0]);
	      }
	  }
	break;

      case 2:
	if (shape->a.integer[0] < 0 || shape->a.integer[1] < 0)
	  {
	    fail ("Specified matrix dimension is negative.");
	    raise_exception ();
	  }

	/* Should this be sparse? */

	if (fat_matrix (shape->a.integer[0], shape->a.integer[1],
			1, v->type, sparse))
	  d = bi_form_scalar (EAT (shape), EAT (v));
	else
	  {
	    d = make_matrix (shape->a.integer[0], shape->a.integer[1],
			     v->type, sparse);
	    if (test_scalar ((SCALAR *) copy_scalar (v)))
	      {
		((MATRIX *) d)->a.ptr = E_MALLOC (1, v->type);
		((MATRIX *) d)->ia =
		  (int *) MALLOC ((shape->a.integer[0]+1)*sizeof (int));
		((MATRIX *) d)->ia[0] = 1;
		for (i=1; i<=shape->a.integer[0]; i++)
		  ((MATRIX *) d)->ia[i] = 2;
		((MATRIX *) d)->ja = (int *) MALLOC (sizeof (int));
		((MATRIX *) d)->ja[0] = 1;
		((MATRIX *) d)->nn = 1;
		memcpy (((MATRIX *) d)->a.ptr, &v->v.integer,
			type_size[v->type]);
		if (((MATRIX *) d)->type == character)
		  ((MATRIX *) d)->a.character[0] =
		    dup_char (((MATRIX *) d)->a.character[0]);
		if (v->type == complex && v->v.complex.imag)
		  ((MATRIX *) d)->symmetry = symmetric;
	      }
	  }
	break;

      default:
	fail ("Invalid dimension (%d) for shape vector in \"cram\".",
	      shape->ne);
	raise_exception ();
      }
  }

  ON_EXCEPTION
  {
    delete_vector (shape);
    delete_scalar (v);
    delete_entity (d);
  }
  END_EXCEPTION;

  delete_vector (shape);
  delete_scalar (v);
  return (d);
}

ENTITY *
bi_cram_vector (VECTOR *shape, VECTOR *v)
{
  ENTITY *d = NULL;
  int i, j, dim, ne, nr, nc;
  double cnt;

  WITH_HANDLING
  {
    if (shape)
      {
	shape = (VECTOR *) dense_vector (EAT (shape));
	shape = (VECTOR *) cast_vector (EAT (shape), integer);
	dim = shape->ne;
      }
    else
      dim = 0;

    switch (dim)
      {
      case 0:
	if (v->ne)
	  {
	    switch (v->type)
	      {
	      case integer:
		d = int_to_scalar (get_vector_element_integer (EAT(v), 1));
		break;
	      case real:
		d = real_to_scalar (get_vector_element_real (EAT(v), 1));
		break;
	      case complex:
		d = complex_to_scalar (get_vector_element_complex (EAT(v), 1));
		break;
	      case character:
		d = char_to_scalar (get_vector_element_character (EAT(v), 1));
		break;
	      default:
		BAD_TYPE (v->type);
		delete_2_vectors (shape, v);
		raise_exception ();
	      }
	  }
	else
	  d = make_scalar (v->type);
	break;

      case 1:
	ne = shape->a.integer[0];
	cnt = (double) ne;
	if (ne < 0)
	  {
	    fail ("Specified vector dimension is negative.");
	    raise_exception ();
	  }
	if (cnt > (double) v->ne) cnt = (double) v->ne;

	/* Is there anything to copy?  Is it dense? */

	if (!test_vector ((VECTOR *) copy_vector (v)))
	  d = make_vector (ne, v->type, sparse);

	else if (v->density == dense &&
		 fat_vector (ne, (int) cnt, v->type, sparse))
	  d = bi_form_vector (EAT (shape), EAT (v));

	else
	  {
	    d = form_vector (ne, v->type, sparse);
	    if (cnt)
	      {
		int k = 0;

		assert (v->nn > 0);

		if (v->density == dense)
		  {
		    ((VECTOR *) d)->ja = MALLOC ((int) cnt * sizeof (int));
		    ((VECTOR *) d)->a.ptr = E_MALLOC ((int) cnt, v->type);
		    memcpy (((VECTOR *) d)->a.ptr, v->a.ptr,
			    (int) cnt * type_size[v->type]);
		    for (i=0; i<(int)cnt; i++) ((VECTOR *) d)->ja[i] = i+1;
		    k = (int) cnt;
		  }
		else
		  {
		    if ((int) cnt >= v->ne)
		      k = v->nn;
		    else
		      for (k = 0; k < v->nn && v->ja[k] <= (int) cnt; k++) ;

		    ((VECTOR *)d)->ja = MALLOC (k * sizeof (int));
		    ((VECTOR *)d)->a.ptr = E_MALLOC (k, v->type);

		    memcpy (((VECTOR *) d)->ja, v->ja, k * sizeof (int));
		    memcpy (((VECTOR *) d)->a.ptr, v->a.ptr,
			    k * type_size[v->type]);
		  }

		if (v->type == character)
		  {
		    for (i = 0; i < k; i++)
		      ((VECTOR *) d)->a.character[i] =
			dup_char (((VECTOR *) d)->a.character[i]);
		  }
		
		((VECTOR *) d)->nn = k;
	      }
	    d = apt_vector ((VECTOR *) EAT(d));
	  }
	break;

      case 2:
	nr = shape->a.integer[0];
	nc = shape->a.integer[1];
	if (nr < 0 || nc < 0)
	  {
	    fail ("Specified matrix dimension is negative.");
	    raise_exception ();
	  }
	cnt = (double) nr * (double) nc;
	if (cnt > (double) v->ne) cnt = (double) v->ne;

	if (!test_vector ((VECTOR *) copy_vector (v)))
	  d = make_matrix (nr, nc, v->type, sparse);

	else if (v->density == dense &&
		 fat_matrix (nr, nc, (int) cnt, v->type, sparse))
	  d = bi_form_vector (EAT (shape), EAT (v));

	else
	  {
	    d = make_matrix (nr, nc, v->type, sparse);
	    
	    if (cnt)
	      {
		int k = 0;
		assert (v->nn > 0);

		if (v->density == dense)
		  {
		    ((MATRIX *) d)->ia = MALLOC ((nr+1) * sizeof(int));
		    ((MATRIX *) d)->ja = MALLOC ((int) cnt * sizeof (int));
		    ((MATRIX *) d)->a.ptr = E_MALLOC ((int) cnt, v->type);

		    memcpy (((MATRIX *) d)->a.ptr, v->a.ptr,
			    (int) cnt * type_size[v->type]);
		    for (i=0; i<(int)cnt; i++) ((MATRIX *)d)->ja[i] = i%nc + 1;
		    for (k=1,j=0,i=0; i<=nr; i++)
		      {
			((MATRIX *) d)->ia[i] = k;
			k += nc;
			if (k > (int) cnt) k = (int) cnt + 1;
		      }
		    ((MATRIX *) d)->nn = (int) cnt;

		    if ((int) cnt > 1)
		      ((MATRIX *) d)->symmetry = general;
		    else if ((int) cnt == 1 && v->type == complex
			     && v->a.complex[0].imag)
		      ((MATRIX *) d)->symmetry = symmetric;
		  }
		else
		  {
		    if ((int) cnt >= v->ne)
		      k = v->nn;
		    else
		      for (k = 0; k < v->nn && v->ja[k] <= (int) cnt; k++) ;

		    ((MATRIX *) d)->ia = CALLOC (nr+1, sizeof(int));
		    ((MATRIX *) d)->ja = MALLOC (k * sizeof (int));
		    ((MATRIX *) d)->a.ptr = E_MALLOC (k, v->type);

		    memcpy (((MATRIX *) d)->a.ptr, v->a.ptr,
			    k * type_size[v->type]);
		    for (i=0; i<k; i++)
		      {
			j = v->ja[i];
			((MATRIX *) d)->ia[(j-1)/nc+1]++;
			((MATRIX *) d)->ja[i] = (j-1) % nc + 1;
		      }
		    for (j=1,i=0; i<=nr; i++)
		      j = (((MATRIX *) d)->ia[i] += j);

		    ((MATRIX *) d)->nn = k;

		    if ((int) cnt > 1)
		      ((MATRIX *) d)->symmetry = general;
		    else if ((int) cnt == 1 && k == 1 && v->type == complex
			     && v->a.complex[0].imag)
		      ((MATRIX *) d)->symmetry = symmetric;
		  }

		if (v->type == character)
		  {
		    for (i = 0; i < ((MATRIX *) d)->nn; i++)
		      ((MATRIX *) d)->a.character[i] =
			dup_char (((MATRIX *) d)->a.character[i]);
		  }

		d = apt_matrix ((MATRIX *) EAT(d));
	      }
	  }
	break;

      default:
	fail ("Invalid dimension (%d) for shape vector in \"cram\".",
	      shape->ne);
	raise_exception ();

      }
  }

  ON_EXCEPTION
  {
    delete_2_vectors (shape, v);
    delete_entity (d);
  }
  END_EXCEPTION;

  delete_2_vectors (shape, v);
  return (d);
}

ENTITY *
bi_cram_matrix (VECTOR *shape, MATRIX *v)
{
  ENTITY *d = NULL;
  int i, j, dim, ne, nr, nc,m;
  double cnt, vcnt;

  WITH_HANDLING
  {
    if (shape)
      {
	shape = (VECTOR *) dense_vector (EAT (shape));
	shape = (VECTOR *) cast_vector (EAT (shape), integer);
	dim = shape->ne;
      }
    else
      dim = 0;

    switch (dim)
      {
      case 0:
	if (v->nn || v->density == sparse_upper)
	  {
	    switch (v->type)
	      {
	      case integer:
		d = int_to_scalar (get_matrix_element_integer (EAT(v), 1, 1));
		break;
	      case real:
		d = real_to_scalar (get_matrix_element_real (EAT(v), 1, 1));
		break;
	      case complex:
		d = complex_to_scalar
		  (get_matrix_element_complex (EAT(v), 1, 1));
		break;
	      case character:
		d = char_to_scalar
		  (get_matrix_element_character (EAT(v), 1, 1));
		break;
	      default:
		BAD_TYPE (v->type);
		delete_vector (shape);
		delete_matrix (v);
		raise_exception ();
	      }
	  }
	else
	  d = make_scalar (v->type);
	break;

      case 1:
	ne =shape->a.integer[0];
	cnt = (double) ne;
	if (cnt < 0)
	  {
	    fail ("Specified vector dimension is negative.");
	    raise_exception ();
	  }
	if (cnt > (double) v->nr * (double) v->nc)
	  cnt = (double) v->nr * (double) v->nc;

	if (!test_matrix ((MATRIX *) copy_matrix (v)))
	  d = make_vector (ne, v->type, sparse);

	else if (v->density == dense &&
		 fat_vector (ne, (int) cnt, v->type, sparse))
	  d = bi_form_matrix (EAT (shape), EAT (v));

	else
	  {
	    d = form_vector (ne, v->type, sparse);
	    if (cnt)
	      {
		int k = 0;

		if (v->density == dense)
		  {
		    assert (v->nn > 0);
		    assert (v->nn <= (int) cnt);

		    ((VECTOR *) d)->ja = MALLOC ((int) cnt * sizeof (int));
		    ((VECTOR *) d)->a.ptr = E_MALLOC ((int) cnt, v->type);

		    switch (v->type)
		      {
		      case integer:
			for (i=0; i<v->nr; i++)
			  for (j=0; j<v->nc; j++)
			    ((VECTOR *) d)->a.integer[k++] =
			      v->a.integer[i+j*v->nr];
			break;
		      case real:
			for (i=0; i<v->nr; i++)
			  for (j=0; j<v->nc; j++)
			    ((VECTOR *) d)->a.real[k++] =
			      v->a.real[i+j*v->nr];
			break;
		      case complex:
			for (i=0; i<v->nr; i++)
			  for (j=0; j<v->nc; j++)
			    ((VECTOR *) d)->a.complex[k++] =
			      v->a.complex[i+j*v->nr];
			break;
		      case character:
			for (i=0; i<v->nr; i++)
			  for (j=0; j<v->nc; j++)
			    ((VECTOR *) d)->a.character[k++] =
			      v->a.character[i+j*v->nr];
			break;
		      default:
			BAD_TYPE (v->type);
			delete_vector (shape);
			delete_matrix (v);
			raise_exception ();
		      }

		    for (i=0; i<(int)cnt; i++) ((VECTOR *) d)->ja[i] = i+1;
		    k = (int) cnt;
		  }
		else
		  {
		    /*
		     * I just don't have time right now to add efficient code
		     * for the sparse_upper case.  Fix this later.
		     */

		    v = (MATRIX *) full_matrix (EAT (v));

		    assert (v->nn > 0);

		    if (cnt == (double) v->nr * (double) v->nc)
		      k = v->nn;
		    else
		      {
			j = (int) ((cnt-1) / (double) v->nc);
			for (k = v->ia[j]-1;
			     k < v->ia[j+1]-1 &&
			       (double) j * (double) v->nc +
			       (double) v->ja[k] <= cnt;
			     k++) ;
		      }

		    ((VECTOR *)d)->ja = MALLOC (k * sizeof (int));
		    ((VECTOR *)d)->a.ptr = E_MALLOC (k, v->type);

		    for (j=0,i=0; i<v->nr && j<k; i++)
		      {
			for (j = v->ia[i]-1; j < v->ia[i+1]-1 && j < k; j++)
			  {
			    ((VECTOR *) d)->ja[j] = v->ja[j] + i*v->nc;
			  }
		      }

		    memcpy (((VECTOR *) d)->a.ptr, v->a.ptr,
			    k * type_size[v->type]);
		  }

		if (v->type == character)
		  {
		    for (i = 0; i < k; i++)
		      ((VECTOR *) d)->a.character[i] =
			dup_char (((VECTOR *) d)->a.character[i]);
		  }
		
		((VECTOR *) d)->nn = k;
	      }
	    d = apt_vector ((VECTOR *) EAT(d));
	  }
	break;

      case 2:
	nr = shape->a.integer[0];
	nc = shape->a.integer[1];
	if (nr < 0 || nc < 0)
	  {
	    fail ("Specified matrix dimension is negative.");
	    raise_exception ();
	  }
	cnt = (double) nr * (double) nc;
	vcnt = (double) v->nr * (double) v->nc;
	if (cnt > vcnt) cnt = vcnt;

	if (!test_matrix ((MATRIX *) copy_matrix (v)))
	  d = make_matrix (nr, nc, v->type, sparse);

	else if (v->density == dense &&
		 fat_matrix (nr, nc, (int) cnt, v->type, sparse))
	  d = bi_form_matrix (EAT (shape), EAT (v));

	else
	  {
	    d = make_matrix (nr, nc, v->type, sparse);
	    
	    if (cnt)
	      {
		int k = 0;

		if (v->density == dense)
		  {
		    assert (v->nn > 0);
		    assert ((double) v->nn <= cnt);

		    ((MATRIX *) d)->ia = MALLOC ((nr+1) * sizeof(int));
		    ((MATRIX *) d)->ja = MALLOC ((int) cnt * sizeof (int));
		    ((MATRIX *) d)->a.ptr = E_MALLOC ((int) cnt, v->type);

		    switch (v->type)
		      {
		      case integer:
			for (i=0; i<v->nr; i++)
			  for (j=0; j<v->nc; j++)
			    ((MATRIX *) d)->a.integer[k++] =
			      v->a.integer[i+j*v->nr];
			break;
		      case real:
			for (i=0; i<v->nr; i++)
			  for (j=0; j<v->nc; j++)
			    ((MATRIX *) d)->a.real[k++] =
			      v->a.real[i+j*v->nr];
			break;
		      case complex:
			for (i=0; i<v->nr; i++)
			  for (j=0; j<v->nc; j++)
			    ((MATRIX *) d)->a.complex[k++] =
			      v->a.complex[i+j*v->nr];
			break;
		      case character:
			for (i=0; i<v->nr; i++)
			  for (j=0; j<v->nc; j++)
			    ((MATRIX *) d)->a.character[k++] =
			      v->a.character[i+j*v->nr];
			break;
		      default:
			BAD_TYPE (v->type);
			delete_vector (shape);
			delete_matrix (v);
			raise_exception ();
		      }

		    for (i=0; i<(int) cnt; i++)
		      ((MATRIX *) d)->ja[i] = i%nc + 1;
		    for (k=1,j=0,i=0; i<=nr; i++)
		      {
			((MATRIX *) d)->ia[i] = k;
			k += nc;
			if (k > (int) cnt) k = (int) cnt + 1;
		      }
		    ((MATRIX *) d)->nn = (int) cnt;

		    if (cnt > 1)
		      ((MATRIX *) d)->symmetry = general;
		    else if ((int) cnt == 1 && v->type == complex
			     && v->a.complex[0].imag)
		      ((MATRIX *) d)->symmetry = symmetric;
		  }
		else
		  {
		    /*
		     * I just don't have time right now to add efficient code
		     * for the sparse_upper case.  Fix this later.
		     */
		    double jj, *jja;

		    v = (MATRIX *) full_matrix (EAT (v));

		    assert (v->nn > 0);

		    if (cnt >= vcnt)
		      k = v->nn;
		    else
		      {
			j = (int) ((cnt-1) / (double) v->nc);
			for (k = v->ia[j]-1;
			     k < v->ia[j+1]-1 &&
			       (double) j * (double) v->nc +
			       (double) v->ja[k] <= cnt;
			     k++) ;
		      }

		    ((MATRIX *) d)->ia = CALLOC (nr+1, sizeof(int));
		    ((MATRIX *) d)->ja = MALLOC (k * sizeof (int));
		    jja = MALLOC (k * sizeof (double));
		    ((MATRIX *) d)->a.ptr = E_MALLOC (k, v->type);

		    memcpy (((MATRIX *) d)->a.ptr, v->a.ptr,
			    k * type_size[v->type]);
		    memcpy (((MATRIX *) d)->ja, v->ja, k * sizeof (int));
		    for (i=0; i<k; i++) jja[i] = (double) (v->ja[i]);

		    for (m=0,i=0; i<(int)((cnt-1)/(double)v->nc)+1; i++)
		      for (j = v->ia[i]-1; j < v->ia[i+1]-1 && m < k; j++,m++)
			jja[m] += (double) i * (double) v->nc;
			
		    for (i=0; i<k; i++)
		      {
			jj = jja[i];
			((MATRIX *) d)->ia[(int)((jj-1)/(double)nc)+1]++;
			((MATRIX *) d)->ja[i] = (int) fmod ((jj-1), nc) + 1;
		      }
		    for (j=1,i=0; i<=nr; i++)
		      j = (((MATRIX *) d)->ia[i] += j);

		    FREE (jja);
		    ((MATRIX *) d)->nn = k;

		    if (cnt > 1)
		      ((MATRIX *) d)->symmetry = general;
		    else if ((int) cnt == 1 && k == 1 && v->type == complex
			     && v->a.complex[0].imag)
		      ((MATRIX *) d)->symmetry = symmetric;
		  }

		if (v->type == character)
		  {
		    for (i = 0; i < ((MATRIX *) d)->nn; i++)
		      ((MATRIX *) d)->a.character[i] =
			dup_char (((MATRIX *) d)->a.character[i]);
		  }

		d = apt_matrix ((MATRIX *) EAT(d));
	      }
	  }
	break;

      default:
	fail ("Invalid dimension (%d) for shape vector in \"cram\".",
	      shape->ne);
	raise_exception ();

      }
  }

  ON_EXCEPTION
  {
    delete_vector (shape);
    delete_matrix (v);
    delete_entity (d);
  }
  END_EXCEPTION;

  delete_vector (shape);
  delete_matrix (v);
  return (d);
}
