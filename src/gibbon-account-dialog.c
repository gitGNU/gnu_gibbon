/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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
 * SECTION:gibbon-account-dialog
 * @short_description: The account dialog.
 *
 * Since: 0.1.1
 */

#include <errno.h>

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "gibbon-account-dialog.h"
#include "gibbon-signal.h"
#include "gibbon-prefs.h"

typedef struct _GibbonAccountDialogPrivate GibbonAccountDialogPrivate;
struct _GibbonAccountDialogPrivate {
        GibbonApp *app;

        GtkDialog *dialog;

        GibbonSignal *cancel_handler;
        GibbonSignal *destroy_handler;
        GibbonSignal *okay_handler;
        GibbonSignal *remember_handler;
};

#define GIBBON_ACCOUNT_DIALOG_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_ACCOUNT_DIALOG, GibbonAccountDialogPrivate))

G_DEFINE_TYPE (GibbonAccountDialog, gibbon_account_dialog, G_TYPE_OBJECT)

static void gibbon_account_dialog_on_cancel (GibbonAccountDialog *self);
static void gibbon_account_dialog_on_okay (GibbonAccountDialog *self);
static void gibbon_account_dialog_on_remember (const GibbonAccountDialog *self);
static void gibbon_account_dialog_reset (const GibbonAccountDialog *self);

static void 
gibbon_account_dialog_init (GibbonAccountDialog *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_ACCOUNT_DIALOG, GibbonAccountDialogPrivate);

        self->priv->app = NULL;

        self->priv->cancel_handler = NULL;
        self->priv->destroy_handler = NULL;
        self->priv->okay_handler = NULL;
        self->priv->remember_handler = NULL;
}

static void
gibbon_account_dialog_finalize (GObject *object)
{
        GibbonAccountDialog *self = GIBBON_ACCOUNT_DIALOG (object);

        self->priv->app = NULL;

        if (self->priv->cancel_handler)
                g_object_unref (self->priv->cancel_handler);
        self->priv->cancel_handler = NULL;

        if (self->priv->destroy_handler)
                g_object_unref (self->priv->destroy_handler);
        self->priv->destroy_handler = NULL;

        if (self->priv->okay_handler)
                g_object_unref (self->priv->okay_handler);
        self->priv->okay_handler = NULL;

        if (self->priv->remember_handler)
                g_object_unref (self->priv->remember_handler);
        self->priv->remember_handler = NULL;

        G_OBJECT_CLASS (gibbon_account_dialog_parent_class)->finalize(object);
}

static void
gibbon_account_dialog_class_init (GibbonAccountDialogClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonAccountDialogPrivate));

        object_class->finalize = gibbon_account_dialog_finalize;
}

/**
 * gibbon_account_dialog_new:
 * @app: The #GibbonApp.
 *
 * Creates a new #GibbonAccountDialog.
 *
 * Returns: The newly created #GibbonAccountDialog or %NULL in case of failure.
 */
GibbonAccountDialog *
gibbon_account_dialog_new (GibbonApp *app)
{
        GibbonAccountDialog *self = g_object_new (GIBBON_TYPE_ACCOUNT_DIALOG,
                                                  NULL);
        GObject *obj;

        self->priv->app = app;
        self->priv->dialog =
                GTK_DIALOG (gibbon_app_find_object (app, "account-dialog",
                                                    GTK_TYPE_DIALOG));
        self->priv->destroy_handler =
                gibbon_signal_new (G_OBJECT (self->priv->dialog), "destroy",
                                   G_CALLBACK (gibbon_account_dialog_on_cancel),
                                   G_OBJECT (self));

        obj = gibbon_app_find_object (app, "account-dialog-cancel",
                                      GTK_TYPE_BUTTON);
        self->priv->cancel_handler =
                gibbon_signal_new (obj, "clicked",
                                   G_CALLBACK (gibbon_account_dialog_on_cancel),
                                   G_OBJECT (self));

        obj = gibbon_app_find_object (app, "account-dialog-okay",
                                      GTK_TYPE_BUTTON);
        self->priv->okay_handler =
                gibbon_signal_new (obj, "clicked",
                                   G_CALLBACK (gibbon_account_dialog_on_okay),
                                   G_OBJECT (self));

        obj = gibbon_app_find_object (app, "account-checkbutton-remember",
                                      GTK_TYPE_CHECK_BUTTON);
        self->priv->remember_handler =
                gibbon_signal_new (obj, "toggled",
                                   G_CALLBACK (gibbon_account_dialog_on_remember),
                                   G_OBJECT (self));
        return self;
}

void
gibbon_account_dialog_show (GibbonAccountDialog *self)
{
        g_return_if_fail (GIBBON_IS_ACCOUNT_DIALOG (self));

        if (gtk_widget_get_visible (GTK_WIDGET (self->priv->dialog)))
                return;

        gibbon_account_dialog_reset (self);
        gtk_widget_show_all (GTK_WIDGET (self->priv->dialog));
}

static void
gibbon_account_dialog_on_cancel (GibbonAccountDialog *self)
{
        g_return_if_fail (GIBBON_IS_ACCOUNT_DIALOG (self));

        gtk_widget_hide (GTK_WIDGET (self->priv->dialog));
}

