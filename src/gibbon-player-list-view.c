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
 * SECTION:gibbon-player-list-view
 * @short_description: Visual representation of the players list!
 *
 * Since: 0.1.0
 *
 * View for the Gibbon player list.
 **/

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "gibbon-player-list-view.h"
#include "gibbon-signal.h"
#include "gibbon-connection.h"
#include "gibbon-reliability-renderer.h"
#include "gibbon-player-list.h"
#include "gibbon-reliability.h"
#include "gibbon-session.h"
#include "gibbon-saved-info.h"
#include "gibbon-settings.h"

typedef struct _GibbonPlayerListViewPrivate GibbonPlayerListViewPrivate;
struct _GibbonPlayerListViewPrivate {
        GibbonApp *app;
        GibbonPlayerList *players;
        GtkTreeView *players_view;
        GtkMenu *player_menu;

        GibbonSignal *button_pressed_handler;

        GibbonSignal *invite_handler;
        GibbonSignal *look_handler;
        GibbonSignal *watch_handler;
        GibbonSignal *tell_handler;
        GibbonSignal *row_activated_handler;

        GtkTreeViewColumn *available_column;
        GtkTreeViewColumn *client_column;
        GtkTreeViewColumn *reliability_column;
        GtkTreeViewColumn *country_column;
};

static int const match_lengths[] = {
                1, 2, 3, 5, 7, 9, 11, 13, 15, 17, 19,
                29, 39, 49, 59, 69, 79, 89, 99, 0
};

#define GIBBON_PLAYER_LIST_VIEW_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_PLAYER_LIST_VIEW, GibbonPlayerListViewPrivate))

G_DEFINE_TYPE (GibbonPlayerListView, gibbon_player_list_view, G_TYPE_OBJECT)

static gboolean gibbon_player_list_view_on_button_pressed (GibbonPlayerListView
                                                           *self,
                                                           GdkEventButton
                                                           *event);
static gchar *gibbon_player_list_view_row_name (const GibbonPlayerListView
                                                *self);
static void gibbon_player_list_view_on_invite (const GibbonPlayerListView *self);
static void gibbon_player_list_view_on_look (const GibbonPlayerListView *self);
static void gibbon_player_list_view_on_watch (const GibbonPlayerListView *self);
static void gibbon_player_list_view_on_tell (const GibbonPlayerListView *self);
static gboolean gibbon_player_list_view_on_query_tooltip (GtkWidget *widget,
                                                          gint x, gint y,
                                                          gboolean keyboard_tip,
                                                          GtkTooltip *tooltip,
                                                          gpointer _self);
static void gibbon_player_list_view_on_saved (GibbonPlayerListView *self,
                                              gchar *invitee, guint count,
                                              GtkWidget *spinner);

static void print2digits (GtkTreeViewColumn *tree_column,
                          GtkCellRenderer *cell, GtkTreeModel *tree_model,
                          GtkTreeIter *iter, gpointer data);

static void 
gibbon_player_list_view_init (GibbonPlayerListView *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_PLAYER_LIST_VIEW, GibbonPlayerListViewPrivate);

        self->priv->app = NULL;
        self->priv->players = NULL;
        self->priv->players_view = NULL;
        self->priv->player_menu = NULL;

        self->priv->button_pressed_handler = NULL;
        self->priv->invite_handler = NULL;
        self->priv->look_handler = NULL;
        self->priv->watch_handler = NULL;
        self->priv->tell_handler = NULL;
        self->priv->row_activated_handler = NULL;

        self->priv->available_column = NULL;
        self->priv->client_column = NULL;
        self->priv->reliability_column = NULL;
        self->priv->country_column = NULL;
}

static void
gibbon_player_list_view_finalize (GObject *object)
{
        GibbonPlayerListView *self = GIBBON_PLAYER_LIST_VIEW (object);

        if (self->priv->players)
                g_object_unref (self->priv->players);

        if (self->priv->player_menu)
                g_object_unref (self->priv->player_menu);

        if (self->priv->button_pressed_handler)
                g_object_unref (self->priv->button_pressed_handler);

        if (self->priv->invite_handler)
                g_object_unref (self->priv->invite_handler);

        if (self->priv->look_handler)
                g_object_unref (self->priv->look_handler);

        if (self->priv->watch_handler)
                g_object_unref (self->priv->watch_handler);

        if (self->priv->tell_handler)
                g_object_unref (self->priv->tell_handler);

        if (self->priv->row_activated_handler)
                g_object_unref (self->priv->row_activated_handler);

        G_OBJECT_CLASS (gibbon_player_list_view_parent_class)->finalize(object);
}

