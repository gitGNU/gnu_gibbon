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

#include <stdlib.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <svg-cairo.h>
#include <libxml/parser.h>

#include "gibbon-cairoboard.h"
#include "svg-util.h"
#include "gibbon-board.h"

enum {
        GIBBON_CAIROBOARD_DICE_PICKED_UP,
        GIBBON_CAIROBOARD_CUBE_TURNED,
        GIBBON_CAIROBOARD_CUBE_TAKEN,
        GIBBON_CAIROBOARD_CUBE_DROPPED,
        GIBBON_CAIROBOARD_RESIGNATION_ACCEPTED,
        GIBBON_CAIROBOARD_RESIGNATION_REJECTED,
        LAST_SIGNAL
};

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

        GibbonPosition *target;
        GibbonMove *move;
        guint animation_id;
        gint animation_move_number;
        GibbonPositionSide animation_side;
        
        /* For each sub move we have these animations steps:
         *    -1: Start.
         *     0: Move the checker from the starting point.
         *     1: Move a possible hit checker to the bar.
         */
        gint animation_step;
        gint animation_promille;
        GibbonPositionSide animation_floating;
        gint animation_from, animation_to;

        gdouble translate_x, translate_y, scale;

        GHashTable *ids;

        struct svg_component *board;
        
        struct svg_component *checker_w_flat;
        struct svg_component *checker_w_home;

        struct svg_component *checker_b_flat;
        struct svg_component *checker_b_home;

        struct svg_component *white_dice[6];
        struct svg_component *black_dice[6];
        struct svg_component *cube;
        struct svg_component *flag;
        struct svg_component *cup;

        struct svg_component *point12;
        struct svg_component *point24;
};

#define ANIMATION_STEP_WIDTH 75
#define ANIMATION_TIMEOUT 25
#define DICE_FADE_OUT_TIMEOUT 750

#define GIBBON_CAIROBOARD_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_CAIROBOARD,           \
                                      GibbonCairoboardPrivate))

static void gibbon_board_iface_init (GibbonBoardIface *iface);
G_DEFINE_TYPE_WITH_CODE (GibbonCairoboard, gibbon_cairoboard, GTK_TYPE_DRAWING_AREA,
                         G_IMPLEMENT_INTERFACE (GIBBON_TYPE_BOARD,
                                                gibbon_board_iface_init))

static void gibbon_cairoboard_set_position (GibbonBoard *self,
                                            const GibbonPosition *pos);
static const GibbonPosition *gibbon_cairoboard_get_position (const GibbonBoard
                                                             *self);
static void gibbon_cairoboard_animate_move (GibbonBoard *self,
                                            const GibbonMove *move,
                                            GibbonPositionSide side,
                                            GibbonPosition *target_position);
static void gibbon_cairoboard_fade_out_dice (GibbonBoard *self);
static void gibbon_cairoboard_cancel_animation (GibbonCairoboard *self);
static gboolean gibbon_cairoboard_do_animation (GibbonCairoboard *self);
static gboolean gibbon_cairoboard_hide_dice (GibbonCairoboard *self);

static gboolean gibbon_cairoboard_expose (GtkWidget *object, 
                                          GdkEventExpose *event);
static void gibbon_cairoboard_draw (GibbonCairoboard *board, cairo_t *cr);
static void gibbon_cairoboard_set_info (GibbonCairoboard *board);
static void gibbon_cairoboard_draw_bar (GibbonCairoboard *board, cairo_t *cr,
                                        gint side);
static void gibbon_draw_home (GibbonCairoboard *board, cairo_t *cr,
                                  gint side);
static void gibbon_cairoboard_draw_point (GibbonCairoboard *board, cairo_t *cr,
                               guint point);
static void gibbon_cairoboard_draw_flat_checker (GibbonCairoboard *board, cairo_t *cr,
                                                 gdouble x, gdouble y,
                                                 gint side);
static void gibbon_cairoboard_draw_die (GibbonCairoboard *self, cairo_t *cr,
                                        guint value, guint die_pos);

static void gibbon_draw_cube (GibbonCairoboard *board, cairo_t *cr);
static void gibbon_draw_flag (GibbonCairoboard *board, cairo_t *cr);
static void gibbon_draw_cup (GibbonCairoboard *board, cairo_t *cr);
static void gibbon_draw_dice (GibbonCairoboard *board, cairo_t *cr);
static void gibbon_draw_animation (GibbonCairoboard *board, cairo_t *cr);

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
static gboolean gibbon_cairoboard_on_button_press (GibbonCairoboard *self,
                                                   GdkEventButton *event);
static gboolean gibbon_cairoboard_on_2button_press (GibbonCairoboard *self,
                                                    GdkEventButton *event);
static void gibbon_cairoboard_redraw (const GibbonBoard *self);

static guint gibbon_cairoboard_signals[LAST_SIGNAL] = { 0 };

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
        self->priv->target = NULL;
        self->priv->move = NULL;
        self->priv->animation_id = 0;
        self->priv->animation_move_number = 0;
        self->priv->animation_floating = GIBBON_POSITION_SIDE_NONE;

        self->priv->translate_x = self->priv->translate_y = 0;
        self->priv->scale = 1;

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
        self->priv->cube = NULL;
        self->priv->flag = NULL;
        self->priv->cup = NULL;

        return;
}

