
/* This file is generated from "assign.c.m4". */

/*
   assign.c -- Array element assignments.

   Copyright (C) 1994-2001  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
*/

static char rcsid[] =
  "$Id: assign.c.m4,v 1.5 2001/08/31 16:20:22 ksh Exp $";

#include "assign.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "partition.h"
#include "cast.h"
#include "dense.h"
#include "binop.h"
#include "full.h"
#include "find.h"
#include "fill.h"
#include "transpose.h"
#include "sparse.h"



ENTITY *
assign_submatrix (ENTITY *left, ENTITY *row, ENTITY *col, ENTITY *right)
{
    /*
     * This routine assigns the elements of `right' to the selected elements
     * of `left'.  The entities `row' and `col' specify the desired rows and
     * columns.  If `row' (`col') is NULL, then all rows (columns) are used.
     *
     * If `row' (`col') has character type, use it as index into `left->rid'
     * (`left->cid').
     */
    
    int nr = 0;
    int nc = 0;
    int i, left_nr, left_nc;
    int *r, *c;
    int *rtmp = NULL, *ctmp = NULL;

    static char msg_bad_dim[] = "Inconsistent dimensions for a submatrix assignment.  The number of %ss specified was %d, but the right hand side has %d %s%s.";
    static char msg_bad_spec[] = "Invalid %s specification for submatrix assignment.  The left hand side has %d %s%s, but %s %d was specified.";
    static char msg_bad_class[] = "Invalid (%s) class as %s specifier in submatrix assignment.";
    static char msg_rows[] = "row";
    static char msg_cols[] = "column";
    
    EASSERT( left, 0, 0 );
    EASSERT( right, 0, 0 );

    WITH_HANDLING {

	if ( left->class != matrix ) left = matrix_entity( EAT( left ) );
	left_nr = ((MATRIX *)left)->nr;
	left_nc = ((MATRIX *)left)->nc;

	/* If `row' is character, use it as index into left->rid. */

	if ( row &&
	     ( row->class == scalar && ((SCALAR *)row)->type == character ||
	       row->class == vector && ((VECTOR *)row)->type == character ||
	       row->class == matrix && ((MATRIX *)row)->type == character ) ) {

	    if ( ((MATRIX *)left)->rid ) {
		if ( row->class == scalar ) {
		    row = bi_find( EAT( row ),
				  copy_entity( ((MATRIX *)left)->rid ) );
		    if ( ((VECTOR *)row)->ne != 1 ) {
			fail( "Specified row label %s in submatrix assignment operation.",
			     ( ((VECTOR *)row)->ne > 1 ) ?
			     "is not unique" : "does not exist" );
			raise_exception();
		    }
		} else {
		    row = bi_find( EAT( row ),
				  copy_entity( ((MATRIX *)left)->rid ) );
		}

	    } else {
		fail( "Row labels don't exist for submatrix assignment." );
		raise_exception();
	    }
	}
	
	/* If `col' is character, use it as index into left->cid. */

	if ( col &&
	     ( col->class == scalar && ((SCALAR *)col)->type == character ||
	       col->class == vector && ((VECTOR *)col)->type == character ||
	       col->class == matrix && ((MATRIX *)col)->type == character ) ) {

	    if ( ((MATRIX *)left)->cid ) {
		
		if ( col->class == scalar ) {
		    col = bi_find( EAT( col ),
				  copy_entity( ((MATRIX *)left)->cid ) );
		    if ( ((VECTOR *)col)->ne != 1 ) {
			fail( "Specified column label %s in submatrix assignment operation.",
			     ( ((VECTOR *)col)->ne > 1 ) ?
			     "is not unique" : "does not exist" );
			raise_exception();
		    }
		} else {
		    col = bi_find( EAT( col ),
				  copy_entity( ((MATRIX *)left)->cid ) );
		}

	    } else {
		fail( "Column labels don't exist for submatrix assignment." );
		raise_exception();
	    }
	}

	/* Convert `row' to an integer scalar or vector. */
	
	if ( row ) {
	    if ( row->class == scalar ) {
		if ( ((SCALAR *)row)->type != integer )
		  row = cast_scalar( (SCALAR *) EAT( row ), integer );
		r = &(((SCALAR *)row)->v.integer);
                nr = 1;
	    } else {
		if ( row->class != vector ) row = vector_entity( EAT( row ) );
		if ( ((VECTOR *)row)->type != integer )
		  row = cast_vector( (VECTOR *) EAT( row ), integer );
		if ( ((VECTOR *)row)->density != dense )
		  row = dense_vector( (VECTOR *) EAT( row ) );
		r = ((VECTOR *)row)->a.integer;
		nr = ((VECTOR *)row)->ne;
	    }
	} else {
	    nr = left_nr;
            rtmp = r = MALLOC( nr * sizeof(int) );
            for ( i=0; i<nr; i++ ) r[i] = i+1;
	}
	
	/* Convert `col' to an integer scalar or vector. */
	
	if ( col ) {
	    if ( col->class == scalar ) {
		if ( ((SCALAR *)col)->type != integer )
		  col = cast_scalar( (SCALAR *) EAT( col ), integer );
		c = &(((SCALAR *)col)->v.integer);
                nc = 1;
	    } else {
		if ( col->class != vector ) col = vector_entity( EAT( col ) );
		if ( ((VECTOR *)col)->type != integer )
		  col = cast_vector( (VECTOR *) EAT( col ), integer );
		if ( ((VECTOR *)col)->density != dense )
		  col = dense_vector( (VECTOR *) EAT( col ) );
		c = ((VECTOR *)col)->a.integer;
		nc = ((VECTOR *)col)->ne;
	    }
	} else {
	    nc = left_nc;
            ctmp = c = MALLOC( nc * sizeof(int) );
            for ( i=0; i<nc; i++ ) c[i] = i+1;
	}

	/* Convert `right' to appropriate class. */

	switch ( row ? row->class : vector ) {

	  case scalar:
            nr = 1;
	    switch ( col ? col->class : vector ) {
	      case scalar:
		right = scalar_entity( EAT( right ) );
                nc = 1;
		break;
	      case vector:
		right = ( right->class == scalar ) ?
                    bi_fill( int_to_scalar( nc ), EAT( right ) ) :
                    vector_entity( EAT( right ) );
		assert( right->class == vector );
                nc = ((VECTOR *)right)->ne;
		break;
	      default:
		fail( msg_bad_class, class_string[ col->class ], msg_cols );
		raise_exception();
	    }
	    break;

	  case vector:
	  case matrix:
	    switch ( col ? col->class : vector ) {
	      case scalar:
		right = ( right->class == scalar ) ?
		  bi_fill( int_to_scalar( row ? ((VECTOR *)row)->ne :
					  left_nr ),
			   EAT( right ) ) :
		    vector_entity( EAT( right ) );
                nr = ((VECTOR *)right)->ne;
                nc = 1;
		break;
	      case vector:
		right = ( right->class == scalar ) ?
		  bi_fill( binop_scalar_integer( BO_APPEND,
			(SCALAR *) int_to_scalar( row ? ((VECTOR *)row)->ne :
						  left_nr ),
			(SCALAR *) int_to_scalar( col ? ((VECTOR *)col)->ne :
						  left_nc ) ),
			   EAT( right ) ) :
		    matrix_entity( EAT( right ) );
		nr = ((MATRIX *)right)->nr;
		nc = ((MATRIX *)right)->nc;
		break;
	      default:
		fail( msg_bad_class, class_string[ col->class ], msg_cols );
		raise_exception();
	    }
	    break;

	  default:
	    fail( msg_bad_class, class_string[ row->class ], msg_rows );
	    raise_exception();
	}

	/* Check row dimensions. */
	
	if ( row ) {
	    if ( row->class == scalar ) {
		if ( nr != 1 ) {
		    fail( msg_bad_dim, msg_rows, 1, nr,
			  msg_rows, PLURAL( nr ) );
		    raise_exception();
		}
	    } else {
		if ( nr != ((VECTOR *)row)->ne ) {
		    fail( msg_bad_dim, msg_rows, ((VECTOR *)row)->ne,
			  nr, msg_rows, PLURAL( nr ) );
		    raise_exception();
		}
	    }
	    for ( i=0; i<nr; i++ ) {
		if ( r[i] < 1 || r[i] > left_nr ) {
		    fail( msg_bad_spec, msg_rows, left_nr,
			  msg_rows, PLURAL( left_nr ), msg_rows, r[i] );
		    raise_exception();
		}
	    }
	} else {
	    if ( left_nr != nr ) {
		fail( msg_bad_dim, msg_rows, left_nr, nr,
		      msg_rows, PLURAL( nr ) );
		raise_exception();
	    }
	}
	
	/* Check column dimensions. */
	
	if ( col ) {
	    if ( col->class == scalar ) {
		if ( nc != 1 ) {
		    fail( msg_bad_dim, msg_cols, 1, nc,
			  msg_cols, PLURAL( nr ) );
		    raise_exception();
		}
	    } else {
		if ( nc != ((VECTOR *)col)->ne ) {
		    fail( msg_bad_dim, msg_cols, ((VECTOR *)col)->ne,
			  nc, msg_cols, PLURAL( nc ) );
		    raise_exception();
		}
	    }
	    for ( i=0; i<nc; i++ ) {
		if ( c[i] < 1 || c[i] > left_nc ) {
		    fail( msg_bad_spec, msg_cols, left_nc,
			  msg_cols, PLURAL( left_nc ), msg_cols, c[i] );
		    raise_exception();
		}
	    }
	} else {
	    if ( left_nc != nc ) {
		fail( msg_bad_dim, msg_cols, left_nc, nc,
		      msg_cols, PLURAL( nc ) );
		raise_exception();
	    }
	}
	
	assert( left->class == matrix );
	switch ( right->class )
        {
          case scalar:
            
	    left = assign_matrix_scalar( (MATRIX *) EAT( left ), r, c,
                                         (SCALAR *) EAT( right ) );
            break;

          case vector:

            right = ( nr == 1 ) ?
                vector_to_matrix( EAT( right ) ) :
                transpose_entity( EAT( right ) );

          case matrix:	/* fall through */

            left = assign_matrix_matrix( (MATRIX *) EAT( left ), r, c,
                                         (MATRIX *) EAT( right ) );
            break;

          default:

            BAD_CLASS( right->class );
            raise_exception();
            
        }

    }
    ON_EXCEPTION {
	delete_entity( left );
	delete_3_entities( row, col, right );
        TFREE( rtmp );
        TFREE( ctmp );
	raise_exception();
    }
    END_EXCEPTION;

    delete_3_entities( row, col, right );
    TFREE( rtmp );
    TFREE( ctmp );
    return left;
}

static int
compare_ints (int *a, int *b, int n)
{
    while ( n-- ) if ( *a++ != *b++ ) return 0;
    return 1;
}

static ENTITY *
dangerous_dup_matrix (MATRIX *m)
{
    /*
     * If `m' has a ref_count of 2, we assume that one of those is
     * being kept in case we raise an exception.  If we're careful not
     * to raise an exception before we get back there, we can modify
     * the copy and no one will ever know...
     */

    return ( m->entity.ref_count > 2 ) ? dup_matrix(m) : ENT(m);
}

static ENTITY *
dangerous_dup_vector (VECTOR *m)
{
    /*
     * If `m' has a ref_count of 2, we assume that one of those is
     * being kept in case we raise an exception.  If we're careful not
     * to raise an exception before we get back there, we can modify
     * the copy and no one will ever know...
     */

    return ( m->entity.ref_count > 2 ) ? dup_vector(m) : ENT(m);
}

ENTITY *
assign_matrix_matrix (MATRIX *left, int *row, int *col, MATRIX *right)
{
    int lnr, nr, nc, i, j, k, m, n;
    ENTITY *stuff;
    MATRIX *result;

    /* Hang on to stuff, so we can put it back on later. */
    
    stuff = left->stuff ? copy_table( left->stuff ) : NULL;

    lnr = left->nr;
    nr = right->nr;
    nc = right->nc;

    switch ( TWO_DENSITY( left->density, right->density ) )
    {

      case dense_dense:

        {
            TYPE rt = AUTO_TYPE( left->type, right->type );

            left = (MATRIX *) ( ( rt == left->type ) ?
                                dangerous_dup_matrix( left ) :
                                cast_matrix( left, rt ) );
            if ( !( nr == nc &&
                    left->symmetry != general &&
                    right->symmetry != general &&
                    ( left->symmetry == right->symmetry ||
                      left->type != right->type ) &&
                    compare_ints( row, col, nr ) ) )
                left->symmetry = general;

            
            switch ( TWO_TYPE( left->type, right->type ) )
            {
              case integer_integer:
    
                                {
                    int *r = left->a.integer;
                    int *p = right->a.integer;
    
                    for ( j=0; j<nc; j++ )
                    {
                        for ( i=0; i<nr; i++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k] =
                                 p[0];
                            p++;
                        }
                    }
                };
                break;
    
              case integer_real:
    
                                {
                    REAL *r = left->a.real;
                    REAL *p = right->a.real;
    
                    for ( j=0; j<nc; j++ )
                    {
                        for ( i=0; i<nr; i++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k] =
                                 p[0];
                            p++;
                        }
                    }
                };
                break;
    
              case integer_complex:
    
                                {
                    COMPLEX *r = left->a.complex;
                    COMPLEX *p = right->a.complex;
    
                    for ( j=0; j<nc; j++ )
                    {
                        for ( i=0; i<nr; i++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k].real =
                                 p[0].real;
                               r[k].imag =
                                 p[0].imag;
                            p++;
                        }
                    }
                };
                break;
    
              case real_integer:
    
                                {
                    REAL *r = left->a.real;
                    int *p = right->a.integer;
    
                    for ( j=0; j<nc; j++ )
                    {
                        for ( i=0; i<nr; i++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k] =
                                 (REAL) p[0];
                            p++;
                        }
                    }
                };
                break;
    
              case real_real:
    
                                {
                    REAL *r = left->a.real;
                    REAL *p = right->a.real;
    
                    for ( j=0; j<nc; j++ )
                    {
                        for ( i=0; i<nr; i++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k] =
                                 p[0];
                            p++;
                        }
                    }
                };
                break;
    
              case real_complex:
    
                                {
                    COMPLEX *r = left->a.complex;
                    COMPLEX *p = right->a.complex;
    
                    for ( j=0; j<nc; j++ )
                    {
                        for ( i=0; i<nr; i++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k].real =
                                 p[0].real;
                               r[k].imag =
                                 p[0].imag;
                            p++;
                        }
                    }
                };
                break;
    
              case complex_integer:
    
                                {
                    COMPLEX *r = left->a.complex;
                    int *p = right->a.integer;
    
                    for ( j=0; j<nc; j++ )
                    {
                        for ( i=0; i<nr; i++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k].real =
                                 (REAL) p[0];
                               r[k].imag = 0.0;
                            p++;
                        }
                    }
                };
                break;
    
              case complex_real:
    
                                {
                    COMPLEX *r = left->a.complex;
                    REAL *p = right->a.real;
    
                    for ( j=0; j<nc; j++ )
                    {
                        for ( i=0; i<nr; i++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k].real =
                                 p[0];
                               r[k].imag = 0.0;
                            p++;
                        }
                    }
                };
                break;
    
              case complex_complex:
    
                                {
                    COMPLEX *r = left->a.complex;
                    COMPLEX *p = right->a.complex;
    
                    for ( j=0; j<nc; j++ )
                    {
                        for ( i=0; i<nr; i++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k].real =
                                 p[0].real;
                               r[k].imag =
                                 p[0].imag;
                            p++;
                        }
                    }
                };
                break;
    
              case character_character:
    
                if ( right->entity.ref_count == 1 )
                {
                    ;
                                    {
                    char * *r = left->a.character;
                    char * *p = right->a.character;
    
                    for ( j=0; j<nc; j++ )
                    {
                        for ( i=0; i<nr; i++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k] =
                                        p[0];
                                      p[0] = NULL_string;
                            p++;
                        }
                    }
                };
    
                } else {
    
                    ;
                                    {
                    char * *r = left->a.character;
                    char * *p = right->a.character;
    
                    for ( j=0; j<nc; j++ )
                    {
                        for ( i=0; i<nr; i++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k] =
                                      dup_char( p[0] );
                            p++;
                        }
                    }
                };
                }
                break;
    
              default:
    
                fail( "Invalid types for submatrix assignment." );
                delete_matrix( left );
                delete_matrix( right );
                delete_entity( stuff );
                raise_exception();
    
            }
