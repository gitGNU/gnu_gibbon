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

AUTOMAKE_OPTIONS = color-tests

TESTS = test-empty 			\
	test-minimal			\
	test-multi-game-tree		\
	test-nested-game-tree		\
	test-really-empty		\
	test-write-empty		\
	test-write-minimal

AM_CPPFLAGS = 				\
    -I$(top_srcdir)			\
    -DTEST_DIR=\"$(abs_srcdir)\"		\
    $(GIBBON_CFLAGS)

check_PROGRAMS = $(TESTS) 

LDADD = $(GIBBON_LIBS) ../libgsgf.a

test_empty_SOURCES = main.c test-empty.c
test_minimal_SOURCES = main.c test-minimal.c
test_multi_game_tree_SOURCES = main.c test-multi-game-tree.c
test_nested_game_tree_SOURCES = main.c test-nested-game-tree.c
test_really_empty_SOURCES = main.c test-really-empty.c
test_write_empty_SOURCES = main.c test-write-empty.c
test_write_minimal_SOURCES = main.c test-write-minimal.c

noinst_HEADERS = test.h

SGF_FILES = empty.sgf 			\
	    minimal.sgf			\
	    multi-game-tree.sgf		\
	    nested-game-tree.sgf	\
	    really-empty.sgf

EXTRA_DIST = $(SGF_FILES)
