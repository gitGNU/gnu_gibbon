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
 * SECTION:gibbon-server-console
 * @short_description: Class representing server output area!
 *
 * Since: 0.1.1
 *
 * Show server communication.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include <time.h>

#include "gibbon-prefs.h"
#include "gibbon-server-console.h"

typedef struct _GibbonServerConsolePrivate GibbonServerConsolePrivate;
struct _GibbonServerConsolePrivate {
        GibbonApp *app;
        GtkTextView *text_view;
        GtkTextBuffer *buffer;

        GtkTextTag *raw_tag;
        GtkTextTag *debug_tag;
};

#define GIBBON_SERVER_CONSOLE_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_SERVER_CONSOLE, GibbonServerConsolePrivate))

G_DEFINE_TYPE (GibbonServerConsole, gibbon_server_console, G_TYPE_OBJECT)

static void _gibbon_server_console_print_raw (GibbonServerConsole *self,
                                              const gchar *string,
                                              GtkTextTag *tag);

static void 
gibbon_server_console_init (GibbonServerConsole *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_SERVER_CONSOLE, GibbonServerConsolePrivate);

        self->priv->app = NULL;
        self->priv->text_view = NULL;
        self->priv->buffer = NULL;

        self->priv->raw_tag = NULL;
        self->priv->debug_tag = NULL;
}

static void
gibbon_server_console_finalize (GObject *object)
{
        GibbonServerConsole *self = GIBBON_SERVER_CONSOLE (object);

        self->priv->app = NULL;
        self->priv->text_view = NULL;
        self->priv->buffer = NULL;

        if (self->priv->raw_tag)
                g_object_unref (self->priv->raw_tag);
        self->priv->raw_tag = NULL;

        if (self->priv->debug_tag)
                g_object_unref (self->priv->debug_tag);
        self->priv->debug_tag = NULL;

        G_OBJECT_CLASS (gibbon_server_console_parent_class)->finalize(object);
}

static void
gibbon_server_console_class_init (GibbonServerConsoleClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonServerConsolePrivate));

        object_class->finalize = gibbon_server_console_finalize;
}

/**
 * gibbon_server_console_new:
 * @app: The #GibbonApp.
 *
 * Creates a new #GibbonServerConsole.
 *
 * Returns: The newly created #GibbonServerConsole or %NULL in case of failure.
 */
GibbonServerConsole *
gibbon_server_console_new (GibbonApp *app)
{
        GibbonServerConsole *self = g_object_new (GIBBON_TYPE_SERVER_CONSOLE,
                                                  NULL);
        PangoFontDescription *font_desc;

        self->priv->app = app;
        self->priv->text_view =
                GTK_TEXT_VIEW (gibbon_app_find_object (app,
                                                       "server_text_view",
                                                       GTK_TYPE_TEXT_VIEW));
        gtk_text_view_set_wrap_mode (self->priv->text_view,
                                     GTK_WRAP_NONE);
        self->priv->buffer = gtk_text_view_get_buffer (self->priv->text_view);

        self->priv->raw_tag =
                gtk_text_buffer_create_tag (self->priv->buffer, NULL,
                                            "foreground", "black",
                                            NULL);
        self->priv->debug_tag =
                gtk_text_buffer_create_tag (self->priv->buffer, NULL,
                                            "style", PANGO_STYLE_ITALIC,
                                            "foreground", "grey",
                                            NULL);

        font_desc = pango_font_description_from_string ("monospace 10");
        gtk_widget_modify_font (GTK_WIDGET (self->priv->text_view),
                                font_desc);
        pango_font_description_free (font_desc);

        gtk_text_view_set_cursor_visible (self->priv->text_view, FALSE);

        return self;
}

static void
_gibbon_server_console_print_raw (GibbonServerConsole *self,
                                  const gchar *string,
                                  GtkTextTag *tag)
{
        GtkTextBuffer *buffer = self->priv->buffer;
        gint length;
        GtkTextIter start, end;
        GibbonPrefs *prefs = prefs;
        struct tm *now;
        GTimeVal timeval;
        gchar *timestamp;

        length = gtk_text_buffer_get_char_count (buffer);

        prefs = gibbon_app_get_prefs (self->priv->app);
        if (1 || gibbon_prefs_get_boolean (prefs, GIBBON_PREFS_DEBUG_TIMESTAMPS)) {
                g_get_current_time (&timeval);
                now = gmtime ((time_t *) &timeval.tv_sec);
                timestamp = g_strdup_printf ("[%02d:%02d.%07ld] ",
                                             now->tm_hour,
                                             now->tm_min,
                                             timeval.tv_usec);
                gtk_text_buffer_insert_at_cursor (buffer, timestamp, -1);
                g_free (timestamp);
        }

        gtk_text_buffer_insert_at_cursor (buffer, string, -1);
        gtk_text_buffer_insert_at_cursor (buffer, "\n", -1);
        gtk_text_buffer_get_iter_at_offset (buffer, &start, length);
        gtk_text_buffer_get_end_iter (buffer, &end);
        gtk_text_buffer_apply_tag (buffer, tag, &start, &end);

        gtk_text_view_scroll_to_mark (self->priv->text_view,
                gtk_text_buffer_get_insert (buffer),
                0.0, TRUE, 0.5, 1);
}

void
gibbon_server_console_print_raw (GibbonServerConsole *self,
                                 const gchar *string)
{
        _gibbon_server_console_print_raw (self, string, self->priv->raw_tag);
}

void
gibbon_server_console_print_debug (GibbonServerConsole *self,
                                 const gchar *string)
{
        _gibbon_server_console_print_raw (self, string, self->priv->debug_tag);
}
