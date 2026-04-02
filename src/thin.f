************************-*-Mode: Fortran -*- **************************
*
* thin.f -- These are the Algae sparse matrix routines.
*    
* Copyright (C) 1994-97  K. Scott Hunziker.
* Copyright (C) 1990-94  The Boeing Company.
*
* See the file COPYING for license, warranty, and permission details.
*
***********************************************************************

* $Id: thin.f,v 1.4 2001/10/12 00:37:52 ksh Exp $

      SUBROUTINE DGSADD( AI, AJ, AN, BI, BJ, BN, N, M,
     $     CI, CJ, CN, W )
      INTEGER AI(*), AJ(*), BI(*), BJ(*), N, M
      INTEGER CI(*), CJ(*)
      DOUBLE PRECISION AN(*), BN(*), CN(*), W(M)

*     DGSADD adds two double precision matrices in the full, unordered,
*     sparse form.  The CI and CJ vectors should be previously set by a
*     call to XGSADD.  This routine does not change the order of CI and
*     CJ, so one can call XGSORD to order them before calling this
*     routine.

*     Input:   AI, AJ, AN       The first matrix.
*              BI, BJ, BN       The second matrix.
*              CI, CJ           The structure of the result.
*              N                The number of rows.
*              M                The number of columns.

*     Output:  CN               The resulting matrix.
*              W                Workspace of length M.

      INTEGER I, J, CI1, CI2

*     Loop over all the rows.

      DO 100 I = 1, N

         CI1 = CI(I)
         CI2 = CI(I+1) - 1

*        Any nonzeros in this row?

         IF ( CI1 .GT. CI2 ) GO TO 100

*        Set result columns to zero.

         DO 10 J = CI1, CI2
            W(CJ(J)) = 0.0D0
 10      CONTINUE

*        Assign nonzeros from left matrix.

         DO 20 J = AI(I), AI(I+1) - 1
            W(AJ(J)) = AN(J)
 20      CONTINUE

*        Add in nonzeros from right matrix.

         DO 30 J = BI(I), BI(I+1) - 1
            W(BJ(J)) = W(BJ(J)) + BN(J)
 30      CONTINUE

*        Collect the results.

         DO 40 J = CI1, CI2
            CN(J) = W(CJ(J))
 40      CONTINUE

 100  CONTINUE

      RETURN
      END

***********************************************************************

      SUBROUTINE DGSMUL( AI, AJ, AN, BI, BJ, BN, NA, MB,
     $     CI, CJ, CN, W )
      INTEGER AI(*), AJ(*), BI(*), BJ(*), NA, MB
      INTEGER CI(*), CJ(*)
      DOUBLE PRECISION AN(*), BN(*), CN(*), W(MB)

*     DGSMUL multiplies two matrices in the full, unordered, sparse
*     form.  The structure of the result must have already been formed,
*     likely by calling XGSMUL.  This routine does not change the order
*     of CI and CJ, so one can call XGSORD to order them before calling
*     this routine.

*     Input:   AI, AJ, AN       The first matrix.
*              BI, BJ, BN       The second matrix.
*              CI, CJ           The results matrix structure.
*              NA               The number of rows of A.
*              MB               The number of columns of B.

*     Output:  CN               The resulting matrix.
*              W                Workspace of length MB.

      INTEGER I, J, K, CI1, CI2

*     Loop over the rows of A.

      DO 100 I = 1, NA

         CI1 = CI(I)
         CI2 = CI(I+1) - 1

*        Any nonzeros in this row?

         IF ( CI1 .GT. CI2 ) GO TO 100

*        Set result columns to zero.

         DO 10 J = CI1, CI2
            W(CJ(J)) = 0.0D0
 10      CONTINUE

*        Step through the nonzeros in this row of A.

         DO 30 J = AI(I), AI(I+1) - 1

*           Work through the nonzeros in the corresponding row of B.

            DO 20 K = BI(AJ(J)), BI(AJ(J)+1) - 1
               W(BJ(K)) = W(BJ(K)) + AN(J)*BN(K)
 20         CONTINUE

 30      CONTINUE 

*        Collect the results.

         DO 40 J = CI1, CI2
            CN(J) = W(CJ(J))
 40      CONTINUE

 100  CONTINUE

      RETURN
      END

***********************************************************************

      SUBROUTINE DGSSUB( AI, AJ, AN, BI, BJ, BN, N, M,
     $     CI, CJ, CN, W )
      INTEGER AI(*), AJ(*), BI(*), BJ(*), N, M
      INTEGER CI(*), CJ(*)
      DOUBLE PRECISION AN(*), BN(*), CN(*), W(M)

*     DGSSUB subtracts two double precision matrices in the full,
*     unordered, sparse form.  The CI and CJ vectors should be
*     previously set by a call to XGSADD.  This routine does not change
*     the order of CI and CJ, so one can call XGSORD to order them
*     before calling this routine.

