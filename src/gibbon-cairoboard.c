/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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
#include "svg-util.h"
#include "gibbon-board.h"

/* This lookup table determines, iff and where to draw a certain checker.
 * The index is the number of checkers, the first number is the position
 * in steps of 0.5 checker widths, the third one is the maximum number
 * when to draw it.  This takes into account that piled checkers may be
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
        GibbonApp *app;

        GibbonPosition *pos;
        
        GHashTable *ids;

        struct svg_component *board;
        
        struct svg_component *checker_w_flat;
        struct svg_component *checker_w_home;

        struct svg_component *checker_b_flat;
        struct svg_component *checker_b_home;

        struct svg_component *white_dice[6];
        struct svg_component *black_dice[6];
        struct svg_component *cube;

        struct svg_component *point12;
        struct svg_component *point24;
};

#define GIBBON_CAIROBOARD_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_CAIROBOARD,           \
                                      GibbonCairoboardPrivate))

static void gibbon_board_iface_init (GibbonBoardIface *iface);
G_DEFINE_TYPE_WITH_CODE (GibbonCairoboard, gibbon_cairoboard, GTK_TYPE_DRAWING_AREA,
                         G_IMPLEMENT_INTERFACE (GIBBON_TYPE_BOARD,
                                                gibbon_board_iface_init))

static void gibbon_cairoboard_set_position (GibbonBoard *self,
                                            GibbonPosition *pos);

static gboolean gibbon_cairoboard_expose (GtkWidget *object, 
                                          GdkEventExpose *event);
static void gibbon_cairoboard_draw (GibbonCairoboard *board, cairo_t *cr);
static void gibbon_cairoboard_draw_bar (GibbonCairoboard *board, cairo_t *cr, gint side);
static void gibbon_draw_home (GibbonCairoboard *board, cairo_t *cr,
                                  gint side);
static void gibbon_cairoboard_draw_point (GibbonCairoboard *board, cairo_t *cr,
                               guint point);
static void gibbon_cairoboard_draw_flat_checker (GibbonCairoboard *board, cairo_t *cr,
                                                 gdouble x, gdouble y,
                                                 gint side);
static void gibbon_cairoboard_draw_die (GibbonCairoboard *self, cairo_t *cr,
                                        gint value, guint die_pos);

static void gibbon_draw_cube (GibbonCairoboard *board, cairo_t *cr);
static void gibbon_draw_dice (GibbonCairoboard *board, cairo_t *cr);
static void gibbon_cairoboard_draw_svg_component (GibbonCairoboard *board, 
                                                  cairo_t *cr,
                                                  struct svg_component *svg,
                                                  gdouble x, gdouble y);

static void gibbon_cairoboard_save_ids (GibbonCairoboard *board,
                                        xmlNode *node);
static struct svg_component *
        gibbon_cairoboard_get_component (GibbonCairoboard *board,
                                         const gchar *id, gboolean render,
                                         xmlDoc *doc, const gchar *filename);

static gdouble gibbon_cairoboard_get_flat_checker_x (GibbonCairoboard *self,
                                                     guint point);
static gdouble gibbon_cairoboard_get_flat_checker_y (GibbonCairoboard *self,
                                                     guint point,
                                                     guint checker);
static gdouble gibbon_cairoboard_get_bar_x (GibbonCairoboard *self);
static gdouble gibbon_cairoboard_get_bar_y (GibbonCairoboard *self,
                                            guint checker, guint total,
                                            GibbonPositionSide side);
static gdouble gibbon_cairoboard_get_home_x (GibbonCairoboard *self,
                                             GibbonPositionSide side);

#ifdef M_PI
# undef M_PI
#endif
#define M_PI 3.14159265358979323846

static void
gibbon_cairoboard_init (GibbonCairoboard *self)
{
        size_t i;

        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, 
                                                  GIBBON_TYPE_CAIROBOARD, 
                                                  GibbonCairoboardPrivate);

        self->priv->app = NULL;
        self->priv->pos = NULL;

        self->priv->board = NULL;
        self->priv->point12 = NULL;
        self->priv->point24 = NULL;
        self->priv->checker_w_flat = NULL;
        self->priv->checker_w_home = NULL;
        self->priv->checker_b_flat = NULL;
        self->priv->checker_b_home = NULL;

        for (i = 0;
             i < sizeof self->priv->white_dice / sizeof self->priv->white_dice[0];
             ++i) {
            self->priv->white_dice[i] = NULL;
        }
        for (i = 0;
             i < sizeof self->priv->black_dice / sizeof self->priv->black_dice[0];
             ++i) {
            self->priv->black_dice[i] = NULL;
        }
        self->priv->cube = 0;

        return;
}

static void
gibbon_board_iface_init (GibbonBoardIface *iface)
{
        iface->set_position = gibbon_cairoboard_set_position;
}

static void
gibbon_cairoboard_finalize (GObject *object)
{
        GibbonCairoboard *self = GIBBON_CAIROBOARD (object);
        size_t i;

        if (self->priv->pos)
                gibbon_position_free (self->priv->pos);

        if (self->priv->ids)
                g_hash_table_destroy (self->priv->ids);
                
        if (self->priv->board)
                svg_util_free_component (self->priv->board);
        
        if (self->priv->point12)
                svg_util_free_component (self->priv->point12);
        if (self->priv->point24)
                svg_util_free_component (self->priv->point24);

        if (self->priv->checker_w_flat)
                svg_util_free_component (self->priv->checker_w_flat);
        if (self->priv->checker_w_home)
                svg_util_free_component (self->priv->checker_w_home);
        
        if (self->priv->checker_b_flat)
                svg_util_free_component (self->priv->checker_b_flat);
        if (self->priv->checker_b_home)
                svg_util_free_component (self->priv->checker_b_home);
        
        for (i = 0;
             i < sizeof self->priv->white_dice / sizeof self->priv->white_dice[0];
             ++i) {
            if (self->priv->white_dice[i])
                    svg_util_free_component (self->priv->white_dice[i]);
            self->priv->white_dice[i] = NULL;
        }
        for (i = 0;
             i < sizeof self->priv->black_dice / sizeof self->priv->black_dice[0];
             ++i) {
                if (self->priv->black_dice[i])
                        svg_util_free_component (self->priv->black_dice[i]);
            self->priv->black_dice[i] = NULL;
        }
        if (self->priv->cube)
                svg_util_free_component (self->priv->cube);

        self->priv->app = NULL;

        G_OBJECT_CLASS (gibbon_cairoboard_parent_class)->finalize (object);
}

static void
gibbon_cairoboard_class_init (GibbonCairoboardClass *klass)
{
        GtkDrawingAreaClass* parent_class = GTK_DRAWING_AREA_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonCairoboardPrivate));

        /* Initialize libxml.  */
        LIBXML_TEST_VERSION
        
        G_OBJECT_CLASS (parent_class)->finalize = gibbon_cairoboard_finalize;
        GTK_WIDGET_CLASS (parent_class)->expose_event = 
                gibbon_cairoboard_expose;
}

