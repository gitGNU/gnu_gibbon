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

#include <sys/types.h>

#include <glib/gi18n.h>

#include "gui.h"
#include "gibbon.h"

GtkBuilder *builder = NULL;

GtkWidget *window = NULL;
GtkWidget *connection_dialog = NULL;
GConfClient *conf_client = NULL;

static GtkWidget *statusbar = NULL;

static GtkBuilder *get_builder (const gchar* filename);
static void set_state_connecting ();
static void set_state_disconnected ();
static void cb_resolving (GtkWidget *emitter, const gchar *hostname);
static void cb_disconnected (GtkWidget *emitter, const gchar *hostname);

gint
init_gui (const gchar *builder_filename)
{
        const gchar *default_server;
        gint default_port;
        gchar *default_port_str;
        const gchar *default_login;
        const gchar *default_password;
        gboolean default_save_password;
        GObject *entry;
        GObject *check;
        
        builder = get_builder (builder_filename);
        
        if (!builder)
                return 0;
                
        window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
        if (!window) {
                g_print (_("Internal error: Cannot find widget '%s'!\n"),
                         "window");
                return 0;
        }
        
        connection_dialog = 
                GTK_WIDGET (gtk_builder_get_object (builder, 
                                                    "connection_dialog"));
        if (!connection_dialog) {
                g_print (_("Internal error: Cannot find widget '%s'!\n"),
                         "connection_dialog");
                return 0;
        }
        
        statusbar = 
                GTK_WIDGET (gtk_builder_get_object (builder, 
                                                    "statusbar"));
        if (!statusbar) {
                g_print (_("Internal error: Cannot find widget '%s'!\n"),
                         "statusbar");
                return 0;
        }
        gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, _("Disconnected"));
        
        gtk_builder_connect_signals (builder, NULL);

        g_signal_connect (G_OBJECT (connection), "resolving",
                          G_CALLBACK (cb_resolving), NULL);
        g_signal_connect (G_OBJECT (connection), "disconnected",
                          G_CALLBACK (cb_disconnected), NULL);

        set_state_disconnected ();
        
        /* FIXME! All this stuff has to go into a new class
         * GibbonPreferences.
         */
        conf_client = gconf_client_get_default ();
        
        default_server = 
                gconf_client_get_string (conf_client,
                                         GIBBON_GCONF_PREFS_PREFIX "server",
                                         NULL);
        default_port = 
                gconf_client_get_int (conf_client,
                                      GIBBON_GCONF_PREFS_PREFIX "port",
                                      NULL);
        default_login = 
                gconf_client_get_string (conf_client,
                                         GIBBON_GCONF_PREFS_PREFIX "login",
                                         NULL);
        default_save_password = 
                gconf_client_get_bool (conf_client,
                                       GIBBON_GCONF_PREFS_PREFIX "save_pwd",
                                       NULL);
        default_password = default_save_password ?
                gconf_client_get_string (conf_client,
                                         GIBBON_GCONF_PREFS_PREFIX "password",
                                         NULL)
                : NULL;
        
        if (default_server) {
                entry = gtk_builder_get_object (builder, "conn_entry_server");
                gtk_entry_set_text (GTK_ENTRY (entry), default_server);
        }
        
        if (default_port) {
                entry = gtk_builder_get_object (builder, "conn_entry_port");
                default_port_str = g_strdup_printf ("%d", default_port);
                gtk_entry_set_text (GTK_ENTRY (entry), default_port_str); 
                g_free (default_port_str);
        }
        
        if (default_login) {
                entry = gtk_builder_get_object (builder, "conn_entry_login");
                gtk_entry_set_text (GTK_ENTRY (entry), default_login);
        }
        
        check = gtk_builder_get_object (builder, "conn_checkbutton_remember");
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), 
                                      default_save_password);
        
        entry = gtk_builder_get_object (builder, "conn_entry_password");
        if (default_save_password && default_password) {
                gtk_entry_set_text (GTK_ENTRY (entry), default_password);
        } else {
                gtk_entry_set_text (GTK_ENTRY (entry), "");
        }
        
	return 1;
}

const gchar *
get_entry_text (const gchar *id) 
{
        GtkWidget *entry = GTK_WIDGET (gtk_builder_get_object (builder, id));
        
        if (!entry) {
                g_print (_("Internal error: Cannot find widget '%s'!\n"),
                         id);
                return "";
        }
        
        return gtk_entry_get_text (GTK_ENTRY (entry));
}

