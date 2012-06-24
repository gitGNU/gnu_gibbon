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
 * SECTION:gibbon-analysis-roll
 * @short_description: Analysis of a roll!
 *
 * Since: 0.2.0
 *
 * The analysis of a roll tells you whether a particular roll of the dice was
 * lucky or unlucky in a certain context.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-analysis-roll.h"

typedef struct _GibbonAnalysisRollPrivate GibbonAnalysisRollPrivate;
struct _GibbonAnalysisRollPrivate {
        GibbonAnalysisRollLuck type;
        gdouble luck;
};

#define GIBBON_ANALYSIS_ROLL_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_ANALYSIS_ROLL, GibbonAnalysisRollPrivate))

G_DEFINE_TYPE (GibbonAnalysisRoll, gibbon_analysis_roll, GIBBON_TYPE_ANALYSIS)

static void 
gibbon_analysis_roll_init (GibbonAnalysisRoll *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_ANALYSIS_ROLL, GibbonAnalysisRollPrivate);

        self->priv->type = GIBBON_ANALYSIS_ROLL_LUCK_UNKNOWN;
        self->priv->luck = 0.0;
}

static void
gibbon_analysis_roll_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_analysis_roll_parent_class)->finalize (object);
}

static void
gibbon_analysis_roll_class_init (GibbonAnalysisRollClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonAnalysisRollPrivate));

        object_class->finalize = gibbon_analysis_roll_finalize;
}

/**
 * gibbon_analysis_roll_new:
 * @type: Luck estimation for that roll.
 * @luck: Luck measurement of that roll.
 *
 * Creates a new #GibbonAnalysisRoll.
 *
 * Returns: The newly created #GibbonAnalysisRoll or %NULL in case of failure.
 */
GibbonAnalysisRoll *
gibbon_analysis_roll_new (GibbonAnalysisRollLuck type, gdouble luck)
{
        GibbonAnalysisRoll *self = g_object_new (GIBBON_TYPE_ANALYSIS_ROLL, NULL);

        self->priv->type = type;
        self->priv->luck = luck;

        return self;
}

gdouble
gibbon_analysis_roll_get_luck_value (const GibbonAnalysisRoll *self)
{
        g_return_val_if_fail (GIBBON_IS_ANALYSIS_ROLL (self), 0.0);

        return self->priv->luck;
}

GibbonAnalysisRollLuck
gibbon_analysis_roll_get_luck_type (const GibbonAnalysisRoll *self)
{
        g_return_val_if_fail (GIBBON_IS_ANALYSIS_ROLL (self),
                              GIBBON_ANALYSIS_ROLL_LUCK_UNKNOWN);

        return self->priv->type;
}

