***************************** -*- Mode: Fortran -*- ***************************
*
* blas.f -- Interface routines for BLAS.
*
* Copyright (C) 1994-95  K. Scott Hunziker.
* Copyright (C) 1990-94  The Boeing Company.
*
* See the file COPYING for license, warranty, and permission details.
*
* 	The BLAS routines have the unfortunate convention of passing
*	character strings to specify options.  This makes it very difficult
*	to make portable calls to BLAS from C routines.  The BLAS
*	convention is that only the first character is significant.  The
*	routines below simply call their corresponding BLAS routines, but
*	they take integers in place of the character strings.  The integers
*	specify the ASCII value of the characters to be passed on to BLAS.
*
* $Id: blas.f,v 1.1.1.1 1996/04/17 05:56:15 ksh Exp $
*
*************************************************************************
*
      SUBROUTINE RGEMM( ITRNSA, ITRNSB, M, N, K, ALPHA, A, LDA, B, LDB,
     $                  BETA, C, LDC )
*
      INTEGER ITRNSA, ITRNSB, M, N, K, LDA, LDB, LDC
      DOUBLE PRECISION ALPHA, BETA
      DOUBLE PRECISION A(LDA,*), B(LDB,*), C(LDC,*)
*
      CALL DGEMM ( CHAR(ITRNSA), CHAR(ITRNSB), M, N, K, ALPHA, A, LDA,
     $             B, LDB, BETA, C, LDC )
*
      RETURN
      END
*
*************************************************************************
*
      SUBROUTINE XGEMM( ITRNSA, ITRNSB, M, N, K, ALPHA, A, LDA, B, LDB,
     $                  BETA, C, LDC )
*
      INTEGER ITRNSA, ITRNSB, M, N, K, LDA, LDB, LDC
      COMPLEX*16 ALPHA, BETA
      COMPLEX*16 A(LDA,*), B(LDB,*), C(LDC,*)
*
      CALL ZGEMM ( CHAR(ITRNSA), CHAR(ITRNSB), M, N, K, ALPHA, A, LDA,
     $             B, LDB, BETA, C, LDC )
*
      RETURN
      END
*
