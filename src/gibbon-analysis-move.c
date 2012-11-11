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
 * SECTION:gibbon-analysis-move
 * @short_description: Collected analysis data for a backgammon move!
 *
 * Since: 0.2.0
 *
 * A #GibbonAnalysisMove holds all data collected by a backgammon evaluation
 * about a particular move.  Despite the name, this also includes the
 * analysis of an implicit doubling decision.
 */

#include <math.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-analysis-move.h"
#include "gibbon-util.h"

typedef struct _GibbonAnalysisMovePrivate GibbonAnalysisMovePrivate;
struct _GibbonAnalysisMovePrivate {
        gint dummy;
};

#define GIBBON_ANALYSIS_MOVE_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_ANALYSIS_MOVE, GibbonAnalysisMovePrivate))

G_DEFINE_TYPE (GibbonAnalysisMove, gibbon_analysis_move, GIBBON_TYPE_ANALYSIS)

/*
 * Taken from GNUBG.
 */
typedef enum {
        GIBBON_ANALYSIS_MOVE_CD_DOUBLE_TAKE,
        GIBBON_ANALYSIS_MOVE_CD_DOUBLE_PASS,
        GIBBON_ANALYSIS_MOVE_CD_NODOUBLE_TAKE,
        GIBBON_ANALYSIS_MOVE_CD_TOOGOOD_TAKE,
        GIBBON_ANALYSIS_MOVE_CD_TOOGOOD_PASS,
        GIBBON_ANALYSIS_MOVE_CD_DOUBLE_BEAVER,
        GIBBON_ANALYSIS_MOVE_CD_NODOUBLE_BEAVER,
        GIBBON_ANALYSIS_MOVE_CD_REDOUBLE_TAKE,
        GIBBON_ANALYSIS_MOVE_CD_REDOUBLE_PASS,
        GIBBON_ANALYSIS_MOVE_CD_NO_REDOUBLE_TAKE,
        GIBBON_ANALYSIS_MOVE_CD_TOOGOODRE_TAKE,
        GIBBON_ANALYSIS_MOVE_CD_TOOGOODRE_PASS,
        GIBBON_ANALYSIS_MOVE_CD_NO_REDOUBLE_BEAVER,
        /* Cube is dead (match play only).  */
        GIBBON_ANALYSIS_MOVE_CD_NODOUBLE_DEADCUBE,
        /* Cube is dead (match play only).  */
        GIBBON_ANALYSIS_MOVE_CD_NO_REDOUBLE_DEADCUBE,
        /* Cube is not available.  */
        GIBBON_ANALYSIS_MOVE_CD_NOT_AVAILABLE,
        GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_DOUBLE_TAKE,
        GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_REDOUBLE_TAKE,
        GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_DOUBLE_BEAVER,
        GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_DOUBLE_PASS,
        GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_REDOUBLE_PASS
} GibbonAnalysisMoveCubeDecision;

#define GIBBON_ANALYSIS_MOVE_IS_OPTIONAL(d1, d2) (fabs (d1 - d2) <= 0.00001)

static void 
gibbon_analysis_move_init (GibbonAnalysisMove *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_ANALYSIS_MOVE, GibbonAnalysisMovePrivate);

        self->ma = FALSE;
        self->ma_variants = NULL;

        self->da = FALSE;
        self->da_bad = 0;
        self->da_rollout = FALSE;
        self->da_plies = 0;
        self->da_cubeful = FALSE;
        self->da_use_prune = FALSE;
        self->da_deterministic = FALSE;
        self->da_trials = 0;
        self->da_take_analysis = FALSE;
}

static void
gibbon_analysis_move_finalize (GObject *object)
{
        GibbonAnalysisMove *self = GIBBON_ANALYSIS_MOVE (object);

        if (self->ma_variants)
                g_object_unref (self->ma_variants);

        G_OBJECT_CLASS (gibbon_analysis_move_parent_class)->finalize (object);
}

static void
gibbon_analysis_move_class_init (GibbonAnalysisMoveClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonAnalysisMovePrivate));

        object_class->finalize = gibbon_analysis_move_finalize;
}

