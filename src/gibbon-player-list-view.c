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
 * SECTION:gibbon-player-list-view
 * @short_description: Visual representation of the players list!
 *
 * Since: 0.1.1
 *
 * View for the Gibbon player list.
 **/

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "gibbon-player-list-view.h"
#include "gibbon-signal.h"

typedef struct _GibbonPlayerListViewPrivate GibbonPlayerListViewPrivate;
struct _GibbonPlayerListViewPrivate {
        const GibbonApp *app;
        GibbonPlayerList *players;
        GtkMenu *player_menu;

        GibbonSignal *button_pressed_handler;
};

#define GIBBON_PLAYER_LIST_VIEW_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_PLAYER_LIST_VIEW, GibbonPlayerListViewPrivate))

G_DEFINE_TYPE (GibbonPlayerListView, gibbon_player_list_view, G_TYPE_OBJECT)

static gboolean gibbon_player_list_view_on_button_pressed (GibbonPlayerListView
                                                           *self,
                                                           GdkEventButton
                                                           *event);

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
        self->priv->player_menu = NULL;

        self->priv->button_pressed_handler = NULL;
}

static void
gibbon_player_list_view_finalize (GObject *object)
{
        GibbonPlayerListView *self = GIBBON_PLAYER_LIST_VIEW (object);

        if (self->priv->players)
                g_object_unref (self->priv->players);
        self->priv->players = NULL;

        if (self->priv->player_menu)
                g_object_unref (self->priv->player_menu);
        self->priv->player_menu = NULL;

        self->priv->app = NULL;

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
 * @dummy: The argument.
 *
 * Creates a new #GibbonPlayerListView.
 *
 * Returns: The newly created #GibbonPlayerListView or %NULL in case of failure.
 */
GibbonPlayerListView *
gibbon_player_list_view_new (const GibbonApp *app, GibbonPlayerList *players)
{
        GibbonPlayerListView *self = g_object_new (GIBBON_TYPE_PLAYER_LIST_VIEW,
                                                   NULL);
        GtkTreeView *view;
        GtkTreeViewColumn *col;
        GtkCellRenderer *renderer;
        GCallback callback;

        self->priv->app = app;
        self->priv->players = players;
        g_object_ref (players);

        view = GTK_TREE_VIEW (gibbon_app_find_object (app, "player_view",
                                                      GTK_TYPE_TREE_VIEW));

        gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Name"),
                gtk_cell_renderer_text_new (),
                "text", GIBBON_PLAYER_LIST_COL_NAME,
                NULL);
        col = gtk_tree_view_get_column (view, GIBBON_PLAYER_LIST_COL_NAME);
        gtk_tree_view_column_set_clickable (col, TRUE);
        gtk_tree_view_column_set_sort_indicator (col, TRUE);
        gtk_tree_view_column_set_sort_order (col, GTK_SORT_ASCENDING);

        gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Available"),
                gtk_cell_renderer_text_new (),
                "text", GIBBON_PLAYER_LIST_COL_AVAILABLE,
                NULL);
        col = gtk_tree_view_get_column (view, GIBBON_PLAYER_LIST_COL_NAME);
        gtk_tree_view_column_set_clickable (col, TRUE);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Rating"),
                renderer,
                "text", GIBBON_PLAYER_LIST_COL_RATING,
                NULL);
        col = gtk_tree_view_get_column (view, GIBBON_PLAYER_LIST_COL_RATING);
        gtk_tree_view_column_set_clickable (col, TRUE);
        gtk_tree_view_column_set_cell_data_func (col, renderer,
                print2digits, (gpointer) GIBBON_PLAYER_LIST_COL_RATING, NULL);

        gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Exp."),
                gtk_cell_renderer_text_new (),
                "text", GIBBON_PLAYER_LIST_COL_EXPERIENCE,
                NULL);
        col = gtk_tree_view_get_column (view, GIBBON_PLAYER_LIST_COL_RATING);
        gtk_tree_view_column_set_clickable (col, TRUE);

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

        gibbon_player_list_connect_view (self->priv->players, view);
        callback = (GCallback) gibbon_player_list_view_on_button_pressed;

        self->priv->button_pressed_handler =
                 gibbon_signal_new (G_OBJECT (view), "button-press-event",
                                    callback, G_OBJECT (self));

        return self;
}

static void
print2digits (GtkTreeViewColumn *tree_column,
              GtkCellRenderer *cell, GtkTreeModel *tree_model,
              GtkTreeIter *iter, gpointer data)
{
        GtkCellRendererText *cell_text = (GtkCellRendererText *) cell;
        gdouble d;

        g_free (cell_text->text);
        gtk_tree_model_get (tree_model, iter, GPOINTER_TO_INT (data), &d, -1);
        cell_text->text = g_strdup_printf ("%.2f", d);
}

static gboolean
gibbon_player_list_view_on_button_pressed (GibbonPlayerListView *self,
                                           GdkEventButton *event)
{
        GtkTreeSelection *selection;
        GtkTreePath *path;
        GtkTreeView *view;
        GObject *player_menu;

        if (event->type != GDK_BUTTON_PRESS  ||  event->button != 3)
                return FALSE;

        view = GTK_TREE_VIEW (gibbon_app_find_object (self->priv->app,
                                                      "player_view",
                                                      GTK_TYPE_TREE_VIEW));

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
        gtk_widget_show_all (GTK_WIDGET (player_menu));

        gtk_menu_popup (GTK_MENU (player_menu),
                        NULL, NULL, NULL, NULL,
                        (event != NULL) ? event->button : 0,
                           gdk_event_get_time ((GdkEvent*) event));

        return TRUE;
}
