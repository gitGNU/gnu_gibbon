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

bin_PROGRAMS = gibbon gibbon-convert

AUTOMAKE_OPTIONS = color-tests

platform_libadd =

if WIN32
gibbon_LDFLAGS = -mwindows

gibbon-res.o: gibbon.rc ../share/icons/application/gibbon.ico
        $(WINDRES) -i gibbon.rc --input-format=rc -o $@ -O coff

gibbon.rc: gibbon.rc.in

platform_libadd += gibbon-res.o -ldnsapi -lws2_32
endif

if GIBBON_GTK_MIGRATION_PRE_FLIGHT
GIBBON_GTK_MIGRATION_CFLAGS =			\
        -DGTK_DISABLE_SINGLE_INCLUDES	\
        -DGDK_DISABLE_DEPRECATED	\
        -DGTK_DISABLE_DEPRECATED	\
        -DGSEAL_ENABLE
else
GIBBON_GTK_MIGRATION_CFLAGS =
endif

AM_CPPFLAGS = 				\
        -D_GNU_SOURCE			\
        -I$(top_srcdir)			\
        -I$(top_srcdir)/libsvg		\
        -I$(top_srcdir)/libsvg-cairo	\
        -I$(srcdir)			\
        $(GIBBON_CFLAGS)		\
        -DGIBBON_DATADIR=\""$(datadir)"\"	\
        -DGIBBON_LIBDIR=\""$(libdir)"\"	\
        -D_ISOC99_SOURCE		\
        $(GIBBON_GTK_MIGRATION_CFLAGS)	\
	-DABS_BUILDDIR=\""$(abs_builddir)"\"	\
	-DABS_SRCDIR=\""$(abs_srcdir)"\"	\
        -D_SVID_SOURCE

common_SOURCES = 			\
        gibbon-accept.c                 \
        gibbon-double.c                 \
        gibbon-drop.c                   \
        gibbon-game.c           	\
        gibbon-game-action.c            \
        gibbon-gmd-lexer.l        	\
        gibbon-gmd-parser.y       	\
        gibbon-gmd-reader.c       	\
        gibbon-gmd-writer.c       	\
        gibbon-java-fibs-lexer.l        \
        gibbon-java-fibs-parser.y       \
        gibbon-java-fibs-reader.c       \
        gibbon-java-fibs-writer.c       \
	gibbon-jelly-fish-lexer.l	\
	gibbon-jelly-fish-parser.y	\
	gibbon-jelly-fish-reader.c	\
        gibbon-jelly-fish-writer.c      \
        gibbon-move.c                   \
        gibbon-movement.c               \
        gibbon-match.c          	\
	gibbon-match-play.c		\
        gibbon-match-reader.c           \
        gibbon-match-writer.c           \
        gibbon-position.c		\
        gibbon-reject.c                 \
        gibbon-resign.c                 \
        gibbon-roll.c                   \
        gibbon-take.c                   \
        gibbon-util.c			\
        gibbon-sgf-reader.c		\
        gibbon-sgf-writer.c		\
	gibbon-analysis.c		\
	gibbon-analysis-roll.c		\
	gibbon-analysis-move.c		\
	gibbon-met.c			\
	gibbon-variant-list.c

AM_YFLAGS = -d -v

gibbon_SOURCES = 			\
        gibbon.c 			\
        gibbon-app.c			\
        gibbon-archive.c		\
        gibbon-board.c			\
        gibbon-cairoboard.c		\
        gibbon-chat.c			\
        gibbon-chat-view.c		\
        gibbon-client-icons.c		\
	gibbon-clip-lexer.l		\
        gibbon-clip-reader.c		\
        gibbon-connection.c		\
        gibbon-connection-dialog.c	\
        gibbon-country.c		\
        gibbon-database.c		\
        gibbon-fibs-command.c		\
        gibbon-fibs-message.c		\
        gibbon-game-chat.c		\
        gibbon-help.c			\
        gibbon-geo-ip-updater.c		\
        gibbon-inviter-list.c		\
        gibbon-inviter-list-view.c	\
        gibbon-java-fibs-importer.c	\
        gibbon-player-list.c		\
        gibbon-player-list-view.c	\
        gibbon-register-dialog.c	\
        gibbon-reliability.c		\
        gibbon-reliability-renderer.c	\
        gibbon-saved-info.c		\
        gibbon-server-console.c		\
        gibbon-session.c		\
        gibbon-settings.c		\
        gibbon-shouts.c			\
        gibbon-signal.c			\
        html-entities.c			\
        svg-util.c			\
	gibbon-match-list.c		\
	gibbon-match-loader.c		\
	gibbon-game-list-view.c		\
	gibbon-move-list-view.c		\
	gibbon-analysis-view.c		\
	gibbon-match-tracker.c		\
        $(common_SOURCES)

