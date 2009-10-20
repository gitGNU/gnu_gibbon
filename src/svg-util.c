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
#ifdef M_PI
# undef M_PI
#endif
#define M_PI 3.14159265358979323846

#include <glib/gi18n.h>

#include <svg.h>

#include "svg-util.h"
#include "gui.h"

struct svg_util_render_state {
        cairo_matrix_t transform;
        
        gchar *font_family;
        gdouble font_size;
        guint font_weight;
        svg_font_style_t font_style;
        gboolean font_dirty;
        svg_text_anchor_t text_anchor;
        
        gdouble stroke_width;        
        
        struct svg_util_render_state *prev;
};

typedef struct svg_util_render_context {
        const gchar *filename;
        
        gdouble min_x;
        gdouble min_y;
        gdouble max_x;
        gdouble max_y;
        
        gdouble x;
        gdouble y;
        
        struct svg_util_render_state *state;
} svg_util_render_context;

static struct svg_util_render_state 
        *svg_util_push_state (struct svg_util_render_state *state);
static struct svg_util_render_state
        *svg_util_pop_state (struct svg_util_render_state *state);
        
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

static void bezier_boundings (gdouble x0, gdouble y0,
                              gdouble x1, gdouble y1,
                              gdouble x2, gdouble y2,
                              gdouble x3, gdouble y3,
                              gdouble *x, gdouble *y,
                              gdouble *width, gdouble *height);
static void bezier1d_boundings (gdouble x0, gdouble x1, gdouble x2, gdouble x3,
                                gdouble *x, gdouble *width);
static void arc_boundings (gdouble cx, gdouble cy, gdouble radius,
                           gdouble arc0, gdouble arc1,
                           gdouble *x, gdouble *y,
                           gdouble *width, gdouble *height);
static void update_boundings (struct svg_util_render_context *ctx,
                              gdouble x, gdouble y, 
                              gdouble width, gdouble height);

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

/*
 * This routine is buggy on purpose.  It can only measure out a simple subset
 * of SVG files.
 */
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
        
        ctx.filename = filename;
        
        ctx.min_x = INFINITY;
        ctx.min_y = INFINITY;
        ctx.max_x = -INFINITY;
        ctx.max_y = -INFINITY;

        ctx.state = svg_util_push_state (NULL);

        status = svg_render (svg, &svg_util_render_engine, &ctx);
        (void) svg_destroy (svg);

        while (ctx.state)
                ctx.state = svg_util_pop_state (ctx.state);

        if (status != SVG_STATUS_SUCCESS) {
                display_error (_("Error getting SVG dimensions of `%s': %s.\n"),
                               filename, svg_strerror (status));
                (void) svg_destroy (svg);
                
                return FALSE;
        }
        
        
        *x = ctx.min_x;
        *y = ctx.min_y;
        *width = ctx.max_x - ctx.min_x;
        *height = ctx.max_y - ctx.min_y;
        
        return FALSE;
}

static svg_status_t
svg_util_begin_group (gpointer closure, double opacity)
{
        svg_util_render_context *ctx = (svg_util_render_context *) closure;

        ctx->state = svg_util_push_state (ctx->state);

        return SVG_STATUS_SUCCESS;
}

static svg_status_t
svg_util_begin_element (gpointer closure)
{
        svg_util_render_context *ctx = (svg_util_render_context *) closure;

        ctx->state = svg_util_push_state (ctx->state);

        return SVG_STATUS_SUCCESS;
}

