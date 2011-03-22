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
 * SECTION:gibbon-app
 * @short_description: The Gibbon Application!
 *
 * Since: 0.1.1
 *
 * Class representing the running Gibbon application!
 **/

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <stdlib.h>

#include "gibbon-app.h"
#include "gibbon-cairoboard.h"
#include "gibbon-game-chat.h"
#include "gibbon-prefs.h"
#include "gibbon-connection-dialog.h"
#include "gibbon-connection.h"
#include "gibbon-signal.h"
#include "gibbon-server-console.h"
#include "gibbon-shouts.h"
#include "gibbon-account-dialog.h"

typedef struct _GibbonAppPrivate GibbonAppPrivate;
struct _GibbonAppPrivate {
        GtkBuilder *builder;
        gchar *pixmaps_directory;
        GtkWidget *window;
        GtkWidget *statusbar;
        GibbonServerConsole *server_console;
        GibbonCairoboard *board;
        GibbonGameChat *game_chat;
        GibbonPrefs *prefs;
        GibbonConnectionDialog *connection_dialog;
        GibbonConnection *connection;
        GibbonShouts *shouts;

        GibbonSignal *resolving_signal;
        GibbonSignal *connecting_signal;
        GibbonSignal *connected_signal;
        GibbonSignal *logged_in_signal;
        GibbonSignal *network_error_signal;
        GibbonSignal *disconnected_signal;

        GibbonAccountDialog *account_dialog;
};

#define GIBBON_APP_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_APP, GibbonAppPrivate))

G_DEFINE_TYPE (GibbonApp, gibbon_app, G_TYPE_OBJECT)

static GtkBuilder *gibbon_app_get_builder (GibbonApp *self, const gchar *path);
static GibbonCairoboard *gibbon_app_init_board (GibbonApp *self,
                                                const gchar *board_filename);
static void gibbon_app_connect_signals (const GibbonApp *self);

/* Signal handlers.  */
static void gibbon_app_on_connect_request (GibbonApp *self,
                                           GtkWidget *emitter);
static void gibbon_app_on_quit_request (GibbonApp *self,
                                        GtkWidget *emitter);
static void gibbon_app_on_resolving (GibbonApp *self,
                                     const gchar *hostname);
static void gibbon_app_on_connecting (GibbonApp *self,
                                      GibbonConnection *connection);
static void gibbon_app_on_connected (GibbonApp *self,
                                      GibbonConnection *connection);
static void gibbon_app_on_logged_in (GibbonApp *self,
                                     GibbonConnection *connection);
static void gibbon_app_on_network_error (GibbonApp *self,
                                         const gchar *error_msg);
static void gibbon_app_on_account_prefs (GibbonApp *self);

static GibbonApp *singleton = NULL;
GibbonApp *app;

static void 
gibbon_app_init (GibbonApp *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_APP, GibbonAppPrivate);

        self->priv->builder = NULL;
        self->priv->pixmaps_directory = NULL;

        self->priv->board = NULL;
        self->priv->window = NULL;
        self->priv->statusbar = NULL;
        if (self->priv->server_console)
                g_object_unref (self->priv->server_console);
        self->priv->server_console = NULL;
        self->priv->game_chat = NULL;
        self->priv->prefs = NULL;
        self->priv->connection_dialog = NULL;
        self->priv->connection = NULL;
        self->priv->shouts = NULL;

        self->priv->resolving_signal = NULL;
        self->priv->connecting_signal = NULL;
        self->priv->connected_signal = NULL;
        self->priv->logged_in_signal = NULL;
        self->priv->network_error_signal = NULL;
        self->priv->disconnected_signal = NULL;

        self->priv->account_dialog = NULL;
}

static void
gibbon_app_finalize (GObject *object)
{
        GibbonApp *self = GIBBON_APP (object);

        gibbon_app_disconnect (self);

        if (self->priv->server_console)
                g_object_unref (self->priv->server_console);

        G_OBJECT_CLASS (gibbon_app_parent_class)->finalize(object);
}

