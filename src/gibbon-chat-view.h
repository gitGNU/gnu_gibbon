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

#ifndef _GIBBON_CHAT_VIEW_H
# define _GIBBON_CHAT_VIEW_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>

#include "gibbon-app.h"
#include "gibbon-chat.h"

#define GIBBON_TYPE_CHAT_VIEW \
        (gibbon_chat_view_get_type ())
#define GIBBON_CHAT_VIEW(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_CHAT_VIEW, \
                GibbonChatView))
#define GIBBON_CHAT_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_CHAT_VIEW, GibbonChatViewClass))
#define GIBBON_IS_CHAT_VIEW(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_CHAT_VIEW))
#define GIBBON_IS_CHAT_VIEW_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_CHAT_VIEW))
#define GIBBON_CHAT_VIEW_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_CHAT_VIEW, GibbonChatViewClass))

/**
 * GibbonChatView:
 *
 * One instance of a #GibbonChatView.  All properties are private.
 */
typedef struct _GibbonChatView GibbonChatView;
struct _GibbonChatView
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonChatViewPrivate *priv;
};

/**
 * GibbonChatViewClass:
 * @on_activate: Handler for "activate" signal.
 *
 * Class representing the combination of a GtkTextArea and a GtkEntry that
 * can be used for communicating with other FIBSters.
 */
typedef struct _GibbonChatViewClass GibbonChatViewClass;
struct _GibbonChatViewClass
{
        /* <private >*/
        GObjectClass parent_class;

        void (*on_activate) (GibbonChatView *self, GtkEntry *entry);
};

GType gibbon_chat_view_get_type (void) G_GNUC_CONST;

GibbonChatView *gibbon_chat_view_new (GibbonApp *app, const gchar *who,
                                      GibbonChat *chat);

void gibbon_chat_view_set_chat (GibbonChatView *self, GibbonChat *chat);
GibbonChat *gibbon_chat_view_get_chat (const GibbonChatView *self);
void gibbon_chat_view_append_message (const GibbonChatView *self,
                                      const GibbonFIBSMessage *message);

#endif
