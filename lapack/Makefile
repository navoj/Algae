
#---------------------------------------------------------------------
# This file is a Makefile (or its template) for the lapack directory
# of Algae.  It is run by the Makefile in the parent directory.
#---------------------------------------------------------------------

.SUFFIXES:
.SUFFIXES: .f .o

srcdir = .


SHELL=/bin/sh

# Tools

F77	= gfortran
FC	= $(F77)
RANLIB	= ranlib
AR	= ar

# Flags.

FFLAGS   = -g -O2
FFLAGS_NOOPT   = -g 
ARFLAGS  = 

# Some bozos hardwire f77 options in their builtin make rules.  If
# we're using a different Fortran compiler (say, if it's a script that
# calls f2c), then you may need to use the following rule.

#.f.o:
#	$(FC) $(FFLAGS) -c $<

# Source files.

SRC = dbdsqr.f dgebak.f dgebal.f dgebrd.f dgecon.f dgeev.f dgegv.f \
  dgehrd.f dgelqf.f dgels.f dgeqrf.f dgesvd.f dgetrf.f dgetrs.f dggbak.f \
  dggbal.f dgghrd.f dhgeqz.f dhseqr.f dlabad.f dlabrd.f dlacon.f \
  dlacpy.f dladiv.f dlahqr.f dlahrd.f dlamch.f dlange.f dlanhs.f \
  dlansb.f dlanst.f dlansy.f dlarf.f dlarfb.f dlarfg.f dlarft.f dlarfx.f \
  dlargv.f dlartg.f dlartv.f dlascl.f dlaset.f dlasr.f dlasrt.f dlassq.f \
  dlaswp.f dlasyf.f dlatrd.f dlatrs.f dorgbr.f dorghr.f dorglq.f \
  dorgql.f dorgqr.f dorgtr.f dormbr.f dormlq.f dormqr.f dpocon.f dposv.f \
  dpotrf.f dpotrs.f drscl.f dsbev.f dsbtrd.f dsteqr.f dsterf.f dsycon.f \
  dsyev.f dsygst.f dsygv.f dsysv.f dsytrd.f dsytrf.f dsytrs.f dtgevc.f \
  dtrevc.f ilaenv.f lsame.f xerbla.f zbdsqr.f zdrot.f zdrscl.f zgebak.f \
  zgebal.f zgebrd.f zgecon.f zgeev.f zgehrd.f zgelqf.f zgels.f zgeqrf.f \
  zgesvd.f zgetrf.f zgetrs.f zhbev.f zhbtrd.f zhecon.f zheev.f zhesv.f \
  zhetrd.f zhetrf.f zhetrs.f zhseqr.f zlabrd.f zlacgv.f zlacon.f \
  zlacpy.f zlacrt.f zladiv.f zlaesy.f zlahef.f zlahqr.f zlahrd.f \
  zlange.f zlanhb.f zlanhe.f zlanhs.f zlarf.f zlarfb.f zlarfg.f zlarft.f \
  zlarfx.f zlargv.f zlartg.f zlartv.f zlascl.f zlaset.f zlasr.f zlassq.f \
  zlaswp.f zlasyf.f zlatrd.f zlatrs.f zpocon.f zposv.f zpotrf.f zpotrs.f \
  zrot.f zsteqr.f zsycon.f zsyr.f zsysv.f zsytrf.f zsytrs.f ztrevc.f \
  zungbr.f zunghr.f zunglq.f zungql.f zungqr.f zungtr.f zunmbr.f \
  zunmlq.f zunmqr.f \
  dlapy2.f dgehd2.f dgeqr2.f dgetf2.f dlag2.f dlasv2.f dlapy3.f \
  dlanv2.f dorgl2.f dorg2r.f dorml2.f dorm2r.f dpotf2.f dlar2v.f \
  dlaev2.f dlae2.f dsytd2.f dsytf2.f dlaln2.f zgehd2.f zgeqr2.f zgetf2.f \
  zlar2v.f zhetd2.f zhetf2.f dzsum1.f izmax1.f zpotf2.f zsytf2.f \
  zungl2.f zung2r.f zunml2.f dlasq1.f dlas2.f dgebd2.f dgelq2.f dsygs2.f \
  zgebd2.f zgelq2.f zung2l.f dorg2l.f zunm2r.f zlaev2.f dlasq2.f dlasq3.f \
  dlasq4.f dlasq5.f dlasq6.f zggev.f dggev.f zhegv.f zhegst.f zhegs2.f \
  zggbal.f zgghrd.f zhgeqz.f ztgevc.f zggbak.f ztrexc.f ztrsyl.f \
  dgeequ.f dpoequ.f zgeequ.f zpoequ.f zlarnv.f ztrsen.f \
  dlarnv.f dlaruv.f second.f dtrsen.f dtrexc.f dtrsyl.f dlaexc.f dlasy2.f

# Objects.

