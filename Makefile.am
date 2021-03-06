##  -*- Mode: Makefile -*-
## Makefile.am --- process this file with automake to produce Makefile.in
##
## This file is part of AutoGen.
## AutoGen Copyright (C) 1992-2020 by Bruce Korb - all rights reserved
##
## AutoGen is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## AutoGen is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program.  If not, see <http://www.gnu.org/licenses/>.

ACLOCAL_AMFLAGS = -I config

SUBDIRS        = compat snprintfv autoopts agen5

if DO_SHELL_CMDS
if  HAVE_XML_LIB
SUBDIRS       += columns getdefs xml2ag doc pkg
else
SUBDIRS       += columns getdefs pkg
endif
else

## Without shell commands, you cannot build docs or packages and the
## columns and getdefs commands are unworkable.
##
if  HAVE_XML_LIB
SUBDIRS       += xml2ag
endif
endif

pkgdata_DATA   = config/liboptschk.m4
misc_extra     = VERSION config build-aux


EXTRA_DIST     = $(misc_extra) $(pkgdata_DATA)
DISTCLEANFILES = stamp-h

distcleancheck_listfiles = \
     find -type f -exec 'test -f $(srcdir)/{} || echo {} ;'

configure   : VERSION

pkg         : package
package     :
	cd pkg && $(MAKE) $@ pkgtype="$(pkgtype)"

docs        : gnudocs doxydocs
gnudoc      : gnudocs
gnudocs     :
	cd doc && $(MAKE) gnudocs

doxydocs    :
	test -f Doxyfile || cp ~/.Doxyfile ./Doxyfile ; \
	doxygen

usage-txt.po: gettext
gettext     :
	cd autoopts && $(MAKE) usage-txt.po

all         : shdefs
shdefs      : $(top_builddir)/config/shdefs
$(top_builddir)/config/shdefs: $(top_builddir)/config/mk-shdefs
	$(SHELL) $< $@

if HAVE_XML_LIB
release     : distcheck
else
release     :
	echo "Distributions cannot be made with a partial build" >&2
	exit 1
endif

.NOTPARALLEL:
.PHONY: all pkg package docs gnudoc gnudocs shdefs doxydocs gettext usage-txt.po

## Makefile.am ends here
