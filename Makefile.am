# This file is part of Gibbon, a graphical frontend to the First Internet 
# Backgammon Server FIBS.
# Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
#
# Gibbon is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Gibbon is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Gibbon.  If not, see <http://www.gnu.org/licenses/>.

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = libgsgf libsvg libsvg-cairo src data pixmaps po share docs help

DISTCLEANFILES =		\
	intltool-extract	\
	intltool-merge		\
	intltool-update

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

noinst_HEADERS = gibbon-geoip-data.h

# This target is meant for maintainer maintainers only.
$(srcdir)/ip2country.csv.gz: $(srcdir)/download_uri
	$(SHELL) $(srcdir)/download_uri \
		"http://software77.net/geo-ip/?DL=1" >$@.new \
		&& mv -f $@.new $@ \
		&& $(SED) "s/@GIBBON_GEOIP_DATA_UPDATE@/`date +%s`/" \
			<$(srcdir)/gibbon-geoip-data.h.tmpl \
			>$(srcdir)/gibbon-geoip-data.h

EXTRA_DIST = COPYING.LESSER README.libsvg README.libsvg-cairo config.rpath \
	gcreate_class.pl.in ip2country.csv.gz gibbon-geoip-data.h.tmpl

.PHONY: ChangeLog ip2country.csv.gz