;

        }
        break;

      case dense_sparse_upper:

        right = (MATRIX *) full_matrix( right );

      case dense_sparse:	/* fall through */

        assert( right->order == ordered );
                
        {
            TYPE rt = AUTO_TYPE( left->type, right->type );

            left = (MATRIX *) ( ( rt == left->type ) ?
                                dangerous_dup_matrix( left ) :
                                cast_matrix( left, rt ) );
            if ( !( nr == nc &&
                    left->symmetry != general &&
                    right->symmetry != general &&
                    ( left->symmetry == right->symmetry ||
                      left->type != right->type ) &&
                    compare_ints( row, col, nr ) ) )
                left->symmetry = general;

            
            switch ( TWO_TYPE( left->type, right->type ) )
            {
              case integer_integer:
    
                
                {
                    int *r = left->a.integer;
                    int *p = right->a.integer;
    
                    for ( i=0; i<nr; i++ )
                    {
                        for ( j=0; j<nc; j++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k] = 0;
                        }
                        if ( right->nn )
                        {
                            for ( j=right->ia[i]-1; j<right->ia[i+1]-1; j++ )
                            {
                                k = row[i]-1+(col[right->ja[j]-1]-1)*lnr;
                                r[k] =
                                 p[j];
                            }
                        }
                    }
                };
                break;
    
              case integer_real:
    
                
                {
                    REAL *r = left->a.real;
                    REAL *p = right->a.real;
    
                    for ( i=0; i<nr; i++ )
                    {
                        for ( j=0; j<nc; j++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k] =  0.0;
                        }
                        if ( right->nn )
                        {
                            for ( j=right->ia[i]-1; j<right->ia[i+1]-1; j++ )
                            {
                                k = row[i]-1+(col[right->ja[j]-1]-1)*lnr;
                                r[k] =
                                 p[j];
                            }
                        }
                    }
                };
                break;
    
              case integer_complex:
    
                
                {
                    COMPLEX *r = left->a.complex;
                    COMPLEX *p = right->a.complex;
    
                    for ( i=0; i<nr; i++ )
                    {
                        for ( j=0; j<nc; j++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k].real = 0.0;
                   r[k].imag = 0.0;
                        }
                        if ( right->nn )
                        {
                            for ( j=right->ia[i]-1; j<right->ia[i+1]-1; j++ )
                            {
                                k = row[i]-1+(col[right->ja[j]-1]-1)*lnr;
                                r[k].real =
                                 p[j].real;
                               r[k].imag =
                                 p[j].imag;
                            }
                        }
                    }
                };
                break;
    
              case real_integer:
    
                
                {
                    REAL *r = left->a.real;
                    int *p = right->a.integer;
    
                    for ( i=0; i<nr; i++ )
                    {
                        for ( j=0; j<nc; j++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k] =  0.0;
                        }
                        if ( right->nn )
                        {
                            for ( j=right->ia[i]-1; j<right->ia[i+1]-1; j++ )
                            {
                                k = row[i]-1+(col[right->ja[j]-1]-1)*lnr;
                                r[k] =
                                 (REAL) p[j];
                            }
                        }
                    }
                };
                break;
    
              case real_real:
    
                
                {
                    REAL *r = left->a.real;
                    REAL *p = right->a.real;
    
                    for ( i=0; i<nr; i++ )
                    {
                        for ( j=0; j<nc; j++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k] =  0.0;
                        }
                        if ( right->nn )
                        {
                            for ( j=right->ia[i]-1; j<right->ia[i+1]-1; j++ )
                            {
                                k = row[i]-1+(col[right->ja[j]-1]-1)*lnr;
                                r[k] =
                                 p[j];
                            }
                        }
                    }
                };
                break;
    
              case real_complex:
    
                
                {
                    COMPLEX *r = left->a.complex;
                    COMPLEX *p = right->a.complex;
    
                    for ( i=0; i<nr; i++ )
                    {
                        for ( j=0; j<nc; j++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k].real = 0.0;
                   r[k].imag = 0.0;
                        }
                        if ( right->nn )
                        {
                            for ( j=right->ia[i]-1; j<right->ia[i+1]-1; j++ )
                            {
                                k = row[i]-1+(col[right->ja[j]-1]-1)*lnr;
                                r[k].real =
                                 p[j].real;
                               r[k].imag =
                                 p[j].imag;
                            }
                        }
                    }
                };
                break;
    
              case complex_integer:
    
                
                {
                    COMPLEX *r = left->a.complex;
                    int *p = right->a.integer;
    
                    for ( i=0; i<nr; i++ )
                    {
                        for ( j=0; j<nc; j++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k].real = 0.0;
                   r[k].imag = 0.0;
                        }
                        if ( right->nn )
                        {
                            for ( j=right->ia[i]-1; j<right->ia[i+1]-1; j++ )
                            {
                                k = row[i]-1+(col[right->ja[j]-1]-1)*lnr;
                                r[k].real =
                                 (REAL) p[j];
                               r[k].imag = 0.0;
                            }
                        }
                    }
                };
                break;
    
              case complex_real:
    
                
                {
                    COMPLEX *r = left->a.complex;
                    REAL *p = right->a.real;
    
                    for ( i=0; i<nr; i++ )
                    {
                        for ( j=0; j<nc; j++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k].real = 0.0;
                   r[k].imag = 0.0;
                        }
                        if ( right->nn )
                        {
                            for ( j=right->ia[i]-1; j<right->ia[i+1]-1; j++ )
                            {
                                k = row[i]-1+(col[right->ja[j]-1]-1)*lnr;
                                r[k].real =
                                 p[j];
                               r[k].imag = 0.0;
                            }
                        }
                    }
                };
                break;
    
              case complex_complex:
    
                
                {
                    COMPLEX *r = left->a.complex;
                    COMPLEX *p = right->a.complex;
    
                    for ( i=0; i<nr; i++ )
                    {
                        for ( j=0; j<nc; j++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            r[k].real = 0.0;
                   r[k].imag = 0.0;
                        }
                        if ( right->nn )
                        {
                            for ( j=right->ia[i]-1; j<right->ia[i+1]-1; j++ )
                            {
                                k = row[i]-1+(col[right->ja[j]-1]-1)*lnr;
                                r[k].real =
                                 p[j].real;
                               r[k].imag =
                                 p[j].imag;
                            }
                        }
                    }
                };
                break;
    
              case character_character:
    
                if ( right->entity.ref_count == 1 )
                {
                    ;
                    
                {
                    char * *r = left->a.character;
                    char * *p = right->a.character;
    
                    for ( i=0; i<nr; i++ )
                    {
                        for ( j=0; j<nc; j++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            FREE_CHAR(r[k]);
                   r[k] = NULL_string;
                        }
                        if ( right->nn )
                        {
                            for ( j=right->ia[i]-1; j<right->ia[i+1]-1; j++ )
                            {
                                k = row[i]-1+(col[right->ja[j]-1]-1)*lnr;
                                r[k] =
                                        p[j];
                                      p[j] = NULL_string;
                            }
                        }
                    }
                };
    
                } else {
    
                    ;
                    
                {
                    char * *r = left->a.character;
                    char * *p = right->a.character;
    
                    for ( i=0; i<nr; i++ )
                    {
                        for ( j=0; j<nc; j++ )
                        {
                            k = row[i]-1+(col[j]-1)*lnr;
                            FREE_CHAR(r[k]);
                   r[k] = NULL_string;
                        }
                        if ( right->nn )
                        {
                            for ( j=right->ia[i]-1; j<right->ia[i+1]-1; j++ )
                            {
                                k = row[i]-1+(col[right->ja[j]-1]-1)*lnr;
                                r[k] =
                                      dup_char( p[j] );
                            }
                        }
                    }
                };
                }
                break;
    
              default:
    
                fail( "Invalid types for submatrix assignment." );
                delete_matrix( left );
                delete_matrix( right );
                delete_entity( stuff );
                raise_exception();
    
            }