*     Input:   AI, AJ, AN       The first matrix.
*              BI, BJ, BN       The second matrix.
*              CI, CJ           The structure of the result.
*              N                The number of rows.
*              M                The number of columns.

*     Output:  CN               The resulting matrix.
*              W                Workspace of length M.

      INTEGER I, J, CI1, CI2

*     Loop over all the rows.

      DO 100 I = 1, N

         CI1 = CI(I)
         CI2 = CI(I+1) - 1

*        Any nonzeros in this row?

         IF ( CI1 .GT. CI2 ) GO TO 100

*        Set result columns to zero.

         DO 10 J = CI1, CI2
            W(CJ(J)) = 0.0D0
 10      CONTINUE

*        Assign nonzeros from left matrix.

         DO 20 J = AI(I), AI(I+1) - 1
            W(AJ(J)) = AN(J)
 20      CONTINUE

*        Subtract nonzeros from right matrix.

         DO 30 J = BI(I), BI(I+1) - 1
            W(BJ(J)) = W(BJ(J)) - BN(J)
 30      CONTINUE

*        Collect the results.

         DO 40 J = CI1, CI2
            CN(J) = W(CJ(J))
 40      CONTINUE

 100  CONTINUE

      RETURN
      END

***********************************************************************

      SUBROUTINE DGSTRN( AI, AJ, AN, N, M, ATI, ATJ, ATN )
      INTEGER AI(*), AJ(*), ATI(*), ATJ(*), N, M
      DOUBLE PRECISION AN(*), ATN(*)

*     DGSTRN transposes a double precision matrix in the full,
*     unordered, sparse form.

*     Input:   AI, AJ, AN       The original matrix.
*              N                The number of rows.
*              M                The number of columns.

*     Output:  ATI, ATJ, ATN    The transposed matrix.

      INTEGER I, J, K, JJ

      DO 10 I = 2, M+1
         ATI(I) = 0
 10   CONTINUE

      DO 20 I = 1, AI(N+1) - 1
         J = AJ(I) + 2
         IF ( J .LE. M+1 ) ATI(J) = ATI(J) + 1
 20   CONTINUE

      ATI(1) = 1
      ATI(2) = 1

      DO 30 I = 3, M+1
         ATI(I) = ATI(I) + ATI(I-1)
 30   CONTINUE

      DO 50 I = 1, N

         DO 40 J = AI(I), AI(I+1) - 1

            JJ = AJ(J) + 1
            K = ATI(JJ)

            ATJ(K) = I
            ATN(K) = AN(J)
            ATI(JJ) = K + 1

 40      CONTINUE

 50   CONTINUE 

      RETURN
      END

***********************************************************************

      SUBROUTINE IGSADD( AI, AJ, AN, BI, BJ, BN, N, M,
     $     CI, CJ, CN, W )
      INTEGER AI(*), AJ(*), BI(*), BJ(*), N, M
      INTEGER CI(*), CJ(*)
      INTEGER AN(*), BN(*), CN(*), W(M)

*     IGSADD adds two integer matrices in the full, unordered, sparse
*     form.  The CI and CJ vectors should be previously set by a call
*     to XGSADD.  This routine does not change the order of CI and CJ,
*     so one can call XGSORD to order them before calling this routine.

*     Input:   AI, AJ, AN       The first matrix.
*              BI, BJ, BN       The second matrix.
*              CI, CJ           The structure of the result.
*              N                The number of rows.
*              M                The number of columns.

*     Output:  CN               The resulting matrix.
*              W                Workspace of length M.

      INTEGER I, J, CI1, CI2

*     Loop over all the rows.

      DO 100 I = 1, N

         CI1 = CI(I)
         CI2 = CI(I+1) - 1

*        Any nonzeros in this row?

         IF ( CI1 .GT. CI2 ) GO TO 100

*        Set result columns to zero.

         DO 10 J = CI1, CI2
            W(CJ(J)) = 0
 10      CONTINUE

*        Assign nonzeros from left matrix.

         DO 20 J = AI(I), AI(I+1) - 1
            W(AJ(J)) = AN(J)
 20      CONTINUE

*        Add in nonzeros from right matrix.

         DO 30 J = BI(I), BI(I+1) - 1
            W(BJ(J)) = W(BJ(J)) + BN(J)
 30      CONTINUE

*        Collect the results.

         DO 40 J = CI1, CI2
            CN(J) = W(CJ(J))
 40      CONTINUE

 100  CONTINUE

      RETURN
      END

***********************************************************************

      SUBROUTINE IGSMUL( AI, AJ, AN, BI, BJ, BN, NA, MB,
     $     CI, CJ, CN, W )
      INTEGER AI(*), AJ(*), BI(*), BJ(*), NA, MB
      INTEGER CI(*), CJ(*)
      INTEGER AN(*), BN(*), CN(*), W(MB)

*     IGSMUL multiplies two matrices in the full, unordered, sparse
*     form.  The structure of the result must have already been formed,
*     likely by calling XGSMUL.

