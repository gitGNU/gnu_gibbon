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

/**
 * SECTION:gibbon-connection-dialog
 * @short_description: The connection dialog in Gibbon.
 *
 * Since: 0.1.0
 *
 * Class representing the Gibbon connection dialog.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include <errno.h>

#include "gibbon-connection-dialog.h"
#include "gibbon-settings.h"

typedef struct _GibbonConnectionDialogPrivate GibbonConnectionDialogPrivate;
struct _GibbonConnectionDialogPrivate {
        GibbonApp *app;

        GSettings *settings;

        GtkWidget *server_entry;
        GtkWidget *port_entry;
        GtkWidget *login_entry;
        GtkWidget *save_password_button;
        GtkWidget *password_entry;
        GtkWidget *address_entry;
};

#define GIBBON_CONNECTION_DIALOG_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_CONNECTION_DIALOG, GibbonConnectionDialogPrivate))

G_DEFINE_TYPE (GibbonConnectionDialog, gibbon_connection_dialog, GTK_TYPE_DIALOG)

static void gibbon_connection_dialog_on_register (GibbonConnectionDialog *self,
                                                  GtkButton *emitter);
static void gibbon_connection_dialog_response (GtkDialog *dialog,
                                               gint response);

static void 
gibbon_connection_dialog_init (GibbonConnectionDialog *self)
{
        GtkWidget *table;
        GtkWidget *content_area;
        gchar *str;
        GtkWidget *register_button;

        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_CONNECTION_DIALOG, GibbonConnectionDialogPrivate);

        self->priv->settings =
                        g_settings_new (GIBBON_PREFS_SERVER_SCHEMA);
        g_settings_delay (self->priv->settings);

        gtk_container_set_border_width (GTK_CONTAINER (self), 5);

        table = gtk_table_new (7, 2, FALSE);
        content_area = gtk_dialog_get_content_area (GTK_DIALOG (self));
        gtk_container_add (GTK_CONTAINER (content_area), GTK_WIDGET (table));

        gtk_table_attach_defaults (GTK_TABLE (table),
                                   gtk_label_new (_("Server")),
                                   0, 1, 0, 1);
        self->priv->server_entry = gtk_entry_new ();
        gtk_table_attach_defaults (GTK_TABLE (table),
                                   self->priv->server_entry,
                                   1, 2, 0, 1);
        g_settings_bind_with_mapping (self->priv->settings,
                                      GIBBON_PREFS_SERVER_HOST,
                                      self->priv->server_entry, "text",
                                      G_SETTINGS_BIND_DEFAULT,
                                      NULL,
                                      gibbon_settings_bind_trimmed_string,
                                      NULL, NULL);

        gtk_table_attach_defaults (GTK_TABLE (table),
                                   gtk_label_new (_("Port")),
                                   0, 1, 1, 2);
        self->priv->port_entry = gtk_entry_new ();
        gtk_table_attach_defaults (GTK_TABLE (table),
                                   self->priv->port_entry,
                                   1, 2, 1, 2);
        g_settings_bind_with_mapping (self->priv->settings,
                                      GIBBON_PREFS_SERVER_PORT,
                                      self->priv->port_entry, "text",
                                      G_SETTINGS_BIND_DEFAULT,
                                      gibbon_settings_bind_port_to_string,
                                      gibbon_settings_bind_string_to_port,
                                      NULL, NULL);

        gtk_table_attach_defaults (GTK_TABLE (table),
                                   gtk_label_new (_("User name")),
                                   0, 1, 2, 3);
        self->priv->login_entry = gtk_entry_new ();
        gtk_table_attach_defaults (GTK_TABLE (table),
                                   self->priv->login_entry,
                                   1, 2, 2, 3);
        g_settings_bind_with_mapping (self->priv->settings,
                                      GIBBON_PREFS_SERVER_LOGIN,
                                      self->priv->login_entry, "text",
                                      G_SETTINGS_BIND_DEFAULT,
                                      NULL,
                                      gibbon_settings_bind_trimmed_string,
                                      NULL, NULL);

        self->priv->save_password_button =
                        gtk_check_button_new_with_label (_("Save password?"));
        gtk_table_attach_defaults (GTK_TABLE (table),
                                   self->priv->save_password_button,
                                   0, 2, 3, 4);
        g_settings_bind (self->priv->settings,
                         GIBBON_PREFS_SERVER_SAVE_PASSWORD,
                         self->priv->save_password_button,
                         "active",
                         G_SETTINGS_BIND_DEFAULT);

        gtk_table_attach_defaults (GTK_TABLE (table),
                                   gtk_label_new (_("Password")),
                                   0, 1, 4, 5);
        self->priv->password_entry = gtk_entry_new ();
        gtk_entry_set_visibility (GTK_ENTRY (self->priv->password_entry),
                                  FALSE);
        gtk_table_attach_defaults (GTK_TABLE (table),
                                   self->priv->password_entry,
                                   1, 2, 4, 5);
        if (g_settings_get_boolean (self->priv->settings,
                                    GIBBON_PREFS_SERVER_SAVE_PASSWORD)) {
                g_settings_bind (self->priv->settings,
                                 GIBBON_PREFS_SERVER_PASSWORD,
                                 self->priv->password_entry, "text",
                                 G_SETTINGS_BIND_DEFAULT);
        } else {
                str = g_settings_get_string (self->priv->settings,
                                                GIBBON_PREFS_SERVER_PASSWORD);
                if (*str)
                        g_settings_set_string (self->priv->settings,
                                               GIBBON_PREFS_SERVER_PASSWORD,
                                               "");
                g_free (str);

                gtk_entry_set_text (GTK_ENTRY (self->priv->password_entry), "");
        }

        register_button = gtk_button_new_with_label (_("Register new account"));
        gtk_table_attach_defaults (GTK_TABLE (table), register_button,
                                   0, 2, 5, 6);
        g_signal_connect_swapped (G_OBJECT (register_button), "clicked",
                                  G_CALLBACK (gibbon_connection_dialog_on_register),
                                  self);

        gtk_table_attach_defaults (GTK_TABLE (table),
                                   gtk_label_new (_("E-mail (optional)")),
                                   0, 1, 6, 7);
        self->priv->address_entry = gtk_entry_new ();
        gtk_table_attach_defaults (GTK_TABLE (table),
                                   self->priv->address_entry,
                                   1, 2, 6, 7);
        g_settings_bind_with_mapping (self->priv->settings,
                                      GIBBON_PREFS_SERVER_ADDRESS,
                                      self->priv->address_entry, "text",
                                      G_SETTINGS_BIND_DEFAULT,
                                      NULL,
                                      gibbon_settings_bind_trimmed_string,
                                      NULL, NULL);

        /*
         * We set the okay button in the constructor, depending on the
         * type of dialog we display.
         */
        gtk_dialog_add_button (GTK_DIALOG (self), GTK_STOCK_CANCEL,
                               GTK_RESPONSE_CANCEL);
}

static void
gibbon_connection_dialog_finalize (GObject *object)
{
        GibbonConnectionDialog *self = GIBBON_CONNECTION_DIALOG (object);

        if (self->priv->settings)
                g_object_unref (self->priv->settings);

        G_OBJECT_CLASS (gibbon_connection_dialog_parent_class)->finalize(object);
}

static void
gibbon_connection_dialog_class_init (GibbonConnectionDialogClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GtkDialogClass *dialog_class = GTK_DIALOG_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonConnectionDialogPrivate));

        dialog_class->response = gibbon_connection_dialog_response;

        object_class->finalize = gibbon_connection_dialog_finalize;
}

/**
 * gibbon_connection_dialog_new:
 * @just_conf: %TRUE, if this is just a configuration dialog..
 *
 * Creates a new #GibbonConnectionDialog.
 *
 * Returns: The newly created #GibbonConnectionDialog or %NULL in case of
 * failure.
 */
GibbonConnectionDialog *
gibbon_connection_dialog_new (GibbonApp *app, gboolean just_conf)
{
        GibbonConnectionDialog *self =
                        g_object_new (GIBBON_TYPE_CONNECTION_DIALOG, NULL);

        self->priv->app = app;

        if (just_conf) {
                gtk_dialog_add_button (GTK_DIALOG (self), GTK_STOCK_OK,
                                       GTK_RESPONSE_OK);
                gtk_window_set_title (GTK_WINDOW (self),
                                      _("Account settings"));
        } else {
                gtk_dialog_add_button (GTK_DIALOG (self), GTK_STOCK_CONNECT,
                                       GTK_RESPONSE_OK);
                gtk_window_set_title (GTK_WINDOW (self),
                                      _("Connection settings"));
        }
        gtk_dialog_set_default_response (GTK_DIALOG (self), GTK_RESPONSE_OK);

        gtk_widget_show_all (GTK_WIDGET (self));

        return self;
}

static void
gibbon_connection_dialog_on_register (GibbonConnectionDialog *self,
                                      GtkButton *emitter)
{
        gtk_dialog_response (GTK_DIALOG (self),
                             GIBBON_CONNECTION_DIALOG_RESPONSE_REGISTER);
}


static void
gibbon_connection_dialog_response (GtkDialog *dialog, gint response)
{
        GibbonConnectionDialog *self = GIBBON_CONNECTION_DIALOG (dialog);
        gchar *saved_password;
        const gchar *new_password;

        if (response != GTK_RESPONSE_OK) {
                g_settings_revert (self->priv->settings);
                return;
        }

        saved_password = g_settings_get_string (self->priv->settings,
                                                GIBBON_PREFS_SERVER_PASSWORD);
        if (g_settings_get_boolean (self->priv->settings,
                                    GIBBON_PREFS_SERVER_SAVE_PASSWORD)) {
                new_password =
                    gtk_entry_get_text (GTK_ENTRY (self->priv->password_entry));
                if (g_strcmp0 (new_password, saved_password))
                        g_settings_set_string (self->priv->settings,
                                               GIBBON_PREFS_SERVER_PASSWORD,
                                               new_password);
        } else if (saved_password && *saved_password) {
                /*
                 * We must unbind before we overwrite the string!
                 */
                g_settings_unbind (G_OBJECT (self->priv->password_entry),
                                   "text");
                g_settings_set_string (self->priv->settings,
                                       GIBBON_PREFS_SERVER_PASSWORD,
                                       "");
        }
        g_free (saved_password);

        g_settings_apply (self->priv->settings);
}

const gchar *
gibbon_connection_dialog_get_password (const GibbonConnectionDialog *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION_DIALOG (self), NULL);

        return gtk_entry_get_text (GTK_ENTRY (self->priv->password_entry));
}