static void
gibbon_player_list_view_class_init (GibbonPlayerListViewClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonPlayerListViewPrivate));

        object_class->finalize = gibbon_player_list_view_finalize;
}

/**
 * gibbon_player_list_view_new:
 * @app: The #GibbonApp.
 * @players: The underlying #GibbonPlayerList.
 *
 * Creates a new #GibbonPlayerListView.
 *
 * Returns: The newly created #GibbonPlayerListView or %NULL in case of failure.
 */
GibbonPlayerListView *
gibbon_player_list_view_new (GibbonApp *app, GibbonPlayerList *players)
{
        GibbonPlayerListView *self = g_object_new (GIBBON_TYPE_PLAYER_LIST_VIEW,
                                                   NULL);
        GtkTreeView *view;
        GtkTreeViewColumn *col;
        GtkCellRenderer *renderer;
        GCallback callback;
        GObject *emitter;
        gint colno;

        self->priv->app = app;
        self->priv->players = players;
        g_object_ref (players);

        self->priv->players_view = view =
            GTK_TREE_VIEW (gibbon_app_find_object (app,
                                                   "player_view",
                                                   GTK_TYPE_TREE_VIEW));

        colno = gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Name"),
                gtk_cell_renderer_text_new (),
                "text", GIBBON_PLAYER_LIST_COL_NAME,
                "weight", GIBBON_PLAYER_LIST_COL_NAME_WEIGHT,
                NULL);
        col = gtk_tree_view_get_column (view, colno - 1);
        gtk_tree_view_column_set_clickable (col, TRUE);
        gtk_tree_view_column_set_sort_indicator (col, TRUE);
        gtk_tree_view_column_set_sort_order (col, GTK_SORT_ASCENDING);
        gtk_tree_view_column_set_sort_column_id (col,
                                                 GIBBON_PLAYER_LIST_COL_NAME);

        colno = gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Status"),
                gtk_cell_renderer_pixbuf_new (),
                "stock-id", GIBBON_PLAYER_LIST_COL_AVAILABLE,
                NULL);
        self->priv->available_column = gtk_tree_view_get_column (view,
                                                                 colno - 1);
        col = gtk_tree_view_get_column (view, colno - 1);
        gtk_tree_view_column_set_clickable (col, TRUE);
        gtk_tree_view_column_set_sort_indicator (col, TRUE);
        gtk_tree_view_column_set_sort_order (col, GTK_SORT_ASCENDING);
        gtk_tree_view_column_set_sort_column_id (
                        col, GIBBON_PLAYER_LIST_COL_AVAILABLE);

        colno = gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                NULL,
                gtk_cell_renderer_pixbuf_new (),
                "pixbuf", GIBBON_PLAYER_LIST_COL_COUNTRY_ICON,
                NULL);
        self->priv->country_column = gtk_tree_view_get_column (view, colno - 1);
        col = gtk_tree_view_get_column (view, colno - 1);
        gtk_tree_view_column_set_clickable (col, TRUE);
        gtk_tree_view_column_set_sort_indicator (col, TRUE);
        gtk_tree_view_column_set_sort_order (col, GTK_SORT_ASCENDING);
        gtk_tree_view_column_set_sort_column_id (
                        col, GIBBON_PLAYER_LIST_COL_COUNTRY);

        renderer = gtk_cell_renderer_text_new ();
        colno = gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Rating"),
                renderer,
                "text", GIBBON_PLAYER_LIST_COL_RATING,
                NULL);
        col = gtk_tree_view_get_column (view, colno - 1);
        gtk_tree_view_column_set_clickable (col, TRUE);
        gtk_tree_view_column_set_sort_indicator (col, TRUE);
        gtk_tree_view_column_set_sort_order (col, GTK_SORT_ASCENDING);
        gtk_tree_view_column_set_cell_data_func (col, renderer,
                print2digits, (gpointer) GIBBON_PLAYER_LIST_COL_RATING, NULL);
        gtk_tree_view_column_set_sort_column_id (
                        col, GIBBON_PLAYER_LIST_COL_RATING);

        colno = gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Exp."),
                gtk_cell_renderer_text_new (),
                "text", GIBBON_PLAYER_LIST_COL_EXPERIENCE,
                NULL);
        col = gtk_tree_view_get_column (view, colno - 1);
        gtk_tree_view_column_set_clickable (col, TRUE);
        gtk_tree_view_column_set_sort_indicator (col, TRUE);
        gtk_tree_view_column_set_sort_order (col, GTK_SORT_ASCENDING);
        gtk_tree_view_column_set_sort_column_id (
                        col, GIBBON_PLAYER_LIST_COL_EXPERIENCE);

        colno = gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                NULL,
                gtk_cell_renderer_pixbuf_new (),
                "pixbuf", GIBBON_PLAYER_LIST_COL_CLIENT_ICON,
                NULL);
        self->priv->client_column = gtk_tree_view_get_column (view, colno - 1);
        col = gtk_tree_view_get_column (view, colno - 1);
        gtk_tree_view_column_set_clickable (col, TRUE);
        gtk_tree_view_column_set_sort_indicator (col, TRUE);
        gtk_tree_view_column_set_sort_order (col, GTK_SORT_ASCENDING);
        gtk_tree_view_column_set_sort_column_id (
                        col, GIBBON_PLAYER_LIST_COL_CLIENT);

        renderer = gibbon_reliability_renderer_new ();
        colno = gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Reliability"),
                renderer,
                "reliability", GIBBON_PLAYER_LIST_COL_RELIABILITY,
                NULL);
        self->priv->reliability_column =
                        gtk_tree_view_get_column (view, colno - 1);
        col = gtk_tree_view_get_column (view, colno - 1);
        gtk_tree_view_column_set_clickable (col, TRUE);
        gtk_tree_view_column_set_sort_indicator (col, TRUE);
        gtk_tree_view_column_set_sort_order (col, GTK_SORT_ASCENDING);
        gtk_tree_view_column_set_sort_column_id (
                        col, GIBBON_PLAYER_LIST_COL_RELIABILITY);

        gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Opponent"),
                gtk_cell_renderer_text_new (),
                "text", GIBBON_PLAYER_LIST_COL_OPPONENT,
                NULL);
        gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Watching"),
                gtk_cell_renderer_text_new (),
                "text", GIBBON_PLAYER_LIST_COL_WATCHING,
                NULL);
        gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Address"),
                gtk_cell_renderer_text_new (),
                "text", GIBBON_PLAYER_LIST_COL_EMAIL,
                NULL);

        g_object_set (G_OBJECT (view), "has-tooltip", TRUE, NULL);

        gibbon_player_list_connect_view (self->priv->players, view);
        callback = (GCallback) gibbon_player_list_view_on_button_pressed;

        self->priv->button_pressed_handler =
                 gibbon_signal_new (G_OBJECT (view), "button-press-event",
                                    callback, G_OBJECT (self));

        emitter = gibbon_app_find_object (app, "invite_player_menu_item",
                                          GTK_TYPE_MENU_ITEM);
        callback = (GCallback) gibbon_player_list_view_on_invite;
        self->priv->invite_handler =
                 gibbon_signal_new (emitter, "activate",
                                    callback, G_OBJECT (self));

        emitter = gibbon_app_find_object (app, "look_player_menu_item",
                                          GTK_TYPE_MENU_ITEM);
        callback = (GCallback) gibbon_player_list_view_on_look;
        self->priv->look_handler =
                 gibbon_signal_new (emitter, "activate",
                                    callback, G_OBJECT (self));

        emitter = gibbon_app_find_object (app, "watch_player_menu_item",
                                          GTK_TYPE_MENU_ITEM);
        callback = (GCallback) gibbon_player_list_view_on_watch;
        self->priv->watch_handler =
                 gibbon_signal_new (emitter, "activate",
                                    callback, G_OBJECT (self));

        emitter = gibbon_app_find_object (app, "tell-player-menu-item",
                                          GTK_TYPE_MENU_ITEM);
        callback = (GCallback) gibbon_player_list_view_on_tell;
        self->priv->tell_handler =
                 gibbon_signal_new (emitter, "activate",
                                    callback, G_OBJECT (self));

        callback = (GCallback) gibbon_player_list_view_on_query_tooltip;
        (void) g_signal_connect (GTK_WIDGET (view), "query-tooltip",
                                 callback, self);

        return self;
}

