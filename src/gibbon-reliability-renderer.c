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

/**
 * SECTION:gibbon-reliability-renderer
 * @short_description: Custom cell renderer for reliability display!
 *
 * Since: 0.1.0
 *
 * Display a little bar where the color indicates the reliability of a certain
 * playder, and its length indicates the confidence for that value.
 */

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gibbon-reliability.h"
#include "gibbon-reliability-renderer.h"

typedef struct _GibbonReliabilityRendererPrivate GibbonReliabilityRendererPrivate;
struct _GibbonReliabilityRendererPrivate {
        GibbonReliability *rel;
};

#define GIBBON_RELIABILITY_RENDERER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_RELIABILITY_RENDERER, GibbonReliabilityRendererPrivate))

G_DEFINE_TYPE (GibbonReliabilityRenderer, gibbon_reliability_renderer,
               GTK_TYPE_CELL_RENDERER)

static void gibbon_reliability_renderer_get_property (GObject *object,
                                                      guint param_id,
                                                      GValue *value,
                                                      GParamSpec *pspec);

static void gibbon_reliability_renderer_set_property (GObject *object,
                                                      guint param_id,
                                                      const GValue *value,
                                                      GParamSpec *pspec);

static void gibbon_reliability_renderer_get_size (GtkCellRenderer *cell,
                                                  GtkWidget *widget,
                                                  GdkRectangle *cell_area,
                                                  gint *x_offset,
                                                  gint *y_offset,
                                                  gint *width,
                                                  gint *height);

static void gibbon_reliability_renderer_render (GtkCellRenderer *cell,
                                                GdkWindow *window,
                                                GtkWidget *widget,
                                                GdkRectangle *background_area,
                                                GdkRectangle *cell_area,
                                                GdkRectangle *expose_area,
                                                guint flags);

enum {
        GIBBON_PROP_RELIABILITY = 1
};

static void 
gibbon_reliability_renderer_init (GibbonReliabilityRenderer *self)
{
        GValue v = G_VALUE_INIT;

        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_RELIABILITY_RENDERER, GibbonReliabilityRendererPrivate);

        self->priv->rel = NULL;

        g_value_init (&v, G_TYPE_INT);

        g_object_set_property (G_OBJECT (self), "mode", &v);
        g_value_set_int (&v, GTK_CELL_RENDERER_MODE_INERT);

        gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (self), 2, 2);
}

static void
gibbon_reliability_renderer_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_reliability_renderer_parent_class)->finalize(object);
}

static void
gibbon_reliability_renderer_class_init (GibbonReliabilityRendererClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GtkCellRendererClass *gtk_cell_renderer_class =
                        GTK_CELL_RENDERER_CLASS (klass);
        GParamSpec *pspec;

        object_class->get_property = gibbon_reliability_renderer_get_property;
        object_class->set_property = gibbon_reliability_renderer_set_property;
        gtk_cell_renderer_class->get_size =
                gibbon_reliability_renderer_get_size;
        gtk_cell_renderer_class->render =
                gibbon_reliability_renderer_render;
        
        /* Install our very own properties */
        pspec = g_param_spec_boxed ("reliability",
                                    _("Reliability"),
                                    _("Reliability of a player"),
                                    GIBBON_TYPE_RELIABILITY,
                                    G_PARAM_READWRITE);
        g_object_class_install_property (object_class,
                                         GIBBON_PROP_RELIABILITY,
                                         pspec);

        g_type_class_add_private (klass, sizeof (GibbonReliabilityRendererPrivate));

        object_class->finalize = gibbon_reliability_renderer_finalize;
}

/**
 * gibbon_reliability_renderer_new:
 *
 * Creates a new #GibbonReliabilityRenderer.
 *
 * Returns: The newly created #GibbonReliabilityRenderer or %NULL in case of
 *          failure.
 */
GtkCellRenderer *
gibbon_reliability_renderer_new (void)
{
        GibbonReliabilityRenderer *self =
            g_object_new (GIBBON_TYPE_RELIABILITY_RENDERER, NULL);

        return GTK_CELL_RENDERER (self);
}

static void
gibbon_reliability_renderer_get_property (GObject *object,
                                          guint param_id,
                                          GValue *value,
                                          GParamSpec *pspec)
{
        GibbonReliabilityRenderer *self = GIBBON_RELIABILITY_RENDERER (object);

        switch (param_id) {
                case GIBBON_PROP_RELIABILITY:
                        g_value_set_boxed (value, self->priv->rel);
                        break;

                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id,
                                                           pspec);
                        break;
        }
}

