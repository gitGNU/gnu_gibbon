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

# gnome-doc-utils.make is not portable. :-(
AUTOMAKE_OPTIONS = -Wno-portability

include $(top_srcdir)/gnome-doc-utils.make

DOC_ID = gibbon
DOC_FIGURES = \
	board-toolbar.png \
	connect-register.png \
	game-chat.png \
	gibbon-logo.png \
	say.png \
	whisper.png

DOC_PAGES = \
	board-toolbar.page \
	bugreports.page \
	cannot-invite.page \
	checker-movement.page \
	connect.page \
	create-account.page \
	do-not.page \
	doubling.page \
	droppers.page \
	game-chat-concept.page \
	index.page \
	introduction.page \
	invite.page \
	kibitz.page \
	resigning.page \
	rolling-dice.page \
	say.page \
	swapping-dice.page \
	watching-matches.page \
	whisper.page

DOC_LINGUAS = de
dist-hook: doc-dist-hook
