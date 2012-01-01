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

/**
 * SECTION:gibbon-game-chat
 * @short_description: GibbonGameChat handles say, whisper, and kibitz.
 *
 * Since: 0.1.0
 *
 * Handling of the in-game chat functionality.  That is the commands
 * say, kibitz, and whisper.
 **/

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "gibbon-app.h"
#include "gibbon-chat.h"
#include "gibbon-game-chat.h"
#include "gibbon-connection.h"
#include "html-entities.h"

enum GibbonGameChatMode {
        GIBBON_GAME_CHAT_MODE_SAY = 0,
        GIBBON_GAME_CHAT_MODE_KIBITZ = 1,
        GIBBON_GAME_CHAT_MODE_WHISPER = 2,
};

typedef struct _GibbonGameChatPrivate GibbonGameChatPrivate;
struct _GibbonGameChatPrivate {
        const GibbonApp *app;

        GtkComboBox *combo;

        GtkToggleToolButton *toggle_say;
        GtkToggleToolButton *toggle_whisper;

        enum GibbonGameChatMode mode;

        GibbonChat *chat;
        GtkTextView *view;
        GtkTextBuffer *buffer;
};

#define GIBBON_GAME_CHAT_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_GAME_CHAT, GibbonGameChatPrivate))

G_DEFINE_TYPE (GibbonGameChat, gibbon_game_chat, G_TYPE_OBJECT)

static GibbonGameChat *singleton = NULL;

static gboolean gibbon_game_chat_fixup_combo (GibbonGameChat *self);
static gboolean gibbon_game_chat_fixup_toolbar (GibbonGameChat *self);
static void gibbon_game_chat_synchronize_toolbar (GibbonGameChat *self);

/* Signal handlers. */
static void gibbon_game_chat_on_combo_change (GibbonGameChat *self,
                                              GtkComboBox *combo);
static void gibbon_game_chat_on_tool_button_toggle (GibbonGameChat *self,
                                                    GtkToggleToolButton *btn);
static void gibbon_game_chat_on_utter (const GibbonGameChat *self,
                                       GtkEntry *entry);

static void 
gibbon_game_chat_init (GibbonGameChat *self)
{        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_GAME_CHAT, GibbonGameChatPrivate);

        self->priv->app = NULL;

        self->priv->combo = NULL;
        self->priv->toggle_say = NULL;
        self->priv->toggle_whisper = NULL;

        self->priv->mode = GIBBON_GAME_CHAT_MODE_SAY;

        self->priv->chat = NULL;
        self->priv->view = NULL;
}

static void
gibbon_game_chat_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_game_chat_parent_class)->finalize (object);
}

static void
gibbon_game_chat_class_init (GibbonGameChatClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonGameChatPrivate));

        object_class->finalize = gibbon_game_chat_finalize;
}

GibbonGameChat *
gibbon_game_chat_new (GibbonApp *app)
{
        GibbonGameChat *self;
        GtkTextBuffer *buffer;
        GObject *entry;

        g_return_val_if_fail (!singleton, singleton);

        self = singleton = g_object_new (GIBBON_TYPE_GAME_CHAT, NULL);

        self->priv->app = app;

        if (!gibbon_game_chat_fixup_combo (self)) {
                g_object_unref (self);
                return NULL;
        }

        if (!gibbon_game_chat_fixup_toolbar (self)) {
                g_object_unref (self);
                return NULL;
        }

        gibbon_game_chat_synchronize_toolbar (self);

        self->priv->view =
                GTK_TEXT_VIEW (gibbon_app_find_object (app,
                                                       "game-chat-text-view",
                                                       GTK_TYPE_TEXT_VIEW));

        self->priv->chat = gibbon_chat_new (app, NULL);

        buffer = gibbon_chat_get_buffer (self->priv->chat);
        gtk_text_view_set_buffer (self->priv->view, buffer);

        entry = gibbon_app_find_object (app, "game-chat-entry", GTK_TYPE_ENTRY);
        g_signal_connect_swapped (entry, "activate",
                                  G_CALLBACK (gibbon_game_chat_on_utter),
                                  G_OBJECT (self));

        return self;
}

