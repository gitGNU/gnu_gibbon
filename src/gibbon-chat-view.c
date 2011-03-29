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
#include "gibbon-chat.h"
#include "gibbon-signal.h"
#include "html-entities.h"
#include "gibbon-connection.h"

typedef struct _GibbonChatViewPrivate GibbonChatViewPrivate;
struct _GibbonChatViewPrivate {
        GibbonApp *app;

        GibbonChat *chat;

        gchar *who;
        gint page_number;

        GibbonSignal *activate_handler;
};

#define GIBBON_CHAT_VIEW_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_CHAT_VIEW, GibbonChatViewPrivate))

G_DEFINE_TYPE (GibbonChatView, gibbon_chat_view, G_TYPE_OBJECT)

static void gibbon_chat_view_on_activate (GibbonChatView *self,
                                          GtkEntry *entry);

static void 
gibbon_chat_view_init (GibbonChatView *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_CHAT_VIEW, GibbonChatViewPrivate);

        self->priv->chat = NULL;

        self->priv->app = NULL;

        self->priv->who = NULL;
        self->priv->page_number = 1;
}

static void
gibbon_chat_view_finalize (GObject *object)
{
        GibbonChatView *self = GIBBON_CHAT_VIEW (object);

        if (self->priv->activate_handler)
                g_object_unref (self->priv->activate_handler);
        self->priv->activate_handler = NULL;

        if (self->priv->chat)
                g_object_unref (self->priv->chat);
        self->priv->chat = NULL;

        if (self->priv->who)
                g_free (self->priv->who);
        self->priv->who = NULL;

        G_OBJECT_CLASS (gibbon_chat_view_parent_class)->finalize(object);
}

static void
gibbon_chat_view_class_init (GibbonChatViewClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        klass->on_activate = gibbon_chat_view_on_activate;

        g_type_class_add_private (klass, sizeof (GibbonChatViewPrivate));

        object_class->finalize = gibbon_chat_view_finalize;
}

/**
 * gibbon_chat_view_new:
 * @app: The #GibbonApp.
 * @who: Name of the other FIBSter.
 *
 * Creates a new #GibbonChatView.
 *
 * Returns: The newly created #GibbonChatView or %NULL in case of failure.
 */
GibbonChatView *
gibbon_chat_view_new (GibbonApp *app, const gchar *who)
{
        GibbonChatView *self = g_object_new (GIBBON_TYPE_CHAT_VIEW, NULL);
        GtkNotebook *notebook;
        GtkWidget *vbox;
        GtkWidget *scroll;
        GtkWidget *text_view;
        GtkWidget *entry;
        GtkWidget *tab_label;
        GibbonChatViewClass *klass;

        self->priv->app = app;
        self->priv->who = g_strdup (who);

        notebook = GTK_NOTEBOOK (gibbon_app_find_object (app, "chat-notebook",
                                                         GTK_TYPE_NOTEBOOK));

        vbox = gtk_vbox_new (FALSE, 0);
        scroll = gtk_scrolled_window_new (FALSE, FALSE);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC);
        gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);
        text_view = gtk_text_view_new ();
        gtk_text_view_set_editable (GTK_TEXT_VIEW (text_view), FALSE);
        gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view), GTK_WRAP_WORD);
        gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (text_view), FALSE);

        gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scroll),
                                               text_view);
        entry = gtk_entry_new ();
        gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, TRUE, 0);
        gtk_widget_show_all (vbox);

        tab_label = gtk_label_new (who);
        gtk_widget_show_all (tab_label);

        self->priv->page_number = gtk_notebook_get_n_pages (notebook);

        gtk_notebook_append_page (notebook, vbox, tab_label);
        gtk_notebook_set_current_page (notebook, self->priv->page_number);
        gtk_widget_grab_focus (GTK_WIDGET (entry));

        klass = GIBBON_CHAT_VIEW_GET_CLASS (self);

        self->priv->activate_handler =
                gibbon_signal_new (G_OBJECT (entry), "activate",
                                   G_CALLBACK (klass->on_activate),
                                   G_OBJECT (self));

        return self;
}

void
gibbon_chat_view_set_chat (GibbonChatView *self, GibbonChat *chat)
{
        g_return_if_fail (GIBBON_IS_CHAT_VIEW (self));
        g_return_if_fail (GIBBON_IS_CHAT (chat));

        if (self->priv->chat)
                g_object_unref (self->priv->chat);
        self->priv->chat = chat;
        g_object_ref (chat);
}

static void
gibbon_chat_view_on_activate (GibbonChatView *self, GtkEntry *entry)
{
        gchar *trimmed;
        gchar *formatted;
        GibbonConnection *connection;

        g_return_if_fail (GIBBON_IS_CHAT_VIEW (self));
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
                                         "tellx %s %s",
                                         self->priv->who, formatted);
        g_free (formatted);

        gtk_entry_set_text (entry, "");
}
