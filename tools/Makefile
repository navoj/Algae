
#---------------------------------------------------------------------
# This file is a Makefile (or its template) for the tools directory
# of Algae.  It is run by the Makefile in the parent directory.
#---------------------------------------------------------------------

srcdir = .


INSTALL = /usr/bin/install -c
INSTALL_DATA = $(INSTALL) -m 644

AWK	= gawk

# ============================ Targets ============================

lib-Makefile: lib-Makefile.in
	$(AWK) '/>>>/ \
	  { \
	    while ("ls *.A" | getline) \
	      printf ("\t  source (\"%s\"); \\\n", $$0); \
	    next; \
	  } \
	  { print }' lib-Makefile.in \
	| sed -e 's|@BINARY@|../src/algae |' > $@ || ( rm -f $@ ; false )

tools.algae: lib-Makefile
	$(MAKE) -f lib-Makefile

# ========================== Distribution =========================

DISTFILES = Makefile.in *.A lib-Makefile.in
BINDISTFILES = Makefile *.A lib-Makefile.in

dist:
	for file in $(DISTFILES); do \
	  ln $$file $(distdir) > /dev/null 2>&1 || cp $$file $(distdir); \
	done
.PHONY:	dist

binaries: lib-Makefile
	for file in $(BINDISTFILES); do \
	  ln $$file $(distdir) > /dev/null 2>&1 || cp $$file $(distdir); \
	done
.PHONY:	binaries

# ========================== Installation =========================

install: tools.algae
	for f in *.A; do $(INSTALL_DATA) $$f $(tooldir); done
	$(INSTALL_DATA) tools.algae $(tooldir)
.PHONY:	install

uninstall:
	rm -f $(tooldir)/*.A $(tooldir)/tools.algae
.PHONY:	uninstall

# ========================== Cleaning Up ==========================

.PHONY:	clean mostlyclean distclean maintainer-clean

clean mostlyclean distclean maintainer-clean::
	rm -f tools.algae

distclean maintainer-clean::
	rm -f Makefile lib-Makefile
