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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <math.h>

#include <glib/gi18n.h>

#include <svg.h>

#include "svg-util.h"
#include "gui.h"

typedef struct svg_util_render_context {
        gdouble min_x;
        gdouble min_y;
        gdouble width;
        gdouble height;
        
        /* Device state.  */
        gdouble x;
        gdouble y;
        gchar *font_family;
} svg_util_render_context;

static svg_status_t svg_util_begin_group (gpointer closure, double opacity);
static svg_status_t svg_util_begin_element (gpointer closure);
static svg_status_t svg_util_end_element (gpointer closure);
static svg_status_t svg_util_end_group (gpointer closure, double opacity);
static svg_status_t svg_util_move_to (gpointer closure, double x, double y);
static svg_status_t svg_util_line_to (gpointer closure, double x, double y);
static svg_status_t svg_util_curve_to (gpointer closure,
                                       double x1, double y1,
                                       double x2, double y2,
                                       double x3, double y3);
static svg_status_t svg_util_quadratic_curve_to (gpointer closure,
                                                 double x1, double y1,
                                                 double x2, double y2);
static svg_status_t svg_util_arc_to (gpointer closure,
                                     double rx,
                                     double ry,
                                     double x_axis_rotation,
                                     int large_arc_flag,
                                     int sweep_flag,
                                     double x,
                                     double y);
static svg_status_t svg_util_close_path (gpointer closure);
static svg_status_t svg_util_set_color (gpointer closure, const svg_color_t *color);
static svg_status_t svg_util_set_fill_opacity (gpointer closure, double fill_opacity);
static svg_status_t svg_util_set_fill_paint (gpointer closure, 
                                             const svg_paint_t *paint);
static svg_status_t svg_util_set_fill_rule (gpointer closure, 
                                            svg_fill_rule_t fill_rule);
static svg_status_t svg_util_set_font_family (gpointer closure, const char *family);
static svg_status_t svg_util_set_font_size (gpointer closure, double size);
static svg_status_t svg_util_set_font_style (gpointer closure, 
                                             svg_font_style_t font_style);
static svg_status_t svg_util_set_font_weight (gpointer closure, 
                                              unsigned int font_weight);
static svg_status_t svg_util_set_opacity (gpointer closure, double opacity);
static svg_status_t svg_util_set_stroke_dash_array (gpointer closure, 
                                                    double *dash_array, 
                                                    int num_dashes);
static svg_status_t svg_util_set_stroke_dash_offset (gpointer closure, 
                                                     svg_length_t *offset);
static svg_status_t svg_util_set_stroke_line_cap (gpointer closure, 
                                                  svg_stroke_line_cap_t 
                                                  line_cap);
static svg_status_t svg_util_set_stroke_line_join (gpointer closure, 
                                                   svg_stroke_line_join_t 
                                                   line_join);
static svg_status_t svg_util_set_stroke_miter_limit (gpointer closure, 
                                                     double limit);
static svg_status_t svg_util_set_stroke_opacity (gpointer closure, 
                                                 double stroke_opacity);
static svg_status_t svg_util_set_stroke_paint (gpointer closure, 
                                               const svg_paint_t *paint);
static svg_status_t svg_util_set_stroke_width (gpointer closure, 
                                               svg_length_t *width);
static svg_status_t svg_util_set_text_anchor (gpointer closure, 
                                              svg_text_anchor_t text_anchor);
static svg_status_t svg_util_transform (gpointer closure,
                                        double a, double b,
                                        double c, double d,
                                        double e, double f);
static svg_status_t svg_util_apply_view_box (gpointer closure,
                                             svg_view_box_t view_box,
                                             svg_length_t *width,
                                             svg_length_t *height);
static svg_status_t svg_util_set_viewport_dimension (gpointer closure,
                                                     svg_length_t *width,
                                                     svg_length_t *height);
