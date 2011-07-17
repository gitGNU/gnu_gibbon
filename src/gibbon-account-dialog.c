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
#include "gibbon-settings.h"

typedef struct _GibbonAccountDialogPrivate GibbonAccountDialogPrivate;
struct _GibbonAccountDialogPrivate {
        GibbonApp *app;

        GtkDialog *dialog;

        GibbonSignal *cancel_handler;
        GibbonSignal *destroy_handler;
        GibbonSignal *okay_handler;

        GSettings *settings;
};

#define GIBBON_ACCOUNT_DIALOG_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_ACCOUNT_DIALOG, GibbonAccountDialogPrivate))

G_DEFINE_TYPE (GibbonAccountDialog, gibbon_account_dialog, G_TYPE_OBJECT)

static void gibbon_account_dialog_on_cancel (GibbonAccountDialog *self);
static void gibbon_account_dialog_on_okay (GibbonAccountDialog *self);

static void 
gibbon_account_dialog_init (GibbonAccountDialog *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_ACCOUNT_DIALOG, GibbonAccountDialogPrivate);

        self->priv->app = NULL;

        self->priv->cancel_handler = NULL;
        self->priv->destroy_handler = NULL;
        self->priv->okay_handler = NULL;

        self->priv->settings = g_settings_new (GIBBON_PREFS_SERVER_SCHEMA);
        g_settings_delay (self->priv->settings);
}

static void
gibbon_account_dialog_finalize (GObject *object)
{
        GibbonAccountDialog *self = GIBBON_ACCOUNT_DIALOG (object);

        self->priv->app = NULL;

        if (self->priv->cancel_handler)
                g_object_unref (self->priv->cancel_handler);

        if (self->priv->destroy_handler)
                g_object_unref (self->priv->destroy_handler);

        if (self->priv->okay_handler)
                g_object_unref (self->priv->okay_handler);

        if (self->priv->settings)
                g_object_unref (self->priv->settings);

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
        gchar *string;

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

        obj = gibbon_app_find_object (app, "account-entry-server",
                                      GTK_TYPE_ENTRY);
        g_settings_bind_with_mapping (self->priv->settings,
                                      GIBBON_PREFS_SERVER_HOST, obj, "text",
                                      G_SETTINGS_BIND_DEFAULT,
                                      NULL,
                                      gibbon_settings_bind_trimmed_string,
                                      NULL, NULL);

        obj = gibbon_app_find_object (app, "account-entry-port",
                                      GTK_TYPE_ENTRY);
        g_settings_bind_with_mapping (self->priv->settings,
                                      GIBBON_PREFS_SERVER_PORT, obj, "text",
                                      G_SETTINGS_BIND_DEFAULT,
                                      gibbon_settings_bind_port_to_string,
                                      gibbon_settings_bind_string_to_port,
                                      NULL, NULL);

        obj = gibbon_app_find_object (app, "account-entry-login",
                                      GTK_TYPE_ENTRY);
        g_settings_bind_with_mapping (self->priv->settings,
                                      GIBBON_PREFS_SERVER_LOGIN, obj, "text",
                                      G_SETTINGS_BIND_DEFAULT,
                                      NULL,
                                      gibbon_settings_bind_trimmed_string,
                                      NULL, NULL);

        obj = gibbon_app_find_object (app,
                                      "account-checkbutton-remember",
                                      GTK_TYPE_CHECK_BUTTON);
        g_settings_bind (self->priv->settings,
                         GIBBON_PREFS_SERVER_SAVE_PASSWORD, obj,
                         "active",
                         G_SETTINGS_BIND_DEFAULT);

        obj = gibbon_app_find_object (app, "account-entry-password",
                                      GTK_TYPE_ENTRY);
        if (g_settings_get_boolean (self->priv->settings,
                                    GIBBON_PREFS_SERVER_SAVE_PASSWORD)) {
                g_settings_bind (self->priv->settings,
                                 GIBBON_PREFS_SERVER_PASSWORD, obj, "text",
                                 G_SETTINGS_BIND_DEFAULT);
        } else {
                string = g_settings_get_string (self->priv->settings,
                                                GIBBON_PREFS_SERVER_PASSWORD);
                if (*string)
                        g_settings_set_string (self->priv->settings,
                                               GIBBON_PREFS_SERVER_PASSWORD,
                                               "");
                g_free (string);

                gtk_entry_set_text (GTK_ENTRY (obj), "");
        }

        obj = gibbon_app_find_object (app, "account-entry-address",
                                      GTK_TYPE_ENTRY);
        g_settings_bind_with_mapping (self->priv->settings,
                                      GIBBON_PREFS_SERVER_ADDRESS, obj, "text",
                                      G_SETTINGS_BIND_DEFAULT,
                                      NULL,
                                      gibbon_settings_bind_trimmed_string,
                                      NULL, NULL);

        return self;
}

