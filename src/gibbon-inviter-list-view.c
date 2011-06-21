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
 * SECTION:gibbon-inviter-list-view
 * @short_description: Visual representation of the inviters list!
 *
 * Since: 0.1.1
 *
 * View for the Gibbon inviter list.
 **/

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "gibbon-inviter-list-view.h"
#include "gibbon-signal.h"
#include "gibbon-connection.h"
#include "gibbon-reliability-renderer.h"
#include "gibbon-inviter-list.h"
#include "gibbon-reliability.h"
#include "gibbon-session.h"

typedef struct _GibbonInviterListViewPrivate GibbonInviterListViewPrivate;
struct _GibbonInviterListViewPrivate {
        GibbonApp *app;
        GibbonInviterList *inviters;
        GtkTreeView *inviters_view;
        GtkMenu *player_menu;

        GibbonSignal *button_pressed_handler;

        GibbonSignal *tell_handler;

        GtkTreeViewColumn *reliability_column;
};

#define GIBBON_INVITER_LIST_VIEW_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_INVITER_LIST_VIEW, GibbonInviterListViewPrivate))

G_DEFINE_TYPE (GibbonInviterListView, gibbon_inviter_list_view, G_TYPE_OBJECT)

static gboolean gibbon_inviter_list_view_on_button_pressed (GibbonInviterListView
                                                           *self,
                                                           GdkEventButton
                                                           *event);
static gchar *gibbon_inviter_list_view_row_name (const GibbonInviterListView
                                                *self);
static void gibbon_inviter_list_view_on_tell (const GibbonInviterListView *self);
static gboolean gibbon_inviter_list_view_on_query_tooltip (GtkWidget *widget,
                                                          gint x, gint y,
                                                          gboolean keyboard_tip,
                                                          GtkTooltip *tooltip,
                                                          gpointer _self);

static void print2digits (GtkTreeViewColumn *tree_column,
                          GtkCellRenderer *cell, GtkTreeModel *tree_model,
                          GtkTreeIter *iter, gpointer data);

static void 
gibbon_inviter_list_view_init (GibbonInviterListView *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_INVITER_LIST_VIEW, GibbonInviterListViewPrivate);

        self->priv->app = NULL;
        self->priv->inviters = NULL;
        self->priv->inviters_view = NULL;
        self->priv->player_menu = NULL;

        self->priv->button_pressed_handler = NULL;
        self->priv->tell_handler = NULL;

        self->priv->reliability_column = NULL;
}

static void
gibbon_inviter_list_view_finalize (GObject *object)
{
        GibbonInviterListView *self = GIBBON_INVITER_LIST_VIEW (object);

        if (self->priv->inviters)
                g_object_unref (self->priv->inviters);
        self->priv->inviters = NULL;

        if (self->priv->player_menu)
                g_object_unref (self->priv->player_menu);
        self->priv->player_menu = NULL;

        self->priv->inviters_view = NULL;
        self->priv->app = NULL;

        if (self->priv->button_pressed_handler)
                g_object_unref (self->priv->button_pressed_handler);
        self->priv->button_pressed_handler = NULL;

        if (self->priv->tell_handler)
                g_object_unref (self->priv->tell_handler);
        self->priv->tell_handler = NULL;

        G_OBJECT_CLASS (gibbon_inviter_list_view_parent_class)->finalize(object);
}

static void
gibbon_inviter_list_view_class_init (GibbonInviterListViewClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonInviterListViewPrivate));

        object_class->finalize = gibbon_inviter_list_view_finalize;
}

/**
 * gibbon_inviter_list_view_new:
 * @app: The #GibbonApp.
 * @inviters: The underlying #GibbonInviterList.
 *
 * Creates a new #GibbonInviterListView.
 *
 * Returns: The newly created #GibbonInviterListView or %NULL in case of failure.
 */
