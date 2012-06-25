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
#include <gdk/gdkkeysyms.h>

#include "gibbon-move-list-view.h"
#include "gibbon-analysis-roll.h"

enum gibbon_move_list_view_signal {
        ACTION_SELECTED,
        LAST_SIGNAL
};
static guint gibbon_move_list_view_signals[LAST_SIGNAL] = { 0 };

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


static void gibbon_move_list_view_on_change (GibbonMoveListView *self,
                                             GtkTreePath *path,
                                             GtkTreeIter *iter,
                                             GtkTreeModel *model);
static void gibbon_move_list_view_on_new_match (const GibbonMoveListView *self,
                                                const GibbonMatchList *list);
static gboolean gibbon_move_list_view_on_query_tooltip (const GibbonMoveListView
                                                        *self,
                                                        gint x, gint y,
                                                        gboolean keyboard_tip,
                                                        GtkTooltip *tooltip,
                                                        GtkTreeView *view);
static gboolean gibbon_move_list_view_on_button_pressed (GibbonMoveListView
                                                         *self,
                                                         GdkEventButton
                                                         *event,
                                                         GtkWidget *widget);
static gboolean gibbon_move_list_view_on_key_press (GibbonMoveListView *self,
                                                    GdkEventKey *event,
                                                    GtkWidget *widget);
static void gibbon_move_list_view_to_left (GibbonMoveListView *self);
static void gibbon_move_list_view_to_right (GibbonMoveListView *self);
static void gibbon_move_list_view_on_row_deleted (GibbonMoveListView *self,
                                                  GtkTreePath  *path,
                                                  GtkTreeModel *tree_model);

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
                                               gint row, gint col,
                                               gboolean send_signal);
static gboolean gibbon_move_list_view_cell_valid (const GibbonMoveListView
                                                  *self,
                                                  GtkTreeIter *iter,
                                                  gint col);
static gboolean gibbon_move_list_view_cell_filled (const GibbonMoveListView
                                                   *self,
                                                   GtkTreeIter *iter,
                                                   gint col);

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

        gibbon_move_list_view_signals[ACTION_SELECTED] =
                g_signal_new ("action-selected",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__INT,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_INT);

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
        GtkStyle *style;
        GtkCellRenderer *renderer;

        self->priv->view = view;

        self->priv->match_list = match_list;

        style = gtk_widget_get_style (GTK_WIDGET (view));
        renderer = gtk_cell_renderer_text_new ();
        g_object_set (renderer,
                     "background-gdk", style->bg + GTK_STATE_NORMAL,
                     "xpad", 10,
                     NULL);
        gtk_tree_view_insert_column_with_attributes (view, -1, _("#"),
                        renderer,
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

        g_signal_connect_swapped (G_OBJECT (model), "row-changed",
                                  (GCallback) gibbon_move_list_view_on_change,
                                  self);
        g_signal_connect_swapped (G_OBJECT (match_list), "new-match",
                                  (GCallback)
                                  gibbon_move_list_view_on_new_match,
                                  self);
        g_signal_connect_swapped (G_OBJECT (view), "query-tooltip",
                                  (GCallback)
                                  gibbon_move_list_view_on_query_tooltip,
                                  self);
        g_signal_connect_swapped (G_OBJECT (view), "button-press-event",
                                  (GCallback)
                                  gibbon_move_list_view_on_button_pressed,
                                  self);
        g_signal_connect_swapped (G_OBJECT (view), "key-press-event",
                                  (GCallback)
                                  gibbon_move_list_view_on_key_press,
                                  self);
        g_signal_connect_swapped (G_OBJECT (model), "row-deleted",
                                  (GCallback)
                                  gibbon_move_list_view_on_row_deleted,
                                  self);
        return self;
}