gibbon_convert_SOURCES =                \
        gibbon-convert.c                \
        $(common_SOURCES)

noinst_HEADERS =			\
        gibbon-accept.h			\
        gibbon-app.h			\
        gibbon-archive.h		\
        gibbon-board.h			\
        gibbon-cairoboard.h		\
        gibbon-chat.h			\
        gibbon-chat-view.h		\
        gibbon-client-icons.h		\
	gibbon-clip-reader.h		\
	gibbon-clip-reader-priv.h	\
        gibbon-connection.h		\
        gibbon-connection-dialog.h	\
        gibbon-country.h		\
        gibbon-database.h		\
        gibbon-double.h			\
        gibbon-drop.h			\
        gibbon-fibs-command.h		\
        gibbon-fibs-message.h		\
        gibbon-game.h           	\
        gibbon-game-action.h		\
	gibbon-game-actions.h		\
        gibbon-game-chat.h		\
        gibbon-geo-ip-updater.h		\
	gibbon-gmd-parser.h		\
	gibbon-gmd-reader.h		\
	gibbon-gmd-reader-priv.h	\
        gibbon-gmd-writer.h		\
        gibbon-help.h			\
        gibbon-inviter-list.h		\
        gibbon-inviter-list-view.h	\
        gibbon-java-fibs-importer.h	\
	gibbon-java-fibs-parser.h	\
        gibbon-java-fibs-reader.h	\
        gibbon-java-fibs-reader-priv.h	\
        gibbon-java-fibs-writer.h	\
	gibbon-jelly-fish-parser.h	\
	gibbon-jelly-fish-reader.h	\
	gibbon-jelly-fish-reader-priv.h	\
        gibbon-jelly-fish-writer.h	\
        gibbon-match.h          	\
	gibbon-match-play.h		\
        gibbon-match-reader.h		\
        gibbon-match-writer.h		\
        gibbon-move.h			\
        gibbon-movement.h		\
        gibbon-player-list.h		\
        gibbon-player-list-view.h	\
        gibbon-position.h		\
        gibbon-register-dialog.h	\
        gibbon-reject.h			\
        gibbon-reliability.h		\
        gibbon-reliability-renderer.h	\
        gibbon-resign.h			\
        gibbon-roll.h			\
        gibbon-saved-info.h		\
        gibbon-server-console.h		\
        gibbon-session.h		\
        gibbon-settings.h		\
        gibbon-sgf-reader.h		\
        gibbon-sgf-writer.h		\
        gibbon-shouts.h			\
        gibbon-signal.h			\
        gibbon-take.h			\
        gibbon-util.h			\
        html-entities.h			\
        svg-util.h			\
	gibbon-match-list.h		\
	gibbon-match-loader.h		\
	gibbon-game-list-view.h		\
	gibbon-move-list-view.h		\
	gibbon-analysis.h		\
	gibbon-analysis-roll.h		\
	gibbon-analysis-move.h		\
	gibbon-analysis-view.h		\
	gibbon-met.h			\
	gibbon-variant-list.h			\
	gibbon-match-tracker.h

# These dependencies are not automatically detected by GNU autotools.
# See https://savannah.nongnu.org/bugs/?35578.
gibbon-gmd-lexer.o: gibbon-gmd-parser.h
gibbon-java-fibs-lexer.o: gibbon-java-fibs-parser.h
gibbon-jelly-fish-lexer.o: gibbon-jelly-fish-parser.h

AM_LDFLAGS = -export-dynamic
LDADD = ../libsvg/libsvg.a ../libsvg-cairo/libsvg-cairo.a ../libgsgf/libgsgf.a \
        $(GIBBON_LIBS) $(platform_libadd)

