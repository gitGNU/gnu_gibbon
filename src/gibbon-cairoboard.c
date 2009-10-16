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

#include <stdlib.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <svg-cairo.h>
#include <libxml/parser.h>

#include "gibbon-cairoboard.h"
#include "gibbon-design.h"
#include "game.h"
#include "gui.h"

/* This lookup table determines, iff and where to draw a certain checker.
 * The index is the number of checkers, the first number is the position
 * in steps of 0.5 checker widths, the third one is the maximum number
 * when to draw it.  This takes into account that piled checkers may get
 * invisible.
 */
struct checker_rule {
        gdouble pos;
        guint max_checkers;
};
struct checker_rule checker_lookup[15] = {
        { 0.0,  1 },
        { 1.0, 10 },
        { 2.0, 11 },
        { 3.0, 12 },
        { 4.0,  5 },
        { 0.5,  6 },
        { 1.5, 13 },
        { 2.5, 14 },
        { 3.5,  9 },
        { 1.0, 10 },
        { 2.0, 11 },
        { 3.0, 12 },
        { 1.5, 13 },
        { 2.5, 14 },
        { 2.0, 15 }
};

struct _GibbonCairoboardPrivate {
        GibbonDesign *design;
        struct GibbonPosition pos;
        
        svg_cairo_t *scr;
};

#define GIBBON_CAIROBOARD_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_CAIROBOARD,           \
                                      GibbonCairoboardPrivate))
G_DEFINE_TYPE (GibbonCairoboard, gibbon_cairoboard, GTK_TYPE_DRAWING_AREA);

static gboolean gibbon_cairoboard_expose (GtkWidget *object, 
                                          GdkEventExpose *event);
static void gibbon_cairoboard_draw (GibbonCairoboard *board, cairo_t *cr);
static void gibbon_draw_bar (GibbonCairoboard *board, cairo_t *cr, gint side);
static void gibbon_draw_home (GibbonCairoboard *board, cairo_t *cr,
                                  gint side);
static void gibbon_draw_point (GibbonCairoboard *board, cairo_t *cr,
                               guint point);
static void gibbon_draw_flat_checker (GibbonCairoboard *board, cairo_t *cr,
                                      guint number,
                                       gdouble x, gdouble y,
                                      gint side);
static void gibbon_draw_cube (GibbonCairoboard *board, cairo_t *cr);
static void gibbon_draw_dice (GibbonCairoboard *board, cairo_t *cr);
static void gibbon_write_text (GibbonCairoboard *board, cairo_t *cr,
                               const gchar *text,
                               gdouble x, gdouble y, gdouble ext);
static void gibbon_draw_die (GibbonCairoboard *board, cairo_t *cr, 
                             guint value, gint side,
                             gdouble x, gdouble y);
static void gibbon_cairoboard_save_ids (GibbonCairoboard *board,
                                        xmlNode *node, GHashTable *id_hash);
static svg_cairo_t *gibbon_cairoboard_draw_node (GibbonCairoboard *board,
                                                 xmlNode *node, xmlDoc *doc);
                                                                                                                         
#ifdef M_PI
# undef M_PI
#endif
#define M_PI 3.14159265358979323846

static const gchar *
svg_cairo_strerror (svg_cairo_status_t status)
{
        switch (status) {
                case SVG_CAIRO_STATUS_SUCCESS:
                        return _("No error (this should not happen)!");
                case SVG_CAIRO_STATUS_NO_MEMORY:
                        return ("Out of memory!");
                case SVG_CAIRO_STATUS_IO_ERROR:
                        return ("Input/output error!");
                case SVG_CAIRO_STATUS_FILE_NOT_FOUND:
                        return ("File not found!");
                case SVG_CAIRO_STATUS_INVALID_VALUE:
                        return ("Invalid value!");
                case SVG_CAIRO_STATUS_INVALID_CALL:
                        return ("Invalid call!");
                case SVG_CAIRO_STATUS_PARSE_ERROR:
                        return ("Parse error!");
        }
        
        return _("Unknown error!");
}

static void
gibbon_cairoboard_init (GibbonCairoboard *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, 
                                                  GIBBON_TYPE_CAIROBOARD, 
                                                  GibbonCairoboardPrivate);
        
        self->priv->scr = NULL;

        return;
}