static void
gibbon_move_list_view_on_change (GibbonMoveListView *self,
                                 GtkTreePath *path,
                                 GtkTreeIter *iter,
                                 GtkTreeModel *model)
{
        gint *indices;
        gint changed_col;
        gint changed_row;

        /*
         * Check for the rightmost "valid" column.  If there is none, no
         * user relevant content is available in this row, and we can postpone
         * all actions.
         */
        if (gibbon_move_list_view_cell_valid (self, iter,
                                              GIBBON_MATCH_LIST_COL_WHITE_MOVE))
                changed_col = GIBBON_MATCH_LIST_COL_WHITE_MOVE;
        else if (gibbon_move_list_view_cell_valid (self, iter,
                                              GIBBON_MATCH_LIST_COL_WHITE_ROLL))
                changed_col = GIBBON_MATCH_LIST_COL_WHITE_ROLL;
        else if (gibbon_move_list_view_cell_valid (self, iter,
                                              GIBBON_MATCH_LIST_COL_BLACK_MOVE))
                changed_col = GIBBON_MATCH_LIST_COL_BLACK_MOVE;
        else if (gibbon_move_list_view_cell_valid (self, iter,
                                              GIBBON_MATCH_LIST_COL_BLACK_ROLL))
                changed_col = GIBBON_MATCH_LIST_COL_BLACK_ROLL;
        else
                return;

        indices = gtk_tree_path_get_indices (path);
        changed_row = indices[0];

        /*
         * Unconditionally select this cell.  The user is either in the middle
         * of a match and should be notified about every change in the match
         * or we are currently loading a match from disk.  Either way we
         * don't care about a previous selection.
         */
        gibbon_move_list_view_select_cell (self, changed_row, changed_col,
                                           FALSE);
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
        gdouble luck_value;
        GibbonAnalysisRollLuck luck_type;
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
                                    &luck_value,
                                    GIBBON_MATCH_LIST_COL_BLACK_LUCK_TYPE,
                                    &luck_type,
                                    -1);
                col = GIBBON_MATCH_LIST_COL_BLACK_ROLL;
        } else {
                gtk_tree_model_get (tree_model, iter,
                                    GIBBON_MATCH_LIST_COL_WHITE_ROLL,
                                    &roll_string,
                                    GIBBON_MATCH_LIST_COL_WHITE_LUCK,
                                    &luck_value,
                                    GIBBON_MATCH_LIST_COL_WHITE_LUCK_TYPE,
                                    &luck_type,
                                    -1);
                col = GIBBON_MATCH_LIST_COL_WHITE_ROLL;
        }

        switch (luck_type) {
        case GIBBON_ANALYSIS_ROLL_LUCK_VERY_LUCKY:
                style = PANGO_STYLE_NORMAL;
                weight = PANGO_WEIGHT_BOLD;
                break;
        case GIBBON_ANALYSIS_ROLL_LUCK_VERY_UNLUCKY:
                style = PANGO_STYLE_ITALIC;
                weight = PANGO_WEIGHT_NORMAL;
                break;
        default:
                style = PANGO_STYLE_NORMAL;
                weight = PANGO_WEIGHT_NORMAL;
                break;
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
gibbon_move_list_view_on_query_tooltip (const GibbonMoveListView *self,
                                        gint x, gint y,
                                        gboolean keyboard_tip,
                                        GtkTooltip *tooltip,
                                        GtkTreeView *view)
{
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;
        GtkTreeViewColumn *column;
        gchar *text = NULL;
        gdouble luck_value;
        GibbonAnalysisRollLuck luck_type;

        g_return_val_if_fail (GIBBON_IS_MOVE_LIST_VIEW (self), FALSE);
        g_return_val_if_fail (GTK_IS_TREE_VIEW (view), FALSE);
        g_return_val_if_fail (view == self->priv->view, FALSE);
        g_return_val_if_fail (GTK_IS_TOOLTIP (tooltip), FALSE);

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
                                    GIBBON_MATCH_LIST_COL_BLACK_LUCK,
                                    &luck_value,
                                    GIBBON_MATCH_LIST_COL_BLACK_LUCK_TYPE,
                                    &luck_type,
                                    -1);
        } else if (column == self->priv->white_roll_column) {
                gtk_tree_model_get (model, &iter,
                                    GIBBON_MATCH_LIST_COL_WHITE_LUCK,
                                    &luck_value,
                                    GIBBON_MATCH_LIST_COL_WHITE_LUCK_TYPE,
                                    &luck_type,
                                    -1);
        }

        switch (luck_type) {
        case GIBBON_ANALYSIS_ROLL_LUCK_UNKNOWN:
                gtk_tree_path_free (path);
                return FALSE;
        case GIBBON_ANALYSIS_ROLL_LUCK_NONE:
                text = g_strdup_printf (_("Luck: %f"), luck_value);
                break;
        case GIBBON_ANALYSIS_ROLL_LUCK_LUCKY:
                text = g_strdup_printf (_("Luck: %f (lucky)"), luck_value);
                break;
        case GIBBON_ANALYSIS_ROLL_LUCK_VERY_LUCKY:
                text = g_strdup_printf (_("Luck: %f (very lucky)"), luck_value);
                break;
        case GIBBON_ANALYSIS_ROLL_LUCK_UNLUCKY:
                text = g_strdup_printf (_("Luck: %f (unlucky)"), luck_value);
                break;
        case GIBBON_ANALYSIS_ROLL_LUCK_VERY_UNLUCKY:
                text = g_strdup_printf (_("Luck: %f (very unlucky)"),
                                        luck_value);
                break;
        }

        if (text) { /* Make gcc -Wall happy! */
                gtk_tooltip_set_text (tooltip, text);
                gtk_tree_view_set_tooltip_row (view, tooltip, path);
                g_free (text);
        }

        gtk_tree_path_free (path);

        return text ? TRUE : FALSE;
}

