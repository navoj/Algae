/*
   matio.c -- Read and write binary MATLAB files.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
*/

static char rcsid[] =
  "$Id: matio.c,v 1.3 2003/08/01 04:57:47 ksh Exp $";

#include "matio.h"
#include "get.h"
#include "put.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "function.h"
#include "file_io.h"
#include "vargs.h"
#include "cast.h"
#include "dense.h"

#if NO_PROTOS
typedef size_t (*READER)();
#else
typedef size_t (*READER)( char *ptr, size_t n, FILE *stream );
#endif

#define READER_INTS( ptr, n, s ) \
  ( errno = 0, \
    ( (*reader_int)( (char *) (ptr), n, s ) < (n) ) ? \
         ( READ_WARN( s ), 0 ) : 1 )

#define READER_DOUBLES( ptr, n, s ) \
  ( errno = 0, \
    ( (*reader_double)( (char *) (ptr), n, s ) < (n) ) ? \
         ( READ_WARN( s ), 0 ) : 1 )

#define READER_INT( ptr, s )	READER_INTS( ptr, 1, s )
#define READER_DOUBLE( ptr, s )	READER_DOUBLES( ptr, 1, s )

static int PROTO( putmat_entity, ( char *aname, ENTITY *x, va_list arg ) );

ENTITY *
bi_getmat (int n, ENTITY *fname)
{
    TABLE *t;
    ENTITY *e;
    char *name;

    /*
     * This routine reads MATLAB (v4) matrices from a file, and returns
     * them in a table.  If `fname' is NULL, then we read from stdin.
     */

    FILE *stream = fname ? find_file( entity_to_string( fname ),
				      FILE_INPUT ) : stdin;

    if ( !stream ) return NULL;

    t = (TABLE *) make_table();

    while ( e = getmat_matrix( stream, &name ) )
      t = (TABLE *) replace_in_table( t, e, name );

    return ENT( t );
}