static void
gibbon_board_iface_init (GibbonBoardIface *iface)
{
        iface->set_position = gibbon_cairoboard_set_position;
        iface->get_position = gibbon_cairoboard_get_position;
        iface->animate_move = gibbon_cairoboard_animate_move;
        iface->fade_out_dice = gibbon_cairoboard_fade_out_dice;
        iface->redraw = gibbon_cairoboard_redraw;
}

static void
gibbon_cairoboard_finalize (GObject *object)
{
        GibbonCairoboard *self = GIBBON_CAIROBOARD (object);
        size_t i;

        gibbon_cairoboard_cancel_animation (self);

        g_signal_handlers_disconnect_by_func (self, 
                                              gibbon_cairoboard_on_button_press,
                                              NULL);

        if (self->priv->pos)
                gibbon_position_free (self->priv->pos);
        if (self->priv->target)
                gibbon_position_free (self->priv->target);
        if (self->priv->move)
                g_object_unref (self->priv->move);

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
        if (self->priv->flag)
                svg_util_free_component (self->priv->flag);
        if (self->priv->cup)
                svg_util_free_component (self->priv->cup);

        self->priv->app = NULL;

        G_OBJECT_CLASS (gibbon_cairoboard_parent_class)->finalize (object);
}

static void
gibbon_cairoboard_class_init (GibbonCairoboardClass *klass)
{
        GtkDrawingAreaClass *parent_class = GTK_DRAWING_AREA_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonCairoboardPrivate));

        /* Initialize libxml.  */
        LIBXML_TEST_VERSION
        
        G_OBJECT_CLASS (parent_class)->finalize = gibbon_cairoboard_finalize;
        GTK_WIDGET_CLASS (parent_class)->expose_event = 
                gibbon_cairoboard_expose;

        gibbon_cairoboard_signals[GIBBON_CAIROBOARD_DICE_PICKED_UP] =
                        g_signal_new ("dice-picked-up",
                                      G_TYPE_FROM_CLASS (klass),
                                      G_SIGNAL_RUN_FIRST,
                                      0, NULL, NULL,
                                      g_cclosure_marshal_VOID__VOID,
                                      G_TYPE_NONE, 0);

        gibbon_cairoboard_signals[GIBBON_CAIROBOARD_CUBE_TURNED] =
                        g_signal_new ("cube-turned",
                                      G_TYPE_FROM_CLASS (klass),
                                      G_SIGNAL_RUN_FIRST,
                                      0, NULL, NULL,
                                      g_cclosure_marshal_VOID__VOID,
                                      G_TYPE_NONE, 0);

        gibbon_cairoboard_signals[GIBBON_CAIROBOARD_CUBE_TAKEN] =
                        g_signal_new ("cube-taken",
                                      G_TYPE_FROM_CLASS (klass),
                                      G_SIGNAL_RUN_FIRST,
                                      0, NULL, NULL,
                                      g_cclosure_marshal_VOID__VOID,
                                      G_TYPE_NONE, 0);

        gibbon_cairoboard_signals[GIBBON_CAIROBOARD_CUBE_DROPPED] =
                        g_signal_new ("cube-dropped",
                                      G_TYPE_FROM_CLASS (klass),
                                      G_SIGNAL_RUN_FIRST,
                                      0, NULL, NULL,
                                      g_cclosure_marshal_VOID__VOID,
                                      G_TYPE_NONE, 0);

        gibbon_cairoboard_signals[GIBBON_CAIROBOARD_RESIGNATION_ACCEPTED] =
                        g_signal_new ("resignation-accepted",
                                      G_TYPE_FROM_CLASS (klass),
                                      G_SIGNAL_RUN_FIRST,
                                      0, NULL, NULL,
                                      g_cclosure_marshal_VOID__VOID,
                                      G_TYPE_NONE, 0);

        gibbon_cairoboard_signals[GIBBON_CAIROBOARD_RESIGNATION_REJECTED] =
                        g_signal_new ("resignation-rejected",
                                      G_TYPE_FROM_CLASS (klass),
                                      G_SIGNAL_RUN_FIRST,
                                      0, NULL, NULL,
                                      g_cclosure_marshal_VOID__VOID,
                                      G_TYPE_NONE, 0);

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
                gibbon_app_display_error (app, NULL,
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
                gibbon_app_display_error (app, NULL,
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

        self->priv->flag = gibbon_cairoboard_get_component (self, "flag",
                                                            TRUE, doc,
                                                            filename);
        if (!self->priv->flag) {
                xmlFree (doc);
                g_object_ref_sink (self);
                return NULL;
        }

        self->priv->cup = gibbon_cairoboard_get_component (self, "cup",
                                                           TRUE, doc,
                                                           filename);
        if (!self->priv->cup) {
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

        gtk_widget_add_events (GTK_WIDGET (self), 
                               GDK_BUTTON_PRESS_MASK);
        g_signal_connect (self, "button-press-event",
                          G_CALLBACK (gibbon_cairoboard_on_button_press),
                          NULL);

        return self;
}

static gboolean 
gibbon_cairoboard_expose (GtkWidget *widget, GdkEventExpose *event) 
{
        cairo_t *cr;
        
        g_return_val_if_fail (GIBBON_IS_CAIROBOARD (widget), FALSE);
        
        cr = gdk_cairo_create (gtk_widget_get_window (widget));
        
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
        GtkAllocation allocation;
        gdouble widget_ratio;
        gdouble translate_x, translate_y, scale;
        gdouble aspect_ratio;
        gdouble width;
        gdouble height;
        gint i;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));

        gibbon_cairoboard_set_info (self);

        width = self->priv->board->width;
        height = self->priv->board->height;

        aspect_ratio = (double) width / height;
        
        widget = GTK_WIDGET (self);
        gtk_widget_get_allocation (widget, &allocation);
        
        if (!allocation.height)
                return;
        if (!allocation.width)
                return;
        
        widget_ratio = (gdouble) allocation.width / allocation.height;

        if (widget_ratio > aspect_ratio) {
                scale = (gdouble) allocation.height / height; 
                translate_y = 0;
                translate_x = (allocation.width 
                               - scale * width) / 2;
        } else {
                scale = (gdouble) allocation.width / width;
                translate_x = 0;
                translate_y = (allocation.height 
                               - scale * height) / 2;
        }

        self->priv->translate_x = translate_x;
        self->priv->translate_y = translate_y;
        self->priv->scale = scale;

        cairo_translate (cr, translate_x, translate_y);
        cairo_scale (cr, scale, scale);

        svg_cairo_set_viewport_dimension (self->priv->board->scr,
                                          allocation.width,
                                          allocation.height);
        svg_cairo_render (self->priv->board->scr, cr);

        gibbon_draw_dice (self, cr);
        gibbon_draw_cube (self, cr);
        gibbon_draw_flag (self, cr);
        gibbon_draw_cup (self, cr);

        gibbon_cairoboard_draw_bar (self, cr, GIBBON_POSITION_SIDE_WHITE);
        gibbon_cairoboard_draw_bar (self, cr, GIBBON_POSITION_SIDE_BLACK);

        gibbon_draw_home (self, cr, GIBBON_POSITION_SIDE_WHITE);
        gibbon_draw_home (self, cr, GIBBON_POSITION_SIDE_BLACK);

        for (i = 0; i < 24; ++i)
                if (self->priv->pos->points[i])
                        gibbon_cairoboard_draw_point (self, cr, i);

        gibbon_draw_animation (self, cr);
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
        gint checkers = 0;
        gint i;
        gdouble x, y;
        struct svg_component *checker;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));
        g_return_if_fail (side);

        checkers = gibbon_position_get_borne_off (self->priv->pos, side);
        if (self->priv->animation_floating == side)
                --checkers;
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
                y -= side * checker->height;
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
        gdouble left, right;
        gdouble saved_size;
        gchar *cube_label;
        gdouble scale;
        gdouble top, bottom;
        gint cube_value;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));

        top = self->priv->checker_w_home->y;
        bottom = self->priv->checker_b_home->y
                 + self->priv->checker_b_home->height;

        if (self->priv->pos->cube_turned != GIBBON_POSITION_SIDE_NONE) {
                if (self->priv->pos->cube_turned
                    == GIBBON_POSITION_SIDE_BLACK) {
                        right = self->priv->point24->x
                                        + self->priv->point24->width;
                        left = right - 6 * self->priv->point24->width;
                } else {
                        left = self->priv->point12->x;
                        right = left + 6 * self->priv->point12->width;
                }
                x = 0.5 * (left + right);
                y = 0.5 * (top + bottom);
                cube_value = self->priv->pos->cube << 1;
        } else if (self->priv->pos->may_double[0]) {
                cube_value = self->priv->pos->cube;
                x = self->priv->cube->x + 0.5 * self->priv->cube->width;
                if (self->priv->pos->may_double[1]) {
                        y = 0.5 * (top + bottom);
                        cube_value = 1;
                } else {
                        y = self->priv->checker_w_home->y
                            - 0.5 * self->priv->checker_w_home->height;
                }
        } else if (self->priv->pos->may_double[1]) {
                x = self->priv->cube->x + 0.5 * self->priv->cube->width;
                y = self->priv->checker_b_home->y
                    + 0.5 * self->priv->cube->height;
                cube_value = self->priv->pos->cube;
        } else {
                return;
        }


        cube_label = g_strdup_printf ("%d", cube_value);
        if (strlen (cube_label) > 2)
                scale = 2.0 / (gdouble) strlen (cube_label);
        else
                scale = 1.0;
        g_return_if_fail (svg_util_steal_text_params (self->priv->cube,
                                                      "cube-value",
                                                      cube_label, scale, 0,
                                                      &saved_size));

        gibbon_cairoboard_draw_svg_component (self, cr, self->priv->cube, x, y);
        g_free (cube_label);

        g_return_if_fail (svg_util_steal_text_params (self->priv->cube,
                                                      "cube-value",
                                                      NULL, 0,
                                                      saved_size,
                                                      NULL));
}

