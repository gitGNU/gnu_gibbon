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

AUTOMAKE_OPTIONS = color-tests

TESTS = test-convert			\
	test-compose			\
	test-empty 			\
	test-full			\
	test-minimal			\
	test-multi-game-tree		\
	test-nested-game-tree		\
	test-number			\
	test-real			\
	test-really-empty		\
	test-root-properties		\
	test-util-read-simpletext	\
	test-util-read-text		\
	test-write-empty		\
	test-write-empty-property	\
	test-write-full			\
	test-write-linefeeds		\
	test-write-minimal

AM_CPPFLAGS = 				\
    -I$(top_srcdir)			\
    -DTEST_DIR=\"$(abs_srcdir)\"	\
    -DTEST_BUILDDIR=\"$(abs_builddir)\"	\
    $(GIBBON_CFLAGS)

check_PROGRAMS = $(TESTS) 

LDADD = $(GIBBON_LIBS) ../libgsgf.a

test_convert_SOURCES = lib.c main.c test-convert.c
test_compose_SOURCES = lib.c test-compose.c
test_empty_SOURCES = lib.c main.c test-empty.c
test_full_SOURCES = lib.c main.c test-full.c
test_minimal_SOURCES = lib.c main.c test-minimal.c
test_multi_game_tree_SOURCES = lib.c main.c test-multi-game-tree.c
test_nested_game_tree_SOURCES = lib.c main.c test-nested-game-tree.c
test_number_SOURCES = lib.c test-number.c
test_real_SOURCES = lib.c test-real.c
test_really_empty_SOURCES = lib.c main.c test-really-empty.c
test_root_properties_SOURCES = lib.c main.c test-root-properties.c
test_util_read_simpletext_SOURCES = lib.c test-util-read-simpletext.c
test_util_read_text_SOURCES = lib.c test-util-read-text.c
test_write_empty_SOURCES = lib.c main.c test-write-empty.c
test_write_empty_property_SOURCES = lib.c main.c test-write-empty-property.c
test_write_full_SOURCES = lib.c main.c test-write-full.c
test_write_linefeeds_SOURCES = lib.c main.c test-write-linefeeds.c
test_write_minimal_SOURCES = lib.c main.c test-write-minimal.c

noinst_HEADERS = test.h

SGF_FILES = cp1251.sgf			\
	    empty.sgf 			\
	    full.sgf.in			\
	    general-properties.sgf	\
	    linefeeds.sgf		\
	    minimal.sgf			\
	    multi-game-tree.sgf		\
	    nested-game-tree.sgf	\
	    really-empty.sgf		\
	    utf-8.sgf.in

EXTRA_DIST = $(SGF_FILES)