static void
gibbon_app_class_init (GibbonAppClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonAppPrivate));

        object_class->finalize = gibbon_app_finalize;
}

/**
 * gibbon_app_new:
 * @builder_path: Complete path to the builder XML file.
 * @pixmaps_dir: Path to the pixmaps.
 *
 * Creates a new #GibbonApp.
 *
 * Returns: The newly created #GibbonApp or %NULL in case of failure.
 */
GibbonApp *
gibbon_app_new (const gchar *builder_path, const gchar *pixmaps_directory)
{
        GibbonApp *self = g_object_new (GIBBON_TYPE_APP, NULL);
        gchar *board_filename;

        g_return_val_if_fail (singleton == NULL, singleton);

        self->priv->prefs = gibbon_prefs_new ();
        if (!self->priv->prefs) {
                g_object_unref (self);
                return NULL;
        }

        self->priv->builder = gibbon_app_get_builder (self, builder_path);
        if (!self->priv->builder) {
                g_object_unref (self);
                return NULL;
        }

        self->priv->pixmaps_directory = g_strdup (pixmaps_directory);

        self->priv->window =
                GTK_WIDGET (gibbon_app_find_object (self, "window",
                                                    GTK_TYPE_WINDOW));
        self->priv->statusbar =
                GTK_WIDGET (gibbon_app_find_object (self,
                                                    "statusbar",
                                                    GTK_TYPE_STATUSBAR));
        self->priv->server_console = gibbon_server_console_new (self);

        board_filename = g_build_filename (pixmaps_directory, "boards",
                                           "default.svg", NULL);
        self->priv->board = gibbon_app_init_board (self, board_filename);
        g_free (board_filename);
        if (!self->priv->board) {
                g_object_unref (self);
                return NULL;
        }

        gtk_statusbar_push (GTK_STATUSBAR (self->priv->statusbar),
                            0, _("Disconnected"));

        self->priv->game_chat = gibbon_game_chat_new (self);
        if (!self->priv->game_chat) {
                g_object_unref (self);
                return NULL;
        }

        self->priv->shouts = gibbon_shouts_new (self);
        if (!self->priv->shouts) {
                g_object_unref (self);
                return NULL;
        }

        gibbon_app_set_state_disconnected (self);

        gibbon_app_connect_signals (self);

        singleton = self;

        return self;
}

static GtkBuilder *
gibbon_app_get_builder (GibbonApp *self, const gchar *path)
{
        GtkBuilder *builder = gtk_builder_new ();
        GError *error = NULL;

        if (!gtk_builder_add_from_file (builder, path, &error)) {
                gibbon_app_display_error (self, "%s.\n%s",
                                          error->message,
                                          _("Do you need to pass the"
                                            " option `--data-dir'?\n"));
                g_object_unref (G_OBJECT (builder));
                return NULL;
        }

        return builder;
}

void
gibbon_app_display_error (const GibbonApp* self,
                          const gchar *message_format, ...)
{
        va_list args;
        gchar *message;

        va_start (args, message_format);
        message = g_strdup_vprintf (message_format, args);
        va_end (args);

        GtkWidget *dialog =
                gtk_message_dialog_new (GTK_WINDOW (self->priv->window),
                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                        GTK_MESSAGE_ERROR,
                                        GTK_BUTTONS_CLOSE,
                                        "%s", message);

        g_free (message);

        gtk_dialog_run (GTK_DIALOG (dialog));

        gtk_widget_destroy (GTK_WIDGET (dialog));
}

void
gibbon_app_display_info (const GibbonApp *self, const gchar *message_format, ...)
{
        va_list args;
        gchar *message;

        va_start (args, message_format);
        message = g_strdup_vprintf (message_format, args);
        va_end (args);

        GtkWidget *dialog =
                gtk_message_dialog_new (GTK_WINDOW (self->priv->window),
                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                        GTK_MESSAGE_INFO,
                                        GTK_BUTTONS_CLOSE,
                                        "%s", message);

        g_free (message);

        gtk_dialog_run (GTK_DIALOG (dialog));

        gtk_widget_destroy (GTK_WIDGET (dialog));
}