static void
gibbon_draw_flag (GibbonCairoboard *self, cairo_t *cr)
{
        gdouble x, y;
        gdouble left, right;
        gdouble saved_size;
        gchar *flag_label;
        gdouble top, bottom;
        guint flag_value;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));

        top = self->priv->checker_w_home->y;
        bottom = self->priv->checker_b_home->y
                 + self->priv->checker_b_home->height;

        if (self->priv->pos->resigned < 0) {
                right = self->priv->point24->x
                                + self->priv->point24->width;
                left = right - 6 * self->priv->point24->width;
        } else if (self->priv->pos->resigned > 0) {
                left = self->priv->point12->x;
                right = left + 6 * self->priv->point12->width;
        } else {
                return;
        }

        x = 0.5 * (left + right);
        y = 0.5 * (top + bottom);

        flag_value = abs (self->priv->pos->resigned / self->priv->pos->cube);
        flag_label = g_strdup_printf ("%d", flag_value);
        g_return_if_fail (svg_util_steal_text_params (self->priv->flag,
                                                      "flag-value",
                                                      flag_label, 1.0, 0,
                                                      &saved_size));
        gibbon_cairoboard_draw_svg_component (self, cr, self->priv->flag, x, y);
        g_free (flag_label);

        g_return_if_fail (svg_util_steal_text_params (self->priv->flag,
                                                      "flag-value",
                                                      NULL, 0,
                                                      saved_size,
                                                      NULL));
}