const gchar *
get_trimmed_entry_text (const gchar *id) 
{
        GtkWidget *entry = GTK_WIDGET (gtk_builder_get_object (builder, id));
        gchar *trimmed;
        
        if (!entry) {
                g_print (_("Internal error: Cannot find widget `%s'!\n"),
                         id);
                return "";
        }
        
        trimmed = pango_trim_string (gtk_entry_get_text (GTK_ENTRY (entry)));
        gtk_entry_set_text (GTK_ENTRY (entry), trimmed);
        g_free (trimmed);
               
        return gtk_entry_get_text (GTK_ENTRY (entry));
}

void
display_error (const gchar *message_format, ...)
{
        va_list args;
        gchar *message;
        
        va_start (args, message_format);
        message = g_strdup_vprintf (message_format, args);        
        va_end (args);
        
        GtkWidget *dialog = 
                gtk_message_dialog_new (GTK_WINDOW (window),
                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                        GTK_MESSAGE_ERROR,
                                        GTK_BUTTONS_CLOSE,
                                        message);
        
        g_free (message);
        
        gtk_dialog_run (GTK_DIALOG (dialog));
        
        gtk_widget_destroy (GTK_WIDGET (dialog));
}

static GtkBuilder *
get_builder (const gchar *builder_filename)
{
        GtkBuilder *builder = gtk_builder_new ();
        GError *error = NULL;
        
        if (!gtk_builder_add_from_file (builder, builder_filename, &error)) {
                g_print ("%s\n", error->message);
                g_print (_("Do you need to pass the option `--ui-file'?\n"));
                g_error_free (error);
                g_object_unref (G_OBJECT (builder));              
                return NULL;
        }
        
        return builder;
}

static void
set_state_connecting ()
{
        GObject *tool_button = 
                gtk_builder_get_object (builder, "toolbar_connect_button");
        GObject *connect_item = 
                gtk_builder_get_object (builder, "connect_menu_item");
        GObject *disconnect_item = 
                gtk_builder_get_object (builder, "disconnect_menu_item");
        
        if (!tool_button) {
                g_print (_("Internal error: Cannot find widget `%s'!\n"),
                         "toolbar_connect_button");
        } else {
                gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON (tool_button),
                                              GTK_STOCK_DISCONNECT);
                gtk_tool_button_set_label (GTK_TOOL_BUTTON (tool_button),
                                           _("Disconnect"));
        }
        
        if (!connect_item) {
                g_print (_("Internal error: Cannot find widget `%s'!\n"),
                         "connect_menu_item");
        } else {
                gtk_action_set_sensitive (GTK_ACTION (connect_item), FALSE);
        }
        
        if (!disconnect_item) {
                g_print (_("Internal error: Cannot find widget `%s'!\n"),
                         "disconnect_menu_item");
        } else {
                gtk_action_set_sensitive (GTK_ACTION (disconnect_item), TRUE);
        }
}

static void
set_state_disconnected ()
{
        GObject *tool_button = 
                gtk_builder_get_object (builder, "toolbar_connect_button");
        GObject *connect_item = 
                gtk_builder_get_object (builder, "connect_menu_item");
        GObject *disconnect_item = 
                gtk_builder_get_object (builder, "disconnect_menu_item");
        
        if (!tool_button) {
                g_print (_("Internal error: Cannot find widget `%s'!\n"),
                         "toolbar_connect_button");
        } else {
                gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON (tool_button),
                                              GTK_STOCK_NETWORK);
                gtk_tool_button_set_label (GTK_TOOL_BUTTON (tool_button),
                                           _("Connect"));
        }
        
        if (!connect_item) {
                g_print (_("Internal error: Cannot find widget `%s'!\n"),
                         "connect_menu_item");
        } else {
                gtk_action_set_sensitive (GTK_ACTION (connect_item), TRUE);
        }
        
        if (!disconnect_item) {
                g_print (_("Internal error: Cannot find widget `%s'!\n"),
                         "disconnect_menu_item");
        } else {
                gtk_action_set_sensitive (GTK_ACTION (disconnect_item), FALSE);
        }
}

static void
cb_resolving (GtkWidget *emitter, const gchar *hostname)
{
/*        GibbonConnection *connection = GIBBON_CONNECTION (emitter);
        gchar *msg = g_strdup_printf (_("Connecting as %s to port %u on %s."),
                                      gibbon_connection_get_login (connection),
                                      gibbon_connection_get_port (connection),
                                      gibbon_connection_get_host (connection)); */
        gchar *msg = g_strdup_printf (_("Resolving address for %s."), hostname);
        
        gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 0);
        gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, msg);
        g_free (msg);
        
        set_state_connecting ();
}

static void
cb_disconnected (GtkWidget *emitter, const gchar *hostname)
{
        gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 0);
        gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, _("Disconnected"));
        
        set_state_disconnected ();
}
