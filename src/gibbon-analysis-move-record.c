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
 * SECTION:gibbon-analysis-move-record
 * @short_description: Boxed type for a move analysis record.
 *
 * Since: 0.2.0
 *
 * This boxed type encapsulate one item in an SGF "A" record.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-analysis-move-record.h"

G_DEFINE_BOXED_TYPE (GibbonAnalysisMoveRecord, gibbon_analysis_move_record,  \
                     NULL, gibbon_analysis_move_record_free)

GibbonAnalysisMoveRecord *
gibbon_analysis_move_record_new (void)
{
        GibbonAnalysisMoveRecord *self;

        self = g_malloc0 (sizeof *self);

        return self;
}

void
gibbon_analysis_move_record_free (GibbonAnalysisMoveRecord *self)
{
        if (self) {
                if (self->move)
                        g_object_unref (self->move);
                g_free (self);
        }
}