GibbonCairoboard *
gibbon_cairoboard_new (GibbonApp *app, const gchar *filename)
{
        GibbonCairoboard *self = g_object_new (GIBBON_TYPE_CAIROBOARD, NULL);
        gchar *data;
        GError *error;
        xmlDoc *doc;
        int i;
        char id_str[8];

        self->priv->app = app;
        self->priv->pos = gibbon_position_new ();

        error = NULL;
        if (!g_file_get_contents (filename, &data, NULL, &error)) {
                gibbon_app_display_error (app,
                                          _("Error reading board definition"
                                            " `%s': %s.\nDo you need to pass"
                                            " the option `--pixmaps-dir'?\n"),
                                          filename, error->message);
                g_error_free (error);
                g_object_ref_sink (self);
                return NULL;
        }
        
        doc = xmlReadMemory (data, strlen (data), filename, NULL, 0);
        if (doc == NULL) {
                gibbon_app_display_error (app,
                                          _("Error parsing board definition"
                                            " `%s'.\n"),
                                          filename);
                g_object_ref_sink (self);
                return NULL;
        }
        
        g_free (data);
        
        self->priv->ids = g_hash_table_new_full (g_str_hash, g_str_equal, 
                                                 xmlFree, NULL);        
        gibbon_cairoboard_save_ids (self, xmlDocGetRootElement (doc));

        self->priv->checker_w_flat =
                gibbon_cairoboard_get_component (self,
                                                 "checker_w_flat", TRUE,
                                                 doc, filename);
        if (!self->priv->checker_w_flat) {
                xmlFree (doc);
                g_object_ref_sink (self);
                return NULL;
        }

        self->priv->checker_w_home =
                gibbon_cairoboard_get_component (self,
                                                 "checker_w_home", TRUE,
                                                 doc, filename);
        if (!self->priv->checker_w_home) {
                xmlFree (doc);
                g_object_ref_sink (self);
                return NULL;
        }

        self->priv->checker_b_flat =
                gibbon_cairoboard_get_component (self,
                                                 "checker_b_flat", TRUE,
                                                 doc, filename);
        if (!self->priv->checker_b_flat) {
                xmlFree (doc);
                g_object_ref_sink (self);
                return NULL;
        }

        self->priv->checker_b_home =
                gibbon_cairoboard_get_component (self,
                                                 "checker_b_home", TRUE,
                                                 doc, filename);
        if (!self->priv->checker_b_home) {
                xmlFree (doc);
                g_object_ref_sink (self);
                return NULL;
        }

        strncpy (id_str, "die_w_6", 8);
        for (i = 0; i < 6; ++i) {
                id_str[6] = '1' + i;
                self->priv->white_dice[i] =
                        gibbon_cairoboard_get_component (self,
                                                         id_str, TRUE,
                                                         doc, filename);
                if (!self->priv->white_dice[i]) {
                        xmlFree (doc);
                        g_object_ref_sink (self);
                        return NULL;
                }
        }

        strncpy (id_str, "die_b_6", 8);
        for (i = 0; i < 6; ++i) {
                id_str[6] = '1' + i;
                self->priv->black_dice[i] =
                        gibbon_cairoboard_get_component (self,
                                                         id_str, TRUE,
                                                         doc, filename);
                if (!self->priv->black_dice[i]) {
                        xmlFree (doc);
                        g_object_ref_sink (self);
                        return NULL;
                }
        }

        self->priv->point12 = gibbon_cairoboard_get_component (self, "point12",
                                                               FALSE, doc,
                                                               filename);
        if (!self->priv->point12) {
                xmlFree (doc);
                g_object_ref_sink (self);
                return NULL;
        }

        self->priv->point24 = gibbon_cairoboard_get_component (self, "point24",
                                                               FALSE, doc,
                                                               filename);
        if (!self->priv->point24) {
                xmlFree (doc);
                g_object_ref_sink (self);
                return NULL;
        }

        self->priv->cube = gibbon_cairoboard_get_component (self, "cube",
                                                            TRUE, doc,
                                                            filename);
        if (!self->priv->cube) {
                xmlFree (doc);
                g_object_ref_sink (self);
                return NULL;
        }

        if (!svg_util_get_dimensions (xmlDocGetRootElement (doc), doc,
                                      filename, &self->priv->board, TRUE)) {
            g_object_ref_sink (self);
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
        gdouble width;
        gdouble height;
        
        gint i;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));

        width = self->priv->board->width;
        height = self->priv->board->height;

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

        gibbon_draw_dice (self, cr);
        gibbon_draw_cube (self, cr);

        gibbon_cairoboard_draw_bar (self, cr, GIBBON_POSITION_SIDE_WHITE);
        gibbon_cairoboard_draw_bar (self, cr, GIBBON_POSITION_SIDE_BLACK);
        
        gibbon_draw_home (self, cr, GIBBON_POSITION_SIDE_WHITE);
        gibbon_draw_home (self, cr, GIBBON_POSITION_SIDE_BLACK);

        for (i = 0; i < 24; ++i)
                if (self->priv->pos->points[i])
                        gibbon_cairoboard_draw_point (self, cr, i);
}

