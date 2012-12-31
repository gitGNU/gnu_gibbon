/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
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

#include "gibbon-game.h"
#include "gibbon-move-list-view.h"
#include "gibbon-analysis-roll.h"
#include "gibbon-app.h"

enum gibbon_move_list_view_signal {
        ACTION_SELECTED,
        LAST_SIGNAL
};
static guint gibbon_move_list_view_signals[LAST_SIGNAL] = { 0 };

typedef struct _GibbonMoveListViewPrivate GibbonMoveListViewPrivate;
struct _GibbonMoveListViewPrivate {
        GtkTreeView *tree_view;
        const GibbonMatchList *match_list;
        GtkTreeModel *model;

        gint last_action_no;

        GtkWidget *move_back;
        GtkWidget *move_forward;
};

#define GIBBON_MOVE_LIST_VIEW_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_MOVE_LIST_VIEW, GibbonMoveListViewPrivate))

G_DEFINE_TYPE (GibbonMoveListView, gibbon_move_list_view, G_TYPE_OBJECT)

static void gibbon_move_list_view_move_data_func (GtkTreeViewColumn
                                                  *tree_column,
                                                  GtkCellRenderer *cell,
                                                  GtkTreeModel *tree_model,
                                                  GtkTreeIter *iter,
                                                  GibbonMoveListView *self);
static void gibbon_move_list_view_roll_data_func (GtkTreeViewColumn
                                                  *tree_column,
                                                  GtkCellRenderer *cell,
                                                  GtkTreeModel *tree_model,
                                                  GtkTreeIter *iter,
                                                  GibbonMoveListView *self);
static gboolean gibbon_move_list_view_on_query_tooltip (
                const GibbonMoveListView *self,
                gint x, gint y,
                gboolean keyboard_tip,
                GtkTooltip *tooltip,
                GtkTreeView *view);
static void gibbon_move_list_view_on_row_changed (GibbonMoveListView *self,
                                                  GtkTreePath *path,
                                                  GtkTreeIter *iter);
static void gibbon_move_list_view_on_cursor_changed (GibbonMoveListView *self,
                                                     GtkTreeView *view);
static gboolean gibbon_move_list_view_on_resize (GibbonMoveListView *self);
static void gibbon_move_list_view_on_game_selected (GibbonMoveListView *self,
                                                    const GibbonMatchList
                                                    *matches);
static void gibbon_move_list_view_on_game_updating (GibbonMoveListView *self,
                                                    const GibbonMatchList
                                                    *matches);
static gboolean gibbon_move_list_view_on_key_pressed (GibbonMoveListView *self,
                                                      GdkEventKey *event,
                                                      GtkTreeView *view);
static gboolean gibbon_move_list_view_previous (GibbonMoveListView *self);
static gboolean gibbon_move_list_view_next (GibbonMoveListView *self);
static void gibbon_move_list_view_set_state (const GibbonMoveListView *self);

static void 
gibbon_move_list_view_init (GibbonMoveListView *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_MOVE_LIST_VIEW, GibbonMoveListViewPrivate);

        self->priv->tree_view = NULL;
        self->priv->match_list = NULL;
        self->priv->model = NULL;

        self->priv->last_action_no = -1;

        self->priv->move_back = NULL;
        self->priv->move_forward = NULL;
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
 * @view: A #GtkTreeVew.
 * @match_list: The #GibbonMatchList holding the match information.
 *
 * Creates a new #GibbonMoveListView.
 *
 * Returns: The newly created #GibbonMoveListView or %NULL in case of failure.
 */