static gboolean
gibbon_move_list_view_on_button_pressed (GibbonMoveListView *self,
                                         GdkEventButton *event,
                                         GtkWidget *widget)
{
        GtkTreeView *view = self->priv->view;
        GtkTreePath *path;
        GtkTreeViewColumn *column;
        gint col, row, *indices;

        if (!gtk_tree_view_get_path_at_pos (view, event->x, event->y,
                                            &path, &column, NULL, NULL)) {
                if (!gtk_widget_has_focus (GTK_WIDGET (view)))
                        gtk_widget_grab_focus (GTK_WIDGET (view));
                return TRUE;
        }

        /*
         * The normal behavior of a GtkTreeView is to unselect the current
         * selection if it is clicked again.  We decide against that because
         * our view is coupled to the board and the analysis window and
         * we could not propagate the unselect there.
         */
        if (column == self->priv->black_roll_column)
                col = GIBBON_MATCH_LIST_COL_BLACK_ROLL;
        else if (column == self->priv->white_roll_column)
                col = GIBBON_MATCH_LIST_COL_WHITE_ROLL;
        else if (column == self->priv->black_move_column)
                col = GIBBON_MATCH_LIST_COL_BLACK_MOVE;
        else if (column == self->priv->white_move_column)
                col = GIBBON_MATCH_LIST_COL_WHITE_MOVE;
        else
                return TRUE;

        indices = gtk_tree_path_get_indices (path);
        row = indices[0];
        gibbon_move_list_view_select_cell (self, row, col, TRUE);
        gtk_tree_path_free (path);

        if (!gtk_widget_has_focus (GTK_WIDGET (view)))
                gtk_widget_grab_focus (GTK_WIDGET (view));

        return TRUE;
}


static gboolean
gibbon_move_list_view_on_key_press (GibbonMoveListView *self,
                                    GdkEventKey *event,
                                    GtkWidget *widget)
{
        g_return_val_if_fail (GIBBON_IS_MOVE_LIST_VIEW (self), FALSE);
        g_return_val_if_fail (event != NULL, FALSE);
        g_return_val_if_fail (GTK_IS_TREE_VIEW (widget), FALSE);
        g_return_val_if_fail (GTK_WIDGET (self->priv->view) == widget, FALSE);

        /*
         * We can ignore modifier keys.  None of them has any semantics that
         * fits for our purposes.  Alternatively: Discard all events with
         * modifier keys?
         */
        switch (event->keyval) {
        case GDK_KEY_Left:
                gibbon_move_list_view_to_left (self);
                return TRUE;
        case GDK_KEY_Right:
                gibbon_move_list_view_to_right (self);
                return TRUE;
        }

        /* Propagate further.  */
        return FALSE;
}