static void
gibbon_cairoboard_draw_bar (GibbonCairoboard *self, cairo_t *cr,
                            gint side)
{
        guint checkers;
        gdouble x, y;
        gint i;
        struct checker_rule *pos;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        g_return_if_fail (side);

        if (side == GIBBON_POSITION_SIDE_WHITE) {
                checkers = self->priv->pos->bar[0];
        } else if (side == GIBBON_POSITION_SIDE_BLACK) {
                checkers = self->priv->pos->bar[1];
        } else {
                return;
        }

        if (!checkers)
                return;

        if (checkers < 0)
                checkers = -1;
        g_return_if_fail (checkers <= 15);

        x = gibbon_cairoboard_get_bar_x (self);

        for (i = 0; i < checkers; ++i) {
                pos = checker_lookup + i;
                if (i <= pos->max_checkers) {
                        y = gibbon_cairoboard_get_bar_y (self, i,
                                                         checkers, side);
                        gibbon_cairoboard_draw_flat_checker (self, cr,
                                                             x, y,
                                                             side);
                }
        }
}

static void
gibbon_draw_home (GibbonCairoboard *self, cairo_t *cr, GibbonPositionSide side)
{
        guint checkers = 0;
        gint i;
        gdouble x, y;
        struct svg_component *checker;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        g_return_if_fail (side);

        for (i = 0; i < 24; ++i) {
                if (side * self->priv->pos->points[i] > 0)
                        checkers += side * self->priv->pos->points[i];
        }

        if (side == GIBBON_POSITION_SIDE_WHITE) {
                checkers -= self->priv->pos->bar[0];
        } else {
                checkers -= self->priv->pos->bar[1];
        }

        if (checkers >= 15)
                return;

        checkers = 15 - checkers;

        x = gibbon_cairoboard_get_home_x (self, side);

        if (side == GIBBON_POSITION_SIDE_WHITE) {
                checker = self->priv->checker_w_home;
                y = checker->y + 0.5 * checker->height;
        } else {
                checker = self->priv->checker_b_home;
                y = checker->y + checker->height
                    - 0.5 * checker->height;
        }

        for (i = 0; i < checkers; ++i) {
                gibbon_cairoboard_draw_svg_component (self, cr,
                                                      checker,
                                                      x, y);
                y += side * checker->height;
        }
}

