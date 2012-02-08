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

AUTOMAKE_OPTIONS = color-tests

TESTS_C = test-convert			\
	  test-compose			\
	  test-compressed-list		\
	  test-date			\
	  test-empty 			\
	  test-full			\
	  test-game-info-properties	\
	  test-markup-properties	\
	  test-minimal			\
	  test-misc-properties		\
	  test-multi-game-tree		\
	  test-move-annotation		\
	  test-move-properties		\
	  test-nested-game-tree		\
	  test-node-annotation		\
	  test-non-unique-points	\
	  test-number			\
	  test-real			\
	  test-real-to-string		\
	  test-really-empty		\
	  test-root-properties		\
	  test-setup-properties		\
	  test-timing-properties	\
	  test-util-read-simpletext	\
	  test-util-read-text		\
	  test-write-empty		\
	  test-write-empty-property	\
	  test-write-full		\
	  test-write-linefeeds		\
	  test-write-minimal		\
	  test-formatd

TESTS_SH = test-write-stream

TESTS = $(TESTS_C) $(TESTS_SH)

AM_CPPFLAGS = 				\
    -I$(top_srcdir)			\
    -DTEST_DIR=\"$(abs_srcdir)\"	\
    -DTEST_BUILDDIR=\"$(abs_builddir)\"	\
    $(GIBBON_CFLAGS)

check_PROGRAMS = $(TESTS_C)
noinst_PROGRAMS = gsgf-write-back

LDADD = ../libgsgf.a $(GIBBON_LIBS) 

test_convert_SOURCES = lib.c main.c test-convert.c
test_compose_SOURCES = lib.c test-compose.c
test_compressed_list_SOURCES = lib.c main.c test-compressed-list.c
test_date_SOURCES = lib.c test-date.c
test_empty_SOURCES = lib.c main.c test-empty.c
test_full_SOURCES = lib.c main.c test-full.c
test_markup_properties_SOURCES = lib.c main.c test-markup-properties.c
test_game_info_properties_SOURCES = lib.c main.c test-game-info-properties.c
test_minimal_SOURCES = lib.c main.c test-minimal.c
test_misc_properties_SOURCES = lib.c main.c test-misc-properties.c
test_move_properties_SOURCES = lib.c main.c test-move-properties.c
test_move_annotation_SOURCES = lib.c main.c test-move-annotation.c
test_multi_game_tree_SOURCES = lib.c main.c test-multi-game-tree.c
test_nested_game_tree_SOURCES = lib.c main.c test-nested-game-tree.c
test_node_annotation_SOURCES = lib.c main.c test-node-annotation.c
test_non_unique_points_SOURCES = lib.c main.c test-non-unique-points.c
test_number_SOURCES = lib.c test-number.c
test_real_SOURCES = lib.c test-real.c
test_real_to_string_SOURCES = test-real-to-string.c
test_really_empty_SOURCES = lib.c main.c test-really-empty.c
test_root_properties_SOURCES = lib.c main.c test-root-properties.c
test_setup_properties_SOURCES = lib.c main.c test-setup-properties.c
test_timing_properties_SOURCES = lib.c main.c test-timing-properties.c
test_util_read_simpletext_SOURCES = lib.c test-util-read-simpletext.c
test_util_read_text_SOURCES = lib.c test-util-read-text.c
test_write_empty_SOURCES = lib.c main.c test-write-empty.c
test_write_empty_property_SOURCES = lib.c main.c test-write-empty-property.c
test_write_full_SOURCES = lib.c main.c test-write-full.c
test_write_linefeeds_SOURCES = lib.c main.c test-write-linefeeds.c
test_write_minimal_SOURCES = lib.c main.c test-write-minimal.c
test_formatd_SOURCES = test-formatd.c
gsgf_write_back_SOURCES = gsgf-write-back.c

noinst_HEADERS = test.h

SGF_IN_FILES = compressed-list.sgf.in	\
	       full.sgf.in		\
	       utf-8.sgf.in

SGF_FILES_GENERATED = $(SGF_IN_FILES:.in=)

SGF_FILES = cp1251.sgf			\
	    empty.sgf 			\
	    game-info-properties.sgf	\
	    linefeeds.sgf		\
	    markup-properties.sgf	\
	    minimal.sgf			\
	    move-annotation.sgf		\
	    move-properties.sgf		\
	    misc-properties.sgf		\
	    multi-game-tree.sgf		\
	    nested-game-tree.sgf	\
	    node-annotation.sgf		\
	    non-unique-points.sgf	\
	    really-empty.sgf		\
	    root-properties.sgf		\
	    setup-properties.sgf	\
	    timing-properties.sgf

TESTS_ENVIRONMENT = builddir=$(builddir) \
                    top_srcdir=$(top_srcdir) \
                    top_builddir=$(top_builddir) \
                    SGF_FILES="$(SGF_FILES) $(SGF_IN_FILES)"

EXTRA_DIST = $(SGF_FILES) $(SGF_IN_FILES) $(TESTS_SH)
