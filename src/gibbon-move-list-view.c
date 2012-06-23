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
#include <gdk/gdk.h>

#include "gibbon-move-list-view.h"

typedef struct _GibbonMoveListViewPrivate GibbonMoveListViewPrivate;
struct _GibbonMoveListViewPrivate {
        GtkTreeView *view;
        GtkTreeViewColumn *black_roll_column;
        GtkTreeViewColumn *white_roll_column;
        GtkTreeViewColumn *black_move_column;
        GtkTreeViewColumn *white_move_column;
        GtkTreeModel *model;
        const GibbonMatchList *match_list;

        gint selected_row;
        gint selected_col;
};

#define GIBBON_MOVE_LIST_VIEW_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_MOVE_LIST_VIEW, GibbonMoveListViewPrivate))

G_DEFINE_TYPE (GibbonMoveListView, gibbon_move_list_view, G_TYPE_OBJECT)


static void gibbon_move_list_view_on_insert (const GibbonMoveListView *self);
static void gibbon_move_list_view_on_new_match (const GibbonMoveListView *self,
                                                const GibbonMatchList *list);
static gboolean gibbon_move_list_view_on_query_tooltip (GtkTreeView *view,
                                                        gint x, gint y,
                                                        gboolean keyboard_tip,
                                                        GtkTooltip *tooltip,
                                                        const GibbonMoveListView
                                                        *self);
static gboolean gibbon_move_list_view_on_button_pressed (GibbonMoveListView
                                                         *self,
                                                         GdkEventButton
                                                         *event);

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
static void gibbon_move_list_view_select_cell (GibbonMoveListView *self,
                                               GtkTreePath *path,
                                               GtkTreeViewColumn *column);

