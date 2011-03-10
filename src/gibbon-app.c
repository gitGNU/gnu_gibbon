/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
 *
 * gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:gibbon-app
 * @short_description: The Gibbon Application!
 *
 * Since: 0.1.1
 *
 * Class representing the running Gibbon application!
 **/

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <stdlib.h>

#include "gibbon-app.h"
#include "gibbon-cairoboard.h"

typedef struct _GibbonAppPrivate GibbonAppPrivate;
struct _GibbonAppPrivate {
        GtkBuilder *builder;
        GtkWidget *window;
        GtkWidget *statusbar;
        GtkWidget *server_text_view;
        GibbonCairoboard *board;
};

#define GIBBON_APP_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_APP, GibbonAppPrivate))

G_DEFINE_TYPE (GibbonApp, gibbon_app, G_TYPE_OBJECT)

static GtkBuilder *gibbon_app_get_builder (GibbonApp *self, const gchar *path);
static GibbonCairoboard *gibbon_app_init_board (GibbonApp *self,
                                                const gchar *board_filename);
static void gibbon_app_connect_signals (const GibbonApp *self);

/* Signal handlers.  */
static void gibbon_app_on_quit_request (GibbonApp *self,
                                        GtkWidget *emitter);

GibbonApp *singleton = NULL;
static struct GibbonPosition initial_position;

static void 
gibbon_app_init (GibbonApp *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_APP, GibbonAppPrivate);

        self->priv->builder = NULL;
        self->priv->board = NULL;
        self->priv->window = NULL;
        self->priv->statusbar = NULL;
        self->priv->server_text_view = NULL;
}

static void
gibbon_app_finalize (GObject *object)
{
        GibbonApp *self = GIBBON_APP (object);

        if (self->priv->builder)
                g_object_unref (self->priv->builder);
        self->priv->builder = NULL;

        if (self->priv->board)
                g_object_unref (self->priv->board);
        self->priv->board = NULL;

        G_OBJECT_CLASS (gibbon_app_parent_class)->finalize(object);
}

static void
gibbon_app_class_init (GibbonAppClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonAppPrivate));

        memset (&initial_position, 0, sizeof initial_position);
        initial_position.checkers[0] = -2;
        initial_position.checkers[5] = 5;
        initial_position.checkers[7] = 3;
        initial_position.checkers[11] = -5;
        initial_position.checkers[12] = 5;
        initial_position.checkers[16] = -3;
        initial_position.checkers[18] = -5;
        initial_position.checkers[23] = 2;
        initial_position.match_length = 23;
        initial_position.score[0] = 5;
        initial_position.score[1] = 7;
        initial_position.dice[0][0] = 0;
        initial_position.dice[0][1] = 0;
        initial_position.dice[1][0] = 0;
        initial_position.dice[1][1] = 0;
        initial_position.bar[0] = 0;
        initial_position.bar[1] = 0;
        initial_position.home[0] = 0;
        initial_position.home[1] = 0;
        initial_position.cube = 1;
        initial_position.may_double[0] = 1;
        initial_position.may_double[1] = 1;

        object_class->finalize = gibbon_app_finalize;
}

/**
 * gibbon_app_new:
 * @builder_path: Complete path to the builder XML file.
 * @pixmaps_dir: Path to the pixmaps.
 *
 * Creates a new #GibbonApp.
 *
 * Returns: The newly created #GibbonApp or %NULL in case of failure.
 */
GibbonApp *
gibbon_app_new (const gchar *builder_path, const gchar *pixmaps_dir)
{
        GibbonApp *self = g_object_new (GIBBON_TYPE_APP, NULL);
        PangoFontDescription *font_desc;
        gchar *board_filename;

        g_return_val_if_fail (singleton == NULL, singleton);

        self->priv->builder = gibbon_app_get_builder (self, builder_path);
        if (!self->priv->builder) {
                g_object_unref (self);
                return NULL;
        }

        self->priv->window =
                GTK_WIDGET (gibbon_app_find_object (self, "window",
                                                    GTK_TYPE_WINDOW));
        self->priv->statusbar =
                GTK_WIDGET (gibbon_app_find_object (self,
                                                    "statusbar",
                                                    GTK_TYPE_STATUSBAR));
        self->priv->server_text_view =
                GTK_WIDGET (gibbon_app_find_object (self,
                                                    "server_text_view",
                                                    GTK_TYPE_TEXT_VIEW));
        if (!(self->priv->window
              && self->priv->statusbar
              && self->priv->server_text_view)) {
                g_object_unref (self);
                return NULL;
        }

        board_filename = g_build_filename (pixmaps_dir, "boards",
                                           "default.svg", NULL);
        self->priv->board = gibbon_app_init_board (self, board_filename);
        g_free (board_filename);
        if (!self->priv->board) {
                g_object_unref (self);
                return NULL;
        }

        gtk_statusbar_push (GTK_STATUSBAR (self->priv->statusbar),
                            0, _("Disconnected"));

        font_desc = pango_font_description_from_string ("monospace 10");
        gtk_widget_modify_font (self->priv->server_text_view,
                                font_desc);
        pango_font_description_free (font_desc);

        gibbon_app_connect_signals (self);

        singleton = self;

        return self;
}