static void
gibbon_cairoboard_finalize (GObject *object)
{
        GibbonCairoboard *self = GIBBON_CAIROBOARD (object);
 
        if (self->priv->design)
                g_object_unref (self->priv->design);
        self->priv->design = NULL;
        
        if (self->priv->pos.player[0])
                g_free (self->priv->pos.player[0]);
        self->priv->pos.player[0] = NULL;
        if (self->priv->pos.player[1])
                g_free (self->priv->pos.player[1]);
        self->priv->pos.player[1] = NULL;
        
        if (self->priv->scr)
                svg_cairo_destroy (self->priv->scr);
        self->priv->scr = NULL;
        
        G_OBJECT_CLASS (gibbon_cairoboard_parent_class)->finalize (object);
}

static void
gibbon_cairoboard_class_init (GibbonCairoboardClass *klass)
{
        GtkDrawingAreaClass* parent_class = GTK_DRAWING_AREA_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonCairoboardPrivate));

        /* Initializ libxml.  */
        LIBXML_TEST_VERSION
        
        G_OBJECT_CLASS (parent_class)->finalize = gibbon_cairoboard_finalize;
        GTK_WIDGET_CLASS (parent_class)->expose_event = 
                gibbon_cairoboard_expose;
}

/* FIXME! How can we return NULL in the constructor?  */
GibbonCairoboard *
gibbon_cairoboard_new (const gchar *filename)
{
        GibbonCairoboard *self = g_object_new (GIBBON_TYPE_CAIROBOARD, NULL);
        svg_cairo_status_t status;
        gchar *saved_locale;
        gchar *data;
        GError *error;
        xmlDoc *doc;
        GHashTable *ids;
        xmlNode *node;
        svg_cairo_t *scr;
                
        if (!g_file_get_contents (filename, &data, NULL, &error)) {
                display_error (_("Error reading board definition `%s': %s\n"),
                                 filename, error->message);
                g_error_free (error);
                g_object_unref (self);
                return NULL;
        }
        
        doc = xmlReadMemory (data, strlen (data), filename, NULL, 0);
        if (doc == NULL) {
                display_error (_("Error parsing board definition `%s'.\n"),
                               filename);
                g_object_unref (self);
                return NULL;
        }
        
        ids = g_hash_table_new_full (g_str_hash, g_str_equal, 
                                     xmlFree, NULL);
        gibbon_cairoboard_save_ids (self, xmlDocGetRootElement (doc), ids);
        node = g_hash_table_lookup (ids, (const xmlChar *) "checker_w_24_1");
        if (!node) {
                display_error (_("Board definition `%s' does not have an "
                                 "element `%s'.\n"),
                               filename, "checker_w_24_1");
                g_object_unref (self);
                g_free (data);
                xmlFree (doc);
                return NULL;
        }
        scr = gibbon_cairoboard_draw_node (self, node, doc);
        g_hash_table_unref (ids);
                
        status = svg_cairo_create (&self->priv->scr);
        if (status != SVG_CAIRO_STATUS_SUCCESS) {
                display_error (_("Error creating libsvg-cairo context: %s\n"),
                               svg_cairo_strerror (status));
                g_object_unref (self);
                g_free (data);
                xmlFree (doc);
                return NULL;
        }
        
        /* Libsvg has a nasty bug: It parses doubles in a locale-dependent
         * manner.  We therefore have to fallback to th C locale while
         * parsing.  This is not thread-safe ...
         */
        saved_locale = setlocale (LC_NUMERIC, "POSIX");
        status = svg_cairo_parse_buffer (self->priv->scr, data, strlen (data));
        setlocale (LC_NUMERIC, saved_locale);
        if (status != SVG_CAIRO_STATUS_SUCCESS) {
                display_error (_("Error parsing `%s': %s\n"),
                               filename, svg_cairo_strerror (status));
                g_object_unref (self);
                g_free (data);
                xmlFree (doc);
                return NULL;
        }
        
        xmlFree (doc);
        g_free (data);
        
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
        gdouble widget_ratio;
        gdouble translate_x, translate_y, scale;
        gdouble aspect_ratio;
        unsigned int width;
        unsigned int height;

        gint i;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        
        svg_cairo_get_size (self->priv->scr, &width, &height);
g_print ("Size: %d x %d\n", width, height);
        aspect_ratio = (double) width / height;
        
        widget = GTK_WIDGET (self);
        allocation = &widget->allocation;
        
        if (!allocation->height)
                return;
        if (!allocation->width)
                return;
        
        widget_ratio = (gdouble) allocation->width / allocation->height;

        if (widget_ratio > aspect_ratio) {
                scale = (gdouble) allocation->height / height; 
                translate_y = 0;
                translate_x = (allocation->width 
                               - scale * width) / 2;
        } else {
                scale = (gdouble) allocation->width / width;
                translate_x = 0;
                translate_y = (allocation->height 
                               - scale * height) / 2;
        }

        cairo_translate (cr, translate_x, translate_y);
        cairo_scale (cr, scale, scale);

        svg_cairo_set_viewport_dimension (self->priv->scr,
                                          allocation->width,
                                          allocation->height);
        svg_cairo_render (self->priv->scr, cr);
return;
        
        gibbon_draw_bar (self, cr, -1);
        gibbon_draw_bar (self, cr, 1);
        
        for (i = 0; i < 24; ++i)
                if (self->priv->pos.checkers[i])
                        gibbon_draw_point (self, cr, i);

        gibbon_draw_home (self, cr, -1);
        gibbon_draw_home (self, cr, 1);

        gibbon_draw_cube (self, cr);
        gibbon_draw_dice (self, cr);
}

