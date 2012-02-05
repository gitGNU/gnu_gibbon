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

#ifndef _GIBBON_CHAT_H
# define _GIBBON_CHAT_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>

#include "gibbon-app.h"
#include "gibbon-fibs-message.h"

#define GIBBON_TYPE_CHAT \
        (gibbon_chat_get_type ())
#define GIBBON_CHAT(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_CHAT, \
                GibbonChat))
#define GIBBON_CHAT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_CHAT, GibbonChatClass))
#define GIBBON_IS_CHAT(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_CHAT))
#define GIBBON_IS_CHAT_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_CHAT))
#define GIBBON_CHAT_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_CHAT, GibbonChatClass))

/**
 * GibbonChat:
 *
 * One instance of a #GibbonChat.  All properties are private.
 */
typedef struct _GibbonChat GibbonChat;
struct _GibbonChat
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonChatPrivate *priv;
};

/**
 * GibbonChatClass:
 *
 * Class representing the communication with another FIBSter or a plurality
 * of other FIBSters.
 */
typedef struct _GibbonChatClass GibbonChatClass;
struct _GibbonChatClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_chat_get_type (void) G_GNUC_CONST;

GibbonChat *gibbon_chat_new (GibbonApp *app, const gchar *me);
GtkTextBuffer *gibbon_chat_get_buffer (const GibbonChat *self);
void gibbon_chat_set_my_name (GibbonChat *self, const gchar *me);
void gibbon_chat_append_message (const GibbonChat *self,
                                 const GibbonFIBSMessage *message);

#endif
