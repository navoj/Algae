
#---------------------------------------------------------------------
# This file is a Makefile (or its template) for the arpack directory
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
ARFLAGS  = 

# Some bozos hardwire f77 options in their builtin make rules.  If
# we're using a different Fortran compiler (say, if it's a script that
# calls f2c), then you may need to use the following rule.

#.f.o:
#	$(FC) $(FFLAGS) -c $<

# Source files.

SRC = dgetv0.f dlaqrb.f dnaitr.f \
dnapps.f dnaup2.f dnaupd.f dnconv.f dneigh.f dneupd.f dngets.f \
dsaitr.f dsapps.f dsaup2.f dsaupd.f dsconv.f dseigt.f dsesrt.f dseupd.f \
dsgets.f dsortc.f dsortr.f dstatn.f dstats.f dstqrb.f zgetv0.f znaitr.f \
znapps.f znaup2.f znaupd.f zneigh.f zneupd.f zngets.f zsortc.f zstatn.f \
ivout.f dvout.f dmout.f zvout.f zmout.f

# Include files.

INC = debug.h stat.h

# Objects.

OBJ = dgetv0.o dlaqrb.o dnaitr.o \
dnapps.o dnaup2.o dnaupd.o dnconv.o dneigh.o dneupd.o dngets.o \
dsaitr.o dsapps.o dsaup2.o dsaupd.o dsconv.o dseigt.o dsesrt.o dseupd.o \
dsgets.o dsortc.o dsortr.o dstatn.o dstats.o dstqrb.o zgetv0.o znaitr.o \
znapps.o znaup2.o znaupd.o zneigh.o zneupd.o zngets.o zsortc.o zstatn.o	\
ivout.o dvout.o dmout.o zvout.o zmout.o

# ============================ Targets ============================

all: libarpack.a
.PHONY:	all

libarpack.a: $(OBJ)
	$(AR) $(ARFLAGS) cr libarpack.a $(OBJ)
	$(RANLIB) libarpack.a

Makefile: Makefile.in ../config.status
	cd ..; ./config.status

tags:
	etags $(SRC) $(INC)
.PHONY: tags

# ========================== Distribution =========================

# Files in the source distribution from this directory.
DISTFILES = $(SRC) $(INC) Makefile.in README

# Files included in the binary distribution from this directory.
# Makefile isn't specified here because it's handled separately.
BINDISTFILES =

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
	rm -f *.o core *.a

distclean maintainer-clean::
	rm -f Makefile TAGS

maintainer-clean::

# ========================== Installation ==========================

install: all
.PHONY:	install

uninstall:
.PHONY:	uninstall