static svg_status_t svg_util_render_line (gpointer closure,
                                          svg_length_t *x1,
                                          svg_length_t *y1,
                                          svg_length_t *x2,
                                          svg_length_t *y2);
static svg_status_t svg_util_render_path (gpointer closure);
static svg_status_t svg_util_render_ellipse (gpointer closure,
                                             svg_length_t *cx,
                                             svg_length_t *cy,
                                             svg_length_t *rx,
                                             svg_length_t *ry);
static svg_status_t svg_util_render_rect (gpointer closure,
                                          svg_length_t *x,
                                          svg_length_t *y,
                                          svg_length_t *width,
                                          svg_length_t *height,
                                          svg_length_t *rx,
                                          svg_length_t *ry);
static svg_status_t svg_util_render_text (gpointer closure,
                                          svg_length_t *x,
                                          svg_length_t *y,
                                          const char *utf8);
static svg_status_t svg_util_render_image (gpointer closure,
                                           unsigned char *data,
                                           unsigned int data_width,
                                           unsigned int data_height,
                                           svg_length_t *x,
                                           svg_length_t *y,
                                           svg_length_t *width,
                                           svg_length_t *height);

svg_render_engine_t svg_util_render_engine = {
        svg_util_begin_group,
        svg_util_begin_element,
        svg_util_end_element,
        svg_util_end_group,
        svg_util_move_to,
        svg_util_line_to,
        svg_util_curve_to,
        svg_util_quadratic_curve_to,
        svg_util_arc_to,
        svg_util_close_path,
        svg_util_set_color,
        svg_util_set_fill_opacity,
        svg_util_set_fill_paint,
        svg_util_set_fill_rule,
        svg_util_set_font_family,
        svg_util_set_font_size,
        svg_util_set_font_style,
        svg_util_set_font_weight,
        svg_util_set_opacity,
        svg_util_set_stroke_dash_array,
        svg_util_set_stroke_dash_offset,
        svg_util_set_stroke_line_cap,
        svg_util_set_stroke_line_join,
        svg_util_set_stroke_miter_limit,
        svg_util_set_stroke_opacity,
        svg_util_set_stroke_paint,
        svg_util_set_stroke_width,
        svg_util_set_text_anchor,
        svg_util_transform,
        svg_util_apply_view_box,
        svg_util_set_viewport_dimension,
        svg_util_render_line,
        svg_util_render_path,
        svg_util_render_ellipse,
        svg_util_render_rect,
        svg_util_render_text,
        svg_util_render_image
};

static const gchar *
svg_strerror (svg_status_t status)
{
        switch (status) {
                case SVG_STATUS_SUCCESS:
                        return _("No error (this should not happen)!");
                case SVG_STATUS_NO_MEMORY:
                        return _("Out of memory!");
                case SVG_STATUS_IO_ERROR:
                        return _("Input/output error!");
                case SVG_STATUS_FILE_NOT_FOUND:
                        return _("File not found!");
                case SVG_STATUS_INVALID_VALUE:
                        return _("Invalid value!");
                case SVG_STATUS_INVALID_CALL:
                        return _("Invalid call!");
                case SVG_STATUS_PARSE_ERROR:
                        return _("Parse error!");
        }
        
        return _("Unknown error!");
}