static void
gibbon_draw_cup (GibbonCairoboard *self, cairo_t *cr)
{
        gdouble x, y;
        gdouble left, right;
        gdouble top, bottom;
        GibbonPosition *position;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));

        position = self->priv->pos;
        if (position->turn != GIBBON_POSITION_SIDE_WHITE)
                return;
        if (position->dice[0])
                return;
        if (position->dice[1])
                return;
        if (self->priv->pos->resigned)
                return;

        top = self->priv->checker_w_home->y;
        bottom = self->priv->checker_b_home->y
                 + self->priv->checker_b_home->height;
        right = self->priv->point24->x
                        + self->priv->point24->width;
        left = right - 6 * self->priv->point24->width;

        x = 0.5 * (left + right);
        y = 0.5 * (top + bottom);

        gibbon_cairoboard_draw_svg_component (self, cr, self->priv->cup, x, y);
}

static void
gibbon_draw_dice (GibbonCairoboard *self, cairo_t *cr)
{
        guint *dice;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));

        dice = self->priv->pos->dice;

        g_return_if_fail (dice[0] <= 6);
        g_return_if_fail (dice[1] <= 6);

        if (self->priv->pos->dice_swapped) {
                gibbon_cairoboard_draw_die (self, cr, dice[0], 1);
                gibbon_cairoboard_draw_die (self, cr, dice[1], 0);
        } else {
                gibbon_cairoboard_draw_die (self, cr, dice[0], 0);
                gibbon_cairoboard_draw_die (self, cr, dice[1], 1);
        }
}

static void
gibbon_cairoboard_draw_die (GibbonCairoboard *self, cairo_t *cr,
                            guint value, guint die_pos)
{
        gdouble x, y;
        struct svg_component *die;
        gdouble top, bottom;
        gdouble left, right;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));

        if (!value)
                return;

        g_return_if_fail (value <= 6);

        top = self->priv->point24->y;
        bottom = self->priv->point12->y + self->priv->point12->height;

        if (self->priv->pos->turn < 0) {
                die = self->priv->black_dice[value - 1];
                left = self->priv->point12->x;
                right = left + 6 * self->priv->point12->width;
        } else if (self->priv->pos->turn > 0) {
                die = self->priv->white_dice[value - 1];
                right = self->priv->point24->x + self->priv->point24->width;
                left = right - 6 * self->priv->point24->width;
        } else if (!die_pos) {
                die = self->priv->white_dice[value - 1];
                right = self->priv->point24->x + self->priv->point24->width;
                left = right - 6 * self->priv->point24->width;
        } else {
                die = self->priv->black_dice[value - 1];
                left = self->priv->point12->x;
                right = left + 6 * self->priv->point12->width;
        }

        x = 0.5 * (left + right) + (die_pos - 0.5) * 1.5 * die->width;
        y = 0.5 * (top + bottom);

        gibbon_cairoboard_draw_svg_component (self, cr, die, x, y);
}