static void
gibbon_account_dialog_on_okay (GibbonAccountDialog *self)
{
        const gchar *server;
        const gchar *port;
        const gchar *login;
        const gchar *password;
        const gchar *address;
        GObject *check_button;
        guint64 portno = 4321;
        char *endptr;
        GibbonApp *app;
        GibbonPrefs *prefs;

        g_return_if_fail (GIBBON_IS_ACCOUNT_DIALOG (self));

        app = self->priv->app;

        server = gibbon_app_get_trimmed_entry_text (app,
                                                    "account-entry-server");
        port = gibbon_app_get_trimmed_entry_text (app,
                                                  "account-entry-port");
        login = gibbon_app_get_trimmed_entry_text (app,
                                                   "account-entry-login");
        password = gibbon_app_get_entry_text (app, "account-entry-password");
        address = gibbon_app_get_entry_text (app, "account-entry-address");

        if (port[0] != '\000') {
                errno = 0;
                portno = g_ascii_strtoull (port, &endptr, 10);
                if (errno) {
                        gibbon_app_display_error (app,
                                                  _("Invalid port `%s': %s."),
                                                  port, g_strerror (errno));

                        return;
                }

                if (*endptr != '\000' || portno > 65536) {
                        gibbon_app_display_error (app,
                                                  _("Invalid port number `%s'."),
                                                  port);
                        return;
                }
        }

        if (login[0] == '\000') {
                gibbon_app_display_error (app,
                                          _("You have to specify your user "
                                          "name (login)."));
                return;
        }

        if (0 == g_strcmp0 ("guest", login)) {
                gibbon_app_display_error (app,
                                          _("Guest login is not supported."));
                return;
        }

        prefs = gibbon_app_get_prefs (app);
        gibbon_prefs_set_string (prefs, GIBBON_PREFS_HOST, server);
        gibbon_prefs_set_int (prefs, GIBBON_PREFS_PORT, portno);
        gibbon_prefs_set_string (prefs, GIBBON_PREFS_LOGIN, login);
        gibbon_prefs_set_string (prefs, GIBBON_PREFS_MAIL_ADDRESS,
                                 address);

        check_button = gibbon_app_find_object (app,
                                               "account-checkbutton-remember",
                                               GTK_TYPE_CHECK_BUTTON);
        if (gibbon_prefs_boolean_read_toggle_button (prefs,
                                         GTK_TOGGLE_BUTTON (check_button),
                                         GIBBON_PREFS_SAVE_PASSWORD)) {
                gibbon_prefs_set_string (prefs,
                                         GIBBON_PREFS_PASSWORD,
                                         password);
        } else {
                gibbon_prefs_set_string (prefs,
                                         GIBBON_PREFS_PASSWORD,
                                         NULL);
        }

        gtk_widget_hide (GTK_WIDGET (self->priv->dialog));
}

static void
gibbon_account_dialog_reset (const GibbonAccountDialog *self)
{
        GibbonPrefs *prefs;
        GibbonApp *app;
        GObject *entry;
        GObject *toggle;
        gboolean save_password;

        g_return_if_fail (GIBBON_IS_ACCOUNT_DIALOG (self));

        app = self->priv->app;

        prefs = gibbon_app_get_prefs (app);

        entry = gibbon_app_find_object (app, "account-entry-server",
                                        GTK_TYPE_ENTRY);
        gibbon_prefs_string_update_entry (prefs, GTK_ENTRY (entry),
                                          GIBBON_PREFS_HOST);
        entry = gibbon_app_find_object (app, "account-entry-login",
                                        GTK_TYPE_ENTRY);
        gibbon_prefs_string_update_entry (prefs, GTK_ENTRY (entry),
                                          GIBBON_PREFS_LOGIN);
        entry = gibbon_app_find_object (app, "account-entry-address",
                                        GTK_TYPE_ENTRY);
        gibbon_prefs_string_update_entry (prefs, GTK_ENTRY (entry),
                                          GIBBON_PREFS_MAIL_ADDRESS);
        toggle = gibbon_app_find_object (app, "account-checkbutton-remember",
                                         GTK_TYPE_CHECK_BUTTON);
        gibbon_prefs_boolean_update_toggle_button (prefs,
                                                   GTK_TOGGLE_BUTTON (toggle),
                                                   GIBBON_PREFS_SAVE_PASSWORD);

        save_password = gibbon_prefs_get_boolean (prefs,
                                                  GIBBON_PREFS_SAVE_PASSWORD);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle),
                                      save_password);

        entry = gibbon_app_find_object (app, "account-entry-password",
                                        GTK_TYPE_ENTRY);
        gibbon_prefs_string_update_entry (prefs, GTK_ENTRY (entry),
                                          GIBBON_PREFS_PASSWORD);
}

static void
gibbon_account_dialog_on_remember (const GibbonAccountDialog *self)
{
        GObject *toggle;
        GObject *entry;

        gboolean remember;

        g_return_if_fail (GIBBON_IS_ACCOUNT_DIALOG (self));

        toggle = gibbon_app_find_object (self->priv->app,
                                         "account-checkbutton-remember",
                                         GTK_TYPE_CHECK_BUTTON);
        remember = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle));
        entry = gibbon_app_find_object (self->priv->app,
                                        "account-entry-password",
                                        GTK_TYPE_ENTRY);

        gtk_widget_set_sensitive (GTK_WIDGET (entry), remember);
}
