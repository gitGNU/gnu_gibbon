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
#include "gibbon-design.h"

struct _GibbonCairoboardPrivate {
        const GibbonDesign *design;
};

#define GIBBON_CAIROBOARD_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_CAIROBOARD,           \
                                      GibbonCairoboardPrivate))
G_DEFINE_TYPE (GibbonCairoboard, gibbon_cairoboard, GTK_TYPE_DRAWING_AREA);

static gboolean gibbon_cairoboard_expose (GtkWidget *object, 
                                          GdkEventExpose *event);
static void gibbon_cairoboard_draw (GibbonCairoboard *board, cairo_t *cr);
static void gibbon_draw_bar (GibbonCairoboard *board, cairo_t *cr,
                             gint checkers);
static void gibbon_draw_bearoff (GibbonCairoboard *board, cairo_t *cr,
                                 gint checkers);
static void gibbon_draw_point (GibbonCairoboard *board, cairo_t *cr,
                               guint pos, gint checkers);
static void gibbon_draw_flat_checker (GibbonCairoboard *board, cairo_t *cr,
                                      guint number,
                                      double x, double y,
                                      struct GibbonColor *color,
                                      struct GibbonColor *text_color);
static void gibbon_draw_cube (GibbonCairoboard *board, cairo_t *cr,
                              guint value, gint side);
static void gibbon_draw_dice (GibbonCairoboard *board, cairo_t *cr,
                              guint left, guint right, gint side);
static void gibbon_write_text (GibbonCairoboard *board, cairo_t *cr,
                               const gchar *text,
                               double x, double y, double ext);
static void gibbon_draw_die (GibbonCairoboard *board, cairo_t *cr, 
                             guint value, gint side,
                             double x, double y);

#ifdef M_PI
# undef M_PI
#endif
#define M_PI 3.14159265358979323846

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
gibbon_cairoboard_new (const GibbonDesign *design)
{
        GibbonCairoboard *self = g_object_new (GIBBON_TYPE_CAIROBOARD, NULL);

        self->priv->design = design;

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
        double design_width;
        double design_height;
        double widget_ratio;
        double translate_x, translate_y, scale;
        struct GibbonColor frame_color = { 0.2, 0.15, 0, 1 };
        double outer_border_w = 10;
        double outer_border_h = 10;
        double checker_width = 30;
        double checker_height = 10;
        double point_width = checker_width;
        double point_length = 5 * checker_width;
        struct GibbonColor board_color = { 0.4, 0.25, 0, 1 };
        struct GibbonColor bearoff_color = board_color;
        double board_x, board_y;
        double dice_area_height = 2 * checker_width;
        struct GibbonColor point_color1 = { 0.6, 0, 0, 1 };
        struct GibbonColor point_color2 = { 0.5, 0.5, 0.5, 1 };
        double aspect_ratio;

        gint i;
        gint checkers[28] = { 3, -3, 
                1, 0, 0, 0, 0, 6,
                0, 0, 0, -2, 0, 0,
                0, 0, 2, 0, 0, 0,
                -6, 0, 0, 0, 0, -1,
                3, -3 };
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        
        design_width = gibbon_design_get_width (self->priv->design);
        design_height = gibbon_design_get_height (self->priv->design);
        aspect_ratio = gibbon_design_get_aspect_ratio (self->priv->design);
        
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
        
        if (checkers[0] != 0)
                gibbon_draw_bar (self, cr, checkers[0]);
        if (checkers[1] != 0)
                gibbon_draw_bar (self, cr, checkers[1]);
                
        for (i = 2; i < 26; ++i)
                if (checkers[i])
                        gibbon_draw_point (self, cr, i - 2, checkers[i]);
        
        if (checkers[26] != 0)
                gibbon_draw_bearoff (self, cr, checkers[26]);
        if (checkers[27] != 0)
                gibbon_draw_bearoff (self, cr, checkers[27]);
                
        gibbon_draw_cube (self, cr, 2, 0);
        gibbon_draw_dice (self, cr, 6, 5, -1);
}