;

        }
        break;

      case sparse_dense:
      case sparse_upper_dense:

        right = (MATRIX *) sparse_matrix( right );

      case sparse_sparse_upper:		/* fall through */
      case sparse_upper_sparse:		/* fall through */
      case sparse_upper_sparse_upper:	/* fall through */

        left = (MATRIX *) full_matrix( left );
        right = (MATRIX *) full_matrix( right );

      case sparse_sparse:		/* fall through */

        {
            int c, jc, *rmap, *cmap, *icol, *right_cmap;
        
            assert( left->order == ordered );
            assert( right->order == ordered );

            /*
             * rmap[i-1] is row number of `right' from which ith row
             * of `left' is assigned.  If it's zero, no assignment
             * is made from that row of `right'.
             */
            
            rmap = CALLOC( left->nr, sizeof( int ) );
            for ( i=0; i<nr; i++ ) rmap[row[i]-1] = i+1;

            /*
             * cmap[i] is column number of `right' from which ith column
             * of `left' is assigned.  If it's zero, no assignment
             * is made from that column of `right'.
             */
            
            cmap = CALLOC( left->nc + 1, sizeof( int ) );
            for ( j=0; j<nc; j++ ) cmap[col[j]] = j+1;

            /* icol[i-1] is the index of the ith nonzero in cmap */

            icol = MALLOC( ( nc + 1 ) * sizeof(int) );
            for ( n=0, k=1; k<left->nc+1; k++ )
                if ( cmap[k] ) icol[n++] = k;
            jc = n;

            /*
             * right_cmap[j] will give the position in `right' at
             * which column j is located.  If column j of `right'
             * is not stored for row i, then right_cmap[j] will be
             * less than right->ia[i] or greater than right->ia[i+1]-1.
             */

            right_cmap = CALLOC( right->nc + 1, sizeof( int ) );
            
            /*
             * Start setting up the result.  We'll still need to decide its
             * type and symmetry, and allocate its data space.
             */
        
            result = (MATRIX *) make_matrix( left->nr, left->nc,
                                             left->type, sparse );

            /* copy rid's and cid's */

            if ( left->rid ) result->rid = copy_entity( left->rid );
            if ( left->cid ) result->cid = copy_entity( left->cid );

            /* allocate as much space as it could possibly need */

            if ( result->nn = ( left->nn + right->nn ) )
            {
                result->ia = MALLOC( ( result->nr + 1 ) * sizeof(int) );
                result->ja = MALLOC( ( left->nn + right->nn ) * sizeof(int) );
            }

            /* set type and symmetry of result */

            result->type = AUTO_TYPE( left->type, right->type );
            result->symmetry = ( nr == nc &&
                                 left->symmetry != general &&
                                 right->symmetry != general &&
                                 ( left->symmetry == right->symmetry ||
                                   left->type != right->type ) &&
                                 compare_ints( row, col, nr ) ) ?
                left->symmetry : general;

            
            switch ( TWO_TYPE( left->type, right->type ) )
            {
              case integer_integer:
    
                if ( result->nn )
                 {	
                     result->a.integer =
                         MALLOC( ( left->nn + right->nn ) *
                                 sizeof( int ) );
        
                     k = n = 0;
                     result->ia[0] = 1;
                     for ( i=0; i<lnr; i++ )
                     {
                         if ( rmap[i] )	/* assignments in this row? */
                         {
                             if ( right->ia )
                             {
                                 for ( m=right->ia[rmap[i]-1];
                                       m<right->ia[rmap[i]]; m++ )
                                     right_cmap[right->ja[m-1]] = m;
                             }
        
                             if ( left->ia &&
                                  left->ia[i+1] > left->ia[i] )
                             {
                                 for ( j=0; j<jc; j++ )
                                 {
                                     c = icol[j];
                                     while ( k < left->ia[i+1]-1 &&
                                             left->ja[k] < c )
                                     {
                                         result->ja[n] = left->ja[k];
                                         result->a.integer[n] =
                                 left->a.integer[k];
                                         n++;
                                         k++;
                                    }
                                    if ( k < left->ia[i+1]-1 &&
                                         left->ja[k] == c ) k++;
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.integer[n] =
                                 right->a.integer[right_cmap[cmap[c]]-1];
                                        n++;
                                    }
                                }
                                while ( k < left->ia[i+1]-1 )
                                {
                                    result->ja[n] = left->ja[k];
                                    result->a.integer[n] =
                                 left->a.integer[k];
                                    n++;
                                    k++;
                                }
                
                            } else {
                
                                for ( j=0; j<jc; j++ )
                                {
                                    c = icol[j];
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.integer[n] =
                                 right->a.integer[right_cmap[cmap[c]]-1];
                                        n++;
                                    }
                                }
                            }
                
                        } else {
                
                            if ( left->ia && left->ia[i+1] > left->ia[i] )
                            {
                                for ( j=left->ia[i]; j<left->ia[i+1]; j++ )
                                {
                                    result->ja[n] = left->ja[j-1];
                                    result->a.integer[n] =
                                 left->a.integer[j-1];
                                    n++;
                                    k++;
                                }
                            }
                        }
                
                        result->ia[i+1] = n+1;
                    }
                
                    result->nn = n;
                    result->ja = REALLOC( result->ja, n*sizeof(int) );
                    result->a.integer =
                        REALLOC( result->a.integer,
                                 n*sizeof(int) );
                };
                break;
    
              case integer_real:
    
                if ( result->nn )
                 {	
                     result->a.real =
                         MALLOC( ( left->nn + right->nn ) *
                                 sizeof( REAL ) );
        
                     k = n = 0;
                     result->ia[0] = 1;
                     for ( i=0; i<lnr; i++ )
                     {
                         if ( rmap[i] )	/* assignments in this row? */
                         {
                             if ( right->ia )
                             {
                                 for ( m=right->ia[rmap[i]-1];
                                       m<right->ia[rmap[i]]; m++ )
                                     right_cmap[right->ja[m-1]] = m;
                             }
        
                             if ( left->ia &&
                                  left->ia[i+1] > left->ia[i] )
                             {
                                 for ( j=0; j<jc; j++ )
                                 {
                                     c = icol[j];
                                     while ( k < left->ia[i+1]-1 &&
                                             left->ja[k] < c )
                                     {
                                         result->ja[n] = left->ja[k];
                                         result->a.real[n] =
                                 (REAL) left->a.integer[k];
                                         n++;
                                         k++;
                                    }
                                    if ( k < left->ia[i+1]-1 &&
                                         left->ja[k] == c ) k++;
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.real[n] =
                                 right->a.real[right_cmap[cmap[c]]-1];
                                        n++;
                                    }
                                }
                                while ( k < left->ia[i+1]-1 )
                                {
                                    result->ja[n] = left->ja[k];
                                    result->a.real[n] =
                                 (REAL) left->a.integer[k];
                                    n++;
                                    k++;
                                }
                
                            } else {
                
                                for ( j=0; j<jc; j++ )
                                {
                                    c = icol[j];
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.real[n] =
                                 right->a.real[right_cmap[cmap[c]]-1];
                                        n++;
                                    }
                                }
                            }
                
                        } else {
                
                            if ( left->ia && left->ia[i+1] > left->ia[i] )
                            {
                                for ( j=left->ia[i]; j<left->ia[i+1]; j++ )
                                {
                                    result->ja[n] = left->ja[j-1];
                                    result->a.real[n] =
                                 (REAL) left->a.integer[j-1];
                                    n++;
                                    k++;
                                }
                            }
                        }
                
                        result->ia[i+1] = n+1;
                    }
                
                    result->nn = n;
                    result->ja = REALLOC( result->ja, n*sizeof(int) );
                    result->a.real =
                        REALLOC( result->a.real,
                                 n*sizeof(REAL) );
                };
                break;
    
              case integer_complex:
    
                if ( result->nn )
                 {	
                     result->a.complex =
                         MALLOC( ( left->nn + right->nn ) *
                                 sizeof( COMPLEX ) );
        
                     k = n = 0;
                     result->ia[0] = 1;
                     for ( i=0; i<lnr; i++ )
                     {
                         if ( rmap[i] )	/* assignments in this row? */
                         {
                             if ( right->ia )
                             {
                                 for ( m=right->ia[rmap[i]-1];
                                       m<right->ia[rmap[i]]; m++ )
                                     right_cmap[right->ja[m-1]] = m;
                             }
        
                             if ( left->ia &&
                                  left->ia[i+1] > left->ia[i] )
                             {
                                 for ( j=0; j<jc; j++ )
                                 {
                                     c = icol[j];
                                     while ( k < left->ia[i+1]-1 &&
                                             left->ja[k] < c )
                                     {
                                         result->ja[n] = left->ja[k];
                                         result->a.complex[n].real =
                                 (REAL) left->a.integer[k];
                               result->a.complex[n].imag = 0.0;
                                         n++;
                                         k++;
                                    }
                                    if ( k < left->ia[i+1]-1 &&
                                         left->ja[k] == c ) k++;
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.complex[n].real =
                                 right->a.complex[right_cmap[cmap[c]]-1].real;
                               result->a.complex[n].imag =
                                 right->a.complex[right_cmap[cmap[c]]-1].imag;
                                        n++;
                                    }
                                }
                                while ( k < left->ia[i+1]-1 )
                                {
                                    result->ja[n] = left->ja[k];
                                    result->a.complex[n].real =
                                 (REAL) left->a.integer[k];
                               result->a.complex[n].imag = 0.0;
                                    n++;
                                    k++;
                                }
                
                            } else {
                
                                for ( j=0; j<jc; j++ )
                                {
                                    c = icol[j];
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.complex[n].real =
                                 right->a.complex[right_cmap[cmap[c]]-1].real;
                               result->a.complex[n].imag =
                                 right->a.complex[right_cmap[cmap[c]]-1].imag;
                                        n++;
                                    }
                                }
                            }
                
                        } else {
                
                            if ( left->ia && left->ia[i+1] > left->ia[i] )
                            {
                                for ( j=left->ia[i]; j<left->ia[i+1]; j++ )
                                {
                                    result->ja[n] = left->ja[j-1];
                                    result->a.complex[n].real =
                                 (REAL) left->a.integer[j-1];
                               result->a.complex[n].imag = 0.0;
                                    n++;
                                    k++;
                                }
                            }
                        }
                
                        result->ia[i+1] = n+1;
                    }
                
                    result->nn = n;
                    result->ja = REALLOC( result->ja, n*sizeof(int) );
                    result->a.complex =
                        REALLOC( result->a.complex,
                                 n*sizeof(COMPLEX) );
                };
                break;
    
              case real_integer:
    
                if ( result->nn )
                 {	
                     result->a.real =
                         MALLOC( ( left->nn + right->nn ) *
                                 sizeof( REAL ) );
        
                     k = n = 0;
                     result->ia[0] = 1;
                     for ( i=0; i<lnr; i++ )
                     {
                         if ( rmap[i] )	/* assignments in this row? */
                         {
                             if ( right->ia )
                             {
                                 for ( m=right->ia[rmap[i]-1];
                                       m<right->ia[rmap[i]]; m++ )
                                     right_cmap[right->ja[m-1]] = m;
                             }
        
                             if ( left->ia &&
                                  left->ia[i+1] > left->ia[i] )
                             {
                                 for ( j=0; j<jc; j++ )
                                 {
                                     c = icol[j];
                                     while ( k < left->ia[i+1]-1 &&
                                             left->ja[k] < c )
                                     {
                                         result->ja[n] = left->ja[k];
                                         result->a.real[n] =
                                 left->a.real[k];
                                         n++;
                                         k++;
                                    }
                                    if ( k < left->ia[i+1]-1 &&
                                         left->ja[k] == c ) k++;
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.real[n] =
                                 (REAL) right->a.integer[right_cmap[cmap[c]]-1];
                                        n++;
                                    }
                                }
                                while ( k < left->ia[i+1]-1 )
                                {
                                    result->ja[n] = left->ja[k];
                                    result->a.real[n] =
                                 left->a.real[k];
                                    n++;
                                    k++;
                                }
                
                            } else {
                
                                for ( j=0; j<jc; j++ )
                                {
                                    c = icol[j];
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.real[n] =
                                 (REAL) right->a.integer[right_cmap[cmap[c]]-1];
                                        n++;
                                    }
                                }
                            }
                
                        } else {
                
                            if ( left->ia && left->ia[i+1] > left->ia[i] )
                            {
                                for ( j=left->ia[i]; j<left->ia[i+1]; j++ )
                                {
                                    result->ja[n] = left->ja[j-1];
                                    result->a.real[n] =
                                 left->a.real[j-1];
                                    n++;
                                    k++;
                                }
                            }
                        }
                
                        result->ia[i+1] = n+1;
                    }
                
                    result->nn = n;
                    result->ja = REALLOC( result->ja, n*sizeof(int) );
                    result->a.real =
                        REALLOC( result->a.real,
                                 n*sizeof(REAL) );
                };
                break;
    
              case real_real:
    
                if ( result->nn )
                 {	
                     result->a.real =
                         MALLOC( ( left->nn + right->nn ) *
                                 sizeof( REAL ) );
        
                     k = n = 0;
                     result->ia[0] = 1;
                     for ( i=0; i<lnr; i++ )
                     {
                         if ( rmap[i] )	/* assignments in this row? */
                         {
                             if ( right->ia )
                             {
                                 for ( m=right->ia[rmap[i]-1];
                                       m<right->ia[rmap[i]]; m++ )
                                     right_cmap[right->ja[m-1]] = m;
                             }
        
                             if ( left->ia &&
                                  left->ia[i+1] > left->ia[i] )
                             {
                                 for ( j=0; j<jc; j++ )
                                 {
                                     c = icol[j];
                                     while ( k < left->ia[i+1]-1 &&
                                             left->ja[k] < c )
                                     {
                                         result->ja[n] = left->ja[k];
                                         result->a.real[n] =
                                 left->a.real[k];
                                         n++;
                                         k++;
                                    }
                                    if ( k < left->ia[i+1]-1 &&
                                         left->ja[k] == c ) k++;
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.real[n] =
                                 right->a.real[right_cmap[cmap[c]]-1];
                                        n++;
                                    }
                                }
                                while ( k < left->ia[i+1]-1 )
                                {
                                    result->ja[n] = left->ja[k];
                                    result->a.real[n] =
                                 left->a.real[k];
                                    n++;
                                    k++;
                                }
                
                            } else {
                
                                for ( j=0; j<jc; j++ )
                                {
                                    c = icol[j];
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.real[n] =
                                 right->a.real[right_cmap[cmap[c]]-1];
                                        n++;
                                    }
                                }
                            }
                
                        } else {
                
                            if ( left->ia && left->ia[i+1] > left->ia[i] )
                            {
                                for ( j=left->ia[i]; j<left->ia[i+1]; j++ )
                                {
                                    result->ja[n] = left->ja[j-1];
                                    result->a.real[n] =
                                 left->a.real[j-1];
                                    n++;
                                    k++;
                                }
                            }
                        }
                
                        result->ia[i+1] = n+1;
                    }
                
                    result->nn = n;
                    result->ja = REALLOC( result->ja, n*sizeof(int) );
                    result->a.real =
                        REALLOC( result->a.real,
                                 n*sizeof(REAL) );
                };
                break;
    
              case real_complex:
    
                if ( result->nn )
                 {	
                     result->a.complex =
                         MALLOC( ( left->nn + right->nn ) *
                                 sizeof( COMPLEX ) );
        
                     k = n = 0;
                     result->ia[0] = 1;
                     for ( i=0; i<lnr; i++ )
                     {
                         if ( rmap[i] )	/* assignments in this row? */
                         {
                             if ( right->ia )
                             {
                                 for ( m=right->ia[rmap[i]-1];
                                       m<right->ia[rmap[i]]; m++ )
                                     right_cmap[right->ja[m-1]] = m;
                             }
        
                             if ( left->ia &&
                                  left->ia[i+1] > left->ia[i] )
                             {
                                 for ( j=0; j<jc; j++ )
                                 {
                                     c = icol[j];
                                     while ( k < left->ia[i+1]-1 &&
                                             left->ja[k] < c )
                                     {
                                         result->ja[n] = left->ja[k];
                                         result->a.complex[n].real =
                                 left->a.real[k];
                               result->a.complex[n].imag = 0.0;
                                         n++;
                                         k++;
                                    }
                                    if ( k < left->ia[i+1]-1 &&
                                         left->ja[k] == c ) k++;
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.complex[n].real =
                                 right->a.complex[right_cmap[cmap[c]]-1].real;
                               result->a.complex[n].imag =
                                 right->a.complex[right_cmap[cmap[c]]-1].imag;
                                        n++;
                                    }
                                }
                                while ( k < left->ia[i+1]-1 )
                                {
                                    result->ja[n] = left->ja[k];
                                    result->a.complex[n].real =
                                 left->a.real[k];
                               result->a.complex[n].imag = 0.0;
                                    n++;
                                    k++;
                                }
                
                            } else {
                
                                for ( j=0; j<jc; j++ )
                                {
                                    c = icol[j];
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.complex[n].real =
                                 right->a.complex[right_cmap[cmap[c]]-1].real;
                               result->a.complex[n].imag =
                                 right->a.complex[right_cmap[cmap[c]]-1].imag;
                                        n++;
                                    }
                                }
                            }
                
                        } else {
                
                            if ( left->ia && left->ia[i+1] > left->ia[i] )
                            {
                                for ( j=left->ia[i]; j<left->ia[i+1]; j++ )
                                {
                                    result->ja[n] = left->ja[j-1];
                                    result->a.complex[n].real =
                                 left->a.real[j-1];
                               result->a.complex[n].imag = 0.0;
                                    n++;
                                    k++;
                                }
                            }
                        }
                
                        result->ia[i+1] = n+1;
                    }
                
                    result->nn = n;
                    result->ja = REALLOC( result->ja, n*sizeof(int) );
                    result->a.complex =
                        REALLOC( result->a.complex,
                                 n*sizeof(COMPLEX) );
                };
                break;
    
              case complex_integer:
    
                if ( result->nn )
                 {	
                     result->a.complex =
                         MALLOC( ( left->nn + right->nn ) *
                                 sizeof( COMPLEX ) );
        
                     k = n = 0;
                     result->ia[0] = 1;
                     for ( i=0; i<lnr; i++ )
                     {
                         if ( rmap[i] )	/* assignments in this row? */
                         {
                             if ( right->ia )
                             {
                                 for ( m=right->ia[rmap[i]-1];
                                       m<right->ia[rmap[i]]; m++ )
                                     right_cmap[right->ja[m-1]] = m;
                             }
        
                             if ( left->ia &&
                                  left->ia[i+1] > left->ia[i] )
                             {
                                 for ( j=0; j<jc; j++ )
                                 {
                                     c = icol[j];
                                     while ( k < left->ia[i+1]-1 &&
                                             left->ja[k] < c )
                                     {
                                         result->ja[n] = left->ja[k];
                                         result->a.complex[n].real =
                                 left->a.complex[k].real;
                               result->a.complex[n].imag =
                                 left->a.complex[k].imag;
                                         n++;
                                         k++;
                                    }
                                    if ( k < left->ia[i+1]-1 &&
                                         left->ja[k] == c ) k++;
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.complex[n].real =
                                 (REAL) right->a.integer[right_cmap[cmap[c]]-1];
                               result->a.complex[n].imag = 0.0;
                                        n++;
                                    }
                                }
                                while ( k < left->ia[i+1]-1 )
                                {
                                    result->ja[n] = left->ja[k];
                                    result->a.complex[n].real =
                                 left->a.complex[k].real;
                               result->a.complex[n].imag =
                                 left->a.complex[k].imag;
                                    n++;
                                    k++;
                                }
                
                            } else {
                
                                for ( j=0; j<jc; j++ )
                                {
                                    c = icol[j];
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.complex[n].real =
                                 (REAL) right->a.integer[right_cmap[cmap[c]]-1];
                               result->a.complex[n].imag = 0.0;
                                        n++;
                                    }
                                }
                            }
                
                        } else {
                
                            if ( left->ia && left->ia[i+1] > left->ia[i] )
                            {
                                for ( j=left->ia[i]; j<left->ia[i+1]; j++ )
                                {
                                    result->ja[n] = left->ja[j-1];
                                    result->a.complex[n].real =
                                 left->a.complex[j-1].real;
                               result->a.complex[n].imag =
                                 left->a.complex[j-1].imag;
                                    n++;
                                    k++;
                                }
                            }
                        }
                
                        result->ia[i+1] = n+1;
                    }
                
                    result->nn = n;
                    result->ja = REALLOC( result->ja, n*sizeof(int) );
                    result->a.complex =
                        REALLOC( result->a.complex,
                                 n*sizeof(COMPLEX) );
                };
                break;
    
              case complex_real:
    
                if ( result->nn )
                 {	
                     result->a.complex =
                         MALLOC( ( left->nn + right->nn ) *
                                 sizeof( COMPLEX ) );
        
                     k = n = 0;
                     result->ia[0] = 1;
                     for ( i=0; i<lnr; i++ )
                     {
                         if ( rmap[i] )	/* assignments in this row? */
                         {
                             if ( right->ia )
                             {
                                 for ( m=right->ia[rmap[i]-1];
                                       m<right->ia[rmap[i]]; m++ )
                                     right_cmap[right->ja[m-1]] = m;
                             }
        
                             if ( left->ia &&
                                  left->ia[i+1] > left->ia[i] )
                             {
                                 for ( j=0; j<jc; j++ )
                                 {
                                     c = icol[j];
                                     while ( k < left->ia[i+1]-1 &&
                                             left->ja[k] < c )
                                     {
                                         result->ja[n] = left->ja[k];
                                         result->a.complex[n].real =
                                 left->a.complex[k].real;
                               result->a.complex[n].imag =
                                 left->a.complex[k].imag;
                                         n++;
                                         k++;
                                    }
                                    if ( k < left->ia[i+1]-1 &&
                                         left->ja[k] == c ) k++;
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.complex[n].real =
                                 right->a.real[right_cmap[cmap[c]]-1];
                               result->a.complex[n].imag = 0.0;
                                        n++;
                                    }
                                }
                                while ( k < left->ia[i+1]-1 )
                                {
                                    result->ja[n] = left->ja[k];
                                    result->a.complex[n].real =
                                 left->a.complex[k].real;
                               result->a.complex[n].imag =
                                 left->a.complex[k].imag;
                                    n++;
                                    k++;
                                }
                
                            } else {
                
                                for ( j=0; j<jc; j++ )
                                {
                                    c = icol[j];
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.complex[n].real =
                                 right->a.real[right_cmap[cmap[c]]-1];
                               result->a.complex[n].imag = 0.0;
                                        n++;
                                    }
                                }
                            }
                
                        } else {
                
                            if ( left->ia && left->ia[i+1] > left->ia[i] )
                            {
                                for ( j=left->ia[i]; j<left->ia[i+1]; j++ )
                                {
                                    result->ja[n] = left->ja[j-1];
                                    result->a.complex[n].real =
                                 left->a.complex[j-1].real;
                               result->a.complex[n].imag =
                                 left->a.complex[j-1].imag;
                                    n++;
                                    k++;
                                }
                            }
                        }
                
                        result->ia[i+1] = n+1;
                    }
                
                    result->nn = n;
                    result->ja = REALLOC( result->ja, n*sizeof(int) );
                    result->a.complex =
                        REALLOC( result->a.complex,
                                 n*sizeof(COMPLEX) );
                };
                break;
    
              case complex_complex:
    
                if ( result->nn )
                 {	
                     result->a.complex =
                         MALLOC( ( left->nn + right->nn ) *
                                 sizeof( COMPLEX ) );
        
                     k = n = 0;
                     result->ia[0] = 1;
                     for ( i=0; i<lnr; i++ )
                     {
                         if ( rmap[i] )	/* assignments in this row? */
                         {
                             if ( right->ia )
                             {
                                 for ( m=right->ia[rmap[i]-1];
                                       m<right->ia[rmap[i]]; m++ )
                                     right_cmap[right->ja[m-1]] = m;
                             }
        
                             if ( left->ia &&
                                  left->ia[i+1] > left->ia[i] )
                             {
                                 for ( j=0; j<jc; j++ )
                                 {
                                     c = icol[j];
                                     while ( k < left->ia[i+1]-1 &&
                                             left->ja[k] < c )
                                     {
                                         result->ja[n] = left->ja[k];
                                         result->a.complex[n].real =
                                 left->a.complex[k].real;
                               result->a.complex[n].imag =
                                 left->a.complex[k].imag;
                                         n++;
                                         k++;
                                    }
                                    if ( k < left->ia[i+1]-1 &&
                                         left->ja[k] == c ) k++;
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.complex[n].real =
                                 right->a.complex[right_cmap[cmap[c]]-1].real;
                               result->a.complex[n].imag =
                                 right->a.complex[right_cmap[cmap[c]]-1].imag;
                                        n++;
                                    }
                                }
                                while ( k < left->ia[i+1]-1 )
                                {
                                    result->ja[n] = left->ja[k];
                                    result->a.complex[n].real =
                                 left->a.complex[k].real;
                               result->a.complex[n].imag =
                                 left->a.complex[k].imag;
                                    n++;
                                    k++;
                                }
                
                            } else {
                
                                for ( j=0; j<jc; j++ )
                                {
                                    c = icol[j];
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.complex[n].real =
                                 right->a.complex[right_cmap[cmap[c]]-1].real;
                               result->a.complex[n].imag =
                                 right->a.complex[right_cmap[cmap[c]]-1].imag;
                                        n++;
                                    }
                                }
                            }
                
                        } else {
                
                            if ( left->ia && left->ia[i+1] > left->ia[i] )
                            {
                                for ( j=left->ia[i]; j<left->ia[i+1]; j++ )
                                {
                                    result->ja[n] = left->ja[j-1];
                                    result->a.complex[n].real =
                                 left->a.complex[j-1].real;
                               result->a.complex[n].imag =
                                 left->a.complex[j-1].imag;
                                    n++;
                                    k++;
                                }
                            }
                        }
                
                        result->ia[i+1] = n+1;
                    }
                
                    result->nn = n;
                    result->ja = REALLOC( result->ja, n*sizeof(int) );
                    result->a.complex =
                        REALLOC( result->a.complex,
                                 n*sizeof(COMPLEX) );
                };
                break;
    
              case character_character:
    
                if ( right->entity.ref_count == 1 )
                {
                    ;
                    if ( result->nn )
                 {	
                     result->a.character =
                         MALLOC( ( left->nn + right->nn ) *
                                 sizeof( char * ) );
        
                     k = n = 0;
                     result->ia[0] = 1;
                     for ( i=0; i<lnr; i++ )
                     {
                         if ( rmap[i] )	/* assignments in this row? */
                         {
                             if ( right->ia )
                             {
                                 for ( m=right->ia[rmap[i]-1];
                                       m<right->ia[rmap[i]]; m++ )
                                     right_cmap[right->ja[m-1]] = m;
                             }
        
                             if ( left->ia &&
                                  left->ia[i+1] > left->ia[i] )
                             {
                                 for ( j=0; j<jc; j++ )
                                 {
                                     c = icol[j];
                                     while ( k < left->ia[i+1]-1 &&
                                             left->ja[k] < c )
                                     {
                                         result->ja[n] = left->ja[k];
                                         result->a.character[n] =
                                        left->a.character[k];
                                      left->a.character[k] = NULL_string;
                                         n++;
                                         k++;
                                    }
                                    if ( k < left->ia[i+1]-1 &&
                                         left->ja[k] == c ) k++;
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.character[n] =
                                        right->a.character[right_cmap[cmap[c]]-1];
                                      right->a.character[right_cmap[cmap[c]]-1] = NULL_string;
                                        n++;
                                    }
                                }
                                while ( k < left->ia[i+1]-1 )
                                {
                                    result->ja[n] = left->ja[k];
                                    result->a.character[n] =
                                        left->a.character[k];
                                      left->a.character[k] = NULL_string;
                                    n++;
                                    k++;
                                }
                
                            } else {
                
                                for ( j=0; j<jc; j++ )
                                {
                                    c = icol[j];
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.character[n] =
                                        right->a.character[right_cmap[cmap[c]]-1];
                                      right->a.character[right_cmap[cmap[c]]-1] = NULL_string;
                                        n++;
                                    }
                                }
                            }
                
                        } else {
                
                            if ( left->ia && left->ia[i+1] > left->ia[i] )
                            {
                                for ( j=left->ia[i]; j<left->ia[i+1]; j++ )
                                {
                                    result->ja[n] = left->ja[j-1];
                                    result->a.character[n] =
                                        left->a.character[j-1];
                                      left->a.character[j-1] = NULL_string;
                                    n++;
                                    k++;
                                }
                            }
                        }
                
                        result->ia[i+1] = n+1;
                    }
                
                    result->nn = n;
                    result->ja = REALLOC( result->ja, n*sizeof(int) );
                    result->a.character =
                        REALLOC( result->a.character,
                                 n*sizeof(char *) );
                };
    
                } else {
    
                    ;
                    if ( result->nn )
                 {	
                     result->a.character =
                         MALLOC( ( left->nn + right->nn ) *
                                 sizeof( char * ) );
        
                     k = n = 0;
                     result->ia[0] = 1;
                     for ( i=0; i<lnr; i++ )
                     {
                         if ( rmap[i] )	/* assignments in this row? */
                         {
                             if ( right->ia )
                             {
                                 for ( m=right->ia[rmap[i]-1];
                                       m<right->ia[rmap[i]]; m++ )
                                     right_cmap[right->ja[m-1]] = m;
                             }
        
                             if ( left->ia &&
                                  left->ia[i+1] > left->ia[i] )
                             {
                                 for ( j=0; j<jc; j++ )
                                 {
                                     c = icol[j];
                                     while ( k < left->ia[i+1]-1 &&
                                             left->ja[k] < c )
                                     {
                                         result->ja[n] = left->ja[k];
                                         result->a.character[n] =
                                      dup_char( left->a.character[k] );
                                         n++;
                                         k++;
                                    }
                                    if ( k < left->ia[i+1]-1 &&
                                         left->ja[k] == c ) k++;
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.character[n] =
                                      dup_char( right->a.character[right_cmap[cmap[c]]-1] );
                                        n++;
                                    }
                                }
                                while ( k < left->ia[i+1]-1 )
                                {
                                    result->ja[n] = left->ja[k];
                                    result->a.character[n] =
                                      dup_char( left->a.character[k] );
                                    n++;
                                    k++;
                                }
                
                            } else {
                
                                for ( j=0; j<jc; j++ )
                                {
                                    c = icol[j];
                                    if ( right->ia &&
                                         right_cmap[cmap[c]] >=
                                         right->ia[rmap[i]-1] &&
                                         right_cmap[cmap[c]] <
                                         right->ia[rmap[i]] )
                                    {
                                        result->ja[n] = c;
                                        result->a.character[n] =
                                      dup_char( right->a.character[right_cmap[cmap[c]]-1] );
                                        n++;
                                    }
                                }
                            }
                
                        } else {
                
                            if ( left->ia && left->ia[i+1] > left->ia[i] )
                            {
                                for ( j=left->ia[i]; j<left->ia[i+1]; j++ )
                                {
                                    result->ja[n] = left->ja[j-1];
                                    result->a.character[n] =
                                      dup_char( left->a.character[j-1] );
                                    n++;
                                    k++;
                                }
                            }
                        }
                
                        result->ia[i+1] = n+1;
                    }
                
                    result->nn = n;
                    result->ja = REALLOC( result->ja, n*sizeof(int) );
                    result->a.character =
                        REALLOC( result->a.character,
                                 n*sizeof(char *) );
                };
                }
                break;
    
              default:
    
                fail( "Invalid types for submatrix assignment." );
                delete_matrix( left );
                delete_matrix( right );
                delete_entity( stuff );
                raise_exception();
    
            }
