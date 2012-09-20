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

#include "gibbon-move-list-view.h"
#include "gibbon-analysis-roll.h"

enum gibbon_move_list_view_signal {
        ACTION_SELECTED,
        LAST_SIGNAL
};
static guint gibbon_move_list_view_signals[LAST_SIGNAL] = { 0 };

typedef struct _GibbonMoveListViewPrivate GibbonMoveListViewPrivate;
struct _GibbonMoveListViewPrivate {
        GtkTreeView *tree_view;
        const GibbonMatchList *match_list;
        const GtkTreeModel *model;
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
static void gibbon_move_list_view_on_new_match (GibbonMoveListView *self,
                                                GibbonMatchList *matches);

static void 
gibbon_move_list_view_init (GibbonMoveListView *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_MOVE_LIST_VIEW, GibbonMoveListViewPrivate);

        self->priv->tree_view = NULL;
        self->priv->match_list = NULL;
        self->priv->model = NULL;
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

        g_signal_connect_swapped (G_OBJECT (match_list), "new-match",
                                  (GCallback)
                                  gibbon_move_list_view_on_new_match, self);

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
        GtkTreeView *view = self->priv->tree_view;
        GtkTreeViewColumn *number_column = gtk_tree_view_get_column (view, 0);

        gtk_tree_view_set_cursor (self->priv->tree_view, path, NULL, FALSE);

        gtk_tree_view_scroll_to_cell (view, path, number_column,
                                      FALSE, 0.0f, 0.0f);
}

static void
gibbon_move_list_view_on_cursor_changed (GibbonMoveListView *self,
                                         GtkTreeView *view)
{
        /* TODO */
        g_printerr ("cursor changed!\n");
}

static void
gibbon_move_list_view_on_new_match (GibbonMoveListView *self,
                                    GibbonMatchList *matches)
{
        g_printerr ("new match!\n");
}
