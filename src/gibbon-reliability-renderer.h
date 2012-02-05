/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
 *
 * Gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GIBBON_RELIABILITY_RENDERER_H
# define _GIBBON_RELIABILITY_RENDERER_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>

#define GIBBON_TYPE_RELIABILITY_RENDERER \
        (gibbon_reliability_renderer_get_type ())
#define GIBBON_RELIABILITY_RENDERER(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_RELIABILITY_RENDERER, \
                GibbonReliabilityRenderer))
#define GIBBON_RELIABILITY_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_RELIABILITY_RENDERER, GibbonReliabilityRendererClass))
#define GIBBON_IS_RELIABILITY_RENDERER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_RELIABILITY_RENDERER))
#define GIBBON_IS_RELIABILITY_RENDERER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_RELIABILITY_RENDERER))
#define GIBBON_RELIABILITY_RENDERER_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_RELIABILITY_RENDERER, GibbonReliabilityRendererClass))

/**
 * GibbonReliabilityRenderer:
 *
 * One instance of a #GibbonReliabilityRenderer.  All properties are private.
 */
typedef struct _GibbonReliabilityRenderer GibbonReliabilityRenderer;
struct _GibbonReliabilityRenderer
{
        GtkCellRenderer parent_instance;

        /*< private >*/
        struct _GibbonReliabilityRendererPrivate *priv;
};

/**
 * GibbonReliabilityRendererClass:
 *
 * Custom cell renderer for reliability display!
 */
typedef struct _GibbonReliabilityRendererClass GibbonReliabilityRendererClass;
struct _GibbonReliabilityRendererClass
{
        /* <private >*/
        GtkCellRendererClass parent_class;
};

GType gibbon_reliability_renderer_get_type (void) G_GNUC_CONST;

GtkCellRenderer *gibbon_reliability_renderer_new (void);

#endif