## Put test_gary_wong_movegen at the end because it takes the most time.
TESTS_C = test_html_entities test_build_match test_position test_moves \
        test_clip_reader test_pretty_print_move test_strsplit	\
	test_crawford_detection test_gmd_reader \
	test_java_fibs_reader test_jelly_fish_reader test_sgf_reader \
	test_match_consistency test_add_drop test_gmd_reader_edited \
	test_sgf_reader_edited test_match_bugs test_position_transform \
        test_gary_wong_movegen
TESTS_SH = test_match_completion.sh

TESTS = $(TESTS_SH) $(TESTS_C)

check_PROGRAMS = test_html_entities test_build_match test_position test_moves \
        test_clip_reader test_pretty_print_move test_strsplit	\
	test_crawford_detection test_gmd_reader \
	test_java_fibs_reader test_jelly_fish_reader test_sgf_reader \
	test_match_consistency test_match_complete test_add_drop \
	test_gmd_reader_edited test_sgf_reader_edited \
	test_match_bugs test_position_transform \
        test_gary_wong_movegen

test_html_entities_SOURCES = $(common_SOURCES) html-entities.c \
	test-html-entities.c
test_build_match_SOURCES = $(common_SOURCES) test-build-match.c
test_position_SOURCES = $(common_SOURCES) test-position.c
test_moves_SOURCES = $(common_SOURCES) test-moves.c
test_gary_wong_movegen_SOURCES = $(common_SOURCES) test-gary-wong-movegen.c
test_pretty_print_move_SOURCES = $(common_SOURCES) test-pretty-print-move.c
test_strsplit_SOURCES = $(common_SOURCES) test-strsplit.c
test_clip_reader_SOURCES = $(common_SOURCES) gibbon-clip-reader.c \
	gibbon-clip-lexer.c test-clip-reader.c
test_crawford_detection_SOURCES = $(common_SOURCES) test-crawford-detection.c
test_gmd_reader_SOURCES = $(common_SOURCES) test-gmd-reader.c
test_java_fibs_reader_SOURCES = $(common_SOURCES) test-java-fibs-reader.c
test_jelly_fish_reader_SOURCES = $(common_SOURCES) test-jelly-fish-reader.c
test_sgf_reader_SOURCES = $(common_SOURCES) test-sgf-reader.c
test_match_consistency_SOURCES = $(common_SOURCES) test-match-consistency.c
test_match_complete_SOURCES = $(common_SOURCES) test-match-complete.c
test_add_drop_SOURCES = $(common_SOURCES) test-add-drop.c
test_gmd_reader_edited_SOURCES = $(common_SOURCES) test-gmd-reader-edited.c
test_sgf_reader_edited_SOURCES = $(common_SOURCES) test-sgf-reader-edited.c
test_match_bugs_SOURCES = $(common_SOURCES) test-match-bugs.c
test_position_transform_SOURCES = $(common_SOURCES) test-position-transform.c

TESTS_ENVIRONMENT = srcdir=$(srcdir)

MATCH_FILES = 7point.match 7point.gmd 7point.mat 7point.sgf \
	maxfill-black-incomplete.gmd maxfill-black-complete.gmd \
	maxfill-white-incomplete.gmd maxfill-white-complete.gmd \
	drop-black-incomplete.gmd drop-black-complete.gmd \
	drop-white-incomplete.gmd drop-white-complete.gmd \
	roll-white-incomplete.gmd roll-white-complete.gmd \
	roll-black-incomplete.gmd roll-black-complete.gmd \
	reject-white-incomplete.gmd reject-white-complete.gmd \
	reject-black-incomplete.gmd reject-black-complete.gmd \
	reject2-white-incomplete.gmd reject2-white-complete.gmd \
	reject2-black-incomplete.gmd reject2-black-complete.gmd \
	dance-incomplete.gmd dance-complete0.gmd \
	dance-complete1.gmd dance-complete2.gmd edited.gmd edited.sgf \
	initial-with-game.gmd initial.gmd initial-with-double.gmd \
	initial-with-one-double.gmd opening-white-complete.gmd \
	opening-black-complete.gmd opening-white-double-complete.gmd \
	opening-black-double-complete.gmd initial-with-two-doubles.gmd \
	win-by-move.gmd win-by-move-complete.gmd \
	win-by-double.gmd win-by-double-complete.gmd \
	win-by-resignation.gmd win-by-resignation-complete.gmd

EXTRA_DIST = $(MATCH_FILES) $(TESTS_SH) gibbon.rc