static void 
gibbon_move_list_view_init (GibbonMoveListView *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_MOVE_LIST_VIEW, GibbonMoveListViewPrivate);

        self->priv->view = NULL;
        self->priv->black_roll_column = NULL;
        self->priv->white_roll_column = NULL;
        self->priv->black_move_column = NULL;
        self->priv->white_move_column = NULL;
        self->priv->model = NULL;
        self->priv->match_list = NULL;

        self->priv->selected_row = -1;
        self->priv->selected_col = -1;
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
        gint colno;

        self->priv->view = view;

        self->priv->match_list = match_list;

        gtk_tree_view_insert_column_with_attributes (view, -1, _("#"),
                        gtk_cell_renderer_text_new (),
                        "text", GIBBON_MATCH_LIST_COL_MOVENO,
                        NULL);
        colno = gtk_tree_view_insert_column_with_data_func (view, -1, "  ",
                        gtk_cell_renderer_text_new (),
                        (GtkTreeCellDataFunc)
                        gibbon_move_list_view_black_roll_data_func,
                        self, NULL);
        self->priv->black_roll_column = gtk_tree_view_get_column (view,
                                                                  colno - 1);
        colno = gtk_tree_view_insert_column_with_data_func (view, -1, _("Black"),
                        gtk_cell_renderer_text_new (),
                        (GtkTreeCellDataFunc)
                        gibbon_move_list_view_black_move_data_func,
                        self, NULL);
        self->priv->black_move_column = gtk_tree_view_get_column (view,
                                                                  colno - 1);
        colno = gtk_tree_view_insert_column_with_data_func (view, -1, "  ",
                        gtk_cell_renderer_text_new (),
                        (GtkTreeCellDataFunc)
                        gibbon_move_list_view_white_roll_data_func,
                        self, NULL);
        self->priv->white_roll_column = gtk_tree_view_get_column (view,
                                                                  colno - 1);
        colno = gtk_tree_view_insert_column_with_data_func (view, -1, _("White"),
                        gtk_cell_renderer_text_new (),
                        (GtkTreeCellDataFunc)
                        gibbon_move_list_view_white_move_data_func,
                        self, NULL);
        self->priv->white_move_column = gtk_tree_view_get_column (view,
                                                                  colno - 1);

        model = gibbon_match_list_get_moves_store (self->priv->match_list);
        self->priv->model = GTK_TREE_MODEL (model);

        gtk_tree_view_set_model (view, GTK_TREE_MODEL (model));

        g_signal_connect_swapped (G_OBJECT (model), "row-inserted",
                                  (GCallback) gibbon_move_list_view_on_insert,
                                  self);
        g_signal_connect_swapped (G_OBJECT (match_list), "new-match",
                                  (GCallback)
                                  gibbon_move_list_view_on_new_match,
                                  self);
        /*
         * FIXME! How can we swap that?
         */
        g_signal_connect (G_OBJECT (view), "query-tooltip",
                          (GCallback) gibbon_move_list_view_on_query_tooltip,
                          self);

        g_signal_connect_swapped (G_OBJECT (view), "button-press-event",
                                  (GCallback)
                                  gibbon_move_list_view_on_button_pressed,
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
        gdouble luck;
        PangoStyle style;
        PangoWeight weight;
        GtkStyle *gtk_style;
        gsize offset;
        gint col, row = -1;
        GtkTreePath *path;
        gboolean selected = FALSE;
        gint *indices;

        if (side < 0) {
                gtk_tree_model_get (tree_model, iter,
                                    GIBBON_MATCH_LIST_COL_BLACK_ROLL,
                                    &roll_string,
                                    GIBBON_MATCH_LIST_COL_BLACK_LUCK,
                                    &luck,
                                    -1);
                col = GIBBON_MATCH_LIST_COL_BLACK_ROLL;
        } else {
                gtk_tree_model_get (tree_model, iter,
                                    GIBBON_MATCH_LIST_COL_WHITE_ROLL,
                                    &roll_string,
                                    GIBBON_MATCH_LIST_COL_WHITE_LUCK,
                                    &luck,
                                    -1);
                col = GIBBON_MATCH_LIST_COL_WHITE_ROLL;
        }

        if (luck >= 0.6) {
                style = PANGO_STYLE_NORMAL;
                weight = PANGO_WEIGHT_BOLD;
        } else if (luck <= -0.6) {
                style = PANGO_STYLE_ITALIC;
                weight = PANGO_WEIGHT_NORMAL;
        } else {
                style = PANGO_STYLE_NORMAL;
                weight = PANGO_WEIGHT_NORMAL;
        }

        if (col == self->priv->selected_col) {
                path = gtk_tree_model_get_path (self->priv->model, iter);
                indices = gtk_tree_path_get_indices (path);
                row = indices[0];
                gtk_tree_path_free (path);
        }

        selected = self->priv->selected_row == row
                   && self->priv->selected_col == col;

        gtk_style = gtk_widget_get_style (GTK_WIDGET (self->priv->view));
        offset = selected ? GTK_STATE_SELECTED : GTK_STATE_NORMAL;

        g_object_set (cell,
                      "text", roll_string,
                      "weight", weight,
                      "style", style,
                      "foreground-gdk", gtk_style->text + offset,
                      "background-gdk", gtk_style->base + offset,
                      NULL);
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
        GtkStyle *gtk_style;
        gsize offset;
        gint col, row = -1;
        GtkTreePath *path;
        gboolean selected = FALSE;
        gint *indices;

        if (side < 0) {
                gtk_tree_model_get (tree_model, iter,
                                    GIBBON_MATCH_LIST_COL_BLACK_MOVE,
                                    &move_string,
                                    -1);
                col = GIBBON_MATCH_LIST_COL_BLACK_MOVE;
        } else {
                gtk_tree_model_get (tree_model, iter,
                                    GIBBON_MATCH_LIST_COL_WHITE_MOVE,
                                    &move_string,
                                    -1);
                col = GIBBON_MATCH_LIST_COL_WHITE_MOVE;
        }

        if (col == self->priv->selected_col) {
                path = gtk_tree_model_get_path (self->priv->model, iter);
                indices = gtk_tree_path_get_indices (path);
                row = indices[0];
                gtk_tree_path_free (path);
        }

        selected = self->priv->selected_row == row
                   && self->priv->selected_col == col;

        gtk_style = gtk_widget_get_style (GTK_WIDGET (self->priv->view));
        offset = selected ? GTK_STATE_SELECTED : GTK_STATE_NORMAL;

        g_object_set (cell,
                      "text", move_string,
                      "foreground-gdk", gtk_style->text + offset,
                      "background-gdk", gtk_style->base + offset,
                      NULL);

        g_free (move_string);
}

static void
gibbon_move_list_view_on_new_match (const GibbonMoveListView *self,
                                    const GibbonMatchList *list)
{
        const GibbonMatch *match;
        GtkTreeViewColumn *column;

        g_return_if_fail (GIBBON_IS_MOVE_LIST_VIEW (self));

        if (self->priv->match_list != list)
                return;

        match = gibbon_match_list_get_match (list);

        column = gtk_tree_view_get_column (self->priv->view, 2);
        gtk_tree_view_column_set_title (column, gibbon_match_get_black (match));
        column = gtk_tree_view_get_column (self->priv->view, 4);
        gtk_tree_view_column_set_title (column, gibbon_match_get_white (match));
}

