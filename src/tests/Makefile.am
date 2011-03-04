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

AUTOMAKE_OPTIONS = color-tests

TESTS_C = test-html-entities
TESTS_SH =

TESTS = $(TESTS_C)

AM_CPPFLAGS = 				\
    -I$(top_srcdir)			\
    -DTEST_DIR=\"$(abs_srcdir)\"	\
    -DTEST_BUILDDIR=\"$(abs_builddir)\"	\
    $(GIBBON_CFLAGS)

check_PROGRAMS = $(TESTS_C)

LDADD = $(GIBBON_LIBS) ../../libgsgf/libgsgf.a

test_html_entities_SOURCES = test-html-entities.c

TESTS_ENVIRONMENT = builddir=$(builddir) \
                    top_srcdir=$(top_srcdir) \
                    top_builddir=$(top_builddir)

EXTRA_DIST = $(TESTS_SH)
