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

#include <sys/types.h>

#include <glib/gi18n.h>

#include "game.h"
#include "gui.h"
#include "gibbon.h"
#include "gibbon-cairoboard.h"
#include "gibbon-player-list.h"
#include "gibbon-game-chat.h"
#include "gibbon-prefs.h"
#include "gibbon-connection.h"

GtkBuilder *builder = NULL;

GtkWidget *window = NULL;
GtkWidget *connection_dialog = NULL;
GtkWidget *server_text_view = NULL;

GibbonPrefs *prefs = NULL;
GibbonPlayerList *players = NULL;

static GtkWidget *statusbar = NULL;

static GibbonCairoboard *board = NULL;

static GibbonConnection *connection = NULL;

/* The converter in Glade 3.4.x converts menus to GtkAction.  Until we
 * upgrade to 3.5.4 or better, construct the menu manually.  */
static GtkWidget *player_menu = NULL;

static GtkBuilder *get_builder (const gchar* filename);
static void cb_resolving (GtkWidget *emitter, const gchar *hostname);
static void cb_connecting (GtkWidget *emitter, const gchar *hostname);
static void cb_disconnected (GtkWidget *emitter, const gchar *hostname);
static void cb_raw_server_output (GtkWidget *emitter, const gchar *text);
static void cb_login (GtkWidget *emitter, const gchar *hostname);
static void cb_logged_in (GtkWidget *emitter, const gchar *hostname);

G_MODULE_EXPORT void on_invite_player_menu_item_activate (GtkObject *object, 
                                                          gpointer user_data);
G_MODULE_EXPORT void on_watch_player_menu_item_activate (GtkObject *object, 
                                                         gpointer user_data);
G_MODULE_EXPORT void on_look_player_menu_item_activate (GtkObject *object, 
                                                        gpointer user_data);

static gboolean view_on_button_pressed (GtkTreeView *view,
                                        GdkEventButton *event,
                                        gpointer user_data);
static void view_player_popup_menu (GtkTreeView *view, GdkEventButton *event, 
                                    gpointer userdata);

static void create_player_view (GtkBuilder *builder);
static void create_player_menu (GtkBuilder *builder);

static void print2digits (GtkTreeViewColumn *tree_column,
                          GtkCellRenderer *cell, GtkTreeModel *tree_model,
                          GtkTreeIter *iter, gpointer data);
static gboolean setup_server_communication (GtkBuilder *builder);
static gboolean init_prefs (void);

static struct GibbonPosition initial_position;

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
        extern GibbonApp *app;
        GtkWidget *window = gibbon_app_get_window (app);

        va_start (args, message_format);
        message = g_strdup_vprintf (message_format, args);        
        va_end (args);
        
        GtkWidget *dialog = 
                gtk_message_dialog_new (GTK_WINDOW (window),
                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                        GTK_MESSAGE_ERROR,
                                        GTK_BUTTONS_CLOSE,
                                        "%s", message);
        
        g_free (message);
        
        gtk_dialog_run (GTK_DIALOG (dialog));

        gtk_widget_destroy (GTK_WIDGET (dialog));
}

void
display_info (const gchar *message_format, ...)
{
        va_list args;
        gchar *message;
        extern GibbonApp *app;
        GtkWidget *window = gibbon_app_get_window (app);

        va_start (args, message_format);
        message = g_strdup_vprintf (message_format, args);
        va_end (args);

        GtkWidget *dialog =
                gtk_message_dialog_new (GTK_WINDOW (window),
                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                        GTK_MESSAGE_INFO,
                                        GTK_BUTTONS_CLOSE,
                                        "%s", message);

        g_free (message);

        gtk_dialog_run (GTK_DIALOG (dialog));

        gtk_widget_destroy (GTK_WIDGET (dialog));
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
        gtk_widget_set_sensitive (GTK_WIDGET (connect_item), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (disconnect_button), TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET (disconnect_item), TRUE);
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
        gtk_widget_set_sensitive (GTK_WIDGET (connect_item), TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET (disconnect_button), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (disconnect_item), FALSE);
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
        gibbon_player_list_clear (players);
        
        gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 0);
        gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, _("Disconnected"));
        
        set_state_disconnected ();
}