static void
gibbon_move_list_view_select_cell (GibbonMoveListView *self,
                                   gint row, gint col,
                                   gboolean send_signal)
{
        GtkTreeIter iter;
        GtkTreePath *path = gtk_tree_path_new_from_indices (row, -1);
        gint action_no;

        if (!gtk_tree_model_get_iter (self->priv->model, &iter, path)) {
                gtk_tree_path_free (path);
                return;
        }
        if (!gibbon_move_list_view_cell_valid (self, &iter, col)) {
                gtk_tree_path_free (path);
                return;
        }

        if (col == self->priv->selected_col && row == self->priv->selected_row)
                return;

        self->priv->selected_col = col;
        self->priv->selected_row = row;

        /*
         * FIXME! We can reduce that to the area covered by the two cells
         * involved.
         */
        gtk_widget_queue_draw (GTK_WIDGET (self->priv->view));


        gtk_tree_view_scroll_to_cell (self->priv->view, path, NULL, FALSE,
                                      0.0, 0.0);

        gtk_tree_path_free (path);

        if (send_signal) {
                gtk_tree_model_get (self->priv->model, &iter,
                                    col + 1, &action_no,
                                    -1);
                g_signal_emit (self,
                               gibbon_move_list_view_signals[ACTION_SELECTED],
                               0, action_no);
        }
}

static void
gibbon_move_list_view_on_row_deleted (GibbonMoveListView *self,
                                      GtkTreePath  *path,
                                      GtkTreeModel *model)
{
        g_return_if_fail (GIBBON_IS_MOVE_LIST_VIEW (self));
        g_return_if_fail (GTK_IS_TREE_MODEL (model));

        self->priv->selected_col = self->priv->selected_row = -1;
}

/*
 * Check whether a selected cell is really used.  A cell in a roll column
 * is valid if either the cell itself has content or its peer move column.
 * A cell in a move column is valid if either the cell itself or the peer
 * roll column has content.
 */
static gboolean
gibbon_move_list_view_cell_valid (const GibbonMoveListView *self,
                                  GtkTreeIter *iter, gint col)
{
        gint peer;
        gchar c, *content;

        switch (col) {
        case GIBBON_MATCH_LIST_COL_BLACK_ROLL:
                peer = GIBBON_MATCH_LIST_COL_BLACK_MOVE;
                break;
        case GIBBON_MATCH_LIST_COL_BLACK_MOVE:
                peer = GIBBON_MATCH_LIST_COL_BLACK_ROLL;
                break;
        case GIBBON_MATCH_LIST_COL_WHITE_ROLL:
                peer = GIBBON_MATCH_LIST_COL_WHITE_MOVE;
                break;
        case GIBBON_MATCH_LIST_COL_WHITE_MOVE:
                peer = GIBBON_MATCH_LIST_COL_WHITE_ROLL;
                break;
        default:
                return FALSE;
        }

        gtk_tree_model_get (self->priv->model, iter, col, &content, -1);
        if (content) {
                c = *content;
                g_free (content);
                if (c)
                        return TRUE;
        }
        gtk_tree_model_get (self->priv->model, iter, peer, &content, -1);
        if (content) {
                c = *content;
                g_free (content);
                if (c)
                        return TRUE;
        }

        return FALSE;
}

/*
 * Check whether a selected cell has relevant content.  The content is not
 * valid if it is either not present, the emtpy string or a lone dash.
 */
static gboolean
gibbon_move_list_view_cell_filled (const GibbonMoveListView *self,
                                   GtkTreeIter *iter, gint col)
{
        gchar *content;

        gtk_tree_model_get (self->priv->model, iter, col, &content, -1);
        if (content) {
                if (!content[0] || (content[0] == '-' && !content[1])) {
                        g_free (content);
                        return FALSE;
                }
                g_free (content);
                return TRUE;
        }

        return FALSE;
}

