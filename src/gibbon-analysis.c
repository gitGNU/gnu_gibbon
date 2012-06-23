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
 * SECTION:gibbon-analysis
 * @short_description: Abstraction for analysis results.
 *
 * Since: 0.2.0
 *
 * A #GibbonAnalysis is an abstraction for the result of an analysis of a
 * particular action in match like #GibbonMove, #GibbonDouble, or even
 * #GibbonRoll.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-analysis.h"

G_DEFINE_TYPE (GibbonAnalysis, gibbon_analysis, G_TYPE_OBJECT)

static void 
gibbon_analysis_init (GibbonAnalysis *self)
{
}

static void
gibbon_analysis_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_analysis_parent_class)->finalize(object);
}

static void
gibbon_analysis_class_init (GibbonAnalysisClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        object_class->finalize = gibbon_analysis_finalize;
}
