/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
 *
 * gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>

#include "gibbon-help.h"

void
gibbon_help_show_about (GObject *emitter, GtkWindow *window)
{
        static const gchar * const authors[] = {
                "Guido Flohr <guido@imperia.bg>",
                NULL
        };
        static const gchar * const documenters[] = {
                "Guido Flohr <guido@imperia.bg>",
                NULL
        };
        static const gchar copyright[] = \
                "Copyright \xc2\xa9 2009-2011 Guido Flohr\n";

        gchar *comments = g_strdup_printf (
                _("%s is a program for playing backgammon online."),
                  PACKAGE);

        gtk_show_about_dialog (window,
                               "program-name", PACKAGE,
                               "authors", authors,
                               "comments", comments,
                               "copyright", copyright,
                               "documenters", documenters,
                               "translator-credits", _("translator-credits"),
                               "version", VERSION,
                               "website", "http://gibbon.guido-flohr.net/",
                               NULL);

        g_free (comments);
}
