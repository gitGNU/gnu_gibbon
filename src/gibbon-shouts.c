/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
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
 * Since: 0.1.0
 *
 * Handling of FIBS shouts.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-shouts.h"
#include "gibbon-connection.h"
#include "gibbon-chat.h"
#include "html-entities.h"

typedef struct _GibbonShoutsPrivate GibbonShoutsPrivate;
struct _GibbonShoutsPrivate {
        GibbonApp *app;

        GtkTextView *view;
        GtkTextBuffer *buffer;
        GibbonChat *chat;
};

#define GIBBON_SHOUTS_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_SHOUTS, GibbonShoutsPrivate))

G_DEFINE_TYPE (GibbonShouts, gibbon_shouts, G_TYPE_OBJECT)

static void gibbon_shouts_on_shout (const GibbonShouts *shouts,
                                    GtkEntry *entry);

static void 
gibbon_shouts_init (GibbonShouts *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_SHOUTS, GibbonShoutsPrivate);

        self->priv->app = NULL;
        self->priv->view = NULL;
        self->priv->chat = NULL;
}

static void
gibbon_shouts_finalize (GObject *object)
{
        GibbonShouts *self = GIBBON_SHOUTS (object);

        if (self->priv->chat)
                g_object_unref (self->priv->chat);

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
        GtkTextBuffer *buffer;
        GObject *entry;

        self->priv->app = app;

        self->priv->view =
                GTK_TEXT_VIEW (gibbon_app_find_object (app,
                                                       "shout-text-view",
                                                       GTK_TYPE_TEXT_VIEW));

        self->priv->chat = gibbon_chat_new (app, NULL);

        buffer = gibbon_chat_get_buffer (self->priv->chat);
        gtk_text_view_set_buffer (self->priv->view, buffer);

        entry = gibbon_app_find_object (app, "shout-entry", GTK_TYPE_ENTRY);
        g_signal_connect_swapped (entry, "activate",
                                  G_CALLBACK (gibbon_shouts_on_shout),
                                  G_OBJECT (self));

        return self;
}

void
gibbon_shouts_append_message (const GibbonShouts *self,
                              const GibbonFIBSMessage *message)
{
        GtkTextBuffer *buffer;

        g_return_if_fail (GIBBON_IS_SHOUTS (self));
        g_return_if_fail (message != NULL);
        g_return_if_fail (message->sender != NULL);
        g_return_if_fail (message->message != NULL);

        gibbon_chat_append_message (self->priv->chat, message);

        buffer = gibbon_chat_get_buffer (self->priv->chat);

        gtk_text_view_scroll_to_mark (self->priv->view,
                gtk_text_buffer_get_insert (buffer),
                0.0, TRUE, 0.5, 1);
}

void
gibbon_shouts_set_my_name (GibbonShouts *self, const gchar *me)
{
        g_return_if_fail (GIBBON_IS_SHOUTS (self));

        gibbon_chat_set_my_name (self->priv->chat, me);
}

static void
gibbon_shouts_on_shout (const GibbonShouts *self, GtkEntry *entry)
{
        GibbonConnection *connection;
        gchar *trimmed;
        gchar *formatted;

        g_return_if_fail (GIBBON_IS_SHOUTS (self));
        g_return_if_fail (GTK_IS_ENTRY (entry));

        connection = gibbon_app_get_connection (self->priv->app);
        if (!connection)
                return;

        trimmed = pango_trim_string (gtk_entry_get_text (entry));
        if (!*trimmed) {
                g_free (trimmed);
                return;
        }
        formatted = encode_html_entities (trimmed);
        g_free (trimmed);
        gibbon_connection_queue_command (connection, FALSE,
                                         "shout %s", formatted);
        g_free (formatted);

        gtk_entry_set_text (entry, "");
}