gboolean
svg_util_get_dimensions (xmlNode *node, xmlDoc *doc, const gchar *filename,
                         gdouble *x, gdouble *y,
                         gdouble *width, gdouble *height)
{
        xmlBuffer* buf;
        gchar *xml_src;
        gchar *saved_locale;
        svg_t *svg = NULL;
        svg_status_t status;
        svg_util_render_context ctx;
                
        g_return_val_if_fail (node, FALSE);
        g_return_val_if_fail (x, FALSE);
        g_return_val_if_fail (y, FALSE);
        g_return_val_if_fail (width, FALSE);
        g_return_val_if_fail (height, FALSE);
        
        status = svg_create (&svg);
        if (status != SVG_STATUS_SUCCESS) {
                display_error (_("Error initializing SVG parser: %s\n"),
                               svg_strerror (status));
                return FALSE;
        }

        buf = xmlBufferCreate ();
        xmlNodeDump (buf, doc, node, 0, 0);
        xml_src = strcmp ("svg", (char *) node->name) ?
                g_strdup_printf ("<svg>%s</svg>", buf->content)
                : g_strdup ((gchar *) buf->content);
        xmlBufferFree (buf);

        saved_locale = setlocale (LC_NUMERIC, "POSIX");
        status = svg_parse_buffer (svg, xml_src, strlen (xml_src));
        setlocale (LC_NUMERIC, saved_locale);
        g_free (xml_src);
        if (status != SVG_STATUS_SUCCESS) {
                display_error (_("Error parsing SVG file `%s': %s\n"),
                               filename, svg_strerror (status));
                (void) svg_destroy (svg);
                
                return FALSE;
        }

        memset (&ctx, 0, sizeof ctx);
        
        ctx.min_x = INFINITY;
        ctx.min_y = INFINITY;
                
        status = svg_render (svg, &svg_util_render_engine, &ctx);
        (void) svg_destroy (svg);

        if (ctx.font_family)
                g_free (ctx.font_family);
                
        if (status != SVG_STATUS_SUCCESS) {
                display_error (_("Error getting SVG dimensions of `%s': %s.\n"),
                               filename, svg_strerror (status));
                (void) svg_destroy (svg);
                
                return FALSE;
        }
        
        
        *x = ctx.min_x;
        *y = ctx.min_y;
        *width = ctx.width;
        *height = ctx.height;
        
        return FALSE;
}

static svg_status_t
svg_util_begin_group (gpointer closure, double opacity)
{ 
        return SVG_STATUS_SUCCESS;
}

static svg_status_t
svg_util_begin_element (gpointer closure)
{ 
        return SVG_STATUS_SUCCESS;
}