;
        
	    FREE( rmap );
	    FREE( cmap );
	    FREE( icol );
	    FREE( right_cmap );

        }

        delete_matrix( left );
        left = result;
	result = NULL;
        
        break;

      default:

        detour( "Flub in assign_matrix_matrix." );
        delete_2_matrices( left, right );
        delete_entity( stuff );
        raise_exception();

    }

    /* Put stuff back on, if necessary. */

    if ( left->stuff )
        delete_entity( stuff );
    else
        left->stuff = (TABLE *) stuff;

    delete_matrix( right );
    return apt_matrix (left);
}

ENTITY *
assign_matrix_scalar (MATRIX *left, int *row, int *col, SCALAR *right)
{
    ENTITY *stuff;

    /* Hang on to stuff, so we can put it back on later. */
    
    stuff = left->stuff ? copy_table( left->stuff ) : NULL;

    AUTO_CAST_MATRIX_SCALAR( left, right );

    switch ( left->type )
    {
      case integer:

        left = (MATRIX *) assign_submatrix_integer( EAT( left ), *row, *col,
                                                    right->v.integer );
        break;
        
      case real:

        left = (MATRIX *) assign_submatrix_real( EAT( left ), *row, *col,
                                                 right->v.real );
        break;
        
      case complex:

        left = (MATRIX *) assign_submatrix_complex( EAT( left ), *row, *col,
                                                    right->v.complex );
        break;
        
      case character:

        left = (MATRIX *) assign_submatrix_character( EAT( left ), *row, *col,
                                       dup_char( right->v.character ) );
        break;

      default:

        BAD_TYPE( left->type );
        raise_exception();
    }

    /* Put stuff back on, if necessary. */

    if ( left->stuff )
        delete_entity( stuff );
    else
        left->stuff = (TABLE *) stuff;

    delete_scalar( right );
    return apt_matrix (left);
}

