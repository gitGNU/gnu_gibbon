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
 * SECTION:gibbon-shouts
 * @short_description: Abstraction for the shout area.
 *
 * Since: 0.1.1
 *
 * Handling of FIBS shouts.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-shouts.h"

typedef struct _GibbonShoutsPrivate GibbonShoutsPrivate;
struct _GibbonShoutsPrivate {
        GibbonApp *app;
        GtkTextView *text_view;
        GtkTextBuffer *buffer;

        GtkTextTag *sender_tag;
        GtkTextTag *date_tag;
};

#define GIBBON_SHOUTS_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_SHOUTS, GibbonShoutsPrivate))

G_DEFINE_TYPE (GibbonShouts, gibbon_shouts, G_TYPE_OBJECT)

static void 
gibbon_shouts_init (GibbonShouts *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_SHOUTS, GibbonShoutsPrivate);

        self->priv->app = NULL;
        self->priv->text_view = NULL;
        self->priv->buffer = NULL;

        self->priv->sender_tag = NULL;
        self->priv->date_tag = NULL;
}

static void
gibbon_shouts_finalize (GObject *object)
{
        GibbonShouts *self = GIBBON_SHOUTS (object);

        self->priv->app = NULL;
        self->priv->text_view = NULL;
        self->priv->buffer = NULL;

        if (self->priv->sender_tag)
                g_object_unref (self->priv->sender_tag);
        self->priv->sender_tag = NULL;

        if (self->priv->date_tag)
                g_object_unref (self->priv->date_tag);
        self->priv->date_tag = NULL;

        G_OBJECT_CLASS (gibbon_shouts_parent_class)->finalize(object);
}

static void
gibbon_shouts_class_init (GibbonShoutsClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonShoutsPrivate));

        object_class->finalize = gibbon_shouts_finalize;
}

/**
 * gibbon_shouts_new:
 * @app: The #GibbonApp.
 *
 * Creates a new #GibbonShouts.
 *
 * Returns: The newly created #GibbonShouts or %NULL in case of failure.
 */
GibbonShouts *
gibbon_shouts_new (GibbonApp *app)
{
        GibbonShouts *self = g_object_new (GIBBON_TYPE_SHOUTS, NULL);

        self->priv->app = NULL;

        self->priv->text_view =
                GTK_TEXT_VIEW (gibbon_app_find_object (app,
                                                       "shout-text-view",
                                                       GTK_TYPE_TEXT_VIEW));
        gtk_text_view_set_wrap_mode (self->priv->text_view,
                                     GTK_WRAP_WORD);
        gtk_text_view_set_cursor_visible (self->priv->text_view, FALSE);

        self->priv->buffer = gtk_text_view_get_buffer (self->priv->text_view);

        /* Pidgin uses #cc00000 and #204a87 as the default colors.  */
        self->priv->date_tag =
                gtk_text_buffer_create_tag (self->priv->buffer, NULL,
                                            "foreground", "#cc0000",
                                            NULL);

        self->priv->sender_tag =
                gtk_text_buffer_create_tag (self->priv->buffer, NULL,
                                            "foreground", "#cc0000",
                                            "weight", PANGO_WEIGHT_BOLD,
                                            NULL);

        return self;
}

void
gibbon_shouts_append_message (const GibbonShouts *self,
                              const GibbonFIBSMessage *message)
{
        GtkTextBuffer *buffer = self->priv->buffer;
        gint length;
        GtkTextIter start, end;
        struct tm *now;
        GTimeVal timeval;
        gchar *timestamp;
        GtkTextTag *tag;
        gchar *formatted;

        g_return_if_fail (GIBBON_IS_SHOUTS (self));

        length = gtk_text_buffer_get_char_count (buffer);
        tag = self->priv->sender_tag;
        gtk_text_buffer_insert_at_cursor (buffer, message->sender, -1);
        gtk_text_buffer_get_iter_at_offset (buffer, &start, length);
        gtk_text_buffer_get_end_iter (buffer, &end);
        gtk_text_buffer_apply_tag (buffer, tag, &start, &end);

        length = gtk_text_buffer_get_char_count (buffer);
        tag = self->priv->date_tag;
        g_get_current_time (&timeval);
        now = localtime ((time_t *) &timeval.tv_sec);
        timestamp = g_strdup_printf (" (%02d:%02d:%02d) ",
                                     now->tm_hour,
                                     now->tm_min,
                                     now->tm_sec);
        gtk_text_buffer_insert_at_cursor (buffer, timestamp, -1);
        g_free (timestamp);
        gtk_text_buffer_get_iter_at_offset (buffer, &start, length);
        gtk_text_buffer_get_end_iter (buffer, &end);
        gtk_text_buffer_apply_tag (buffer, tag, &start, &end);

        formatted = gibbon_fibs_message_formatted (message);
        gtk_text_buffer_insert_at_cursor (buffer, formatted, -1);
        g_free (formatted);
        gtk_text_buffer_insert_at_cursor (buffer, "\n", -1);

        gtk_text_view_scroll_to_mark (self->priv->text_view,
                gtk_text_buffer_get_insert (buffer),
                0.0, TRUE, 0.5, 1);
}