static void
print2digits (GtkTreeViewColumn *tree_column,
              GtkCellRenderer *cell, GtkTreeModel *tree_model,
              GtkTreeIter *iter, gpointer data)
{
        GtkCellRendererText *cell_text = (GtkCellRendererText *) cell;
        gchar *text;
        gdouble d;

        g_object_get (cell_text,
                      "text", &text,
                      NULL);

        gtk_tree_model_get (tree_model, iter, GPOINTER_TO_INT (data), &d, -1);
        text = g_strdup_printf ("%.2f", d);
        g_object_set (cell_text,
                      "text", text,
                      NULL);
        g_free (text);
}

static gboolean
gibbon_player_list_view_on_button_pressed (GibbonPlayerListView *self,
                                           GdkEventButton *event)
{
        GtkTreeSelection *selection;
        GtkTreePath *path;
        GtkTreeView *view;
        GObject *player_menu;
        gchar *who;

        if (event->type == GDK_2BUTTON_PRESS && event->button == 1) {
                gibbon_player_list_view_on_invite (self);
                return TRUE;
        }

        if (event->type != GDK_BUTTON_PRESS  ||  event->button != 3)
                return FALSE;

        view = self->priv->players_view;

        selection = gtk_tree_view_get_selection (view);
        if (gtk_tree_selection_count_selected_rows (selection)  <= 1) {
                if (gtk_tree_view_get_path_at_pos(view, event->x, event->y,
                                                  &path, NULL, NULL, NULL)) {
                        gtk_tree_selection_unselect_all(selection);
                        gtk_tree_selection_select_path(selection, path);
                        gtk_tree_path_free(path);
                }
        }

        player_menu = gibbon_app_find_object (self->priv->app, "player_menu",
                                              GTK_TYPE_MENU);
        who = gibbon_player_list_view_row_name (self);
        if (!who)
                return TRUE;
        gibbon_app_configure_player_menu (self->priv->app, who,
                                          GTK_MENU (player_menu));
        g_free (who);

        gtk_widget_show_all (GTK_WIDGET (player_menu));

        gtk_menu_popup (GTK_MENU (player_menu),
                        NULL, NULL, NULL, NULL,
                        (event != NULL) ? event->button : 0,
                           gdk_event_get_time ((GdkEvent*) event));

        return TRUE;
}