static void
gibbon_draw_animation (GibbonCairoboard *self, cairo_t *cr)
{
        gdouble from_x, from_y;
        gdouble to_x, to_y;
        gdouble x, y;
        gint from, to;
        GibbonPositionSide side;
        guint num_checkers;
        struct svg_component *checker;
        GibbonPosition *pos;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (self));

        side = self->priv->animation_floating;
        if (!side)
                return;

        pos = self->priv->pos;

        from = self->priv->animation_from;
        to = self->priv->animation_to;

        if (from == 0) {
                if (side == GIBBON_POSITION_SIDE_WHITE) {
                        from_x = gibbon_cairoboard_get_home_x (self, side);

                        num_checkers =
                             gibbon_position_get_borne_off (self->priv->pos,
                                                            side);
                        checker = self->priv->checker_w_home;
                        from_y = checker->y +
                                 (num_checkers + 0.5) * checker->height;
                } else {
                        from_x = gibbon_cairoboard_get_bar_x (self);
                        from_y = gibbon_cairoboard_get_bar_y (self,
                                                              pos->bar[1] + 1,
                                                              pos->bar[1],
                                                              side);
                }
        } else if (from == 25) {
                if (side == GIBBON_POSITION_SIDE_BLACK) {
                        from_x = gibbon_cairoboard_get_home_x (self, side);
                        num_checkers =
                             gibbon_position_get_borne_off (self->priv->pos,
                                                            side);
                        checker = self->priv->checker_b_home;
                        from_y = checker->y +
                                 (num_checkers + 0.5) * checker->height;
                } else {
                        from_x = gibbon_cairoboard_get_bar_x (self);
                        from_y = gibbon_cairoboard_get_bar_y (self,
                                                              pos->bar[0] + 1,
                                                              pos->bar[0],
                                                              side);
                }
        } else {
                from_x = gibbon_cairoboard_get_flat_checker_x (self, from - 1);
                num_checkers = abs (self->priv->pos->points[from - 1]) - 1;
                from_y = gibbon_cairoboard_get_flat_checker_y (self, from - 1,
                                                               num_checkers
                                                               + 1);
        }

        if (to == 0) {
                if (side == GIBBON_POSITION_SIDE_WHITE) {
                        to_x = gibbon_cairoboard_get_home_x (self, side);
                        num_checkers =
                             gibbon_position_get_borne_off (self->priv->pos,
                                                            side);
                        checker = self->priv->checker_w_home;
                        to_y = checker->y + 0.5 * checker->height;
                } else {
                        to_x = gibbon_cairoboard_get_bar_x (self);
                        to_y = gibbon_cairoboard_get_bar_y (self,
                                                            pos->bar[1] + 1,
                                                            pos->bar[1],
                                                            side);
                }
        } else if (to == 25) {
                if (side == GIBBON_POSITION_SIDE_BLACK) {
                        to_x = gibbon_cairoboard_get_home_x (self, side);
                        num_checkers =
                             gibbon_position_get_borne_off (self->priv->pos,
                                                            side);
                        checker = self->priv->checker_w_home;
                        to_y = checker->y + 0.5 * checker->height;
                } else {
                        to_x = gibbon_cairoboard_get_bar_x (self);
                        to_y = gibbon_cairoboard_get_bar_y (self,
                                                            pos->bar[0] + 1,
                                                            pos->bar[0],
                                                            side);
                }
        } else {
                to_x = gibbon_cairoboard_get_flat_checker_x (self, to - 1);
                num_checkers = abs (self->priv->pos->points[to - 1]) - 1;
                to_y = gibbon_cairoboard_get_flat_checker_y (self, to - 1,
                                                             num_checkers
                                                             + 1);
        }

        x = from_x + self->priv->animation_promille * ((to_x - from_x) / 1000);
        y = from_y + self->priv->animation_promille * ((to_y - from_y) / 1000);

        gibbon_cairoboard_draw_flat_checker (self, cr, x, y, side);
}

static void
gibbon_cairoboard_set_position (GibbonBoard *_self,
                                const GibbonPosition *pos)
{
        GibbonCairoboard *self;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (_self));
        g_return_if_fail (pos != NULL);
        
        self = GIBBON_CAIROBOARD (_self);

        gibbon_cairoboard_cancel_animation (self);

        if (self->priv->pos)
                gibbon_position_free (self->priv->pos);
        self->priv->pos = gibbon_position_copy (pos);

        gtk_widget_queue_draw (GTK_WIDGET (self));
}