/**
 * gibbon_analysis_move_new:
 *
 * Creates a new #GibbonAnalysisMove with default values.
 *
 * Returns: The newly created #GibbonAnalysisMove.
 */
GibbonAnalysisMove *
gibbon_analysis_move_new ()
{
        GibbonAnalysisMove *self = g_object_new (GIBBON_TYPE_ANALYSIS_MOVE,
                                                 NULL);

        return self;
}

/*
 * Find the best cube decision.  The function closely follows
 * FindBestCubeDecision() in GNUBG because they are supposed to trigger the
 * same results.
 *
 * NB! Some results produced by this function can never happen.  For example
 * FIBS does not support the Jacoby rule, and we should never see a cube
 * that is not available (unless dead).
 *
 * FIXME! A bit mask is better suited for the cube decision.
 */
static GibbonAnalysisMoveCubeDecision
_gibbon_analysis_move_cube_decision (GibbonAnalysisMove *self,
                                     gdouble eq_nodouble, gdouble eq_take,
                                     gdouble eq_drop)
{
        gboolean is_optional;

        if (!self->may_double)
                return GIBBON_ANALYSIS_MOVE_CD_NOT_AVAILABLE;

        if (self->match_length > 0) {
                if (self->my_score + self->cube >= self->match_length) {
                        return self->cube > 1 ?
                                GIBBON_ANALYSIS_MOVE_CD_NODOUBLE_DEADCUBE
                                : GIBBON_ANALYSIS_MOVE_CD_NO_REDOUBLE_DEADCUBE;
                }
        }

        if (eq_take > eq_nodouble && eq_drop >= eq_nodouble) {
                /* Double.  */
                if (eq_drop > eq_take) {
                        /* Double, take.  */
                        is_optional = GIBBON_ANALYSIS_MOVE_IS_OPTIONAL (eq_take,
                                                                        eq_nodouble);
                        if (self->beavers
                            && self->match_length < 1
                            && eq_take >= -2.0f
                            && eq_take <= 0.0f) {
                                if (2.0f * eq_take < eq_nodouble)
                                        return GIBBON_ANALYSIS_MOVE_CD_NODOUBLE_BEAVER;
                                else
                                        return is_optional ?
                                               GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_DOUBLE_BEAVER
                                               : GIBBON_ANALYSIS_MOVE_CD_DOUBLE_BEAVER;
                        } else if (is_optional) {
                                return self->cube < 2 ?
                                        GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_DOUBLE_TAKE
                                        : GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_REDOUBLE_TAKE;
                        } else {
                                return self->cube < 2 ?
                                        GIBBON_ANALYSIS_MOVE_CD_DOUBLE_TAKE
                                        : GIBBON_ANALYSIS_MOVE_CD_REDOUBLE_TAKE;
                        }
                } else {
                        /* Double, drop.  */
                        is_optional = GIBBON_ANALYSIS_MOVE_IS_OPTIONAL (eq_drop,
                                                                        eq_nodouble);
                        if (is_optional &&
                            self->da_p[0][GIBBON_ANALYSIS_MOVE_PWIN_GAMMON]
                            > 0.0f
                            && (self->match_length > 0 || self->opp_may_double
                                || !self->jacoby))
                                return self->cube < 2 ?
                                        GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_DOUBLE_PASS
                                        : GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_REDOUBLE_PASS;
                        else
                                return self->cube < 2 ?
                                        GIBBON_ANALYSIS_MOVE_CD_DOUBLE_PASS
                                        : GIBBON_ANALYSIS_MOVE_CD_REDOUBLE_PASS;
                }
        } else {
                /* No double.  */
                if (eq_nodouble > eq_take) {
                        if (eq_take > eq_drop) {
                                if (self->da_p[0][GIBBON_ANALYSIS_MOVE_PWIN_GAMMON]) {
                                        return self->cube < 2 ?
                                                GIBBON_ANALYSIS_MOVE_CD_TOOGOOD_PASS
                                                : GIBBON_ANALYSIS_MOVE_CD_TOOGOODRE_PASS;
                                } else {
                                        return self->cube < 2 ?
                                                GIBBON_ANALYSIS_MOVE_CD_DOUBLE_PASS
                                                : GIBBON_ANALYSIS_MOVE_CD_REDOUBLE_PASS;
                                }
                        } else if (eq_nodouble > eq_drop) {
                                if (self->da_p[0][GIBBON_ANALYSIS_MOVE_PWIN_GAMMON]) {
                                        return self->cube < 2 ?
                                                GIBBON_ANALYSIS_MOVE_CD_TOOGOOD_TAKE
                                                : GIBBON_ANALYSIS_MOVE_CD_TOOGOODRE_TAKE;
                                } else {
                                        return self->cube < 2 ?
                                                GIBBON_ANALYSIS_MOVE_CD_NODOUBLE_TAKE
                                                : GIBBON_ANALYSIS_MOVE_CD_NO_REDOUBLE_TAKE;

                                }
                        } else {
                                if (eq_take >= -2.0f
                                    && eq_take <= 0.0f
                                    && self->match_length < 0
                                    && self->beavers)
                                        return self->cube < 2 ?
                                                GIBBON_ANALYSIS_MOVE_CD_NODOUBLE_BEAVER :
                                                GIBBON_ANALYSIS_MOVE_CD_NO_REDOUBLE_BEAVER;
                                else
                                        return self->cube < 2 ?
                                                GIBBON_ANALYSIS_MOVE_CD_NODOUBLE_TAKE :
                                                GIBBON_ANALYSIS_MOVE_CD_NO_REDOUBLE_TAKE;
                        }
                } else {
                        if (self->da_p[0][GIBBON_ANALYSIS_MOVE_PWIN_GAMMON])
                                return self->cube < 2 ?
                                        GIBBON_ANALYSIS_MOVE_CD_TOOGOOD_PASS :
                                        GIBBON_ANALYSIS_MOVE_CD_TOOGOODRE_PASS;
                        else
                                return self->cube < 2 ?
                                        GIBBON_ANALYSIS_MOVE_CD_DOUBLE_PASS :
                                        GIBBON_ANALYSIS_MOVE_CD_REDOUBLE_PASS;
                }
        }
}

