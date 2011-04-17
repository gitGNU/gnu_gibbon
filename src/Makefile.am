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

bin_PROGRAMS = gibbon

AUTOMAKE_OPTIONS = color-tests

platform_libadd =

if MSDOS
platform_libadd += -ldnsapi -lws2_32
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
	-D_SVID_SOURCE

common_SOURCES = 		\
	gibbon-cairoboard.c	\
	gibbon-connection.c	\
	gibbon-connector.c	\
	gibbon-game.c           \
	gibbon-match.c          \
	gibbon-player-list.c	\
	gibbon-session.c	\
	html-entities.c		\
	svg-util.c			\
	gibbon-game-chat.c				\
	gibbon-prefs.c				\
	gibbon-archive.c				\
	gibbon-java-fibs-importer.c				\
	gibbon-action.c				\
	gibbon-fibs-message.c				\
	gibbon-app.c				\
	gibbon-player-list-view.c				\
	gibbon-connection-dialog.c				\
	gibbon-signal.c				\
	gibbon-server-console.c							\
	gibbon-fibs-command.c				\
	gibbon-shouts.c				\
	gibbon-position.c				\
	gibbon-board.c			\
	gibbon-account-dialog.c			\
	gibbon-chat-view.c				\
	gibbon-chat.c			\
	gibbon-help.c

gibbon_SOURCES = gibbon.c $(common_SOURCES)

noinst_HEADERS =		\
	gibbon-cairoboard.h	\
	gibbon-connection.h	\
	gibbon-connector.h	\
	gibbon-game.h           \
	gibbon-match.h          \
	gibbon-player-list.h	\
	gibbon-session.h	\
	html-entities.h		\
	svg-util.h			\
	gibbon-game-chat.h			\
	gibbon-prefs.h			\
	gibbon-archive.h			\
	gibbon-java-fibs-importer.h			\
	gibbon-action.h			\
	gibbon-fibs-message.h			\
	gibbon-app.h			\
	gibbon-player-list-view.h			\
	gibbon-connection-dialog.h			\
	gibbon-signal.h			\
	gibbon-server-console.h				\
	gibbon-fibs-command.h			\
	gibbon-shouts.h			\
	gibbon-position.h			\
	gibbon-board.h		\
	gibbon-account-dialog.h			\
	gibbon-chat-view.h			\
	gibbon-chat.h			\
	gibbon-help.h

AM_LDFLAGS = -export-dynamic
LDADD = $(GIBBON_LIBS) ../libsvg/libsvg.a \
	../libsvg-cairo/libsvg-cairo.a ../libgsgf/libgsgf.a $(platform_libadd)

TESTS_C = test_html_entities test_basic_match test_position test_moves \
	test_gary_wong_movegen
TESTS_SH =

TESTS = $(TESTS_C) $(TESTS_SH)

check_PROGRAMS = test_html_entities test_basic_match test_position test_moves \
	test_gary_wong_movegen

test_html_entities_SOURCES = $(common_SOURCES) test-html-entities.c
test_basic_match_SOURCES = $(common_SOURCES) test-basic-match.c
test_position_SOURCES = $(common_SOURCES) test-position.c
test_moves_SOURCES = $(common_SOURCES) test-moves.c
test_gary_wong_movegen_SOURCES = $(common_SOURCES) test-gary-wong-movegen.c

SGF_FILES =

EXTRA_DIST = $(SGF_FILES) $(TESTS_SH)