static svg_status_t
svg_util_end_element (gpointer closure)
{ 
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_end_group (gpointer closure, double opacity)
{ 
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_move_to (gpointer closure, double x, double y)
{
        svg_util_render_context *ctx = (svg_util_render_context *) closure;
        ctx->x = x;
        ctx->y = y;

        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_line_to (gpointer closure, double x, double y)
{
        svg_util_render_context *ctx = (svg_util_render_context *) closure;
        gdouble x1, x2;
        gdouble y1, y2;
        
        if (x > ctx->x) {
                x1 = ctx->x;
                x2 = x;
        } else {
                x1 = x;
                x2 = ctx->x;
        }

        if (y > ctx->y) {
                y1 = ctx->y;
                y2 = y;
        } else {
                y1 = y;
                y2 = ctx->y;
        }
        
        if (x1 < ctx->min_x)
                ctx->min_x = x1;
        if (y1 < ctx->min_y)
                ctx->min_y = y1;
        
        if (x2 - x1 > ctx->width)
                ctx->width = x2 - x1;
                
        if (y2 - y1 > ctx->height)
                ctx->height = y2 - y1;
                
        ctx->x = x;
        ctx->y = y;

        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_curve_to (gpointer closure,
                   double x1, double y1,
                   double x2, double y2,
                   double x3, double y3)
{
        g_print ("curve_to :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_quadratic_curve_to (gpointer closure,
                             double x1, double y1,
                             double x2, double y2)
{
        g_print ("quadratic_curve_to :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_arc_to (gpointer closure,
                 double rx,
                 double ry,
                 double x_axis_rotation,
                 int large_arc_flag,
                 int sweep_flag,
                 double x,
                 double y)
{ 
        g_print ("arc_to :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_close_path (gpointer closure)
{ 
        g_print ("close_path :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_color (gpointer closure, const svg_color_t *color)
{ 
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_fill_opacity (gpointer closure, double fill_opacity)
{ 
        return SVG_STATUS_SUCCESS; 
}
static svg_status_t
svg_util_set_fill_paint (gpointer closure, 
                         const svg_paint_t *paint)
{ 
        return SVG_STATUS_SUCCESS; 
}
static svg_status_t
svg_util_set_fill_rule (gpointer closure, 
                        svg_fill_rule_t fill_rule)
{ 
        g_print ("set_fill_rule :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_font_family (gpointer closure, const char *family)
{ 
        svg_util_render_context *ctx = (svg_util_render_context *) closure;

        if (ctx->font_family)
                g_free (ctx->font_family);
                
        /* g_strdup handles NULL arguments gracefully.  */
        ctx->font_family = g_strdup (ctx->font_family);
        
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_font_size (gpointer closure, double size)
{ 
        g_print ("set_font_size :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_font_style (gpointer closure, 
                         svg_font_style_t font_style)
{ 
        g_print ("set_font_style :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_font_weight (gpointer closure, 
                          unsigned int font_weight)
{ 
        g_print ("set_font_weight :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_opacity (gpointer closure, double opacity)
{ 
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_stroke_dash_array (gpointer closure, 
                                double *dash_array, 
                                int num_dashes)
{ 
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_stroke_dash_offset (gpointer closure, 
                                 svg_length_t *offset)
{ 
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_stroke_line_cap (gpointer closure, 
                              svg_stroke_line_cap_t 
                              line_cap)
{ 
        g_print ("set_stroke_line_cap :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_stroke_line_join (gpointer closure, 
                               svg_stroke_line_join_t 
                               line_join)
{ 
        g_print ("set_stroke_line_join :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_stroke_miter_limit (gpointer closure, 
                                 double limit)
{ 
        g_print ("set_stroke_miter_limit :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_stroke_opacity (gpointer closure, 
                             double stroke_opacity)
{ 
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_stroke_paint (gpointer closure, 
                           const svg_paint_t *paint)
{ 
        g_print ("set_stroke_paint :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_stroke_width (gpointer closure, 
                           svg_length_t *width)
{ 
        g_print ("set_stroke_width :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_text_anchor (gpointer closure, 
                          svg_text_anchor_t text_anchor)
{ 
        g_print ("set_text_anchor :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_transform (gpointer closure,
                    double a, double b,
                    double c, double d,
                    double e, double f)
{ 
        g_print ("transform :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_apply_view_box (gpointer closure,
                         svg_view_box_t view_box,
                         svg_length_t *width,
                         svg_length_t *height)
{ 
        g_print ("apply_view_box :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_viewport_dimension (gpointer closure,
                                 svg_length_t *width,
                                 svg_length_t *height)
{ 

        /* Looks like we can ignore that?  */
        
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_render_line (gpointer closure,
                      svg_length_t *x1,
                      svg_length_t *y1,
                      svg_length_t *x2,
                      svg_length_t *y2)
{ 
        g_print ("render_line :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_render_path (gpointer closure)
{ 
        g_print ("render_path :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_render_ellipse (gpointer closure,
                         svg_length_t *cx,
                         svg_length_t *cy,
                         svg_length_t *rx,
                         svg_length_t *ry)
{ 
        g_print ("render_ellipse :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_render_rect (gpointer closure,
                      svg_length_t *x,
                      svg_length_t *y,
                      svg_length_t *width,
                      svg_length_t *height,
                      svg_length_t *rx,
                      svg_length_t *ry)
{ 
        g_print ("render_rect :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_render_text (gpointer closure,
                      svg_length_t *x,
                      svg_length_t *y,
                      const char *utf8)
{ 
        g_print ("render_text :-(\n");
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_render_image (gpointer closure,
                       unsigned char *data,
                       unsigned int data_width,
                       unsigned int data_height,
                       svg_length_t *x,
                       svg_length_t *y,
                       svg_length_t *width,
                       svg_length_t *height)
{ 
        g_print ("render_image :-(\n");
        return SVG_STATUS_SUCCESS; 
}