ENTITY *
getmat_matrix (FILE *stream, char **name)
{
    /*
     * Read a MATLAB matrix from binary file `stream'.  Return its
     * name in `name'.
     */

    MATRIX *m = NULL;

    READER reader_int, reader_double;
    int type_flag, machine_flag, order_flag, precision_flag, text_flag;
    int mrows, ncols, imagf, namelen;
    
    /*
     * Read the type flag.  Looks like MATLAB just writes
     * this 4-byte integer in whatever format is handy.  
     */

#ifdef CRAY
    if (fread_int_cray ((char *) &type_flag, 1, stream) < 1) return NULL;
#else
    if (fread (&type_flag, 4, 1, stream) < 1) return NULL;
#endif

    /* Perhaps we need to reverse the bytes. */

    if ( type_flag < 0 || type_flag > 4999 )
        reverse_words( (char *) &type_flag, 4, 1 );

    text_flag = type_flag % 10;
    type_flag = ( type_flag - text_flag ) / 10;
    precision_flag = type_flag % 10;
    type_flag = ( type_flag - precision_flag ) / 10;
    order_flag = type_flag % 10;
    type_flag = ( type_flag - order_flag ) / 10;
    machine_flag = type_flag % 10;
    type_flag = ( type_flag - machine_flag ) / 10;

    /* Set up for binary translation. */

    if ( machine_flag == BINARY_FORMAT )
      {
	reader_int = (READER) fread_int;
	reader_double = (READER) fread_double;
      }
    else
      {
#ifdef CRAY
	if (machine_flag == IEEE_BIG_ENDIAN)
	  {
	    reader_int = (READER) fread_int_cray;
	    reader_double = (READER) fread_double_cray;
	  }
	else if (machine_flag == IEEE_LITTLE_ENDIAN)
	  {
	    reader_int = (READER) fread_int_cray_reverse;
	    reader_double = (READER) fread_double_cray_reverse;
	  }
#else
	if ( machine_flag == IEEE_BIG_ENDIAN ||
	     machine_flag == IEEE_LITTLE_ENDIAN )
	  {
	    reader_int = (READER) fread_int_reverse;
	    reader_double = (READER) fread_double_reverse;
	  }
#endif
	else
	  {
            if (type_flag == 129612 || type_flag == 128059)
              warn ("Looks like a MATLAB 5.0 binary file, which Algae can't yet read.  Use MATLAB's \"-v4\" option to use the old format.");
            else
              warn( "Can't read non-IEEE format MATLAB binary file." );
	    return NULL;
	  }
      }

    /* Reject sparse format. */

    if ( text_flag == 2 ) {
	warn( "Can't read sparse MATLAB matrix." );
	return NULL;
    }

    /* Read other header info. */

    if ( !READER_INT( &mrows, stream ) ||
	 !READER_INT( &ncols, stream ) ||
	 !READER_INT( &imagf, stream ) ||
	 !READER_INT( &namelen, stream ) ) return NULL;

    /* Check for bad data. */
    
    if (order_flag ||
	text_flag < 0 || text_flag > 2 ||
	mrows < 0 || ncols < 0 ||
	imagf < 0 || imagf > 1 ||
	namelen < 1)
      {
	warn ("Invalid MATLAB binary file.");
	return NULL;
      }
    
    /* Form the matrix. */

    m = (MATRIX *) form_matrix( mrows, ncols,
			        imagf ? complex :
			          precision_flag > 1 ? integer : real,
			        dense );

    /* Read the name. */

    *name = MALLOC( namelen );
    errno = 0;
    if ( fread( *name, 1, namelen, stream ) < (size_t) namelen ) {
	READ_WARN( stream );
	goto err;
    }

    /* Read the data. */

    switch ( precision_flag ) {

      case 0:	/* double */

	if ( !READER_DOUBLES( m->a.real, m->nn, stream ) ) goto err;

	if ( imagf ) {

	    int i;
	    REAL *c = MALLOC( m->nn * sizeof( REAL ) );
	    if ( !READER_DOUBLES( c, m->nn, stream ) ) {
		FREE( c );
		goto err;
	    }

	    for ( i=m->nn-1; i>=0; i-- ) {
		m->a.complex[i].real = m->a.real[i];
		m->a.complex[i].imag = c[i];
	    }

	    FREE( c );

	}

	break;

      case 1:	/* float */

	{
	    int i;
	    float *c = MALLOC( m->nn * sizeof( int ) );

	    if ( !READER_INTS( c, m->nn, stream ) ) {
		FREE( c );
		goto err;
	    }

	    if ( imagf ) {

		for ( i=0; i<m->nn; i++ )
		  m->a.complex[i].real = (REAL) c[i];

		if ( !READER_INTS( c, m->nn, stream ) ) {
		    FREE( c );
		    goto err;
		}

		for ( i=0; i<m->nn; i++ )
		  m->a.complex[i].imag = (REAL) c[i];

	    } else {

		for ( i=0; i<m->nn; i++ )
		  m->a.real[i] = (REAL) c[i];

	    }

	    FREE( c );

	}

	break;

      case 2:	/* integer */

	if ( imagf ) {

	    int i;
	    int *c = MALLOC( m->nn * sizeof( int ) );

	    if ( !READER_INTS( c, m->nn, stream ) ) {
		FREE( c );
		goto err;
	    }

	    for ( i=0; i<m->nn; i++ )
	      m->a.complex[i].real = (REAL) c[i];

	    if ( !READER_INTS( c, m->nn, stream ) ) {
		FREE( c );
		goto err;
	    }

	    for ( i=0; i<m->nn; i++ )
	      m->a.complex[i].imag = (REAL) c[i];

	    FREE( c );

	} else {

	    if ( !READER_INTS( m->a.integer, m->nn, stream ) ) goto err;

	}

	break;

      case 3:	/* short int */

	{
	    int i;
	    short int *c = MALLOC( m->nn * 2 );

	    if ( imagf ) {

		errno = 0;
		if ( fread( c, 2, m->nn, stream ) < (size_t) m->nn ) {
		    FREE( c );
		    READ_WARN( stream );
		    goto err;
		}
		if ( machine_flag != BINARY_FORMAT )
		  reverse_words( (char *) c, 2, m->nn );

		for ( i=0; i<m->nn; i++ )
		  m->a.complex[i].real = (REAL) c[i];

		errno = 0;
		if ( fread( c, 2, m->nn, stream ) < (size_t) m->nn ) {
		    FREE( c );
		    READ_WARN( stream );
		    goto err;
		}
		if ( machine_flag != BINARY_FORMAT )
		  reverse_words( (char *) c, 2, m->nn );

		for ( i=0; i<m->nn; i++ )
		  m->a.complex[i].imag = (REAL) c[i];

	    } else {

		errno = 0;
		if ( fread( c, 2, m->nn, stream ) < (size_t) m->nn ) {
		    FREE( c );
		    READ_WARN( stream );
		    goto err;
		}
		if ( machine_flag != BINARY_FORMAT )
		  reverse_words( (char *) c, 2, m->nn );

		for ( i=0; i<m->nn; i++ )
		  m->a.real[i] = (REAL) c[i];

	    }

	    FREE( c );

	}

	break;

      case 4:	/* unsigned short int */

	{
	    int i;
	    unsigned short int *c = MALLOC( m->nn * 2 );

	    if ( imagf ) {

		errno = 0;
		if ( fread( c, 2, m->nn, stream ) < (size_t) m->nn ) {
		    FREE( c );
		    READ_WARN( stream );
		    goto err;
		}
		if ( machine_flag != BINARY_FORMAT )
		  reverse_words( (char *) c, 2, m->nn );

		for ( i=0; i<m->nn; i++ )
		  m->a.complex[i].real = (REAL) c[i];

		errno = 0;
		if ( fread( c, 2, m->nn, stream ) < (size_t) m->nn ) {
		    FREE( c );
		    READ_WARN( stream );
		    goto err;
		}
		if ( machine_flag != BINARY_FORMAT )
		  reverse_words( (char *) c, 2, m->nn );

		for ( i=0; i<m->nn; i++ )
		  m->a.complex[i].imag = (REAL) c[i];

	    } else {

		errno = 0;
		if ( fread( c, 2, m->nn, stream ) < (size_t) m->nn ) {
		    FREE( c );
		    READ_WARN( stream );
		    goto err;
		}
		if ( machine_flag != BINARY_FORMAT )
		  reverse_words( (char *) c, 2, m->nn );

		for ( i=0; i<m->nn; i++ )
		  m->a.real[i] = (REAL) c[i];

	    }

	    FREE( c );

	}

	break;

      case 5:	/* char */

	{
	    int i;
	    char *c = MALLOC( m->nn );

	    if ( imagf ) {

		errno = 0;
		if ( fread( c, 2, m->nn, stream ) < (size_t) m->nn ) {
		    FREE( c );
		    READ_WARN( stream );
		    goto err;
		}

		for ( i=0; i<m->nn; i++ )
		  m->a.complex[i].real = (REAL) c[i];

		errno = 0;
		if ( fread( c, 2, m->nn, stream ) < (size_t) m->nn ) {
		    FREE( c );
		    READ_WARN( stream );
		    goto err;
		}

		for ( i=0; i<m->nn; i++ )
		  m->a.complex[i].imag = (REAL) c[i];

	    } else {

		errno = 0;
		if ( fread( c, 2, m->nn, stream ) < (size_t) m->nn ) {
		    FREE( c );
		    READ_WARN( stream );
		    goto err;
		}

		for ( i=0; i<m->nn; i++ )
		  m->a.real[i] = (REAL) c[i];

	    }

	    FREE( c );

	}

	break;

      default:	/* invalid */

	warn( "Invalid precision in MATLAB file." );
	goto err;

    }

    /* Convert to string if MATLAB says so. */

    if ( text_flag ) {
	int i, j;
	MATRIX *mm = (MATRIX *) form_matrix( m->nr, 1, character, dense );
	assert( m->density == dense );
	m = (MATRIX *) cast_matrix( m, integer );
	for ( i=0; i<m->nr; i++ ) {
	    mm->a.character[i] = MALLOC( m->nc + 1 );
	    for ( j=0; j<m->nc; j++ )
	      mm->a.character[i][j] = (char) ( m->a.integer[i+m->nr*j] % 256 );
	    mm->a.character[i][m->nc] = '\0';
	}
	delete_matrix( m );
	m = mm;
    }

    return ENT( m );

  err:

    FREE( *name );
    *name = NULL;
    delete_matrix( m );
    return NULL;
}