static const GibbonPosition *
gibbon_cairoboard_get_position (const GibbonBoard *_self)
{
        GibbonCairoboard *self;

        g_return_val_if_fail (GIBBON_IS_CAIROBOARD (_self), NULL);

        self = GIBBON_CAIROBOARD (_self);

        /* When animating towards a new position, fake the reply.  */
        if (self->priv->target)
                return self->priv->target;

        return self->priv->pos;
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
                gibbon_app_display_error (self->priv->app, NULL,
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
                return self->priv->checker_w_flat->y
                       + (0.5 - pos->pos) * self->priv->checker_w_flat->height;
        } else {
                return self->priv->checker_b_flat->y
                       + (0.5 + pos->pos) * self->priv->checker_b_flat->height;
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

static void
gibbon_cairoboard_set_info (GibbonCairoboard *self)
{
        gchar *text;
        guint num_players = 0;
        gboolean running;
        GibbonPosition *pos = self->priv->pos;
        guint64 away[2];
        guint pip_count[2];

        text = pos->players[0] ? pos->players[0] : "";
        svg_util_steal_text_params (self->priv->board, "player1", text,
                                    1.0, 0, NULL);
        if (text)
                ++num_players;

        text = pos->players[1] ? pos->players[1] : "";
        svg_util_steal_text_params (self->priv->board, "player2", text,
                                    1.0, 0, NULL);

        if (text)
                ++num_players;

        running = num_players == 2;

        text = running && pos->game_info ? pos->game_info : "";
        svg_util_steal_text_params (self->priv->board, "game_info", text,
                                    1.0, 0, NULL);
        text = pos->status ? pos->status : "";
        svg_util_steal_text_params (self->priv->board, "status-bar", text,
                                    1.0, 0, NULL);

        if (running) {
                if (pos->match_length) {
                        text = g_strdup_printf (_("%llu-point match"),
                                                  (unsigned long long)
                                                  pos->match_length);
                } else {
                        text = g_strdup (_("Unlimited match"));
                }
        } else {
                text = g_strdup ("");
        }
        svg_util_steal_text_params (self->priv->board, "match_length", text,
                                    1.0, 0, NULL);
        g_free (text);

        if (!running) {
                svg_util_steal_text_params (self->priv->board, "score0", "",
                                            1.0, 0, NULL);
                svg_util_steal_text_params (self->priv->board, "score1", "",
                                            1.0, 0, NULL);
        } else if (pos->match_length
            && pos->scores[0] < pos->match_length
            && pos->scores[1] < pos->match_length) {
                away[0] = pos->match_length - pos->scores[0];
                text = g_strdup_printf (_("Score: %llu (%llu-away)"),
                                        (unsigned long long) pos->scores[0],
                                        (unsigned long long) away[0]);
                svg_util_steal_text_params (self->priv->board, "score1", text,
                                            1.0, 0, NULL);
                g_free (text);
                away[1] = pos->match_length - pos->scores[1];
                text = g_strdup_printf (_("Score: %llu (%llu-away)"),
                                        (unsigned long long) pos->scores[1],
                                        (unsigned long long) away[1]);
                svg_util_steal_text_params (self->priv->board, "score2", text,
                                            1.0, 0, NULL);
                g_free (text);
        } else {
                text = g_strdup_printf (_("Score: %llu"),
                                        (unsigned long long) pos->scores[0]);
                svg_util_steal_text_params (self->priv->board, "score1", text,
                                            1.0, 0, NULL);
                g_free (text);
                text = g_strdup_printf (_("Score: %llu"),
                                        (unsigned long long) pos->scores[1]);
                svg_util_steal_text_params (self->priv->board, "score2", text,
                                            1.0, 0, NULL);
                g_free (text);
        }

        pip_count[0] = gibbon_position_get_pip_count (pos,
                                                    GIBBON_POSITION_SIDE_WHITE);
        pip_count[1] = gibbon_position_get_pip_count (pos,
                                                    GIBBON_POSITION_SIDE_BLACK);
        text = g_strdup_printf (_("Pips: %u (%+d)"),
                                  pip_count[0], pip_count[0] - pip_count[1]);
        svg_util_steal_text_params (self->priv->board, "pip1", text,
                                    1.0, 0, NULL);
        g_free (text);
        text = g_strdup_printf (_("Pips: %u (%+d)"),
                                  pip_count[1], pip_count[1] - pip_count[0]);
        svg_util_steal_text_params (self->priv->board, "pip2", text,
                                    1.0, 0, NULL);
        g_free (text);
}

static void
gibbon_cairoboard_animate_move (GibbonBoard *_self, const GibbonMove *move,
                                GibbonPositionSide side,
                                GibbonPosition *target_position)
{
        GibbonCairoboard *self;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (_self));
        g_return_if_fail (target_position != NULL);

        self = GIBBON_CAIROBOARD (_self);

        gibbon_cairoboard_cancel_animation (self);

        self->priv->target = target_position;

        self->priv->move = gibbon_move_copy (move);

        self->priv->animation_move_number = 0;
        self->priv->animation_side = side;
        self->priv->animation_step = 0;

        self->priv->animation_id =
                g_timeout_add (ANIMATION_TIMEOUT,
                               (GSourceFunc) gibbon_cairoboard_do_animation,
                               (gpointer) self);
}

static void
gibbon_cairoboard_fade_out_dice (GibbonBoard *_self)
{
        GibbonCairoboard *self;
        GibbonPosition *target_position;

        g_return_if_fail (GIBBON_IS_CAIROBOARD (_self));

        self = GIBBON_CAIROBOARD (_self);

        gibbon_cairoboard_cancel_animation (self);

        target_position = gibbon_position_copy (self->priv->pos);
        target_position->dice[0] = 0;
        target_position->dice[1] = 0;
        self->priv->target = target_position;

        self->priv->animation_id =
                g_timeout_add (DICE_FADE_OUT_TIMEOUT,
                               (GSourceFunc) gibbon_cairoboard_hide_dice,
                                (gpointer) self);
}

static void
gibbon_cairoboard_cancel_animation (GibbonCairoboard *self)
{
        if (!self->priv->target)
                return;

        gibbon_position_free (self->priv->pos);
        self->priv->pos = self->priv->target;
        self->priv->target = NULL;

        if (self->priv->move)
                g_object_unref (self->priv->move);
        self->priv->move = NULL;

        if (self->priv->animation_id)
                g_source_remove (self->priv->animation_id);
        self->priv->animation_id = 0;

        self->priv->animation_floating = GIBBON_POSITION_SIDE_NONE;

        gtk_widget_queue_draw (GTK_WIDGET (self));
}

static gboolean
gibbon_cairoboard_do_animation (GibbonCairoboard *self)
{
        gint move_number = self->priv->animation_move_number;
        gint from, to;
        GibbonMove *move = self->priv->move;
        GibbonPosition *pos = self->priv->pos;
        GibbonPositionSide side = self->priv->animation_side;

#define stop_animation(s) { \
        self->priv->animation_id = 0; \
        gibbon_cairoboard_cancel_animation (s); \
        return FALSE; \
}

        if (!move || move_number >= move->number || !side)
                stop_animation (self);

        from = move->movements[move_number].from;
        to = move->movements[move_number].to;

        if (self->priv->animation_step == 0) {
                /* Checker starts floating.  */
                self->priv->animation_step = 1;
                if (side == GIBBON_POSITION_SIDE_WHITE) {
                        if (from == 25) {
                                if (!pos->bar[0]--)
                                        stop_animation (self);
                        } else {
                                if (!pos->points[from - 1]--)
                                        stop_animation (self);
                        }
                } else {
                        if (from == 0) {
                                if (!pos->bar[1]--)
                                        stop_animation (self);
                        } else {
                                /*
                                 * Yes, increment, not decrement! We are moving
                                 * black's checkers here and they have negative
                                 * counts!
                                 */
                                if (!pos->points[from - 1]++)
                                        stop_animation (self);
                        }
                }
                self->priv->animation_floating = side;
                self->priv->animation_from = from;
                self->priv->animation_to = to;
                self->priv->animation_promille = 0;
        }

        if (self->priv->animation_step == 1) {
                /* Checker is floating.  */
                self->priv->animation_promille += ANIMATION_STEP_WIDTH;
                if (self->priv->animation_promille > 1000) {
                        self->priv->animation_promille = 0;
                        self->priv->animation_floating =
                            GIBBON_POSITION_SIDE_NONE;
                        self->priv->animation_step = 2;
                }
        }

        if (self->priv->animation_step == 2) {
                /* Checker landed.  */
                self->priv->animation_promille = 0;
                if (side == GIBBON_POSITION_SIDE_WHITE) {
                        if (pos->points[to - 1] == -1) {
                                /* Blot hit.  */
                                pos->points[to - 1] = 1;
                                self->priv->animation_step = 3;
                                self->priv->animation_floating = -side;
                                self->priv->animation_from = to;
                                self->priv->animation_to = 0;
                        } else {
                                 if (to != 0)
                                        ++pos->points[to - 1];
                                 self->priv->animation_step = 4;
                                 self->priv->animation_floating =
                                         GIBBON_POSITION_SIDE_NONE;
                        }
                } else {
                        if (pos->points[to - 1] == +1) {
                                /* Blot hit.  */
                                pos->points[to - 1] = -1;
                                self->priv->animation_step = 3;
                                self->priv->animation_floating = -side;
                                self->priv->animation_from = to;
                                self->priv->animation_to = 25;
                        } else {
                                if (to != 25)
                                        --pos->points[to - 1];
                                self->priv->animation_step = 4;
                                self->priv->animation_floating =
                                         GIBBON_POSITION_SIDE_NONE;
                        }
                }
        }

        if (self->priv->animation_step == 3) {
                /* Hit checker is travelling to bar.  */
                self->priv->animation_promille += ANIMATION_STEP_WIDTH;
                if (self->priv->animation_promille > 1000) {
                        if (side == GIBBON_POSITION_SIDE_WHITE)
                                ++pos->bar[1];
                        else
                                ++pos->bar[0];
                        self->priv->animation_promille = 0;
                        self->priv->animation_floating =
                            GIBBON_POSITION_SIDE_NONE;
                        self->priv->animation_step = 4;
                }
        }

        if (self->priv->animation_step > 3) {
                ++self->priv->animation_move_number;
                self->priv->animation_step = 0;
                self->priv->animation_promille = 0;
                self->priv->animation_floating = GIBBON_POSITION_SIDE_NONE;
                if (self->priv->animation_move_number >= move->number) {
                        stop_animation (self);
                } else {
                        return gibbon_cairoboard_do_animation (self);
                }
        }

        gtk_widget_queue_draw (GTK_WIDGET (self));

        return TRUE;
}