static void
gibbon_draw_bar (GibbonCairoboard *self, cairo_t *cr, gint side)
{
        gdouble checker_width = 30;
        gdouble design_width = 490;
        gdouble design_height = 380;
        gdouble x, y;
        gint direction;
        guint checkers;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        g_return_if_fail (side);
        
        design_width = gibbon_design_get_width (self->priv->design);
        design_height = gibbon_design_get_height (self->priv->design);
        x = design_width / 2;
        y = design_height / 2;
        
        if (side < 0) {
                direction = -1;
                checkers = self->priv->pos.bar[0];
        } else if (side > 0) {
                direction = 1;
                checkers = self->priv->pos.bar[1];
        }
        
        if (!checkers)
                return;
                
        y += direction * checker_width;
        gibbon_draw_flat_checker (self, cr, ((guint) checkers + 1) / 2,
                                  x, y, side);
        
        if (checkers < 2)
                return;
                
        y += direction * checker_width;
        gibbon_draw_flat_checker (self, cr, (guint) checkers / 2,
                                  x, y, side);
}

static void
gibbon_draw_home (GibbonCairoboard *self, cairo_t *cr, gint side)
{
        struct GibbonColor black = { 0, 0, 0, 1 };
        struct GibbonColor white = { 0.9, 0.9, 0.9, 1 };
        struct GibbonColor shade_on_black = { 0.9, 0.9, 0.9, 1 };
        struct GibbonColor shade_on_white = { 0, 0, 0, 1 };
        struct GibbonColor *color;
        struct GibbonColor *shade_color;
        gdouble checker_width = 30;
        gdouble checker_height = 10;
        gdouble design_width = 490;
        gdouble design_height = 380;
        gdouble outer_width = 10;
        gdouble outer_height = 10;
        gdouble x = design_width - outer_width - checker_width;
        gdouble y;
        gdouble separator_width = 0.2;
        gint direction;
        gint i;
        gint checkers;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        
        if (side < 0) {
                color = &black;
                shade_color = &shade_on_black;
                direction = -1;
                y = design_height - outer_height - checker_height;
                checkers = self->priv->pos.home[0];
        } else if (side > 0) {
                color = &white;
                shade_color = &shade_on_white;
                direction = 1;
                y = outer_height;
                checkers = self->priv->pos.home[1];
        } else {
                g_return_if_fail (side);
        }

        if (checkers < 0)
                checkers = -checkers;
        g_return_if_fail (checkers <= 15);
        
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
                          gdouble x, gdouble y, gint side)
{
        struct GibbonColor black = { 0, 0, 0, 1 };
        struct GibbonColor white = { 0.9, 0.9, 0.9, 1 };
        struct GibbonColor *color;
        struct GibbonColor text_on_black = { 0.9, 0.9, 0.9, 1 };
        struct GibbonColor text_on_white = { 0, 0, 0, 1 };
        struct GibbonColor *text_color;

        gdouble checker_width = 30;
        gdouble text_width = 10;
        gchar *texts[] = { "2", "3", "4", "5", "6", "7", "8" };
        const gchar *font_family = "sans-serif";
        const cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;
        const cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_NORMAL;
        cairo_pattern_t *pat;
        gdouble shade_offset = checker_width / 15;
        gdouble checker_radius = checker_width / 2;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        
        g_return_if_fail (side);
        
        if (side < 0) {
                color = &black;
                text_color = &text_on_black;
        } else {
                color = &white;
                text_color = &text_on_white;
        }
        
        cairo_set_source_rgb (cr,
                              color->red,
                              color->green,
                              color->blue);
         
        cairo_arc (cr, x, y, checker_width / 2, 0, 2 * M_PI);

        cairo_fill (cr);

        if (side > 0) {        
                pat = cairo_pattern_create_radial (x - shade_offset, 
                                                   y - shade_offset / 2, 
                                                   checker_radius 
                                                       - shade_offset,
                                                   x - shade_offset, 
                                                   y - shade_offset / 2, 
                                                   checker_radius 
                                                       + shade_offset);
                cairo_pattern_add_color_stop_rgba (pat, 0, 0, 0, 0, 0);
                cairo_pattern_add_color_stop_rgba (pat, 1, 0, 0, 0, 1);
                cairo_set_source (cr, pat);
                cairo_arc (cr, x, y, checker_width / 2, 0, 2 * M_PI);
                cairo_fill (cr);
                cairo_pattern_destroy (pat);
        } else if (side < 0) {
                pat = cairo_pattern_create_radial (x + shade_offset, 
                                                   y + shade_offset / 2, 
                                                   checker_radius
                                                       - shade_offset / 2,
                                                   x + shade_offset, 
                                                   y + shade_offset / 2, 
                                                   checker_radius
                                                       + 1.5 * shade_offset);
                cairo_pattern_add_color_stop_rgba (pat, 0, 0.7, 0.7, 0.7, 0);
                cairo_pattern_add_color_stop_rgba (pat, 1, 0.7, 0.7, 0.7, 1);
                cairo_set_source (cr, pat);
                cairo_arc (cr, x, y, checker_width / 2, 0, 2 * M_PI);
                cairo_fill (cr);
                cairo_pattern_destroy (pat);
        }
        
        /* On normal points, we can only have a maximum of 3 checkers,
         * on the bar it is 15: We have two spaces for the bar, and a
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
gibbon_draw_point (GibbonCairoboard *self, cairo_t *cr, guint point)
{
        gdouble x, y;
        gdouble design_width = 490;
        gdouble design_height = 380;
        gdouble outer_border_h = 10;
        gdouble checker_width = 30;
        gdouble bar_width = 30;
        gdouble point_width = 30;
        int side;
        
        gint direction;
        gint i;
        gint checkers;
        struct checker_rule *pos;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        g_return_if_fail (point < 24);

        checkers = self->priv->pos.checkers[point];
        
        if (checkers < 0) {
                checkers = -checkers;
                side = -1;
        } else if (checkers > 0) {
                side = 1;
        }
        g_return_if_fail (checkers <= 15);
        
        if (point < 6) {
                x = design_width / 2 + bar_width / 2 + point_width / 2
                        + (5 - point) * point_width;
                y = outer_border_h + checker_width / 2;
                direction = 1;
        } else if (point < 12) {
                x = design_width / 2 - bar_width / 2 - point_width / 2
                        - (point - 6) * point_width;
                y = outer_border_h + checker_width / 2;
                direction = 1;
        } else if (point < 18) {
                x = design_width / 2 - bar_width / 2 - point_width / 2
                        - (17 - point) * point_width;
                y = design_height - outer_border_h - checker_width / 2;
                direction = -1;
        } else {
                x = design_width / 2 + bar_width / 2 + point_width / 2
                        + (point - 18) * point_width;
                y = design_height - outer_border_h - checker_width / 2;
                direction = -1;
        }
        
        for (i = 0; i < checkers; ++i) {
                pos = checker_lookup + i;
                if (i <= pos->max_checkers) {
                        gibbon_draw_flat_checker (self, cr, 
                                                  1, 
                                                  x, 
                                                  y + (direction * pos->pos 
                                                        * checker_width), 
                                                  side);
                }
        }
}

static void
gibbon_draw_cube (GibbonCairoboard *self, cairo_t *cr)
{
        gdouble cube_width = 30;
        gdouble cube_text_width = 22;
        struct GibbonColor foreground = { 0, 0, 0, 1 };
        struct GibbonColor background = { 0.9, 0.9, 0.9, 1 };
        gdouble design_height = 380;
        gdouble outer_width = 10;
        gdouble outer_height = 10;
        gdouble checker_width = 30;
        gdouble x = outer_width + checker_width / 2 - cube_width / 2;
        gdouble y = design_height / 2 - cube_width / 2;
        const gchar *font_family = "serif";
        const cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;
        const cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_NORMAL;
        gchar *text;
        guint value;
        gint side;
                
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        
        cairo_set_source_rgb (cr,
                              background.red,
                              background.green,
                              background.blue);

        x = outer_width + checker_width / 2 - cube_width / 2;

        value = self->priv->pos.cube;
        if (self->priv->pos.cube == 1) {
                side = 0;
                value = 2;
        } else if (self->priv->pos.may_double[0]) {
                side = 1;
        } else {
                side = -1;
        }
        
        if (side < 0) {
                y = design_height - outer_height - cube_width;                
        } else if (side > 0) {
                y = outer_height;
        } else {
                y = design_height / 2 - cube_width / 2;
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
                               gdouble x, gdouble y, gdouble ext)
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
gibbon_draw_dice (GibbonCairoboard *self, cairo_t *cr)
{
        gdouble design_width;
        gdouble design_height;
        gdouble y;
        gdouble die_width = 20;
        gdouble bar_width = 30;
        gdouble point_width = 30;
                
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        
        design_width = gibbon_design_get_width (self->priv->design);
        design_height = gibbon_design_get_height (self->priv->design);
        
        g_return_if_fail (self->priv->pos.dice[0][0] >= 0
                          && self->priv->pos.dice[0][0] <= 6);
        g_return_if_fail (self->priv->pos.dice[0][1] >= 0
                          && self->priv->pos.dice[0][1] <= 6);
        g_return_if_fail (self->priv->pos.dice[1][0] >= 0
                          && self->priv->pos.dice[1][0] <= 6);
        g_return_if_fail (self->priv->pos.dice[1][1] >= 0
                          && self->priv->pos.dice[1][1] <= 6);
       
        y = design_height / 2;

        if (self->priv->pos.dice[0][0]) {
                gibbon_draw_die (self, cr, 
                                 self->priv->pos.dice[0][0],
                                 -1,
                                 design_width / 2 + bar_width / 2 + 3 * point_width
                                 - 0.75 * die_width, 
                                 y);
        }
        
        if (self->priv->pos.dice[0][1]) {
                gibbon_draw_die (self, cr, 
                                 self->priv->pos.dice[0][1],
                                 -1,
                                 design_width / 2 + bar_width / 2 + 3 * point_width
                                 + 0.75 * die_width, 
                                 y);
        }

        if (self->priv->pos.dice[1][0]) {
                gibbon_draw_die (self, cr, 
                                 self->priv->pos.dice[1][0],
                                 1,
                                 design_width / 2 - bar_width / 2 - 3 * point_width
                                 - 0.75 * die_width, 
                                 y);
        }
        
        if (self->priv->pos.dice[1][1]) {
                gibbon_draw_die (self, cr, 
                                 self->priv->pos.dice[1][1],
                                 1,
                                 design_width / 2 - bar_width / 2 - 3 * point_width
                                 + 0.75 * die_width, 
                                 y);
        }

}

static void
gibbon_draw_die (GibbonCairoboard *self, cairo_t *cr, 
                 guint value, gint side,
                 gdouble x, gdouble y)
{
        struct GibbonColor black = { 0, 0, 0, 1 };
        struct GibbonColor white = { 0.9, 0.9, 0.9, 1 };
        struct GibbonColor fg_on_black = { 0.9, 0.9, 0.9, 1 };
        struct GibbonColor fg_on_white = { 0, 0, 0, 1 };
        struct GibbonColor *bg_color;
        struct GibbonColor *fg_color;
        gdouble die_width = 20;
        gdouble eye_radius = 2;
        gdouble eye_distance = 5;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self)); 
        g_return_if_fail (value <= 6);
        g_return_if_fail (side != 0);
        
        if (side < 0) {
                bg_color = &black;
                fg_color = &fg_on_black;
        } else if (side > 0) {
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

void
gibbon_cairoboard_set_position (GibbonCairoboard *self,
                                const struct GibbonPosition *pos)
{
        struct GibbonPosition *mypos;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self)); 
        g_return_if_fail (pos);
        
        mypos = &self->priv->pos;

        if (mypos->player[0])
                g_free (mypos->player[0]);
        if (mypos->player[1])
                g_free (mypos->player[1]);
                
        self->priv->pos = *pos;
        if (self->priv->pos.player[0])
                self->priv->pos.player[0] = 
                        g_strdup (self->priv->pos.player[0]);
        if (self->priv->pos.player[1])
                self->priv->pos.player[1] = 
                        g_strdup (self->priv->pos.player[1]);
        
        gtk_widget_queue_draw (GTK_WIDGET (self));
}

static void
gibbon_cairoboard_save_ids (GibbonCairoboard *self,
                            xmlNode *node, GHashTable *hash) 
{
        xmlNode *cur;
        xmlChar *id;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        g_return_if_fail (node != NULL);
        
        for (cur = node; cur != NULL; cur = cur->next) {
                if (cur->type == XML_ELEMENT_NODE) {
                        id = xmlGetProp (cur, (const xmlChar *) "id");
                        if (!id)
                                id = xmlGetProp (cur, 
                                                 (const xmlChar*) "xml:id");
                        if (id)
                                g_hash_table_insert (hash, id, cur);
                }
                
                if (cur->children)
                        gibbon_cairoboard_save_ids (self, cur->children, hash);
        }
}

static svg_cairo_t *
gibbon_cairoboard_draw_node (GibbonCairoboard *self, 
                             xmlNode *node, xmlDoc *doc)
{
        xmlBuffer* buf;
        gchar *svg;
        svg_cairo_t *scr = NULL;
        gchar *saved_locale;
        svg_cairo_status_t status;
        unsigned int width;
        unsigned int height;
                        
        g_return_val_if_fail (GIBBON_IS_CAIROBOARD (self), NULL);
        
        status = svg_cairo_create (&scr);
        if (status != SVG_CAIRO_STATUS_SUCCESS) {
                display_error (_("Error creating libsvg-cairo context: %s\n"),
                               svg_cairo_strerror (status));
                return NULL;
        }
                
        buf = xmlBufferCreate ();
        xmlNodeDump (buf, doc, node, 0, 0);
        svg = g_strdup_printf ("<svg>%s</svg>", buf->content);
        xmlBufferFree (buf);

        saved_locale = setlocale (LC_NUMERIC, "POSIX");
        status = svg_cairo_parse_buffer (scr, svg, strlen (svg));
        setlocale (LC_NUMERIC, saved_locale);
        if (status != SVG_CAIRO_STATUS_SUCCESS) {
                display_error (_("Error parsing internal SVG node: %s.\n"),
                               svg_cairo_strerror (status));
                g_free (svg);
                svg_cairo_destroy (scr);
                
                return NULL;
        }

        /* FIXME! This method in libsvg-cairo only works with a valid
         * viewBox or width and height in the svg root element.  We have
         * to include libsvg-cairo in gibbon, and make that more accurate.
         * Furthermore, the returned values must be double, not unsigned
         * integers.
         */
        svg_cairo_get_size (scr, &width, &height);
        g_print ("Checker dimensions: %u|%u.\n", width, height);
        g_free (svg);
                
        return scr;
}