void
gibbon_account_dialog_show (GibbonAccountDialog *self)
{
        g_return_if_fail (GIBBON_IS_ACCOUNT_DIALOG (self));

        if (gtk_widget_get_visible (GTK_WIDGET (self->priv->dialog)))
                return;

        gtk_widget_show_all (GTK_WIDGET (self->priv->dialog));
}

static void
gibbon_account_dialog_on_cancel (GibbonAccountDialog *self)
{
        g_return_if_fail (GIBBON_IS_ACCOUNT_DIALOG (self));

        g_settings_revert (self->priv->settings);

        gtk_widget_hide (GTK_WIDGET (self->priv->dialog));
}

static void
gibbon_account_dialog_on_okay (GibbonAccountDialog *self)
{
        GibbonApp *app;
        GVariant *variant;
        guint16 portno;
        gchar *string;

        GObject *obj;

        g_return_if_fail (GIBBON_IS_ACCOUNT_DIALOG (self));

        app = self->priv->app;

        variant = g_settings_get_value (self->priv->settings,
                                        GIBBON_PREFS_SERVER_PORT);
        portno = g_variant_get_uint16 (variant);
        g_variant_unref (variant);
        if (!portno) {
                g_settings_revert (self->priv->settings);
                gibbon_app_display_error (app, _("Invalid port number!"));
                return;
        }

        string = g_settings_get_string (self->priv->settings,
                                        GIBBON_PREFS_SERVER_LOGIN);

        if (0 == g_strcmp0 ("guest", string)) {
                g_free (string);
                g_settings_revert (self->priv->settings);
                gibbon_app_display_error (app,
                                          _("Guest login is not supported."));
                return;
        }

        string = g_settings_get_string (self->priv->settings,
                                        GIBBON_PREFS_SERVER_PASSWORD);
        obj = gibbon_app_find_object (app,
                                      "account-entry-password",
                                      GTK_TYPE_ENTRY);
        if (g_settings_get_boolean (self->priv->settings,
                                    GIBBON_PREFS_SERVER_SAVE_PASSWORD)) {
                /*
                 * Well, ummh, ... the effect of this is that it works. ;-)
                 */
                g_settings_bind (self->priv->settings,
                                 GIBBON_PREFS_SERVER_PASSWORD, obj,
                                 "text",
                                 G_SETTINGS_BIND_SET);
                g_settings_bind (self->priv->settings,
                                 GIBBON_PREFS_SERVER_PASSWORD, obj,
                                 "text",
                                 G_SETTINGS_BIND_DEFAULT);
        } else {
                if (*string)
                        g_settings_set_string (self->priv->settings,
                                               GIBBON_PREFS_SERVER_PASSWORD,
                                               "");
                g_settings_unbind (obj, "text");
                gtk_entry_set_text (GTK_ENTRY (obj), "");
        }
        g_free (string);

        gtk_widget_hide (GTK_WIDGET (self->priv->dialog));

        g_settings_apply (self->priv->settings);
}