static void
gibbon_reliability_renderer_set_property (GObject *object,
                                          guint param_id,
                                          const GValue *value,
                                          GParamSpec *pspec)
{
        GibbonReliabilityRenderer *self = GIBBON_RELIABILITY_RENDERER (object);

        switch (param_id) {
                case GIBBON_PROP_RELIABILITY:
                        if (self->priv->rel)
                                g_boxed_free (GIBBON_TYPE_RELIABILITY,
                                              self->priv->rel);
                        self->priv->rel = (GibbonReliability *)
                                g_boxed_copy (GIBBON_TYPE_RELIABILITY,
                                              g_value_get_boxed (value));
                        break;

                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id,
                                                           pspec);
                        break;
        }
}

static void
gibbon_reliability_renderer_get_size (GtkCellRenderer *cell,
                                      GtkWidget *widget,
                                      GdkRectangle *cell_area,
                                      gint *x_offset,
                                      gint *y_offset,
                                      gint *width,
                                      gint *height)
{
        gint calc_width;
        gint calc_height;
        gint xpad, ypad;
        gfloat xalign, yalign;

        gtk_cell_renderer_get_padding (cell, &xpad, &ypad);

        calc_width  = xpad * 2 + 80;
        calc_height = ypad * 2 + 12;

        if (width)
                *width = calc_width;

        if (height)
                *height = calc_height;

        gtk_cell_renderer_get_alignment (cell, &xalign, &yalign);

        if (cell_area) {
                if (x_offset) {
                        *x_offset = xalign * (cell_area->width
                                                    - calc_width);
                        *x_offset = MAX (*x_offset, 0);
                }

                if (y_offset) {
                        *y_offset = yalign * 0.5 * (cell_area->height
                                                          - calc_height);
                        *y_offset = MAX (*y_offset, 0);
                }
        }
}

static void
gibbon_reliability_renderer_render (GtkCellRenderer *cell,
                                    GdkWindow *window,
                                    GtkWidget *widget,
                                    GdkRectangle *background_area,
                                    GdkRectangle *cell_area,
                                    GdkRectangle *expose_area,
                                    guint flags)
{
        GibbonReliabilityRenderer *self = GIBBON_RELIABILITY_RENDERER (cell);
        gint width, height;
        gint x_offset, y_offset;
        gint xpad, ypad;
        gdouble percentage;
        cairo_t *cr;
        GdkColor color;
        GdkRectangle bar;

        gibbon_reliability_renderer_get_size (cell, widget, cell_area,
                                              &x_offset, &y_offset,
                                              &width, &height);

        /*
        g_printerr ("\nBA: %d x %d at (%d|%d).\n",
                    background_area->width, background_area->height,
                    background_area->x, background_area->y);
        g_printerr ("CA: %d x %d at (%d|%d).\n",
                    cell_area->width, cell_area->height,
                    cell_area->x, cell_area->y);
        g_printerr ("EA: %d x %d at (%d|%d).\n",
                    expose_area->width, expose_area->height,
                    expose_area->x, expose_area->y);
        */

        gtk_cell_renderer_get_padding (cell, &xpad, &ypad);
        width  -= xpad * 2;
        height -= ypad * 2;

        percentage = 0.1 * self->priv->rel->confidence;
        if (percentage > 1.0)
                percentage = 1.0;

        cr = gdk_cairo_create (window);

        gdk_cairo_rectangle (cr, expose_area);
        cairo_clip (cr);

        color.red = 0x0000;
        color.green = 0x0000;
        color.blue = 0x0000;
        gdk_cairo_set_source_color (cr, &color);

        bar.x = cell_area->x + x_offset + xpad;
        bar.y = cell_area->y + y_offset + ypad;
        bar.width = width;
        bar.height = height - 1;
        gdk_cairo_rectangle (cr, &bar);

        cairo_set_line_width (cr, 0.2);
        cairo_stroke (cr);

        bar.x += 2;
        bar.y += 2;
        bar.width = (width - 4) * percentage;
        bar.height = height - 5;
        gdk_cairo_rectangle (cr, &bar);

        color.pixel = 0;
        if (self->priv->rel->value >= 0.95) {
                color.red = 0x0000;
                color.green = 0xb800;
                color.blue = 0x0000;
        } else if (self->priv->rel->value >= 0.85) {
                color.red = 0xff00;
                color.green = 0xff00;
                color.blue = 0x0000;
        } else if (self->priv->rel->value >= 0.65) {
                color.red = 0xff00;
                color.green = 0xa000;
                color.blue = 0x0000;
        } else {
                color.red = 0xec00;
                color.green = 0x0000;
                color.blue = 0x0000;
        }
        gdk_cairo_set_source_color (cr, &color);
        cairo_fill (cr);

        cairo_destroy (cr);
}