GibbonMoveListView *
gibbon_move_list_view_new (GtkTreeView *view,
                           const GibbonMatchList *match_list)
{
        GibbonMoveListView *self = g_object_new (GIBBON_TYPE_MOVE_LIST_VIEW,
                                                 NULL);
        GtkListStore *model;
        GtkCellRenderer *renderer;
        GtkStyle *style;
        GtkTreeViewColumn *column;
        GObject *obj;

        self->priv->match_list = match_list;
        model = gibbon_match_list_get_moves_store (self->priv->match_list);
        self->priv->model = GTK_TREE_MODEL (model);

        self->priv->tree_view = view;
        gtk_tree_view_set_model (view, GTK_TREE_MODEL (model));

        style = gtk_widget_get_style (GTK_WIDGET (view));
        renderer = gtk_cell_renderer_text_new ();
        g_object_set (renderer,
                     "background-gdk", style->bg + GTK_STATE_NORMAL,
                     "xalign", 0.5f,
                     "xpad", 3,
                     NULL);
        gtk_tree_view_insert_column_with_attributes (view, -1, _("#"),
                        renderer,
                        "text", GIBBON_MATCH_LIST_COL_MOVENO,
                        NULL);

        renderer = gtk_cell_renderer_text_new ();
        g_object_set (renderer,
                        "style", PANGO_STYLE_ITALIC,
                        "foreground-gdk", style->light + GTK_STATE_NORMAL,
                     NULL);
        gtk_tree_view_insert_column_with_attributes (view, -1, NULL,
                        renderer,
                        "text", GIBBON_MATCH_LIST_COL_PLAYER,
                        NULL);

        gtk_tree_view_insert_column_with_data_func (view, -1, NULL,
                        gtk_cell_renderer_text_new (),
                        (GtkTreeCellDataFunc)
                        gibbon_move_list_view_roll_data_func,
                        self, NULL);

        column = gtk_tree_view_column_new ();
        gtk_tree_view_append_column (view, column);
        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_column_pack_start (column, renderer, FALSE);
        gtk_tree_view_column_set_cell_data_func (
                        column, renderer,
                        (GtkTreeCellDataFunc)
                        gibbon_move_list_view_move_data_func,
                        self, NULL);

        g_signal_connect_swapped (G_OBJECT (view), "query-tooltip",
                                  (GCallback)
                                  gibbon_move_list_view_on_query_tooltip,
                                  self);

        g_signal_connect_swapped (G_OBJECT (view), "cursor-changed",
                                  (GCallback)
                                  gibbon_move_list_view_on_cursor_changed,
                                  self);

        g_signal_connect_swapped (G_OBJECT (model), "row-changed",
                                  (GCallback)
                                  gibbon_move_list_view_on_row_changed, self);

        g_signal_connect_swapped (G_OBJECT (match_list), "game-selected",
                                  (GCallback)
                                  gibbon_move_list_view_on_game_selected, self);
        g_signal_connect_swapped (G_OBJECT (match_list), "game-updating",
                                  (GCallback)
                                  gibbon_move_list_view_on_game_updating, self);

        g_signal_connect_swapped (G_OBJECT (view), "key-press-event",
                                  (GCallback)
                                  gibbon_move_list_view_on_key_pressed, self);

        obj = gibbon_app_find_object (app, "board-move-back",
                                      GTK_TYPE_TOOL_BUTTON);
        g_signal_connect_swapped (obj, "clicked",
                                  (GCallback) gibbon_move_list_view_previous,
                                  self);
        self->priv->move_back = GTK_WIDGET (obj);

        obj = gibbon_app_find_object (app, "board-next-move",
                                      GTK_TYPE_TOOL_BUTTON);
        g_signal_connect_swapped (obj, "clicked",
                                  (GCallback) gibbon_move_list_view_next,
                                  self);
        self->priv->move_forward = GTK_WIDGET (obj);

        gibbon_move_list_view_set_state (self);

        return self;
}

static void
gibbon_move_list_view_roll_data_func (GtkTreeViewColumn *tree_column,
                                      GtkCellRenderer *cell,
                                      GtkTreeModel *tree_model,
                                      GtkTreeIter *iter,
                                      GibbonMoveListView *self)
{
        gchar *roll_string;
        gdouble luck_value;
        GibbonAnalysisRollLuck luck_type;
        PangoStyle style;
        PangoWeight weight;

        gtk_tree_model_get (tree_model, iter,
                            GIBBON_MATCH_LIST_COL_ROLL,
                            &roll_string,
                            GIBBON_MATCH_LIST_COL_LUCK,
                            &luck_value,
                            GIBBON_MATCH_LIST_COL_LUCK_TYPE,
                            &luck_type,
                            -1);

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

        g_object_set (cell,
                      "text", roll_string,
                      "weight", weight,
                      "style", style,
                      NULL);
        g_free (roll_string);
}

