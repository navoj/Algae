
#---------------------------------------------------------------------
# This file is a Makefile (or its template) for the superlu directory
# of Algae.  It is run by the Makefile in the parent directory.
#---------------------------------------------------------------------

.SUFFIXES:
.SUFFIXES: .c .o

srcdir = .


INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

SHELL=/bin/sh

# Tools

CC	= gcc
AWK	= gawk
RANLIB	= ranlib
AR	= ar

#  C preprocessor defs for compilation (-DNoChange, -DAdd_, or -DUpCase)

CDEFS    = -DAdd_

# Flags.

CFLAGS   = -g -O2 -Wno-error=implicit-function-declaration -DHAVE_CONFIG_H $(CDEFS) -I..  -DUSE_VENDOR_BLAS
ARFLAGS  = 

# Source files.

C_SOURCES = colamd.c dcolumn_bmod.c dcolumn_dfs.c dcomplex.c dcopy_to_ucol.c \
  dgscon.c dgsequ.c dgsrfs.c dgssv.c dgssvx.c dgstrf.c dgstrs.c dlacon.c \
  dlangs.c dlaqgs.c dmemory.c dpanel_bmod.c dpanel_dfs.c dpivotgrowth.c \
  dpivotL.c dpruneL.c dreadhb.c dsnode_bmod.c dsnode_dfs.c dsp_blas2.c \
  dsp_blas3.c dutil.c dzsum1.c get_perm_c.c izmax1.c lsame.c memory.c mmd.c \
  relax_snode.c sp_coletree.c sp_ienv.c sp_preorder.c superlu_timer.c util.c \
  xerbla.c zcolumn_bmod.c zcolumn_dfs.c zcopy_to_ucol.c zgscon.c zgsequ.c \
  zgsrfs.c zgssv.c zgssvx.c zgstrf.c zgstrs.c zlacon.c zlangs.c zlaqgs.c \
  zmemory.c zpanel_bmod.c zpanel_dfs.c zpivotgrowth.c zpivotL.c zpruneL.c \
  zreadhb.c zsnode_bmod.c zsnode_dfs.c zsp_blas2.c zsp_blas3.c zutil.c

H_SOURCES = Cnames.h colamd.h dcomplex.h \
  dsp_defs.h ssp_defs.h supermatrix.h util.h zsp_defs.h

# Objects.

OBJ = colamd.o dcolumn_bmod.o dcolumn_dfs.o dcomplex.o dcopy_to_ucol.o \
  dgscon.o dgsequ.o dgsrfs.o dgssv.o dgssvx.o dgstrf.o dgstrs.o dlacon.o \
  dlangs.o dlaqgs.o dmemory.o dpanel_bmod.o dpanel_dfs.o dpivotgrowth.o \
  dpivotL.o dpruneL.o dreadhb.o dsnode_bmod.o dsnode_dfs.o dsp_blas2.o \
  dsp_blas3.o dutil.o dzsum1.o get_perm_c.o izmax1.o lsame.o memory.o mmd.o \
  relax_snode.o sp_coletree.o sp_ienv.o sp_preorder.o superlu_timer.o util.o \
  xerbla.o zcolumn_bmod.o zcolumn_dfs.o zcopy_to_ucol.o zgscon.o zgsequ.o \
  zgsrfs.o zgssv.o zgssvx.o zgstrf.o zgstrs.o zlacon.o zlangs.o zlaqgs.o \
  zmemory.o zpanel_bmod.o zpanel_dfs.o zpivotgrowth.o zpivotL.o zpruneL.o \
  zreadhb.o zsnode_bmod.o zsnode_dfs.o zsp_blas2.o zsp_blas3.o zutil.o

# ============================ Targets ============================

all: libsuperlu.a
.PHONY:	all

libsuperlu.a: $(OBJ)
	$(AR) $(ARFLAGS) cr libsuperlu.a $(OBJ)
	$(RANLIB) libsuperlu.a

Makefile: Makefile.in ../config.status
	cd ..; ./config.status

deps:
	$(AWK) -f ../src/depends.awk \
	  -v config=../config.h -v exclude=../src \
	  $(C_SOURCES) > DEPENDS || \
	  ( rm -f DEPENDS; false )
.PHONY: deps

tags:
	etags $(C_SOURCES) $(H_SOURCES)
.PHONY: tags

# ========================== Distribution =========================

# Files in the source distribution from this directory.
DISTFILES = $(C_SOURCES) $(H_SOURCES) Makefile.in DEPENDS \
  README README.changes

# Files included in the binary distribution from this directory.
# Makefile isn't specified here because it's handled separately.
BINDISTFILES = DEPENDS

dist: deps
	for file in $(DISTFILES); do \
	  ln $$file $(distdir) > /dev/null 2>&1 || cp $$file $(distdir); \
	done
.PHONY:	dist

binaries:
.PHONY:	binaries

# ========================== Cleaning Up ==========================

.PHONY:	clean mostlyclean distclean maintainer-clean

clean mostlyclean distclean maintainer-clean::
	rm -f *.o core *.a

distclean maintainer-clean::
	rm -f Makefile TAGS

maintainer-clean::

# ========================== Installation ==========================

install: all
.PHONY:	install

uninstall:
.PHONY:	uninstall

# ========================== Dependencies ==========================

DEPENDS:
	$(AWK) -f ../src/depends.awk \
	  $(C_SOURCES) > $@ || \
	  ( rm -f $@; false )

include DEPENDS
