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

desktopdir = $(datadir)/applications
desktop_in_files = $(PACKAGE).desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

gsettings_files = \
	bg.gibbon.gschema.xml.in \
	$(NULL)
gsettings_SCHEMAS = $(gsettings_files:.xml.in=.xml)
@INTLTOOL_XML_NOMERGE_RULE@
@GSETTINGS_RULES@

man_MANS = gibbon.1 gibbon-convert.1
man_in_files = gibbon.1.in gibbon-convert.1.in

gtkbuilderdir = $(datadir)/$(PACKAGE)
gtkbuilder_DATA = $(PACKAGE).ui

EXTRA_DIST = \
	$(desktop_in_files) \
	$(schemas_in_files) \
	$(gtkbuilder_DATA) \
	$(gsettings_files) \
	$(man_in_files)

CLEANFILES = \
	$(desktop_DATA) \
	$(gsettings_SCHEMAS)