static void
cb_raw_server_output (GtkWidget *emitter, const gchar *text)
{
        GtkTextBuffer *buffer = 
                gtk_text_view_get_buffer (GTK_TEXT_VIEW (server_text_view));
        
        gtk_text_buffer_insert_at_cursor (buffer, text, -1);
        gtk_text_buffer_insert_at_cursor (buffer, "\n", -1);
        
        gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (server_text_view),
                gtk_text_buffer_get_insert (buffer),
                0.0, TRUE, 0.5, 1);
}

G_MODULE_EXPORT void html_server_output_cb (GObject *emitter, 
                                            const gchar *html)
{
        GtkTextBuffer *buffer = 
                gtk_text_view_get_buffer (GTK_TEXT_VIEW (server_text_view));
        
        gtk_text_buffer_insert_at_cursor (buffer, html, -1);
        gtk_text_buffer_insert_at_cursor (buffer, "\n", -1);
        
        gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (server_text_view),
                gtk_text_buffer_get_insert (buffer),
                0.0, TRUE, 0.5, 1);
}

G_MODULE_EXPORT void
on_look_player_menu_item_activate (GtkObject *object, gpointer user_data)
{
        GtkTreeView *view = GTK_TREE_VIEW (user_data);
        GtkTreeSelection *selection;
        gint num_rows;
        GList *selected_rows;
        GList *first;
        GtkTreePath *path;
        GtkTreeModel *model;
        GtkTreeIter iter;
        gchar *who;
                
        selection = gtk_tree_view_get_selection (view);
        num_rows = gtk_tree_selection_count_selected_rows (selection);

        /* Should actually not happen.  */        
        if (num_rows != 1)
                return;
        
        selected_rows = gtk_tree_selection_get_selected_rows (selection, NULL);
        if (!selected_rows)
                return;
        
        first = g_list_first (selected_rows);
        if (first && first->data) {
                path = (GtkTreePath *) first->data;
                model = gtk_tree_view_get_model (view);
                
                if (gtk_tree_model_get_iter (model, &iter, path)) {
                        gtk_tree_model_get (model, &iter,
                                            GIBBON_PLAYER_LIST_COL_NAME, &who,
                                            -1);
                        
                        gibbon_connection_queue_command (connection,
                                                         "look %s", who);  
                }
        }
        
        g_list_foreach (selected_rows, (GFunc) gtk_tree_path_free, NULL);        
        g_list_free (selected_rows);
}

G_MODULE_EXPORT void
on_watch_player_menu_item_activate (GtkObject *object, gpointer user_data)
{
        GtkTreeView *view = GTK_TREE_VIEW (user_data);
        GtkTreeSelection *selection;
        gint num_rows;
        GList *selected_rows;
        GList *first;
        GtkTreePath *path;
        GtkTreeModel *model;
        GtkTreeIter iter;
        gchar *who;
                
        selection = gtk_tree_view_get_selection (view);
        num_rows = gtk_tree_selection_count_selected_rows (selection);

        /* Should actually not happen.  */        
        if (num_rows != 1)
                return;
        
        selected_rows = gtk_tree_selection_get_selected_rows (selection, NULL);
        if (!selected_rows)
                return;
        
        first = g_list_first (selected_rows);
        if (first && first->data) {
                path = (GtkTreePath *) first->data;
                model = gtk_tree_view_get_model (view);
                
                if (gtk_tree_model_get_iter (model, &iter, path)) {
                        gtk_tree_model_get (model, &iter,
                                            GIBBON_PLAYER_LIST_COL_NAME, &who,
                                            -1);
                        
                        gibbon_connection_queue_command (connection,
                                                         "watch %s", who);  
                        gibbon_connection_queue_command (connection,
                                                         "board");
                }
        }
        
        g_list_foreach (selected_rows, (GFunc) gtk_tree_path_free, NULL);        
        g_list_free (selected_rows);
}

G_MODULE_EXPORT void
on_invite_player_menu_item_activate (GtkObject *object, gpointer user_data)
{

}

void
set_position (const struct GibbonPosition *pos)
{
        gibbon_cairoboard_set_position (board, pos);
}

G_MODULE_EXPORT void
on_edit_menu_item_activate (GtkObject *object, gpointer user_data)
{
}

/* This method is mostly here to make valgrind happy.  */
void
cleanup_gui ()
{

}

