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

#ifndef _GIBBON_GAME_CHAT_H
# define _GIBBON_GAME_CHAT_H

#include <glib.h>
#include <gtk/gtk.h>

#include "gibbon-app.h"
#include "gibbon-fibs-message.h"

#define GIBBON_TYPE_GAME_CHAT \
        (gibbon_game_chat_get_type ())
#define GIBBON_GAME_CHAT(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_GAME_CHAT, \
                GibbonGameChat))
#define GIBBON_GAME_CHAT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_GAME_CHAT, GibbonGameChatClass))
#define GIBBON_IS_GAME_CHAT(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_GAME_CHAT))
#define GIBBON_IS_GAME_CHAT_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_GAME_CHAT))
#define GIBBON_GAME_CHAT_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_GAME_CHAT, GibbonGameChatClass))

/**
 * GibbonGameChat:
 *
 * One instance of a #GibbonGameChat.  All properties are private.
 **/
typedef struct _GibbonGameChat GibbonGameChat;
struct _GibbonGameChat
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonGameChatPrivate *priv;
};

/**
 * GibbonGameChatClass:
 *
 * Handle say, kibitz, and whisper.
 **/
typedef struct _GibbonGameChatClass GibbonGameChatClass;
struct _GibbonGameChatClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_game_chat_get_type (void) G_GNUC_CONST;

GibbonGameChat *gibbon_game_chat_new (GibbonApp *app);
void gibbon_game_chat_set_my_name (GibbonGameChat *self, const gchar *me);
void gibbon_game_chat_append_message (const GibbonGameChat *self,
                                      const GibbonFIBSMessage *message);

#endif
