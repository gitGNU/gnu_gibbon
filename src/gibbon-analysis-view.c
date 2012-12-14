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
 * SECTION:gibbon-analysis-view
 * @short_description: View components for analysis data!
 *
 * Since: 0.2.0
 *
 * The view components displaying move analysis data are shown and hidden
 * on demand, depending on whether analysis data is available for a particular
 * move or roll.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-app.h"
#include "gibbon-analysis-view.h"
#include "gibbon-analysis-roll.h"
#include "gibbon-analysis-move.h"
#include "gibbon-roll.h"
#include "gibbon-move.h"
#include "gibbon-double.h"
#include "gibbon-take.h"
#include "gibbon-drop.h"
#include "gibbon-met.h"
#include "gibbon-board.h"

typedef struct _GibbonAnalysisViewPrivate GibbonAnalysisViewPrivate;
struct _GibbonAnalysisViewPrivate {
        const GibbonApp *app;

        GtkBox *detail_box;
        GtkNotebook *notebook;
        gboolean notebook_sized;
        GtkTreeView *variants_view;

        GtkLabel *move_summary;
        GtkLabel *cube_summary;

        GtkLabel *cube_equity_type;
        GtkLabel *cube_equity_values;

        GtkLabel *cube_percents;
        GtkLabel *cube_win;
        GtkLabel *cube_win_g;
        GtkLabel *cube_win_bg;
        GtkLabel *cube_lose;
        GtkLabel *cube_lose_g;
        GtkLabel *cube_lose_bg;
        GtkLabel *cube_percents1;
        GtkLabel *cube_win1;
        GtkLabel *cube_win_g1;
        GtkLabel *cube_win_bg1;
        GtkLabel *cube_lose1;
        GtkLabel *cube_lose_g1;
        GtkLabel *cube_lose_bg1;

        GtkLabel *action_1;
        GtkLabel *action_2;
        GtkLabel *action_3;

        GtkLabel *eq_1;
        GtkLabel *eq_2;
        GtkLabel *eq_3;
        GtkLabel *eq_delta_1;
        GtkLabel *eq_delta_2;
        GtkLabel *eq_delta_3;

        GtkLabel *mwc_1;
        GtkLabel *mwc_2;
        GtkLabel *mwc_3;
        GtkLabel *mwc_delta_1;
        GtkLabel *mwc_delta_2;
        GtkLabel *mwc_delta_3;

        GtkLabel *proper_action;

        GibbonAnalysisMove *ma;
};

#define GIBBON_ANALYSIS_VIEW_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_ANALYSIS_VIEW, GibbonAnalysisViewPrivate))

G_DEFINE_TYPE (GibbonAnalysisView, gibbon_analysis_view, G_TYPE_OBJECT)

static void gibbon_analysis_view_set_move (GibbonAnalysisView *self,
                                           GibbonAnalysisMove *a);
static void gibbon_analysis_view_set_roll (GibbonAnalysisView *self,
                                           GibbonAnalysisRoll *a);
static void gibbon_analysis_view_equity_data_func (GtkTreeViewColumn
                                                   *tree_column,
                                                   GtkCellRenderer *cell,
                                                   GtkTreeModel *tree_model,
                                                   GtkTreeIter *iter,
                                                   GibbonAnalysisView *self);
static void gibbon_analysis_view_equity_diff_data_func (GtkTreeViewColumn
                                                        *tree_column,
                                                        GtkCellRenderer *cell,
                                                        GtkTreeModel
                                                        *tree_model,
                                                        GtkTreeIter *iter,
                                                        GibbonAnalysisView
                                                        *self);
static void gibbon_analysis_view_mwc_data_func (GtkTreeViewColumn *tree_column,
                                                GtkCellRenderer *cell,
                                                GtkTreeModel *tree_model,
                                                GtkTreeIter *iter,
                                                GibbonAnalysisView *self);
static void gibbon_analysis_view_mwc_diff_data_func (GtkTreeViewColumn
                                                     *tree_column,
                                                     GtkCellRenderer *cell,
                                                     GtkTreeModel *tree_model,
                                                     GtkTreeIter *iter,
                                                     GibbonAnalysisView *self);
static gboolean gibbon_analysis_view_on_query_tooltip (const GibbonAnalysisView
                                                       *self,
                                                       gint x, gint y,
                                                       gboolean keyboard_tip,
                                                       GtkTooltip *tooltip,
                                                       GtkTreeView *view);
static void gibbon_analysis_view_on_cursor_changed (GibbonAnalysisView *self,
                                                    GtkTreeView *view);


/* TRANSLATORS: This is used for displaying equities!  */
#define EQ_FORMAT _("%+.3f")

/* TRANSLATORS: This is used for displaying equity differences!  */
#define EQ_DIFF_FORMAT _("%+.3f")

/* TRANSLATORS: This is used for displaying match winning chances (MWC)!  */
#define MWC_FORMAT _("%.2f%%")

/* TRANSLATORS: This is used for displaying differences in MWC!  */
#define MWC_DIFF_FORMAT _("%+.2f%%")