GibbonInviterListView *
gibbon_inviter_list_view_new (GibbonApp *app, GibbonInviterList *inviters)
{
        GibbonInviterListView *self = g_object_new (GIBBON_TYPE_INVITER_LIST_VIEW,
                                                   NULL);
        GtkTreeView *view;
        GtkTreeViewColumn *col;
        GtkCellRenderer *renderer;
        GCallback callback;
        GObject *emitter;

        self->priv->app = app;
        self->priv->inviters = inviters;
        g_object_ref (inviters);

        self->priv->inviters_view = view =
            GTK_TREE_VIEW (gibbon_app_find_object (app,
                                                   "inviter-view",
                                                   GTK_TYPE_TREE_VIEW));

        gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Inviter"),
                gtk_cell_renderer_text_new (),
                "text", GIBBON_INVITER_LIST_COL_NAME,
                NULL);
        col = gtk_tree_view_get_column (view, GIBBON_INVITER_LIST_COL_NAME);
        gtk_tree_view_column_set_clickable (col, TRUE);
        gtk_tree_view_column_set_sort_indicator (col, TRUE);
        gtk_tree_view_column_set_sort_order (col, GTK_SORT_ASCENDING);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Rating"),
                renderer,
                "text", GIBBON_INVITER_LIST_COL_RATING,
                NULL);
        col = gtk_tree_view_get_column (view, GIBBON_INVITER_LIST_COL_RATING);
        gtk_tree_view_column_set_clickable (col, TRUE);
        gtk_tree_view_column_set_cell_data_func (col, renderer,
                print2digits, (gpointer) GIBBON_INVITER_LIST_COL_RATING, NULL);

        gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Exp."),
                gtk_cell_renderer_text_new (),
                "text", GIBBON_INVITER_LIST_COL_EXPERIENCE,
                NULL);
        col = gtk_tree_view_get_column (view, GIBBON_INVITER_LIST_COL_EXPERIENCE);
        gtk_tree_view_column_set_clickable (col, TRUE);

        renderer = gibbon_reliability_renderer_new ();
        gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Reliability"),
                renderer,
                "reliability", GIBBON_INVITER_LIST_COL_RELIABILITY,
                NULL);
        self->priv->reliability_column =
            gtk_tree_view_get_column (view, GIBBON_INVITER_LIST_COL_RELIABILITY);

        gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Saved games"),
                gtk_cell_renderer_text_new (),
                "text", GIBBON_INVITER_LIST_COL_SAVEDCOUNT,
                NULL);
        col = gtk_tree_view_get_column (view,
                                        GIBBON_INVITER_LIST_COL_SAVEDCOUNT);
        gtk_tree_view_column_set_clickable (col, TRUE);

        gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Software"),
                gtk_cell_renderer_text_new (),
                "text", GIBBON_INVITER_LIST_COL_CLIENT,
                NULL);
        gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Hostname"),
                gtk_cell_renderer_text_new (),
                "text", GIBBON_INVITER_LIST_COL_HOSTNAME,
                NULL);
        gtk_tree_view_insert_column_with_attributes (
                view,
                -1,
                _("Address"),
                gtk_cell_renderer_text_new (),
                "text", GIBBON_INVITER_LIST_COL_EMAIL,
                NULL);

        g_object_set (G_OBJECT (view), "has-tooltip", TRUE, NULL);

        gibbon_inviter_list_connect_view (self->priv->inviters, view);
        callback = (GCallback) gibbon_inviter_list_view_on_button_pressed;

        self->priv->button_pressed_handler =
                 gibbon_signal_new (G_OBJECT (view), "button-press-event",
                                    callback, G_OBJECT (self));

        emitter = gibbon_app_find_object (app, "tell-inviter-menu-item",
                                          GTK_TYPE_MENU_ITEM);
        callback = (GCallback) gibbon_inviter_list_view_on_tell;
        self->priv->tell_handler =
                 gibbon_signal_new (emitter, "activate",
                                    callback, G_OBJECT (self));

        callback = (GCallback) gibbon_inviter_list_view_on_query_tooltip;
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
        gdouble d;

        g_free (cell_text->text);
        gtk_tree_model_get (tree_model, iter, GPOINTER_TO_INT (data), &d, -1);
        cell_text->text = g_strdup_printf ("%.2f", d);
}