GObject *
find_object (GtkBuilder *builder, const gchar *id, GType type)
{
        GObject *obj;
        GType got_type;

        g_return_val_if_fail (GTK_IS_BUILDER (builder), NULL);
        g_return_val_if_fail (G_TYPE_IS_OBJECT (type), NULL);

        obj = gtk_builder_get_object (builder, id);

        if (!obj) {
                /* TRANSLATORS: UI means user interface.  */
                display_error (_("Object `%s' not found in UI definition!"),
                                id);
                return NULL;
        }

        if (!G_IS_OBJECT (obj)) {
                display_error (_("Object `%s' is not a GObject!"),
                               id);
                return NULL;
        }

        got_type = G_OBJECT_TYPE (obj);
        if (type != got_type) {
                display_error (_("Object `%s' is not of type `%s' but `%s'!"),
                               id, g_type_name (type), g_type_name (got_type));
                return NULL;
        }

        return obj;
}

GtkImage *load_scaled_image (const gchar *path,
                             gint width, gint height)
{
        GError *error = NULL;
        GdkPixbuf *pixbuf;
        GtkImage *image;

        pixbuf = gdk_pixbuf_new_from_file_at_scale (path,
                                                    width,
                                                    height,
                                                    FALSE,
                                                    &error);

        if (!pixbuf) {
                display_error (_("Error loading image `%s': %s!"),
                               path, error->message);
                return NULL;
        }

        image = GTK_IMAGE (gtk_image_new ());
        gtk_image_set_from_pixbuf (image, pixbuf);
        g_object_unref (pixbuf);

        gtk_widget_show (GTK_WIDGET (image));
        return image;
}

static void
cb_server_command_fired (gpointer obj, GtkEntry *entry)
{
        gchar *trimmed;

        trimmed = pango_trim_string (gtk_entry_get_text (entry));

        gibbon_connection_queue_command (connection, "%s", trimmed);

        g_free (trimmed);

        gtk_entry_set_text (entry, "");
}

static gboolean
setup_server_communication (GtkBuilder *builder)
{
        GtkEntry *entry =
                GTK_ENTRY (find_object (builder, "server-command-entry",
                                        GTK_TYPE_ENTRY));

        if (!entry)
                return FALSE;

        g_signal_connect_swapped (entry, "activate",
                                  G_CALLBACK (cb_server_command_fired), NULL);

        return TRUE;
}

static gboolean
init_prefs (void)
{
        GtkEntry *entry;
        GtkToggleButton *toggle;
        gboolean save_password;

        prefs = gibbon_prefs_new ();
        if (!prefs)
                return FALSE;

        entry = GTK_ENTRY (find_object (builder, "conn_entry_server",
                                        GTK_TYPE_ENTRY));
        gibbon_prefs_string_update_entry (prefs, entry,
                                          GIBBON_PREFS_HOST);
        entry = GTK_ENTRY (find_object (builder, "conn_entry_login",
                                        GTK_TYPE_ENTRY));
        gibbon_prefs_string_update_entry (prefs, entry,
                                          GIBBON_PREFS_LOGIN);
        entry = GTK_ENTRY (find_object (builder, "conn_entry_address",
                                        GTK_TYPE_ENTRY));
        gibbon_prefs_string_update_entry (prefs, entry,
                                          GIBBON_PREFS_MAIL_ADDRESS);
        toggle = GTK_TOGGLE_BUTTON (find_object (builder,
                                                 "conn_checkbutton_remember",
                                                 GTK_TYPE_CHECK_BUTTON));
        gibbon_prefs_boolean_update_toggle_button (prefs, toggle,
                                            GIBBON_PREFS_SAVE_PASSWORD);

        save_password = gibbon_prefs_get_boolean (prefs,
                                                  GIBBON_PREFS_SAVE_PASSWORD);

        if (!save_password)
                gibbon_prefs_set_string (prefs,
                                         GIBBON_PREFS_PASSWORD,
                                         NULL);

        entry = GTK_ENTRY (find_object (builder, "conn_entry_password",
                                        GTK_TYPE_ENTRY));
        gibbon_prefs_string_update_entry (prefs, entry,
                                          GIBBON_PREFS_PASSWORD);

        return TRUE;
}
