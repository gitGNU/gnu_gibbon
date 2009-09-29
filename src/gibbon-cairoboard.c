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

struct GibbonColor {
        double red;
        double green;
        double blue;
        double alpha;
};

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
        GtkWidget *widget;
        GtkAllocation *allocation;
        double design_width = 490;
        double design_height = 380;
        double aspect_ratio = design_width / design_height;
        double widget_ratio;
        double translate_x, translate_y, scale;
        struct GibbonColor frame_color = { 0.2, 0.15, 0, 1 };
        struct GibbonColor bearoff_color = { 0, 0, 0, 1 };
        double outer_border_w = 10;
        double outer_border_h = 10;
        double checker_width = 30;
        double checker_height = 10;
        double point_width = checker_width;
        double point_length = 5 * checker_width;
        struct GibbonColor board_color = { 0.4, 0.25, 0, 1 };
        double board_x, board_y;
        double dice_area_height = 2 * checker_width;
        struct GibbonColor point_color1 = { 0.6, 0, 0, 1 };
        struct GibbonColor point_color2 = { 0.5, 0.5, 0.5, 1 };
        double d;
        gint i;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        
        widget = GTK_WIDGET (self);
        allocation = &widget->allocation;
        
        if (!allocation->height)
                return;
        if (!allocation->width)
                return;
        
        widget_ratio = (double) allocation->width / allocation->height;

        if (widget_ratio > aspect_ratio) {
                scale = allocation->height / design_height; 
                translate_y = 0;
                translate_x = (allocation->width 
                               - scale * design_width) / 2;
        } else {
                scale = allocation->width / design_width;
                translate_x = 0;
                translate_y = (allocation->height 
                               - scale * design_height) / 2;
        }

        cairo_translate (cr, translate_x, translate_y);
        cairo_scale (cr, scale, scale);

        /* Board background.  */
        cairo_rectangle (cr,
                         0, 0,
                         design_width, design_height);

        cairo_set_source_rgb (cr, 
                              frame_color.red, 
                              frame_color.green,
                              frame_color.blue);
        cairo_fill (cr);
        
        /* Bear-off compartments.  */
        cairo_set_source_rgb (cr, 
                              bearoff_color.red, 
                              bearoff_color.green, 
                              bearoff_color.blue);
        cairo_rectangle (cr,
                         outer_border_w, outer_border_h, 
                         checker_width, 15 * checker_height);        
        cairo_fill (cr);
        cairo_rectangle (cr,
                         outer_border_w, 
                         design_height - outer_border_h - 15 * checker_height,
                         checker_width, 15 * checker_height);
        cairo_fill (cr);
        cairo_rectangle (cr,
                         design_width - outer_border_w - checker_width,
                         outer_border_h,
                         checker_width, 15 * checker_height);
        cairo_fill (cr);
        cairo_rectangle (cr,
                         design_width - outer_border_w - checker_width,
                         design_height - outer_border_h - 15 * checker_height,
                         checker_width, 15 * checker_height);
        cairo_fill (cr);
        
        /* Board background.  */
        cairo_set_source_rgb (cr,
                              board_color.red,
                              board_color.green,
                              board_color.blue);
        board_x = 2 * outer_border_w + checker_width;
        board_y = outer_border_h;
         
        cairo_rectangle (cr,
                         board_x, board_y,
                         6 * point_width,
                         2 * point_length + dice_area_height);
        cairo_fill (cr);
        cairo_rectangle (cr,
                         design_width - board_x - 6 * point_width, board_y,
                         6 * point_width,
                         2 * point_length + dice_area_height);
        cairo_fill (cr);
        
        cairo_set_source_rgb (cr,
                              point_color1.red,
                              point_color1.green,
                              point_color1.blue);
        for (i = 0; i < 6; i += 2) {
                cairo_move_to (cr, board_x + i * point_width, board_y);
                cairo_rel_line_to (cr, point_width, 0);
                cairo_rel_line_to (cr, -point_width / 2, point_length);
                cairo_close_path (cr);
                cairo_fill (cr);
                
                cairo_move_to (cr, 
                               design_width - board_x - (6 - i) * point_width,
                               board_y);
                cairo_rel_line_to (cr, point_width, 0);
                cairo_rel_line_to (cr, -point_width / 2, point_length);
                cairo_close_path (cr);
                cairo_fill (cr);
                
                cairo_move_to (cr,
                               board_x + (i + 1) * point_width,
                               design_height - outer_border_h);
                cairo_rel_line_to (cr, point_width, 0);
                cairo_rel_line_to (cr, -point_width / 2, -point_length);
                cairo_close_path (cr);
                cairo_fill (cr);
                
                cairo_move_to (cr,
                               design_width - board_x - (5 - i) * point_width,
                               design_height - outer_border_h);
                cairo_rel_line_to (cr, point_width, 0);
                cairo_rel_line_to (cr, -point_width / 2, -point_length);
                cairo_close_path (cr);
                cairo_fill (cr);
        }

        cairo_set_source_rgb (cr,
                              point_color2.red,
                              point_color2.green,
                              point_color2.blue);
        for (i = 0; i < 6; i += 2) {
                cairo_move_to (cr, board_x + (i + 1) * point_width, board_y);
                cairo_rel_line_to (cr, point_width, 0);
                cairo_rel_line_to (cr, -point_width / 2, point_length);
                cairo_close_path (cr);
                cairo_fill (cr);
                
                cairo_move_to (cr, 
                               design_width - board_x - (5 - i) * point_width,
                               board_y);
                cairo_rel_line_to (cr, point_width, 0);
                cairo_rel_line_to (cr, -point_width / 2, point_length);
                cairo_close_path (cr);
                cairo_fill (cr);
                
                cairo_move_to (cr,
                               board_x + i * point_width,
                               design_height - outer_border_h);
                cairo_rel_line_to (cr, point_width, 0);
                cairo_rel_line_to (cr, -point_width / 2, -point_length);
                cairo_close_path (cr);
                cairo_fill (cr);
                
                cairo_move_to (cr,
                               design_width - board_x - (6 - i) * point_width,
                               design_height - outer_border_h);
                cairo_rel_line_to (cr, point_width, 0);
                cairo_rel_line_to (cr, -point_width / 2, -point_length);
                cairo_close_path (cr);
                cairo_fill (cr);
        }
}
