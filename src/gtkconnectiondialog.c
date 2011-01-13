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

#include <stdlib.h>
#include <errno.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#ifdef G_OS_UNIX
# include <gdk/gdkx.h>
#endif

#include "gibbon.h"
#include "gibbon-connection.h"
#include "gui.h"

G_MODULE_EXPORT void
on_conn_button_connect_clicked (GtkObject *object, gpointer user_data)
{
        const gchar *server = get_trimmed_entry_text ("conn_entry_server");
        const gchar *port = get_trimmed_entry_text ("conn_entry_port");
        const gchar *login = get_trimmed_entry_text ("conn_entry_login");
        const gchar *password = get_entry_text ("conn_entry_password");
        const gchar *address = get_entry_text ("conn_entry_address");
        GObject *check_button;
        unsigned long portno = 4321;
        char *endptr;
        
        if (port[0] != '\000') {
                errno = 0;
                portno = strtoul (port, &endptr, 10);
                if (errno) {
                        display_error (_("Invalid port `%s': %s."),
                                         port, g_strerror (errno));
                        gtk_dialog_run (GTK_DIALOG (connection_dialog));
                                           
                        return;
                }
                
                if (*endptr != '\000') {
                        display_error (_("Invalid port number '%s'."), port);
                        gtk_dialog_run (GTK_DIALOG (connection_dialog));
                        return;
                }
        }
        
        if (login[0] == '\000') {
                display_error (_("You have to specify your user "
                                 "name (login)."));
                gtk_dialog_run (GTK_DIALOG (connection_dialog));
                return;
        }
        
        if (0 == g_strcmp0 ("guest", login)) {
                display_error (_("Guest login is not supported."));
                gtk_dialog_run (GTK_DIALOG (connection_dialog));
                return;
        }
        
        if (password[0] == '\000') {
                display_error (_("You have to specify a password."));
                gtk_dialog_run (GTK_DIALOG (connection_dialog));
                return;
        }
        
        gtk_widget_hide (GTK_WIDGET (connection_dialog));
        
        gibbon_connection_set_hostname (connection, server);
        gibbon_connection_set_port (connection, portno);
        gibbon_connection_set_login (connection, login);
        gibbon_connection_set_password (connection, password);
        
        gconf_client_set_string (conf_client, 
                                 GIBBON_GCONF_SERVER_PREFS_PREFIX "host",
                                 server,
                                 NULL);
        gconf_client_set_int (conf_client,
                              GIBBON_GCONF_SERVER_PREFS_PREFIX "port",
                              portno,
                              NULL);
        gconf_client_set_string (conf_client, 
                                 GIBBON_GCONF_SERVER_PREFS_PREFIX "login",
                                 login,
                                 NULL);
        if (address && *address) {
                gconf_client_set_string (conf_client, 
                                         GIBBON_GCONF_SERVER_PREFS_PREFIX "address",
                                         address,
                                         NULL);
        } else {
                gconf_client_unset (conf_client,
                                    GIBBON_GCONF_SERVER_PREFS_PREFIX "address",
                                    NULL);
        }
        
        check_button = gtk_builder_get_object (builder, 
                                               "conn_checkbutton_remember");
        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button))) {
                gconf_client_set_string (conf_client, 
                                         GIBBON_GCONF_SERVER_PREFS_PREFIX "password",
                                         password,
                                         NULL);
                gconf_client_set_bool (conf_client,
                                       GIBBON_GCONF_SERVER_PREFS_PREFIX "save_pwd",
                                       TRUE, NULL);
        } else {
                /* First overwrite the password, then unset it.  One of the
                 * two will hopefully succeed.
                 */
                gconf_client_set_string (conf_client, 
                                         GIBBON_GCONF_SERVER_PREFS_PREFIX "password",
                                         "",
                                         NULL);
                gconf_client_unset (conf_client, 
                                         GIBBON_GCONF_SERVER_PREFS_PREFIX "password",
                                         NULL);
                gconf_client_set_bool (conf_client,
                                       GIBBON_GCONF_SERVER_PREFS_PREFIX "save_pwd",
                                       FALSE, NULL);
        }

        set_state_connecting ();        
        gibbon_connection_connect (connection);
}

G_MODULE_EXPORT void
on_conn_button_cancel_clicked (GtkObject *object, gpointer user_data)
{
        gtk_widget_hide (GTK_WIDGET (connection_dialog));        
}

G_MODULE_EXPORT void
on_register_link_clicked (GtkObject *object, gpointer user_data)
{
        GdkScreen *screen;
        GError *error;
        const gchar *uri = gtk_link_button_get_uri (GTK_LINK_BUTTON (object));
                                             
        if (gtk_widget_has_screen (window))
                screen = gtk_widget_get_screen (window);
        else
                screen = gdk_screen_get_default ();

        error = NULL;
        gtk_show_uri (screen, uri,
                      gtk_get_current_event_time (),
                      &error);
}
