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
 * SECTION:gibbon-chat-view
 * @short_description: Gtk components for FIBS communication.
 *
 * Since: 0.1.0
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
#include "html-entities.h"
#include "gibbon-connection.h"

typedef struct _GibbonChatViewPrivate GibbonChatViewPrivate;
struct _GibbonChatViewPrivate {
        GibbonApp *app;

        GtkTextView *view;
        GtkLabel *tab_label;
        GtkNotebook *notebook;
        GtkEntry *entry;

        GtkWidget *hbox;
        GtkWidget *vbox;

        GibbonChat *chat;

        gchar *who;
        gint page_number;
        guint unread;
};

#define GIBBON_CHAT_VIEW_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_CHAT_VIEW, GibbonChatViewPrivate))

G_DEFINE_TYPE (GibbonChatView, gibbon_chat_view, G_TYPE_OBJECT)

static void gibbon_chat_view_on_activate (GibbonChatView *self,
                                          GtkEntry *entry);
static void gibbon_chat_view_on_page_change (const GibbonChatView *self);
static void gibbon_chat_view_close (GibbonChatView *self);

static void 
gibbon_chat_view_init (GibbonChatView *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_CHAT_VIEW, GibbonChatViewPrivate);

        self->priv->view = NULL;
        self->priv->tab_label = NULL;
        self->priv->notebook = NULL;
        self->priv->entry = NULL;

        self->priv->chat = NULL;

        self->priv->app = NULL;

        self->priv->who = NULL;
        self->priv->page_number = 1;
        self->priv->unread = 0;
}

static void
gibbon_chat_view_finalize (GObject *object)
{
        GibbonChatView *self = GIBBON_CHAT_VIEW (object);

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
gibbon_chat_view_new (GibbonApp *app, const gchar *who, GibbonChat *chat)
{
        GibbonChatView *self = g_object_new (GIBBON_TYPE_CHAT_VIEW, NULL);
        GtkNotebook *notebook;
        GtkWidget *vbox;
        GtkWidget *hbox;
        GtkWidget *scroll;
        GtkWidget *text_view;
        GtkWidget *entry;
        GtkWidget *tab_label;
        GtkWidget *close_button;
        GibbonChatViewClass *klass;
        GtkTextBuffer *buffer;

        g_return_val_if_fail (GIBBON_IS_CHAT (chat), NULL);
        self->priv->chat = chat;
        g_object_ref (chat);

        self->priv->app = app;
        self->priv->who = g_strdup (who);

        notebook = GTK_NOTEBOOK (gibbon_app_find_object (app, "chat-notebook",
                                                         GTK_TYPE_NOTEBOOK));
        self->priv->notebook = notebook;

        vbox = gtk_vbox_new (FALSE, 0);
        scroll = gtk_scrolled_window_new (FALSE, FALSE);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC);
        gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);
        buffer = gibbon_chat_get_buffer (chat);
        text_view = gtk_text_view_new_with_buffer (buffer);
        self->priv->view = GTK_TEXT_VIEW (text_view);
        gtk_text_view_set_editable (self->priv->view, FALSE);
        gtk_text_view_set_wrap_mode (self->priv->view, GTK_WRAP_WORD);
        gtk_text_view_set_cursor_visible (self->priv->view, FALSE);

        gtk_container_add (GTK_CONTAINER (scroll), text_view);
        entry = gtk_entry_new ();
        self->priv->entry = GTK_ENTRY (entry);
        gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, TRUE, 0);
        gtk_widget_show_all (vbox);

        hbox = gtk_hbox_new (FALSE, 0);

        tab_label = gtk_label_new (who);
        self->priv->tab_label = GTK_LABEL (tab_label);
        gtk_widget_show_all (tab_label);

        gtk_box_pack_start (GTK_BOX (hbox), tab_label, FALSE, FALSE, 0);

        close_button = gtk_button_new ();
        gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
        gtk_button_set_focus_on_click (GTK_BUTTON (close_button), FALSE);
        gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
        gtk_widget_set_tooltip_text (close_button, _("Close tab"));
        g_signal_connect_swapped (G_OBJECT (close_button), "clicked",
                                  G_CALLBACK (gibbon_chat_view_close),
                                  self);

        gtk_container_add (GTK_CONTAINER (close_button),
                           gtk_image_new_from_stock (GTK_STOCK_CLOSE,
                                                     GTK_ICON_SIZE_MENU));
        gtk_box_pack_end (GTK_BOX (hbox), close_button, FALSE, FALSE, 0);

        gtk_widget_show_all (hbox);

        self->priv->page_number = gtk_notebook_get_n_pages (notebook);

        gtk_notebook_append_page (notebook, vbox, hbox);
        gtk_notebook_set_current_page (notebook, self->priv->page_number);
        gtk_widget_grab_focus (GTK_WIDGET (entry));

        self->priv->vbox = vbox;
        self->priv->hbox = hbox;

        klass = GIBBON_CHAT_VIEW_GET_CLASS (self);

        g_signal_connect_swapped (G_OBJECT (entry), "activate",
                                  G_CALLBACK (klass->on_activate),
                                  G_OBJECT (self));
        g_signal_connect_swapped (G_OBJECT (text_view), "map",
                                  G_CALLBACK (gibbon_chat_view_on_page_change),
                                  G_OBJECT (self));

        return self;
}

