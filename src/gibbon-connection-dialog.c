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

typedef struct _GibbonConnectionDialogPrivate GibbonConnectionDialogPrivate;
struct _GibbonConnectionDialogPrivate {
        const GibbonApp *app;
};

#define GIBBON_CONNECTION_DIALOG_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_CONNECTION_DIALOG, GibbonConnectionDialogPrivate))

G_DEFINE_TYPE (GibbonConnectionDialog, gibbon_connection_dialog, G_TYPE_OBJECT)

static void 
gibbon_connection_dialog_init (GibbonConnectionDialog *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_CONNECTION_DIALOG, GibbonConnectionDialogPrivate);

        self->priv->app = NULL;
}

static void
gibbon_connection_dialog_finalize (GObject *object)
{
        GibbonConnectionDialog *self = GIBBON_CONNECTION_DIALOG (object);

        self->priv->app = NULL;

        g_object_unref (self);

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
gibbon_connection_dialog_new (const GibbonApp *app)
{
        GibbonConnectionDialog *self =
                        g_object_new (GIBBON_TYPE_CONNECTION_DIALOG, NULL);
        GtkObject *entry;
        GtkToggleButton *toggle;
        gboolean save_password;
        GObject *dialog;
        GibbonPrefs *prefs;

        self->priv->app = app;

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
        gibbon_prefs_string_update_entry (prefs, entry,
                                          GIBBON_PREFS_PASSWORD);

        dialog = gibbon_app_find_object (app, "connection_dialog",
                                         GTK_TYPE_DIALOG);

        gtk_widget_show (GTK_WIDGET (dialog));

        return self;
}