static gchar *
gibbon_player_list_view_row_name (const GibbonPlayerListView *self)
{
        GtkTreeSelection *selection;
        gint num_rows;
        GList *selected_rows;
        GList *first;
        GtkTreePath *path;
        GtkTreeModel *model;
        GtkTreeIter iter;
        gchar *who = NULL;

        g_return_val_if_fail (GIBBON_IS_PLAYER_LIST_VIEW (self), NULL);

        selection = gtk_tree_view_get_selection (self->priv->players_view);
        num_rows = gtk_tree_selection_count_selected_rows (selection);

        /* Should actually not happen.  */
        if (num_rows != 1)
                return NULL;

        selected_rows = gtk_tree_selection_get_selected_rows (selection, NULL);
        if (!selected_rows)
                return NULL;

        first = g_list_first (selected_rows);
        if (first && first->data) {
                path = (GtkTreePath *) first->data;
                model = gtk_tree_view_get_model (self->priv->players_view);

                if (gtk_tree_model_get_iter (model, &iter, path)) {
                        gtk_tree_model_get (model, &iter,
                                            GIBBON_PLAYER_LIST_COL_NAME, &who,
                                            -1);
                }
        }

        g_list_foreach (selected_rows, (GFunc) gtk_tree_path_free, NULL);
        g_list_free (selected_rows);

        return who;
}