ENTITY *
assign_vector_vector (VECTOR *left, int *elem, VECTOR *right)
{
    int ne, i, j, k, m, n;
    ENTITY *stuff;
    VECTOR *result;

    /* Hang on to stuff, so we can put it back on later. */
    
    stuff = left->stuff ? copy_table( left->stuff ) : NULL;

    ne = right->ne;

    switch ( TWO_DENSITY( left->density, right->density ) )
    {

      case dense_dense:

        {
            TYPE rt = AUTO_TYPE( left->type, right->type );

            left = (VECTOR *) ( ( rt == left->type ) ?
                                dangerous_dup_vector( left ) :
                                cast_vector( left, rt ) );

            
            switch ( TWO_TYPE( left->type, right->type ) )
            {
              case integer_integer:
    
                                {
                    int *r = left->a.integer;
                    int *p = right->a.integer;
    
                    for ( i=0; i<ne; i++ )
                    {
                        k = elem[i]-1;
                        r[k] =
                                 p[0];
                        p++;
                    }
                };
                break;
    
              case integer_real:
    
                                {
                    REAL *r = left->a.real;
                    REAL *p = right->a.real;
    
                    for ( i=0; i<ne; i++ )
                    {
                        k = elem[i]-1;
                        r[k] =
                                 p[0];
                        p++;
                    }
                };
                break;
    
              case integer_complex:
    
                                {
                    COMPLEX *r = left->a.complex;
                    COMPLEX *p = right->a.complex;
    
                    for ( i=0; i<ne; i++ )
                    {
                        k = elem[i]-1;
                        r[k].real =
                                 p[0].real;
                               r[k].imag =
                                 p[0].imag;
                        p++;
                    }
                };
                break;
    
              case real_integer:
    
                                {
                    REAL *r = left->a.real;
                    int *p = right->a.integer;
    
                    for ( i=0; i<ne; i++ )
                    {
                        k = elem[i]-1;
                        r[k] =
                                 (REAL) p[0];
                        p++;
                    }
                };
                break;
    
              case real_real:
    
                                {
                    REAL *r = left->a.real;
                    REAL *p = right->a.real;
    
                    for ( i=0; i<ne; i++ )
                    {
                        k = elem[i]-1;
                        r[k] =
                                 p[0];
                        p++;
                    }
                };
                break;
    
              case real_complex:
    
                                {
                    COMPLEX *r = left->a.complex;
                    COMPLEX *p = right->a.complex;
    
                    for ( i=0; i<ne; i++ )
                    {
                        k = elem[i]-1;
                        r[k].real =
                                 p[0].real;
                               r[k].imag =
                                 p[0].imag;
                        p++;
                    }
                };
                break;
    
              case complex_integer:
    
                                {
                    COMPLEX *r = left->a.complex;
                    int *p = right->a.integer;
    
                    for ( i=0; i<ne; i++ )
                    {
                        k = elem[i]-1;
                        r[k].real =
                                 (REAL) p[0];
                               r[k].imag = 0.0;
                        p++;
                    }
                };
                break;
    
              case complex_real:
    
                                {
                    COMPLEX *r = left->a.complex;
                    REAL *p = right->a.real;
    
                    for ( i=0; i<ne; i++ )
                    {
                        k = elem[i]-1;
                        r[k].real =
                                 p[0];
                               r[k].imag = 0.0;
                        p++;
                    }
                };
                break;
    
              case complex_complex:
    
                                {
                    COMPLEX *r = left->a.complex;
                    COMPLEX *p = right->a.complex;
    
                    for ( i=0; i<ne; i++ )
                    {
                        k = elem[i]-1;
                        r[k].real =
                                 p[0].real;
                               r[k].imag =
                                 p[0].imag;
                        p++;
                    }
                };
                break;
    
              case character_character:
    
                if ( right->entity.ref_count == 1 )
                {
                    ;
                                    {
                    char * *r = left->a.character;
                    char * *p = right->a.character;
    
                    for ( i=0; i<ne; i++ )
                    {
                        k = elem[i]-1;
                        r[k] =
                                        p[0];
                                      p[0] = NULL_string;
                        p++;
                    }
                };
    
                } else {
    
                    ;
                                    {
                    char * *r = left->a.character;
                    char * *p = right->a.character;
    
                    for ( i=0; i<ne; i++ )
                    {
                        k = elem[i]-1;
                        r[k] =
                                      dup_char( p[0] );
                        p++;
                    }
                };
                }
                break;
    
              default:
    
                fail( "Invalid types for subvector assignment." );
                delete_vector( left );
                delete_vector( right );
                delete_entity( stuff );
                raise_exception();
    
            }
;

        }
        break;

      case dense_sparse:

        assert( right->order == ordered );
                
        {
            TYPE rt = AUTO_TYPE( left->type, right->type );

            left = (VECTOR *) ( ( rt == left->type ) ?
                                dangerous_dup_vector( left ) :
                                cast_vector( left, rt ) );

            
            switch ( TWO_TYPE( left->type, right->type ) )
            {
              case integer_integer:
    
                
            {
                int *r = left->a.integer;
                int *p = right->a.integer;

                for ( i=0; i<ne; i++ )
                {
                    k = elem[i]-1;
                    r[k] = 0;
                }
                for ( i=0; i<right->nn; i++ )
                {
                    k = elem[ right->ja[i] - 1 ] - 1;
                    r[k] =
                                 p[i];
                }
            };
                break;
    
              case integer_real:
    
                
            {
                REAL *r = left->a.real;
                REAL *p = right->a.real;

                for ( i=0; i<ne; i++ )
                {
                    k = elem[i]-1;
                    r[k] =  0.0;
                }
                for ( i=0; i<right->nn; i++ )
                {
                    k = elem[ right->ja[i] - 1 ] - 1;
                    r[k] =
                                 p[i];
                }
            };
                break;
    
              case integer_complex:
    
                
            {
                COMPLEX *r = left->a.complex;
                COMPLEX *p = right->a.complex;

                for ( i=0; i<ne; i++ )
                {
                    k = elem[i]-1;
                    r[k].real = 0.0;
                   r[k].imag = 0.0;
                }
                for ( i=0; i<right->nn; i++ )
                {
                    k = elem[ right->ja[i] - 1 ] - 1;
                    r[k].real =
                                 p[i].real;
                               r[k].imag =
                                 p[i].imag;
                }
            };
                break;
    
              case real_integer:
    
                
            {
                REAL *r = left->a.real;
                int *p = right->a.integer;

                for ( i=0; i<ne; i++ )
                {
                    k = elem[i]-1;
                    r[k] =  0.0;
                }
                for ( i=0; i<right->nn; i++ )
                {
                    k = elem[ right->ja[i] - 1 ] - 1;
                    r[k] =
                                 (REAL) p[i];
                }
            };
                break;
    
              case real_real:
    
                
            {
                REAL *r = left->a.real;
                REAL *p = right->a.real;

                for ( i=0; i<ne; i++ )
                {
                    k = elem[i]-1;
                    r[k] =  0.0;
                }
                for ( i=0; i<right->nn; i++ )
                {
                    k = elem[ right->ja[i] - 1 ] - 1;
                    r[k] =
                                 p[i];
                }
            };
                break;
    
              case real_complex:
    
                
            {
                COMPLEX *r = left->a.complex;
                COMPLEX *p = right->a.complex;

                for ( i=0; i<ne; i++ )
                {
                    k = elem[i]-1;
                    r[k].real = 0.0;
                   r[k].imag = 0.0;
                }
                for ( i=0; i<right->nn; i++ )
                {
                    k = elem[ right->ja[i] - 1 ] - 1;
                    r[k].real =
                                 p[i].real;
                               r[k].imag =
                                 p[i].imag;
                }
            };
                break;
    
              case complex_integer:
    
                
            {
                COMPLEX *r = left->a.complex;
                int *p = right->a.integer;

                for ( i=0; i<ne; i++ )
                {
                    k = elem[i]-1;
                    r[k].real = 0.0;
                   r[k].imag = 0.0;
                }
                for ( i=0; i<right->nn; i++ )
                {
                    k = elem[ right->ja[i] - 1 ] - 1;
                    r[k].real =
                                 (REAL) p[i];
                               r[k].imag = 0.0;
                }
            };
                break;
    
              case complex_real:
    
                
            {
                COMPLEX *r = left->a.complex;
                REAL *p = right->a.real;

                for ( i=0; i<ne; i++ )
                {
                    k = elem[i]-1;
                    r[k].real = 0.0;
                   r[k].imag = 0.0;
                }
                for ( i=0; i<right->nn; i++ )
                {
                    k = elem[ right->ja[i] - 1 ] - 1;
                    r[k].real =
                                 p[i];
                               r[k].imag = 0.0;
                }
            };
                break;
    
              case complex_complex:
    
                
            {
                COMPLEX *r = left->a.complex;
                COMPLEX *p = right->a.complex;

                for ( i=0; i<ne; i++ )
                {
                    k = elem[i]-1;
                    r[k].real = 0.0;
                   r[k].imag = 0.0;
                }
                for ( i=0; i<right->nn; i++ )
                {
                    k = elem[ right->ja[i] - 1 ] - 1;
                    r[k].real =
                                 p[i].real;
                               r[k].imag =
                                 p[i].imag;
                }
            };
                break;
    
              case character_character:
    
                if ( right->entity.ref_count == 1 )
                {
                    ;
                    
            {
                char * *r = left->a.character;
                char * *p = right->a.character;

                for ( i=0; i<ne; i++ )
                {
                    k = elem[i]-1;
                    FREE_CHAR(r[k]);
                   r[k] = NULL_string;
                }
                for ( i=0; i<right->nn; i++ )
                {
                    k = elem[ right->ja[i] - 1 ] - 1;
                    r[k] =
                                        p[i];
                                      p[i] = NULL_string;
                }
            };
    
                } else {
    
                    ;
                    
            {
                char * *r = left->a.character;
                char * *p = right->a.character;

                for ( i=0; i<ne; i++ )
                {
                    k = elem[i]-1;
                    FREE_CHAR(r[k]);
                   r[k] = NULL_string;
                }
                for ( i=0; i<right->nn; i++ )
                {
                    k = elem[ right->ja[i] - 1 ] - 1;
                    r[k] =
                                      dup_char( p[i] );
                }
            };
                }
                break;
    
              default:
    
                fail( "Invalid types for subvector assignment." );
                delete_vector( left );
                delete_vector( right );
                delete_entity( stuff );
                raise_exception();
    
            }
;

        }
        break;

      case sparse_dense:

        right = (VECTOR *) sparse_vector( right );

      case sparse_sparse:		/* fall through */

        {
            int c, jc, *cmap, *icol, *right_cmap;
        
            assert( left->order == ordered );
            assert( right->order == ordered );

            /*
             * cmap[i] is column number of `right' from which ith column
             * of `left' is assigned.  If it's zero, no assignment
             * is made from that column of `right'.
             */
            
            cmap = CALLOC( left->ne + 1, sizeof( int ) );
            for ( j=0; j<ne; j++ ) cmap[elem[j]] = j+1;

            /* icol[i-1] is the index of the ith nonzero in cmap */

            icol = MALLOC( ( ne + 1 ) * sizeof(int) );
            for ( n=0, k=1; k<left->ne+1; k++ )
                if ( cmap[k] ) icol[n++] = k;
            jc = n;

            /*
             * right_cmap[j] will give the position in `right' at
             * which column j is located.  If column j of `right'
             * is not stored, then right_cmap[j] will be zero.
             */

            right_cmap = CALLOC( right->ne + 1, sizeof( int ) );
            
            /*
             * Start setting up the result.  We'll still need to decide its
             * type and symmetry, and allocate its data space.
             */
        
            result = (VECTOR *) make_vector( left->ne, left->type, sparse );

            /* copy eid's */

            if ( left->eid ) result->eid = copy_entity( left->eid );

            /* allocate as much space as it could possibly need */

            if ( result->nn = ( left->nn + right->nn ) )
                result->ja = MALLOC( ( left->nn + right->nn ) * sizeof(int) );

            /* set type of result */

            result->type = AUTO_TYPE( left->type, right->type );

            
            switch ( TWO_TYPE( left->type, right->type ) )
            {
              case integer_integer:
    
                if ( result->nn )
         {	
             result->a.integer =
                 MALLOC( ( left->nn + right->nn ) *
                         sizeof( int ) );

             k = n = 0;

             for ( m=1; m<right->nn+1; m++ )
               right_cmap[right->ja[m-1]] = m;
             
             if ( left->nn )
             {
                 for ( j=0; j<jc; j++ )
                 {
                     c = icol[j];
                     while ( k < left->nn && left->ja[k] < c )
                     {
                         result->ja[n] = left->ja[k];
                         result->a.integer[n] =
                                 left->a.integer[k];
                         n++;
                         k++;
                    }
                    if ( k < left->nn && left->ja[k] == c ) k++;
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.integer[n] =
                                 right->a.integer[right_cmap[cmap[c]]-1];
                	n++;
                    }
                }
                while ( k < left->nn )
                {
                    result->ja[n] = left->ja[k];
                    result->a.integer[n] =
                                 left->a.integer[k];
                    n++;
                    k++;
                }
            
            } else {
            
                for ( j=0; j<jc; j++ )
                {
                    c = icol[j];
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.integer[n] =
                                 right->a.integer[right_cmap[cmap[c]]-1];
                	n++;
                    }
                }
            }
	
	    result->nn = n;
	    result->ja = REALLOC( result->ja, n*sizeof(int) );
	    result->a.integer =
		REALLOC( result->a.integer,
                         n*sizeof(int) );
	};
                break;
    
              case integer_real:
    
                if ( result->nn )
         {	
             result->a.real =
                 MALLOC( ( left->nn + right->nn ) *
                         sizeof( REAL ) );

             k = n = 0;

             for ( m=1; m<right->nn+1; m++ )
               right_cmap[right->ja[m-1]] = m;
             
             if ( left->nn )
             {
                 for ( j=0; j<jc; j++ )
                 {
                     c = icol[j];
                     while ( k < left->nn && left->ja[k] < c )
                     {
                         result->ja[n] = left->ja[k];
                         result->a.real[n] =
                                 left->a.real[k];
                         n++;
                         k++;
                    }
                    if ( k < left->nn && left->ja[k] == c ) k++;
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.real[n] =
                                 right->a.real[right_cmap[cmap[c]]-1];
                	n++;
                    }
                }
                while ( k < left->nn )
                {
                    result->ja[n] = left->ja[k];
                    result->a.real[n] =
                                 left->a.real[k];
                    n++;
                    k++;
                }
            
            } else {
            
                for ( j=0; j<jc; j++ )
                {
                    c = icol[j];
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.real[n] =
                                 right->a.real[right_cmap[cmap[c]]-1];
                	n++;
                    }
                }
            }
	
	    result->nn = n;
	    result->ja = REALLOC( result->ja, n*sizeof(int) );
	    result->a.real =
		REALLOC( result->a.real,
                         n*sizeof(REAL) );
	};
                break;
    
              case integer_complex:
    
                if ( result->nn )
         {	
             result->a.complex =
                 MALLOC( ( left->nn + right->nn ) *
                         sizeof( COMPLEX ) );

             k = n = 0;

             for ( m=1; m<right->nn+1; m++ )
               right_cmap[right->ja[m-1]] = m;
             
             if ( left->nn )
             {
                 for ( j=0; j<jc; j++ )
                 {
                     c = icol[j];
                     while ( k < left->nn && left->ja[k] < c )
                     {
                         result->ja[n] = left->ja[k];
                         result->a.complex[n].real =
                                 left->a.complex[k].real;
                               result->a.complex[n].imag =
                                 left->a.complex[k].imag;
                         n++;
                         k++;
                    }
                    if ( k < left->nn && left->ja[k] == c ) k++;
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.complex[n].real =
                                 right->a.complex[right_cmap[cmap[c]]-1].real;
                               result->a.complex[n].imag =
                                 right->a.complex[right_cmap[cmap[c]]-1].imag;
                	n++;
                    }
                }
                while ( k < left->nn )
                {
                    result->ja[n] = left->ja[k];
                    result->a.complex[n].real =
                                 left->a.complex[k].real;
                               result->a.complex[n].imag =
                                 left->a.complex[k].imag;
                    n++;
                    k++;
                }
            
            } else {
            
                for ( j=0; j<jc; j++ )
                {
                    c = icol[j];
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.complex[n].real =
                                 right->a.complex[right_cmap[cmap[c]]-1].real;
                               result->a.complex[n].imag =
                                 right->a.complex[right_cmap[cmap[c]]-1].imag;
                	n++;
                    }
                }
            }
	
	    result->nn = n;
	    result->ja = REALLOC( result->ja, n*sizeof(int) );
	    result->a.complex =
		REALLOC( result->a.complex,
                         n*sizeof(COMPLEX) );
	};
                break;
    
              case real_integer:
    
                if ( result->nn )
         {	
             result->a.real =
                 MALLOC( ( left->nn + right->nn ) *
                         sizeof( REAL ) );

             k = n = 0;

             for ( m=1; m<right->nn+1; m++ )
               right_cmap[right->ja[m-1]] = m;
             
             if ( left->nn )
             {
                 for ( j=0; j<jc; j++ )
                 {
                     c = icol[j];
                     while ( k < left->nn && left->ja[k] < c )
                     {
                         result->ja[n] = left->ja[k];
                         result->a.real[n] =
                                 (REAL) left->a.integer[k];
                         n++;
                         k++;
                    }
                    if ( k < left->nn && left->ja[k] == c ) k++;
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.real[n] =
                                 (REAL) right->a.integer[right_cmap[cmap[c]]-1];
                	n++;
                    }
                }
                while ( k < left->nn )
                {
                    result->ja[n] = left->ja[k];
                    result->a.real[n] =
                                 (REAL) left->a.integer[k];
                    n++;
                    k++;
                }
            
            } else {
            
                for ( j=0; j<jc; j++ )
                {
                    c = icol[j];
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.real[n] =
                                 (REAL) right->a.integer[right_cmap[cmap[c]]-1];
                	n++;
                    }
                }
            }
	
	    result->nn = n;
	    result->ja = REALLOC( result->ja, n*sizeof(int) );
	    result->a.real =
		REALLOC( result->a.real,
                         n*sizeof(REAL) );
	};
                break;
    
              case real_real:
    
                if ( result->nn )
         {	
             result->a.real =
                 MALLOC( ( left->nn + right->nn ) *
                         sizeof( REAL ) );

             k = n = 0;

             for ( m=1; m<right->nn+1; m++ )
               right_cmap[right->ja[m-1]] = m;
             
             if ( left->nn )
             {
                 for ( j=0; j<jc; j++ )
                 {
                     c = icol[j];
                     while ( k < left->nn && left->ja[k] < c )
                     {
                         result->ja[n] = left->ja[k];
                         result->a.real[n] =
                                 left->a.real[k];
                         n++;
                         k++;
                    }
                    if ( k < left->nn && left->ja[k] == c ) k++;
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.real[n] =
                                 right->a.real[right_cmap[cmap[c]]-1];
                	n++;
                    }
                }
                while ( k < left->nn )
                {
                    result->ja[n] = left->ja[k];
                    result->a.real[n] =
                                 left->a.real[k];
                    n++;
                    k++;
                }
            
            } else {
            
                for ( j=0; j<jc; j++ )
                {
                    c = icol[j];
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.real[n] =
                                 right->a.real[right_cmap[cmap[c]]-1];
                	n++;
                    }
                }
            }
	
	    result->nn = n;
	    result->ja = REALLOC( result->ja, n*sizeof(int) );
	    result->a.real =
		REALLOC( result->a.real,
                         n*sizeof(REAL) );
	};
                break;
    
              case real_complex:
    
                if ( result->nn )
         {	
             result->a.complex =
                 MALLOC( ( left->nn + right->nn ) *
                         sizeof( COMPLEX ) );

             k = n = 0;

             for ( m=1; m<right->nn+1; m++ )
               right_cmap[right->ja[m-1]] = m;
             
             if ( left->nn )
             {
                 for ( j=0; j<jc; j++ )
                 {
                     c = icol[j];
                     while ( k < left->nn && left->ja[k] < c )
                     {
                         result->ja[n] = left->ja[k];
                         result->a.complex[n].real =
                                 left->a.complex[k].real;
                               result->a.complex[n].imag =
                                 left->a.complex[k].imag;
                         n++;
                         k++;
                    }
                    if ( k < left->nn && left->ja[k] == c ) k++;
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.complex[n].real =
                                 right->a.complex[right_cmap[cmap[c]]-1].real;
                               result->a.complex[n].imag =
                                 right->a.complex[right_cmap[cmap[c]]-1].imag;
                	n++;
                    }
                }
                while ( k < left->nn )
                {
                    result->ja[n] = left->ja[k];
                    result->a.complex[n].real =
                                 left->a.complex[k].real;
                               result->a.complex[n].imag =
                                 left->a.complex[k].imag;
                    n++;
                    k++;
                }
            
            } else {
            
                for ( j=0; j<jc; j++ )
                {
                    c = icol[j];
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.complex[n].real =
                                 right->a.complex[right_cmap[cmap[c]]-1].real;
                               result->a.complex[n].imag =
                                 right->a.complex[right_cmap[cmap[c]]-1].imag;
                	n++;
                    }
                }
            }
	
	    result->nn = n;
	    result->ja = REALLOC( result->ja, n*sizeof(int) );
	    result->a.complex =
		REALLOC( result->a.complex,
                         n*sizeof(COMPLEX) );
	};
                break;
    
              case complex_integer:
    
                if ( result->nn )
         {	
             result->a.complex =
                 MALLOC( ( left->nn + right->nn ) *
                         sizeof( COMPLEX ) );

             k = n = 0;

             for ( m=1; m<right->nn+1; m++ )
               right_cmap[right->ja[m-1]] = m;
             
             if ( left->nn )
             {
                 for ( j=0; j<jc; j++ )
                 {
                     c = icol[j];
                     while ( k < left->nn && left->ja[k] < c )
                     {
                         result->ja[n] = left->ja[k];
                         result->a.complex[n].real =
                                 (REAL) left->a.integer[k];
                               result->a.complex[n].imag = 0.0;
                         n++;
                         k++;
                    }
                    if ( k < left->nn && left->ja[k] == c ) k++;
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.complex[n].real =
                                 (REAL) right->a.integer[right_cmap[cmap[c]]-1];
                               result->a.complex[n].imag = 0.0;
                	n++;
                    }
                }
                while ( k < left->nn )
                {
                    result->ja[n] = left->ja[k];
                    result->a.complex[n].real =
                                 (REAL) left->a.integer[k];
                               result->a.complex[n].imag = 0.0;
                    n++;
                    k++;
                }
            
            } else {
            
                for ( j=0; j<jc; j++ )
                {
                    c = icol[j];
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.complex[n].real =
                                 (REAL) right->a.integer[right_cmap[cmap[c]]-1];
                               result->a.complex[n].imag = 0.0;
                	n++;
                    }
                }
            }
	
	    result->nn = n;
	    result->ja = REALLOC( result->ja, n*sizeof(int) );
	    result->a.complex =
		REALLOC( result->a.complex,
                         n*sizeof(COMPLEX) );
	};
                break;
    
              case complex_real:
    
                if ( result->nn )
         {	
             result->a.complex =
                 MALLOC( ( left->nn + right->nn ) *
                         sizeof( COMPLEX ) );

             k = n = 0;

             for ( m=1; m<right->nn+1; m++ )
               right_cmap[right->ja[m-1]] = m;
             
             if ( left->nn )
             {
                 for ( j=0; j<jc; j++ )
                 {
                     c = icol[j];
                     while ( k < left->nn && left->ja[k] < c )
                     {
                         result->ja[n] = left->ja[k];
                         result->a.complex[n].real =
                                 left->a.real[k];
                               result->a.complex[n].imag = 0.0;
                         n++;
                         k++;
                    }
                    if ( k < left->nn && left->ja[k] == c ) k++;
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.complex[n].real =
                                 right->a.real[right_cmap[cmap[c]]-1];
                               result->a.complex[n].imag = 0.0;
                	n++;
                    }
                }
                while ( k < left->nn )
                {
                    result->ja[n] = left->ja[k];
                    result->a.complex[n].real =
                                 left->a.real[k];
                               result->a.complex[n].imag = 0.0;
                    n++;
                    k++;
                }
            
            } else {
            
                for ( j=0; j<jc; j++ )
                {
                    c = icol[j];
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.complex[n].real =
                                 right->a.real[right_cmap[cmap[c]]-1];
                               result->a.complex[n].imag = 0.0;
                	n++;
                    }
                }
            }
	
	    result->nn = n;
	    result->ja = REALLOC( result->ja, n*sizeof(int) );
	    result->a.complex =
		REALLOC( result->a.complex,
                         n*sizeof(COMPLEX) );
	};
                break;
    
              case complex_complex:
    
                if ( result->nn )
         {	
             result->a.complex =
                 MALLOC( ( left->nn + right->nn ) *
                         sizeof( COMPLEX ) );

             k = n = 0;

             for ( m=1; m<right->nn+1; m++ )
               right_cmap[right->ja[m-1]] = m;
             
             if ( left->nn )
             {
                 for ( j=0; j<jc; j++ )
                 {
                     c = icol[j];
                     while ( k < left->nn && left->ja[k] < c )
                     {
                         result->ja[n] = left->ja[k];
                         result->a.complex[n].real =
                                 left->a.complex[k].real;
                               result->a.complex[n].imag =
                                 left->a.complex[k].imag;
                         n++;
                         k++;
                    }
                    if ( k < left->nn && left->ja[k] == c ) k++;
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.complex[n].real =
                                 right->a.complex[right_cmap[cmap[c]]-1].real;
                               result->a.complex[n].imag =
                                 right->a.complex[right_cmap[cmap[c]]-1].imag;
                	n++;
                    }
                }
                while ( k < left->nn )
                {
                    result->ja[n] = left->ja[k];
                    result->a.complex[n].real =
                                 left->a.complex[k].real;
                               result->a.complex[n].imag =
                                 left->a.complex[k].imag;
                    n++;
                    k++;
                }
            
            } else {
            
                for ( j=0; j<jc; j++ )
                {
                    c = icol[j];
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.complex[n].real =
                                 right->a.complex[right_cmap[cmap[c]]-1].real;
                               result->a.complex[n].imag =
                                 right->a.complex[right_cmap[cmap[c]]-1].imag;
                	n++;
                    }
                }
            }
	
	    result->nn = n;
	    result->ja = REALLOC( result->ja, n*sizeof(int) );
	    result->a.complex =
		REALLOC( result->a.complex,
                         n*sizeof(COMPLEX) );
	};
                break;
    
              case character_character:
    
                if ( right->entity.ref_count == 1 )
                {
                    ;
                    if ( result->nn )
         {	
             result->a.character =
                 MALLOC( ( left->nn + right->nn ) *
                         sizeof( char * ) );

             k = n = 0;

             for ( m=1; m<right->nn+1; m++ )
               right_cmap[right->ja[m-1]] = m;
             
             if ( left->nn )
             {
                 for ( j=0; j<jc; j++ )
                 {
                     c = icol[j];
                     while ( k < left->nn && left->ja[k] < c )
                     {
                         result->ja[n] = left->ja[k];
                         result->a.character[n] =
                                        left->a.character[k];
                                      left->a.character[k] = NULL_string;
                         n++;
                         k++;
                    }
                    if ( k < left->nn && left->ja[k] == c ) k++;
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.character[n] =
                                        right->a.character[right_cmap[cmap[c]]-1];
                                      right->a.character[right_cmap[cmap[c]]-1] = NULL_string;
                	n++;
                    }
                }
                while ( k < left->nn )
                {
                    result->ja[n] = left->ja[k];
                    result->a.character[n] =
                                        left->a.character[k];
                                      left->a.character[k] = NULL_string;
                    n++;
                    k++;
                }
            
            } else {
            
                for ( j=0; j<jc; j++ )
                {
                    c = icol[j];
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.character[n] =
                                        right->a.character[right_cmap[cmap[c]]-1];
                                      right->a.character[right_cmap[cmap[c]]-1] = NULL_string;
                	n++;
                    }
                }
            }
	
	    result->nn = n;
	    result->ja = REALLOC( result->ja, n*sizeof(int) );
	    result->a.character =
		REALLOC( result->a.character,
                         n*sizeof(char *) );
	};
    
                } else {
    
                    ;
                    if ( result->nn )
         {	
             result->a.character =
                 MALLOC( ( left->nn + right->nn ) *
                         sizeof( char * ) );

             k = n = 0;

             for ( m=1; m<right->nn+1; m++ )
               right_cmap[right->ja[m-1]] = m;
             
             if ( left->nn )
             {
                 for ( j=0; j<jc; j++ )
                 {
                     c = icol[j];
                     while ( k < left->nn && left->ja[k] < c )
                     {
                         result->ja[n] = left->ja[k];
                         result->a.character[n] =
                                      dup_char( left->a.character[k] );
                         n++;
                         k++;
                    }
                    if ( k < left->nn && left->ja[k] == c ) k++;
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.character[n] =
                                      dup_char( right->a.character[right_cmap[cmap[c]]-1] );
                	n++;
                    }
                }
                while ( k < left->nn )
                {
                    result->ja[n] = left->ja[k];
                    result->a.character[n] =
                                      dup_char( left->a.character[k] );
                    n++;
                    k++;
                }
            
            } else {
            
                for ( j=0; j<jc; j++ )
                {
                    c = icol[j];
                    if ( right->nn && right_cmap[cmap[c]] >= 1 )
                    {
                	result->ja[n] = c;
                        result->a.character[n] =
                                      dup_char( right->a.character[right_cmap[cmap[c]]-1] );
                	n++;
                    }
                }
            }
	
	    result->nn = n;
	    result->ja = REALLOC( result->ja, n*sizeof(int) );
	    result->a.character =
		REALLOC( result->a.character,
                         n*sizeof(char *) );
	};
                }
                break;
    
              default:
    
                fail( "Invalid types for subvector assignment." );
                delete_vector( left );
                delete_vector( right );
                delete_entity( stuff );
                raise_exception();
    
            }
