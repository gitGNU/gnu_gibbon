# libsvg - Parse SVG documents
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

noinst_LIBRARIES = libsvg.a
noinst_HEADERS = svg.h

AM_CPPFLAGS =				\
	-I$(top_srcdir)			\
	-I$(srcdir)			\
	$(GIBBON_CFLAGS)

libsvg_a_SOURCES = \
	svg.c \
	svg.h \
	svgint.h \
	svg_ascii.h \
	svg_ascii.c \
	svg_attribute.c \
	svg_color.c \
	svg_element.c \
	svg_gradient.c \
	svg_group.c \
	svg_image.c \
	svg_length.c \
	svg_paint.c \
	svg_parser.c \
	svg_parser_libxml.c \
	svg_pattern.c \
	svg_path.c \
	svg_str.c \
	svg_style.c \
	svg_text.c \
	svg_transform.c