static gboolean
gibbon_move_list_view_on_query_tooltip (GtkTreeView *view, gint x, gint y,
                                        gboolean keyboard_tip,
                                        GtkTooltip *tooltip,
                                        const GibbonMoveListView *self)
{
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;
        GtkTreeViewColumn *column;
        gchar *text = NULL;
        gdouble luck;

        g_return_val_if_fail (GIBBON_IS_MOVE_LIST_VIEW (self), FALSE);
        g_return_val_if_fail (GTK_IS_TREE_VIEW (view), FALSE);
        g_return_val_if_fail (view == self->priv->view, FALSE);

        if (!gtk_tree_view_get_tooltip_context (view, &x, &y,
                                                keyboard_tip,
                                                &model, &path, &iter))
                return FALSE;
        gtk_tree_view_get_path_at_pos (view, x, y, NULL,
                                       &column, NULL, NULL);
        if (!column)
                return FALSE;

        if (column == self->priv->black_roll_column) {
                gtk_tree_model_get (model, &iter,
                                    GIBBON_MATCH_LIST_COL_BLACK_LUCK, &luck,
                                    -1);
                if (luck >= 0.6)
                        text = g_strdup_printf (_("Luck: %g (very lucky)"),
                                                luck);
                else if (luck >= 0.3)
                        text = g_strdup_printf (_("Luck: %g (lucky)"),
                                                luck);
                else if (luck > 0.0)
                        text = g_strdup_printf (_("Luck: %g"),
                                                luck);
                else if (luck <= -0.6)
                        text = g_strdup_printf (_("Luck: %g (very unlucky)"),
                                                luck);
                else if (luck <= -0.3)
                        text = g_strdup_printf (_("Luck: %g (unlucky)"),
                                                luck);
                else if (luck < 0.0)
                        text = g_strdup_printf (_("Luck: %g"),
                                                luck);
        } else if (column == self->priv->white_roll_column) {
                gtk_tree_model_get (model, &iter,
                                    GIBBON_MATCH_LIST_COL_WHITE_LUCK, &luck,
                                    -1);
                if (luck >= 0.6)
                        text = g_strdup_printf (_("Luck: %g (very lucky)"),
                                                luck);
                else if (luck >= 0.3)
                        text = g_strdup_printf (_("Luck: %g (lucky)"),
                                                luck);
                else if (luck > 0.0)
                        text = g_strdup_printf (_("Luck: %g"),
                                                luck);
                else if (luck <= -0.6)
                        text = g_strdup_printf (_("Luck: %g (very unlucky)"),
                                                luck);
                else if (luck <= -0.3)
                        text = g_strdup_printf (_("Luck: %g (unlucky)"),
                                                luck);
                else if (luck < 0.0)
                        text = g_strdup_printf (_("Luck: %g"),
                                                luck);
        }

        if (text) {
                gtk_tooltip_set_text (tooltip, text);
                gtk_tree_view_set_tooltip_row (view, tooltip, path);
                g_free (text);
        }

        gtk_tree_path_free (path);

        return text ? TRUE : FALSE;
}

static gboolean
gibbon_move_list_view_on_button_pressed (GibbonMoveListView *self,
                                         GdkEventButton *event)
{
        GtkTreeView *view = self->priv->view;
        GtkTreePath *path;
        GtkTreeViewColumn *column;

        if (!gtk_tree_view_get_path_at_pos (view, event->x, event->y,
                                            &path, &column, NULL, NULL)) {
                return TRUE;
        }

        gibbon_move_list_view_select_cell (self, path, column);
        gtk_tree_path_free (path);

        return TRUE;
}

static void
gibbon_move_list_view_select_cell (GibbonMoveListView *self,
                                   GtkTreePath *path,
                                   GtkTreeViewColumn *column)
{
        gint col = -1, row = -1;
        GtkTreeIter iter;
        gint *indices;

        if (gtk_tree_model_get_iter (self->priv->model, &iter, path)) {
                /* Path is valid.  */
                if (column == self->priv->black_roll_column)
                        col = GIBBON_MATCH_LIST_COL_BLACK_ROLL;
                else if (column == self->priv->white_roll_column)
                        col = GIBBON_MATCH_LIST_COL_WHITE_ROLL;
                else if (column == self->priv->black_move_column)
                        col = GIBBON_MATCH_LIST_COL_BLACK_MOVE;
                else if (column == self->priv->white_move_column)
                        col = GIBBON_MATCH_LIST_COL_WHITE_MOVE;

                indices = gtk_tree_path_get_indices (path);
                row = indices[0];
        }

        if (col < 0 || row < 0)
                col = row = -1;

        if (col == self->priv->selected_col && row == self->priv->selected_row)
                return;

        self->priv->selected_col = col;
        self->priv->selected_row = row;

        /*
         * FIXME! We can reduce that to the area covered by the two cells
         * involved.
         */
        gtk_widget_queue_draw (GTK_WIDGET (self->priv->view));
}
