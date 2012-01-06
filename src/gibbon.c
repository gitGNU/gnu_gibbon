/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
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

#include "gibbon-app.h"
#include "gibbon-connection.h"
#include "gibbon-archive.h"

static gchar *program_name;

static gchar *data_dir = NULL;
static gchar *pixmaps_dir = NULL;
gboolean version;

static const GOptionEntry options[] =
{
                { "data-dir", 'd', 0, G_OPTION_ARG_FILENAME, &data_dir,
                  N_("Path to data directory (developers only)"),
                  N_("DIRECTORY")
                },
                { "pixmaps-dir", 'p', 0, G_OPTION_ARG_FILENAME, &pixmaps_dir,
                  N_("Path to pixmaps directory (developers only)"),
                  N_("DIRECTORY")
                },
                { "version", 'V', 0, G_OPTION_ARG_NONE, &version,
                  N_("output version information and exit"),
                  N_("FORMAT")
                },
	        { NULL }
};

static void print_version ();
static guint parse_command_line (int argc, char *argv[]);
#ifdef G_OS_WIN32
static void setup_path (const gchar *installdir);
static void init_i18n (const gchar *installdir);
#else
static void init_i18n (void);
#endif

int
main (int argc, char *argv[])
{	
        GibbonApp *app;
        gchar *builder_filename;
        gchar *pixmaps_dir_buf = NULL;
#ifdef G_OS_WIN32
        gchar *win32_dir =
                g_win32_get_package_installation_directory_of_module (NULL);

        init_i18n (win32_dir);                
        setup_path (win32_dir);
#else
        init_i18n ();
#endif

        program_name = argv[0];

        if (!parse_command_line (argc, argv))
                return 1;
        

        if (version) {
                print_version ();
                return 0;
        }

        if (!g_thread_supported ()) {
                g_thread_init (NULL);
		gdk_threads_init ();
	}

        gtk_init (&argc, &argv);

        /* It is unsafe to guess that we are in a development environment
         * just because there is a data/gibbon.ui file.  Rather require
         * an option!
         */
        if (data_dir) {
                builder_filename = g_build_filename (data_dir,
                                                     PACKAGE ".ui",
                                                     NULL);
        } else {
#ifdef G_OS_WIN32
                builder_filename = g_build_filename (win32_dir, "share", 
                                                     PACKAGE,
                                                     PACKAGE ".ui", NULL);
#else
                builder_filename = g_build_filename (GIBBON_DATADIR, PACKAGE,
                                                     PACKAGE ".ui",
                                                     NULL);
#endif
        }

        if (!pixmaps_dir) {
                pixmaps_dir = pixmaps_dir_buf
#ifdef G_OS_WIN32
                        = g_build_filename (win32_dir, "share",
                                            "pixmaps", PACKAGE, NULL);
#else
                        = g_build_filename (GIBBON_DATADIR,
                                            "pixmaps", PACKAGE, NULL);
#endif
        }

        app = gibbon_app_new (builder_filename, pixmaps_dir,
                              data_dir ? data_dir : GIBBON_DATADIR);
        if (!app)
                return -1;

        if (pixmaps_dir_buf)
                g_free (pixmaps_dir_buf);
        g_free (builder_filename);

        gtk_widget_show (gibbon_app_get_window (app));
        gtk_main ();

        g_object_unref (app);
        
        return 0;
}

static void
#ifdef G_OS_WIN32
init_i18n (const gchar *installdir)
#else
init_i18n (void)
#endif
{
        gchar *locale_dir;

        setlocale(LC_ALL, "");

#ifdef G_OS_WIN32
        locale_dir = g_build_filename (installdir, "share", "locale", NULL);
#else
        locale_dir = g_build_filename (GIBBON_DATADIR, "locale", NULL);
#endif
        bindtextdomain (GETTEXT_PACKAGE, locale_dir);
        g_free (locale_dir);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
        textdomain (GETTEXT_PACKAGE);
}
 
static guint 
parse_command_line (int argc, char *argv[])
{
        GOptionContext *context;
        GError *error = NULL;

        context = g_option_context_new (_("- Gtk+ frontend for FIBS"));
        g_option_context_set_summary (context, "Play backgammon online.");
        g_option_context_set_description (context,
           _("Report bugs at https://savannah.nongnu.org/projects/gibbon."));
        g_option_context_add_main_entries (context, options, PACKAGE);
        g_option_context_add_group (context, gtk_get_option_group (TRUE));
        g_option_context_parse (context, &argc, &argv, &error);

        g_option_context_free (context);

        if (error) {
                g_printerr ("%s\n", error->message);
                g_printerr (_("Run `%s --help' for more information!\n"),
                            argv[0]);
                g_error_free (error);
                return 0;
        }
        
        return 1;
}

#ifdef G_OS_WIN32
/*
 * Under MS-DOS shared libraries are searched in $PATH.  We have to make sure
 * that gconfd-2 finds its libraries.  
 */
void
setup_path (const gchar *installdir)
{
        gchar *bin = g_build_filename (installdir, "bin", NULL);
        gchar *path = g_build_path (";", bin, g_getenv ("PATH"), NULL);
        
        g_free (bin);
        
        if (!g_setenv ("PATH", path, TRUE))
                g_printerr (_("Error setting PATH environment variable!\n"));
        
        g_free (path);
}
#endif

static void
print_version ()
{
        g_print ("%s (%s) %s\n", program_name, PACKAGE, VERSION);
        /* xgettext: no-wrap */
        g_print (_("Copyright (C) %s %s.\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
"),
                "2009-2012", _("Guido Flohr"));
        g_print (_("Written by %s.\n"), _("Guido Flohr"));
}