static void
gibbon_player_list_view_on_invite (const GibbonPlayerListView *self)
{
        gchar *who;
        GibbonConnection *connection;
        GtkWidget *main_window;
        GtkWidget *dialog;
        gint reply;
        GibbonSession *session;
        const GibbonSavedInfo *saved_info;
        GtkWidget *combo;
        gint selected = -1;
        GSettings *settings;
        guint pref_length;
        gchar *length_string;
        gint length;
        gint i;
        gchar *message;
        GtkWidget *label;
        GtkWidget *hbox;
        GtkWidget *spinner;
        GtkWidget *dialog_vbox;

        g_return_if_fail (GIBBON_IS_PLAYER_LIST_VIEW (self));

        who = gibbon_player_list_view_row_name (self);
        if (!who)
                return;

        connection = gibbon_app_get_connection (self->priv->app);

        /* We could have got here via double-click or row activation.  */
        if (!g_strcmp0 (gibbon_connection_get_login (connection), who))
                return;
        if (!gibbon_player_list_get_available (self->priv->players, who))
                return;

        main_window = gibbon_app_get_window (self->priv->app);
        session = gibbon_app_get_session (self->priv->app);
        saved_info = gibbon_session_get_saved (session, who);

        if (saved_info && !saved_info->match_length) {
                dialog = gtk_message_dialog_new_with_markup(
                                GTK_WINDOW (main_window),
                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                GTK_MESSAGE_ERROR,
                                GTK_BUTTONS_NONE,
                                "<span weight='bold' size='larger'>"
                                "%s</span>\n%s",
                                _("New match or resume?"),
                                _("You still have a saved match of unlimited"
                                  " length with that player.  You can either"
                                  " resume your saved match or start a new"
                                  " match.  The latter will terminate your"
                                  " saved match.\n"));
                gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                                        _("Cancel"),
                                        GTK_RESPONSE_CANCEL,
                                        _("Resume unlimited match"), 1,
                                        _("Start new match"), 2,
                                        NULL);
                reply = gtk_dialog_run (GTK_DIALOG (dialog));
                gtk_widget_destroy (dialog);
                if (reply == GTK_RESPONSE_CANCEL) {
                        return;
                } else if (reply == 1) {
                        gibbon_connection_queue_command (connection, FALSE,
                                                         "invite %s", who);
                        return;
                } else {
                        saved_info = NULL;
                }
        }

        if (saved_info) {
                dialog = gtk_message_dialog_new (GTK_WINDOW (main_window),
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_MESSAGE_QUESTION,
                                         GTK_BUTTONS_YES_NO,
                                         _("Invite %s to resume"
                                           " your saved match?"), who);
                reply = gtk_dialog_run (GTK_DIALOG (dialog));
                gtk_widget_destroy (dialog);

                if (reply == GTK_RESPONSE_YES)
                        gibbon_connection_queue_command (connection, FALSE,
                                                         "invite %s", who);
        } else {
                message = g_strdup_printf (_("Invite %s"), who);
                dialog = gtk_dialog_new_with_buttons (message,
                                                      GTK_WINDOW (main_window),
                                                      GTK_DIALOG_MODAL
                                                      | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                      GTK_STOCK_CANCEL,
                                                      GTK_RESPONSE_REJECT,
                                                      GTK_STOCK_OK,
                                                      GTK_RESPONSE_ACCEPT,
                                                      NULL);
                g_free (message);

                label = gtk_label_new (NULL);
                message = g_strdup_printf (_("<b>Invite %s</b>"), who);
                gtk_label_set_markup (GTK_LABEL (label), message);
                g_free (message);

                dialog_vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
                gtk_box_pack_start (GTK_BOX (dialog_vbox),
                                    label, TRUE, TRUE, 10);

                hbox = gtk_hbox_new (FALSE, 5);
                gtk_box_pack_start (GTK_BOX (dialog_vbox),
                                    hbox, TRUE, TRUE, 10);
                gtk_box_pack_start (GTK_BOX (hbox),
                                    gtk_label_new (_("Number of saved"
                                                     " matches:")),
                                    TRUE, TRUE, 5);

                spinner = gtk_spinner_new ();
                gtk_box_pack_start (GTK_BOX (hbox), spinner, TRUE, TRUE, 5);
                gtk_spinner_start (GTK_SPINNER (spinner));

                hbox = gtk_hbox_new (FALSE, 5);
                gtk_box_pack_start (GTK_BOX (dialog_vbox),
                                    hbox, TRUE, TRUE, 10);
                gtk_box_pack_start (GTK_BOX (hbox),
                                    gtk_label_new (_("Match length:")),
                                    TRUE, TRUE, 10);
                combo = gtk_combo_box_text_new ();
                gtk_box_pack_start (GTK_BOX (hbox),
                                    combo, TRUE, TRUE, 0);

                settings = g_settings_new (GIBBON_PREFS_MATCH_SCHEMA);

                pref_length = gibbon_settings_get_uint (settings,
                                                     GIBBON_PREFS_MATCH_LENGTH);
                g_object_unref (settings);

                for (i = 0;
                     i < (sizeof match_lengths) / (sizeof match_lengths[0]);
                     ++i) {
                        length = match_lengths[i];
                        if (!length) {
                                length_string = g_strdup (_("unlimited"));
                        } else {
                                length_string = g_strdup_printf ("%d", length);
                        }
                        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo),
                                                        length_string);
                        g_free (length_string);
                        if (pref_length == length)
                                g_object_set (G_OBJECT (combo), "active", i,
                                              NULL);
                }

                gibbon_session_get_saved_count (
                                session, g_strdup (who),
                                (GibbonSessionCallback)
                                gibbon_player_list_view_on_saved,
                                G_OBJECT (self), spinner);

                gtk_widget_show_all (dialog);

                reply = gtk_dialog_run (GTK_DIALOG (dialog));

                if (reply != GTK_RESPONSE_ACCEPT) {
                        gtk_widget_destroy (dialog);
                        g_free (who);
                        return;
                }

                g_object_get (G_OBJECT (combo), "active", &selected, NULL);
                if (selected < 0
                    || selected >= (sizeof match_lengths)
                            / (sizeof match_lengths[0])) {
                        gtk_widget_destroy (dialog);
                        g_free (who);
                        return;
                }

                length = match_lengths[selected];
                if (!length) {
                        length_string = g_strdup ("unlimited");
                } else {
                        length_string = g_strdup_printf ("%d", length);
                }

                gibbon_connection_queue_command (connection, FALSE,
                                                 "invite %s %s\n",
                                                 who, length_string);
                g_free (length_string);

                settings = g_settings_new (GIBBON_PREFS_MATCH_SCHEMA);
                gibbon_settings_set_uint (settings, GIBBON_PREFS_MATCH_LENGTH,
                                          length);
                g_object_unref (settings);

                gtk_widget_destroy (dialog);
        }

        g_free (who);
}

