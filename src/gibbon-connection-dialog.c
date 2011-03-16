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
 * SECTION:gibbon-connection-dialog
 * @short_description: The connection dialog in Gibbon.
 *
 * Since: 0.1.1
 *
 * Class representing the Gibbon connection dialog.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-connection-dialog.h"
#include "gibbon-prefs.h"
#include "gibbon-signal.h"

typedef struct _GibbonConnectionDialogPrivate GibbonConnectionDialogPrivate;
struct _GibbonConnectionDialogPrivate {
        GibbonApp *app;
        GtkDialog *dialog;

        GibbonSignal *cancel_signal;
        GibbonSignal *register_link_signal;
};

#define GIBBON_CONNECTION_DIALOG_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_CONNECTION_DIALOG, GibbonConnectionDialogPrivate))

G_DEFINE_TYPE (GibbonConnectionDialog, gibbon_connection_dialog, G_TYPE_OBJECT)

static void gibbon_connection_dialog_on_cancel (GibbonConnectionDialog *self);
static void gibbon_connection_dialog_on_register_link (GibbonConnectionDialog
                                                       *self,
                                                       GtkLinkButton *emitter);

static void 
gibbon_connection_dialog_init (GibbonConnectionDialog *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_CONNECTION_DIALOG, GibbonConnectionDialogPrivate);

        self->priv->app = NULL;
        self->priv->dialog = NULL;

        self->priv->cancel_signal = NULL;
        self->priv->register_link_signal = NULL;
}

static void
gibbon_connection_dialog_finalize (GObject *object)
{
        GibbonConnectionDialog *self = GIBBON_CONNECTION_DIALOG (object);

        self->priv->app = NULL;

        if (self->priv->dialog)
                gtk_widget_hide (GTK_WIDGET (self->priv->dialog));
        self->priv->dialog = NULL;

        if (self->priv->cancel_signal)
                g_object_unref (self->priv->cancel_signal);
        self->priv->cancel_signal = NULL;

        if  (self->priv->register_link_signal)
                g_object_unref (self->priv->register_link_signal);
        self->priv->register_link_signal = NULL;

        G_OBJECT_CLASS (gibbon_connection_dialog_parent_class)->finalize(object);
}

static void
gibbon_connection_dialog_class_init (GibbonConnectionDialogClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonConnectionDialogPrivate));

        object_class->finalize = gibbon_connection_dialog_finalize;
}

/**
 * gibbon_connection_dialog_new:
 * @app: The #GibbonApp.
 *
 * Creates a new #GibbonConnectionDialog.
 *
 * Returns: The newly created #GibbonConnectionDialog or %NULL in case of
 * failure.
 */
GibbonConnectionDialog *
gibbon_connection_dialog_new (GibbonApp *app)
{
        GibbonConnectionDialog *self =
                        g_object_new (GIBBON_TYPE_CONNECTION_DIALOG, NULL);
        GObject *entry;
        GObject *toggle;
        gboolean save_password;
        GibbonPrefs *prefs;
        GObject *emitter;

        self->priv->app = app;

        gibbon_app_set_state_connecting (app);

        prefs = gibbon_app_get_prefs (app);

        entry = gibbon_app_find_object (app, "conn_entry_server",
                                        GTK_TYPE_ENTRY);
        gibbon_prefs_string_update_entry (prefs, GTK_ENTRY (entry),
                                          GIBBON_PREFS_HOST);
        entry = gibbon_app_find_object (app, "conn_entry_login",
                                        GTK_TYPE_ENTRY);
        gibbon_prefs_string_update_entry (prefs, GTK_ENTRY (entry),
                                          GIBBON_PREFS_LOGIN);
        entry = gibbon_app_find_object (app, "conn_entry_address",
                                        GTK_TYPE_ENTRY);
        gibbon_prefs_string_update_entry (prefs, GTK_ENTRY (entry),
                                          GIBBON_PREFS_MAIL_ADDRESS);
        toggle = gibbon_app_find_object (app, "conn_checkbutton_remember",
                                         GTK_TYPE_CHECK_BUTTON);
        gibbon_prefs_boolean_update_toggle_button (prefs,
                                                   GTK_TOGGLE_BUTTON (toggle),
                                                   GIBBON_PREFS_SAVE_PASSWORD);

        save_password = gibbon_prefs_get_boolean (prefs,
                                                  GIBBON_PREFS_SAVE_PASSWORD);

        if (!save_password)
                gibbon_prefs_set_string (prefs,
                                         GIBBON_PREFS_PASSWORD,
                                         NULL);

        entry = gibbon_app_find_object (app, "conn_entry_password",
                                        GTK_TYPE_ENTRY);
        gibbon_prefs_string_update_entry (prefs, GTK_ENTRY (entry),
                                          GIBBON_PREFS_PASSWORD);

        emitter = gibbon_app_find_object (app, "conn_button_cancel",
                                          GTK_TYPE_BUTTON);
        self->priv->cancel_signal =
                gibbon_signal_new (emitter, "clicked",
                                G_CALLBACK (gibbon_connection_dialog_on_cancel),
                                   G_OBJECT (self));

        emitter = gibbon_app_find_object (app, "register_link",
                                          GTK_TYPE_LINK_BUTTON);
        self->priv->register_link_signal =
                gibbon_signal_new (emitter, "clicked",
                         G_CALLBACK (gibbon_connection_dialog_on_register_link),
                                   G_OBJECT (self));

        self->priv->dialog =
                GTK_DIALOG (gibbon_app_find_object (app,
                                                    "connection_dialog",
                                                    GTK_TYPE_DIALOG));

        gtk_widget_show (GTK_WIDGET (self->priv->dialog));

        return self;
}

static void
gibbon_connection_dialog_on_cancel (GibbonConnectionDialog *self)
{
        gibbon_app_set_state_disconnected (self->priv->app);

        g_object_unref (self);
}

static void
gibbon_connection_dialog_on_register_link (GibbonConnectionDialog *self,
                                           GtkLinkButton *emitter)
{
        GdkScreen *screen;
        GError *error;
        const gchar *uri = gtk_link_button_get_uri (emitter);
        GtkWidget *window = gibbon_app_get_window (self->priv->app);

        if (gtk_widget_has_screen (window))
                screen = gtk_widget_get_screen (window);
        else
                screen = gdk_screen_get_default ();

        gibbon_app_display_info (self->priv->app,
                                 _("Please fill in the register form in"
                                   " your browser!"));
        error = NULL;
        if (!gtk_show_uri (screen, uri,
                           gtk_get_current_event_time (),
                           &error)) {
                gibbon_app_display_error (self->priv->app, "%s",
                                          error->message);
        }
}
