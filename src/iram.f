***************************** -*- Mode: Fortran -*- ********************
*
* iram.f -- Wrappers for ARPACK.
* 
* Copyright (C) 2002  K. Scott Hunziker.
*
* See the file COPYING for license, warranty, and permission details.
*
* The ARPACK routines have the unfortunate convention of passing
* character strings to specify options.  This makes it very difficult
* to make portable calls to ARPACK from C routines.  The wrappers
* here take integers in place of the strings, then call the real
* ARPACK subroutines with the strings they expect.
*
************************************************************************
*
      SUBROUTINE RSAUPD (IDO, IBMAT, N, IWHICH, NEV, TOL, RESID, NCV, V, 
     *     LDV, IPARAM, IPNTR, WORKD, WORKL, LWORKL, INFO)
*
      INTEGER IDO, IBMAT, N, IWHICH, NEV, NCV, LDV, LWORKL, INFO
      INTEGER IPARAM(11), IPNTR(11)
      DOUBLE PRECISION TOL, RESID(N), V(LDV,NCV), WORKD(*), WORKL(*)
      CHARACTER*1 BMAT
      CHARACTER*2 WHICH
*
      IF (IBMAT .EQ. 1) THEN
         BMAT = 'I'
      ELSEIF (IBMAT .EQ. 2) THEN
         BMAT = 'G'
      ELSE
         BMAT = 'X'
      ENDIF
*
      IF (IWHICH .EQ. 1) THEN
         WHICH = 'LA'
      ELSEIF (IWHICH .EQ. 2) THEN
         WHICH = 'SA'
      ELSEIF (IWHICH .EQ. 3) THEN
         WHICH = 'LM'
      ELSEIF (IWHICH .EQ. 4) THEN
         WHICH = 'SM'
      ELSEIF (IWHICH .EQ. 5) THEN
         WHICH = 'BE'
      ELSE
         WHICH = 'XX'
      ENDIF
*
      CALL DSAUPD (IDO, BMAT, N, WHICH, NEV, TOL, RESID, NCV, V, 
     *     LDV, IPARAM, IPNTR, WORKD, WORKL, LWORKL, INFO)
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RSEUPD (IRVEC, IHOW, SELECT, D, Z, LDZ, SIGMA, IBMAT, 
     *     N, IWHICH, NEV, TOL, RESID, NCV, V, LDV, IPARAM, IPNTR, 
     *     WORKD, WORKL, LWORKL, INFO)
*
      INTEGER IRVEC, IHOW, IBMAT, N, IWHICH, NEV, NCV, LDV, LDZ
      INTEGER LWORKL, INFO
      INTEGER IPARAM(11), IPNTR(11)
      DOUBLE PRECISION RESID(N), Z(LDZ,*), V(LDV,NCV), WORKD(*)
      DOUBLE PRECISION TOL, SIGMA, D(NEV), WORKL(*)
*
      CHARACTER*1 BMAT, HOWMNY
      CHARACTER*2 WHICH
      LOGICAL SELECT(NCV), RVEC
*
      RVEC = IRVEC .EQ. 1
*
      IF (IBMAT .EQ. 1) THEN
         BMAT = 'I'
      ELSEIF (IBMAT .EQ. 2) THEN
         BMAT = 'G'
      ELSE
         BMAT = 'X'
      ENDIF
*
      IF (IWHICH .EQ. 1) THEN
         WHICH = 'LA'
      ELSEIF (IWHICH .EQ. 2) THEN
         WHICH = 'SA'
      ELSEIF (IWHICH .EQ. 3) THEN
         WHICH = 'LM'
      ELSEIF (IWHICH .EQ. 4) THEN
         WHICH = 'SM'
      ELSEIF (IWHICH .EQ. 5) THEN
         WHICH = 'BE'
      ELSE
         WHICH = 'XX'
      ENDIF
*
      HOWMNY = 'A'