static void 
gibbon_analysis_view_init (GibbonAnalysisView *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_ANALYSIS_VIEW, GibbonAnalysisViewPrivate);

        self->priv->app = NULL;

        self->priv->detail_box = NULL;
        self->priv->notebook = NULL;
        self->priv->notebook_sized = FALSE;
        self->priv->variants_view = NULL;

        self->priv->move_summary = NULL;
        self->priv->cube_summary = NULL;

        self->priv->cube_equity_type = NULL;
        self->priv->cube_equity_values = NULL;

        self->priv->cube_percents = NULL;
        self->priv->cube_win = NULL;
        self->priv->cube_win_g = NULL;
        self->priv->cube_win_bg = NULL;
        self->priv->cube_lose = NULL;
        self->priv->cube_lose_g = NULL;
        self->priv->cube_lose_bg = NULL;
        self->priv->cube_percents1 = NULL;
        self->priv->cube_win1 = NULL;
        self->priv->cube_win_g1 = NULL;
        self->priv->cube_win_bg1 = NULL;
        self->priv->cube_lose1 = NULL;
        self->priv->cube_lose_g1 = NULL;
        self->priv->cube_lose_bg1 = NULL;

        self->priv->action_1 = NULL;
        self->priv->action_2 = NULL;
        self->priv->action_3 = NULL;

        self->priv->eq_1 = NULL;
        self->priv->eq_2 = NULL;
        self->priv->eq_3 = NULL;
        self->priv->eq_delta_1 = NULL;
        self->priv->eq_delta_2 = NULL;
        self->priv->eq_delta_3 = NULL;

        self->priv->mwc_1 = NULL;
        self->priv->mwc_2 = NULL;
        self->priv->mwc_3 = NULL;
        self->priv->mwc_delta_1 = NULL;
        self->priv->mwc_delta_2 = NULL;
        self->priv->mwc_delta_3 = NULL;

        self->priv->proper_action = NULL;

        self->priv->ma = NULL;
}

static void
gibbon_analysis_view_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_analysis_view_parent_class)->finalize(object);
}

static void
gibbon_analysis_view_class_init (GibbonAnalysisViewClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonAnalysisViewPrivate));

        object_class->finalize = gibbon_analysis_view_finalize;
}

/**
 * gibbon_analysis_view_new:
 * @app: The GibbonApp.
 *
 * Creates a new #GibbonAnalysisView.
 *
 * Returns: The newly created #GibbonAnalysisView or %NULL in case of failure.
 */
