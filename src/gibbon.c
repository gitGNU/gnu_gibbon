/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009 Guido Flohr, http://guido-flohr.net/.
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
GibbonDesign *design = NULL;

static gchar *builder_filename = NULL;

static const GOptionEntry options[] =
{
	{ "ui-file", 'u', 0, G_OPTION_ARG_FILENAME, &builder_filename,
	  N_("Use alternatve UI definition (developers only)"),
	  N_("FILENAME")}, 
	{ NULL }
};

static void init_i18n (void);
static guint parse_command_line (int argc, char *argv[]);

int
main(int argc, char *argv[])
{	
        gchar *builder_filename_buf = NULL;
        
        init_i18n ();
        
        if (!parse_command_line (argc, argv))
                return 1;
        
        if (!g_thread_supported ()) {
                g_thread_init (NULL);
		gdk_threads_init ();
	}
        connection = gibbon_connection_new ();
        design = gibbon_design_new ();
                
        gtk_init (&argc, &argv);
        
        /* It is unsafe to guess that we are in a development environment
         * just because there is a data/gibbon.xml file.  Rather require
         * an option!
         */
        if (!builder_filename)
                builder_filename = builder_filename_buf 
                        = g_build_filename(GIBBON_DATADIR, PACKAGE, 
                                           PACKAGE ".xml", NULL);
                                           
        if (!init_gui (builder_filename))               
                return 1;

        if (builder_filename_buf)
                g_free(builder_filename_buf);
        
        gtk_widget_show (window);       
        gtk_main ();

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
        
        if (error) {
                g_print ("%s\n%s\n",
                         error->message,
                         _("Run `%s --help' for more information!"));
                g_error_free (error);
                return 0;
        }
        
        return 1;
}