*     Input:   AI, AJ, AN       The first matrix.
*              BI, BJ, BN       The second matrix.
*              CI, CJ           The results matrix structure.
*              NA               The number of rows of A.
*              MB               The number of columns of B.

*     Output:  CN               The resulting matrix.
*              W                Workspace of length MB.

      INTEGER I, J, K, CI1, CI2

*     Loop over the rows of A.

      DO 100 I = 1, NA

         CI1 = CI(I)
         CI2 = CI(I+1) - 1

         IF ( CI1 .GT. CI2 ) GO TO 100

*        Set result columns to zero.

         DO 10 J = CI1, CI2
            W(CJ(J)) = 0
 10      CONTINUE

*        Step through the nonzeros in this row of A.

         DO 30 J = AI(I), AI(I+1) - 1

*           Work through the nonzeros in the corresponding row of B.

            DO 20 K = BI(AJ(J)), BI(AJ(J)+1) - 1
               W(BJ(K)) = W(BJ(K)) + AN(J)*BN(K)
 20         CONTINUE

 30      CONTINUE 

*        Collect the results.

         DO 40 J = CI1, CI2
            CN(J) = W(CJ(J))
 40      CONTINUE

 100  CONTINUE

      RETURN
      END

***********************************************************************

      SUBROUTINE IGSSUB( AI, AJ, AN, BI, BJ, BN, N, M,
     $     CI, CJ, CN, W )
      INTEGER AI(*), AJ(*), BI(*), BJ(*), N, M
      INTEGER CI(*), CJ(*)
      INTEGER AN(*), BN(*), CN(*), W(M)

*     IGSSUB subtracts two integer matrices in the full, unordered
*     sparse form.  The CI and CJ vectors should be previously set by a
*     call to XGSADD.  This routine does not change the order of CI and
*     CJ, so one can call XGSORD to order them before calling this
*     routine.

*     Input:   AI, AJ, AN       The first matrix.
*              BI, BJ, BN       The second matrix.
*              CI, CJ           The structure of the result.
*              N                The number of rows.
*              M                The number of columns.

*     Output:  CN               The resulting matrix.
*              W                Workspace of length M.

      INTEGER I, J, CI1, CI2

      DO 100 I = 1, N

         CI1 = CI(I)
         CI2 = CI(I+1) - 1

*        Any nonzeros in this row?

         IF ( CI1 .GT. CI2 ) GO TO 100

*        Set result columns to zero.

         DO 10 J = CI1, CI2
            W(CJ(J)) = 0
 10      CONTINUE

*        Assign nonzeros from left matrix.

         DO 20 J = AI(I), AI(I+1) - 1
            W(AJ(J)) = AN(J)
 20      CONTINUE

*        Subtract nonzeros from right matrix.

         DO 30 J = BI(I), BI(I+1) - 1
            W(BJ(J)) = W(BJ(J)) - BN(J)
 30      CONTINUE

*        Collect the results.

         DO 40 J = CI1, CI2
            CN(J) = W(CJ(J))
 40      CONTINUE

 100  CONTINUE

      RETURN
      END

***********************************************************************

      SUBROUTINE IGSTRN( AI, AJ, AN, N, M, ATI, ATJ, ATN )
      INTEGER AI(*), AJ(*), ATI(*), ATJ(*), N, M
      INTEGER AN(*), ATN(*)

*     IGSTRN transposes an integer matrix in the full, unordered, sparse
*     form.

*     Input:   AI, AJ, AN       The original matrix.
*              N                The number of rows.
*              M                The number of columns.

*     Output:  ATI, ATJ, ATN    The transposed matrix.

      INTEGER I, J, K, JJ

      DO 10 I = 2, M+1
         ATI(I) = 0
 10   CONTINUE

      DO 20 I = 1, AI(N+1) - 1
         J = AJ(I) + 2
         IF ( J .LE. M+1 ) ATI(J) = ATI(J) + 1
 20   CONTINUE

      ATI(1) = 1
      ATI(2) = 1

      DO 30 I = 3, M+1
         ATI(I) = ATI(I) + ATI(I-1)
 30   CONTINUE

      DO 50 I = 1, N

         DO 40 J = AI(I), AI(I+1) - 1

            JJ = AJ(J) + 1
            K = ATI(JJ)

            ATJ(K) = I
            ATN(K) = AN(J)
            ATI(JJ) = K + 1

 40      CONTINUE

 50   CONTINUE

      RETURN
      END

***********************************************************************

      SUBROUTINE XGSADD( AI, AJ, BI, BJ, N, M, CI, CJ, IW )
      INTEGER AI(*), AJ(*), BI(*), BJ(*), N, M
      INTEGER CI(*), CJ(*), IW(M)

* XGSADD symbolicly adds two matrices in the full, unordered sparse
* form.

*     Input:   AI, AJ  The first matrix structure.
*              BI, BJ  The second matrix structure.
*              N       The number of rows.
*              M       The number of columns.