static gboolean
gibbon_game_chat_fixup_combo (GibbonGameChat *self)
{
        GtkComboBox *combo;
        GtkCellRenderer *cell;
        GtkListStore *store;
        const GibbonApp *app;

        app = self->priv->app;

        combo = GTK_COMBO_BOX (gibbon_app_find_object (app, "combo-game-chat",
                                                       GTK_TYPE_COMBO_BOX));
        if (!combo)
                return FALSE;

        self->priv->combo = combo;

        store = gtk_list_store_new (1, G_TYPE_STRING);
        gtk_combo_box_set_model (combo, GTK_TREE_MODEL (store));
        g_object_unref (store);

        cell = gtk_cell_renderer_text_new ();
        gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell,
                                    TRUE);
        gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo),
                                        cell, "text", 0,
                                        NULL);

        /* TRANSLATORS: This is the FIBS `say' command! */
        gtk_combo_box_append_text (combo, _("Say"));
        /* TRANSLATORS: This is the FIBS `kibitz' command! */
        gtk_combo_box_append_text (combo, _("Kibitz"));
        /* TRANSLATORS: This is the FIBS `whisper' command! */
        gtk_combo_box_append_text (combo, _("Whisper"));

        gtk_combo_box_set_active (combo, 0);

        g_signal_connect_swapped (G_OBJECT (combo), "changed",
                                  G_CALLBACK (gibbon_game_chat_on_combo_change),
                                  self);

        return TRUE;
}

static gboolean
gibbon_game_chat_fixup_toolbar (GibbonGameChat *self)
{
        const GibbonApp *app = self->priv->app;
        const gchar *pixmaps_dir = gibbon_app_get_pixmaps_directory (app);
        GtkToggleToolButton *toggle_say =
                        GTK_TOGGLE_TOOL_BUTTON (gibbon_app_find_object (app,
                                                "game-chat-say-button",
                                                GTK_TYPE_TOGGLE_TOOL_BUTTON));
        GtkToggleToolButton *toggle_whisper =
                        GTK_TOGGLE_TOOL_BUTTON (gibbon_app_find_object (app,
                                                "game-chat-whisper-button",
                                                GTK_TYPE_TOGGLE_TOOL_BUTTON));
        GtkRequisition requisition;
        gchar *icon_path;
        GtkImage *image;

        if (!toggle_say)
                return FALSE;
        if (!toggle_whisper)
                return FALSE;

        self->priv->toggle_say = toggle_say;
        self->priv->toggle_whisper = toggle_whisper;

        gtk_widget_size_request (GTK_WIDGET (toggle_say), &requisition);
        icon_path = g_build_filename (pixmaps_dir, "icons", "say.svg",
                                      NULL);
        image = gibbon_app_load_scaled_image (self->priv->app, icon_path,
                                              requisition.width,
                                              requisition.height);
        if (!image)
                return FALSE;

        gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (toggle_say),
                                         GTK_WIDGET (image));
        g_object_unref (image);

        gtk_widget_size_request (GTK_WIDGET (toggle_say), &requisition);
        icon_path = g_build_filename (pixmaps_dir, "icons", "whisper.svg",
                                      NULL);
        image = gibbon_app_load_scaled_image (self->priv->app, icon_path,
                                              requisition.width,
                                              requisition.height);
        if (!image)
                return FALSE;

        gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (toggle_whisper),
                                         GTK_WIDGET (image));
        g_object_unref (image);

        g_signal_connect_swapped (
                        G_OBJECT (toggle_say), "toggled",
                        G_CALLBACK (gibbon_game_chat_on_tool_button_toggle),
                        self);
        g_signal_connect_swapped (
                        G_OBJECT (toggle_whisper), "toggled",
                        G_CALLBACK (gibbon_game_chat_on_tool_button_toggle),
                        self);

        return TRUE;
}

