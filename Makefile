
# Makefile for Algae.

# Copyright (C) 1994-2003  K. Scott Hunziker.
# Copyright (C) 1990-1994  The Boeing Company.

# See the file COPYING for license, warranty, and permission details.

# =============================	Usage =============================

# <<<< This section is deleted in the "binary" distribution.

# make all
#	Compile the entire program.  This is the default target.

# >>>> End of deleted section.

# make check
#	Perform self-tests.  The program must be compiled first, but need
#	not be installed.

# make install
#	Compile the program and copy the executables, libraries, and so
#	on to the directories in which they should reside for actual use.
#	These directories, specified below, will be created if they don't
#	already exist.

# make uninstall
#	Delete the files that the "install" target would create.

# <<<< This section is deleted in the "binary" distribution.

# make clean
#	Delete all files from the current directory that are normally
#	created by building the program.

# make distclean
#	Delete all files from the current directory that are created by
#	configuring or building the program.  If you have unpacked the
#	source and built the program without creating any other files,
#	`make distclean' should leave only the files that were in the
#	distribution.

# make maintainer-clean
#	Delete everything from the current directory that can be
#	reconstructed with this Makefile.

# >>>> End of deleted section.

include VERSION

# If make doesn't predefine MAKE, set it here.


srcdir = .


INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

# ==================== Where to Install Things ====================

# NOTE:  Some of these paths are compiled into the code.  If you're
# going to change something here, be sure to do it before you compile
# anything.  (Or else do a `make clean' first.)

# Common prefix for installed files.  (Typically, /usr.)
prefix = /home/jtrujil1

# Directory in which to install binary.
bindir = $(prefix)/bin

# Directory in which to install machine-independent files.
datadir = $(prefix)/share

# Directory in which to install machine-dependent files.
libdir = $(prefix)/lib

# Directory in which to install the man page.
mandir = $(datadir)/man/man$(manext)
# The manpage extension
manext = 1

# Directory in which to install the Info files.
infodir = $(datadir)/info

# Directory in which to install the algae tools.
tooldir = $(datadir)/algae/$(VERSION_NUMBER)/tools

# Directory in which to install the algae html files.
htmldir = $(datadir)/algae/$(VERSION_NUMBER)/html

# Directory and name of the startup file.  This is compiled into the
# executable, so don't change either assignment if you have a binary
# distribution.  Also, if you make a change here you must also change
# the definition of ALGAE_RC0_DEF in "config.h" to match.
rcdir = $(libdir)/algae/$(VERSION_NUMBER)
RC0 = $(rcdir)/algae.A

# Directory containing site files: that is, files installed locally
# which are not part of the standard Algae distribution.
sitedir = $(datadir)/algae/site/A

# Temporary directory (relative path) for making a distribution.
distdir = tmp

# ============================ Targets ============================

SHELL = /bin/sh
.SUFFIXES:

# Subdirectories to make recursively.
BINSUBDIRS = doc etc examples src test timing tools
SUBDIRS = blas lapack superlu arpack fpe $(BINSUBDIRS)

# Files in the source distribution from this directory.
DISTFILES = COPYING INSTALL LICENSE Makefile.in NEWS PROBLEMS README \
    VERSION aclocal.m4 config.guess config.h.in config.sub configure.in \
    configure compile-sh install-sh mkinstalldirs rc0.in f77-sh.in \
    algae.spec.in

# Files in the binary distribution from this directory.  Makefile isn't
# included because it's handled separately.
BINDISTFILES = COPYING INSTALL LICENSE NEWS PROBLEMS README \
    VERSION install-sh mkinstalldirs rc0

# Files in the web site distribution from this directory.
WEBDISTFILES = COPYING LICENSE

# <<<< This section is deleted in the "binary" distribution.

all:	source algae.A
.PHONY: all

spec: algae.spec.in VERSION
	sed -e "s|VERSION_NUMBER|$(VERSION_NUMBER)|" \
	  algae.spec.in > algae-$(VERSION_NUMBER).spec || ( rm -f $@; false )
.PHONY: spec

Makefile: Makefile.in ./config.status
	./config.status

config.status: configure
	./config.status --recheck

configure: configure.in
	cd $(srcdir); autoconf

# Compile the source code.

source:  
	cd superlu; $(MAKE) RC0=$(RC0)
	cd arpack; $(MAKE)
	cd src; $(MAKE) RC0=$(RC0)
.PHONY: source

# Compile the BLAS and LAPACK libraries, if required.

xblas:
	cd blas; $(MAKE)
.PHONY: xblas

xlapack:
	cd lapack; $(MAKE)
.PHONY: xlapack

check: source
times: source

# >>>> End of deleted section.

algae.A: rc0
	sed -e "s|@tooldir@|$(tooldir)|" \
	    -e "s|@sitedir@|$(sitedir)|" \
	    -e "s|@lib_dir@|$(libdir)|" \
	    -e "s|@info_dir@|$(infodir)|" \
	    -e "s|${docdir}|$(htmldir)|" rc0 > $@ || rm -f $@

# Run test cases.

check:
	cd test; $(MAKE)
.PHONY: check

# Run some simple timing tests.

times:
	cd timing; $(MAKE)
.PHONY: times

# <<<< This section is deleted in the "binary" distribution.