static gboolean
gibbon_cairoboard_hide_dice (GibbonCairoboard *self)
{
        self->priv->animation_id = 0;
        gibbon_cairoboard_cancel_animation (self);

        return FALSE;
}

static gboolean 
gibbon_cairoboard_on_button_press (GibbonCairoboard *self,
                                   GdkEventButton *event)
{
        gdouble x, y;
        gdouble cx, cy, left, right, top, bottom;
        guint column;
        guint point;
        guint signo;
        
        if (event->type == GDK_2BUTTON_PRESS)
                return gibbon_cairoboard_on_2button_press (self, event);

        if (event->button != 1 && event->button != 3)
                return FALSE;

        x = event->x - self->priv->translate_x;
        y = event->y - self->priv->translate_y;
        if (self->priv->scale) {
                x /= self->priv->scale;
                y /= self->priv->scale;
        }

        /*
         * These are the least probable cases but it is better to test them
         * first because the borne-off checkers are more or less outside
         * of the board.
         */
        if (x >= self->priv->checker_b_home->x
            && x <= self->priv->checker_b_home->x
                    + self->priv->checker_b_home->width
            && y >= self->priv->checker_b_home->y
            && y <= self->priv->checker_b_home->y
                    + 15 * self->priv->checker_b_home->height) {
                /*
                 * Click in black bear-off tray.  Ignore.
                 */
                return TRUE;
        }

        if (x >= self->priv->checker_w_home->x
            && x <= self->priv->checker_w_home->x
                    + self->priv->checker_w_home->width
            && y <= self->priv->checker_w_home->y
            && y >= self->priv->checker_w_home->y
                    - 15 * self->priv->checker_w_home->height) {
                gibbon_board_process_quick_bear_off (GIBBON_BOARD (self));
                return TRUE;
        }

        /*
         * First we test whether there is a remote possibility of a click in
         * the active area of the board.
         */
        if (x < self->priv->point12->x
            || x > self->priv->point24->x
                   + self->priv->point24->width
            || y < self->priv->point24->y
            || y > self->priv->point12->y
                   + self->priv->point12->height) {
                return FALSE;
        }

        if (self->priv->pos->cube_turned) {
                if (GIBBON_POSITION_SIDE_WHITE == self->priv->pos->cube_turned)
                        return FALSE;
                right = self->priv->point24->x + self->priv->point24->width;
                left = right - 6 * self->priv->point24->width;
                cx = 0.5 * (left + right);
                top = self->priv->checker_w_home->y;
                bottom = self->priv->checker_b_home->y
                         + self->priv->checker_b_home->height;
                cy = 0.5 * (top + bottom);
                if (x >= cx - self->priv->cube->width / 2 - 3
                    && x <= cx + self->priv->cube->width / 2 + 3
                    && y >= cy - self->priv->cube->height / 2 - 3
                    && y <= cy + self->priv->cube->height / 2 + 3) {
                        if (event->button == 1)
                                signo = GIBBON_CAIROBOARD_CUBE_TAKEN;
                        else
                                signo = GIBBON_CAIROBOARD_CUBE_DROPPED;
                        g_signal_emit (self, gibbon_cairoboard_signals[signo],
                                       0, self);
                        return TRUE;
                }
                return FALSE;
        } else if (self->priv->pos->resigned < 0) {
                right = self->priv->point24->x
                                + self->priv->point24->width;
                left = right - 6 * self->priv->point24->width;
                cx = 0.5 * (left + right);
                top = self->priv->checker_w_home->y;
                bottom = self->priv->checker_b_home->y
                         + self->priv->checker_b_home->height;
                cy = 0.5 * (top + bottom);
                if (x >= cx - self->priv->flag->width / 2 - 3
                    && x <= cx + self->priv->flag->width / 2 + 3
                    && y >= cy - self->priv->flag->height / 2 - 3
                    && y <= cy + self->priv->flag->height / 2 + 3) {
                        if (event->button == 1)
                                signo = GIBBON_CAIROBOARD_RESIGNATION_ACCEPTED;
                        else
                                signo = GIBBON_CAIROBOARD_RESIGNATION_REJECTED;
                        g_signal_emit (self, gibbon_cairoboard_signals[signo],
                                       0, self);
                        return TRUE;
                }
                return FALSE;
        } else if (x <= self->priv->point12->x
                 + 6 * self->priv->checker_w_flat->width
                 && y >= self->priv->point24->y
                 && y <= self->priv->point12->y + self->priv->point12->height) {
                column = (x - self->priv->point12->x)
                                / self->priv->checker_w_flat->width;
                if (y <= self->priv->point24->y + self->priv->point12->height)
                        point = 13 + column;
                else if (y >= self->priv->point12->y)
                        point = 12 - column;
                else
                        return FALSE;

                gibbon_board_process_point_click (GIBBON_BOARD (self),
                                                  point, event->button);
        } else if (x <= self->priv->point24->x
                        - 5 * self->priv->checker_w_flat->width) {
                gibbon_board_process_bar_click (GIBBON_BOARD (self),
                                                event->button);
                return TRUE;
        } else if (x <= self->priv->point24->x
                        + self->priv->point24->width
                        && y >= self->priv->point24->y
                        && y <= self->priv->point12->y
                                + self->priv->point12->height) {
                column = (self->priv->point24->x
                          + self->priv->point24->width - x)
                          / self->priv->checker_w_flat->width;
                if (y <= self->priv->point24->y + self->priv->point12->height)
                        point = 24 - column;
                else if (y >= self->priv->point12->y)
                        point = 1 + column;
                else if (y >= self->priv->point24->y
                              + self->priv->point24->height
                              + 10
                         && y <= self->priv->point12->y + 15) {
                        /*
                         * We give an extra offset of 15 pixels here because
                         * the click is `dangerous', and it shouldn't be too
                         * close to a click on a point.
                         *
                         * There is no danger of confusing picking up of the
                         * dice with a click on the bar.  If the user
                         * mistakenly picked up the dice, when he meant a
                         * click on the bar, the legality checker will undo
                         * the error for us.
                         */
                        signo = GIBBON_CAIROBOARD_DICE_PICKED_UP;
                        g_signal_emit (self, gibbon_cairoboard_signals[signo],
                                       0, self);
                        return TRUE;
                } else {
                        return FALSE;
                }
                gibbon_board_process_point_click (GIBBON_BOARD (self),
                                                  point, event->button);
        }

        return TRUE;
}


