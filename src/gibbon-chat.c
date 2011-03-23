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
 * SECTION:gibbon-chat
 * @short_description: Communication with another FIBSter or a plurarility
 *                     of FIBSters.
 *
 * Since: 0.1.1
 *
 * The model for a #GibbonChatView.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-chat.h"
#include "gibbon-fibs-message.h"

typedef struct _GibbonChatPrivate GibbonChatPrivate;
struct _GibbonChatPrivate {
        GibbonApp *app;
        GtkTextBuffer *buffer;
        gchar *user;

        GtkTextTag *sender_tag;
        GtkTextTag *date_tag;

        GtkTextTag *sender_gat;
        GtkTextTag *date_gat;
};

#define GIBBON_CHAT_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_CHAT, GibbonChatPrivate))

G_DEFINE_TYPE (GibbonChat, gibbon_chat, G_TYPE_OBJECT)

static void 
gibbon_chat_init (GibbonChat *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_CHAT, GibbonChatPrivate);

        self->priv->app = NULL;
        self->priv->buffer = NULL;

        self->priv->user = NULL;

        self->priv->sender_tag = NULL;
        self->priv->date_tag = NULL;

        self->priv->sender_gat = NULL;
        self->priv->date_gat = NULL;
}

static void
gibbon_chat_finalize (GObject *object)
{
        GibbonChat *self = GIBBON_CHAT (object);

        if (self->priv->buffer)
                g_object_unref (self->priv->buffer);
        self->priv->buffer = NULL;

        self->priv->app = NULL;

        if (self->priv->user)
                g_free (self->priv->user);
        self->priv->user = NULL;

        if (self->priv->sender_tag)
                g_object_unref (self->priv->sender_tag);
        self->priv->sender_tag = NULL;

        if (self->priv->date_tag)
                g_object_unref (self->priv->date_tag);
        self->priv->date_tag = NULL;

        if (self->priv->sender_gat)
                g_object_unref (self->priv->sender_gat);
        self->priv->sender_gat = NULL;

        if (self->priv->date_gat)
                g_object_unref (self->priv->date_gat);
        self->priv->date_gat = NULL;

        G_OBJECT_CLASS (gibbon_chat_parent_class)->finalize(object);
}

static void
gibbon_chat_class_init (GibbonChatClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonChatPrivate));

        object_class->finalize = gibbon_chat_finalize;
}

/**
 * gibbon_chat_new:
 * @app: The #GibbonApp.
 * @buffer: The #GtkTextBuffer for saving the communication.
 * @user: Name of the Gibbon user.
 *
 * Creates a new #GibbonChat.
 *
 * Returns: The newly created #GibbonChat or %NULL in case of failure.
 */
GibbonChat *
gibbon_chat_new (GibbonApp *app, GtkTextBuffer *buffer,
                 const gchar *user)
{
        GibbonChat *self = g_object_new (GIBBON_TYPE_CHAT, NULL);

        self->priv->app = app;
        self->priv->buffer = buffer;
        g_object_ref (buffer);
        self->priv->user = g_strdup (user);

        self->priv->date_tag =
                gtk_text_buffer_create_tag (self->priv->buffer, NULL,
                                            "foreground", "#cc0000",
                                            NULL);

        self->priv->sender_tag =
                gtk_text_buffer_create_tag (self->priv->buffer, NULL,
                                            "foreground", "#cc0000",
                                            "weight", PANGO_WEIGHT_BOLD,
                                            NULL);

        self->priv->date_gat =
                gtk_text_buffer_create_tag (self->priv->buffer, NULL,
                                            "foreground", "#204a87",
                                            NULL);

        self->priv->sender_gat =
                gtk_text_buffer_create_tag (self->priv->buffer, NULL,
                                            "foreground", "#204a87",
                                            "weight", PANGO_WEIGHT_BOLD,
                                            NULL);

        return self;
}

void
gibbon_chat_append_message (const GibbonChat *self,
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

        g_return_if_fail (GIBBON_IS_CHAT (self));

        length = gtk_text_buffer_get_char_count (buffer);
        if (g_strcmp0 (message->sender, self->priv->user))
                tag = self->priv->sender_tag;
        else
                tag = self->priv->sender_gat;
        gtk_text_buffer_insert_at_cursor (buffer, message->sender, -1);
        gtk_text_buffer_get_iter_at_offset (buffer, &start, length);
        gtk_text_buffer_get_end_iter (buffer, &end);
        gtk_text_buffer_apply_tag (buffer, tag, &start, &end);

        length = gtk_text_buffer_get_char_count (buffer);
        if (g_strcmp0 (message->sender, self->priv->user))
                tag = self->priv->date_tag;
        else
                tag = self->priv->date_gat;
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
}
