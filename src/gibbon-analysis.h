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

#ifndef _GIBBON_ANALYSIS_H
# define _GIBBON_ANALYSIS_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#define GIBBON_TYPE_ANALYSIS \
        (gibbon_analysis_get_type ())
#define GIBBON_ANALYSIS(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_ANALYSIS, \
                GibbonAnalysis))
#define GIBBON_ANALYSIS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_ANALYSIS, GibbonAnalysisClass))
#define GIBBON_IS_ANALYSIS(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_ANALYSIS))
#define GIBBON_IS_ANALYSIS_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_ANALYSIS))
#define GIBBON_ANALYSIS_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_ANALYSIS, GibbonAnalysisClass))

/**
 * GibbonAnalysis:
 *
 * One instance of a #GibbonAnalysis.  All properties are private.
 */
typedef struct _GibbonAnalysis GibbonAnalysis;
struct _GibbonAnalysis
{
        GObject parent_instance;
};

/**
 * GibbonAnalysisClass:
 *
 * Abstract base class for all kind of analysis for game actions.
 */
typedef struct _GibbonAnalysisClass GibbonAnalysisClass;
struct _GibbonAnalysisClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_analysis_get_type (void) G_GNUC_CONST;

#endif