*
      CALL DSEUPD (RVEC, HOWMNY, SELECT, D, Z, LDZ, SIGMA, BMAT, 
     *     N, WHICH, NEV, TOL, RESID, NCV, V, LDV, IPARAM, IPNTR, 
     *     WORKD, WORKL, LWORKL, INFO)
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RNAUPD (IDO, IBMAT, N, IWHICH, NEV, TOL, RESID, NCV, V, 
     *     LDV, IPARAM, IPNTR, WORKD, WORKL, LWORKL, INFO)
*
      INTEGER IDO, IBMAT, N, IWHICH, NEV, NCV, LDV, LWORKL, INFO
      INTEGER IPARAM(11), IPNTR(14)
      DOUBLE PRECISION TOL, RESID(N), V(LDV,NCV), WORKD(*), WORKL(*)
      CHARACTER*1 BMAT
      CHARACTER*2 WHICH
*
      IF (IBMAT .EQ. 1) THEN
         BMAT = 'I'
      ELSEIF (IBMAT .EQ. 2) THEN
         BMAT = 'G'
      ELSE
         BMAT = 'X'
      ENDIF
*
      IF (IWHICH .EQ. 1) THEN
         WHICH = 'LM'
      ELSEIF (IWHICH .EQ. 2) THEN
         WHICH = 'SM'
      ELSEIF (IWHICH .EQ. 3) THEN
         WHICH = 'LR'
      ELSEIF (IWHICH .EQ. 4) THEN
         WHICH = 'SR'
      ELSEIF (IWHICH .EQ. 5) THEN
         WHICH = 'LI'
      ELSEIF (IWHICH .EQ. 6) THEN
         WHICH = 'SI'
      ELSE
         WHICH = 'XX'
      ENDIF
*
      CALL DNAUPD (IDO, BMAT, N, WHICH, NEV, TOL, RESID, NCV, V, 
     *     LDV, IPARAM, IPNTR, WORKD, WORKL, LWORKL, INFO)
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE RNEUPD (IRVEC, IHOW, SELECT, DR, DI, Z, LDZ, SIGMAR,
     *     SIGMAI, WORKEV, IBMAT, 
     *     N, IWHICH, NEV, TOL, RESID, NCV, V, LDV, IPARAM, IPNTR, 
     *     WORKD, WORKL, LWORKL, INFO)
*
      INTEGER IRVEC, IHOW, IBMAT, N, IWHICH, NEV, NCV, LDV, LDZ
      INTEGER LWORKL, INFO
      INTEGER IPARAM(11), IPNTR(14)
      DOUBLE PRECISION RESID(N), Z(LDZ,*), V(LDV,NCV), WORKD(*)
      DOUBLE PRECISION TOL, SIGMAR, SIGMAI, DR(*), DI(*), WORKL(*)
      DOUBLE PRECISION WORKEV(*)
*
      CHARACTER*1 BMAT, HOWMNY
      CHARACTER*2 WHICH
      LOGICAL SELECT(NCV), RVEC
*
      RVEC = IRVEC .EQ. 1
*
      IF (IBMAT .EQ. 1) THEN
         BMAT = 'I'
      ELSEIF (IBMAT .EQ. 2) THEN
         BMAT = 'G'
      ELSE
         BMAT = 'X'
      ENDIF
*
      IF (IWHICH .EQ. 1) THEN
         WHICH = 'LM'
      ELSEIF (IWHICH .EQ. 2) THEN
         WHICH = 'SM'
      ELSEIF (IWHICH .EQ. 3) THEN
         WHICH = 'LR'
      ELSEIF (IWHICH .EQ. 4) THEN
         WHICH = 'SR'
      ELSEIF (IWHICH .EQ. 5) THEN
         WHICH = 'LI'
      ELSEIF (IWHICH .EQ. 6) THEN
         WHICH = 'SI'
      ELSE
         WHICH = 'XX'
      ENDIF
*
      HOWMNY = 'A'
*
      CALL DNEUPD (RVEC, HOWMNY, SELECT, DR, DI, Z, LDZ, SIGMAR,
     *     SIGMAI, WORKEV, BMAT, 
     *     N, WHICH, NEV, TOL, RESID, NCV, V, LDV, IPARAM, IPNTR, 
     *     WORKD, WORKL, LWORKL, INFO)
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XNAUPD (IDO, IBMAT, N, IWHICH, NEV, TOL, RESID, NCV, V, 
     *     LDV, IPARAM, IPNTR, WORKD, WORKL, LWORKL, RWORK, INFO)