static void
gibbon_draw_bar (GibbonCairoboard *self, cairo_t *cr, gint checkers)
{
        struct GibbonColor black = { 0, 0, 0, 1 };
        struct GibbonColor white = { 0.9, 0.9, 0.9, 1 };
        struct GibbonColor *color;
        struct GibbonColor text_on_black = { 0.9, 0.9, 0.9, 1 };
        struct GibbonColor text_on_white = { 0, 0, 0, 1 };
        struct GibbonColor *text_color;
        double checker_width = 30;
        double design_width = 490;
        double design_height = 380;
        double x = design_width / 2, y = design_height / 2;
        gint direction;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        g_return_if_fail (checkers != 0);
        
        if (checkers < 0) {
                color = &black;
                text_color = &text_on_black;
                direction = -1;
                checkers = -checkers;
        } else if (checkers > 0) {
                color = &white;
                text_color = &text_on_white;
                direction = 1;
        }
        
        y += direction * checker_width;
        gibbon_draw_flat_checker (self, cr, ((guint) checkers + 1) / 2,
                                  x, y, color, text_color);
        
        if (checkers < 2)
                return;
                
        y += direction * checker_width;
        gibbon_draw_flat_checker (self, cr, (guint) checkers / 2,
                                  x, y, color, text_color);
}

static void
gibbon_draw_bearoff (GibbonCairoboard *self, cairo_t *cr, gint checkers)
{
        struct GibbonColor black = { 0, 0, 0, 1 };
        struct GibbonColor white = { 0.9, 0.9, 0.9, 1 };
        struct GibbonColor shade_on_black = { 0.9, 0.9, 0.9, 1 };
        struct GibbonColor shade_on_white = { 0, 0, 0, 1 };
        struct GibbonColor *color;
        struct GibbonColor *shade_color;
        double checker_width = 30;
        double checker_height = 10;
        double design_width = 490;
        double design_height = 380;
        double outer_width = 10;
        double outer_height = 10;
        double x = design_width - outer_width - checker_width;
        double y;
        double separator_width = 0.2;
        gint direction;
        gint i;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        g_return_if_fail (checkers != 0);
        
        if (checkers < 0) {
                color = &black;
                shade_color = &shade_on_black;
                direction = -1;
                checkers = -checkers;
                y = design_height - outer_height - checker_height;
        } else if (checkers > 0) {
                color = &white;
                shade_color = &shade_on_white;
                direction = 1;
                y = outer_height;
        }
        
        cairo_set_source_rgb (cr,
                              color->red,
                              color->green,
                              color->blue);         
        for (i = 0; i < checkers; ++i) {
                cairo_rectangle (cr, x, y + i * direction * checker_height,
                                 checker_width, checker_height);
                cairo_fill (cr);
        }
        
        cairo_set_source_rgb (cr,
                              shade_color->red,
                              shade_color->green,
                              shade_color->blue);
        cairo_set_line_width (cr, separator_width);
        cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
        cairo_set_dash (cr, 0, 0, 0);
        for (i = 0; i < checkers; ++i) {
                if (direction > 0)
                        cairo_move_to (cr, x, 
                                       y + ((i + 1) * direction 
                                            * checker_height));
                else
                        cairo_move_to (cr, x,
                                       y + i * direction * checker_height);
                cairo_rel_line_to (cr, checker_width, 0);
                cairo_stroke (cr);
        }
}

static void
gibbon_draw_flat_checker (GibbonCairoboard *self, cairo_t *cr, guint number,
                          double x, double y, 
                          struct GibbonColor *color,
                          struct GibbonColor *text_color)
{
        double checker_width = 30;
        double text_width = 10;
        gchar *texts[] = { "2", "3", "4", "5", "6", "7", "8" };
        const gchar *font_family = "sans-serif";
        const cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;
        const cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_NORMAL;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        
        cairo_set_source_rgb (cr,
                              color->red,
                              color->green,
                              color->blue);
         
        cairo_arc (cr, x, y, checker_width / 2, 0, 2 * M_PI);

        cairo_fill (cr);
        
        /* On normal points, we can only have a maximum of 3 checkers,
         * on the bar it is 15: We have to spaces for the bar, and a
         * maximum of 15 checkers.
         */
        g_return_if_fail (number <= 8);
        