static svg_status_t
svg_util_end_element (gpointer closure)
{
        svg_util_render_context *ctx = (svg_util_render_context *) closure;
        
        ctx->state = svg_util_pop_state (ctx->state);
        
        /* Handle wrong stacking gracefully.  */
        g_warn_if_fail (ctx->state);
        if (!ctx->state)
                ctx->state = svg_util_push_state (NULL);
        
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_end_group (gpointer closure, double opacity)
{
        svg_util_render_context *ctx = (svg_util_render_context *) closure;

        ctx->state = svg_util_pop_state (ctx->state);
        
        /* Handle wrong stacking gracefully.  */
        g_warn_if_fail (ctx->state);
        if (!ctx->state)
                ctx->state = svg_util_push_state (NULL);
        
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
        
        if (x1 - ctx->state->stroke_width / 2 < ctx->min_x)
                ctx->min_x = x1 - ctx->state->stroke_width / 2;
        if (y1 - ctx->state->stroke_width / 2 < ctx->min_y)
                ctx->min_y = y1 - ctx->state->stroke_width / 2;
        
        if (x2 + ctx->state->stroke_width / 2 > ctx->max_x)
                ctx->max_x = x2 + ctx->state->stroke_width / 2;
                
        if (y2 + ctx->state->stroke_width / 2 > ctx->max_y)
                ctx->max_y = y2 + ctx->state->stroke_width / 2;
                
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
        svg_util_render_context *ctx = (svg_util_render_context *) closure;
        gdouble x, y, width, height;
        
        bezier_boundings (ctx->x, ctx->y, x1, y1, x2, y2, x3, y3,
                          &x, &y, &width, &height);

        ctx->x = x3;
        ctx->y = y3;
        
        if (x - ctx->state->stroke_width / 2 < ctx->min_x)
                ctx->min_x = x - ctx->state->stroke_width / 2;
        if (y - ctx->state->stroke_width / 2 < ctx->min_y)
                ctx->min_y = y;

        if (x + width + ctx->state->stroke_width / 2 > ctx->max_x)
                ctx->max_x = x + width + ctx->state->stroke_width / 2;
        if (y + height + ctx->state->stroke_width / 2 > ctx->max_y)
                ctx->max_y = y + height + ctx->state->stroke_width / 2;
                
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_quadratic_curve_to (gpointer closure,
                             double x1, double y1,
                             double x2, double y2)
{
        svg_util_render_context *ctx = (svg_util_render_context *) closure;
        gdouble x, y;
        
        x = ctx->x;
        y = ctx->y;
        
        return svg_util_curve_to (closure,
                                  x  + 2.0 / 3.0 * (x1 - x),  
                                  y  + 2.0 / 3.0 * (y1 - y),
                                  x2 + 2.0 / 3.0 * (x1 - x2), 
                                  y2 + 2.0 / 3.0 * (y1 - y2),
                                  x2, y2);
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
        /* Closing the path cannot change the convex hull of the current
         * path.  It is safe to ignore this event.
         */
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
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_font_family (gpointer closure, const char *family)
{ 
        svg_util_render_context *ctx = (svg_util_render_context *) closure;

        if (ctx->state->font_family)
                g_free (ctx->state->font_family);
                
        /* g_strdup handles NULL arguments gracefully.  */
        ctx->state->font_family = g_strdup (ctx->state->font_family);
        ctx->state->font_dirty = 1;
        
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_font_size (gpointer closure, double size)
{ 
        svg_util_render_context *ctx = (svg_util_render_context *) closure;
        
        ctx->state->font_size = size;
        ctx->state->font_dirty = 1;
        
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_font_style (gpointer closure, 
                         svg_font_style_t font_style)
{ 
        svg_util_render_context *ctx = (svg_util_render_context *) closure;
        
        ctx->state->font_style = font_style;
        ctx->state->font_dirty = 1;
        
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_font_weight (gpointer closure, 
                          unsigned int font_weight)
{ 
        svg_util_render_context *ctx = (svg_util_render_context *) closure;
        
        ctx->state->font_weight = font_weight;
        ctx->state->font_dirty = 1;

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
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_stroke_line_join (gpointer closure, 
                               svg_stroke_line_join_t 
                               line_join)
{ 
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_stroke_miter_limit (gpointer closure, 
                                 double limit)
{ 
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
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_stroke_width (gpointer closure, 
                           svg_length_t *width)
{ 
        svg_util_render_context *ctx = (svg_util_render_context *) closure;
        
        ctx->state->stroke_width = width->value;

        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_set_text_anchor (gpointer closure, 
                          svg_text_anchor_t text_anchor)
{ 
        svg_util_render_context *ctx = (svg_util_render_context *) closure;
        
        ctx->state->text_anchor = text_anchor;

        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_transform (gpointer closure,
                    double a, double b,
                    double c, double d,
                    double e, double f)
{ 
        g_print ("Transform: %f | %f | %f | %f | %f | %f.\n",
                a, b, c, d, e, f);
                        
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_apply_view_box (gpointer closure,
                         svg_view_box_t view_box,
                         svg_length_t *width,
                         svg_length_t *height)
{ 
        /* Ignored for now. :-( */
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
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_render_ellipse (gpointer closure,
                         svg_length_t *cx,
                         svg_length_t *cy,
                         svg_length_t *rx,
                         svg_length_t *ry)
{
        svg_util_render_context *ctx = (svg_util_render_context *) closure;
        
        if (cx->value - rx->value - ctx->state->stroke_width / 2 < ctx->min_x)
                ctx->min_x = cx->value - rx->value - ctx->state->stroke_width / 2;
        if (cy->value - ry->value - ctx->state->stroke_width / 2 < ctx->min_y)
                ctx->min_y = cy->value - ry->value - ctx->state->stroke_width / 2;
        if (cx->value + rx->value + ctx->state->stroke_width / 2 > ctx->max_x)
                ctx->max_x = cx->value + rx->value + ctx->state->stroke_width / 2;
        if (cy->value + ry->value + ctx->state->stroke_width / 2 > ctx->max_y)
                ctx->max_y = cy->value + ry->value + ctx->state->stroke_width / 2;
                
        ctx->x = cx->value - rx->value;
        ctx->y = cy->value - ry->value;
        
        return SVG_STATUS_SUCCESS; 
}

static svg_status_t
svg_util_render_rect (gpointer closure,
                      svg_length_t *_x,
                      svg_length_t *_y,
                      svg_length_t *_width,
                      svg_length_t *_height,
                      svg_length_t *_rx,
                      svg_length_t *_ry)
{ 
        svg_util_render_context *ctx = (svg_util_render_context *) closure;
        gdouble x = _x->value;
        gdouble y = _y->value;
        gdouble width = _width->value;
        gdouble height = _height->value;
        gdouble surplus = ctx->state->stroke_width / 2;
        
        if (x - surplus < ctx->min_x)
                ctx->min_x = x - surplus;
        if (y - surplus < ctx->min_y)
                ctx->min_y = y - surplus;
        if (x + surplus + width > ctx->max_x)
                ctx->max_x = x + surplus + width;
        if (y + surplus  + height > ctx->max_y)
                ctx->max_y = y + surplus + height;
        
        /* This is not completely accurate, if we draw a rounded rectangle.
         * However, libsvg-cairo also seems to rely on a move_to operation
         * after a rectangle is rendered, in order to initialize cairo's
         * current point.
         */
        ctx->x = x;
        ctx->y = y;
                
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

/* Given the four control points of a two-dimensional cubic Bézier curve, 
 * calculate the bounding rectangle.  We split the 2D Bézier into its two
 * single-dimensional components, ...
 */
static void
bezier_boundings (gdouble x0, gdouble y0,
                  gdouble x1, gdouble y1,
                  gdouble x2, gdouble y2,
                  gdouble x3, gdouble y3,
                  gdouble *x, gdouble *y, gdouble *width, gdouble *height)
{
        /* Divida et impera!  Split the 2D spline into two 1D splines.  */
        bezier1d_boundings (x0, x1, x2, x3, x, width);
        bezier1d_boundings (y0, y1, y2, y3, y, height);
}

/* ... and calculate the extrema of the component functions.  If they are
 * inside our definition interval from 0 to 1, they are candidates for
 * the extrema that have to compete against the values at the edges
 * 0 and 1.
 */
static void
bezier1d_boundings (gdouble x0, gdouble x1, gdouble x2, gdouble x3,
                    gdouble *x, gdouble *width)
{
        gdouble min_x = x0;
        gdouble max_x = x0;
        gdouble c0, c1, c2, c3;
        gdouble p, q, rad, root;
        gdouble t1 = 0, t2 = 0;
        gdouble f;

        if (x3 < min_x)
                min_x = x3;
        if (x3 > max_x)
                max_x = x3;

        /*
         * Calculate the component function:
         * 
         *     f(t) = c0 t^3 + c1 t^2 + c2 t + c3
         *
         * The formula is straightforward:
         */
        c0 = -x0 + 3 * x1 - 3 * x2 + x3;
        c1 = 3 * x0 - 6 * x1 + 3 * x2;
        c2 = -3 * x0 + 3 * x1;
        c3 = x0;

        /*
         * Now calculate df/dt:
         *
         *     f'(t) = 3c0 t^2 + 2 c1t + c2
         *
         * We need the roots of this function:
         */
        if (!c0) {
                if (c1)
                        t2 = -0.5 * (c2 / c1);
        } else if (!c1) {
                t2 = sqrt (c2 / c0);
                t2 = -t2;
        } else {
                p = (2 * c1) / (3 * c0);
                q = c2 / (3 * c0);
                rad = p * p - 4 * q;
                if (rad >= 0) {
                        root = sqrt (rad);
                        t1 = -p / 2 + root / 2;
                        t2 = -p / 2 - root / 2;
                }
        }

        if (t1 > 0 && t1 < 1) {
                f = c0 * t1 * t1 * t1 + c1 * t1 * t1 + c2 * t1 + c3;
                if (f < min_x)
                        min_x = f;
                else if (f > max_x)
                        max_x = f;
        }
        if (t2 > 0 && t2 < 1 && t2 != t1 && t2 >= 0 && t2 <= 1) {
                f = c0 * t2 * t2 * t2 + c1 * t2 * t2 + c2 * t2 + c3;
                if (f < min_x)
                        min_x = f;
                else if (f > max_x)
                        max_x = f;
        }

        *x = min_x;
        *width = max_x - min_x;
}

static struct svg_util_render_state 
        *svg_util_push_state (struct svg_util_render_state *state)
{
        struct svg_util_render_state *new_state = g_malloc (sizeof *new_state);

        memset (new_state, 0, sizeof *new_state);
                
        cairo_matrix_init_identity (&new_state->transform);

        /* Cairo default font size.  */
        new_state->font_size = 10;
        new_state->font_style = SVG_FONT_STYLE_NORMAL;
        new_state->font_weight = 1;
        new_state->font_dirty = TRUE;
        new_state->text_anchor = SVG_TEXT_ANCHOR_START;
                      
        new_state->prev = state;
        
        return new_state;
}

static struct svg_util_render_state
        *svg_util_pop_state (struct svg_util_render_state *state)
{
        struct svg_util_render_state *prev_state = state->prev;
        
        if (state->font_family)
                g_free (state->font_family);
        
        g_free (state);

        return prev_state;
}

static void 
arc_boundings (gdouble cx, gdouble cy, gdouble radius, 
               gdouble arc0, gdouble arc1,
               gdouble *x, gdouble *y,
               gdouble *width, gdouble *height)
{
        gdouble min_x, min_y, max_x, max_y;
        gdouble x0, x1, y0, y1;
        gint i;

        i = arc0 / (2 * M_PI);
        arc0 -= i * 2 * M_PI;
        arc1 -= i * 2 * M_PI;

        while (arc1 < arc0)
                arc1 += 2 * M_PI;
        
        if (arc1 - arc0 >= 2 * M_PI) {
                *x = cx - radius;
                *y = cy - radius;
                *width = 2 * radius;
                *height = 2 * radius;
                return;
        }

        while (arc1 > 2 * M_PI) {
                arc0 -= 2 * M_PI;
                arc1 -= 2 * M_PI;
        }
        while (arc0 < -2 * M_PI) {
                arc0 += 2 * M_PI;
                arc1 += 2 * M_PI;
        }

        /* We now have an arc with a minimum start angle of -2pi, and
         * a maximum end angle of +2pi, and we also know that the
         * maximum difference is 2pi.
         */
        x0 = radius * cos (arc0);
        y0 = radius * sin (arc0);
        x1 = radius * cos (arc1);
        y1 = radius * sin (arc1);

        if (x0 < x1) {
                min_x = x0;
                max_x = x1;
        } else {
                min_x = x1;
                max_x = x0;
        }

        if (y0 < y1) {
                min_y = y0;
                max_y = y1;
        } else {
                min_y = y1;
                max_y = y0;
        }

        if (arc0 <= 0 && arc1 >= 0)
                max_x = radius;
        if ((arc0 <= M_PI / 2 && arc1 >= M_PI / 2)
            || (arc0 <= -3 * M_PI / 2 && arc1 >= -3 * M_PI / 2))
                max_y = radius;
        if ((arc0 <= M_PI && arc1 >= M_PI)
            || (arc0 <= -M_PI && arc1 >= -M_PI))
                min_x = -radius;
        if ((arc0 <= -M_PI / 2 && arc1 >= -M_PI / 2)
            || (arc0 <= 3 * M_PI / 2 && arc1 >= 3 * M_PI / 2))
                min_y = -radius;

        *x = cx + min_x;
        *y = cx + min_y;

        *width = max_x - min_x;
        *height = max_y - min_y;
}