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
#include "game.h"
#include "gui.h"
#include "svg-util.h"

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
        struct GibbonPosition pos;
        
        GHashTable *ids;

        struct svg_component *board;
        
        struct svg_component *white_checker;
        struct svg_component *black_checker;
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
#if (0)
static void gibbon_write_text (GibbonCairoboard *board, cairo_t *cr,
                               const gchar *text,
                               gdouble x, gdouble y, gdouble ext);
#endif
#if (0)
static void gibbon_draw_die (GibbonCairoboard *board, cairo_t *cr, 
                             guint value, gint side,
                             gdouble x, gdouble y);
#endif
static void gibbon_cairoboard_save_ids (GibbonCairoboard *board,
                                        xmlNode *node);
static struct svg_component *
        gibbon_cairoboard_get_component (GibbonCairoboard *board,
                                         const gchar *id, gboolean render,
                                         xmlDoc *doc, const gchar *filename);
   
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

        self->priv->board = NULL;
        self->priv->white_checker = NULL;
        self->priv->black_checker = NULL;

        return;
}

static void
gibbon_cairoboard_finalize (GObject *object)
{
        GibbonCairoboard *self = GIBBON_CAIROBOARD (object);
 
        if (self->priv->pos.player[0])
                g_free (self->priv->pos.player[0]);
        self->priv->pos.player[0] = NULL;
        if (self->priv->pos.player[1])
                g_free (self->priv->pos.player[1]);
        self->priv->pos.player[1] = NULL;

        if (self->priv->ids)
                g_hash_table_destroy (self->priv->ids);
        self->priv->ids = NULL;
                
        if (self->priv->board)
                svg_util_free_component (self->priv->board);
        self->priv->board = NULL;
        
        if (self->priv->white_checker)
                svg_util_free_component (self->priv->white_checker);
        self->priv->white_checker = NULL;
        
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
        gchar *data;
        GError *error;
        xmlDoc *doc;
        xmlNode *node;
                        
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
        
        g_free (data);
        
        self->priv->ids = g_hash_table_new_full (g_str_hash, g_str_equal, 
                                                 xmlFree, NULL);        
        gibbon_cairoboard_save_ids (self, xmlDocGetRootElement (doc));

        self->priv->white_checker = 
                gibbon_cairoboard_get_component (self,
                                                 "checker_w_24_1", TRUE,
                                                 doc, filename);
        if (!self->priv->white_checker) {
                xmlFree (doc);
                g_object_unref (self);
                return NULL;
        }

        if (!svg_util_get_dimensions (xmlDocGetRootElement (doc), doc, 
                                      filename, &self->priv->board, TRUE)) {
            g_object_unref (self);
            xmlFreeDoc (doc);
            return NULL;
        }

        xmlFreeDoc (doc);

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
        
        svg_cairo_get_size (self->priv->board->scr, &width, &height);
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

        svg_cairo_set_viewport_dimension (self->priv->board->scr,
                                          allocation->width,
                                          allocation->height);
        svg_cairo_render (self->priv->board->scr, cr);
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
        gint direction;
        guint checkers;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        g_return_if_fail (side);
        
        if (side < 0) {
                direction = -1;
                checkers = self->priv->pos.bar[0];
        } else if (side > 0) {
                direction = 1;
                checkers = self->priv->pos.bar[1];
        }
        
        if (!checkers)
                return;
}

static void
gibbon_draw_home (GibbonCairoboard *self, cairo_t *cr, gint side)
{
        gint direction;
        gint checkers;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        g_return_if_fail (side);
                
        if (side < 0) {
                direction = -1;
                checkers = self->priv->pos.home[0];
        } else {
                direction = 1;
                checkers = self->priv->pos.home[1];
        }

        if (checkers < 0)
                checkers = -checkers;
        g_return_if_fail (checkers <= 15);
        
}

static void
gibbon_draw_flat_checker (GibbonCairoboard *self, cairo_t *cr, guint number,
                          gdouble x, gdouble y, gint side)
{
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        
        g_return_if_fail (side);
        
        /* On normal points, we can only have a maximum of 3 checkers,
         * on the bar it is 15: We have two spaces for the bar, and a
         * maximum of 15 checkers.
         */
        g_return_if_fail (number <= 8);
        
#if (0)
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
#endif
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
        gint side;
        guint value;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        
        value = self->priv->pos.cube;

        if (value == 1) {
                side = 0;
                value = 2;
        } else if (self->priv->pos.may_double[0]) {
                side = 1;
        } else {
                side = -1;
        }
}

#if (0)
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
#endif

static void
gibbon_draw_dice (GibbonCairoboard *self, cairo_t *cr)
{
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        
        g_return_if_fail (self->priv->pos.dice[0][0] >= 0
                          && self->priv->pos.dice[0][0] <= 6);
        g_return_if_fail (self->priv->pos.dice[0][1] >= 0
                          && self->priv->pos.dice[0][1] <= 6);
        g_return_if_fail (self->priv->pos.dice[1][0] >= 0
                          && self->priv->pos.dice[1][0] <= 6);
        g_return_if_fail (self->priv->pos.dice[1][1] >= 0
                          && self->priv->pos.dice[1][1] <= 6);
       
}

#if (0)
static void
gibbon_draw_die (GibbonCairoboard *self, cairo_t *cr, 
                 guint value, gint side,
                 gdouble x, gdouble y)
{
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self)); 
        g_return_if_fail (value <= 6);
        g_return_if_fail (side != 0);
        
}
#endif

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
gibbon_cairoboard_save_ids (GibbonCairoboard *self, xmlNode *node) 
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
                                g_hash_table_insert (self->priv->ids, id, cur);
                }
                
                if (cur->children)
                        gibbon_cairoboard_save_ids (self, cur->children);
        }
}

static struct svg_component *
gibbon_cairoboard_get_component (GibbonCairoboard *self,
                                 const gchar *id, gboolean render,
                                 xmlDoc *doc, const gchar *filename)
{
        struct svg_component *svg;
        xmlNode *node = 
                g_hash_table_lookup (self->priv->ids, (const xmlChar *) id);


        if (!node) {
                display_error (_("Board definition `%s' does not have an "
                                 "element `%s'.\n"),
                               filename, "checker_w_24_1");
                return NULL;
        }

        if (!svg_util_get_dimensions (node, doc, filename, 
                                      &svg, render))
                return NULL;

        return svg;
}
