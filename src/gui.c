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

#include <gtk/gtk.h>
#include <glib/gi18n.h>

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
static GtkBuilder *get_builder (void);

static GtkWidget *window;
static GtkWidget *connection_dialog;

int
main(int argc, char *argv[])
{	
        GtkBuilder *builder = NULL;
        
        init_i18n ();
        
        if (!parse_command_line (argc, argv))
                return 1;
                
        gtk_init (&argc, &argv);
        
        builder = get_builder ();
        if (!builder)
                return 1;
                
        window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
        if (!window) {
                g_print (_("Internal error: Cannot find widget '%s'!\n"),
                         "window");
                return 1;
        }
        
        connection_dialog = 
                GTK_WIDGET (gtk_builder_get_object (builder, 
                                                    "connection_dialog"));
        if (!connection_dialog) {
                g_print (_("Internal error: Cannot find widget '%s'!\n"),
                         "connection_dialog");
                return 1;
        }
        
        gtk_builder_connect_signals (builder, NULL);          
        g_object_unref (G_OBJECT (builder));

        gtk_widget_show (window);       
        gtk_main ();

	return 0;
}

static void
init_i18n (void)
{
        gchar *locale_dir;

        setlocale(LC_ALL, "");

        locale_dir = g_build_filename(DATADIR, "locale", NULL);
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

static GtkBuilder *
get_builder (void)
{
        GtkBuilder *builder = gtk_builder_new ();
        gchar *builder_filename_buf = NULL;
        GError *error = NULL;
        
        /* It is unsafe to guess that we are in a development environment
         * just because there is a data/gibbon.xml file.  Rather require
         * an option!
         */
        if (!builder_filename)
                builder_filename = builder_filename_buf 
                        = g_build_filename(DATADIR, PACKAGE, 
                                           PACKAGE ".xml", NULL);
                                           
        if (!gtk_builder_add_from_file (builder, builder_filename, &error)) {
                g_print ("%s\n", error->message);
                g_error_free (error);
                g_object_unref (G_OBJECT (builder));              
                return NULL;
        }
        
        if (builder_filename_buf)
                g_free(builder_filename_buf);
        
        return builder;
}