*     Output:  CI, CJ  The resulting matrix structure.
*              IW      Integer workspace of length M.

      INTEGER I, J, K

      K = 1

      DO 10 I = 1, M
         IW(I) = 0
 10   CONTINUE

      DO 40 I = 1, N

         CI(I) = K

         DO 20 J = AI(I), AI(I+1) - 1
            CJ(K) = AJ(J)
            IW(AJ(J)) = I
            K = K + 1
 20      CONTINUE

         DO 30 J = BI(I), BI(I+1) - 1
            IF ( IW(BJ(J)) .EQ. I ) GO TO 30
            CJ(K) = BJ(J)
            K = K + 1
 30      CONTINUE

 40   CONTINUE

      CI(N+1) = K

      RETURN
      END

***********************************************************************

      SUBROUTINE XGSMUL( AI, AJ, BI, BJ, NA, MB, CI, CJ, MAXCJ, IW )
      INTEGER AI(*), AJ(*), BI(*), BJ(*), NA, MB
      INTEGER CI(*), CJ(MAXCJ), IW(MB)

*     XGSMUL symbolicly multiplies two matrices in full, unordered
*     sparse form.  If CJ is not dimensioned large enough, we return
*     with CI(1)=0.

*     Input:   AI, AJ  The first matrix structure.
*              BI, BJ  The second matrix structure.
*              NA      The number of rows of A.
*              MB      The number of columns of B.

*     Output:  CI, CJ  The resulting matrix structure.
*              MAXCJ   The dimensioned length of CJ.
*              IW      Integer workspace of length MB.

      INTEGER I, J, K, JJ
*     
      JJ = 1

*     Initialize

      DO 10 I = 1, MB
         IW(I) = 0
 10   CONTINUE

*     Loop over rows of left matrix.

      DO 40 I = 1, NA

         CI(I) = JJ

         DO 30 J = AI(I), AI(I+1) - 1

            DO 20 K = BI(AJ(J)), BI(AJ(J)+1) - 1

               IF ( IW(BJ(K)) .EQ. I ) GO TO 20

               IF ( JJ .GT. MAXCJ ) THEN
                  CI(1) = 0
                  GO TO 50
               ENDIF

               CJ(JJ) = BJ(K)
               JJ = JJ + 1
               IW(BJ(K)) = I

 20         CONTINUE
 30      CONTINUE
 40   CONTINUE

      CI(NA+1) = JJ

 50   CONTINUE
      RETURN
      END

***********************************************************************

      SUBROUTINE XGSTRN( AI, AJ, N, M, ATI, ATJ )
      INTEGER AI(*), AJ(*), ATI(*), ATJ(*), N, M

*     XGSTRN symbolically transposes a matrix in the full, unordered,
*     sparse form.  In the process, the rows of the matrix are ordered,
*     so that two calls to this routine will order the matrix.

*     Input:   AI, AJ           The original matrix structure.
*              N                The number of rows.
*              M                The number of columns.

*     Output:  ATI, ATJ         The transposed matrix structure.

      INTEGER I, J

      DO 10 I = 2, M+1
         ATI(I) = 0
 10   CONTINUE

      DO 20 I = 1, AI(N+1) - 1
         J = AJ(I) + 2
         IF ( J .LE. M+1 ) ATI(J) = ATI(J) + 1
 20   CONTINUE

      ATI(1) = 1
      ATI(2) = 1

      DO 30 I = 3, M+1
         ATI(I) = ATI(I) + ATI(I-1)
 30   CONTINUE

      DO 50 I = 1, N

         DO 40 J = AI(I), AI(I+1) - 1
            ATJ( ATI( AJ(J)+1 ) ) = I
            ATI( AJ(J)+1 ) = ATI( AJ(J)+1 ) + 1
 40      CONTINUE

 50   CONTINUE

      RETURN
      END

***********************************************************************

      SUBROUTINE ZGSADD( AI, AJ, AN, BI, BJ, BN, N, M,
     $     CI, CJ, CN, W )
      INTEGER AI(*), AJ(*), BI(*), BJ(*), N, M
      INTEGER CI(*), CJ(*)
      DOUBLE PRECISION AN(*), BN(*), CN(*), W(*)

*     ZGSADD adds matrices in the full, unordered, sparse form.  The CI
*     and CJ vectors should be previously set by a call to XGSADD.  This
*     routine does not change the order of CI and CJ, so one could call
*     XGSORD to order them before calling this routine.

*     Double precision complex data is stored as two consecutive double
*     precision numbers.  (This is equivalent to the COMPLEX*16 data
*     type that most FORTRAN compilers provide but which is not part
*     of the standard language.)

*     Input:   AI, AJ, AN       The first matrix.
*              BI, BJ, BN       The second matrix.
*              CI, CJ           The structure of the result.
*              N                The number of rows.
*              M                The number of columns.