static void
gibbon_cairoboard_draw_flat_checker (GibbonCairoboard *self, cairo_t *cr,
                                     gdouble x, gdouble y, gint side)
{
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        
        g_return_if_fail (side);
        
        if (side > 0) {
                gibbon_cairoboard_draw_svg_component (self, cr, 
                                                      self->priv->checker_w_flat,
                                                      x, y);
        } else if (side < 0) {
                gibbon_cairoboard_draw_svg_component (self, cr, 
                                                      self->priv->checker_b_flat,
                                                      x, y);
        }
}

static void
gibbon_cairoboard_draw_point (GibbonCairoboard *self, cairo_t *cr, guint point)
{
        gdouble x, y;
        GibbonPositionSide side = GIBBON_POSITION_SIDE_NONE;
        
        gint i;
        gint checkers;
        struct checker_rule *pos;
        
        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        g_return_if_fail (point < 24);

        checkers = self->priv->pos->points[point];
        if (checkers < 0) {
                checkers = -checkers;
                side = GIBBON_POSITION_SIDE_BLACK;
        } else if (checkers > 0) {
                side = GIBBON_POSITION_SIDE_WHITE;
        }
        g_return_if_fail (checkers <= 15);
        
        x = gibbon_cairoboard_get_flat_checker_x (self, point);
        for (i = 0; i < checkers; ++i) {
                pos = checker_lookup + i;
                if (i <= pos->max_checkers) {
                        y = gibbon_cairoboard_get_flat_checker_y (self, point,
                                                                  i);
                        gibbon_cairoboard_draw_flat_checker (self, cr, x, y,
                                                             side);
                }
        }
}

