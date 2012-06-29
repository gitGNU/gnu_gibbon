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

#ifndef _GIBBON_GRID_VIEW_H
# define _GIBBON_GRID_VIEW_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>

#define GIBBON_TYPE_GRID_VIEW \
        (gibbon_grid_view_get_type ())
#define GIBBON_GRID_VIEW(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_GRID_VIEW, \
                GibbonGridView))
#define GIBBON_GRID_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_GRID_VIEW, GibbonGridViewClass))
#define GIBBON_IS_GRID_VIEW(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_GRID_VIEW))
#define GIBBON_IS_GRID_VIEW_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_GRID_VIEW))
#define GIBBON_GRID_VIEW_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_GRID_VIEW, GibbonGridViewClass))

/**
 * GibbonGridView:
 *
 * One instance of a #GibbonGridView.  All properties are private.
 */
typedef struct _GibbonGridView GibbonGridView;
struct _GibbonGridView
{
        GtkTreeView parent_instance;
};

/**
 * GibbonGridViewClass:
 *
 * Wrapper class for a GtkTreeView.
 */
typedef struct _GibbonGridViewClass GibbonGridViewClass;
struct _GibbonGridViewClass
{
        /* <private >*/
        GtkTreeViewClass parent_class;
};

GType gibbon_grid_view_get_type (void) G_GNUC_CONST;

GibbonGridView *gibbon_grid_view_new (void);

#endif