;

	    FREE( cmap );
	    FREE( icol );
	    FREE( right_cmap );
        
        }
        delete_vector( left );
        left = result;
        
        break;

      default:

        detour( "Flub in assign_vector_vector." );
        delete_2_vectors( left, right );
        delete_entity( stuff );
        raise_exception();

    }

    /* Put stuff back on, if necessary. */

    if ( left->stuff )
        delete_entity( stuff );
    else
        left->stuff = (TABLE *) stuff;

    delete_vector( right );
    return apt_vector (left);
}

ENTITY *
assign_submatrix_integer (MATRIX * left, int row, int col, int e)
{
    /*
     * Assign the integer `e' to the specified `row' and `col' of `left'.
     * The matrix `left' must have integer type.
     */

    int i, k, kk;

    EASSERT( left, matrix, integer );

    /*
     * Here we do a dangerous thing.  If `left' has ref_count > 1,
     * we assume that one of those is being kept in case we raise an
     * exception.  If we're careful not to raise an exception, we can
     * modify that copy and no one will ever know...
     */

    if ( left->entity.ref_count > 2 ) {
	ENTITY *stuff = left->stuff ? copy_table( left->stuff ) : NULL;
	left = (MATRIX *) dup_matrix( left );
	left->stuff = (TABLE *) stuff;
    }

    switch ( left->density ) {
      case dense:
	left->a.integer[row-1+left->nr*(col-1)] = e;
	break;
      case sparse_upper:
	if ( row == col ) {
	    if ( e != 0 ) {
		if ( left->d.integer == NULL )
		  left->d.integer = (int *) CALLOC( left->nr, sizeof(int) );
		left->d.integer[row-1] = e;
	    } else {
		if ( left->d.integer != NULL ) left->d.integer[row-1] = e;
	    }
	    break;
	}
	left = (MATRIX *) full_matrix( left ); 	/* Fall through. */
      case sparse:
	if ( left->nn > 0 ) {
	    kk = left->ia[row-1]-1;
	    for ( k=kk; k<left->ia[row]-1; k++ ) {
		if ( left->ja[k] > col && left->order == ordered ) {
		    k = left->ia[row]-1;
		    break;
		} else if ( left->ja[k] < col ) {
		    kk = k + 1;
		} else if ( left->ja[k] == col ) {
		    left->a.integer[k] = e;
		    break;
		}
	    }
	    if ( k == left->ia[row]-1 && e != 0 ) {
		left->ja =
		  (int *) REALLOC( left->ja, (left->nn+1)*sizeof(int) );
		left->a.integer =
		  (int *) REALLOC( left->a.integer, (left->nn+1)*sizeof(int) );
		if ( left->nn > kk ) {
		    memmove_forward( left->ja+kk+1, left->ja+kk,
				     (left->nn-kk)*sizeof(int) );
		    memmove_forward( left->a.integer+kk+1,
				     left->a.integer+kk,
				     (left->nn-kk)*sizeof(int) );
		}
		for ( i=row; i<left->nr+1; i++ ) left->ia[i]++;
		left->a.integer[kk] = e;
		left->ja[kk] = col;
		left->nn++;
	    }
	} else if ( e != 0 ) {
	    left->ia = (int *) MALLOC( (left->nr+1)*sizeof(int) );
	    for ( i=0; i<row; i++ ) left->ia[i] = 1;
	    for ( i=row; i<=left->nr; i++ ) left->ia[i] = 2;
	    left->ja = (int *) MALLOC( sizeof(int) );
	    left->ja[0] = col;
	    left->a.integer = (int *) MALLOC( sizeof(int) );
	    left->a.integer[0] = e;
	    left->nn++;
	}
	break;
      default:
	BAD_DENSITY( left->density );
	delete_matrix( left );
	raise_exception();
    }

    if ( row != col ) left->symmetry = general;

    return( apt_matrix( left ) );
}

