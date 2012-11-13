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

#ifndef _GIBBON_ANALYSIS_VIEW_H
# define _GIBBON_ANALYSIS_VIEW_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>
#include "gibbon-app.h"
#include "gibbon-game.h"

#define GIBBON_TYPE_ANALYSIS_VIEW \
        (gibbon_analysis_view_get_type ())
#define GIBBON_ANALYSIS_VIEW(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_ANALYSIS_VIEW, \
                GibbonAnalysisView))
#define GIBBON_ANALYSIS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_ANALYSIS_VIEW, GibbonAnalysisViewClass))
#define GIBBON_IS_ANALYSIS_VIEW(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_ANALYSIS_VIEW))
#define GIBBON_IS_ANALYSIS_VIEW_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_ANALYSIS_VIEW))
#define GIBBON_ANALYSIS_VIEW_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_ANALYSIS_VIEW, GibbonAnalysisViewClass))

/**
 * GibbonAnalysisView:
 *
 * One instance of a #GibbonAnalysisView.  All properties are private.
 */
typedef struct _GibbonAnalysisView GibbonAnalysisView;
struct _GibbonAnalysisView
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonAnalysisViewPrivate *priv;
};

/**
 * GibbonAnalysisViewClass:
 *
 * The various analysis views underneath the move list!
 */
typedef struct _GibbonAnalysisViewClass GibbonAnalysisViewClass;
struct _GibbonAnalysisViewClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_analysis_view_get_type (void) G_GNUC_CONST;

GibbonAnalysisView *gibbon_analysis_view_new (const GibbonApp *app);
void gibbon_analysis_view_set_analysis (GibbonAnalysisView *self,
                                        const GibbonGame *game,
                                        gint action_number);
void gibbon_analysis_view_fixup_layout (const GibbonAnalysisView *self);

#endif
