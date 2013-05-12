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
 * SECTION:gibbon-game-list-view
 * @short_description: Game list combo.
 *
 * Since: 0.2.0
 *
 * The combo with the list of games in the moves tab.
 */

#include <gtk/gtk.h>

#include "gibbon-game-list-view.h"
#include "gibbon-app.h"

typedef struct _GibbonGameListViewPrivate GibbonGameListViewPrivate;
struct _GibbonGameListViewPrivate {
        GibbonMatchList *match_list;
        GtkComboBox *combo;

        GtkWidget *game_back;
        GtkWidget *game_forward;
};

#define GIBBON_GAME_LIST_VIEW_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_GAME_LIST_VIEW, GibbonGameListViewPrivate))

G_DEFINE_TYPE (GibbonGameListView, gibbon_game_list_view, G_TYPE_OBJECT)

static void gibbon_game_list_view_on_change (const GibbonGameListView *self);
static void gibbon_game_list_view_on_select (const GibbonGameListView *self);
static void gibbon_game_list_view_set_state (const GibbonGameListView *self);
static void gibbon_game_list_view_back (const GibbonGameListView *self);
static void gibbon_game_list_view_forward (const GibbonGameListView *self);

static void
gibbon_game_list_view_init (GibbonGameListView *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_GAME_LIST_VIEW, GibbonGameListViewPrivate);

        self->priv->combo = NULL;
        self->priv->match_list = NULL;

        self->priv->game_back = NULL;
        self->priv->game_forward = NULL;
}

static void
gibbon_game_list_view_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_game_list_view_parent_class)->finalize(object);
}

static void
gibbon_game_list_view_class_init (GibbonGameListViewClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonGameListViewPrivate));

        object_class->finalize = gibbon_game_list_view_finalize;
}

/**
 * gibbon_game_list_view_new:
 * @combo: The #GtkComboBox.
 * @match_list: The #GibbonMatchList .
 *
 * Creates a new #GibbonGameListView.
 *
 * Returns: The newly created #GibbonGameListView or %NULL in case of failure.
 */
GibbonGameListView *
gibbon_game_list_view_new (GtkComboBox *combo,
                           GibbonMatchList *match_list)
{
        GibbonGameListView *self = g_object_new (GIBBON_TYPE_GAME_LIST_VIEW,
                                                 NULL);
        GtkCellRenderer *cell;
        GtkListStore *model = gibbon_match_list_get_games_store (match_list);
        GObject *obj;

        self->priv->combo = combo;
        self->priv->match_list = match_list;

        gtk_combo_box_set_model (combo, GTK_TREE_MODEL (model));

        gtk_cell_layout_clear (GTK_CELL_LAYOUT (combo));
        cell = gtk_cell_renderer_text_new ();
        gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
        gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), cell,
                                        "text", 0,
                                        NULL);

        obj = gibbon_app_find_object (app, "board-game-back",
                                      GTK_TYPE_TOOL_BUTTON);
        self->priv->game_back = GTK_WIDGET (obj);
        g_signal_connect_swapped (obj, "clicked",
                                  (GCallback) gibbon_game_list_view_back,
                                  self);

        obj = gibbon_app_find_object (app, "board-next-game",
                                      GTK_TYPE_TOOL_BUTTON);
        self->priv->game_forward = GTK_WIDGET (obj);
        g_signal_connect_swapped (obj, "clicked",
                                  (GCallback) gibbon_game_list_view_forward,
                                  self);

        g_signal_connect_swapped (G_OBJECT (model), "row-inserted",
                                  (GCallback) gibbon_game_list_view_on_change,
                                  self);
        g_signal_connect_swapped (G_OBJECT (model), "row-deleted",
                                  (GCallback) gibbon_game_list_view_on_change,
                                  self);
        g_signal_connect_swapped (G_OBJECT (combo), "changed",
                                  (GCallback) gibbon_game_list_view_on_select,
                                  self);

        gibbon_game_list_view_set_state (self);

        return self;
}

static void
gibbon_game_list_view_on_change (const GibbonGameListView *self)
{
        GtkTreeModel *model;
        gsize num_items;

        g_return_if_fail (GIBBON_IS_GAME_LIST_VIEW (self));

        model = gtk_combo_box_get_model (self->priv->combo);
        num_items = gtk_tree_model_iter_n_children (model, NULL);
        gtk_combo_box_set_active (self->priv->combo, num_items - 1);

        gibbon_game_list_view_set_state (self);
}

static void
gibbon_game_list_view_on_select (const GibbonGameListView *self)
{
        gint active;

        g_return_if_fail (GIBBON_IS_GAME_LIST_VIEW (self));

        active = gtk_combo_box_get_active (self->priv->combo);

        gibbon_match_list_set_active_game (self->priv->match_list, active);

        gibbon_game_list_view_set_state (self);
}


static void
gibbon_game_list_view_set_state (const GibbonGameListView *self)
{
        GtkTreeModel *model;
        gsize num_items;
        gint active;

        model = gtk_combo_box_get_model (self->priv->combo);
        num_items = gtk_tree_model_iter_n_children (model, NULL);
        active = gtk_combo_box_get_active (self->priv->combo);

        if (!num_items || active < 0) {
                gtk_widget_set_sensitive (GTK_WIDGET (self->priv->game_back),
                                          FALSE);
                gtk_widget_set_sensitive (GTK_WIDGET (self->priv->game_forward),
                                          FALSE);
                return;
        }

        if (active == num_items - 1) {
                gtk_widget_set_sensitive (GTK_WIDGET (self->priv->game_forward),
                                          FALSE);
        } else {
                gtk_widget_set_sensitive (GTK_WIDGET (self->priv->game_forward),
                                          TRUE);
        }

        if (active == 0) {
                gtk_widget_set_sensitive (GTK_WIDGET (self->priv->game_back),
                                          FALSE);
        } else {
                gtk_widget_set_sensitive (GTK_WIDGET (self->priv->game_back),
                                          TRUE);
        }
}

static void
gibbon_game_list_view_back (const GibbonGameListView *self)
{
        gint active;

        g_return_if_fail (GIBBON_IS_GAME_LIST_VIEW (self));

        active = gtk_combo_box_get_active (self->priv->combo);

        if (active < 1)
                return;

        gtk_combo_box_set_active (self->priv->combo, active - 1);
}

static void
gibbon_game_list_view_forward (const GibbonGameListView *self)
{
        GtkTreeModel *model;
        gsize num_items;
        gint active;

        g_return_if_fail (GIBBON_IS_GAME_LIST_VIEW (self));

        model = gtk_combo_box_get_model (self->priv->combo);
        num_items = gtk_tree_model_iter_n_children (model, NULL);
        active = gtk_combo_box_get_active (self->priv->combo);

        if (active == num_items - 1)
                return;

        gtk_combo_box_set_active (self->priv->combo, active + 1);
}