OBJ = dbdsqr.o dgebak.o dgebal.o dgebrd.o dgecon.o dgeev.o dgegv.o \
  dgehrd.o dgelqf.o dgels.o dgeqrf.o dgesvd.o dgetrf.o dgetrs.o dggbak.o \
  dggbal.o dgghrd.o dhgeqz.o dhseqr.o dlabad.o dlabrd.o dlacon.o \
  dlacpy.o dladiv.o dlahqr.o dlahrd.o dlamch.o dlange.o dlanhs.o \
  dlansb.o dlanst.o dlansy.o dlarf.o dlarfb.o dlarfg.o dlarft.o dlarfx.o \
  dlargv.o dlartg.o dlartv.o dlascl.o dlaset.o dlasr.o dlasrt.o dlassq.o \
  dlaswp.o dlasyf.o dlatrd.o dlatrs.o dorgbr.o dorghr.o dorglq.o \
  dorgql.o dorgqr.o dorgtr.o dormbr.o dormlq.o dormqr.o dpocon.o dposv.o \
  dpotrf.o dpotrs.o drscl.o dsbev.o dsbtrd.o dsteqr.o dsterf.o dsycon.o \
  dsyev.o dsygst.o dsygv.o dsysv.o dsytrd.o dsytrf.o dsytrs.o dtgevc.o \
  dtrevc.o ilaenv.o lsame.o xerbla.o zbdsqr.o zdrot.o zdrscl.o zgebak.o \
  zgebal.o zgebrd.o zgecon.o zgeev.o zgehrd.o zgelqf.o zgels.o zgeqrf.o \
  zgesvd.o zgetrf.o zgetrs.o zhbev.o zhbtrd.o zhecon.o zheev.o zhesv.o \
  zhetrd.o zhetrf.o zhetrs.o zhseqr.o zlabrd.o zlacgv.o zlacon.o \
  zlacpy.o zlacrt.o zladiv.o zlaesy.o zlahef.o zlahqr.o zlahrd.o \
  zlange.o zlanhb.o zlanhe.o zlanhs.o zlarf.o zlarfb.o zlarfg.o zlarft.o \
  zlarfx.o zlargv.o zlartg.o zlartv.o zlascl.o zlaset.o zlasr.o zlassq.o \
  zlaswp.o zlasyf.o zlatrd.o zlatrs.o zpocon.o zposv.o zpotrf.o zpotrs.o \
  zrot.o zsteqr.o zsycon.o zsyr.o zsysv.o zsytrf.o zsytrs.o ztrevc.o \
  zungbr.o zunghr.o zunglq.o zungql.o zungqr.o zungtr.o zunmbr.o \
  zunmlq.o zunmqr.o \
  dlapy2.o dgehd2.o dgeqr2.o dgetf2.o dlag2.o dlasv2.o dlapy3.o \
  dlanv2.o dorgl2.o dorg2r.o dorml2.o dorm2r.o dpotf2.o dlar2v.o \
  dlaev2.o dlae2.o dsytd2.o dsytf2.o dlaln2.o zgehd2.o zgeqr2.o zgetf2.o \
  zlar2v.o zhetd2.o zhetf2.o dzsum1.o izmax1.o zpotf2.o zsytf2.o \
  zungl2.o zung2r.o zunml2.o dlasq1.o dlas2.o dgebd2.o dgelq2.o dsygs2.o \
  zgebd2.o zgelq2.o zung2l.o dorg2l.o zunm2r.o zlaev2.o dlasq2.o dlasq3.o \
  dlasq4.o dlasq5.o dlasq6.o zggev.o dggev.o zhegv.o zhegst.o zhegs2.o \
  zggbal.o zgghrd.o zhgeqz.o ztgevc.o zggbak.o ztrexc.o ztrsyl.o \
  dgeequ.o dpoequ.o zgeequ.o zpoequ.o zlarnv.o ztrsen.o \
  dlarnv.o dlaruv.o second.o dtrsen.o dtrexc.o dtrsyl.o dlaexc.o dlasy2.o

# ============================ Targets ============================

all: liblapack.a
.PHONY:	all

# DLAMCH must be compiled without optimization.
dlamch.o: dlamch.f
	$(FC) $(FFLAGS_NOOPT) -c $<

liblapack.a: $(OBJ)
	$(AR) $(ARFLAGS) cr liblapack.a $(OBJ)
	$(RANLIB) liblapack.a

Makefile: Makefile.in ../config.status
	cd ..; ./config.status

# ========================== Distribution =========================

# Files in the source distribution from this directory.
DISTFILES = $(SRC) Makefile.in README

dist:
	for file in $(DISTFILES); do \
	  ln $$file $(distdir) > /dev/null 2>&1 || cp $$file $(distdir); \
	done
.PHONY:	dist

binaries:
.PHONY:	binaries

# ========================== Cleaning Up ==========================

.PHONY:	clean mostlyclean distclean maintainer-clean

clean mostlyclean distclean maintainer-clean::
	rm -f *.o *.a

distclean maintainer-clean::
	rm -f Makefile

maintainer-clean::

# ========================== Installation ==========================

install:
.PHONY:	install

uninstall:
.PHONY:	uninstall