static void
gibbon_draw_cube (GibbonCairoboard *self, cairo_t *cr)
{
        gdouble x, y;
        gdouble saved_size;
        const gchar *saved_text;
        gchar *cube_value;
        gdouble scale;
        gdouble top, bottom;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));

        top = self->priv->checker_w_home->y;
        bottom = self->priv->checker_b_home->y
                 + self->priv->checker_b_home->height;

        if (self->priv->pos->may_double[0]) {
                if (self->priv->pos->may_double[1])
                        y = 0.5 * (top + bottom);
                else
                        y = top + 0.5 * self->priv->cube->height;
        } else if (self->priv->pos->may_double[1]) {
                y = bottom - 0.5 * self->priv->cube->height;

        } else {
                return;
        }

        x = self->priv->cube->x + 0.5 * self->priv->cube->width;

        cube_value = g_strdup_printf ("%d", self->priv->pos->cube);
        if (strlen (cube_value) > 2)
                scale = 2.0 / (gdouble) strlen (cube_value);
        else
                scale = 1.0;
        g_return_if_fail (svg_util_steal_text_params (self->priv->cube,
                                                      "cube-value",
                                                      cube_value, scale, 0,
                                                      &saved_text,
                                                      &saved_size));

        gibbon_cairoboard_draw_svg_component (self, cr, self->priv->cube, x, y);
        g_free (cube_value);

        g_return_if_fail (svg_util_steal_text_params (self->priv->cube,
                                                      "cube-value",
                                                      saved_text, 0,
                                                      saved_size,
                                                      NULL,
                                                      NULL));
}

static void
gibbon_draw_dice (GibbonCairoboard *self, cairo_t *cr)
{
        gint *dice;
        gint die_pos = 0;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));

        dice = self->priv->pos->dice;

        g_return_if_fail (dice[0] >= -6);
        g_return_if_fail (dice[0] <= +6);
        g_return_if_fail (dice[1] >= -6);
        g_return_if_fail (dice[1] <= +6);

        if (dice[0])
                gibbon_cairoboard_draw_die (self, cr, dice[0], die_pos);

        if (dice[1]) {
                if (dice[0] * dice[1] > 0)
                        die_pos = 1;
                gibbon_cairoboard_draw_die (self, cr, dice[1], die_pos);
        }
}

static void
gibbon_cairoboard_draw_die (GibbonCairoboard *self, cairo_t *cr,
                            gint value, guint die_pos)
{
        GibbonPositionSide side;
        gdouble x, y;
        struct svg_component *die;
        gdouble top, bottom;
        gdouble left, right;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));

        g_return_if_fail (value != 0);
        g_return_if_fail (value >= -6);
        g_return_if_fail (value <= +6);

        top = self->priv->point24->y;
        bottom = self->priv->point12->y + self->priv->point12->height;

        if (value < 0)
                side = GIBBON_POSITION_SIDE_BLACK;
        else if (value > 0)
                side = GIBBON_POSITION_SIDE_WHITE;
        else
                return;
        
        /* Normalize value.  */
        value *= side;

        if (side == GIBBON_POSITION_SIDE_BLACK) {
                die = self->priv->black_dice[value - 1];
                left = self->priv->point12->x;
                right = left + 6 * self->priv->point12->width;
        } else {
                die = self->priv->white_dice[value - 1];
                right = self->priv->point24->x + self->priv->point24->width;
                left = right - 6 * self->priv->point24->width;
        }

        x = 0.5 * (left + right) + (die_pos - 0.5) * 1.5 * die->width;
        y = 0.5 * (top + bottom);

        gibbon_cairoboard_draw_svg_component (self, cr, die, x, y);
}