*     Output:  CN               The resulting matrix.
*              W                Workspace of length M.

      INTEGER I, J, K, CI1, CI2

      DO 50 I = 1, N

         CI1 = CI(I)
         CI2 = CI(I+1) - 1

         IF ( CI1 .GT. CI2 ) GO TO 50

         DO 10 K = CI1, CI2
            W(2*CJ(K)-1) = 0.0D0
            W(2*CJ(K)) = 0.0D0
 10      CONTINUE

         DO 20 K = AI(I), AI(I+1) - 1
            W(2*AJ(K)-1) = AN(2*K-1)
            W(2*AJ(K)) = AN(2*K)
 20      CONTINUE

         DO 30 K = BI(I), BI(I+1) - 1
            J = BJ(K)
            W(2*J-1) = W(2*J-1) + BN(2*K-1)
            W(2*J) = W(2*J) + BN(2*K)
 30      CONTINUE

         DO 40 K = CI1, CI2
            CN(2*K-1) = W(2*CJ(K)-1)
            CN(2*K) = W(2*CJ(K))
 40      CONTINUE

 50   CONTINUE

      RETURN
      END

***********************************************************************

      SUBROUTINE ZGSMUL( AI, AJ, AN, BI, BJ, BN, NA, MB,
     $     CI, CJ, CN, W )
      INTEGER AI(*), AJ(*), BI(*), BJ(*), NA, MB
      INTEGER CI(*), CJ(*)
      DOUBLE PRECISION AN(*), BN(*), CN(*), W(MB)

*     ZGSMUL multiplies two matrices in the full, unordered, sparse
*     form.  The structure of the result must have already been formed,
*     likely by calling XGSMUL.

*     Input:   AI, AJ, AN       The first matrix.
*              BI, BJ, BN       The second matrix.
*              CI, CJ           The results matrix structure.
*              NA               The number of rows of A.
*              MB               The number of columns of B.

*     Output:  CN               The resulting matrix.
*              W                Workspace of length MB.

      INTEGER I, J, K, KK, CI1, CI2
      DOUBLE PRECISION ARE, AIM

      DO 50 I = 1, NA

         CI1 = CI(I)
         CI2 = CI(I+1) - 1

         IF ( CI1 .GT. CI2 ) GO TO 50

         DO 10 J = CI1, CI2
            W(2*CJ(J)-1) = 0.0D0
            W(2*CJ(J)) = 0.0D0
 10      CONTINUE

         DO 30 J = AI(I), AI(I+1) - 1

            ARE = AN(2*J-1)
            AIM = AN(2*J)

            DO 20 KK = BI(AJ(J)), BI(AJ(J)+1) - 1
               K = BJ(KK)
               W(2*K-1) = W(2*K-1) + ARE*BN(2*KK-1)-AIM*BN(2*KK)
               W(2*K) = W(2*K) + ARE*BN(2*KK)+AIM*BN(2*KK-1)
 20         CONTINUE

 30      CONTINUE

         DO 40 J = CI1, CI2
            CN(2*J-1) = W(2*CJ(J)-1)
            CN(2*J) = W(2*CJ(J))
 40      CONTINUE

 50   CONTINUE

      RETURN
      END

***********************************************************************

      SUBROUTINE ZGSSUB( AI, AJ, AN, BI, BJ, BN, N, M,
     $     CI, CJ, CN, W )
      INTEGER AI(*), AJ(*), BI(*), BJ(*), N, M
      INTEGER CI(*), CJ(*)
      DOUBLE PRECISION AN(*), BN(*), CN(*), W(*)

*     ZGSSUB subtracts double precision complex matrices in the full,
*     unordered, sparse form.  The CI and CJ vectors should be
*     previously set by a call to XGSADD.  This routine does not change
*     the order of CI and CJ, so one could call XGSORD to order them
*     before calling this routine.

*     Input:   AI, AJ, AN       The first matrix.
*              BI, BJ, BN       The second matrix.
*              CI, CJ           The structure of the result.
*              N                The number of rows.
*              M                The number of columns.

*     Output:  CN               The resulting matrix.
*              W                Workspace of length M.

      INTEGER I, J, K, CI1, CI2

      DO 50 I = 1, N

         CI1 = CI(I)
         CI2 = CI(I+1) - 1

         IF ( CI1 .GT. CI2 ) GO TO 50

         DO 10 J = CI1, CI2
            W(2*CJ(J)-1) = 0.0D0
            W(2*CJ(J)) = 0.0D0
 10      CONTINUE

         DO 20 J = AI(I), AI(I+1) - 1
            W(2*AJ(J)-1) = AN(2*J-1)
            W(2*AJ(J)) = AN(2*J)
 20      CONTINUE

         DO 30 J = BI(I), BI(I+1) - 1
            K = BJ(J)
            W(2*K-1) = W(2*K-1) - BN(2*J-1)
            W(2*K) = W(2*K) - BN(2*J)
 30      CONTINUE

         DO 40 J = CI1, CI2
            CN(2*J-1) = W(2*CJ(J)-1)
            CN(2*J) = W(2*CJ(J))
 40      CONTINUE

 50   CONTINUE

      RETURN
      END

