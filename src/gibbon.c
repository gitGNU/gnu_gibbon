/* This file is part of Gibbon
 * Copyright (C) 2009 Guido Flohr
 * 
 * Gibbon is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * Gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Gibbon; if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <locale.h>

#include <glib/gi18n.h>

int
main(int argc, char *argv[])
{
	gchar *locale_dir;

	setlocale(LC_ALL, "");

	locale_dir = g_build_filename(DATADIR, "locale", NULL);
	bindtextdomain(GETTEXT_PACKAGE, locale_dir);
	g_free(locale_dir);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	return 0;
}