static void
gibbon_move_list_view_to_left (GibbonMoveListView *self)
{
        GtkTreeIter iter;
        GtkTreePath *path;
        gint prev, row;

        if (self->priv->selected_row < 0 || self->priv->selected_col < 0)
                return;

        path = gtk_tree_path_new_from_indices (self->priv->selected_row, -1);
        if (!gtk_tree_model_get_iter (self->priv->model, &iter, path)) {
                gtk_tree_path_free (path);
                return;
        }
        gtk_tree_path_free (path);

        row = self->priv->selected_row;

        switch (self->priv->selected_col) {
        case GIBBON_MATCH_LIST_COL_WHITE_MOVE:
                if (gibbon_move_list_view_cell_filled (self, &iter,
                                GIBBON_MATCH_LIST_COL_WHITE_ROLL)) {
                        prev = GIBBON_MATCH_LIST_COL_WHITE_ROLL;
                        gibbon_move_list_view_select_cell (self, row, prev,
                                                           TRUE);
                        break;
                }
                /* FALLTHROUGH.  No break.  */
        case GIBBON_MATCH_LIST_COL_WHITE_ROLL:
                /*
                 * This can be invalid if white has the first roll of the
                 * game.  But this case is caught, when actually selecting
                 * the cell.
                 */
                prev = GIBBON_MATCH_LIST_COL_BLACK_MOVE;
                gibbon_move_list_view_select_cell (self, row, prev, TRUE);
                break;
        case GIBBON_MATCH_LIST_COL_BLACK_MOVE:
                if (gibbon_move_list_view_cell_filled (self, &iter,
                                GIBBON_MATCH_LIST_COL_BLACK_ROLL)) {
                        prev = GIBBON_MATCH_LIST_COL_BLACK_ROLL;
                        gibbon_move_list_view_select_cell (self, row, prev,
                                                           TRUE);
                        break;
                }
                /* FALLTHROUGH.  No break.  */
        case GIBBON_MATCH_LIST_COL_BLACK_ROLL:
                /*
                 * Again, possible errors are caught, when selecting a cell.
                 */
                prev = GIBBON_MATCH_LIST_COL_WHITE_MOVE;
                gibbon_move_list_view_select_cell (self, row - 1, prev, TRUE);
                break;
        }
}

static void
gibbon_move_list_view_to_right (GibbonMoveListView *self)
{
        GtkTreeIter iter;
        GtkTreePath *path;
        gint next, row;

        if (self->priv->selected_row < 0 || self->priv->selected_col < 0)
                return;

        path = gtk_tree_path_new_from_indices (self->priv->selected_row, -1);
        if (!gtk_tree_model_get_iter (self->priv->model, &iter, path)) {
                gtk_tree_path_free (path);
                return;
        }
        gtk_tree_path_free (path);

        row = self->priv->selected_row;

        switch (self->priv->selected_col) {
        case GIBBON_MATCH_LIST_COL_BLACK_ROLL:
                /*
                 * This can be invalid if white has the first roll of the
                 * game.  But this case is caught, when actually selecting
                 * the cell.
                 */
                next = GIBBON_MATCH_LIST_COL_BLACK_MOVE;
                gibbon_move_list_view_select_cell (self, row, next, TRUE);
                break;
        case GIBBON_MATCH_LIST_COL_BLACK_MOVE:
                if (gibbon_move_list_view_cell_filled (self, &iter,
                                GIBBON_MATCH_LIST_COL_WHITE_ROLL)) {
                        next = GIBBON_MATCH_LIST_COL_WHITE_ROLL;
                        gibbon_move_list_view_select_cell (self, row, next,
                                                           TRUE);
                        break;
                }
                /* FALLTHROUGH.  No break.  */
        case GIBBON_MATCH_LIST_COL_WHITE_ROLL:
                /*
                 * This can be invalid if white has the first roll of the
                 * game.  But this case is caught, when actually selecting
                 * the cell.
                 */
                next = GIBBON_MATCH_LIST_COL_WHITE_MOVE;
                gibbon_move_list_view_select_cell (self, row, next, TRUE);
                break;
        case GIBBON_MATCH_LIST_COL_WHITE_MOVE:
                if (!gtk_tree_model_iter_next (self->priv->model, &iter))
                        return;
                ++row;
                if (gibbon_move_list_view_cell_filled (self, &iter,
                                GIBBON_MATCH_LIST_COL_BLACK_ROLL)) {
                        next = GIBBON_MATCH_LIST_COL_BLACK_ROLL;
                } else {
                        next = GIBBON_MATCH_LIST_COL_BLACK_MOVE;
                }
                gibbon_move_list_view_select_cell (self, row, next,
                                                   TRUE);
                break;
        }
}