GibbonAnalysisView *
gibbon_analysis_view_new (const GibbonApp *app)
{
        GibbonAnalysisView *self = g_object_new (GIBBON_TYPE_ANALYSIS_VIEW,
                                                 NULL);
        GObject *obj;
        GtkCellRenderer *renderer;

        self->priv->app = app;

        obj = gibbon_app_find_object (app, "hbox-analysis-detail",
                                      GTK_TYPE_BOX);
        gtk_widget_hide (GTK_WIDGET (obj));
        self->priv->detail_box = GTK_BOX (obj);

        obj = gibbon_app_find_object (app, "label-move-summary",
                                      GTK_TYPE_LABEL);
        self->priv->move_summary = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-summary",
                                      GTK_TYPE_LABEL);
        self->priv->cube_summary = GTK_LABEL (obj);

        obj = gibbon_app_find_object (app, "notebook-analysis",
                                      GTK_TYPE_NOTEBOOK);
        self->priv->notebook = GTK_NOTEBOOK (obj);

        obj = gibbon_app_find_object (app, "label-cube-equity-type",
                                      GTK_TYPE_LABEL);
        self->priv->cube_equity_type = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-equity-values",
                                      GTK_TYPE_LABEL);
        self->priv->cube_equity_values = GTK_LABEL (obj);

        obj = gibbon_app_find_object (app, "label-cube-percents",
                                      GTK_TYPE_LABEL);
        self->priv->cube_percents = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-win",
                                      GTK_TYPE_LABEL);
        self->priv->cube_win = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-win-g",
                                      GTK_TYPE_LABEL);
        self->priv->cube_win_g = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-win-bg",
                                      GTK_TYPE_LABEL);
        self->priv->cube_win_bg = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-lose",
                                      GTK_TYPE_LABEL);
        self->priv->cube_lose = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-lose-g",
                                      GTK_TYPE_LABEL);
        self->priv->cube_lose_g = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-lose-bg",
                                      GTK_TYPE_LABEL);
        self->priv->cube_lose_bg = GTK_LABEL (obj);

        obj = gibbon_app_find_object (app, "label-cube-percents1",
                                      GTK_TYPE_LABEL);
        self->priv->cube_percents1 = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-win1",
                                      GTK_TYPE_LABEL);
        self->priv->cube_win1 = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-win-g1",
                                      GTK_TYPE_LABEL);
        self->priv->cube_win_g1 = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-win-bg1",
                                      GTK_TYPE_LABEL);
        self->priv->cube_win_bg1 = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-lose1",
                                      GTK_TYPE_LABEL);
        self->priv->cube_lose1 = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-lose-g1",
                                      GTK_TYPE_LABEL);
        self->priv->cube_lose_g1 = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-lose-bg1",
                                      GTK_TYPE_LABEL);
        self->priv->cube_lose_bg1 = GTK_LABEL (obj);

        obj = gibbon_app_find_object (app, "label-cube-action1",
                                      GTK_TYPE_LABEL);
        self->priv->action_1 = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-action2",
                                      GTK_TYPE_LABEL);
        self->priv->action_2 = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-action3",
                                      GTK_TYPE_LABEL);
        self->priv->action_3 = GTK_LABEL (obj);

        obj = gibbon_app_find_object (app, "label-cube-eq1-l",
                                      GTK_TYPE_LABEL);
        self->priv->eq_1 = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-eq2-l",
                                      GTK_TYPE_LABEL);
        self->priv->eq_2 = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-eq3-l",
                                      GTK_TYPE_LABEL);
        self->priv->eq_3 = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-eq1-r",
                                      GTK_TYPE_LABEL);
        self->priv->eq_delta_1 = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-eq2-r",
                                      GTK_TYPE_LABEL);
        self->priv->eq_delta_2 = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-eq3-r",
                                      GTK_TYPE_LABEL);
        self->priv->eq_delta_3 = GTK_LABEL (obj);

        obj = gibbon_app_find_object (app, "label-cube-mwc1-l",
                                      GTK_TYPE_LABEL);
        self->priv->mwc_1 = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-mwc2-l",
                                      GTK_TYPE_LABEL);
        self->priv->mwc_2 = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-mwc3-l",
                                      GTK_TYPE_LABEL);
        self->priv->mwc_3 = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-mwc1-r",
                                      GTK_TYPE_LABEL);
        self->priv->mwc_delta_1 = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-mwc2-r",
                                      GTK_TYPE_LABEL);
        self->priv->mwc_delta_2 = GTK_LABEL (obj);
        obj = gibbon_app_find_object (app, "label-cube-mwc3-r",
                                      GTK_TYPE_LABEL);
        self->priv->mwc_delta_3 = GTK_LABEL (obj);

        obj = gibbon_app_find_object (app, "label-proper-cube-action",
                                      GTK_TYPE_LABEL);
        self->priv->proper_action = GTK_LABEL (obj);

        obj = gibbon_app_find_object (app, "variants-view",
                                      GTK_TYPE_TREE_VIEW);
        self->priv->variants_view = GTK_TREE_VIEW (obj);

        renderer = gtk_cell_renderer_text_new ();
        g_object_set (G_OBJECT (renderer),
                      "xalign", 1.0f,
                      NULL);
        gtk_tree_view_insert_column_with_attributes (
                        self->priv->variants_view,
                        -1, _("Rank"), renderer,
                        "text", GIBBON_VARIANT_LIST_COL_NUMBER,
                        "weight", GIBBON_VARIANT_LIST_COL_WEIGHT,
                        NULL);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_insert_column_with_attributes (
                        self->priv->variants_view,
                        -1, _("Move"), renderer,
                        "text", GIBBON_VARIANT_LIST_COL_MOVE,
                        "weight", GIBBON_VARIANT_LIST_COL_WEIGHT,
                        NULL);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_insert_column_with_attributes (
                        self->priv->variants_view,
                        -1, _("Type"), renderer,
                        "text", GIBBON_VARIANT_LIST_COL_ANALYSIS_TYPE,
                        "weight", GIBBON_VARIANT_LIST_COL_WEIGHT,
                        NULL);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_insert_column_with_data_func (
                        self->priv->variants_view,
                        -1, _("Equity"), renderer,
                        (GtkTreeCellDataFunc)
                        gibbon_analysis_view_equity_data_func,
                        self, NULL);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_insert_column_with_data_func (
                        self->priv->variants_view,
                        -1, _("\xce\x94"), renderer,
                        (GtkTreeCellDataFunc)
                        gibbon_analysis_view_equity_diff_data_func,
                        self, NULL);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_insert_column_with_data_func (
                        self->priv->variants_view,
                        -1, _("MWC"), renderer,
                        (GtkTreeCellDataFunc)
                        gibbon_analysis_view_mwc_data_func,
                        self, NULL);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_insert_column_with_data_func (
                        self->priv->variants_view,
                        -1, _("\xce\x94"), renderer,
                        (GtkTreeCellDataFunc)
                        gibbon_analysis_view_mwc_diff_data_func,
                        self, NULL);

        /*
         * Dummy column.  The rightmost column is right-aligned but we do not
         * want it to expand and fill all the remaining space.
         */
        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_insert_column_with_attributes (
                        self->priv->variants_view,
                        -1, NULL, renderer,
                        NULL);

        g_signal_connect_swapped (G_OBJECT (self->priv->variants_view),
                                  "query-tooltip",
                                  (GCallback)
                                  gibbon_analysis_view_on_query_tooltip,
                                  self);

        g_signal_connect_swapped (G_OBJECT (self->priv->variants_view),
                                  "cursor-changed",
                                  (GCallback)
                                  gibbon_analysis_view_on_cursor_changed,
                                  self);

        return self;
}

void
gibbon_analysis_view_fixup_layout (const GibbonAnalysisView *self)
{
        GtkWidget *nbw;
        GtkAllocation alloc;

        g_return_if_fail (GIBBON_IS_ANALYSIS_VIEW (self));

        /*
         * Set the minimum size of the notebook so that it can accommodate the
         * cube decision page of the notebook, no matter whether we display
         * a cube analysis or not.
         */
        nbw = GTK_WIDGET (self->priv->notebook);
        gtk_widget_show_all (nbw);
        gtk_widget_get_allocation (nbw, &alloc);
        gtk_widget_set_size_request (nbw, alloc.width, alloc.height);
        gtk_widget_hide (GTK_WIDGET (self->priv->notebook));

        self->priv->notebook_sized = TRUE;
}

