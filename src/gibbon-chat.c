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

typedef struct _GibbonChatPrivate GibbonChatPrivate;
struct _GibbonChatPrivate {
        GibbonApp *app;
        GtkTextBuffer *buffer;
        gchar *user;
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

        if (self->priv->user)
                g_free (self->priv->user);
        self->priv->user = NULL;
}

static void
gibbon_chat_finalize (GObject *object)
{
        GibbonChat *self = GIBBON_CHAT (object);

        if (self->priv->buffer)
                g_object_unref (self->priv->buffer);
        self->priv->buffer = NULL;

        self->priv->app = NULL;

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

        return self;
}