static gboolean
gibbon_cairoboard_on_2button_press (GibbonCairoboard *self,
                                    GdkEventButton *event)
{
        gdouble x, y;
        struct svg_component *cube;
        guint signo;

        if (event->button != 1)
                return FALSE;

        x = event->x - self->priv->translate_x;
        y = event->y - self->priv->translate_y;
        if (self->priv->scale) {
                x /= self->priv->scale;
                y /= self->priv->scale;
        }

        cube = self->priv->cube;
        if (x >= cube->x && x <= cube->x + cube->width) {
                /* Centered cube? */
                if (y >= cube->y && y <= cube->y + cube->height) {
                        signo = GIBBON_CAIROBOARD_CUBE_TURNED;
                        g_signal_emit (self, gibbon_cairoboard_signals[signo],
                                       0, self);
                        return TRUE;
                }

                /*
                 * We ignore double-clicks on black's cube since we are
                 * always white.
                 */

                if (y <= self->priv->checker_w_home->y
                         + self->priv->checker_w_home->height
                    && y >= self->priv->checker_w_home->y
                            + self->priv->checker_w_home->height
                            - cube->height) {
                        signo = GIBBON_CAIROBOARD_CUBE_TURNED;
                        g_signal_emit (self, gibbon_cairoboard_signals[signo],
                                       0, self);
                        return TRUE;
                }
        }

        return FALSE;
}

static void
gibbon_cairoboard_redraw (const GibbonBoard *self)
{
        gtk_widget_queue_draw (GTK_WIDGET (self));
}