void
gibbon_analysis_view_set_analysis (GibbonAnalysisView *self,
                                   const GibbonGame *game, gint action_number)
{
        gint i;
        const GibbonGameAction *action;
        GibbonAnalysis *analysis;
        GibbonAnalysisRoll *roll_analysis = NULL;
        GibbonAnalysisMove *move_analysis = NULL;
        GtkListStore *store;
        GtkWidget *cube_page, *move_page;

        g_return_if_fail (GIBBON_IS_ANALYSIS_VIEW (self));

        cube_page = gtk_notebook_get_nth_page (self->priv->notebook, 0);
        move_page = gtk_notebook_get_nth_page (self->priv->notebook, 1);

        if (action_number < 0) {
                gtk_widget_hide (GTK_WIDGET (self->priv->notebook));
                return;
        }

        /* First find the corresponding roll.  */
        for (i = action_number; i >= 0; --i) {
                action = gibbon_game_get_nth_action (game, i, NULL);
                if (GIBBON_IS_ROLL (action)) {
                        analysis = gibbon_game_get_nth_analysis (game, i);
                        roll_analysis = GIBBON_ANALYSIS_ROLL (analysis);
                        break;
                }
        }

        /* Then find the corresponding move.  */
        for (i = action_number; ; ++i) {
                action = gibbon_game_get_nth_action (game, i, NULL);
                if (!action)
                        break;
                if (GIBBON_IS_MOVE (action)
                    || GIBBON_IS_DOUBLE (action)
                    || GIBBON_IS_TAKE (action)
                    || GIBBON_IS_DROP (action)) {
                        analysis = gibbon_game_get_nth_analysis (game, i);
                        move_analysis = GIBBON_ANALYSIS_MOVE (analysis);
                        break;
                }
        }

        if (roll_analysis) {
                gibbon_analysis_view_set_roll (self, GIBBON_ANALYSIS_ROLL (
                                roll_analysis));
        } else {
                gtk_widget_hide (GTK_WIDGET (self->priv->detail_box));
        }

        if (!move_analysis) {
                gtk_widget_hide (GTK_WIDGET (self->priv->notebook));
                return;
        }

        gibbon_analysis_view_set_move (self, move_analysis);

        action = gibbon_game_get_nth_action (game, action_number, NULL);
        if (action) {
                if (GIBBON_IS_ROLL (action) && roll_analysis
                    && move_analysis
                    && move_analysis->da) {
                        gtk_notebook_set_current_page (self->priv->notebook, 0);
                } else if (move_analysis) {
                        gtk_notebook_set_current_page (self->priv->notebook, 1);
                }
        }

        if (move_analysis->ma_variants) {
                store = gibbon_variant_list_get_store (
                                move_analysis->ma_variants);
                gtk_tree_view_set_model (self->priv->variants_view,
                                         GTK_TREE_MODEL (store));
        } else {
                gtk_tree_view_set_model (self->priv->variants_view, NULL);
        }

        if (self->priv->notebook_sized) {
                if (move_analysis->ma && move_analysis->da) {
                        gtk_widget_show_all (GTK_WIDGET (self->priv->notebook));
                } else if (move_analysis->ma) {
                        gtk_widget_show (GTK_WIDGET (self->priv->notebook));
                        gtk_widget_hide (cube_page);
                        gtk_widget_show (move_page);
                } else if (move_analysis->da) {
                        gtk_widget_show (GTK_WIDGET (self->priv->notebook));
                        gtk_widget_show (cube_page);
                        gtk_widget_hide (move_page);
                } else {
                        gtk_widget_hide (GTK_WIDGET (self->priv->notebook));
                }
        }
}

