# This file is part of Gibbon.
# Copyright (C) 2009 Guido Flohr
# 
# Gibbon is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3 of the License, or (at your option) any later version.
#
# Gibbon is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with Gibbon; if not, see <http://www.gnu.org/licenses/>.

bin_PROGRAMS = gibbon

AM_CPPFLAGS = 				\
	-I$(top_srcdir)			\
	-I$(srcdir)			\
	$(GIBBON_CFLAGS)		\
	-DDATADIR=\""$(datadir)"\"	\
	-DLIBDIR=\""$(libdir)"\"

gibbon_SOURCES =	\
	gibbon.c 	\
	gtkmainwindow.c

noinst_HEADERS = game.h \
	gtkmainwindow.h

gibbon_LDFLAGS = -export-dynamic
gibbon_LDADD = $(GIBBON_LIBS)
