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

static void 
gibbon_analysis_move_init (GibbonAnalysisMove *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_ANALYSIS_MOVE, GibbonAnalysisMovePrivate);

        self->da = FALSE;
        self->da_bad = 0;
        self->da_rollout = FALSE;
        self->da_plies = 0;
        self->da_cubeful = FALSE;
        self->da_use_prune = FALSE;
        self->da_deterministic = FALSE;
}

static void
gibbon_analysis_move_finalize (GObject *object)
{
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

static GibbonAnalysisMoveCubeDecision
_gibbon_analysis_move_cube_decision (GibbonAnalysisMove *self)
{
        if (!self->may_double)
                return GIBBON_ANALYSIS_MOVE_CD_NOT_AVAILABLE;

        if (self->match_length > 0) {
                if (self->my_score + self->cube >= self->match_length) {
                        return self->cube > 1 ?
                                GIBBON_ANALYSIS_MOVE_CD_NODOUBLE_DEADCUBE
                                : GIBBON_ANALYSIS_MOVE_CD_NO_REDOUBLE_DEADCUBE;
                }
        }



        return GIBBON_ANALYSIS_MOVE_CD_NODOUBLE_TAKE;
}

gchar *
gibbon_analysis_move_cube_decision (GibbonAnalysisMove *self)
{
        const gchar *s = NULL;
        GibbonAnalysisMoveCubeDecision cd;

        cd = _gibbon_analysis_move_cube_decision (self);
        switch (cd) {
        case GIBBON_ANALYSIS_MOVE_CD_NOT_AVAILABLE:
                s = _("Doubling not allowed");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_NODOUBLE_DEADCUBE:
                s = _("Never double, take (dead cube)");
                break;
        case GIBBON_ANALYSIS_MOVE_CD_NO_REDOUBLE_DEADCUBE:
                s = _("Never redouble, take (dead cube)");
                break;
        }

        if (!s)
                s = _("No idea");

        return g_strdup_printf (_("Proper cube action: %s"), s);
}