static void
gibbon_analysis_view_set_move (GibbonAnalysisView *self,
                               GibbonAnalysisMove *a)
{
        gchar *buf;
        gint f;
        const GibbonMET *met;
        gdouble *p, *p2;
        gdouble money_equity, mwc;
        gdouble equity;
        gdouble p_nodouble, p_take, p_drop, p_optimal;
        gdouble eq_nodouble, eq_take, eq_drop, eq_optimal;

        if (self->priv->ma)
                g_object_unref (self->priv->ma);
        self->priv->ma = g_object_ref (a);

        gtk_widget_show (GTK_WIDGET (self->priv->detail_box));
        gtk_widget_show (GTK_WIDGET (self->priv->notebook));

        p = a->da_p[0];

        buf = g_strdup_printf (MWC_FORMAT,
                               100 * p[GIBBON_ANALYSIS_MOVE_PWIN]);
        if (a->da_take_analysis)
                gtk_label_set_text (self->priv->cube_lose, buf);
        else
                gtk_label_set_text (self->priv->cube_win, buf);
        g_free (buf);

        buf = g_strdup_printf (MWC_FORMAT,
                               100 * p[GIBBON_ANALYSIS_MOVE_PWIN_GAMMON]);
        if (a->da_take_analysis)
                gtk_label_set_text (self->priv->cube_lose_g, buf);
        else
                gtk_label_set_text (self->priv->cube_win_g, buf);
        g_free (buf);

        buf = g_strdup_printf (MWC_FORMAT,
                               100 * p[GIBBON_ANALYSIS_MOVE_PWIN_BACKGAMMON]);
        if (a->da_take_analysis)
                gtk_label_set_text (self->priv->cube_lose_bg, buf);
        else
                gtk_label_set_text (self->priv->cube_win_bg, buf);
        g_free (buf);

        buf = g_strdup_printf (MWC_FORMAT, 100 * (1 - p[0]));
        if (a->da_take_analysis)
                gtk_label_set_text (self->priv->cube_win, buf);
        else
                gtk_label_set_text (self->priv->cube_lose, buf);
        g_free (buf);

        buf = g_strdup_printf (MWC_FORMAT, 100 * p[3]);
        if (a->da_take_analysis)
                gtk_label_set_text (self->priv->cube_win_g, buf);
        else
                gtk_label_set_text (self->priv->cube_lose_g, buf);
        g_free (buf);

        buf = g_strdup_printf ("%.2f%%", 100 * p[4]);
        if (a->da_take_analysis)
                gtk_label_set_text (self->priv->cube_win_bg, buf);
        else
                gtk_label_set_text (self->priv->cube_lose_bg, buf);
        g_free (buf);

        if (a->da_rollout) {
                p2 = a->da_p[1];

                buf = g_strdup_printf (MWC_FORMAT,
                                       100 * p2[GIBBON_ANALYSIS_MOVE_PWIN]);
                if (a->da_take_analysis)
                        gtk_label_set_text (self->priv->cube_lose1, buf);
                else
                        gtk_label_set_text (self->priv->cube_win1, buf);
                g_free (buf);

                buf = g_strdup_printf (MWC_FORMAT,
                                 100 * p2[GIBBON_ANALYSIS_MOVE_PWIN_GAMMON]);
                if (a->da_take_analysis)
                        gtk_label_set_text (self->priv->cube_lose_g1, buf);
                else
                        gtk_label_set_text (self->priv->cube_win_g1, buf);
                g_free (buf);

                buf = g_strdup_printf (MWC_FORMAT,
                                100 * p2[GIBBON_ANALYSIS_MOVE_PWIN_BACKGAMMON]);
                if (a->da_take_analysis)
                        gtk_label_set_text (self->priv->cube_lose_bg1, buf);
                else
                        gtk_label_set_text (self->priv->cube_win_bg1, buf);
                g_free (buf);

                buf = g_strdup_printf (MWC_FORMAT, 100 * (1 - p2[0]));
                if (a->da_take_analysis)
                        gtk_label_set_text (self->priv->cube_win1, buf);
                else
                        gtk_label_set_text (self->priv->cube_lose1, buf);
                g_free (buf);

                buf = g_strdup_printf (MWC_FORMAT, 100 * p2[3]);
                if (a->da_take_analysis)
                        gtk_label_set_text (self->priv->cube_win_g1, buf);
                else
                        gtk_label_set_text (self->priv->cube_lose_g1, buf);
                g_free (buf);

                buf = g_strdup_printf (MWC_FORMAT, 100 * p2[4]);
                if (a->da_take_analysis)
                        gtk_label_set_text (self->priv->cube_win_bg1, buf);
                else
                        gtk_label_set_text (self->priv->cube_lose_bg1, buf);
                g_free (buf);
                gtk_widget_show (GTK_WIDGET (self->priv->cube_percents));
                gtk_widget_show (GTK_WIDGET (self->priv->cube_percents1));
                gtk_widget_show (GTK_WIDGET (self->priv->cube_win1));
                gtk_widget_show (GTK_WIDGET (self->priv->cube_win_g1));
                gtk_widget_show (GTK_WIDGET (self->priv->cube_win_bg1));
                gtk_widget_show (GTK_WIDGET (self->priv->cube_lose1));
                gtk_widget_show (GTK_WIDGET (self->priv->cube_lose_g1));
                gtk_widget_show (GTK_WIDGET (self->priv->cube_lose_bg1));
        } else {
                gtk_widget_hide (GTK_WIDGET (self->priv->cube_percents));
                gtk_widget_hide (GTK_WIDGET (self->priv->cube_percents1));
                gtk_widget_hide (GTK_WIDGET (self->priv->cube_win1));
                gtk_widget_hide (GTK_WIDGET (self->priv->cube_win_g1));
                gtk_widget_hide (GTK_WIDGET (self->priv->cube_win_bg1));
                gtk_widget_hide (GTK_WIDGET (self->priv->cube_lose1));
                gtk_widget_hide (GTK_WIDGET (self->priv->cube_lose_g1));
                gtk_widget_hide (GTK_WIDGET (self->priv->cube_lose_bg1));
        }

        if (a->da_rollout) {
                buf = g_strdup_printf (_("Cubeless rollout (%llu trials):"),
                                       (unsigned long long) a->da_trials);
        } else {
                buf = g_strdup_printf (_("Cubeless %llu-ply evaluation:"),
                                       (unsigned long long) a->da_plies);
        }
        gtk_label_set_text (self->priv->cube_equity_type, buf);
        g_free (buf);

        f = a->da_take_analysis ? -1 : 1;
        met = gibbon_app_get_met (self->priv->app);

        equity = p[GIBBON_ANALYSIS_MOVE_EQUITY];
        money_equity = f * (2.0f * p[GIBBON_ANALYSIS_MOVE_PWIN]
            -1.0f + p[GIBBON_ANALYSIS_MOVE_PWIN_GAMMON]
            + p[GIBBON_ANALYSIS_MOVE_PWIN_BACKGAMMON]
            - p[GIBBON_ANALYSIS_MOVE_PLOSE_GAMMON]
            - p[GIBBON_ANALYSIS_MOVE_PLOSE_BACKGAMMON]);

        if (a->match_length > 0) {
                mwc = 100.0f * gibbon_met_eq2mwc (met, equity,
                                                  a->match_length,
                                                  a->cube,
                                                  a->my_score,
                                                  a->opp_score);
                if (a->da_take_analysis)
                        mwc = 100.0f - mwc;
                buf = g_strdup_printf (_("Equity: %+.3f (Money: %+.3f),"
                                         " MWC: %.2f"),
                                       equity, money_equity, mwc);
        } else {
                buf = g_strdup_printf (_("Equity: %+.3f (Money: %+.3f)"),
                        equity, money_equity);
        }
        gtk_label_set_text (self->priv->cube_equity_values, buf);
        g_free (buf);

        /*
         * Cubeful equities.
         */
        p_nodouble = a->da_p[0][GIBBON_ANALYSIS_MOVE_CUBEFUL_EQUITY];
        p_take = a->da_p[1][GIBBON_ANALYSIS_MOVE_CUBEFUL_EQUITY];
        if (a->da_take_analysis) {
                eq_nodouble = gibbon_met_mwc2eq (met, p_nodouble,
                                                 a->match_length, a->cube,
                                                 a->opp_score, a->my_score);
                eq_take = gibbon_met_mwc2eq (met, p_take,
                                             a->match_length, a->cube,
                                             a->opp_score, a->my_score);
                p_drop = gibbon_met_get_match_equity (met, a->match_length,
                                                      a->cube, a->opp_score,
                                                      a->my_score);
        } else {
                eq_nodouble = gibbon_met_mwc2eq (met, p_nodouble,
                                                 a->match_length, a->cube,
                                                 a->my_score, a->opp_score);
                eq_take = gibbon_met_mwc2eq (met, p_take,
                                             a->match_length, a->cube,
                                             a->my_score, a->opp_score);
                p_drop = gibbon_met_get_match_equity (met, a->match_length,
                                                      a->cube, a->my_score,
                                                      a->opp_score);
        }
        eq_drop = 1.0f;

        gtk_label_set_text (self->priv->action_1, _("No double"));
        gtk_label_set_text (self->priv->action_2, _("Double, take"));
        gtk_label_set_text (self->priv->action_3, _("Double, drop"));

        if (a->da_take_analysis) {
                gtk_label_set_text (self->priv->eq_1, NULL);
        } else {
                buf = g_strdup_printf (EQ_FORMAT, f * eq_nodouble);
                gtk_label_set_text (self->priv->eq_1, buf);
                g_free (buf);
        }

        buf = g_strdup_printf (EQ_FORMAT, f * eq_take);
        gtk_label_set_text (self->priv->eq_2, buf);
        g_free (buf);
        buf = g_strdup_printf (EQ_FORMAT, f * eq_drop);
        gtk_label_set_text (self->priv->eq_3, buf);
        g_free (buf);

        if (a->match_length > 0) {
                /* Display MWC.  */
                if (a->da_take_analysis) {
                        gtk_label_set_text (self->priv->mwc_1, NULL);
                        buf = g_strdup_printf (MWC_FORMAT, 100 - 100 * p_take);
                        gtk_label_set_text (self->priv->mwc_2, buf);
                        g_free (buf);
                        buf = g_strdup_printf (MWC_FORMAT, 100 - 100 * p_drop);
                        gtk_label_set_text (self->priv->mwc_3, buf);
                        g_free (buf);
                } else {
                        buf = g_strdup_printf (MWC_FORMAT, 100 * p_nodouble);
                        gtk_label_set_text (self->priv->mwc_1, buf);
                        g_free (buf);
                        buf = g_strdup_printf (MWC_FORMAT, 100 * p_take);
                        gtk_label_set_text (self->priv->mwc_2, buf);
                        g_free (buf);
                        buf = g_strdup_printf (MWC_FORMAT, 100 * p_drop);
                        gtk_label_set_text (self->priv->mwc_3, buf);
                        g_free (buf);
                }
        } else {
                gtk_label_set_text (self->priv->mwc_1, NULL);
                gtk_label_set_text (self->priv->mwc_2, NULL);
                gtk_label_set_text (self->priv->mwc_3, NULL);
        }

        if (eq_take > eq_nodouble) {
                if (eq_drop < eq_take) {
                        eq_optimal = eq_drop;
                        p_optimal = p_drop;
                } else {
                        eq_optimal = eq_take;
                }
        } else {
                eq_optimal = eq_nodouble;
                p_optimal = p_nodouble;
        }

        if (a->da_take_analysis) {
                gtk_label_set_text (self->priv->eq_delta_1, NULL);
        } else {
                if (eq_nodouble == eq_optimal) {
                        gtk_label_set_text (self->priv->eq_delta_1, NULL);
                } else {
                        buf = g_strdup_printf (EQ_DIFF_FORMAT,
                                               f * (eq_nodouble - eq_optimal));
                        gtk_label_set_text (self->priv->eq_delta_1, buf);
                        g_free (buf);
                }
        }

        if (a->match_length > 0 ) {
                if (a->da_take_analysis) {
                        gtk_label_set_text (self->priv->eq_delta_1, NULL);
                } else {
                        if (p_nodouble == p_optimal) {
                                gtk_label_set_text (
                                        self->priv->mwc_delta_1, NULL);
                        } else {
                                buf = g_strdup_printf (
                                        MWC_DIFF_FORMAT,
                                        100 * f
                                        * (p_nodouble - p_optimal));
                                gtk_label_set_text (
                                        self->priv->mwc_delta_1, buf);
                                g_free (buf);
                        }
                }
        } else {
                gtk_label_set_text (self->priv->eq_delta_1, NULL);
        }

        if (eq_take == eq_optimal) {
                gtk_label_set_text (self->priv->eq_delta_2, NULL);
        } else {
                buf = g_strdup_printf (EQ_DIFF_FORMAT,
                                       f * (eq_take - eq_optimal));
                gtk_label_set_text (self->priv->eq_delta_2, buf);
                g_free (buf);
        }

        if (a->match_length > 0) {
                if (p_take == p_optimal) {
                        gtk_label_set_text (self->priv->mwc_delta_2, NULL);
                } else {
                        buf = g_strdup_printf (MWC_DIFF_FORMAT,
                                               100 * f * (p_take - p_optimal));
                        gtk_label_set_text (self->priv->mwc_delta_2, buf);
                        g_free (buf);
                }
        } else {
                gtk_label_set_text (self->priv->eq_delta_2, NULL);
        }

        if (eq_drop == eq_optimal) {
                gtk_label_set_text (self->priv->eq_delta_3, NULL);
        } else {
                buf = g_strdup_printf (EQ_DIFF_FORMAT,
                                       f * (eq_drop - eq_optimal));
                gtk_label_set_text (self->priv->eq_delta_3, buf);
                g_free (buf);
        }

        if (a->match_length > 0) {
                if (p_drop == p_optimal) {
                        gtk_label_set_text (self->priv->mwc_delta_3, NULL);
                } else {
                        buf = g_strdup_printf (MWC_DIFF_FORMAT,
                                               100 * f * (p_drop - p_optimal));
                        gtk_label_set_text (self->priv->mwc_delta_3, buf);
                        g_free (buf);
                }
        } else {
                gtk_label_set_text (self->priv->eq_delta_3, NULL);
        }

        if (a->da_take_analysis)
                buf = gibbon_analysis_move_take_decision (a, eq_nodouble,
                                                          eq_take, eq_drop);
        else
                buf = gibbon_analysis_move_cube_decision (a, eq_nodouble,
                                                          eq_take, eq_drop);
        gtk_label_set_text (self->priv->proper_action, buf);
        g_free (buf);
}