***********************************************************************

      SUBROUTINE ZGSTRN( AI, AJ, AN, N, M, ATI, ATJ, ATN )
      INTEGER AI(*), AJ(*), ATI(*), ATJ(*), N, M
      DOUBLE PRECISION AN(*), ATN(*)

*     DGSTRN transposes a double precision complex matrix in the
*     full, unordered sparse form.

*     Input:   AI, AJ, AN       The original matrix.
*              N                The number of rows.
*              M                The number of columns.

*     Output:  ATI, ATJ, ATN    The transposed matrix.

      INTEGER I, J, K, JP

      DO 10 I = 2, M+1
         ATI(I) = 0
 10   CONTINUE

      DO 20 I = 1, AI(N+1) - 1
         J = AJ(I) + 2
         IF ( J .LE. M+1 ) ATI(J) = ATI(J) + 1
 20   CONTINUE

      ATI(1) = 1
      ATI(2) = 1

      DO 30 I = 3, M+1
         ATI(I) = ATI(I) + ATI(I-1)
 30   CONTINUE

      DO 50 I = 1, N
         DO 40 JP = AI(I), AI(I+1) - 1
            J = AJ(JP) + 1
            K = ATI(J)
            ATJ(K) = I
            ATN(2*K-1) = AN(2*JP-1)
            ATN(2*K) = AN(2*JP)
            ATI(J) = K + 1
 40      CONTINUE
 50   CONTINUE

      RETURN
      END

***********************************************************************

      SUBROUTINE DPSSOL( IU, JU, UN, DI, N, B, X )
      INTEGER IU(*), JU(*), N
      DOUBLE PRECISION UN(*), DI(N), B(N), X(N)

*     DPSSOL solves a system of linear equations "U'*D*U*x=b" for "x",
*     where "U" is an upper triangular, double precision sparse matrix
*     in upper, unordered form.  The "U" matrix likely comes from calls
*     to XPSFAC, XGSORD, and DPSFAC.  "b" is a full input vector, and
*     "x" is the full output vector.

*     Input:   IU, JU, UN       The upper triangular matrix "U".  Its
*                               diagonal is assumed to be identity,
*                               and is not used.
*              DI               The inverses of the diagonal elements
*                               of D.
*              N                The order of the system.
*              B                The right-hand-side vector.

*     Output:  X                The solution vector.

      INTEGER I, K, NM
      DOUBLE PRECISION XX

      NM = N - 1
      DO 10 I = 1, N
         X(I) = B(I)
 10   CONTINUE
      DO 40 K = 1, NM
         XX = X(K)
         DO 20 I = IU(K), IU(K+1) - 1
            X(JU(I)) = X(JU(I)) - UN(I)*XX
 20      CONTINUE
         X(K) = XX*DI(K)
 40   CONTINUE
      X(N) = X(N)*DI(N)
      K = NM
 50   CONTINUE
      XX = X(K)
      DO 60 I = IU(K), IU(K+1) - 1
         XX = XX - UN(I)*X(JU(I))
 60   CONTINUE
      X(K) = XX
      K = K - 1
      IF ( K .GT. 0 ) GO TO 50

      RETURN
      END

***********************************************************************

      SUBROUTINE DPSFAC( AI, AJ, AN, AD, N, IU, JU, UN, DI, IP, IUP )
      INTEGER AI(*), AJ(*), N
      INTEGER IU(*), JU(*), IP(N), IUP(N)
      DOUBLE PRECISION AN(*), AD(N), UN(*), DI(N)

*     DPSFAC performs a triangular factorization of a symmetric,
*     positive definite, sparse matrix in upper, unordered form.  The
*     structure of the result must already exist, and it must be in
*     RR(U)O form.  Usually, one calls XPSFAC, then XGSORD, and then
*     DPSFAC.  The factorization satisfies A=U'*D*U.  The matrix
*     returned is in RR(U)O form, has 1/D on the diagonal and the
*     off-diag elements of U in the upper triangle.  (The diagonal
*     elements of U are all equal to one.)

*     Input:	AI, AJ, AN, AD  The matrix to be factored.
*               N               The order of the matrix.
*               IU, JU          The matrix U structure.