static gboolean
gibbon_inviter_list_view_on_button_pressed (GibbonInviterListView *self,
                                           GdkEventButton *event)
{
        GtkTreeSelection *selection;
        GtkTreePath *path;
        GtkTreeView *view;
        GObject *player_menu;
        gchar *who;

        if (event->type != GDK_BUTTON_PRESS  ||  event->button != 3)
                return FALSE;

        view = self->priv->inviters_view;

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
        who = gibbon_inviter_list_view_row_name (self);
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
gibbon_inviter_list_view_row_name (const GibbonInviterListView *self)
{
        GtkTreeSelection *selection;
        gint num_rows;
        GList *selected_rows;
        GList *first;
        GtkTreePath *path;
        GtkTreeModel *model;
        GtkTreeIter iter;
        gchar *who = NULL;

        g_return_val_if_fail (GIBBON_IS_INVITER_LIST_VIEW (self), NULL);

        selection = gtk_tree_view_get_selection (self->priv->inviters_view);
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
                model = gtk_tree_view_get_model (self->priv->inviters_view);

                if (gtk_tree_model_get_iter (model, &iter, path)) {
                        gtk_tree_model_get (model, &iter,
                                            GIBBON_INVITER_LIST_COL_NAME, &who,
                                            -1);
                }
        }

        g_list_foreach (selected_rows, (GFunc) gtk_tree_path_free, NULL);
        g_list_free (selected_rows);

        return who;
}

static void
gibbon_inviter_list_view_on_tell (const GibbonInviterListView *self)
{
        gchar *whom;

        g_return_if_fail (GIBBON_IS_INVITER_LIST_VIEW (self));

        whom = gibbon_inviter_list_view_row_name (self);
        if (!whom)
                return;

        gibbon_app_start_chat (self->priv->app, whom);

        g_free (whom);
}

static gboolean
gibbon_inviter_list_view_on_query_tooltip (GtkWidget *widget,
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
        GibbonInviterListView *self;
        gchar *inviter_name;
        gint bx, by;
        gchar *text = NULL;
        GibbonReliability *rel;
        const gchar *rel_descr;
        const gchar *conf_descr;

        g_return_val_if_fail (GIBBON_IS_INVITER_LIST_VIEW (_self), FALSE);
        self = GIBBON_INVITER_LIST_VIEW (_self);
        g_return_val_if_fail (GTK_IS_TREE_VIEW (widget), FALSE);

        tree_view = GTK_TREE_VIEW (widget);
        model = gtk_tree_view_get_model (tree_view);

        if (!gtk_tree_view_get_tooltip_context (tree_view, &x, &y,
                                                keyboard_tip,
                                                &model, &path, &iter))
                return FALSE;
        gtk_tree_view_convert_widget_to_bin_window_coords (tree_view,
                                                           x, y, &bx, &by);
        gtk_tree_view_get_path_at_pos (tree_view, bx, by, NULL,
                                       &column, NULL, NULL);
        if (!column)
                return FALSE;

        gtk_tree_model_get (model, &iter, 0, &inviter_name, -1);

        if (column == self->priv->reliability_column) {
                gtk_tree_model_get (model, &iter,
                                    GIBBON_INVITER_LIST_COL_RELIABILITY, &rel,
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

                        text = g_strdup_printf ("<b>Reliability of inviter"
                                                " <i>%s</i>:</b>\n"
                                                " %f (%s) with a measurement"
                                                " confidence "
                                                " of %u (%s).",
                                                inviter_name,
                                                rel->value, rel_descr,
                                                rel->confidence, conf_descr);
                } else {
                        text = g_strdup_printf ("<b>Reliability of inviter"
                                                " <i>%s</i></b>: unknown.",
                                                inviter_name);
                }
        } else {
                return FALSE;
        }

        gtk_tooltip_set_markup (tooltip, text);

        gtk_tree_view_set_tooltip_row (tree_view, tooltip, path);

        gtk_tree_path_free (path);
        g_free (inviter_name);
        g_free (text);

        return TRUE;
}