GObject *
gibbon_app_find_object (const GibbonApp *self, const gchar *id, GType type)
{
        GObject *obj;
        GType got_type;

        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);
        g_return_val_if_fail (G_TYPE_IS_OBJECT (type), NULL);

        obj = gtk_builder_get_object (self->priv->builder, id);

        if (!obj) {
                /* TRANSLATORS: UI means user interface.  */
                gibbon_app_display_error (self,
                                          _("Object `%s' not found in UI"
                                            " definition!"), id);
                exit (-1);
        }

        if (!G_IS_OBJECT (obj)) {
                gibbon_app_display_error (self,
                                          _("Object `%s' is not a GObject!"),
                                          id);
                exit (-1);
        }

        if (!G_TYPE_CHECK_INSTANCE_TYPE (obj, type)) {
                got_type = G_OBJECT_TYPE (obj);
                gibbon_app_display_error (self,
                                          _("Object `%s' is not of type `%s'"
                                            " but `%s'!"),
                                           id, g_type_name (type),
                                           g_type_name (got_type));
                exit (-1);
        }

        return obj;
}

static GibbonCairoboard *
gibbon_app_init_board (GibbonApp *self, const gchar *board_filename)
{
        GObject *left_vpane;
        GibbonCairoboard *board = gibbon_cairoboard_new (self, board_filename);

        if (!board)
                return NULL;

        gtk_widget_show (GTK_WIDGET (board));
        /* FIXME! This should occupy reasonable space by default!  Do
         * not hardcode the values.
         */
        gtk_widget_set_size_request (GTK_WIDGET (board), 490, 380);

        left_vpane = gibbon_app_find_object (self, "left_vpane",
                                             GTK_TYPE_VPANED);

        gtk_widget_destroy (gtk_paned_get_child1 (GTK_PANED (left_vpane)));
        gtk_paned_pack1 (GTK_PANED (left_vpane), GTK_WIDGET (board),
                         TRUE, FALSE);

        return board;
}

GtkWidget *
gibbon_app_get_window (const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->window;
}

const gchar *
gibbon_app_get_pixmaps_directory (const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->pixmaps_directory;
}

static void
gibbon_app_connect_signals (const GibbonApp *self)
{
        GObject* obj;

        obj = gibbon_app_find_object (self, "toolbar_quit_button",
                                      GTK_TYPE_TOOL_BUTTON);
        g_signal_connect_swapped (obj, "clicked",
                                  G_CALLBACK (gibbon_app_on_quit_request),
                                  (gpointer) self);

        obj = gibbon_app_find_object (self, "quit_menu_item",
                                      GTK_TYPE_MENU_ITEM);
        g_signal_connect_swapped (obj, "activate",
                                  G_CALLBACK (gibbon_app_on_quit_request),
                                  (gpointer) self);

        obj = gibbon_app_find_object (self, "connect_menu_item",
                                      GTK_TYPE_MENU_ITEM);
        g_signal_connect_swapped (obj, "activate",
                                  G_CALLBACK (gibbon_app_on_connect_request),
                                  (gpointer) self);
        obj = gibbon_app_find_object (self, "toolbar_connect_button",
                                      GTK_TYPE_TOOL_BUTTON);
        g_signal_connect_swapped (obj, "clicked",
                                  G_CALLBACK (gibbon_app_on_connect_request),
                                  (gpointer) self);

        obj = gibbon_app_find_object (self, "disconnect_menu_item",
                                      GTK_TYPE_MENU_ITEM);
        g_signal_connect_swapped (obj, "activate",
                                  G_CALLBACK (gibbon_app_disconnect),
                                  (gpointer) self);
        obj = gibbon_app_find_object (self, "toolbar_disconnect_button",
                                      GTK_TYPE_TOOL_BUTTON);
        g_signal_connect_swapped (obj, "clicked",
                                  G_CALLBACK (gibbon_app_disconnect),
                                  (gpointer) self);

        obj = gibbon_app_find_object (self, "window", GTK_TYPE_WINDOW);
        g_signal_connect_swapped (obj, "destroy",
                                  G_CALLBACK (gibbon_app_on_quit_request),
                                  (gpointer) self);

        obj = gibbon_app_find_object (self, "account-menu-item",
                                      GTK_TYPE_MENU_ITEM);
        g_signal_connect_swapped (obj, "activate",
                                  G_CALLBACK (gibbon_app_on_account_prefs),
                                  (gpointer) self);
}

