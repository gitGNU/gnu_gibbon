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

NULL =

# The name of the module

DOC_MODULE = libgsgf

# The top-level SGML file.
DOC_MAIN_SGML_FILE = libgsgf-docs.xml

# Extra options to supply to gtkdoc-scan
SCAN_OPTIONS = --rebuild-types --rebuild-sections

# The directory containing the source code. Relative to $(srcdir)
DOC_SOURCE_DIR = $(top_srcdir)/libgsgf

HFILE_GLOB = $(top_srcdir)/libgsgf/*.h
CFILE_GLOB = $(top_srcdir)/libgsgf/*.c

# Headers to ignore.
IGNORE_HFILES =				\
	gsgf.h				\
	gsgf-private.h			\
	gsgf-flavor-protected.h		\
	test.h

# CFLAGS and LDFLAGS needed for compiling scan program.  Only needed
# if $(DOC_MODULE).types is non-empty.
INCLUDES = \
	-I$(srcdir)			\
	-I$(top_srcdir)			\
	-I$(top_srcdir)/libgsgf		\
	$(GIBBON_CFLAGS)

GTKDOC_LIBS =					\
	$(top_builddir)/libgsgf/libgsgf.a	\
	$(GIBBON_LIBS)

MKDB_OPTIONS = --output-format=xml --sgml-mode --name-space=gsgf

content_files = version.xml overview.xml 	\
	flavors.xml				\
	flavor-general.xml			\
	flavor-backgammon.xml

expand_content_files = overview.xml
extra_files = version.xml.in

include $(top_srcdir)/gtk-doc.make

EXTRA_DIST += version.xml.in 		\
	flavor-general.xml		\
	flavor-backgammon.xml		\
	flavors.xml