ENTITY *
assign_submatrix_real (MATRIX * left, int row, int col, REAL e)
{
    /*
     * Assign the real `e' to the specified `row' and `col' of `left'.
     * The matrix `left' must have real type.
     */

    int i, k, kk;

    EASSERT( left, matrix, real );

    /*
     * Here we do a dangerous thing.  If `left' has ref_count > 1,
     * we assume that one of those is being kept in case we raise an
     * exception.  If we're careful not to raise an exception, we can
     * modify that copy and no one will ever know...
     */

    if ( left->entity.ref_count > 2 ) {
	ENTITY *stuff = left->stuff ? copy_table( left->stuff ) : NULL;
	left = (MATRIX *) dup_matrix( left );
	left->stuff = (TABLE *) stuff;
    }


    switch ( left->density ) {
      case dense:
	left->a.real[row-1+left->nr*(col-1)] = e;
	break;
      case sparse_upper:
	if ( row == col ) {
	    if ( e != 0.0 ) {
		if ( left->d.real == NULL )
		  left->d.real = (REAL *) CALLOC( left->nr, sizeof(REAL) );
		left->d.real[row-1] = e;
	    } else {
		if ( left->d.real != NULL ) left->d.real[row-1] = e;
	    }
	    break;
	}
	left = (MATRIX *) full_matrix( left ); 	/* Fall through. */
      case sparse:
	if ( left->nn > 0 ) {
	    kk = left->ia[row-1]-1;
	    for ( k=kk; k<left->ia[row]-1; k++ ) {
		if ( left->ja[k] > col && left->order == ordered ) {
		    k = left->ia[row]-1;
		    break;
		} else if ( left->ja[k] < col ) {
		    kk = k + 1;
		} else if ( left->ja[k] == col ) {
		    left->a.real[k] = e;
		    break;
		}
	    }
	    if ( k == left->ia[row]-1 && e != 0.0 ) {
		left->ja =
		  (int *) REALLOC( left->ja, (left->nn+1)*sizeof(int) );
		left->a.real =
		  (REAL *) REALLOC( left->a.real, (left->nn+1)*sizeof(REAL) );
		if ( left->nn > kk ) {
		    memmove_forward( left->ja+kk+1, left->ja+kk,
				     (left->nn-kk)*sizeof(int) );
		    memmove_forward( left->a.real+kk+1,
				     left->a.real+kk,
				     (left->nn-kk)*sizeof(REAL) );
		}
		for ( i=row; i<left->nr+1; i++ ) left->ia[i]++;
		left->a.real[kk] = e;
		left->ja[kk] = col;
		left->nn++;
	    }
	} else if ( e != 0 ) {
	    left->ia = (int *) MALLOC( (left->nr+1)*sizeof(int) );
	    for ( i=0; i<row; i++ ) left->ia[i] = 1;
	    for ( i=row; i<=left->nr; i++ ) left->ia[i] = 2;
	    left->ja = (int *) MALLOC( sizeof(int) );
	    left->ja[0] = col;
	    left->a.real = (REAL *) MALLOC( sizeof(REAL) );
	    left->a.real[0] = e;
	    left->nn++;
	}
	break;
      default:
	BAD_DENSITY( left->density );
	delete_matrix( left );
	raise_exception();
    }

    if ( row != col ) left->symmetry = general;

    return( apt_matrix( left ) );
}

ENTITY *
assign_submatrix_complex (MATRIX * left, int row, int col, COMPLEX e)
{
    /*
     * Assign the complex `e' to the specified `row' and `col' of `left'.
     * The matrix `left' must have complex type.
     */

    int i, k, kk;
    
    EASSERT( left, matrix, complex );
    
    /*
     * Here we do a dangerous thing.  If `left' has ref_count > 1,
     * we assume that one of those is being kept in case we raise an
     * exception.  If we're careful not to raise an exception, we can
     * modify that copy and no one will ever know...
     */

    if ( left->entity.ref_count > 2 ) {
	ENTITY *stuff = left->stuff ? copy_table( left->stuff ) : NULL;
	left = (MATRIX *) dup_matrix( left );
	left->stuff = (TABLE *) stuff;
    }

    switch ( left->density ) {
      case dense:
	left->a.complex[row-1+left->nr*(col-1)] = e;
	break;
      case sparse_upper:
	if ( row == col ) {
	    if ( e.real != 0.0 || e.imag != 0.0 ) {
		if ( left->d.complex == NULL )
		  left->d.complex =
		    (COMPLEX *) CALLOC( left->nr, sizeof(COMPLEX) );
		left->d.complex[row-1] = e;
	    } else {
		if ( left->d.complex != NULL ) left->d.complex[row-1] = e;
	    }
	    break;
	}
	left = (MATRIX *) full_matrix( left ); 	/* Fall through. */
      case sparse:
	if ( left->nn > 0 ) {
	    kk = left->ia[row-1]-1;
	    for ( k=kk; k<left->ia[row]-1; k++ ) {
		if ( left->ja[k] > col && left->order == ordered ) {
		    k = left->ia[row]-1;
		    break;
		} else if ( left->ja[k] < col ) {
		    kk = k + 1;
		} else if ( left->ja[k] == col ) {
		    left->a.complex[k] = e;
		    break;
		}
	    }
	    if ( k == left->ia[row]-1 &&
		( e.real != 0.0 || e.imag != 0.0 ) ) {
		left->ja =
		  (int *) REALLOC( left->ja, (left->nn+1)*sizeof(int) );
		left->a.complex =
		  (COMPLEX *) REALLOC( left->a.complex,
				      (left->nn+1)*sizeof(COMPLEX) );
		if ( left->nn > kk ) {
		    memmove_forward( left->ja+kk+1, left->ja+kk,
				     (left->nn-kk)*sizeof(int) );
		    memmove_forward( left->a.complex+kk+1,
				     left->a.complex+kk,
				     (left->nn-kk)*sizeof(COMPLEX) );
		}
		for ( i=row; i<left->nr+1; i++ ) left->ia[i]++;
		left->a.complex[kk] = e;
		left->ja[kk] = col;
		left->nn++;
	    }
	} else if ( e.real != 0.0 || e.imag != 0.0 ) {
	    left->ia = (int *) MALLOC( (left->nr+1)*sizeof(int) );
	    for ( i=0; i<row; i++ ) left->ia[i] = 1;
	    for ( i=row; i<=left->nr; i++ ) left->ia[i] = 2;
	    left->ja = (int *) MALLOC( sizeof(int) );
	    left->ja[0] = col;
	    left->a.complex = (COMPLEX *) MALLOC( sizeof(COMPLEX) );
	    left->a.complex[0] = e;
	    left->nn++;
	}
	break;
      default:
	BAD_DENSITY( left->density );
	delete_matrix( left );
	raise_exception();
    }
    
    if ( row != col ) left->symmetry = general;
    
    return( apt_matrix( left ) );
}

ENTITY *
assign_submatrix_character (MATRIX * left, int row, int col, char * e)
{
    /*
     * Assign the string `e' to the specified `row' and `col' of `left'.
     * The matrix `left' must have character type.  You don't get `e' back
     * so dup_char it if you want to keep it.
     */

    int i, k, kk;

    EASSERT( left, matrix, character );

    /*
     * Here we do a dangerous thing.  If `left' has ref_count > 1,
     * we assume that one of those is being kept in case we raise an
     * exception.  If we're careful not to raise an exception, we can
     * modify that copy and no one will ever know...
     */

    if ( left->entity.ref_count > 2 ) {
	ENTITY *stuff = left->stuff ? copy_table( left->stuff ) : NULL;
	left = (MATRIX *) dup_matrix( left );
	left->stuff = (TABLE *) stuff;
    }

    switch ( left->density ) {
      case dense:
	ASSIGN_CHAR( left->a.character[row-1+left->nr*(col-1)], e );
	break;
      case sparse_upper:
	if ( row == col ) {
	    if ( *e ) {
		if ( left->d.character == NULL )
		  left->d.character =
		    (char **) CALLOC( left->nr, sizeof(char *) );
		for ( i=0; i<left->nr; i++ )
		  left->d.character[i] = NULL_string;
		left->d.character[row-1] = e;
	    } else {
		if ( left->d.character != NULL )
		  ASSIGN_CHAR( left->d.character[row-1], e );
	    }
	    break;
	}
	left = (MATRIX *) full_matrix( left ); 	/* Fall through. */
      case sparse:
	if ( left->nn > 0 ) {
	    kk = left->ia[row-1]-1;
	    for ( k=kk; k<left->ia[row]-1; k++ ) {
		if ( left->ja[k] > col && left->order == ordered ) {
		    k = left->ia[row]-1;
		    break;
		} else if ( left->ja[k] < col ) {
		    kk = k + 1;
		} else if ( left->ja[k] == col ) {
		    ASSIGN_CHAR( left->a.character[k], e );
		    break;
		}
	    }
	    if ( k == left->ia[row]-1 && *e ) {
		left->ja =
		  (int *) REALLOC( left->ja, (left->nn+1)*sizeof(int) );
		left->a.character =
		  (char **) REALLOC( left->a.character,
				    (left->nn+1)*sizeof(char *) );
		if ( left->nn > kk ) {
		    memmove_forward( left->ja+kk+1, left->ja+kk,
				     (left->nn-kk)*sizeof(int) );
		    memmove_forward( left->a.character+kk+1,
				     left->a.character+kk,
				     (left->nn-kk)*sizeof(char *) );
		}
		for ( i=row; i<left->nr+1; i++ ) left->ia[i]++;
		left->a.character[kk] = e;
		left->ja[kk] = col;
		left->nn++;
	    }
	} else if ( *e ) {
	    left->ia = (int *) MALLOC( (left->nr+1)*sizeof(int) );
	    for ( i=0; i<row; i++ ) left->ia[i] = 1;
	    for ( i=row; i<=left->nr; i++ ) left->ia[i] = 2;
	    left->ja = (int *) MALLOC( sizeof(int) );
	    left->ja[0] = col;
	    left->a.character = (char **) MALLOC( sizeof(char *) );
	    left->a.character[0] = e;
	    left->nn++;
	}
	break;
      default:
	BAD_DENSITY( left->density );
	delete_matrix( left );
	raise_exception();
    }

    if ( row != col ) left->symmetry = general;

    return( apt_matrix( left ) );
}