static void
gibbon_app_on_quit_request (GibbonApp *self, GtkWidget *emitter)
{
        g_object_unref (self);

        gtk_main_quit ();
}

static void
gibbon_app_on_connect_request (GibbonApp *self, GtkWidget *emitter)
{
        if (!self->priv->connection_dialog)
                self->priv->connection_dialog =
                                gibbon_connection_dialog_new (self);
}

void
gibbon_app_set_state_disconnected (GibbonApp *self)
{
        GObject* obj;

        obj = gibbon_app_find_object (self, "toolbar_connect_button",
                                      GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive (GTK_WIDGET (obj), TRUE);

        obj = gibbon_app_find_object (self, "toolbar_disconnect_button",
                                      GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive (GTK_WIDGET (obj), FALSE);

        obj = gibbon_app_find_object (self, "connect_menu_item",
                                      GTK_TYPE_IMAGE_MENU_ITEM);
        gtk_widget_set_sensitive (GTK_WIDGET (obj), TRUE);

        obj = gibbon_app_find_object (self, "disconnect_menu_item",
                                      GTK_TYPE_IMAGE_MENU_ITEM);
        gtk_widget_set_sensitive (GTK_WIDGET (obj), FALSE);

        obj = gibbon_app_find_object (self, "server-command-entry",
                                      GTK_TYPE_ENTRY);
        gtk_entry_set_editable (GTK_ENTRY (obj), FALSE);

        obj = gibbon_app_find_object (self, "game-chat-entry",
                                      GTK_TYPE_ENTRY);
        gtk_entry_set_editable (GTK_ENTRY (obj), FALSE);

        obj = gibbon_app_find_object (self, "shout-entry",
                                      GTK_TYPE_ENTRY);
        gtk_entry_set_editable (GTK_ENTRY (obj), FALSE);

}

void
gibbon_app_set_state_connecting (GibbonApp *self)
{
        GObject* obj;

        obj = gibbon_app_find_object (self, "toolbar_connect_button",
                                      GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive (GTK_WIDGET (obj), FALSE);

        obj = gibbon_app_find_object (self, "toolbar_disconnect_button",
                                      GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive (GTK_WIDGET (obj), TRUE);

        obj = gibbon_app_find_object (self, "connect_menu_item",
                                      GTK_TYPE_IMAGE_MENU_ITEM);
        gtk_widget_set_sensitive (GTK_WIDGET (obj), FALSE);

        obj = gibbon_app_find_object (self, "disconnect_menu_item",
                                      GTK_TYPE_IMAGE_MENU_ITEM);
        gtk_widget_set_sensitive (GTK_WIDGET (obj), TRUE);
}

GibbonPrefs *
gibbon_app_get_prefs (const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->prefs;
}

const gchar *
gibbon_app_get_entry_text (const GibbonApp *self, const gchar *id)
{
        GtkWidget *entry;

        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        entry = GTK_WIDGET (gibbon_app_find_object (self, id, GTK_TYPE_ENTRY));

        return gtk_entry_get_text (GTK_ENTRY (entry));
}

const gchar *
gibbon_app_get_trimmed_entry_text (const GibbonApp *self, const gchar *id)
{
        GtkWidget *entry;
        gchar *trimmed;

        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        entry = GTK_WIDGET (gibbon_app_find_object (self, id, GTK_TYPE_ENTRY));

        trimmed = pango_trim_string (gtk_entry_get_text (GTK_ENTRY (entry)));
        gtk_entry_set_text (GTK_ENTRY (entry), trimmed);
        g_free (trimmed);

        return gtk_entry_get_text (GTK_ENTRY (entry));
}

void
gibbon_app_connect (GibbonApp *self)
{
        /* Clean up opened connection dialog and stale connection objects.
         * The latter should not happen.
         */
        gibbon_app_disconnect (self);
        gibbon_app_set_state_connecting (self);

        /* FIXME: Pass the password explicitely, as it might not be saved
         * in the user preferences!
         */
        self->priv->connection = gibbon_connection_new (self);
        if (!self->priv->connection) {
              gibbon_app_disconnect (self);
              return;
        }

        self->priv->resolving_signal = 
                gibbon_signal_new (G_OBJECT (self->priv->connection),
                                   "resolving",
                                   G_CALLBACK (gibbon_app_on_resolving),
                                   G_OBJECT (self));
        self->priv->connecting_signal = 
                gibbon_signal_new (G_OBJECT (self->priv->connection),
                                   "connecting",
                                   G_CALLBACK (gibbon_app_on_connecting),
                                   G_OBJECT (self));
        self->priv->connected_signal = 
                gibbon_signal_new (G_OBJECT (self->priv->connection),
                                   "connected",
                                   G_CALLBACK (gibbon_app_on_connected),
                                   G_OBJECT (self));
        self->priv->logged_in_signal =
                gibbon_signal_new (G_OBJECT (self->priv->connection),
                                   "logged_in",
                                   G_CALLBACK (gibbon_app_on_logged_in),
                                   G_OBJECT (self));
        self->priv->network_error_signal = 
                gibbon_signal_new (G_OBJECT (self->priv->connection),
                                   "network-error",
                                   G_CALLBACK (gibbon_app_on_network_error),
                                   G_OBJECT (self));
        self->priv->disconnected_signal = 
                gibbon_signal_new (G_OBJECT (self->priv->connection),
                                   "disconnected",
                                   G_CALLBACK (gibbon_app_disconnect),
                                   G_OBJECT (self));

        if (!gibbon_connection_connect (self->priv->connection))
                gibbon_app_disconnect (self);
}

void
gibbon_app_disconnect (GibbonApp *self)
{
        if (self->priv->resolving_signal)
                g_object_unref (self->priv->resolving_signal);
        self->priv->resolving_signal = NULL;

        if (self->priv->connecting_signal)
                g_object_unref (self->priv->connecting_signal);
        self->priv->connecting_signal = NULL;

        if (self->priv->connected_signal)
                g_object_unref (self->priv->connected_signal);
        self->priv->connected_signal = NULL;

        if (self->priv->logged_in_signal)
                g_object_unref (self->priv->logged_in_signal);
        self->priv->logged_in_signal = NULL;

        if (self->priv->network_error_signal)
                g_object_unref (self->priv->network_error_signal);
        self->priv->network_error_signal = NULL;

        if (self->priv->disconnected_signal)
                g_object_unref (self->priv->disconnected_signal);
        self->priv->disconnected_signal = NULL;

        if (self->priv->connection_dialog)
                g_object_unref (self->priv->connection_dialog);
        self->priv->connection_dialog = NULL;

        if (self->priv->connection)
                g_object_unref (self->priv->connection);
        self->priv->connection = NULL;

        gibbon_app_set_state_disconnected (self);
}

static void
gibbon_app_on_resolving (GibbonApp *self, const gchar *hostname)
{
        gchar *msg = g_strdup_printf (_("Resolving address for %s."), hostname);
        GtkStatusbar *statusbar = 
                GTK_STATUSBAR (gibbon_app_find_object (self, "statusbar",
                                                       GTK_TYPE_STATUSBAR));
        
        gtk_statusbar_pop (statusbar, 0);
        gtk_statusbar_push (statusbar, 0, msg);
        g_free (msg);
}

static void
gibbon_app_on_connecting (GibbonApp *self, GibbonConnection *conn)
{
        g_return_if_fail (GIBBON_IS_CONNECTION (conn));
        
        gchar *msg = g_strdup_printf (_("Connecting with %s port %d."), 
                                      gibbon_connection_get_hostname (conn),
                                      gibbon_connection_get_port (conn));
        GtkStatusbar *statusbar = 
                GTK_STATUSBAR (gibbon_app_find_object (self, "statusbar",
                                                       GTK_TYPE_STATUSBAR));

        gtk_statusbar_pop (statusbar, 0);
        gtk_statusbar_push (statusbar, 0, msg);
        g_free (msg);
}

static void
gibbon_app_on_connected (GibbonApp *self, GibbonConnection *conn)
{
        g_return_if_fail (GIBBON_IS_CONNECTION (conn));
        
        gchar *msg = g_strdup_printf (_("Connected with %s port %d."), 
                                      gibbon_connection_get_hostname (conn),
                                      gibbon_connection_get_port (conn));
        GtkStatusbar *statusbar = 
                GTK_STATUSBAR (gibbon_app_find_object (self, "statusbar",
                                                       GTK_TYPE_STATUSBAR));

        gtk_statusbar_pop (statusbar, 0);
        gtk_statusbar_push (statusbar, 0, msg);
        g_free (msg);
}

static void
gibbon_app_on_logged_in (GibbonApp *self, GibbonConnection *conn)
{
        GObject *obj;

        g_return_if_fail (GIBBON_IS_CONNECTION (conn));

        gchar *msg = g_strdup_printf (_("Logged in on %s port %d."),
                                      gibbon_connection_get_hostname (conn),
                                      gibbon_connection_get_port (conn));
        GtkStatusbar *statusbar =
                GTK_STATUSBAR (gibbon_app_find_object (self, "statusbar",
                                                       GTK_TYPE_STATUSBAR));

        gtk_statusbar_pop (statusbar, 0);
        gtk_statusbar_push (statusbar, 0, msg);
        g_free (msg);

        obj = gibbon_app_find_object (self, "server-command-entry",
                                      GTK_TYPE_ENTRY);
        gtk_entry_set_editable (GTK_ENTRY (obj), TRUE);

        obj = gibbon_app_find_object (self, "game-chat-entry",
                                      GTK_TYPE_ENTRY);
        gtk_entry_set_editable (GTK_ENTRY (obj), TRUE);

        obj = gibbon_app_find_object (self, "shout-entry",
                                      GTK_TYPE_ENTRY);
        gtk_entry_set_editable (GTK_ENTRY (obj), TRUE);


}

static void
gibbon_app_on_network_error (GibbonApp *self, const gchar *message)
{
        gibbon_app_display_error (self, "%s", message);
        gibbon_app_disconnect (self);
}

GtkImage *
gibbon_app_load_scaled_image (const GibbonApp *self, const gchar *path,
                              gint width, gint height)
{
        GError *error = NULL;
        GtkImage *image;
        GdkPixbuf *pixbuf;

        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);
        
        pixbuf = gdk_pixbuf_new_from_file_at_scale (path,
                                                    width,
                                                    height,
                                                    FALSE,
                                                    &error);

        if (!pixbuf) {
                gibbon_app_display_error (NULL, _("Error loading image `%s': %s!"),
                               path, error->message);
                return NULL;
        }

        image = GTK_IMAGE (gtk_image_new ());
        gtk_image_set_from_pixbuf (image, pixbuf);
        g_object_unref (pixbuf);

        gtk_widget_show (GTK_WIDGET (image));
        return image;
}

GibbonServerConsole *
gibbon_app_get_server_console (const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->server_console;
}

GibbonCairoboard *
gibbon_app_get_board (const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->board;
}

GibbonConnection *
gibbon_app_get_connection (const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->connection;
}

GibbonShouts *
gibbon_app_get_shouts (const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->shouts;
}

static void
gibbon_app_on_account_prefs (GibbonApp *self)
{
        g_return_if_fail (GIBBON_IS_APP (self));

        if (!self->priv->account_dialog)
                self->priv->account_dialog = gibbon_account_dialog_new (self);

        gibbon_account_dialog_show (self->priv->account_dialog);
}
