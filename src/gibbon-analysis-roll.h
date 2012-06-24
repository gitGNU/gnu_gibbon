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

#ifndef _GIBBON_ANALYSIS_ROLL_H
# define _GIBBON_ANALYSIS_ROLL_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-analysis.h"

#define GIBBON_TYPE_ANALYSIS_ROLL \
        (gibbon_analysis_roll_get_type ())
#define GIBBON_ANALYSIS_ROLL(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_ANALYSIS_ROLL, \
                GibbonAnalysisRoll))
#define GIBBON_ANALYSIS_ROLL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_ANALYSIS_ROLL, GibbonAnalysisRollClass))
#define GIBBON_IS_ANALYSIS_ROLL(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_ANALYSIS_ROLL))
#define GIBBON_IS_ANALYSIS_ROLL_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_ANALYSIS_ROLL))
#define GIBBON_ANALYSIS_ROLL_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_ANALYSIS_ROLL, GibbonAnalysisRollClass))

typedef enum {
        GIBBON_ANALYSIS_ROLL_LUCK_NONE = -1,
        GIBBON_ANALYSIS_ROLL_LUCK_UNKNOWN = 0,
        GIBBON_ANALYSIS_ROLL_LUCK_LUCKY,
        GIBBON_ANALYSIS_ROLL_LUCK_VERY_LUCKY,
        GIBBON_ANALYSIS_ROLL_LUCK_UNLUCKY,
        GIBBON_ANALYSIS_ROLL_LUCK_VERY_UNLUCKY
} GibbonAnalysisRollLuck;

/**
 * GibbonAnalysisRoll:
 *
 * One instance of a #GibbonAnalysisRoll.  All properties are private.
 */
typedef struct _GibbonAnalysisRoll GibbonAnalysisRoll;
struct _GibbonAnalysisRoll
{
        GibbonAnalysis parent_instance;

        /*< private >*/
        struct _GibbonAnalysisRollPrivate *priv;
};

/**
 * GibbonAnalysisRollClass:
 *
 * Class for holding the result of the analysis of a dice roll.
 */
typedef struct _GibbonAnalysisRollClass GibbonAnalysisRollClass;
struct _GibbonAnalysisRollClass
{
        /* <private >*/
        GibbonAnalysisClass parent_class;
};

GType gibbon_analysis_roll_get_type (void) G_GNUC_CONST;

GibbonAnalysisRoll *gibbon_analysis_roll_new (GibbonAnalysisRollLuck type,
                                              gdouble luck);
gdouble gibbon_analysis_roll_get_luck_value (const GibbonAnalysisRoll *self);
GibbonAnalysisRollLuck gibbon_analysis_roll_get_luck_type (const
                                                           GibbonAnalysisRoll
                                                           *self);

#endif