void
gibbon_chat_view_set_chat (GibbonChatView *self, GibbonChat *chat)
{
        GtkTextBuffer *buffer;

        g_return_if_fail (GIBBON_IS_CHAT_VIEW (self));
        g_return_if_fail (GIBBON_IS_CHAT (chat));

        if (self->priv->chat)
                g_object_unref (self->priv->chat);
        self->priv->chat = chat;
        g_object_ref (chat);

        buffer = gibbon_chat_get_buffer (chat);
        gtk_text_view_set_buffer (self->priv->view, buffer);
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

GibbonChat *
gibbon_chat_view_get_chat (const GibbonChatView *self)
{
        g_return_val_if_fail (GIBBON_IS_CHAT_VIEW (self), NULL);

        return self->priv->chat;
}

void
gibbon_chat_view_append_message (const GibbonChatView *self,
                                 const GibbonFIBSMessage *message)
{
        GtkTextBuffer *buffer;
        gint page_number;
        gchar *markup;

        g_return_if_fail (GIBBON_IS_CHAT_VIEW (self));
        g_return_if_fail (message != NULL);
        g_return_if_fail (message->sender != NULL);
        g_return_if_fail (message->message != NULL);

        gibbon_chat_append_message (self->priv->chat, message);

        buffer = gibbon_chat_get_buffer (self->priv->chat);

        gtk_text_view_scroll_to_mark (self->priv->view,
                gtk_text_buffer_get_insert (buffer),
                0.0, TRUE, 0.5, 1);

        page_number = gtk_notebook_get_current_page (self->priv->notebook);
        if (2 == gtk_notebook_get_n_pages (self->priv->notebook)) {
                gtk_notebook_set_current_page (self->priv->notebook,
                                               self->priv->page_number);
                if (!gtk_widget_has_focus (GTK_WIDGET (self->priv->entry)))
                        gtk_widget_grab_focus (GTK_WIDGET (self->priv->entry));
        } else if (page_number != self->priv->page_number) {
                ++self->priv->unread;
                markup = g_markup_printf_escaped ("<span weight=\"bold\""
                                                  " color=\"#204a87\">"
                                                  "%s (%u)</span>",
                                                  self->priv->who,
                                                  self->priv->unread);
                gtk_label_set_markup (self->priv->tab_label, markup);
                g_free (markup);
        }
}

static void
gibbon_chat_view_on_page_change (const GibbonChatView *self)
{
        g_return_if_fail (GIBBON_IS_CHAT_VIEW (self));

        self->priv->unread = 0;
        gtk_label_set_text (self->priv->tab_label, self->priv->who);
}

static void
gibbon_chat_view_close (GibbonChatView *self)
{
        GtkWidget *main_window;
        GtkWidget *dialog;
        gint result;

        g_return_if_fail (GIBBON_IS_CHAT_VIEW (self));

        if (self->priv->unread) {
                main_window = gibbon_app_get_window (self->priv->app);
                dialog = gtk_message_dialog_new (GTK_WINDOW (main_window),
                                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                                 GTK_MESSAGE_QUESTION,
                                                 GTK_BUTTONS_YES_NO,
                                                 "%s",
                                                 _("Really close?"));
                result = gtk_dialog_run (GTK_DIALOG (dialog));
                gtk_widget_destroy (dialog);
                if (GTK_RESPONSE_YES != result)
                        return;
        }

        gibbon_app_close_chat (self->priv->app, self->priv->who);

        /*
         * Removing the page from the notebook will destroy this object.
         * It would maybe be cleaner to let self->priv->app do all this.
         */
        gtk_notebook_remove_page (self->priv->notebook,
                                  self->priv->page_number);
}