static void
gibbon_player_list_view_on_look (const GibbonPlayerListView *self)
{
        gchar *who;
        GibbonConnection *connection;

        g_return_if_fail (GIBBON_IS_PLAYER_LIST_VIEW (self));

        who = gibbon_player_list_view_row_name (self);
        if (!who)
                return;

        connection = gibbon_app_get_connection (self->priv->app);
        gibbon_connection_queue_command (connection, FALSE,
                                         "look %s", who);

        g_free (who);
}

static void
gibbon_player_list_view_on_watch (const GibbonPlayerListView *self)
{
        gchar *who;
        GibbonConnection *connection;
        GibbonSession *session;
        const gchar *command;

        g_return_if_fail (GIBBON_IS_PLAYER_LIST_VIEW (self));

        who = gibbon_player_list_view_row_name (self);
        if (!who)
                return;

        session = gibbon_app_get_session (self->priv->app);
        if (0 == g_strcmp0 (gibbon_session_get_watching (session), who))
                command = "unwatch";
        else
                command = "watch";
        connection = gibbon_app_get_connection (self->priv->app);
        gibbon_connection_queue_command (connection, FALSE,
                                         "%s %s", command, who);
        gibbon_connection_queue_command (connection, FALSE, "board");

        g_free (who);
}

static void
gibbon_player_list_view_on_tell (const GibbonPlayerListView *self)
{
        gchar *whom;

        g_return_if_fail (GIBBON_IS_PLAYER_LIST_VIEW (self));

        whom = gibbon_player_list_view_row_name (self);
        if (!whom)
                return;

        gibbon_app_start_chat (self->priv->app, whom);

        g_free (whom);
}