static void
gibbon_cairoboard_set_position (GibbonBoard *_self,
                                GibbonPosition *pos)
{
        GibbonCairoboard *self;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (_self));
        g_return_if_fail (pos != NULL);
        
        self = GIBBON_CAIROBOARD (_self);

        if (self->priv->pos)
                gibbon_position_free (self->priv->pos);
        self->priv->pos = pos;
        
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
                gibbon_app_display_error (self->priv->app,
                                          _("Board definition `%s' does not"
                                            " have an element `%s'.\n"),
                                          filename, id);
                return NULL;
        }

        if (!svg_util_get_dimensions (node, doc, filename,
                                      &svg, render))
                return NULL;

        return svg;
}

static void 
gibbon_cairoboard_draw_svg_component (GibbonCairoboard *board, cairo_t *cr,
                                      struct svg_component *svg,
                                      gdouble x, gdouble y)
{
        gdouble cx, cy;

        cx = svg->x + svg->width / 2;
        cy = svg->y + svg->height / 2;

        cairo_translate (cr, x - cx, y - cy);
        svg_cairo_render (svg->scr, cr);
        cairo_translate (cr, cx - x, cy - y);
}

static gdouble
gibbon_cairoboard_get_flat_checker_x (GibbonCairoboard *self,
                                      guint point)
{
        g_return_val_if_fail (point < 24, 0.0);

        if (point < 6)
                return self->priv->point24->x
                       - (point - 0.5) * self->priv->point24->width;
        else if (point < 12)
                return self->priv->point12->x
                       + (12 - point - 0.5) * self->priv->point12->width;
        else if (point < 18)
                return self->priv->point12->x
                       + (0.5 + point - 12) * self->priv->point12->width;
        else
                return self->priv->point24->x
                       - (23 - point - 0.5) * self->priv->point24->width;
}

static gdouble
gibbon_cairoboard_get_flat_checker_y (GibbonCairoboard *self,
                                      guint point, guint checker)
{
        struct checker_rule *pos;

        g_return_val_if_fail (point < 24, 0.0);
        g_return_val_if_fail (checker < 15, 0.0);

        pos = checker_lookup + checker;

        if (point < 12) {
                return self->priv->checker_b_flat->y
                       + (0.5 - pos->pos) * self->priv->checker_b_flat->height;
        } else {
                return self->priv->checker_w_flat->y
                       + (0.5 + pos->pos) * self->priv->checker_w_flat->height;
        }
}

static gdouble
gibbon_cairoboard_get_bar_x (GibbonCairoboard *self)
{
        gdouble left = self->priv->point12->x;
        gdouble right = self->priv->point24->x + self->priv->point24->width;

        return 0.5 * (left + right);
}

static gdouble
gibbon_cairoboard_get_bar_y (GibbonCairoboard *self, guint checker_number,
                             guint total_checkers,
                             GibbonPositionSide side)
{
        struct checker_rule *pos;
        struct svg_component *checker;
        gdouble y;
        gdouble base_offset;

        g_return_val_if_fail (checker_number < 15, 0.0);

        base_offset = 0.5 * (gdouble) total_checkers;
        if (base_offset > 2)
                base_offset = 2;

        pos = checker_lookup + checker_number;

        if (side == GIBBON_POSITION_SIDE_BLACK) {
                checker = self->priv->checker_b_flat;
                y = self->priv->point12->y + self->priv->point12->height
                    - 3 * checker->height
                    + base_offset * checker->height;
        } else {
                checker = self->priv->checker_w_flat;
                y = self->priv->point24->y
                     + 3.5 * checker->height
                     - base_offset * checker->height;
        }

        y += side * pos->pos * checker->height;

        return y;
}

static gdouble
gibbon_cairoboard_get_home_x (GibbonCairoboard *self, GibbonPositionSide side)
{
        g_return_val_if_fail (side != GIBBON_POSITION_SIDE_NONE, 0.0);

        if (side == GIBBON_POSITION_SIDE_BLACK)
                return self->priv->checker_b_home->x
                       + 0.5 * self->priv->checker_b_home->width;
        else
                return self->priv->checker_w_home->x
                       + 0.5 * self->priv->checker_w_home->width;
}
