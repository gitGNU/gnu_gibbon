# This file is part of Gibbon, a graphical frontend to the First Internet 
# Backgammon Server FIBS.
# Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
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

SUBDIRS = libgsgf libsvg libsvg-cairo src data pixmaps po share docs win32
if BUILD_HELP 
SUBDIRS += help
endif

DISTCLEANFILES =		\
	intltool-extract	\
	intltool-merge		\
	intltool-update

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

noinst_HEADERS = gibbon-geo-ip-data.h

geo_ipdir = $(datadir)/$(PACKAGE)
geo_ip_DATA = ip2country.csv.gz

# This target is meant for maintainer maintainers only.
geo_ip:
	$(SHELL) $(srcdir)/download_uri \
		"http://software77.net/geo-ip/?DL=1" \
			>$(srcdir)/ip2country.csv.gz.new \
		&& mv -f $(srcdir)/ip2country.csv.gz.new \
			$(srcdir)/ip2country.csv.gz \
		&& $(SED) "s/@GIBBON_GEO_IP_DATA_UPDATE@/`date +%s`/" \
			<$(srcdir)/gibbon-geo-ip-data.h.tmpl \
			>$(srcdir)/gibbon-geo-ip-data.h

EXTRA_DIST = COPYING.LESSER README.libsvg README.libsvg-cairo config.rpath \
	gcreate_class.pl.in gibbon-geo-ip-data.h.tmpl \
	ip2country.csv.gz README-alpha

.PHONY: ChangeLog geo_ip
