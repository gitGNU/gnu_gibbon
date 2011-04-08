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
gibbon_help_show_about (GObject *emitter, const GibbonApp *app)
{
        GtkWindow *window = GTK_WINDOW (gibbon_app_get_window (app));
        static const gchar * const authors[] = {
                "Guido Flohr <guido@imperia.bg>",
                NULL
        };
        static const gchar copyright[] = \
                "Copyright \xc2\xa9 2009-2011 Guido Flohr\n";

        gchar *comments = g_strdup_printf (
                _("%s is a program for playing backgammon online."),
                  PACKAGE);

        gchar *logo_path = g_build_filename (GIBBON_DATADIR,
                                             "icons", "hicolor",
                                             "128x128", "apps",
                                             PACKAGE ".png", NULL);
        GdkPixbuf *logo = gdk_pixbuf_new_from_file (logo_path, NULL);

        gtk_show_about_dialog (window,
                               "program-name", PACKAGE,
                               "authors", authors,
                               "comments", comments,
                               "copyright", copyright,
                               "translator-credits", _("translator-credits"),
                               "version", VERSION,
                               "website", "http://gibbon.guido-flohr.net/",
                               "logo", logo,
                               NULL);

        if (logo)
                g_object_unref (logo);
        g_free (logo_path);
        g_free (comments);
}