static void
gibbon_move_list_view_move_data_func (GtkTreeViewColumn *tree_column,
                                      GtkCellRenderer *cell,
                                      GtkTreeModel *tree_model,
                                      GtkTreeIter *iter,
                                      GibbonMoveListView *self)
{
        gchar *move_string;

        gtk_tree_model_get (tree_model, iter,
                            GIBBON_MATCH_LIST_COL_MOVE,
                            &move_string,
                            -1);

        g_object_set (cell,
                      "markup", move_string,
                      NULL);

        g_free (move_string);
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
        gchar *text = NULL;
        gdouble luck_value;
        GibbonAnalysisRollLuck luck_type;

        g_return_val_if_fail (GIBBON_IS_MOVE_LIST_VIEW (self), FALSE);
        g_return_val_if_fail (GTK_IS_TREE_VIEW (view), FALSE);
        g_return_val_if_fail (view == self->priv->tree_view, FALSE);
        g_return_val_if_fail (GTK_IS_TOOLTIP (tooltip), FALSE);

        if (!gtk_tree_view_get_tooltip_context (view, &x, &y,
                                                keyboard_tip,
                                                &model, &path, &iter))
                return FALSE;

        gtk_tree_model_get (model, &iter,
                            GIBBON_MATCH_LIST_COL_LUCK,
                            &luck_value,
                            GIBBON_MATCH_LIST_COL_LUCK_TYPE,
                            &luck_type,
                            -1);

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

static void
gibbon_move_list_view_on_row_changed (GibbonMoveListView *self,
                                      GtkTreePath *path,
                                      GtkTreeIter *iter)
{
        GtkTreeView *view;
        GtkTreeViewColumn *number_column;

        view = self->priv->tree_view;

        /*
         * If the view currently disconnected from the model, there is no
         * need to update anything.
         */
        if (!gtk_tree_view_get_model (view))
                return;

        number_column = gtk_tree_view_get_column (view, 0);

        gtk_tree_view_set_cursor (self->priv->tree_view, path, NULL, FALSE);

        gtk_tree_view_scroll_to_cell (view, path, number_column,
                                      FALSE, 0.0f, 0.0f);
}

static void
gibbon_move_list_view_on_cursor_changed (GibbonMoveListView *self,
                                         GtkTreeView *view)
{
        GtkTreeSelection *selection;
        gint action_no = -1, move_action_no, roll_action_no;
        GList *selected;
        GtkTreePath *path;
        GtkTreeIter iter;

        selection = gtk_tree_view_get_selection (self->priv->tree_view);
        if (selection) {
                selected = gtk_tree_selection_get_selected_rows (
                                selection, NULL);
                if (selected) {
                        path = (GtkTreePath *) selected->data;
                        if (gtk_tree_model_get_iter (self->priv->model, &iter,
                                                     path)) {
                                gtk_tree_model_get (
                                        self->priv->model, &iter,
                                        GIBBON_MATCH_LIST_COL_MOVE_ACTION,
                                        &move_action_no,
                                        GIBBON_MATCH_LIST_COL_ROLL_ACTION,
                                        &roll_action_no,
                                        -1);
                                if (move_action_no < 0)
                                        action_no = roll_action_no;
                                else
                                        action_no = move_action_no;
                        }
                        g_list_foreach (selected, (GFunc) gtk_tree_path_free,
                                        NULL);
                        g_list_free (selected);
                }
                /*
                 * This can happen for a fresh row.
                 */
                if (action_no < 0)
                        return;
        }

        self->priv->last_action_no = action_no;
        g_signal_emit (self,
                       gibbon_move_list_view_signals[ACTION_SELECTED],
                       0, action_no);

        gibbon_move_list_view_set_state (self);

        /*
         * Sending the signal may cause the analysis widgets to be shown.
         * This may shrink the tree view and may make the selected row
         * be invisible.
         *
         * Scrolling here has no effect because it happens too early.  We
         * have to schedule that manually instead.
         */
        g_timeout_add (1, (GSourceFunc) gibbon_move_list_view_on_resize, self);
}

static gboolean
gibbon_move_list_view_on_resize (GibbonMoveListView *self)
{
        GtkTreeSelection *selection;
        GList *selected;
        GtkTreePath *path;
        GtkTreeView *view;

        view = self->priv->tree_view;

        selection = gtk_tree_view_get_selection (self->priv->tree_view);
        if (!selection)
                return FALSE;

        selected = gtk_tree_selection_get_selected_rows (
                        selection,
                        &self->priv->model);
        if (!selected)
                return FALSE;

        path = (GtkTreePath *) selected->data;

        gtk_tree_view_scroll_to_cell (view, path, NULL, FALSE, 0.0f, 0.0f);

        g_list_foreach (selected, (GFunc) gtk_tree_path_free, NULL);
        g_list_free (selected);

        return FALSE;
}

void
gibbon_move_list_view_on_new_match (GibbonMoveListView *self,
                                    GibbonMatch *match)
{
        gsize num_actions = 0;
        GibbonGame *game;

        g_return_if_fail (GIBBON_IS_MOVE_LIST_VIEW (self));
        g_return_if_fail (GIBBON_IS_MATCH (match));

        game = gibbon_match_get_current_game (match);
        if (game)
                num_actions = gibbon_game_get_num_actions (game);

        g_signal_emit (self,
                       gibbon_move_list_view_signals[ACTION_SELECTED],
                       0, (gint) num_actions - 1);
}

static void
gibbon_move_list_view_on_game_updating (GibbonMoveListView *self,
                                        const GibbonMatchList *matches)
{
        /*
         * Disconnect the model from the view while updating.
         */
        gtk_tree_view_set_model (self->priv->tree_view, NULL);
}

static void
gibbon_move_list_view_on_game_selected (GibbonMoveListView *self,
                                        const GibbonMatchList *matches)
{
        gint num_rows;
        GtkTreeIter iter;
        GtkTreePath *path;

        /*
         * Reconnect the model from the view after updating.
         */
        gtk_tree_view_set_model (self->priv->tree_view, self->priv->model);

        /* Mark the last row as dirty.  */
        num_rows = gtk_tree_model_iter_n_children (self->priv->model, NULL);
        if (!num_rows)
                return;

        if (!gtk_tree_model_iter_nth_child (self->priv->model, &iter, NULL,
                                            num_rows - 1))
                return;

        path = gtk_tree_model_get_path (self->priv->model, &iter);
        if (!path)
                return;

        gibbon_move_list_view_on_row_changed (self, path, &iter);
        gtk_tree_path_free (path);
}

static gboolean
gibbon_move_list_view_on_key_pressed (GibbonMoveListView *self,
                                      GdkEventKey *event,
                                      GtkTreeView *view)
{
        if (event->keyval == GDK_KEY_Left)
                return gibbon_move_list_view_previous (self);
        if (event->keyval == GDK_KEY_Right)
                return gibbon_move_list_view_next (self);
        return FALSE;
}

static gboolean
gibbon_move_list_view_previous (GibbonMoveListView *self)
{
        GtkTreeSelection *selection;
        gint action_no, move_action_no = -2, roll_action_no = -2;
        GList *selected;
        GtkTreePath *path;
        GtkTreeIter iter;
        GtkTreeViewColumn *number_column;

        selection = gtk_tree_view_get_selection (self->priv->tree_view);
        if (!selection)
                return TRUE;

        selected = gtk_tree_selection_get_selected_rows (selection,
                                                         &self->priv->model);
        if (!selected)
                return TRUE;

        path = (GtkTreePath *) selected->data;
        /* The path gets freed with the list.  We need a copy.  */
        if (path)
                path = gtk_tree_path_copy (path);

        if (gtk_tree_model_get_iter (self->priv->model, &iter, path)) {
                gtk_tree_model_get (self->priv->model, &iter,
                                    GIBBON_MATCH_LIST_COL_MOVE_ACTION,
                                    &move_action_no,
                                    GIBBON_MATCH_LIST_COL_ROLL_ACTION,
                                    &roll_action_no,
                                    -1);
        }
        g_list_foreach (selected, (GFunc) gtk_tree_path_free, NULL);
        g_list_free (selected);
        if (!path)
                return TRUE;

        /* Already at the start of game? */
        if (self->priv->last_action_no < 0) {
                gtk_tree_path_free (path);
                gtk_widget_error_bell (GTK_WIDGET (self->priv->tree_view));
                return TRUE;
        }

        /* Display the roll? */
        if (move_action_no >= 0
            && move_action_no == self->priv->last_action_no
            && roll_action_no >= 0) {
                gtk_tree_path_free (path);
                self->priv->last_action_no = roll_action_no;
                gibbon_move_list_view_set_state (self);
                g_signal_emit (
                        self,
                        gibbon_move_list_view_signals[ACTION_SELECTED],
                        0, roll_action_no);
                return TRUE;
        }

        /* Move to the previous node.  */
        action_no = self->priv->last_action_no - 1;
        gtk_tree_path_prev (path);

        if (!gtk_tree_model_get_iter (self->priv->model, &iter, path)) {
                gtk_tree_path_free (path);
                gtk_widget_error_bell (GTK_WIDGET (self->priv->tree_view));
                return TRUE;
        }

        number_column = gtk_tree_view_get_column (self->priv->tree_view, 0);

        gtk_tree_view_set_cursor (self->priv->tree_view, path, NULL, FALSE);

        gtk_tree_view_scroll_to_cell (self->priv->tree_view, path,
                                      number_column, FALSE, 0.0f, 0.0f);
        gtk_tree_path_free (path);

        self->priv->last_action_no = action_no;

        gibbon_move_list_view_set_state (self);

        g_signal_emit (self,
                       gibbon_move_list_view_signals[ACTION_SELECTED],
                       0, self->priv->last_action_no);

        return TRUE;
}

static gboolean
gibbon_move_list_view_next (GibbonMoveListView *self)
{
        GtkTreeSelection *selection;
        gint action_no, move_action_no = -2, roll_action_no = -2;
        GList *selected;
        GtkTreePath *path;
        GtkTreeIter iter;
        GtkTreeViewColumn *number_column;

        selection = gtk_tree_view_get_selection (self->priv->tree_view);
        if (!selection)
                return TRUE;

        selected = gtk_tree_selection_get_selected_rows (selection,
                                                         &self->priv->model);
        if (!selected)
                return TRUE;

        path = (GtkTreePath *) selected->data;
        /* The path gets freed with the list.  We need a copy.  */
        if (path)
                path = gtk_tree_path_copy (path);
        if (gtk_tree_model_get_iter (self->priv->model, &iter, path)) {
                gtk_tree_model_get (self->priv->model, &iter,
                                    GIBBON_MATCH_LIST_COL_MOVE_ACTION,
                                    &move_action_no,
                                    GIBBON_MATCH_LIST_COL_ROLL_ACTION,
                                    &roll_action_no,
                                    -1);
        }
        g_list_foreach (selected, (GFunc) gtk_tree_path_free, NULL);
        g_list_free (selected);
        if (!path)
                return TRUE;

        /* Display the roll? */
        if (roll_action_no >= 0
            && roll_action_no == self->priv->last_action_no
            && move_action_no >= 0) {
                gtk_tree_path_free (path);
                self->priv->last_action_no = move_action_no;
                gibbon_move_list_view_set_state (self);
                g_signal_emit (
                        self,
                        gibbon_move_list_view_signals[ACTION_SELECTED],
                        0, move_action_no);
                return TRUE;
        }

        /* Move to the next node.  */
        action_no = self->priv->last_action_no + 1;
        gtk_tree_path_next (path);

        if (!gtk_tree_model_get_iter (self->priv->model, &iter, path)) {
                gtk_tree_path_free (path);
                gtk_widget_error_bell (GTK_WIDGET (self->priv->tree_view));
                return TRUE;
        }

        number_column = gtk_tree_view_get_column (self->priv->tree_view, 0);

        /* Defer signal delivery while setting the cursor.  */
        gtk_tree_view_set_cursor (self->priv->tree_view, path, NULL, FALSE);

        gtk_tree_view_scroll_to_cell (self->priv->tree_view, path,
                                      number_column, FALSE, 0.0f, 0.0f);
        gtk_tree_path_free (path);

        self->priv->last_action_no = action_no;

        gibbon_move_list_view_set_state (self);

        g_signal_emit (self,
                       gibbon_move_list_view_signals[ACTION_SELECTED],
                       0, self->priv->last_action_no);

        return TRUE;
}

static void
gibbon_move_list_view_set_state (const GibbonMoveListView *self)
{
        const GibbonMatch *match;
        const GibbonGame *game;
        gint active;
        gsize num_actions;

        match = gibbon_app_get_match (app);

        /*
         * Already at the start of the list?
         */
        if (!match) {
                gtk_widget_set_sensitive (GTK_WIDGET (self->priv->move_back),
                                          FALSE);
                gtk_widget_set_sensitive (GTK_WIDGET (self->priv->move_forward),
                                          FALSE);
                return;
        }

        if (self->priv->last_action_no < 0)
                gtk_widget_set_sensitive (GTK_WIDGET (self->priv->move_back),
                                          FALSE);
        else
                gtk_widget_set_sensitive (GTK_WIDGET (self->priv->move_back),
                                          TRUE);

        active = gibbon_match_list_get_active_game (self->priv->match_list);
        game = gibbon_match_get_nth_game (match, active);
        if (!game) {
                gtk_widget_set_sensitive (GTK_WIDGET (self->priv->move_forward),
                                          FALSE);
                return;
        }
        num_actions = gibbon_game_get_num_actions (game);
        if (!num_actions || self->priv->last_action_no == num_actions - 1)
                gtk_widget_set_sensitive (GTK_WIDGET (self->priv->move_forward),
                                          FALSE);
        else
                gtk_widget_set_sensitive (GTK_WIDGET (self->priv->move_forward),
                                          TRUE);
}