void
reverse_words (char *p, int size, int n)
{
    char t[4];
    int i, j;

    assert( size <= 4 );

    for ( i=0; i<n; i++ ) {
	for ( j=0; j<size; j++ ) t[j] = p[size-j-1];
	memcpy( p, t, size );
	p += size;
    }
}

ENTITY *
bi_putmat (int n, ENTITY *t, ENTITY *fname)
{
    /*
     * This routine writes a table to a binary MATLAB file.
     *
     * If `fname' is not given, then we write to stdout.  If `fname'
     * can't be taken as a scalar or `t' isn't a table then we'll
     * raise an exception.  Otherwise we return NULL on error and 1
     * otherwise.
     */
    
    FILE * volatile stream;
    
    EASSERT( t, 0, 0 );
    
    WITH_HANDLING {
	stream = fname ? find_file( entity_to_string( EAT( fname ) ),
				    FILE_OUTPUT ) : stdout;

	if ( t->class != table ) {
	    fail( "Not a table." );
	    raise_exception();
	}
    }
    ON_EXCEPTION {
	delete_2_entities( t, fname );
    }
    END_EXCEPTION;

    if ( !stream ) {
	delete_entity( t );
	return NULL;
    }

    return visit_table( (TABLE *) t, putmat_entity, stream ) ?
      int_to_scalar(1) : NULL;
}

