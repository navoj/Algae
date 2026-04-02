***************************** -*- Mode: Fortran -*- ********************
*
* lapack.f -- Interface routines for LAPACK.
* 
* Copyright (C) 1994-2001  K. Scott Hunziker.
* Copyright (C) 1990-1994  The Boeing Company.
*
* See the file COPYING for license, warranty, and permission details.
*
* The LAPACK routines have the unfortunate convention of passing
* character strings to specify options.  This makes it very difficult
* to make portable calls to LAPACK from C routines.  The LAPACK
* convention is that only the first character is significant.  The
* routines below simply call their corresponding LAPACK routines, but
* they take integers in place of the character strings.  The integers
* specify the ASCII value of the characters to be passed on to LAPACK.
*
* We can't let IEEECK generate floating point exceptions, so we have
* our own copy here.
*
* I also have a copy of XERBLA here, to replace the one in LAPACK.
*
************************************************************************
*
      SUBROUTINE RPOSV( IUPLO, N, NRHS, A, LDA, B, LDB, INFO )
*
      INTEGER IUPLO, INFO, LDA, LDB, N, NRHS
      DOUBLE PRECISION A(LDA,*), B(LDB,*)
*
      CALL DPOSV( CHAR( IUPLO ), N, NRHS, A, LDA, B, LDB, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RSYGV( ITYPE, IJOBZ, IUPLO, N, A, LDA, B, LDB, W,
     $                  WORK, LWORK, INFO )
*
      INTEGER ITYPE, IJOBZ, IUPLO, INFO, LDA, LDB, LWORK, N
      DOUBLE PRECISION A(LDA,*), B(LDB,*), W(*), WORK(*)
*
      CALL DSYGV( ITYPE, CHAR( IJOBZ ), CHAR( IUPLO ), N, A, LDA, B,
     $            LDB, W, WORK, LWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RGGEV( IJOBVL, IJOBVR, N, A, LDA, B, LDB,
     $                  ALPHAR, ALPHAI, BETA, VL, LDVL, VR, LDVR,
     $                  WORK, LWORK, INFO )
*
      INTEGER IJOBVL, IJOBVR, INFO, LDA, LDB, LDVL, LDVR, LWORK, N
      DOUBLE PRECISION A(LDA,*), B(LDB,*), WORK(*)
      DOUBLE PRECISION ALPHAR(*), ALPHAI(*), BETA(*)
      DOUBLE PRECISION VL(LDVL,*), VR(LDVR,*)
*
      CALL DGGEV( CHAR( IJOBVL ), CHAR( IJOBVR ), N, A, LDA, B, LDB,
     $            ALPHAR, ALPHAI, BETA, VL, LDVL, VR, LDVR,
     $            WORK, LWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RSYSV( IUPLO, N, NRHS, A, LDA, IPIV, B, LDB, WORK,
     $                  LWORK, INFO )
*
      INTEGER IUPLO, INFO, LDA, LDB, LWORK, N, NRHS
      INTEGER IPIV(*)
      DOUBLE PRECISION A(LDA,*), B(LDB,*), WORK(LWORK)
*
      CALL DSYSV( CHAR( IUPLO ), N, NRHS, A, LDA, IPIV, B, LDB, WORK,
     $            LWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XSYSV( IUPLO, N, NRHS, A, LDA, IPIV, B, LDB, WORK,
     $                  LWORK, INFO )
*
      INTEGER IUPLO, INFO, LDA, LDB, LWORK, N, NRHS
      INTEGER IPIV(*)
      COMPLEX*16 A(LDA,*), B(LDB,*), WORK(LWORK)
*
      CALL ZSYSV( CHAR( IUPLO ), N, NRHS, A, LDA, IPIV, B, LDB, WORK,
     $            LWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XHESV( IUPLO, N, NRHS, A, LDA, IPIV, B, LDB, WORK,
     $                  LWORK, INFO )
*
      INTEGER IUPLO, INFO, LDA, LDB, LWORK, N, NRHS
      INTEGER IPIV(*)
      COMPLEX*16 A(LDA,*), B(LDB,*), WORK(LWORK)
*
      CALL ZHESV( CHAR( IUPLO ), N, NRHS, A, LDA, IPIV, B, LDB, WORK,
     $            LWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XERBLA( SRNAME, INFO )
      CHARACTER*6 SRNAME
      INTEGER INFO
*
      RETURN
      END
*
*************************************************************************
*
      SUBROUTINE XPOSV( IUPLO, N, NRHS, A, LDA, B, LDB, INFO )
*
      INTEGER IUPLO, INFO, LDA, LDB, N, NRHS
      COMPLEX*16 A(LDA,*), B(LDB,*)
*
      CALL ZPOSV( CHAR( IUPLO ), N, NRHS, A, LDA, B, LDB, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RSYEV( IJOBZ, IUPLO, N, A, LDA, W, WORK, LWORK, INFO )
*
      INTEGER IJOBZ, IUPLO, INFO, LDA, LWORK, N
      DOUBLE PRECISION A(LDA,*), W(*), WORK(*)
*
      CALL DSYEV( CHAR( IJOBZ ), CHAR( IUPLO ), N, A, LDA, W, WORK,
     $            LWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RGEEV( IJOBVL, IJOBVR, N, A, LDA, WR, WI, VL, LDVL, VR,
     $     LDVR, WORK, LWORK, INFO )
*
      INTEGER IJOBVL, IJOBVR, INFO, LDA, LDVL, LDVR, LWORK, N
      DOUBLE PRECISION A(LDA,*), VL(LDVL,*), VR(LDVR,*),
     $     WI(*), WORK(*), WR(*)
*
      CALL DGEEV( CHAR( IJOBVL ), CHAR( IJOBVR ), N, A, LDA, WR, WI, VL,
     $     LDVL, VR, LDVR, WORK, LWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RSYTRF( IUPLO, N, A, LDA, IPIV, WORK, LWORK, INFO )
*
      INTEGER IUPLO, INFO, LDA, LWORK, N, IPIV(*)
      DOUBLE PRECISION A(LDA,*), WORK(LWORK)
*
      CALL DSYTRF( CHAR( IUPLO ), N, A, LDA, IPIV, WORK, LWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RPOTRF( IUPLO, N, A, LDA, INFO )
*
      INTEGER IUPLO, INFO, LDA, N
      DOUBLE PRECISION A(LDA,*)
*
      CALL DPOTRF( CHAR( IUPLO ), N, A, LDA, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RGETRS( ITRANS, N, NRHS, A, LDA, IPIV, B, LDB, INFO )
*
      INTEGER ITRANS, INFO, LDA, LDB, N, NRHS
      DOUBLE PRECISION A(LDA,*), B(LDB,*)
      INTEGER IPIV(*)
*
      CALL DGETRS( CHAR( ITRANS ), N, NRHS, A, LDA, IPIV, B, LDB, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XGETRS( ITRANS, N, NRHS, A, LDA, IPIV, B, LDB, INFO )
*
      INTEGER ITRANS, INFO, LDA, LDB, N, NRHS
      COMPLEX*16 A(LDA,*), B(LDB,*)
      INTEGER IPIV(*)
*
      CALL ZGETRS( CHAR( ITRANS ), N, NRHS, A, LDA, IPIV, B, LDB, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RPOTRS( IUPLO, N, NRHS, A, LDA, B, LDB, INFO )
*
      INTEGER IUPLO, INFO, LDA, LDB, N, NRHS
      DOUBLE PRECISION A(LDA,*), B(LDB,*)
*
      CALL DPOTRS( CHAR( IUPLO ), N, NRHS, A, LDA, B, LDB, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XPOTRS( IUPLO, N, NRHS, A, LDA, B, LDB, INFO )
*
      INTEGER IUPLO, INFO, LDA, LDB, N, NRHS
      COMPLEX*16 A(LDA,*), B(LDB,*)
*
      CALL ZPOTRS( CHAR( IUPLO ), N, NRHS, A, LDA, B, LDB, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RSYTRS( IUPLO, N, NRHS, A, LDA, IPIV, B, LDB, INFO )
*
      INTEGER IUPLO, INFO, LDA, LDB, N, NRHS
      INTEGER IPIV(*)
      DOUBLE PRECISION A(LDA,*), B(LDB,*)
*
      CALL DSYTRS( CHAR( IUPLO ), N, NRHS, A, LDA, IPIV, B, LDB, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XSYTRS( IUPLO, N, NRHS, A, LDA, IPIV, B, LDB, INFO )
*
      INTEGER IUPLO, INFO, LDA, LDB, N, NRHS
      INTEGER IPIV(*)
      COMPLEX*16 A(LDA,*), B(LDB,*)
*
      CALL ZSYTRS( CHAR( IUPLO ), N, NRHS, A, LDA, IPIV, B, LDB, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XHETRS( IUPLO, N, NRHS, A, LDA, IPIV, B, LDB, INFO )
*
      INTEGER IUPLO, INFO, LDA, LDB, N, NRHS
      INTEGER IPIV(*)
      COMPLEX*16 A(LDA,*), B(LDB,*)
*
      CALL ZHETRS( CHAR( IUPLO ), N, NRHS, A, LDA, IPIV, B, LDB, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RGESVD( IJOBU, IJOBVT, M, N, A, LDA, S,
     $                   U, LDU, VT, LDVT, WORK, LWORK, INFO )
*
      INTEGER IJOBU, IJOBVT, INFO, LDA, LDU, LDVT, LWORK, M, N
      DOUBLE PRECISION A(LDA,*), S(*), U(LDU,*), VT(LDVT,*), WORK(*)
*
      CALL DGESVD( CHAR( IJOBU ), CHAR( IJOBVT ), M, N, A, LDA, S,
     $             U, LDU, VT, LDVT, WORK, LWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XGESVD( IJOBU, IJOBVT, M, N, A, LDA, S,
     $                   U, LDU, VT, LDVT, WORK, LWORK, RWORK, INFO )
*
      INTEGER IJOBU, IJOBVT, INFO, LDA, LDU, LDVT, LWORK, M, N
      COMPLEX*16 A(LDA,*), U(LDU,*), VT(LDVT,*), WORK(*)
      DOUBLE PRECISION S(*), RWORK(*)
*
      CALL ZGESVD( CHAR( IJOBU ), CHAR( IJOBVT ), M, N, A, LDA, S,
     $             U, LDU, VT, LDVT, WORK, LWORK, RWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RPOCON( IUPLO, N, A, LDA, ANORM, RCOND,
     $                   WORK, IWORK, INFO )
*
      INTEGER IUPLO, INFO, LDA, N
      DOUBLE PRECISION ANORM, RCOND
      INTEGER IWORK(*)
      DOUBLE PRECISION A(LDA,*), WORK(*)
*
      CALL DPOCON( CHAR( IUPLO ), N, A, LDA, ANORM, RCOND,
     $             WORK, IWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XPOTRF( IUPLO, N, A, LDA, INFO )
*
      INTEGER IUPLO, INFO, LDA, N
      COMPLEX*16 A(LDA,*)
*
      CALL ZPOTRF( CHAR( IUPLO ), N, A, LDA, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XPOCON( IUPLO, N, A, LDA, ANORM, RCOND,
     $                   WORK, RWORK, INFO )
*
      INTEGER IUPLO, INFO, LDA, N
      DOUBLE PRECISION ANORM, RCOND
      DOUBLE PRECISION RWORK(*)
      COMPLEX*16 A(LDA,*), WORK(*)
*
      CALL ZPOCON( CHAR( IUPLO ), N, A, LDA, ANORM, RCOND,
     $             WORK, RWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RSYCON( IUPLO, N, A, LDA, IPIV, ANORM, RCOND,
     $                   WORK, IWORK, INFO )
*
      INTEGER IUPLO, INFO, LDA, N
      DOUBLE PRECISION ANORM, RCOND
      INTEGER IPIV(*), IWORK(*)
      DOUBLE PRECISION A(LDA,*), WORK(*)
*
      CALL DSYCON( CHAR( IUPLO ), N, A, LDA, IPIV, ANORM, RCOND,
     $             WORK, IWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XSYCON( IUPLO, N, A, LDA, IPIV, ANORM, RCOND,
     $                   WORK, INFO )
*
      INTEGER IUPLO, INFO, LDA, N
      DOUBLE PRECISION ANORM, RCOND
      INTEGER IPIV(*)
      COMPLEX*16 A(LDA,*), WORK(*)
*
      CALL ZSYCON( CHAR( IUPLO ), N, A, LDA, IPIV, ANORM, RCOND,
     $             WORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XHECON( IUPLO, N, A, LDA, IPIV, ANORM, RCOND,
     $                   WORK, INFO )
*
      INTEGER IUPLO, INFO, LDA, N
      DOUBLE PRECISION ANORM, RCOND
      INTEGER IPIV(*)
      COMPLEX*16 A(LDA,*), WORK(*)
*
      CALL ZHECON( CHAR( IUPLO ), N, A, LDA, IPIV, ANORM, RCOND,
     $             WORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RGECON( INORM, N, A, LDA, ANORM, RCOND,
     $                   WORK, IWORK, INFO )
*
      INTEGER INORM, INFO, LDA, N
      DOUBLE PRECISION ANORM, RCOND
      INTEGER IWORK(*)
      DOUBLE PRECISION A(LDA,*), WORK(*)
*
      CALL DGECON( CHAR( INORM ), N, A, LDA, ANORM, RCOND,
     $             WORK, IWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XGECON( INORM, N, A, LDA, ANORM, RCOND,
     $                   WORK, RWORK, INFO )
*
      INTEGER INORM, INFO, LDA, N
      DOUBLE PRECISION ANORM, RCOND
      DOUBLE PRECISION RWORK(*)
      COMPLEX*16 A(LDA,*), WORK(*)
*
      CALL ZGECON( CHAR( INORM ), N, A, LDA, ANORM, RCOND,
     $             WORK, RWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XGEEV( IJOBVL, IJOBVR, N, A, LDA, W, VL, LDVL, VR,
     $                  LDVR, WORK, LWORK, RWORK, INFO )
*
      INTEGER IJOBVL, IJOBVR, INFO, LDA, LDVL, LDVR, LWORK, N
      DOUBLE PRECISION RWORK(*)
      COMPLEX*16 A(LDA,*), VL(LDVL,*), VR(LDVR,*), W(*), WORK(*)
*
      CALL ZGEEV( CHAR(IJOBVL), CHAR(IJOBVR), N, A, LDA, W, VL, LDVL,
     $            VR, LDVR, WORK, LWORK, RWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XHEEV( IJOBZ, IUPLO, N, A, LDA, W, WORK, LWORK,
     $                  RWORK, INFO )
*
      INTEGER IJOBZ, IUPLO, INFO, LDA, LWORK, N
      DOUBLE PRECISION RWORK(*), W(*)
      COMPLEX*16 A(LDA,*), WORK(*)
*
      CALL ZHEEV( CHAR(IJOBZ), CHAR(IUPLO), N, A, LDA, W, WORK,
     $            LWORK, RWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XHBEV( IJOBZ, IUPLO, N, KD, AB, LDAB, W, Z, LDZ, WORK,
     $                  RWORK, INFO )
*
      INTEGER IJOBZ, IUPLO, INFO, KD, LDAB, LDZ, N
      DOUBLE PRECISION RWORK(*), W(*)
      COMPLEX*16 AB(LDAB,*), WORK(*), Z(LDZ,*)
*
      CALL ZHBEV( CHAR(IJOBZ), CHAR(IUPLO), N, KD, AB, LDAB, W, Z, LDZ,
     $            WORK, RWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RSBEV( IJOBZ, IUPLO, N, KD, AB, LDAB, W, Z, LDZ, WORK,
     $                  INFO )
*
      INTEGER IJOBZ, IUPLO, INFO, KD, LDAB, LDZ, N
      DOUBLE PRECISION AB(LDAB,*), W(*), WORK(*), Z(LDZ,*)
*
      CALL DSBEV( CHAR(IJOBZ), CHAR(IUPLO), N, KD, AB, LDAB, W, Z, LDZ,
     $            WORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XSYTRF( IUPLO, N, A, LDA, IPIV, WORK, LWORK, INFO )
*
      INTEGER IUPLO, INFO, LDA, LWORK, N, IPIV(*)
      COMPLEX*16 A(LDA,*), WORK(LWORK)
*
      CALL ZSYTRF( CHAR( IUPLO ), N, A, LDA, IPIV, WORK, LWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XHETRF( IUPLO, N, A, LDA, IPIV, WORK, LWORK, INFO )
*
      INTEGER IUPLO, INFO, LDA, LWORK, N, IPIV(*)
      COMPLEX*16 A(LDA,*), WORK(LWORK)
*
      CALL ZHETRF( CHAR( IUPLO ), N, A, LDA, IPIV, WORK, LWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RGELS( ITRANS, M, N, NRHS, A, LDA, B, LDB, WORK, LWORK,
     $     INFO )
*
      INTEGER ITRANS, INFO, LDA, LDB, LWORK, M, N, NRHS
      DOUBLE PRECISION A(LDA,*), B(LDB,*), WORK(LWORK)
*
      CALL DGELS( CHAR (ITRANS), M, N, NRHS, A, LDA, B, LDB, WORK,
     $     LWORK, INFO)
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XGELS( ITRANS, M, N, NRHS, A, LDA, B, LDB, WORK, LWORK,
     $     INFO )
*
      INTEGER ITRANS, INFO, LDA, LDB, LWORK, M, N, NRHS
      COMPLEX*16 A(LDA,*), B(LDB,*), WORK(LWORK)
*
      CALL ZGELS( CHAR (ITRANS), M, N, NRHS, A, LDA, B, LDB, WORK,
     $     LWORK, INFO)
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XHEGV( ITYPE, IJOBZ, IUPLO, N, A, LDA, B, LDB, W,
     $                  WORK, LWORK, RWORK, INFO )
*
      INTEGER ITYPE, IJOBZ, IUPLO, INFO, LDA, LDB, LWORK, N
      COMPLEX*16 A(LDA,*), B(LDB,*), WORK(*)
      DOUBLE PRECISION W(*), RWORK(*)
*
      CALL ZHEGV( ITYPE, CHAR( IJOBZ ), CHAR( IUPLO ), N, A, LDA, B,
     $            LDB, W, WORK, LWORK, RWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XGGEV( IJOBVL, IJOBVR, N, A, LDA, B, LDB,
     $                  ALPHA, BETA, VL, LDVL, VR, LDVR,
     $                  WORK, LWORK, RWORK, INFO )
*
      INTEGER IJOBVL, IJOBVR, INFO, LDA, LDB, LDVL, LDVR, LWORK, N
      COMPLEX*16 A(LDA,*), B(LDB,*), WORK(*)
      COMPLEX*16 ALPHA(*), BETA(*)
      COMPLEX*16 VL(LDVL,*), VR(LDVR,*)
      DOUBLE PRECISION RWORK(*)
*
      CALL ZGGEV( CHAR( IJOBVL ), CHAR( IJOBVR ), N, A, LDA, B, LDB,
     $            ALPHA, BETA, VL, LDVL, VR, LDVR,
     $            WORK, LWORK, RWORK, INFO )
*
      RETURN
      END
*
************************************************************************
*
      INTEGER          FUNCTION IEEECK( ISPEC, ZERO, ONE )
*
*     Algae specifies interrupts on floating point exceptions, so we
*     must return "not safe" here.
*
      INTEGER            ISPEC
      REAL               ONE, ZERO
*
*  Purpose
*  =======
*
*  IEEECK is called from the ILAENV to verify that Infinity and
*  possibly NaN arithmetic is safe (i.e. will not trap).
*
*  Arguments
*  =========
*
*  ISPEC   (input) INTEGER
*          Specifies whether to test just for inifinity arithmetic
*          or whether to test for infinity and NaN arithmetic.
*          = 0: Verify infinity arithmetic only.
*          = 1: Verify infinity and NaN arithmetic.
*
*  ZERO    (input) REAL
*          Must contain the value 0.0
*          This is passed to prevent the compiler from optimizing
*          away this code.
*
*  ONE     (input) REAL
*          Must contain the value 1.0
*          This is passed to prevent the compiler from optimizing
*          away this code.
*
*  RETURN VALUE:  INTEGER
*          = 0:  Arithmetic failed to produce the correct answers
*          = 1:  Arithmetic produced the correct answers
*
*     .. Executable Statements ..
*
      IEEECK = 0
      RETURN
      END
