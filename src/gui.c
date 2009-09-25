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
GtkWidget *server_text_view = NULL;
GConfClient *conf_client = NULL;

static GtkWidget *statusbar = NULL;

static GtkBuilder *get_builder (const gchar* filename);
static void cb_resolving (GtkWidget *emitter, const gchar *hostname);
static void cb_connecting (GtkWidget *emitter, const gchar *hostname);
static void cb_disconnected (GtkWidget *emitter, const gchar *hostname);
static void cb_raw_server_output (GtkWidget *emitter, const gchar *text);
static void cb_login (GtkWidget *emitter, const gchar *hostname);
static void cb_logged_in (GtkWidget *emitter, const gchar *hostname);

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
        PangoFontDescription *font_desc;
        
        builder = get_builder (builder_filename);
        
        if (!builder)
                return 0;
                
        window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
        connection_dialog = 
                GTK_WIDGET (gtk_builder_get_object (builder, 
                                                    "connection_dialog"));
        server_text_view = 
                GTK_WIDGET (gtk_builder_get_object (builder,
                                                    "server_text_view"));
        statusbar = 
                GTK_WIDGET (gtk_builder_get_object (builder, 
                                                    "statusbar"));
        gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, _("Disconnected"));
        
        gtk_builder_connect_signals (builder, NULL);

        font_desc = pango_font_description_from_string ("monospace 10");
        gtk_widget_modify_font (server_text_view, font_desc);
        pango_font_description_free (font_desc);
        
        g_signal_connect (G_OBJECT (connection), "resolving",
                          G_CALLBACK (cb_resolving), NULL);
        g_signal_connect (G_OBJECT (connection), "connecting",
                          G_CALLBACK (cb_connecting), NULL);
        g_signal_connect (G_OBJECT (connection), "login",
                          G_CALLBACK (cb_login), NULL);
        g_signal_connect (G_OBJECT (connection), "logged-in",
                          G_CALLBACK (cb_logged_in), NULL);
        g_signal_connect (G_OBJECT (connection), "disconnected",
                          G_CALLBACK (cb_disconnected), NULL);
        g_signal_connect (G_OBJECT (connection), "raw-server-output",
                          G_CALLBACK (cb_raw_server_output), NULL);
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

void
set_state_connecting ()
{
        GObject *connect_button = 
                gtk_builder_get_object (builder, "toolbar_connect_button");
        GObject *disconnect_button = 
                gtk_builder_get_object (builder, "toolbar_disconnect_button");
        GObject *connect_item = 
                gtk_builder_get_object (builder, "connect_menu_item");
        GObject *disconnect_item = 
                gtk_builder_get_object (builder, "disconnect_menu_item");
        
        gtk_widget_set_sensitive (GTK_WIDGET (connect_button), FALSE);
        gtk_action_set_sensitive (GTK_ACTION (connect_item), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (disconnect_button), TRUE);
        gtk_action_set_sensitive (GTK_ACTION (disconnect_item), TRUE);
}

void
set_state_disconnected ()
{
        GObject *connect_button = 
                gtk_builder_get_object (builder, "toolbar_connect_button");
        GObject *disconnect_button = 
                gtk_builder_get_object (builder, "toolbar_disconnect_button");
        GObject *connect_item = 
                gtk_builder_get_object (builder, "connect_menu_item");
        GObject *disconnect_item = 
                gtk_builder_get_object (builder, "disconnect_menu_item");
        
        gtk_widget_set_sensitive (GTK_WIDGET (connect_button), TRUE);
        gtk_action_set_sensitive (GTK_ACTION (connect_item), TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET (disconnect_button), FALSE);
        gtk_action_set_sensitive (GTK_ACTION (disconnect_item), FALSE);
}

static void
cb_resolving (GtkWidget *emitter, const gchar *hostname)
{
        gchar *msg = g_strdup_printf (_("Resolving address for %s."), hostname);
        
        gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 0);
        gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, msg);
        g_free (msg);
}

static void
cb_connecting (GtkWidget *emitter, const gchar *hostname)
{
        GibbonConnection *connection = GIBBON_CONNECTION (emitter);
        gchar *msg = g_strdup_printf (_("Connecting as %s to port %u on %s."),
                                      gibbon_connection_get_login (connection),
                                      gibbon_connection_get_port (connection),
                                      gibbon_connection_get_hostname (connection));
        
        gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 0);
        gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, msg);
        g_free (msg);
}

static void
cb_login (GtkWidget *emitter, const gchar *hostname)
{
        GibbonConnection *connection = GIBBON_CONNECTION (emitter);
        gchar *msg = g_strdup_printf (_("Log in as %s on %s, port %u."),
                                      gibbon_connection_get_login (connection),
                                      gibbon_connection_get_hostname (connection),
                                      gibbon_connection_get_port (connection));
        
        gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 0);
        gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, msg);
        g_free (msg);
}

static void
cb_logged_in (GtkWidget *emitter, const gchar *hostname)
{
        GibbonConnection *connection = GIBBON_CONNECTION (emitter);
        gchar *msg = g_strdup_printf (_("Logged in as %s on %s, port %u."),
                                      gibbon_connection_get_login (connection),
                                      gibbon_connection_get_hostname (connection),
                                      gibbon_connection_get_port (connection));
        
        gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 0);
        gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, msg);
        g_free (msg);
}

static void
cb_disconnected (GtkWidget *emitter, const gchar *error)
{
        gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 0);
        gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, _("Disconnected"));
        
        set_state_disconnected ();
}

static void
cb_raw_server_output (GtkWidget *emitter, const gchar *text)
{
        GtkTextBuffer *buffer = 
                gtk_text_view_get_buffer (GTK_TEXT_VIEW (server_text_view));
        GtkTextIter iter;
        
        gtk_text_buffer_insert_at_cursor (buffer, text, -1);
        gtk_text_buffer_insert_at_cursor (buffer, "\n", -1);
        
        gtk_text_buffer_get_end_iter (buffer, &iter);
       
        gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (server_text_view),
                                      &iter, 0, FALSE, 0, 0); 
}