        if (number > 1) {
                cairo_select_font_face (cr, font_family, slant, weight);
                cairo_set_source_rgb (cr,
                                      text_color->red,
                                      text_color->green,
                                      text_color->blue);
                gibbon_write_text (self, cr, texts[number - 2], 
                                   x, y, 
                                   text_width);
        }        
}

static void
gibbon_draw_point (GibbonCairoboard *self, cairo_t *cr, 
                   guint pos, gint checkers)
{
        struct GibbonColor black = { 0, 0, 0, 1 };
        struct GibbonColor white = { 0.9, 0.9, 0.9, 1 };
        struct GibbonColor *color;
        struct GibbonColor text_on_white = { 0, 0, 0, 1 };
        struct GibbonColor text_on_black = { 0.9, 0.9, 0.9, 1 };
        struct GibbonColor *text_color;
        double x, y;
        double design_width = 490;
        double design_height = 380;
        double outer_border_h = 10;
        double checker_width = 30;
        double bar_width = 30;
        double point_width = 30;
        
        gint direction;
        gint i;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        g_return_if_fail (checkers != 0);
        g_return_if_fail (pos < 24);
        
        if (checkers < 0) {
                color = &black;
                text_color = &text_on_black;
                checkers = -checkers;
        } else if (checkers > 0) {
                color = &white;
                text_color = &text_on_white;
        }
        
        if (pos < 6) {
                x = design_width / 2 + bar_width / 2 + point_width / 2
                        + (5 - pos) * point_width;
                y = outer_border_h + checker_width / 2;
                direction = 1;
        } else if (pos < 12) {
                x = design_width / 2 - bar_width / 2 - point_width / 2
                        - (11 - pos) * point_width;
                y = outer_border_h + checker_width / 2;
                direction = 1;
        } else if (pos < 18) {
                x = design_width / 2 - bar_width / 2 - point_width / 2
                        - (16 - pos) * point_width;
                y = design_height - outer_border_h - checker_width / 2;
                direction = -1;
        } else {
                x = design_width / 2 + bar_width / 2 + point_width / 2
                        + (pos - 18) * point_width;
                y = design_height - outer_border_h - checker_width / 2;
                direction = -1;
        }
        
        for (i = 0; i < 5 && i < checkers; ++i) {
                gibbon_draw_flat_checker (self, cr, 
                                          (checkers + 4 - i) / 5, 
                                          x, 
                                          y + (direction * i 
                                               * checker_width), 
                                          color, text_color);
        }
}

static void
gibbon_draw_cube (GibbonCairoboard *self, cairo_t *cr, 
                  guint value, gint side)
{
        double cube_width = 30;
        double cube_text_width = 22;
        struct GibbonColor foreground = { 0, 0, 0, 1 };
        struct GibbonColor background = { 0.9, 0.9, 0.9, 1 };
        double design_height = 380;
        double outer_width = 10;
        double outer_height = 10;
        double checker_width = 30;
        double x = outer_width + checker_width / 2 - cube_width / 2;
        double y = design_height / 2 - cube_width / 2;
        const gchar *font_family = "serif";
        const cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;
        const cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_NORMAL;
        gchar *text;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        
        cairo_set_source_rgb (cr,
                              background.red,
                              background.green,
                              background.blue);

        x = outer_width + checker_width / 2 - cube_width / 2;

        if (side < 0) {
                y = design_height - outer_height - cube_width;                
        } else if (side > 0) {
                y = outer_height;
        } else {
                y = design_height / 2 - cube_width / 2;
                value = 2;
        }
        
        cairo_rectangle (cr, x, y, cube_width, cube_width);
        cairo_fill (cr);
        
        cairo_select_font_face (cr, font_family, slant, weight);
        cairo_set_source_rgb (cr,
                              foreground.red,
                              foreground.green,
                              foreground.blue);
        
        text = g_strdup_printf ("%u", (unsigned) value);
        gibbon_write_text (self, cr, text, 
                           x + cube_width / 2, y + cube_width / 2, 
                           cube_text_width);
        g_free (text);
}

static void gibbon_write_text (GibbonCairoboard *self, cairo_t *cr,
                               const gchar *text,
                               double x, double y, double ext)
{
        cairo_text_extents_t te;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));

        cairo_set_font_size (cr, ext);
        cairo_text_extents (cr, text, &te);
        cairo_move_to (cr, 
                       x - te.width / 2 - te.x_bearing, 
                       y + te.height / 2);
        cairo_show_text (cr, text);
}