*     Output:	UN              The matrix U off-diagonal elements.
*               DI              The inverse of matrix D.
*               IP              Integer workspace of length N.
*               IUP             Integer workspace of length N.  The
*                               value of IUP(1) is zero on a successful
*                               return or nonzero on an error.

      INTEGER I, J, L, JJ, IH, IUA, IUB, AI1, AI2, LAST, LN
      INTEGER IUC, IUD
      DOUBLE PRECISION UM

      DO 10 J = 1, N
         IP(J) = 0
 10   CONTINUE
      DO 130 I = 1, N
         IH = I + 1
         IUA = IU(I)
         IUB = IU(IH) - 1
         IF ( IUB .LT. IUA ) GO TO 40
         DO 20 J = IUA, IUB
            DI(JU(J)) = 0.0D0
 20      CONTINUE
         AI1 = AI(I)
         AI2 = AI(IH) - 1
         IF ( AI2 .LT. AI1 ) GO TO 40
         DO 30 J = AI1, AI2
            DI(AJ(J)) = AN(J)
 30      CONTINUE
 40      CONTINUE
         DI(I) = AD(I)
         LAST = IP(I)
         IF ( LAST .EQ. 0 ) GO TO 90
         LN = IP(LAST)
 50      CONTINUE
         L = LN
         LN = IP(L)
         IUC = IUP(L)
         IUD = IU(L+1) - 1
         UM = UN(IUC)*DI(L)
         DO 60 J = IUC, IUD
            JJ = JU(J)
            DI(JJ) = DI(JJ) - UN(J)*UM
 60      CONTINUE
         UN(IUC) = UM
         IUP(L) = IUC + 1
         IF ( IUC .EQ. IUD ) GO TO 80
         J = JU(IUC+1)
         JJ = IP(J)
         IF ( JJ .EQ. 0 ) GO TO 70
         IP(L) = IP(JJ)
         IP(JJ) = L
         GO TO 80
 70      CONTINUE
         IP(J) = L
         IP(L) = L
 80      CONTINUE
         IF ( L .NE. LAST ) GO TO 50
 90      CONTINUE
         IF ( 1.0D0 + DI(I) .EQ. 1.0D0 ) THEN
            IUP(1) = -1
            RETURN
         ENDIF
         DI(I) = 1.0D0/DI(I)
         IF ( IUB .LT. IUA ) GO TO 120
         DO 100 J = IUA, IUB
            UN(J) = DI(JU(J))
 100     CONTINUE
         J = JU(IUA)
         JJ = IP(J)
         IF ( JJ .EQ. 0 ) GO TO 110
         IP(I) = IP(JJ)
         IP(JJ) = I
         GO TO 120
 110     CONTINUE
         IP(J) = I
         IP(I) = I
 120     CONTINUE
         IUP(I) = IUA
 130  CONTINUE

      IUP(1) = 0
      RETURN
      END

***********************************************************************

      SUBROUTINE XPSFAC( AI, AJ, N, NN, IU, JU, IP )
      INTEGER AI(*), AJ(*), N, NN
      INTEGER IU(*), JU(*), IP(N)

*     XPSFAC performs a symbolic triangular factorization of a
*     symmetric, positive definite, sparse matrix in upper, unordered
*     form.

*     Input:   AI, AJ           The matrix to be factored.
*              N                The order of the matrix.
*              NN               The length of JU.

*     Output:  IU, JU           The resulting matrix structure.
*              IP               Integer workspace of length N.

*     If the required length of JU is greater than NN, then the routine
*     returns with IU(1)=0 and should be called again with a longer JU.

      INTEGER I, JJ, L, NM, NH, JP, JPI, JPP, MN, AI1, AI2, LAST
      INTEGER LH, IUA, IUB

      NM = N - 1
      NH = N + 1
      DO 10 I = 1, N
         IU(I) = 0
         IP(I) = 0
 10   CONTINUE
      JP = 1
      DO 90 I = 1, NM
         JPI = JP
         JPP = N + JP - 1
         MN = NH
         AI1 = AI(I)
         AI2 = AI(I+1) - 1
         IF ( AI2 .LT. AI1 ) GO TO 30
         DO 20 J = AI1, AI2
            JJ = AJ(J)
            IF ( JP .GT. NN ) GO TO 100
            JU(JP) = JJ
            JP = JP + 1
            IF ( JJ .LT. MN ) MN = JJ
            IU(JJ) = I
 20      CONTINUE
 30      CONTINUE
         LAST = IP(I)
         IF ( LAST .EQ. 0 ) GO TO 60
         L = LAST
 40      CONTINUE
         L = IP(L)
         LH = L + 1
         IUA = IU(L)
         IUB = IU(LH) - 1
         IF ( LH .EQ. I ) IUB = JPI - 1
         IU(I) = I
         DO 50 J = IUA, IUB
            JJ = JU(J)
            IF ( IU(JJ) .EQ. I ) GO TO 50
            IF ( JP .GT. NN ) GO TO 100
            JU(JP) = JJ
            JP = JP + 1
            IU(JJ) = I
            IF ( JJ .LT. MN ) MN = JJ
 50      CONTINUE
         IF ( JP .EQ. JPP ) GO TO 70
         IF ( L .NE. LAST ) GO TO 40
 60      CONTINUE
         IF ( MN .EQ. NH ) GO TO 90
 70      CONTINUE
         L = IP(MN)
         IF ( L .EQ. 0 ) GO TO 80
         IP(I) = IP(L)
         IP(L) = I
         GO TO 90
 80      CONTINUE
         IP(MN) = I
         IP(I) = I
 90      IU(I) = JPI
      IU(N) = JP
      IU(NH) = JP
      RETURN

 100  CONTINUE
      IU(1) = 0
      RETURN

      END