static gboolean
gibbon_player_list_view_on_query_tooltip (GtkWidget *widget,
                                          gint x, gint y,
                                          gboolean keyboard_tip,
                                          GtkTooltip *tooltip,
                                          gpointer _self)
{
        GtkTreeIter iter;
        GtkTreeView *tree_view;
        GtkTreeModel *model;
        GtkTreePath *path = NULL;
        GtkTreeViewColumn *column = NULL;
        GibbonPlayerListView *self;
        gchar *player_name = NULL;
        gchar *available = NULL;
        gchar *text = NULL;
        GibbonReliability *rel;
        const gchar *rel_descr;
        const gchar *conf_descr;
        GibbonCountry *country;
        gchar *hostname;

        g_return_val_if_fail (GIBBON_IS_PLAYER_LIST_VIEW (_self), FALSE);
        self = GIBBON_PLAYER_LIST_VIEW (_self);
        g_return_val_if_fail (GTK_IS_TREE_VIEW (widget), FALSE);

        tree_view = GTK_TREE_VIEW (widget);
        model = gtk_tree_view_get_model (tree_view);

        if (!gtk_tree_view_get_tooltip_context (tree_view, &x, &y,
                                                keyboard_tip,
                                                &model, &path, &iter))
                return FALSE;
        gtk_tree_view_get_path_at_pos (tree_view, x, y, NULL,
                                       &column, NULL, NULL);
        if (!column)
                return FALSE;

        gtk_tree_model_get (model, &iter, 0, &player_name, -1);

        if (column == self->priv->available_column) {
                gtk_tree_model_get (model, &iter,
                                    GIBBON_PLAYER_LIST_COL_AVAILABLE,
                                    &available,
                                    -1);
                if (g_strcmp0 ("gtk-yes", available) == 0)
                        text = g_strdup_printf ("<i>%s</i> is ready to play.",
                                                player_name);
                else if (g_strcmp0 ("gtk-no", available) == 0)
                        text = g_strdup_printf ("<i>%s</i> is currently playing.",
                                                player_name);
                else
                        text = g_strdup_printf (_("<i>%s</i> does not want to"
                                                  " play right now."),
                                                player_name);
        } else if (column == self->priv->client_column) {
                gtk_tree_model_get (model, &iter,
                                    GIBBON_PLAYER_LIST_COL_CLIENT, &text,
                                    -1);
        } else if (column == self->priv->country_column) {
                gtk_tree_model_get (model, &iter,
                                    GIBBON_PLAYER_LIST_COL_COUNTRY, &country,
                                    GIBBON_PLAYER_LIST_COL_HOSTNAME, &hostname,
                                    -1);
                text = g_strdup_printf ("<b>%s</b>\n%s",
                                        gibbon_country_get_name (country),
                                        hostname);
                g_object_unref (country);
                g_free (hostname);
        } else if (column == self->priv->reliability_column) {
                gtk_tree_model_get (model, &iter,
                                    GIBBON_PLAYER_LIST_COL_RELIABILITY, &rel,
                                    -1);
                if (rel->confidence) {
                        if (rel->value > 0.95)
                                rel_descr = _("good");
                        else if (rel->value > 0.85)
                                rel_descr = ("okay");
                        else if (rel->value > 0.65)
                                rel_descr = ("poor");
                        else
                                rel_descr = _("keep away");
                        if (rel->confidence >= 10)
                                conf_descr = _("very certain");
                        else if (rel->confidence >= 5)
                                conf_descr = _("certain");
                        else if (rel->confidence >= 1)
                                conf_descr = _("uncertain");
                        else
                                conf_descr = _("unknown");

                        text = g_strdup_printf (_("<b>Reliability of player"
                                                  " <i>%s</i>:</b>\n"
                                                  " %f (%s) with a measurement"
                                                  " confidence "
                                                  " of %u (%s)."),
                                                player_name,
                                                rel->value, rel_descr,
                                                rel->confidence, conf_descr);
                } else {
                        text = g_strdup_printf (_("<b>Reliability of player"
                                                  " <i>%s</i></b>: unknown."),
                                                player_name);
                }
        } else {
                gtk_tree_path_free (path);
                g_free (player_name);
                return FALSE;
        }

        gtk_tooltip_set_markup (tooltip, text);

        gtk_tree_view_set_tooltip_row (tree_view, tooltip, path);

        gtk_tree_path_free (path);
        g_free (player_name);
        g_free (text);

        return TRUE;
}

static void
gibbon_player_list_view_on_saved (GibbonPlayerListView *self, gchar *invitee,
                                  guint count, GtkWidget *spinner)
{
        GtkWidget *hbox;
        gchar *text;

        if (!GIBBON_IS_PLAYER_LIST_VIEW (self))
                return;

        hbox = gtk_widget_get_parent (spinner);
        gtk_widget_destroy (spinner);

        text = g_strdup_printf ("%u", count);
        gtk_box_pack_start (GTK_BOX (hbox), gtk_label_new (text),
                            TRUE, TRUE, 5);
        g_free (text);
        gtk_widget_show_all (hbox);
}
