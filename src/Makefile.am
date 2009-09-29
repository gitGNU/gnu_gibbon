# This file is part of Gibbon, a graphical frontend to the First Internet 
# Backgammon Server FIBS.
# Copyright (C) 2009 Guido Flohr, http://guido-flohr.net/.
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

bin_PROGRAMS = gibbon

AM_CPPFLAGS = 				\
	-I$(top_srcdir)			\
	-I$(srcdir)			\
	$(GIBBON_CFLAGS)		\
	-DGIBBON_DATADIR=\""$(datadir)"\"	\
	-DGIBBON_LIBDIR=\""$(libdir)"\"

gibbon_SOURCES =		\
	gibbon.c 		\
	gibbon-cairoboard.c	\
	gibbon-connection.c	\
	gibbon-connector.c	\
	gibbon-design.c		\
	gibbon-session.c	\
	gtkmainwindow.c		\
	gtkconnectiondialog.c	\
	gui.c

noinst_HEADERS =		\
	game.h			\
	gibbon.h		\
	gibbon-cairoboard.h	\
	gibbon-connection.h	\
	gibbon-connector.h	\
	gibbon-design.h		\
	gibbon-session.h	\
	gui.h

gibbon_LDFLAGS = -export-dynamic
gibbon_LDADD = $(GIBBON_LIBS)