***********************************************************************

      SUBROUTINE ISSTRF( N, M, AI, AJ, AN, AD, P, C, W )
      INTEGER N, AI(*), AJ(*)
      INTEGER AN(*), AD(*), P(N,M), C(M,M), W(N,M)

*     This is the "transform" routine.  It multiplies P'*A*P to form the
*     result C, where P and C are dense and A is an INTEGER, sparse,
*     symmetric matrix in upper, unordered form.

*     Input:	N               The order of A.
*               M               The number of columns of P.
*               AI,AJ,AN,AD     The N by N matrix A.
*               P               The N by M matrix P.
*               W               A work matrix, N by M.

*     Output:   C               The M by M result.

      DO 20 I = 1, N
         DO 10 J = 1, M
            W(I,J) = AD(I)*P(I,J)
 10      CONTINUE
 20   CONTINUE

      DO 100 I = 1, N
         DO 50 K = AI(I), AI(I+1)-1
            DO 40 J = 1, M
               W(I,J) = W(I,J) + AN(K)*P(AJ(K),J)
               W(AJ(K),J) = W(AJ(K),J) + AN(K)*P(I,J)
 40         CONTINUE
 50      CONTINUE
 100  CONTINUE

      DO 200 I = 1, M
         DO 180 J = I, M
            C(I,J) = 0
            DO 160 K = 1, N
               C(I,J) = C(I,J) + P(K,I)*W(K,J)
 160        CONTINUE
            C(J,I) = C(I,J)
 180     CONTINUE
 200  CONTINUE

      RETURN
      END

***********************************************************************

      SUBROUTINE DSSTRF( N, M, AI, AJ, AN, AD, P, C, W )
      INTEGER N, AI(*), AJ(*)
      DOUBLE PRECISION AN(*), AD(*), P(N,M), C(M,M), W(N,M)

*     This is the "transform" routine.  It multiplies P'*A*P to form the
*     result C, where P and C are dense and A is a REAL*8, sparse,
*     symmetric matrix in upper, unordered form.

*     Input:	N               The order of A.
*               M               The number of columns of P.
*               AI,AJ,AN,AD     The N by N matrix A.
*               P               The N by M matrix P.
*               W               A work matrix, N by M.

*     Output:   C               The M by M result.

      DO 20 I = 1, N
         DO 10 J = 1, M
            W(I,J) = AD(I)*P(I,J)
 10      CONTINUE
 20   CONTINUE

      DO 100 I = 1, N
         DO 50 K = AI(I), AI(I+1)-1
            DO 40 J = 1, M
               W(I,J) = W(I,J) + AN(K)*P(AJ(K),J)
               W(AJ(K),J) = W(AJ(K),J) + AN(K)*P(I,J)
 40         CONTINUE
 50      CONTINUE
 100  CONTINUE

      DO 200 I = 1, M
         DO 180 J = I, M
            C(I,J) = 0.0
            DO 160 K = 1, N
               C(I,J) = C(I,J) + P(K,I)*W(K,J)
 160        CONTINUE
            C(J,I) = C(I,J)
 180     CONTINUE
 200  CONTINUE

      RETURN
      END

***********************************************************************

      SUBROUTINE ZHSTRF( N, M, AI, AJ, AN, AD, P, C, W )
      INTEGER N, AI(*), AJ(*)
      COMPLEX*16 AN(*), AD(*), P(N,M), C(M,M), W(N,M)

*     This is the "transform" routine.  It multiplies P'*A*P to form the
*     result C, where P and C are dense and A is a COMPLEX*16, sparse,
*     hermitian matrix in upper, unordered form.

*     Input:	N               The order of A.
*               M               The number of columns of P.
*               AI,AJ,AN,AD     The N by N matrix A.
*               P               The N by M matrix P.
*               W               A work matrix, N by M.

*     Output:   C               The M by M result.

      DO 20 I = 1, N
         DO 10 J = 1, M
            W(I,J) = AD(I)*P(I,J)
 10      CONTINUE
 20   CONTINUE

      DO 100 I = 1, N
         DO 50 K = AI(I), AI(I+1)-1
            DO 40 J = 1, M
               W(I,J) = W(I,J) + AN(K)*P(AJ(K),J)
               W(AJ(K),J) = W(AJ(K),J) + CONJG(AN(K))*P(I,J)
 40         CONTINUE
 50      CONTINUE
 100  CONTINUE

      DO 200 I = 1, M
         DO 180 J = I, M
            C(I,J) = ( 0.0, 0.0 )
            DO 160 K = 1, N
               C(I,J) = C(I,J) + CONJG(P(K,I))*W(K,J)
 160        CONTINUE
            C(J,I) = CONJG(C(I,J))
 180     CONTINUE
         C(I,I) = DBLE(C(I,I))
 200  CONTINUE

      RETURN
      END