static void
gibbon_game_chat_synchronize_toolbar (GibbonGameChat *self)
{
        switch (self->priv->mode) {
                case GIBBON_GAME_CHAT_MODE_WHISPER:
                        gtk_toggle_tool_button_set_active (
                                        self->priv->toggle_say, FALSE);
                        gtk_toggle_tool_button_set_active (
                                        self->priv->toggle_whisper, TRUE);
                        break;
                case GIBBON_GAME_CHAT_MODE_KIBITZ:
                        gtk_toggle_tool_button_set_active (
                                        self->priv->toggle_say, TRUE);
                        gtk_toggle_tool_button_set_active (
                                        self->priv->toggle_whisper, TRUE);
                        break;
                default:
                        self->priv->mode = GIBBON_GAME_CHAT_MODE_SAY;
                        gtk_toggle_tool_button_set_active (
                                        self->priv->toggle_say, TRUE);
                        gtk_toggle_tool_button_set_active (
                                        self->priv->toggle_whisper, FALSE);
                        break;
        }
}

/* Signal handlers.  */
static void
gibbon_game_chat_on_combo_change (GibbonGameChat *self, GtkComboBox *combo)
{
        gint new_mode;

        g_return_if_fail (self == singleton);
        g_return_if_fail (GTK_IS_COMBO_BOX (combo));
        g_return_if_fail (combo == self->priv->combo);

        new_mode = gtk_combo_box_get_active (combo);
        if (new_mode == self->priv->mode)
                return;

        self->priv->mode = new_mode;

        gibbon_game_chat_synchronize_toolbar (self);
}

static void
gibbon_game_chat_on_tool_button_toggle (GibbonGameChat *self,
                                        GtkToggleToolButton *button)
{
        gboolean say;
        gboolean whisper;
        enum GibbonGameChatMode mode;

        g_return_if_fail (self == singleton);
        g_return_if_fail (GTK_IS_TOGGLE_TOOL_BUTTON (button));
        g_return_if_fail (button == self->priv->toggle_say
                          || button == self->priv->toggle_whisper);

        say = gtk_toggle_tool_button_get_active (self->priv->toggle_say);
        whisper =
                gtk_toggle_tool_button_get_active (self->priv->toggle_whisper);

        if (say && whisper)
                mode = GIBBON_GAME_CHAT_MODE_KIBITZ;
        else if (!say && whisper)
                mode = GIBBON_GAME_CHAT_MODE_WHISPER;
        else
                mode = GIBBON_GAME_CHAT_MODE_SAY;

        if (mode == self->priv->mode)
                return;

        self->priv->mode = GIBBON_GAME_CHAT_MODE_SAY;
        gtk_combo_box_set_active (self->priv->combo, mode);
}

void
gibbon_game_chat_append_message (const GibbonGameChat *self,
                                 const GibbonFIBSMessage *message)
{
        GtkTextBuffer *buffer;

        g_return_if_fail (GIBBON_IS_GAME_CHAT (self));
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
gibbon_game_chat_set_my_name (GibbonGameChat *self, const gchar *me)
{
        g_return_if_fail (GIBBON_IS_GAME_CHAT (self));

        gibbon_chat_set_my_name (self->priv->chat, me);
}

static void
gibbon_game_chat_on_utter (const GibbonGameChat *self, GtkEntry *entry)
{
        GibbonConnection *connection;
        gchar *trimmed;
        gchar *formatted;

        g_return_if_fail (GIBBON_IS_GAME_CHAT (self));
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

        switch (self->priv->mode) {
                case GIBBON_GAME_CHAT_MODE_SAY:
                        gibbon_connection_queue_command (connection, FALSE,
                                                         "say %s", formatted);
                        break;
                case GIBBON_GAME_CHAT_MODE_KIBITZ:
                        gibbon_connection_queue_command (connection, FALSE,
                                                         "kibitz %s",
                                                         formatted);
                        break;
                case GIBBON_GAME_CHAT_MODE_WHISPER:
                        gibbon_connection_queue_command (connection, FALSE,
                                                         "whisper %s",
                                                         formatted);
                        break;
        }
        g_free (formatted);

        gtk_entry_set_text (entry, "");
}

