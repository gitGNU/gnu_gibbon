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

bin_PROGRAMS = gibbon

AUTOMAKE_OPTIONS = color-tests

platform_libadd =

if MSDOS
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
	-D_SVID_SOURCE

common_SOURCES = 			\
	gibbon-cairoboard.c		\
	gibbon-connection.c		\
	gibbon-game.c           	\
	gibbon-match.c          	\
	gibbon-player-list.c		\
	gibbon-session.c		\
	html-entities.c			\
	svg-util.c			\
	gibbon-game-chat.c		\
	gibbon-archive.c		\
	gibbon-java-fibs-importer.c	\
	gibbon-fibs-message.c		\
	gibbon-app.c			\
	gibbon-player-list-view.c	\
	gibbon-connection-dialog.c	\
	gibbon-signal.c			\
	gibbon-server-console.c		\
	gibbon-fibs-command.c		\
	gibbon-shouts.c			\
	gibbon-position.c		\
	gibbon-board.c			\
	gibbon-chat-view.c		\
	gibbon-chat.c			\
	gibbon-help.c			\
	gibbon-database.c		\
	gibbon-reliability.c		\
	gibbon-reliability-renderer.c	\
	gibbon-util.c			\
	gibbon-clip.c			\
	gibbon-inviter-list.c		\
	gibbon-inviter-list-view.c	\
	gibbon-saved-info.c		\
	gibbon-client-icons.c		\
	gibbon-geo-ip-updater.c		\
	gibbon-country.c		\
	gibbon-settings.c		\
	gibbon-register-dialog.c	\
	gibbon-game-action.c		\
	gibbon-movement.c		\
	gibbon-move.c			\
	gibbon-double.c			\
	gibbon-drop.c			\
	gibbon-roll.c			\
	gibbon-take.c			\
	gibbon-resign.c			\
	gibbon-accept.c			\
	gibbon-reject.c			\
	gibbon-setup.c			\
	gibbon-match-reader.c		\
	gibbon-java-fibs-reader.c	\
	gibbon-match-writer.c		\
	gibbon-java-fibs-writer.c	\
	gibbon-java-fibs-parser.y	\
	gibbon-java-fibs-lexer.c

LEXCOMPILE = $(LEX) $(LFLAGS) $(AM_LFLAGS)
LTLEXCOMPILE = $(LIBTOOL) $(AM_V_lt) $(AM_LIBTOOLFLAGS) \
        $(LIBTOOLFLAGS) --mode=compile $(LEX) $(LFLAGS) $(AM_LFLAGS)
AM_V_LEX = $(am__v_LEX_$(V))
am__v_LEX_ = $(am__v_LEX_$(AM_DEFAULT_VERBOSITY))
am__v_LEX_0 = @echo "  LEX   " $@;

gibbon-java-fibs-lexer.c: gibbon-java-fibs-lexer.l
	$(AM_V_LEX)$(am__skiplex) $(SHELL) $(YLWRAP) $< lex.gibbon_java_fibs_lexer_.c $@ -- $(LEXCOMPILE)

gibbon_SOURCES = gibbon.c $(common_SOURCES)

noinst_HEADERS =			\
	gibbon-cairoboard.h		\
	gibbon-connection.h		\
	gibbon-game.h           	\
	gibbon-match.h          	\
	gibbon-player-list.h		\
	gibbon-session.h		\
	html-entities.h			\
	svg-util.h			\
	gibbon-game-chat.h		\
	gibbon-archive.h		\
	gibbon-java-fibs-importer.h	\
	gibbon-fibs-message.h		\
	gibbon-app.h			\
	gibbon-player-list-view.h	\
	gibbon-connection-dialog.h	\
	gibbon-signal.h			\
	gibbon-server-console.h		\
	gibbon-fibs-command.h		\
	gibbon-shouts.h			\
	gibbon-position.h		\
	gibbon-board.h			\
	gibbon-chat-view.h		\
	gibbon-chat.h			\
	gibbon-help.h			\
	gibbon-database.h		\
	gibbon-reliability.h		\
	gibbon-reliability-renderer.h	\
	gibbon-util.h			\
	gibbon-clip.h			\
	gibbon-inviter-list.h		\
	gibbon-inviter-list-view.h	\
	gibbon-saved-info.h		\
	gibbon-client-icons.h		\
	gibbon-geo-ip-updater.h		\
	gibbon-country.h		\
	gibbon-settings.h		\
	gibbon-register-dialog.h	\
	gibbon-game-action.h		\
	gibbon-game-actions.h		\
	gibbon-movement.h		\
	gibbon-move.h			\
	gibbon-double.h			\
	gibbon-drop.h			\
	gibbon-roll.h			\
	gibbon-take.h			\
	gibbon-resign.h			\
	gibbon-accept.h			\
	gibbon-reject.h			\
	gibbon-setup.h			\
	gibbon-match-reader.h		\
	gibbon-java-fibs-reader.h	\
	gibbon-match-writer.h		\
	gibbon-java-fibs-writer.h


BUILT_SOURCES = gibbon-java-fibs-parser.c gibbon-java-fibs-parser.h \
	gibbon-java-fibs-lexer.c

AM_YFLAGS =-d

AM_LDFLAGS = -export-dynamic
LDADD = ../libsvg/libsvg.a ../libsvg-cairo/libsvg-cairo.a ../libgsgf/libgsgf.a \
	$(GIBBON_LIBS) $(platform_libadd)
if MSDOS
gibbon_LDFLAGS = -mwindows
endif

## Put test_gary_wong_movegen at the end because it takes the most time.
TESTS_C = test_html_entities test_build_match test_position test_moves \
	test_pretty_print_move test_strsplit test_clip \
	test_gary_wong_movegen test_crawford_detection \
	test_java_fibs_reader

TESTS_SH =

TESTS = $(TESTS_C) $(TESTS_SH)

check_PROGRAMS = test_html_entities test_build_match test_position test_moves \
	test_clip test_pretty_print_move test_strsplit	\
	test_gary_wong_movegen test_crawford_detection \
	test_java_fibs_reader

test_html_entities_SOURCES = $(common_SOURCES) test-html-entities.c
test_build_match_SOURCES = $(common_SOURCES) test-build-match.c
test_position_SOURCES = $(common_SOURCES) test-position.c
test_moves_SOURCES = $(common_SOURCES) test-moves.c
test_gary_wong_movegen_SOURCES = $(common_SOURCES) test-gary-wong-movegen.c
test_pretty_print_move_SOURCES = $(common_SOURCES) test-pretty-print-move.c
test_strsplit_SOURCES = $(common_SOURCES) test-strsplit.c
test_clip_SOURCES = $(common_SOURCES) test-clip.c
test_crawford_detection_SOURCES = $(common_SOURCES) test-crawford-detection.c
test_java_fibs_reader_SOURCES = $(common_SOURCES) test-java-fibs-reader.c

MATCH_FILES = complete.match

EXTRA_DIST = $(MATCH_FILES) $(TESTS_SH) gibbon.rc