# Make documentation.

info:
	cd doc; $(MAKE) info
.PHONY: info

dvi:
	cd doc; $(MAKE) dvi
.PHONY: dvi

# ========================== Distribution =========================

# Tar up the source distribution.

dist:	distdir
	rm -f algae-$(VERSION_NUMBER).tar.gz
	for file in $(DISTFILES); do \
	  ln $$file $(distdir) > /dev/null 2>&1 || cp $$file $(distdir); \
	done
	for dir in $(SUBDIRS); do \
	  cd $${dir}; \
	  $(MAKE) distdir=../$(distdir)/$$dir dist; \
	  cd ..; \
	done
	mv $(distdir) algae-$(VERSION_NUMBER)
	tar chf algae-$(VERSION_NUMBER).tar algae-$(VERSION_NUMBER)
	rm -rf algae-$(VERSION_NUMBER)
	gzip --best algae-$(VERSION_NUMBER).tar
.PHONY:	dist

# Tar up a binary distribution.

binaries: bindistdir
	rm -f algae-$(VERSION_NUMBER)-x86_64-unknown-linux-gnu.tar.gz
	for file in $(BINDISTFILES); do \
	  ln $$file $(distdir) > /dev/null 2>&1 || cp $$file $(distdir); \
	done
	awk '/<<[<]</,/>>[>]>/ {next} {print}' Makefile > $(distdir)/Makefile
	for dir in $(BINSUBDIRS); do \
	  cd $${dir}; \
	  $(MAKE) distdir=../$(distdir)/$$dir binaries; \
	  cd ..; \
	done
	mv $(distdir) algae-$(VERSION_NUMBER)
	tar chf algae-$(VERSION_NUMBER)-x86_64-unknown-linux-gnu.tar algae-$(VERSION_NUMBER)
	rm -rf algae-$(VERSION_NUMBER)
	gzip --best algae-$(VERSION_NUMBER)-x86_64-unknown-linux-gnu.tar
.PHONY:	binaries

# Tar up the web site files.

web:
	-rm -rf $(distdir)
	mkdir $(distdir)
	rm -f algae-web-$(VERSION_NUMBER).tar.gz
	for file in $(WEBDISTFILES); do \
	  ln $$file $(distdir) > /dev/null 2>&1 || cp $$file $(distdir); \
	done
	cd doc; $(MAKE) distdir=../$(distdir) web
	cd $(distdir); tar chf ../algae-web-$(VERSION_NUMBER).tar .
	rm -rf $(distdir)
	gzip algae-web-$(VERSION_NUMBER).tar
.PHONY: web

# Make a temporary subdirectory for the distribution files.

distdir:
	-rm -rf algae-$(VERSION_NUMBER) $(distdir)
	mkdir $(distdir)
	for dir in $(SUBDIRS); do mkdir $(distdir)/$${dir}; done
.PHONY:	distdir

bindistdir:
	-rm -rf algae-$(VERSION_NUMBER) $(distdir)
	mkdir $(distdir)
	for dir in $(BINSUBDIRS); do mkdir $(distdir)/$${dir}; done
.PHONY:	bindistdir

# ========================== Cleaning Up ==========================

.PHONY:	clean mostlyclean distclean maintainer-clean

maintainer-clean::
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

clean mostlyclean distclean maintainer-clean::
	for dir in $(SUBDIRS); do cd $${dir}; $(MAKE) $@; cd ..; done

clean mostlyclean distclean maintainer-clean::
	rm -f a.out core conft* algae-*.tar algae-*.tar.gz
	rm -rf algae-$(VERSION_NUMBER) $(distdir)

distclean maintainer-clean::
	rm -f config.h Makefile config.status config.cache config.log

# ========================== Installation =========================

install: all

# >>>> End of deleted section.

install: algae.A installdirs
	$(INSTALL_DATA) algae.A $(RC0)
	$(INSTALL_DATA) COPYING $(htmldir)
	$(INSTALL_DATA) LICENSE $(htmldir)
	cd src; \
	  $(MAKE) bindir=$(bindir) install
	cd tools; \
	  $(MAKE) tooldir=$(tooldir) install
	cd doc; \
	  $(MAKE) mandir=$(mandir) datadir=$(datadir) \
	  manext=$(manext) infodir=$(infodir) htmldir=$(htmldir) install
.PHONY:	install

# Make sure all installation directories actually exist by making them
# if necessary.

installdirs: mkinstalldirs
	$(srcdir)/mkinstalldirs $(bindir) $(datadir) $(libdir) $(infodir) \
          $(mandir) $(tooldir) $(rcdir) $(htmldir)
.PHONY:	installdirs

# Why would anyone want to uninstall?

uninstall:
	rm -f $(RC0)
	cd src; $(MAKE) bindir=$(bindir) uninstall
	cd tools; $(MAKE) tooldir=$(tooldir) uninstall
	cd doc; $(MAKE) mandir=$(mandir) manext=$(manext) \
	  infodir=$(infodir) htmldir=$(htmldir) uninstall
	rm -f $(htmldir)/COPYING $(htmldir)/LICENSE
	-rmdir $(rcdir) $(htmldir) $(tooldir) \
	  $(datadir)/algae/$(VERSION_NUMBER)
.PHONY:
