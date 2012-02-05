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
 * SECTION:gibbon-register-dialog
 * @short_description: The registration dialog in Gibbon.
 *
 * Since: 0.1.0
 *
 * Class representing the Gibbon registration dialog.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include <errno.h>

#include "gibbon-register-dialog.h"
#include "gibbon-settings.h"
#include "gibbon-archive.h"

typedef struct _GibbonRegisterDialogPrivate GibbonRegisterDialogPrivate;
struct _GibbonRegisterDialogPrivate {
        GibbonApp *app;

        GSettings *settings;

        GtkWidget *server_entry;
        GtkWidget *port_entry;
        GtkWidget *login_entry;
        GtkWidget *save_password_button;
        GtkWidget *password_entry;
        GtkWidget *password_entry2;

        gboolean check_okay;
};

#define GIBBON_REGISTER_DIALOG_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_REGISTER_DIALOG, GibbonRegisterDialogPrivate))

G_DEFINE_TYPE (GibbonRegisterDialog, gibbon_register_dialog, GTK_TYPE_DIALOG)

static void gibbon_register_dialog_response (GtkDialog *dialog,
                                               gint response);
static gboolean gibbon_register_dialog_check (const GibbonRegisterDialog *self);

static void 
gibbon_register_dialog_init (GibbonRegisterDialog *self)
{
        GtkWidget *table;
        GtkWidget *content_area;
        gchar *str;

        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_REGISTER_DIALOG, GibbonRegisterDialogPrivate);

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
        gtk_entry_set_text (GTK_ENTRY (self->priv->login_entry), "");

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
        }
        gtk_entry_set_text (GTK_ENTRY (self->priv->password_entry), "");

        gtk_table_attach_defaults (GTK_TABLE (table),
                                   gtk_label_new (_("Repeat password")),
                                   0, 1, 5, 6);
        self->priv->password_entry2 = gtk_entry_new ();
        gtk_entry_set_visibility (GTK_ENTRY (self->priv->password_entry2),
                                  FALSE);
        gtk_table_attach_defaults (GTK_TABLE (table),
                                   self->priv->password_entry2,
                                   1, 2, 5, 6);

        gtk_dialog_add_button (GTK_DIALOG (self), GTK_STOCK_CANCEL,
                               GTK_RESPONSE_CANCEL);
        gtk_dialog_add_button (GTK_DIALOG (self), GTK_STOCK_OK,
                               GTK_RESPONSE_OK);

        self->priv->check_okay = FALSE;
}

static void
gibbon_register_dialog_finalize (GObject *object)
{
        GibbonRegisterDialog *self = GIBBON_REGISTER_DIALOG (object);

        if (self->priv->settings)
                g_object_unref (self->priv->settings);

        G_OBJECT_CLASS (gibbon_register_dialog_parent_class)->finalize(object);
}

static void
gibbon_register_dialog_class_init (GibbonRegisterDialogClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GtkDialogClass *dialog_class = GTK_DIALOG_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonRegisterDialogPrivate));

        dialog_class->response = gibbon_register_dialog_response;

        object_class->finalize = gibbon_register_dialog_finalize;
}

/**
 * gibbon_register_dialog_new:
 *
 * Creates a new #GibbonRegisterDialog.
 *
 * Returns: The newly created #GibbonRegisterDialog or %NULL in case of
 * failure.
 */
GibbonRegisterDialog *
gibbon_register_dialog_new (GibbonApp *app)
{
        GibbonRegisterDialog *self =
                        g_object_new (GIBBON_TYPE_REGISTER_DIALOG, NULL);

        self->priv->app = app;

        gtk_window_set_title (GTK_WINDOW (self), _("Register New Account"));
        gtk_dialog_set_default_response (GTK_DIALOG (self), GTK_RESPONSE_OK);

        gtk_widget_show_all (GTK_WIDGET (self));

        return self;
}

static void
gibbon_register_dialog_response (GtkDialog *dialog, gint response)
{
        GibbonRegisterDialog *self = GIBBON_REGISTER_DIALOG (dialog);
        gchar *saved_password;
        const gchar *new_password;

        if (response != GTK_RESPONSE_OK
            || !gibbon_register_dialog_check (self)) {
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
gibbon_register_dialog_get_password (const GibbonRegisterDialog *self)
{
        g_return_val_if_fail (GIBBON_IS_REGISTER_DIALOG (self), NULL);

        return gtk_entry_get_text (GTK_ENTRY (self->priv->password_entry));
}

static gboolean
gibbon_register_dialog_check (const GibbonRegisterDialog *self)
{
        const gchar *hostname;
        guint port;
        const gchar *login;
        const gchar *password;
        const gchar *password2;
        GibbonArchive *archive;
        gint i;
        GSList *accounts, *iter;
        guint num_accounts;
        GString *message;
        gchar *msg;
        GVariant *variant;
        GtkWidget *window;
        GtkWidget *dialog;
        gint response;
        guint64 length;

        login = gtk_entry_get_text (GTK_ENTRY (self->priv->login_entry));
        if (!login || !*login) {
                gibbon_app_display_error (self->priv->app, NULL,
                                          _("You must specify a user name!"));
                return FALSE;
        }

        if (0 == g_strcmp0 (login, "guest")) {
                gibbon_app_display_error (self->priv->app, NULL,
                                          _("The login `%s' is reserved!"),
                                          "guest");
                return FALSE;
        }

        length = strlen (login);
        if (20 < length) {
                gibbon_app_display_error (self->priv->app, NULL,
                                          _("Your user name may not be longer"
                                            " than 20 characters!"));
                return FALSE;
        } else if (3 > length) {
                gibbon_app_display_error (self->priv->app, NULL,
                                          _("Your user name must have at least"
                                            " 3 characters!"));
                return FALSE;
        }
        for (i = 0; i < 20; ++i) {
                if (!login[i])
                        break;
                if ((login[i] >= 'a' && login[i] <= 'z')
                        || (login[i] >= 'A' && login[i] <= 'Z')
                        || login[i] == '_')
                        continue;
                gibbon_app_display_error (self->priv->app, NULL,
                                          _("Your user name may only contain"
                                            " characters from A to Z, a to z,"
                                            " or the underscore ('_')!"));
                return FALSE;
        }

        password = gtk_entry_get_text (GTK_ENTRY (self->priv->password_entry));
        if (!password || !*password) {
                gibbon_app_display_error (self->priv->app, NULL,
                                          _("You must specify a password!"));
                return FALSE;
        }

        length = strlen (password);
        if (4 > length) {
                gibbon_app_display_error (self->priv->app, NULL,
                                          _("Your password must have at least"
                                            " 4 characters!"));
                return FALSE;
        }
        for (i = 0; i < length; ++i) {
                if (password[i] == ':') {
                        gibbon_app_display_error (self->priv->app, NULL, "%s",
                                                  _("Your password must not"
                                                    " contain a colon (`:')!"));
                        return FALSE;
                }
        }

        password2 = gtk_entry_get_text (GTK_ENTRY (self->priv->password_entry2));
        if (!password2 || !*password2) {
                gibbon_app_display_error (self->priv->app, NULL,
                                          _("Please repeat the password!"));
                return FALSE;
        }

        if (g_strcmp0 (password, password2)) {
                gibbon_app_display_error (self->priv->app, NULL,
                                          _("Passwords do not match!"));
                return FALSE;
        }

        hostname = gtk_entry_get_text (GTK_ENTRY (self->priv->server_entry));
        if (!hostname || !*hostname) {
                gibbon_app_display_error (self->priv->app, NULL,
                                          _("You must specify a server name."
                                          " In doubt try `fibs.com'."));
                return FALSE;
        }

        variant = g_settings_get_value (self->priv->settings,
                                        GIBBON_PREFS_SERVER_PORT);
        port = g_variant_get_uint16 (variant);
        g_variant_unref (variant);
        if (!port) {
                gibbon_app_display_error (self->priv->app, NULL,
                                         _("Invalid port number!"
                                           " In doubt try the default"
                                           " port number 4321."));
                return FALSE;
        }

        archive = gibbon_app_get_archive (self->priv->app);
        accounts = gibbon_archive_get_accounts(archive, hostname, port);
        if (accounts) {
                num_accounts = g_slist_length (accounts);
                msg = g_strdup_printf (g_dngettext (GETTEXT_PACKAGE,
                                                    "You already have an"
                                                    " account on that"
                                                    " server:\n",
                                                    "You alreaday have %llu"
                                                    " accounts on that"
                                                    " server:\n",
                                                    num_accounts),
                                       (unsigned long long) num_accounts);
                message = g_string_new (msg);
                g_free (msg);
                g_string_append_c (message, '\n');
                iter = accounts;
                while (iter) {
                        g_string_append (message, iter->data);
                        g_string_append_c (message, '\n');
                        iter = iter->next;
                }
                g_string_append_c (message, '\n');
                g_string_append (message,
                                 _("It is not allowed to have more than one"
                                   " account per person.  Register anyway?"));

                window = gibbon_app_get_window (self->priv->app);
                dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                                 GTK_MESSAGE_QUESTION,
                                                 GTK_BUTTONS_OK_CANCEL,
                                                 "%s", message->str);
                g_string_free (message, TRUE);
                response = gtk_dialog_run (GTK_DIALOG (dialog));
                gtk_widget_destroy (dialog);

                if (response != GTK_RESPONSE_OK) {
                        iter = accounts;
                        while (iter) {
                                g_free (iter->data);
                                iter = iter->next;
                        }
                        g_slist_free (accounts);
                        return FALSE;
                }
        }
        iter = accounts;
        while (iter) {
                g_free (iter->data);
                iter = iter->next;
        }
        g_slist_free (accounts);

        self->priv->check_okay = TRUE;

        return TRUE;
}

gboolean
gibbon_register_dialog_okay (const GibbonRegisterDialog *self)
{
        g_return_val_if_fail (GIBBON_IS_REGISTER_DIALOG (self), FALSE);

        return self->priv->check_okay;
}
