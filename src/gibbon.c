/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
 *
 * Gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <locale.h>

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "gui.h"
#include "gibbon.h"

GibbonConnection *connection = NULL;

static gchar *data_dir = NULL;
static gchar *pixmaps_dir = NULL;

static const GOptionEntry options[] =
{
                { "data-dir", 'u', 0, G_OPTION_ARG_FILENAME, &data_dir,
                  N_("Path to data directory (developers only)"),
                  N_("DIRECTORY")
                },
                { "pixmaps-dir", 'b', 0, G_OPTION_ARG_FILENAME, &pixmaps_dir,
                  N_("Path to pixmaps directory"),
                  N_("DIRECTORY")
                },
	        { NULL }
};

static void init_i18n (void);
static guint parse_command_line (int argc, char *argv[]);

int
main(int argc, char *argv[])
{	
        gchar *builder_filename;
        gchar *pixmaps_dir_buf = NULL;

        init_i18n ();
        
        if (!parse_command_line (argc, argv))
                return 1;
        
        if (!g_thread_supported ()) {
                g_thread_init (NULL);
		gdk_threads_init ();
	}
        connection = gibbon_connection_new ();
                
        gtk_init (&argc, &argv);
        
        /* It is unsafe to guess that we are in a development environment
         * just because there is a data/gibbon.xml file.  Rather require
         * an option!
         */
        if (data_dir) {
                builder_filename = g_build_filename (data_dir,
                                                     PACKAGE ".xml",
                                                     NULL);
        } else {
                builder_filename = g_build_filename (GIBBON_DATADIR, PACKAGE,
                                                     PACKAGE ".xml",
                                                     NULL);
        }

        if (!pixmaps_dir) {
                pixmaps_dir = pixmaps_dir_buf
                        = g_build_filename (GIBBON_DATADIR,
                                            "pixmaps", PACKAGE, NULL);
        }

        if (!init_gui (builder_filename, pixmaps_dir, "default.svg"))
                return -1;

        if (pixmaps_dir_buf)
                g_free (pixmaps_dir_buf);
        g_free (builder_filename);
        
        gtk_widget_show (window);       
        gtk_main ();

        if (connection)
                g_object_unref (connection);

        cleanup_gui ();

        return 0;
}

static void
init_i18n (void)
{
        gchar *locale_dir;

        setlocale(LC_ALL, "");

        locale_dir = g_build_filename(GIBBON_DATADIR, "locale", NULL);
        bindtextdomain(GETTEXT_PACKAGE, locale_dir);
        g_free(locale_dir);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
        textdomain(GETTEXT_PACKAGE);
}
 
static guint 
parse_command_line (int argc, char *argv[])
{
        GOptionContext *context;
        GError *error = NULL;

        context = g_option_context_new (_("- Gtk+ frontend for FIBS"));
        g_option_context_add_main_entries (context, options, PACKAGE);
        g_option_context_add_group (context, gtk_get_option_group (TRUE));
        g_option_context_parse (context, &argc, &argv, &error);

        g_option_context_free (context);

        if (error) {
                g_print ("%s\n%s\n",
                         error->message,
                         _("Run `%s --help' for more information!"));
                g_error_free (error);
                return 0;
        }
        
        return 1;
}
