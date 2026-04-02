/* src/fortran.h.  Generated from fortran.h.in by configure. */

/*
   fortran.h -- Macros for FORTRAN subroutine and common block names.

   Copyright (C) 1994-2002  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
*/

/* $Id: fortran.h.in,v 1.4 2002/10/01 01:21:48 ksh Exp $ */

#ifndef FORTRAN_H
#define FORTRAN_H	1

#if !NO_FORTRAN_DEFS
#define BAREL	barel_
#define CFFTB	cfftb_
#define CFFTF	cfftf_
#define CFFTI	cffti_
#define CGSTRN	cgstrn_
#define DGECO	dgeco_
#define DGEDI	dgedi_
#define DGESL	dgesl_
#define DGSADD	dgsadd_
#define DGSMUL	dgsmul_
#define DGSSUB	dgssub_
#define DGSTRN	dgstrn_
#define DOFLAB	doflab_
#define DPSFAC	dpsfac_
#define DPSSOL	dpssol_
#define DVERK	dverk_
#define DSVDC	dsvdc_
#define DYNEND	dynend_
#define DYNIOC	dynioc_
#define DYNIOH	dynioh_
#define DYNION	dynion_
#define ETIME_FUNCTION	etime_
#define EXPVEC	expvec_
#define EZGVAL	ezgval_
#define EZGAVV	ezgavv_
#define GENJAC	genjac_
#define GETENT	getent_
#define GETSUM	getsum_
#define GPSKCA	gpskca_
#define IAC8CA	iac8ca_
#define IAC8CD	iac8cd_
#define IAC8CR	iac8cr_
#define IACOND	iacond_
#define IGSADD	igsadd_
#define IGSMUL	igsmul_
#define IGSSUB	igssub_
#define IGSTRN	igstrn_
#define INNUM	innum_
#define MASEL	masel_
#define MEGVAL	megval_
#define MOVALN	movaln_
#define NPSOL	npsol_
#define NPOPTE	npopte_
#define NPOPTN	npoptn_
#define PUTENT	putent_
#define PUTSUM	putsum_
#define RFFTB	rfftb_
#define RFFTF	rfftf_
#define RFFTI	rffti_
#define SGSTRN	sgstrn_
#define SORTZ4	sortz4_
#define TLRNC	tlrnc_
#define WRTMAT	wrtmat_
#define XGSADD	xgsadd_
#define XGSMUL	xgsmul_
#define XGSTRN	xgstrn_
#define XPSFAC	xpsfac_
#define ZGECO	zgeco_
#define ZGEDI	zgedi_
#define ZGESL	zgesl_
#define ZGSADD	zgsadd_
#define ZGSMUL	zgsmul_
#define ZGSSUB	zgssub_
#define ZGSTRN	zgstrn_
#define ZSVDC	zsvdc_
#endif

extern void PROTO(GPSKCA, (int *n, int *degree, int *rstart, int *connec,
                           int *optpro, int *wrklen, int *permut, int *work,
                           int *bandwd, double *profil, int *error, int *space,
                           int *adjcnt));

#endif				/* FORTRAN_H */
