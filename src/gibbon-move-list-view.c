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
 * SECTION:gibbon-move-list-view
 * @short_description: Tree view for the move list
 *
 * Since: 0.2.0
 *
 * Handling of the move list in the move tab (below the game selection).
 */

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "gibbon-move-list-view.h"

typedef struct _GibbonMoveListViewPrivate GibbonMoveListViewPrivate;
struct _GibbonMoveListViewPrivate {
        GtkTreeView *view;
        GtkTreeModel *model;
        const GibbonMatchList *match_list;
};

#define GIBBON_MOVE_LIST_VIEW_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_MOVE_LIST_VIEW, GibbonMoveListViewPrivate))

G_DEFINE_TYPE (GibbonMoveListView, gibbon_move_list_view, G_TYPE_OBJECT)


static void gibbon_move_list_view_on_insert (const GibbonMoveListView *self);

static void gibbon_move_list_view_black_roll_data_func (GtkTreeViewColumn
                                                        *tree_column,
                                                        GtkCellRenderer *cell,
                                                        GtkTreeModel
                                                        *tree_model,
                                                        GtkTreeIter *iter,
                                                        GibbonMoveListView
                                                        *self);
static void gibbon_move_list_view_white_roll_data_func (GtkTreeViewColumn
                                                        *tree_column,
                                                        GtkCellRenderer *cell,
                                                        GtkTreeModel
                                                        *tree_model,
                                                        GtkTreeIter *iter,
                                                        GibbonMoveListView
                                                        *self);
static void gibbon_move_list_view_black_move_data_func (GtkTreeViewColumn
                                                        *tree_column,
                                                        GtkCellRenderer *cell,
                                                        GtkTreeModel
                                                        *tree_model,
                                                        GtkTreeIter *iter,
                                                        GibbonMoveListView
                                                        *self);
static void gibbon_move_list_view_white_move_data_func (GtkTreeViewColumn
                                                        *tree_column,
                                                        GtkCellRenderer *cell,
                                                        GtkTreeModel
                                                        *tree_model,
                                                        GtkTreeIter *iter,
                                                        GibbonMoveListView
                                                        *self);
static void gibbon_move_list_view_roll (GibbonMoveListView *self,
                                        GibbonPositionSide side,
                                        GtkCellRenderer *cell,
                                        GtkTreeModel *tree_model,
                                        GtkTreeIter *iter);
static void gibbon_move_list_view_move (GibbonMoveListView *self,
                                        GibbonPositionSide side,
                                        GtkCellRenderer *cell,
                                        GtkTreeModel *tree_model,
                                        GtkTreeIter *iter);

static void 
gibbon_move_list_view_init (GibbonMoveListView *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_MOVE_LIST_VIEW, GibbonMoveListViewPrivate);

        self->priv->view = NULL;
        self->priv->model = NULL;
        self->priv->match_list = NULL;
}

static void
gibbon_move_list_view_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_move_list_view_parent_class)->finalize(object);
}

static void
gibbon_move_list_view_class_init (GibbonMoveListViewClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonMoveListViewPrivate));

        object_class->finalize = gibbon_move_list_view_finalize;
}

/**
 * gibbon_move_list_view_new:
 * @view: The #GtkTreeView.a
 * @match_list: The #GibbonMatchList holding the match information.
 *
 * Creates a new #GibbonMoveListView.
 *
 * Returns: The newly created #GibbonMoveListView or %NULL in case of failure.
 */
GibbonMoveListView *
gibbon_move_list_view_new (GtkTreeView *view, const GibbonMatchList *match_list)
{
        GibbonMoveListView *self = g_object_new (GIBBON_TYPE_MOVE_LIST_VIEW,
                                                 NULL);
        GtkListStore *model;
        GtkTreeSelection *selection;

        self->priv->view = view;
        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
        gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);

        self->priv->match_list = match_list;

        gtk_tree_view_insert_column_with_attributes (view, -1, _("#"),
                        gtk_cell_renderer_text_new (),
                        "text", GIBBON_MATCH_LIST_COL_MOVENO,
                        NULL);
        gtk_tree_view_insert_column_with_data_func (view, -1, "  ",
                        gtk_cell_renderer_text_new (),
                        (GtkTreeCellDataFunc)
                        gibbon_move_list_view_black_roll_data_func,
                        self, NULL);
       gtk_tree_view_insert_column_with_data_func (view, -1, _("Black"),
                        gtk_cell_renderer_text_new (),
                        (GtkTreeCellDataFunc)
                        gibbon_move_list_view_black_move_data_func,
                        self, NULL);
       gtk_tree_view_insert_column_with_data_func (view, -1, "  ",
                        gtk_cell_renderer_text_new (),
                        (GtkTreeCellDataFunc)
                        gibbon_move_list_view_white_roll_data_func,
                        self, NULL);
       gtk_tree_view_insert_column_with_data_func (view, -1, _("White"),
                        gtk_cell_renderer_text_new (),
                        (GtkTreeCellDataFunc)
                        gibbon_move_list_view_white_move_data_func,
                        self, NULL);

        model = gibbon_match_list_get_moves_store (self->priv->match_list);
        self->priv->model = GTK_TREE_MODEL (model);

        gtk_tree_view_set_model (view, GTK_TREE_MODEL (model));

        g_signal_connect_swapped (G_OBJECT (model), "row-inserted",
                                  (GCallback) gibbon_move_list_view_on_insert,
                                  self);

        return self;
}

