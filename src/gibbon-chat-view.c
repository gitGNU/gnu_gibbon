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
 * SECTION:gibbon-chat-view
 * @short_description: Gtk components for FIBS communication.
 *
 * Since: 0.1.1
 *
 * Communicating with other FIBSters, be it the "shouts" room, private
 * chatting via "tell", or the game chat for "say", "kibitz", and "whisper"
 * always involve a GtkEntry for the Gibbon user and a GtkTextView for
 * displaying all messages.
 *
 * The game chatting area is a little different, as it also has controls
 * for selecting between say, kibitz, and whisper.  See #GibbonGameChatView
 * for that.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-chat-view.h"

typedef struct _GibbonChatViewPrivate GibbonChatViewPrivate;
struct _GibbonChatViewPrivate {
        GibbonApp *app;

        gchar *label;
};

#define GIBBON_CHAT_VIEW_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_CHAT_VIEW, GibbonChatViewPrivate))

G_DEFINE_TYPE (GibbonChatView, gibbon_chat_view, G_TYPE_OBJECT)

static void 
gibbon_chat_view_init (GibbonChatView *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_CHAT_VIEW, GibbonChatViewPrivate);

        self->priv->app = NULL;

        self->priv->label = NULL;
}

static void
gibbon_chat_view_finalize (GObject *object)
{
        GibbonChatView *self = GIBBON_CHAT_VIEW (object);

        if (self->priv->label)
                g_free (self->priv->label);
        self->priv->label = NULL;

        G_OBJECT_CLASS (gibbon_chat_view_parent_class)->finalize(object);
}

static void
gibbon_chat_view_class_init (GibbonChatViewClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonChatViewPrivate));

        object_class->finalize = gibbon_chat_view_finalize;
}

/**
 * gibbon_chat_view_new:
 * @dummy: The argument.
 *
 * Creates a new #GibbonChatView.
 *
 * Returns: The newly created #GibbonChatView or %NULL in case of failure.
 */
GibbonChatView *
gibbon_chat_view_new (GibbonApp *app, const gchar *label)
{
        GibbonChatView *self = g_object_new (GIBBON_TYPE_CHAT_VIEW, NULL);

        self->priv->app = app;
        self->priv->label = g_strdup (label);

        return self;
}