static int
putmat_entity( char *aname, ENTITY *x, va_list arg )
{
    MATRIX *e;
    int type_flag, imagf;
    int nc, size, maxlen = 0;
    FILE *stream;

    stream = va_arg( arg, FILE * );

    e = (MATRIX *) dense_matrix( (MATRIX *) matrix_entity( x ) );

    imagf = 0;
    nc = e->nc;

    type_flag = 1000;
    switch ( e->type ) {
      case integer:
	type_flag += 20;
	break;
      case real:
	break;
      case complex:
	imagf = 1;
	break;
      case character:
	{
	    int i, len;

	    type_flag += 21;

	    if ( e->nc != 1 ) {
		fail( "MATLAB character matrix must have exactly one column.");
		delete_matrix( e );
		raise_exception();
	    }

	    /* find longest string */

	    for ( i=0; i<e->nr; i++ ) {
		len = strlen( e->a.character[i] );
		if ( len > maxlen ) maxlen = len;
	    }

	    nc = maxlen;
	}

	break;
      default:
	wipeout( "Bad type." );
    }

    size = strlen( aname ) + 1;

    if ( !WRITE_INT( &type_flag, stream ) ||
	 !WRITE_INT( &e->nr, stream ) ||
	 !WRITE_INT( &nc, stream ) ||
	 !WRITE_INT( &imagf, stream ) ||
	 !WRITE_INT( &size, stream ) ||
	 fwrite( aname, 1, size, stream ) < (size_t) size ) {
	delete_matrix( e );
	FREE_CHAR( aname );
	return 0;
    }

    switch ( e->type ) {

      case integer:

	if ( !WRITE_INTS( e->a.integer, e->nn, stream ) ) {
	    delete_matrix( e );
	    return 0;
	}
	break;

      case real:

	if ( !WRITE_DOUBLES( e->a.real, e->nn, stream ) ) {
	    delete_matrix( e );
	    return 0;
	}
	break;

      case complex:

	{
	    int i;
	    REAL *c = MALLOC( e->nn * sizeof( REAL ) );

	    for ( i=0; i<e->nn; i++ ) c[i] = e->a.complex[i].real;
	    if ( !WRITE_DOUBLES( c, e->nn, stream ) ) {
		delete_matrix( e );
		return 0;
	    }
	    for ( i=0; i<e->nn; i++ ) c[i] = e->a.complex[i].imag;
	    if ( !WRITE_DOUBLES( c, e->nn, stream ) ) {
		delete_matrix( e );
		return 0;
	    }

	    FREE( c );
	}

	break;

      case character:

	{
	    int i, j, len, c;

	    /* write the characters, padding with blanks */

	    for ( j=0; j<maxlen; j++ ) {
		for ( i=0; i<e->nr; i++ ) {
		    len = strlen( e->a.character[i] );
		    c = (int) ( ( j < len ) ? e->a.character[i][j] : ' ' );
		    if ( !WRITE_INT( &c, stream ) ) {
			delete_matrix( e );
			return 0;
		    }
		}
	    }

	}

	break;

      default:

	wipeout( "Bad type." );

    }

    delete_matrix( e );
    FREE_CHAR( aname );

    return 1;
}
