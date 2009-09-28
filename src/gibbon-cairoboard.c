/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009 Guido Flohr, http://guido-flohr.net/.
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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gibbon-cairoboard.h"

struct _GibbonCairoboardPrivate {
        gint dummy;
};

#define GIBBON_CAIROBOARD_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_CAIROBOARD,           \
                                      GibbonCairoboardPrivate))
G_DEFINE_TYPE (GibbonCairoboard, gibbon_cairoboard, GTK_TYPE_DRAWING_AREA);

static gboolean gibbon_cairoboard_expose (GtkWidget *object, 
                                          GdkEventExpose *event);
static void gibbon_cairoboard_draw (GibbonCairoboard *board, cairo_t *cr);

static void
gibbon_cairoboard_init (GibbonCairoboard *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, 
                                                  GIBBON_TYPE_CAIROBOARD, 
                                                  GibbonCairoboardPrivate);
}

static void
gibbon_cairoboard_finalize (GObject *object)
{
/*        GibbonCairoboard *self = GIBBON_CAIROBOARD (object); */

        G_OBJECT_CLASS (gibbon_cairoboard_parent_class)->finalize (object);
}

static void
gibbon_cairoboard_class_init (GibbonCairoboardClass *klass)
{
        GtkDrawingAreaClass* parent_class = GTK_DRAWING_AREA_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonCairoboardPrivate));

        G_OBJECT_CLASS (parent_class)->finalize = gibbon_cairoboard_finalize;
        GTK_WIDGET_CLASS (parent_class)->expose_event = 
                gibbon_cairoboard_expose;
}

GibbonCairoboard *
gibbon_cairoboard_new ()
{
        GibbonCairoboard *self = g_object_new (GIBBON_TYPE_CAIROBOARD, NULL);

        return self;
}

static gboolean 
gibbon_cairoboard_expose (GtkWidget *widget, GdkEventExpose *event) 
{
        cairo_t *cr;
        
        g_return_val_if_fail (GIBBON_IS_CAIROBOARD (widget), FALSE);
        
        cr = gdk_cairo_create (widget->window);
        
        cairo_rectangle (cr,
                         event->area.x, event->area.y,
                         event->area.width, event->area.height);
        cairo_clip (cr);

        gibbon_cairoboard_draw (GIBBON_CAIROBOARD (widget), cr);

        cairo_destroy (cr);
        
        return FALSE;
}                                                

static void
gibbon_cairoboard_draw (GibbonCairoboard *self, cairo_t *cr)
{
        double x, y, radius;
        GtkWidget *widget;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        
        widget = GTK_WIDGET (self);

         widget->allocation.x,
         widget->allocation.y,
         widget->allocation.width,
         widget->allocation.height);
         
        x = 0 + widget->allocation.width / 2;
        y = 0 + widget->allocation.height / 2;
        radius = MIN (widget->allocation.width / 2,
                      widget->allocation.height / 2) - 5;

        cairo_arc (cr, x, y, radius, 0, 2 * 3.1415927);

        cairo_set_source_rgb (cr, 1, 1, 1);
        cairo_fill_preserve (cr);
        cairo_set_source_rgb (cr, 0, 0, 0);
        cairo_stroke (cr);
}