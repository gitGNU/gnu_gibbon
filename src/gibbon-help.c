/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
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

#ifdef G_OS_WIN32
# include <windows.h>
#endif

static const gchar *const authors[] = {
                "Guido Flohr <guido@imperia.net>",
                NULL
};

static const gchar copyright[] = "Copyright \xc2\xa9 2009-2012 Guido Flohr\n";

static gchar *descr =  N_("%s is a program for playing backgammon online.");

void
gibbon_help_show_about (GObject *emitter, const GibbonApp *app)
{
        GtkWindow *window;
        gchar *comments;

        window = GTK_WINDOW (gibbon_app_get_window (app));

        gchar *logo_path = g_build_filename (GIBBON_DATADIR,
                                             "icons", "hicolor",
                                             "128x128", "apps",
                                             PACKAGE ".png", NULL);
        GdkPixbuf *logo = gdk_pixbuf_new_from_file (logo_path, NULL);

        comments = g_strdup_printf (_(descr), PACKAGE);

        gtk_show_about_dialog (window,
                               "program-name", PACKAGE,
                               "authors", authors,
                               "comments", comments,
                               "copyright", copyright,
                               "translator-credits", _("translator-credits"),
                               "version", VERSION,
                               "website", "http://www.gibbon.bg/",
                               "logo", logo,
                               NULL);

        if (logo)
                g_object_unref (logo);
        g_free (logo_path);
        g_free (comments);
}

void
gibbon_help_show_help (GObject *emitter, const GibbonApp *app)
{
        gchar *uri;
        GError *error_local = NULL;
        GError *error_online = NULL;
        gboolean success;
        GtkWidget *window;

        g_return_if_fail (G_IS_OBJECT (emitter));
        g_return_if_fail (GIBBON_IS_APP (app));

        window = gibbon_app_get_window (app);
        uri = g_strdup_printf ("help:%s/index", PACKAGE);

        success = gtk_show_uri (gtk_widget_get_screen (window), uri,
                                GDK_CURRENT_TIME, &error_local);

        g_free (uri);

        if (!error_local && success)
                return;

        /*
         * TRANSLATORS: This is the URI of the online help.  Please "translate"
         * it with the appropriate URI for your language.
         */
        uri = _("http://www.gibbon.bg/gibbon/docs/gibbon/en/index.html.en");
#ifdef G_OS_WIN32
        /*
         * gtk_show_uri() fails to open web pages under Windows.  This should
         * be fixed in glib.  If gibbon requires a glib version that fixes
         * the bug of gtk_show_uri(), the gibbon bug #35457 (see
         * http://savannah.nongnu.org/bugs/?35457) should be closed. 
         */
	if (!(ShellExecute (NULL, "open", uri, NULL, NULL, 
                            SW_SHOWNORMAL) > (HINSTANCE) 32)) {
                error_online = g_error_new_literal (0, 0,
                                                    _("Gibbon web page cannot"
                                                      " be opened"));
                success = FALSE;
        } else {
                success = TRUE;
        }
#else
        success = gtk_show_uri (gtk_widget_get_screen (window), uri,
                                GDK_CURRENT_TIME, &error_online);
#endif

        if (!error_online && success)
                return;


        gibbon_app_display_error (app, _("Error Displaying Help"),
                                  _("Locally installed help: %s.\n"
                                    "Online help: %s.\n"
                                    "Please read the documentation under"
                                    " '%s' instead!"),
                                  error_local ? error_local->message
                                                  : _("Unknown error"),
                                  error_online ? error_online->message
                                                  : _("Unknown error"),
                                  uri);

        if (error_local)
                g_error_free (error_local);
        if (error_online)
                g_error_free (error_online);
}
