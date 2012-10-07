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

#ifndef _GIBBON_ANALYSIS_MOVE_H
# define _GIBBON_ANALYSIS_MOVE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>

#include "gibbon-analysis.h"
#include "gibbon-variant-list.h"

#define GIBBON_TYPE_ANALYSIS_MOVE \
        (gibbon_analysis_move_get_type ())
#define GIBBON_ANALYSIS_MOVE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_ANALYSIS_MOVE, \
                GibbonAnalysisMove))
#define GIBBON_ANALYSIS_MOVE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_ANALYSIS_MOVE, GibbonAnalysisMoveClass))
#define GIBBON_IS_ANALYSIS_MOVE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_ANALYSIS_MOVE))
#define GIBBON_IS_ANALYSIS_MOVE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_ANALYSIS_MOVE))
#define GIBBON_ANALYSIS_MOVE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_ANALYSIS_MOVE, GibbonAnalysisMoveClass))

enum {
        GIBBON_ANALYSIS_MOVE_PWIN = 0,
        GIBBON_ANALYSIS_MOVE_PWIN_GAMMON,
        GIBBON_ANALYSIS_MOVE_PWIN_BACKGAMMON,
        GIBBON_ANALYSIS_MOVE_PLOSE_GAMMON,
        GIBBON_ANALYSIS_MOVE_PLOSE_BACKGAMMON,
        GIBBON_ANALYSIS_MOVE_EQUITY,
        GIBBON_ANALYSIS_MOVE_CUBEFUL_EQUITY,
};

/**
 * GibbonAnalysisMove:
 *
 * Collected analysis data for a backgammon move.
 */
typedef struct _GibbonAnalysisMove GibbonAnalysisMove;
struct _GibbonAnalysisMove
{
        GibbonAnalysis parent_instance;

        gsize match_length;
        guint cube;
        guint my_score, opp_score;
        gboolean crawford;
        gboolean beavers;
        gboolean jacoby;
        gboolean is_crawford, post_crawford;
        gboolean may_double;
        gboolean opp_may_double;

        gboolean ma;
        guint ma_bad;

        /*
         * Index of the actual move made into the following move list(s).
         */
        guint64 ma_imove;
        /* List of GibbonAnalysisMoveRecord objects.  */
        GibbonVariantList *ma_variants;

        gboolean da;
        guint da_bad;
        gboolean da_rollout;
        guint64 da_plies;
        gboolean da_cubeful;
        gboolean da_deterministic;
        gdouble da_noise;
        gboolean da_use_prune;
        gdouble da_p[2][7];
        guint64 da_trials;
        gboolean da_take_analysis;

        /*< private >*/
        struct _GibbonAnalysisMovePrivate *priv;
};

/**
 * GibbonAnalysisMoveClass:
 *
 * Class definition for a #GibbonAnalysisMove.!
 */
typedef struct _GibbonAnalysisMoveClass GibbonAnalysisMoveClass;
struct _GibbonAnalysisMoveClass
{
        /* <private >*/
        GibbonAnalysisClass parent_class;
};

GType gibbon_analysis_move_get_type (void) G_GNUC_CONST;

GibbonAnalysisMove *gibbon_analysis_move_new ();
gchar *gibbon_analysis_move_cube_decision (GibbonAnalysisMove *self,
                                           gdouble eq_nodouble,
                                           gdouble eq_take, gdouble eq_drop);
gchar *gibbon_analysis_move_take_decision (GibbonAnalysisMove *self,
                                           gdouble eq_nodouble,
                                           gdouble eq_take, gdouble eq_drop);

#endif