gchar *
gibbon_analysis_move_cube_decision (GibbonAnalysisMove *self,
                                    gdouble eq_nodouble, gdouble eq_take,
                                    gdouble eq_drop)
{
        const gchar *s = NULL;
        GibbonAnalysisMoveCubeDecision cd;
        gdouble percent = -1.0f;

        cd = _gibbon_analysis_move_cube_decision (self, eq_nodouble,
                                                  eq_take, eq_drop);
        switch (cd) {
        case GIBBON_ANALYSIS_MOVE_CD_DOUBLE_TAKE:
                s = _("Double, take");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_DOUBLE_PASS:
                s = _("Double, drop");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_NODOUBLE_TAKE:
                s = _("No double, take");
                if (eq_drop - eq_take != 0.0f)
                        percent = (eq_nodouble - eq_take) / (eq_drop - eq_take);
                break;
        case GIBBON_ANALYSIS_MOVE_CD_TOOGOOD_TAKE:
                s = _("Too good to double, take");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_TOOGOOD_PASS:
                s = _("Too good to double, drop");
                if (eq_drop - eq_take != 0.0f
                    && eq_nodouble <= eq_take)
                        percent = (eq_nodouble - eq_take) / (eq_drop - eq_take);
                break;
        case GIBBON_ANALYSIS_MOVE_CD_DOUBLE_BEAVER:
                s = _("Double, beaver");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_NODOUBLE_BEAVER:
                s = _("No double, beaver");
                if (eq_drop - eq_take != 0.0f)
                        percent = (eq_nodouble - eq_take) / (eq_drop - eq_take);
                break;
        case GIBBON_ANALYSIS_MOVE_CD_REDOUBLE_TAKE:
                s = _("Redouble, take");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_REDOUBLE_PASS:
                s = _("Redouble, drop");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_NO_REDOUBLE_TAKE:
                s = _("No redouble, take");
                if (eq_drop - eq_take != 0.0f)
                        percent = (eq_nodouble - eq_take) / (eq_drop - eq_take);
                break;
        case GIBBON_ANALYSIS_MOVE_CD_TOOGOODRE_TAKE:
                s = _("Too good to redouble, take");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_TOOGOODRE_PASS:
                s = _("Too good to redouble, drop");
                if (eq_drop - eq_take != 0.0f
                    && eq_nodouble <= eq_take)
                        percent = (eq_nodouble - eq_take) / (eq_drop - eq_take);
                break;
        case GIBBON_ANALYSIS_MOVE_CD_NO_REDOUBLE_BEAVER:
                s = _("No redouble, beaver");
                if (eq_drop - eq_take != 0.0f)
                        percent = (eq_nodouble - eq_take) / (eq_drop - eq_take);
                break;
        case GIBBON_ANALYSIS_MOVE_CD_NODOUBLE_DEADCUBE:
                s = _("Never double, take (dead cube)");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_NO_REDOUBLE_DEADCUBE:
                s = _("Never redouble, take (dead cube)");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_NOT_AVAILABLE:
                s = _("Doubling not possible");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_DOUBLE_BEAVER:
                s = _("Optional double, beaver");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_DOUBLE_TAKE:
                s = _("Optional double, take");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_REDOUBLE_TAKE:
                s = _("Optional redouble, take");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_DOUBLE_PASS:
                s = _("Optional double, drop");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_REDOUBLE_PASS:
                s = _("Optional redouble, drop");
                break;
        }

        if (!s)
                s = _("No idea");

        if (percent >= 0.0f)
                return g_strdup_printf (_("Proper cube action: %s (%.1f%%)"),
                                        s, 100 * percent);
        else
                return g_strdup_printf (_("Proper cube action: %s"), s);
}

