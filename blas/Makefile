
#---------------------------------------------------------------------
# This file is a Makefile (or its template) for the blas directory of
# Algae.  It is run by the Makefile in the parent directory.
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
ARFLAGS  = 

# Some bozos hardwire f77 options in their builtin make rules.  If
# we're using a different Fortran compiler (say, if it's a script that
# calls f2c), then you may need to use the following rule.

#.f.o:
#	$(FC) $(FFLAGS) -c $<

# Source files.

SRC = dasum.f daxpy.f dcopy.f ddot.f dgemm.f dgemv.f dger.f drot.f \
  dscal.f dswap.f dsymm.f dsymv.f dsyr.f dsyrk.f dtrmm.f dtrmv.f dtrsm.f \
  dtrsv.f dzasum.f idamax.f izamax.f lsame.f xerbla.f zaxpy.f zcopy.f \
  zdotc.f zdotu.f zdscal.f zgemm.f zgemv.f zgerc.f zgeru.f zhemv.f \
  zher.f zherk.f zscal.f zswap.f ztrmm.f ztrmv.f ztrsm.f ztrsv.f \
  dnrm2.f dsyr2k.f dznrm2.f zher2k.f dsyr2.f zhemm.f zher2.f dcabs1.f

# Objects.

OBJ = dasum.o daxpy.o dcopy.o ddot.o dgemm.o dgemv.o dger.o drot.o \
  dscal.o dswap.o dsymm.o dsymv.o dsyr.o dsyrk.o dtrmm.o dtrmv.o dtrsm.o \
  dtrsv.o dzasum.o idamax.o izamax.o lsame.o xerbla.o zaxpy.o zcopy.o \
  zdotc.o zdotu.o zdscal.o zgemm.o zgemv.o zgerc.o zgeru.o zhemv.o \
  zher.o zherk.o zscal.o zswap.o ztrmm.o ztrmv.o ztrsm.o ztrsv.o \
  dnrm2.o dsyr2k.o dznrm2.o zher2k.o dsyr2.o zhemm.o zher2.o dcabs1.o

# ============================ Targets ============================

all: libblas.a
.PHONY:	all

libblas.a: $(OBJ)
	$(AR) $(ARFLAGS) cr libblas.a $(OBJ)
	$(RANLIB) libblas.a

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
