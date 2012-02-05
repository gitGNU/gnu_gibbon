# libsvg-cairo - Render SVG documents using the cairo library
#
# Copyright (C) 2002 University of Southern California
# Copyright (C) 2009-2012 Guido Flohr
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with GCinema; if not, see <http://www.gnu.org/licenses/>.
#
# Original author: Carl D. Worth <cworth@isi.edu>
 
noinst_LIBRARIES = libsvg-cairo.a
noinst_HEADERS = svg-cairo.h

AM_CPPFLAGS =				\
	-I$(top_srcdir)			\
	-I$(srcdir)			\
	-I$(top_srcdir)/libsvg		\
	$(GIBBON_CFLAGS)

libsvg_cairo_a_SOURCES = \
	svg_cairo.c \
	svg-cairo.h \
	svg-cairo-internal.h \
	svg_cairo_sprintf_alloc.c \
	svg_cairo_state.c