static void
gibbon_move_list_view_on_insert (const GibbonMoveListView *self)
{
        gint num_rows = gtk_tree_model_iter_n_children (self->priv->model,
                                                        NULL);
        GtkTreeIter iter;
        GtkTreePath *path;

        if (!num_rows)
                return;

        if (!gtk_tree_model_iter_nth_child (self->priv->model, &iter, NULL,
                                            num_rows - 1))
                return;

        path = gtk_tree_model_get_path (self->priv->model, &iter);

        /* FIXME! Only scroll if the last row is currently visible.  If
         * not, the user has scrolled the
         */

        gtk_tree_view_scroll_to_cell (self->priv->view, path, NULL, FALSE,
                                      0.0, 0.0);

        gtk_tree_path_free (path);
}

static void
gibbon_move_list_view_black_roll_data_func (GtkTreeViewColumn *tree_column,
                                            GtkCellRenderer *cell,
                                            GtkTreeModel *tree_model,
                                            GtkTreeIter *iter,
                                            GibbonMoveListView *self)
{
        gibbon_move_list_view_roll (self, GIBBON_POSITION_SIDE_BLACK,
                                    cell, tree_model, iter);
}

static void
gibbon_move_list_view_white_roll_data_func (GtkTreeViewColumn *tree_column,
                                            GtkCellRenderer *cell,
                                            GtkTreeModel *tree_model,
                                            GtkTreeIter *iter,
                                            GibbonMoveListView *self)
{
        gibbon_move_list_view_roll (self, GIBBON_POSITION_SIDE_WHITE,
                                    cell, tree_model, iter);
}

static void
gibbon_move_list_view_roll (GibbonMoveListView *self, GibbonPositionSide side,
                            GtkCellRenderer *cell, GtkTreeModel *tree_model,
                            GtkTreeIter *iter)
{
        gchar *roll_string;

        if (side < 0)
                gtk_tree_model_get (tree_model, iter,
                                    GIBBON_MATCH_LIST_COL_BLACK_ROLL,
                                    &roll_string,
                                    -1);
        else
                gtk_tree_model_get (tree_model, iter,
                                    GIBBON_MATCH_LIST_COL_WHITE_ROLL,
                                    &roll_string,
                                    -1);

        if (!roll_string)
                return;

        g_object_set (cell, "text", roll_string, NULL);

        g_free (roll_string);
}


static void
gibbon_move_list_view_black_move_data_func (GtkTreeViewColumn *tree_column,
                                            GtkCellRenderer *cell,
                                            GtkTreeModel *tree_model,
                                            GtkTreeIter *iter,
                                            GibbonMoveListView *self)
{
        gibbon_move_list_view_move (self, GIBBON_POSITION_SIDE_BLACK,
                                    cell, tree_model, iter);
}

static void
gibbon_move_list_view_white_move_data_func (GtkTreeViewColumn *tree_column,
                                            GtkCellRenderer *cell,
                                            GtkTreeModel *tree_model,
                                            GtkTreeIter *iter,
                                            GibbonMoveListView *self)
{
        gibbon_move_list_view_move (self, GIBBON_POSITION_SIDE_WHITE,
                                    cell, tree_model, iter);
}

static void
gibbon_move_list_view_move (GibbonMoveListView *self, GibbonPositionSide side,
                            GtkCellRenderer *cell, GtkTreeModel *tree_model,
                            GtkTreeIter *iter)
{
        gchar *move_string;

        if (side < 0)
                gtk_tree_model_get (tree_model, iter,
                                    GIBBON_MATCH_LIST_COL_BLACK_MOVE,
                                    &move_string,
                                    -1);
        else
                gtk_tree_model_get (tree_model, iter,
                                    GIBBON_MATCH_LIST_COL_WHITE_MOVE,
                                    &move_string,
                                    -1);
        if (!move_string)
                return;

        g_object_set (cell, "text", move_string, NULL);

        g_free (move_string);
}