ENTITY *
assign_subvector (ENTITY *left, ENTITY *elem, ENTITY *right)
{
    /*
     * This routine assigns the elements of `right' to the selected elements
     * of `left'.  The entities `elem' specify the desired elements.
     * If `elem' is NULL, then all elements are indicated.
     *
     * If `elem' has character type, then we look it up in left->eid.
     */
    
    int ne = 0;
    int i, left_ne;
    int *e;
    int *etmp = NULL;
    
    static char msg_bad_dim[] = "Inconsistent dimensions for a subvector assignment.  The number of elements specified was %d, but the right hand side has %d element%s.";
    static char msg_bad_spec[] = "Invalid element specification for subvector assignment.  The left hand side has %d element%s, but element %d was specified.";
    
    EASSERT( left, 0, 0 );
    EASSERT( right, 0, 0 );
    
    WITH_HANDLING {
	
	if ( left->class != vector ) left = vector_entity( EAT( left ) );
	left_ne = ((VECTOR *)left)->ne;
	
	/* If `elem' is character, use it as index into left->eid. */

	if ( elem &&
	    ( elem->class == scalar && ((SCALAR *)elem)->type == character ||
	     elem->class == vector && ((VECTOR *)elem)->type == character ||
	     elem->class == matrix && ((MATRIX *)elem)->type == character ) ) {
	    if ( ((VECTOR *)left)->eid ) {
		if ( elem->class == scalar ) {
		    elem = bi_find( EAT( elem ),
				    copy_entity( ((VECTOR *)left)->eid ) );
		    if ( ((VECTOR *)elem)->ne != 1 ) {
			fail( "Specified element label %s in subvector assignment operation.",
			     ( ((VECTOR *)elem)->ne > 1 ) ?
			     "is not unique" : "does not exist" );
			raise_exception();
		    }
		} else {
		    elem = bi_find( EAT( elem ),
				    copy_entity( ((VECTOR *)left)->eid ) );
		}
	    } else {
		fail( "Element labels don't exist for subvector assignment." );
		raise_exception();
	    }
	}

	/* Convert `elem' to an integer scalar or vector. */
	
	if ( elem ) {
	    if ( elem->class == scalar ) {
		if ( ((SCALAR *)elem)->type != integer )
		  elem = cast_scalar( (SCALAR *) EAT( elem ), integer );
		e = &(((SCALAR *)elem)->v.integer);
	    } else {
		if ( elem->class != vector )
		  elem = vector_entity( EAT( elem ) );
		if ( ((VECTOR *)elem)->type != integer )
		  elem = cast_vector( (VECTOR *) EAT( elem ), integer );
		if ( ((VECTOR *)elem)->density != dense )
		  elem = dense_vector( (VECTOR *) EAT( elem ) );
		e = ((VECTOR *)elem)->a.integer;
		ne = ((VECTOR *)elem)->ne;
	    }
	} else {
	    ne = left_ne;
            etmp = e = MALLOC( ne * sizeof(int) );
            for ( i=0; i<ne; i++ ) e[i] = i+1;
	}

	/* Convert `right' to appropriate class. */

	switch ( elem ? elem->class : vector ) {

	  case scalar:
	    ne = 1;
	    right = scalar_entity( EAT( right ) );
	    break;

	  case vector:
	    right = ( right->class == scalar ) ?
	      bi_fill( int_to_scalar( elem ? ((VECTOR *)elem)->ne :
				      left_ne ),
		       EAT( right ) ) :
		vector_entity( EAT( right ) );
	    ne = ((VECTOR *)right)->ne;
	    break;

	  default:
	    fail( "Invalid (%s) class as element specifier in subvector assignment.",
		  class_string[ elem->class ] );
	    raise_exception();
	}

	/* Check dimensions. */

	if ( elem ) {
	    if ( elem->class == scalar ) {
		if ( ne != 1 ) {
		    fail( msg_bad_dim, 1, ne, PLURAL( ne ) );
		    raise_exception();
		}
	    } else {
		if ( ne != ((VECTOR *)elem)->ne ) {
		    fail( msg_bad_dim, ((VECTOR *)elem)->ne, ne,
			  PLURAL( ne ) );
		    raise_exception();
		}
	    }
	    for ( i=0; i<ne; i++ ) {
		if ( e[i] < 1 || e[i] > left_ne ) {
		    fail( msg_bad_spec, left_ne, PLURAL( left_ne ), e[i] );
		    raise_exception();
		}
	    }
	} else {
	    if ( left_ne != ne ) {
		fail( msg_bad_dim, left_ne, ne, PLURAL( ne ) );
		raise_exception();
	    }
	}
	
	assert( left->class == vector );
	if ( right->class == scalar )
        {
	    AUTO_CAST_VECTOR_SCALAR( left, right );
	    
	    switch ( ((VECTOR *)left)->type ) {
	      case integer:
		left = assign_subvector_integer( (VECTOR *) EAT( left ),
			       ( elem == NULL ) ? 1 : e[0],
			       ((SCALAR *)right)->v.integer );
		break;
	      case real:
		left = assign_subvector_real( (VECTOR *) EAT( left ),
			       ( elem == NULL ) ? 1 : e[0],
			       ((SCALAR *)right)->v.real );
		break;
	      case complex:
		left = assign_subvector_complex( (VECTOR *) EAT( left ),
			       ( elem == NULL ) ? 1 : e[0],
			       ((SCALAR *)right)->v.complex );
		break;
	      case character:
		left = assign_subvector_character( (VECTOR *) EAT( left ),
			       ( elem == NULL ) ? 1 : e[0],
			       dup_char( ((SCALAR *)right)->v.character ) );
		break;
	      default:
		BAD_TYPE( ((VECTOR *)left)->type );
		raise_exception();
	    }

	} else {

	    assert( right->class == vector );

	    left = assign_vector_vector( (VECTOR *) EAT( left ), e,
                                         (VECTOR *) EAT( right ) );
	}
    }
    ON_EXCEPTION {
	delete_3_entities( left, elem, right );
        TFREE( etmp );
	raise_exception();
    }
    END_EXCEPTION;

    TFREE( etmp );
    delete_2_entities( elem, right );
    return( left );
}

ENTITY *
assign_subvector_integer (VECTOR *left, int elem, int e)
{
    int k, kk;

    EASSERT( left, vector, integer );

    /*
     * Here we do a dangerous thing.  If `left' has ref_count > 1,
     * we assume that one of those is being kept in case we raise an
     * exception.  If we're careful not to raise an exception, we can
     * modify that copy and no one will ever know...
     */

    if ( left->entity.ref_count > 2 ) {
	ENTITY *stuff = left->stuff ? copy_table( left->stuff ) : NULL;
	left = (VECTOR *) dup_vector( left );
	left->stuff = (TABLE *) stuff;
    }

    switch ( left->density ) {
      case dense:
	left->a.integer[elem-1] = e;
	break;
      case sparse:
	if ( left->nn > 0 ) {
	    kk = 0;
	    for ( k=0; k<left->nn; k++ ) {
		if ( left->ja[k] > elem && left->order == ordered ) {
		    k = left->nn;
		    break;
		} else if ( left->ja[k] < elem ) {
		    kk = k + 1;
		} else if ( left->ja[k] == elem ) {
		    left->a.integer[k] = e;
		    break;
		}
	    }
	    if ( k == left->nn && e != 0 ) {
		left->ja =
		  (int *) REALLOC( left->ja, (left->nn+1)*sizeof(int) );
		left->a.integer =
		  (int *) REALLOC( left->a.integer, (left->nn+1)*sizeof(int) );
		if ( left->nn > kk ) {
		    memmove_forward( left->ja+kk+1, left->ja+kk,
				     (left->nn-kk)*sizeof(int) );
		    memmove_forward( left->a.integer+kk+1,
				     left->a.integer+kk,
				     (left->nn-kk)*sizeof(int) );
		}
		left->a.integer[kk] = e;
		left->ja[kk] = elem;
		left->nn++;
	    }
	} else if ( e != 0 ) {
	    left->ja = (int *) MALLOC( sizeof(int) );
	    left->ja[0] = elem;
	    left->a.integer = (int *) MALLOC( sizeof(int) );
	    left->a.integer[0] = e;
	    left->nn++;
	}
	break;
      default:
	BAD_DENSITY( left->density );
	delete_vector( left );
	raise_exception();
    }

    return( apt_vector( left ) );
}

ENTITY *
assign_subvector_real (VECTOR * left, int elem, REAL e)
{
    int k, kk;

    EASSERT( left, vector, real );

    /*
     * Here we do a dangerous thing.  If `left' has ref_count > 1,
     * we assume that one of those is being kept in case we raise an
     * exception.  If we're careful not to raise an exception, we can
     * modify that copy and no one will ever know...
     */

    if ( left->entity.ref_count > 2 ) {
	ENTITY *stuff = left->stuff ? copy_table( left->stuff ) : NULL;
	left = (VECTOR *) dup_vector( left );
	left->stuff = (TABLE *) stuff;
    }

    switch ( left->density ) {
      case dense:
	left->a.real[elem-1] = e;
	break;
      case sparse:
	if ( left->nn > 0 ) {
	    kk = 0;
	    for ( k=0; k<left->nn; k++ ) {
		if ( left->ja[k] > elem && left->order == ordered ) {
		    k = left->nn;
		    break;
		} else if ( left->ja[k] < elem ) {
		    kk = k + 1;
		} else if ( left->ja[k] == elem ) {
		    left->a.real[k] = e;
		    break;
		}
	    }
	    if ( k == left->nn && e != 0 ) {
		left->ja =
		  (int *) REALLOC( left->ja, (left->nn+1)*sizeof(int) );
		left->a.real =
		  (REAL *) REALLOC( left->a.real, (left->nn+1)*sizeof(REAL) );
		if ( left->nn > kk ) {
		    memmove_forward( left->ja+kk+1, left->ja+kk,
				     (left->nn-kk)*sizeof(int) );
		    memmove_forward( left->a.real+kk+1,
				     left->a.real+kk,
				     (left->nn-kk)*sizeof(REAL) );
		}
		left->a.real[kk] = e;
		left->ja[kk] = elem;
		left->nn++;
	    }
	} else if ( e != 0 ) {
	    left->ja = (int *) MALLOC( sizeof(int) );
	    left->ja[0] = elem;
	    left->a.real = (REAL *) MALLOC( sizeof(REAL) );
	    left->a.real[0] = e;
	    left->nn++;
	}
	break;
      default:
	BAD_DENSITY( left->density );
	delete_vector( left );
	raise_exception();
    }

    return( apt_vector( left ) );
}

ENTITY *
assign_subvector_complex (VECTOR * left, int elem, COMPLEX e)
{
    int k, kk;

    EASSERT( left, vector, complex );

    /*
     * Here we do a dangerous thing.  If `left' has ref_count > 1,
     * we assume that one of those is being kept in case we raise an
     * exception.  If we're careful not to raise an exception, we can
     * modify that copy and no one will ever know...
     */

    if ( left->entity.ref_count > 2 ) {
	ENTITY *stuff = left->stuff ? copy_table( left->stuff ) : NULL;
	left = (VECTOR *) dup_vector( left );
	left->stuff = (TABLE *) stuff;
    }

    switch ( left->density ) {
      case dense:
	left->a.complex[elem-1] = e;
	break;
      case sparse:
	if ( left->nn > 0 ) {
	    kk = 0;
	    for ( k=0; k<left->nn; k++ ) {
		if ( left->ja[k] > elem && left->order == ordered ) {
		    k = left->nn;
		    break;
		} else if ( left->ja[k] < elem ) {
		    kk = k + 1;
		} else if ( left->ja[k] == elem ) {
		    left->a.complex[k] = e;
		    break;
		}
	    }
	    if ( k == left->nn && ( e.real != 0.0 || e.imag != 0.0 ) ) {
		left->ja =
		  (int *) REALLOC( left->ja, (left->nn+1)*sizeof(int) );
		left->a.complex =
		  (COMPLEX *) REALLOC( left->a.complex,
				      (left->nn+1)*sizeof(COMPLEX) );
		if ( left->nn > kk ) {
		    memmove_forward( left->ja+kk+1, left->ja+kk,
				     (left->nn-kk)*sizeof(int) );
		    memmove_forward( left->a.complex+kk+1,
				     left->a.complex+kk,
				     (left->nn-kk)*sizeof(COMPLEX) );
		}
		left->a.complex[kk] = e;
		left->ja[kk] = elem;
		left->nn++;
	    }
	} else if ( e.real != 0.0 || e.imag != 0.0 ) {
	    left->ja = (int *) MALLOC( sizeof(int) );
	    left->ja[0] = elem;
	    left->a.complex = (COMPLEX *) MALLOC( sizeof(COMPLEX) );
	    left->a.complex[0] = e;
	    left->nn++;
	}
	break;
      default:
	BAD_DENSITY( left->density );
	delete_vector( left );
	raise_exception();
    }

    return( apt_vector( left ) );
}

ENTITY *
assign_subvector_character (VECTOR * left, int elem, char * e)
{
    int k, kk;

    EASSERT( left, vector, character );

    /*
     * Here we do a dangerous thing.  If `left' has ref_count > 1,
     * we assume that one of those is being kept in case we raise an
     * exception.  If we're careful not to raise an exception, we can
     * modify that copy and no one will ever know...
     */

    if ( left->entity.ref_count > 2 ) {
	ENTITY *stuff = left->stuff ? copy_table( left->stuff ) : NULL;
	left = (VECTOR *) dup_vector( left );
	left->stuff = (TABLE *) stuff;
    }

    switch ( left->density ) {
      case dense:
	ASSIGN_CHAR( left->a.character[elem-1], e );
	break;
      case sparse:
	if ( left->nn > 0 ) {
	    kk = 0;
	    for ( k=0; k<left->nn; k++ ) {
		if ( left->ja[k] > elem && left->order == ordered ) {
		    k = left->nn;
		    break;
		} else if ( left->ja[k] < elem ) {
		    kk = k + 1;
		} else if ( left->ja[k] == elem ) {
		    ASSIGN_CHAR( left->a.character[k], e );
		    break;
		}
	    }
	    if ( k == left->nn && *e ) {
		left->ja =
		  (int *) REALLOC( left->ja, (left->nn+1)*sizeof(int) );
		left->a.character =
		  (char **) REALLOC( left->a.character,
				     (left->nn+1)*sizeof(char *) );
		if ( left->nn > kk ) {
		    memmove_forward( left->ja+kk+1, left->ja+kk,
				     (left->nn-kk)*sizeof(int) );
		    memmove_forward( left->a.character+kk+1,
				     left->a.character+kk,
				     (left->nn-kk)*sizeof(char *) );
		}
		left->a.character[kk] = e;
		left->ja[kk] = elem;
		left->nn++;
	    }
	} else if ( *e ) {
	    left->ja = (int *) MALLOC( sizeof(int) );
	    left->ja[0] = elem;
	    left->a.character = (char **) MALLOC( sizeof(char *) );
	    left->a.character[0] = e;
	    left->nn++;
	}
	break;
      default:
	BAD_DENSITY( left->density );
	delete_vector( left );
	raise_exception();
    }

    return( apt_vector( left ) );
}
