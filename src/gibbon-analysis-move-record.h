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

#ifndef _GIBBON_ANALYSIS_MOVE_RECORD_H
# define _GIBBON_ANALYSIS_MOVE_RECORD_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-move.h"

#define GIBBON_TYPE_ANALYSIS_MOVE_RECORD \
        (gibbon_analysis_move_record_get_type ())

/**
 * GibbonAnalysisMoveRecord:
 *
 * Boxed type for a move analysis record.
 */
typedef struct _GibbonAnalysisMoveRecord GibbonAnalysisMoveRecord;
struct _GibbonAnalysisMoveRecord
{
        GibbonMove *move;

        gboolean rollout;
        guint64 plies;
        gboolean cubeful;
        gboolean deterministic;
        gdouble noise;
        gboolean use_prune;

        /*
         * You can use the constants GIBBON_ANALYSIS_MOVE_PWIN to
         * GIBBON_ANALYSIS_MOVE_EQUITY for indexes into this array.
         */
        gdouble p[6];
};

GType gibbon_analysis_move_record_get_type (void) G_GNUC_CONST;

GibbonAnalysisMoveRecord *gibbon_analysis_move_record_new (void);
void gibbon_analysis_move_record_free (GibbonAnalysisMoveRecord *self);
GibbonAnalysisMoveRecord *gibbon_analysis_move_record_copy (
                const GibbonAnalysisMoveRecord *self);

#endif
