# This file is part of Gibbon, a graphical frontend to the First Internet 
# Backgammon Server FIBS.
# Copyright (C) 2009-2010 Guido Flohr, http://guido-flohr.net/.
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

SUBDIRS = tests

noinst_LIBRARIES = libgsgf.a

platform_libadd =

if MSDOS
platform_libadd += -ldnsapi -lws2_32
endif

AM_CPPFLAGS =				\
	-I$(top_srcdir)			\
	-DVERSION=\"@VERSION@\"		\
	$(GIBBON_CFLAGS)

libgsgf_a_SOURCES = 			\
	gsgf-collection.c		\
	gsgf-cooked-value.c		\
	gsgf-error.c			\
	gsgf-flavor.c			\
	gsgf-game-tree.c		\
	gsgf-node.c			\
	gsgf-number.c			\
	gsgf-property.c			\
	gsgf-raw.c			\
	gsgf-real.c			\
	gsgf-simple-text.c		\
	gsgf-text.c			\
	libgsgf.c			\
	libgsgf-util.c

noinst_HEADERS =			\
	gsgf.h				\
	gsgf-collection.h		\
	gsgf-cooked-value.h		\
	gsgf-error.h			\
	gsgf-flavor.h			\
	gsgf-game-tree.h		\
	gsgf-node.h			\
	gsgf-number.h			\
	gsgf-private.h			\
	gsgf-property.h			\
	gsgf-raw.h			\
	gsgf-real.h			\
	gsgf-simple-text.h		\
	gsgf-text.h			\
	libgsgf.h
