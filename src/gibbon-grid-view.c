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
 * SECTION:gibbon-grid-view
 * @short_description: Wrapper class for GtkTreeView.
 *
 * Since: 0.2.0
 *
 * GtkTreeView objects are row-centric while our move list can focus on
 * indivdual cells.  This class inherits from GtkTreeView and overrides those
 * methods that are in the way for our cell-based approach.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-grid-view.h"

G_DEFINE_TYPE (GibbonGridView, gibbon_grid_view, GTK_TYPE_TREE_VIEW)

static gint gibbon_grid_view_focus (GtkWidget *widget,
                                    GtkDirectionType direction);

static void 
gibbon_grid_view_init (GibbonGridView *self)
{
}

static void
gibbon_grid_view_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_grid_view_parent_class)->finalize(object);
}

static void
gibbon_grid_view_class_init (GibbonGridViewClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GtkWidgetClass *gtk_widget_class = GTK_WIDGET_CLASS (klass);

        gtk_widget_class->focus = gibbon_grid_view_focus;

        object_class->finalize = gibbon_grid_view_finalize;
}

/**
 * gibbon_grid_view_new:
 *
 * Creates a new #GibbonGridView.
 *
 * Returns: The newly created #GibbonGridView or %NULL in case of failure.
 */
GibbonGridView *
gibbon_grid_view_new ()
{
        GibbonGridView *self = g_object_new (GIBBON_TYPE_GRID_VIEW, NULL);

        return self;
}

static gint
gibbon_grid_view_focus (GtkWidget *widget, GtkDirectionType direction)
{
        return FALSE;
}
