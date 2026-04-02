***************************** -*- Mode: Fortran -*- ********************
*
* bcs_sp.f -- Interface routines for single precision BCSLIB.
* 
* Copyright (C) 1994-95  K. Scott Hunziker.
*
* See the file COPYING for license, warranty, and permission details.
*
* The BCSLIB routines have the unfortunate convention of passing
* character strings to specify options.  This makes it difficult to make
* portable calls to BCSLIB from C routines.  Fortunately, the BCSLIB
* convention is that only the first character is significant.  The
* routines below simply call their corresponding BCSLIB routines, but
* they take integers in place of the character strings.  The integers
* specify the ASCII value of the characters to be passed on to BCSLIB.
*
* $Id: bcs_sp.f,v 1.1.1.1 1996/04/17 05:56:15 ksh Exp $
*
************************************************************************
*
      SUBROUTINE HXSLIN( NEQNS, MTXTYP, MSGLVL, IPU,
     1                   SQFILE, WAFIL1, WAFIL2, HOLD, NHOLD, IER )
*
      INTEGER NEQNS, MSGLVL, IPU,
     1        NHOLD, SQFILE, WAFIL1, WAFIL2, IER, MTXTYP
      COMPLEX*16 HOLD(*)
*
      CALL HCSLIN( NEQNS, CHAR(MTXTYP), MSGLVL, IPU,
     1             SQFILE, WAFIL1, WAFIL2, HOLD, NHOLD, IER )
*
      RETURN
      END
*
************************************************************************
*
      SUBROUTINE HRSLIN( NEQNS, MTXTYP, MSGLVL, IPU,
     1                   SQFILE, WAFIL1, WAFIL2, HOLD, NHOLD, IER )
*
      INTEGER NEQNS, MSGLVL, IPU,
     1        NHOLD, SQFILE, WAFIL1, WAFIL2, IER, MTXTYP
      DOUBLE PRECISION HOLD(*)
*
      CALL HSSLIN( NEQNS, CHAR(MTXTYP), MSGLVL, IPU,
     1             SQFILE, WAFIL1, WAFIL2, HOLD, NHOLD, IER )
*
      RETURN
      END