*
      INTEGER IDO, IBMAT, N, IWHICH, NEV, NCV, LDV, LWORKL, INFO
      INTEGER IPARAM(11), IPNTR(14)
      DOUBLE PRECISION TOL, RWORK
      COMPLEX*16 RESID(N), V(LDV,NCV), WORKD(*), WORKL(*)
      CHARACTER*1 BMAT
      CHARACTER*2 WHICH
*
      IF (IBMAT .EQ. 1) THEN
         BMAT = 'I'
      ELSEIF (IBMAT .EQ. 2) THEN
         BMAT = 'G'
      ELSE
         BMAT = 'X'
      ENDIF
*
      IF (IWHICH .EQ. 1) THEN
         WHICH = 'LM'
      ELSEIF (IWHICH .EQ. 2) THEN
         WHICH = 'SM'
      ELSEIF (IWHICH .EQ. 3) THEN
         WHICH = 'LR'
      ELSEIF (IWHICH .EQ. 4) THEN
         WHICH = 'SR'
      ELSEIF (IWHICH .EQ. 5) THEN
         WHICH = 'LI'
      ELSEIF (IWHICH .EQ. 6) THEN
         WHICH = 'SI'
      ELSE
         WHICH = 'XX'
      ENDIF
*
      CALL ZNAUPD (IDO, BMAT, N, WHICH, NEV, TOL, RESID, NCV, V, 
     *     LDV, IPARAM, IPNTR, WORKD, WORKL, LWORKL, RWORK, INFO)
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE XNEUPD (IRVEC, IHOW, SELECT, D, Z, LDZ, SIGMA,
     *     WORKEV, IBMAT, 
     *     N, IWHICH, NEV, TOL, RESID, NCV, V, LDV, IPARAM, IPNTR, 
     *     WORKD, WORKL, LWORKL, RWORK, INFO)
*
      INTEGER IRVEC, IHOW, IBMAT, N, IWHICH, NEV, NCV, LDV, LDZ
      INTEGER LWORKL, INFO
      INTEGER IPARAM(11), IPNTR(14)
      COMPLEX*16 RESID(N), Z(LDZ,*), V(LDV,NCV), WORKD(*)
      COMPLEX*16 SIGMA, D(*), WORKL(*), WORKEV(*)
      DOUBLE PRECISION TOL, RWORK(*)
*
      CHARACTER*1 BMAT, HOWMNY
      CHARACTER*2 WHICH
      LOGICAL SELECT(NCV), RVEC
*
      RVEC = IRVEC .EQ. 1
*
      IF (IBMAT .EQ. 1) THEN
         BMAT = 'I'
      ELSEIF (IBMAT .EQ. 2) THEN
         BMAT = 'G'
      ELSE
         BMAT = 'X'
      ENDIF
*
      IF (IWHICH .EQ. 1) THEN
         WHICH = 'LM'
      ELSEIF (IWHICH .EQ. 2) THEN
         WHICH = 'SM'
      ELSEIF (IWHICH .EQ. 3) THEN
         WHICH = 'LR'
      ELSEIF (IWHICH .EQ. 4) THEN
         WHICH = 'SR'
      ELSEIF (IWHICH .EQ. 5) THEN
         WHICH = 'LI'
      ELSEIF (IWHICH .EQ. 6) THEN
         WHICH = 'SI'
      ELSE
         WHICH = 'XX'
      ENDIF
*
      HOWMNY = 'A'
*
      CALL ZNEUPD (RVEC, HOWMNY, SELECT, D, Z, LDZ, SIGMA,
     *     WORKEV, BMAT, 
     *     N, WHICH, NEV, TOL, RESID, NCV, V, LDV, IPARAM, IPNTR, 
     *     WORKD, WORKL, LWORKL, RWORK, INFO)
*
      RETURN
      END
*
************************************************************************