static void
gibbon_draw_dice (GibbonCairoboard *self, cairo_t *cr, guint left, guint right,
                  gint side)
{
        double design_width = 490;
        double design_height = 380;
        double y;
        double die_width = 20;
        double bar_width = 30;
        double point_width = 30;
                
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        
        g_return_if_fail (left);
        g_return_if_fail (right);
        g_return_if_fail (left <= 6);
        g_return_if_fail (right <= 6);
       
        y = design_height / 2;

        gibbon_draw_die (self, cr, 
                         left,
                         side,
                         design_width / 2 + bar_width / 2 + 3 * point_width
                         - 0.75 * die_width, 
                         y);
        gibbon_draw_die (self, cr, 
                         right,
                         side,
                         design_width / 2 + bar_width / 2 + 3 * point_width
                         + 0.75 * die_width, 
                         y);
}

static void
gibbon_draw_die (GibbonCairoboard *self, cairo_t *cr, 
                 guint value, gint side,
                 double x, double y)
{
        struct GibbonColor black = { 0, 0, 0, 1 };
        struct GibbonColor white = { 0.9, 0.9, 0.9, 1 };
        struct GibbonColor fg_on_black = { 0.9, 0.9, 0.9, 1 };
        struct GibbonColor fg_on_white = { 0, 0, 0, 1 };
        struct GibbonColor *bg_color;
        struct GibbonColor *fg_color;
        double die_width = 20;
        double eye_radius = 2;
        double eye_distance = 5;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self)); 
        g_return_if_fail (value <= 6);
        g_return_if_fail (side != 0);
        
        if (side < 0) {
                bg_color = &black;
                fg_color = &fg_on_black;
        } else {
                bg_color = &white;
                fg_color = &fg_on_white;
        }
        
        cairo_set_source_rgb (cr,
                              bg_color->red,
                              bg_color->green,
                              bg_color->blue);
        
        x = x - die_width / 2;
        y = y - die_width / 2;
        
        cairo_rectangle (cr, 
                         x, y,
                         die_width, die_width);
        cairo_fill (cr);

        cairo_set_source_rgb (cr,
                              fg_color->red,
                              fg_color->green,
                              fg_color->blue);
                              
        /* Top left.  */
        if (value != 1) {
                cairo_arc (cr, 
                           x + die_width / 2 - eye_distance, 
                           y + die_width / 2 - eye_distance, 
                           eye_radius, 0, 2 * M_PI);
                cairo_fill (cr);
        }
        /* Top right.  */
        if (value > 3) {
                cairo_arc (cr, 
                           x + die_width / 2 + eye_distance, 
                           y + die_width / 2 - eye_distance, 
                           eye_radius, 0, 2 * M_PI);
                cairo_fill (cr);
        }
        /* Middle left.  */
        if (value == 6) {
                cairo_arc (cr, 
                           x + die_width / 2 - eye_distance, 
                           y + die_width / 2, 
                           eye_radius, 0, 2 * M_PI);
                cairo_fill (cr);
        }
        /* Middle right.  */
        if (value == 6) {
                cairo_arc (cr, 
                           x + die_width / 2 + eye_distance, 
                           y + die_width / 2, 
                           eye_radius, 0, 2 * M_PI);
                cairo_fill (cr);
        }
        /* Bottom left.  */
        if (value > 3) {
                cairo_arc (cr, 
                           x + die_width / 2 - eye_distance, 
                           y + die_width / 2 + eye_distance, 
                           eye_radius, 0, 2 * M_PI);
                cairo_fill (cr);
        }
        /* Bottom right.  */
        if (value != 1) {
                cairo_arc (cr, 
                           x + die_width / 2 + eye_distance, 
                           y + die_width / 2 + eye_distance, 
                           eye_radius, 0, 2 * M_PI);
                cairo_fill (cr);
        }
        /* Center.  */
        if (value == 1 || value == 3 || value == 5) {
                cairo_arc (cr, 
                           x + die_width / 2, 
                           y + die_width / 2, 
                           eye_radius, 0, 2 * M_PI);
                cairo_fill (cr);
        }
}