static GtkBuilder *
gibbon_app_get_builder (GibbonApp *self, const gchar *path)
{
        GtkBuilder *builder = gtk_builder_new ();
        GError *error = NULL;

        if (!gtk_builder_add_from_file (builder, path, &error)) {
                gibbon_app_display_error (self, "%s.\n%s",
                                          error->message,
                                          _("Do you need to pass the"
                                            " option `--data-dir'?\n"));
                g_object_unref (G_OBJECT (builder));
                return NULL;
        }

        return builder;
}

void
gibbon_app_display_error (const GibbonApp* self,
                          const gchar *message_format, ...)
{
        va_list args;
        gchar *message;

        va_start (args, message_format);
        message = g_strdup_vprintf (message_format, args);
        va_end (args);

        GtkWidget *dialog =
                gtk_message_dialog_new (GTK_WINDOW (self->priv->window),
                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                        GTK_MESSAGE_ERROR,
                                        GTK_BUTTONS_CLOSE,
                                        "%s", message);

        g_free (message);

        gtk_dialog_run (GTK_DIALOG (dialog));

        gtk_widget_destroy (GTK_WIDGET (dialog));
}

void
gibbon_app_display_info (const GibbonApp *self, const gchar *message_format, ...)
{
        va_list args;
        gchar *message;

        va_start (args, message_format);
        message = g_strdup_vprintf (message_format, args);
        va_end (args);

        GtkWidget *dialog =
                gtk_message_dialog_new (GTK_WINDOW (self->priv->window),
                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                        GTK_MESSAGE_INFO,
                                        GTK_BUTTONS_CLOSE,
                                        "%s", message);

        g_free (message);

        gtk_dialog_run (GTK_DIALOG (dialog));

        gtk_widget_destroy (GTK_WIDGET (dialog));
}

GObject *
gibbon_app_find_object (const GibbonApp *self, const gchar *id, GType type)
{
        GObject *obj;
        GType got_type;

        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);
        g_return_val_if_fail (G_TYPE_IS_OBJECT (type), NULL);

        obj = gtk_builder_get_object (self->priv->builder, id);

        if (!obj) {
                /* TRANSLATORS: UI means user interface.  */
                gibbon_app_display_error (self,
                                          _("Object `%s' not found in UI"
                                            " definition!"), id);
                exit (-1);
        }

        if (!G_IS_OBJECT (obj)) {
                gibbon_app_display_error (self,
                                          _("Object `%s' is not a GObject!"),
                                          id);
                exit (-1);
        }

        got_type = G_OBJECT_TYPE (obj);
        if (type != got_type) {
                gibbon_app_display_error (self,
                                          _("Object `%s' is not of type `%s'"
                                            " but `%s'!"),
                                           id, g_type_name (type),
                                           g_type_name (got_type));
                exit (-1);
        }

        return obj;
}

static GibbonCairoboard *
gibbon_app_init_board (GibbonApp *self, const gchar *board_filename)
{
        GObject *left_vpane;
        GibbonCairoboard *board = gibbon_cairoboard_new (board_filename);

        if (!board)
                return NULL;

        gibbon_cairoboard_set_position (board, &initial_position);

        gtk_widget_show (GTK_WIDGET (board));
        /* FIXME! This should occupy reasonable space by default!  Do
         * not hardcode the values.
         */
        gtk_widget_set_size_request (GTK_WIDGET (board), 490, 380);

        left_vpane = gibbon_app_find_object (self, "left_vpane",
                                             GTK_TYPE_VPANED);

        gtk_widget_destroy (gtk_paned_get_child1 (GTK_PANED (left_vpane)));
        gtk_paned_pack1 (GTK_PANED (left_vpane), GTK_WIDGET (board),
                         TRUE, FALSE);

        return board;
}

GtkWidget *
gibbon_app_get_window (const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->window;
}

static void
gibbon_app_connect_signals (const GibbonApp *self)
{
        GObject* obj;

        obj = gibbon_app_find_object (self, "toolbar_quit_button",
                                      GTK_TYPE_TOOL_BUTTON);
        if (!obj)
                return;
        g_signal_connect_swapped (obj, "clicked",
                                  G_CALLBACK (gibbon_app_on_quit_request),
                                  self);

        obj = gibbon_app_find_object (self, "quit_menu_item",
                                      GTK_TYPE_IMAGE_MENU_ITEM);
        if (!obj)
                return;
        g_signal_connect_swapped (obj, "activate",
                                  G_CALLBACK (gibbon_app_on_quit_request),
                                  self);

        g_signal_connect_swapped (obj, "destroy",
                                  G_CALLBACK (gibbon_app_on_quit_request),
                                  self);
}

static void
gibbon_app_on_quit_request (GibbonApp *self, GtkWidget *emitter)
{
        gtk_main_quit ();
}