static void
gibbon_analysis_view_set_roll (GibbonAnalysisView *self, GibbonAnalysisRoll *a)
{
        GibbonAnalysisRollLuck luck_type;
        gchar *text;
        gdouble luck_value;

        gtk_widget_show (GTK_WIDGET (self->priv->detail_box));
        gtk_widget_hide (GTK_WIDGET (self->priv->notebook));

        gtk_label_set_text (self->priv->move_summary, NULL);

        luck_type = gibbon_analysis_roll_get_luck_type (a);
        luck_value = gibbon_analysis_roll_get_luck_value (a);
        switch (luck_type) {
        default:
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

        gtk_label_set_text (self->priv->cube_summary, text);
        g_free (text);
}

static void
gibbon_analysis_view_equity_data_func (GtkTreeViewColumn *tree_column,
                                       GtkCellRenderer *cell,
                                       GtkTreeModel *tree_model,
                                       GtkTreeIter *iter,
                                       GibbonAnalysisView *self)
{
        PangoWeight weight;
        gdouble equity;
        gchar *formatted_equity;

        gtk_tree_model_get (tree_model, iter,
                            GIBBON_VARIANT_LIST_COL_WEIGHT,
                            &weight,
                            GIBBON_VARIANT_LIST_COL_EQUITY,
                            &equity,
                            -1);

        formatted_equity = g_strdup_printf (EQ_FORMAT, equity);

        g_object_set (cell,
                      "text", formatted_equity,
                      "weight", weight,
                      "xalign", 1.0f,
                      NULL);

        g_free (formatted_equity);
}

static void
gibbon_analysis_view_equity_diff_data_func (GtkTreeViewColumn *tree_column,
                                            GtkCellRenderer *cell,
                                            GtkTreeModel *tree_model,
                                            GtkTreeIter *iter,
                                            GibbonAnalysisView *self)
{
        PangoWeight weight;
        gdouble equity;
        GtkTreeIter best_iter;
        gdouble best_equity;
        gchar *formatted_equity_diff = NULL;
        guint rank;

        gtk_tree_model_get (tree_model, iter,
                            GIBBON_VARIANT_LIST_COL_NUMBER,
                            &rank,
                            GIBBON_VARIANT_LIST_COL_WEIGHT,
                            &weight,
                            GIBBON_VARIANT_LIST_COL_EQUITY,
                            &equity,
                            -1);

        if (rank != 1
            && gtk_tree_model_get_iter_first (tree_model, &best_iter)) {
                gtk_tree_model_get (tree_model, &best_iter,
                                    GIBBON_VARIANT_LIST_COL_EQUITY,
                                    &best_equity,
                                    -1);

                if (best_equity != equity)
                        formatted_equity_diff = g_strdup_printf (
                                        EQ_FORMAT, equity - best_equity);
        }

        g_object_set (cell,
                      "text", formatted_equity_diff,
                      "weight", weight,
                      "xalign", 1.0f,
                      NULL);

        g_free (formatted_equity_diff);
}

static void
gibbon_analysis_view_mwc_data_func (GtkTreeViewColumn *tree_column,
                                    GtkCellRenderer *cell,
                                    GtkTreeModel *tree_model,
                                    GtkTreeIter *iter,
                                    GibbonAnalysisView *self)
{
        PangoWeight weight;
        gdouble equity;
        guint match_length;
        guint cube, scores[2];
        gdouble mwc;
        gchar *formatted_mwc;

        gtk_tree_model_get (tree_model, iter,
                            GIBBON_VARIANT_LIST_COL_WEIGHT, &weight,
                            GIBBON_VARIANT_LIST_COL_EQUITY, &equity,
                            GIBBON_VARIANT_LIST_COL_MATCH_LENGTH, &match_length,
                            GIBBON_VARIANT_LIST_COL_CUBE, &cube,
                            GIBBON_VARIANT_LIST_COL_MY_SCORE, &scores[0],
                            GIBBON_VARIANT_LIST_COL_OPP_SCORE, &scores[1],
                            -1);
        mwc = gibbon_met_eq2mwc (gibbon_app_get_met (app), equity,
                                 match_length, cube, scores[0], scores[1]);
        formatted_mwc = g_strdup_printf ("%.2f %%", 100 * mwc);

        g_object_set (cell,
                      "text", formatted_mwc,
                      "weight", weight,
                      "xalign", 1.0f,
                      NULL);

        g_free (formatted_mwc);
}

static void
gibbon_analysis_view_mwc_diff_data_func (GtkTreeViewColumn *tree_column,
                                         GtkCellRenderer *cell,
                                         GtkTreeModel *tree_model,
                                         GtkTreeIter *iter,
                                         GibbonAnalysisView *self)
{
        PangoWeight weight;
        gdouble equity;
        gdouble mwc;
        guint match_length;
        guint cube, scores[2];
        GtkTreeIter best_iter;
        gdouble best_equity;
        gdouble best_mwc;
        gchar *formatted_mwc_diff = NULL;
        guint rank;

        gtk_tree_model_get (tree_model, iter,
                            GIBBON_VARIANT_LIST_COL_NUMBER, &rank,
                            GIBBON_VARIANT_LIST_COL_WEIGHT, &weight,
                            GIBBON_VARIANT_LIST_COL_EQUITY, &equity,
                            GIBBON_VARIANT_LIST_COL_MATCH_LENGTH, &match_length,
                            GIBBON_VARIANT_LIST_COL_CUBE, &cube,
                            GIBBON_VARIANT_LIST_COL_MY_SCORE, &scores[0],
                            GIBBON_VARIANT_LIST_COL_OPP_SCORE, &scores[1],
                            -1);

        if (rank != 1
            && gtk_tree_model_get_iter_first (tree_model, &best_iter)) {
                gtk_tree_model_get (tree_model, &best_iter,
                                    GIBBON_VARIANT_LIST_COL_EQUITY,
                                    &best_equity,
                                    -1);

                if (best_equity != equity) {
                        mwc = gibbon_met_eq2mwc (gibbon_app_get_met (app),
                                                 equity, match_length, cube,
                                                 scores[0], scores[1]);
                        best_mwc = gibbon_met_eq2mwc (gibbon_app_get_met (app),
                                                      best_equity, match_length,
                                                      cube,
                                                      scores[0], scores[1]);
                        formatted_mwc_diff = g_strdup_printf (
                                        "%.2f %%", 100 * (mwc - best_mwc));
                }
        }

        g_object_set (cell,
                      "text", formatted_mwc_diff,
                      "weight", weight,
                      "xalign", 1.0f,
                      NULL);

        g_free (formatted_mwc_diff);
}

static gboolean
gibbon_analysis_view_on_query_tooltip (const GibbonAnalysisView *self,
                                       gint x, gint y,
                                       gboolean keyboard_tip,
                                       GtkTooltip *tooltip,
                                       GtkTreeView *view)
{
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;
        gchar *text = NULL;
        guint row_num;
        gdouble p[6];

        g_return_val_if_fail (GIBBON_IS_ANALYSIS_VIEW (self), FALSE);
        g_return_val_if_fail (GTK_IS_TREE_VIEW (view), FALSE);
        g_return_val_if_fail (view == self->priv->variants_view, FALSE);
        g_return_val_if_fail (GTK_IS_TOOLTIP (tooltip), FALSE);

        if (!gtk_tree_view_get_tooltip_context (view, &x, &y,
                                                keyboard_tip,
                                                &model, &path, &iter))
                return FALSE;

        gtk_tree_model_get (model, &iter,
                            GIBBON_VARIANT_LIST_COL_NUMBER, &row_num,
                            GIBBON_VARIANT_LIST_COL_PWIN, &p[0],
                            GIBBON_VARIANT_LIST_COL_PWIN_G, &p[1],
                            GIBBON_VARIANT_LIST_COL_PWIN_BG, &p[2],
                            GIBBON_VARIANT_LIST_COL_PLOSE, &p[3],
                            GIBBON_VARIANT_LIST_COL_PLOSE_G, &p[4],
                            GIBBON_VARIANT_LIST_COL_PLOSE_BG, &p[5],
                            -1);

        if (!row_num) {
                /* Should not happen ... */
                gtk_tree_path_free (path);
                return FALSE;
        }

        /*
         * TRANSLATORS: This is the tooltip displayed for a move variant.  The
         * six numbers give the probabilities for winning, winning a gammon,
         * winning a backgammon, and losing, losing a gammon, and losing a
         * backgammon.
         */
        text = g_strdup_printf ("%.2f%% %.2f%% %.2f%% - %.2f%% %.2f%% %.2f%%",
                                100.f * p[0], 100.f * p[1], 100.f * p[2],
                                100.f * p[3], 100.f * p[4], 100.f * p[5]);
        gtk_tooltip_set_text (tooltip, text);
        gtk_tree_view_set_tooltip_row (view, tooltip, path);
        g_free (text);

        gtk_tree_path_free (path);

        return TRUE;
}

static void
gibbon_analysis_view_on_cursor_changed (GibbonAnalysisView *self,
                                        GtkTreeView *view)
{
        GtkTreeSelection *selection;
        GList *selected;
        GtkTreePath *path;
        GtkTreeIter iter;
        GtkTreeModel *model = NULL;
        GibbonPosition *pos = NULL;
        GibbonBoard *board;

        g_return_if_fail (GIBBON_IS_ANALYSIS_VIEW (self));
        g_return_if_fail (GTK_IS_TREE_VIEW (view));
        g_return_if_fail (view == self->priv->variants_view);

        selection = gtk_tree_view_get_selection (view);
        if (!selection)
                return;

        selected = gtk_tree_selection_get_selected_rows (selection, &model);
        if (!selected)
                return;
        g_return_if_fail (model != NULL);

        path = (GtkTreePath *) selected->data;
        if (gtk_tree_model_get_iter (model, &iter, path)) {
                gtk_tree_model_get (model, &iter,
                                    GIBBON_VARIANT_LIST_COL_POSITION, &pos,
                                    -1);
        }
        g_list_foreach (selected, (GFunc) gtk_tree_path_free, NULL);
        g_list_free (selected);

        if (!pos)
                return;

        board = gibbon_app_get_board (app);
        gibbon_board_set_position (board, pos);

        gibbon_position_free (pos);
}