gchar *
gibbon_analysis_move_take_decision (GibbonAnalysisMove *self,
                                    gdouble eq_nodouble, gdouble eq_take,
                                    gdouble eq_drop)
{
        const gchar *s = NULL;
        GibbonAnalysisMoveCubeDecision cd;

        cd = _gibbon_analysis_move_cube_decision (self, eq_nodouble,
                                                  eq_take, eq_drop);
        switch (cd) {
        case GIBBON_ANALYSIS_MOVE_CD_DOUBLE_TAKE:
        case GIBBON_ANALYSIS_MOVE_CD_NODOUBLE_TAKE:
        case GIBBON_ANALYSIS_MOVE_CD_TOOGOOD_TAKE:
        case GIBBON_ANALYSIS_MOVE_CD_REDOUBLE_TAKE:
        case GIBBON_ANALYSIS_MOVE_CD_NO_REDOUBLE_TAKE:
        case GIBBON_ANALYSIS_MOVE_CD_TOOGOODRE_TAKE:
        case GIBBON_ANALYSIS_MOVE_CD_NODOUBLE_DEADCUBE:
        case GIBBON_ANALYSIS_MOVE_CD_NO_REDOUBLE_DEADCUBE:
        case GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_DOUBLE_TAKE:
        case GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_REDOUBLE_TAKE:
                s = _("Take");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_DOUBLE_PASS:
        case GIBBON_ANALYSIS_MOVE_CD_TOOGOOD_PASS:
        case GIBBON_ANALYSIS_MOVE_CD_REDOUBLE_PASS:
        case GIBBON_ANALYSIS_MOVE_CD_TOOGOODRE_PASS:
        case GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_DOUBLE_PASS:
        case GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_REDOUBLE_PASS:
                s = _("Drop");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_DOUBLE_BEAVER:
        case GIBBON_ANALYSIS_MOVE_CD_NODOUBLE_BEAVER:
        case GIBBON_ANALYSIS_MOVE_CD_NO_REDOUBLE_BEAVER:
        case GIBBON_ANALYSIS_MOVE_CD_OPTIONAL_DOUBLE_BEAVER:
                s = _("Beaver");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_NOT_AVAILABLE:
                s = _("Huh?");
                break;
        }

        if (!s)
                s = _("No idea");

        return g_strdup_printf (_("Proper response: %s"), s);
}
