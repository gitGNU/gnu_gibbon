/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with gibbon; if not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:gibbon-game-chat
 * @short_description: GibbonGameChat handles say, whisper, and kibitz.
 *
 * Since: 0.1.1
 *
 * Handling of the in-game chat functionality.  That is the commands
 * say, kibitz, and whisper.
 **/

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "gui.h"
#include "gibbon-game-chat.h"

typedef struct _GibbonGameChatPrivate GibbonGameChatPrivate;
struct _GibbonGameChatPrivate {
        GtkBuilder *builder;
        const gchar *pixmaps_dir;

        GtkComboBox *combo;

        GtkToggleToolButton *toggle_say;
        GtkToggleToolButton *toggle_whisper;
};

#define GIBBON_GAME_CHAT_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_GAME_CHAT, GibbonGameChatPrivate))

G_DEFINE_TYPE (GibbonGameChat, gibbon_game_chat, G_TYPE_OBJECT)

static GibbonGameChat *singleton = NULL;

static gboolean gibbon_game_chat_fixup_combo (GibbonGameChat *self);
static gboolean gibbon_game_chat_fixup_toolbar (GibbonGameChat *self);
/* Signal handlers. */
static void gibbon_game_chat_on_combo_change (GibbonGameChat *self,
                                              GtkComboBox *combo);
static void gibbon_game_chat_on_tool_button_toggle (GibbonGameChat *self,
                                                    GtkToggleToolButton *btn);

static void 
gibbon_game_chat_init (GibbonGameChat *self)
{        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_GAME_CHAT, GibbonGameChatPrivate);

        self->priv->builder = NULL;
        self->priv->pixmaps_dir = NULL;
        self->priv->combo = NULL;
        self->priv->toggle_say = NULL;
        self->priv->toggle_whisper = NULL;
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
gibbon_game_chat_new (GtkBuilder *builder, const gchar *pixmaps_dir)
{
        GibbonGameChat *self;

        g_return_val_if_fail (!singleton, singleton);

        self = singleton = g_object_new (GIBBON_TYPE_GAME_CHAT, NULL);

        self->priv->builder = builder;
        self->priv->pixmaps_dir = pixmaps_dir;

        if (!gibbon_game_chat_fixup_combo (self)) {
                g_object_unref (self);
                return NULL;
        }

        if (!gibbon_game_chat_fixup_toolbar (self)) {
                g_object_unref (self);
                return NULL;
        }

        return self;
}

static gboolean
gibbon_game_chat_fixup_combo (GibbonGameChat *self)
{
        GtkComboBox *combo;
        GtkCellRenderer *cell;
        GtkListStore *store;
        GtkBuilder *builder;

        builder = self->priv->builder;

        combo = GTK_COMBO_BOX (find_object (builder, "combo-game-chat",
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
_gibbon_game_chat_fixup_toolbar (GibbonGameChat *self)
{
        const gchar *pixmaps_dir = self->priv->pixmaps_dir;
        GtkBuilder *builder = self->priv->builder;
        GtkToggleToolButton *say_button =
                        GTK_TOGGLE_TOOL_BUTTON (find_object (builder,
                                                "game-chat-say-button",
                                                GTK_TYPE_TOGGLE_TOOL_BUTTON));
        GtkToggleToolButton *whisper_button =
                        GTK_TOGGLE_TOOL_BUTTON (find_object (builder,
                                                "game-chat-whisper-button",
                                                GTK_TYPE_TOGGLE_TOOL_BUTTON));
        GtkRequisition requisition;
        gchar *icon_path;
        GtkImage *image;

        if (!say_button)
                return FALSE;
        if (!whisper_button)
                return FALSE;

        gtk_widget_size_request (GTK_WIDGET (say_button), &requisition);
        icon_path = g_build_filename (pixmaps_dir, "icons", "say.png",
                                      NULL);
        image = load_scaled_image (icon_path,
                                   requisition.width,
                                   requisition.height);
        if (!image)
                return FALSE;
        gtk_tool_button_set_icon_widget (say_button, GTK_WIDGET (image));

        return TRUE;
}

static gboolean
gibbon_game_chat_fixup_toolbar (GibbonGameChat *self)
{
        const gchar *pixmaps_dir = self->priv->pixmaps_dir;
        GtkBuilder *builder = self->priv->builder;
        GtkImage *say_icon = GTK_IMAGE (gtk_builder_get_object (builder,
                                                                "say-icon"));
        GtkToolButton *say_button =
                GTK_TOOL_BUTTON (gtk_builder_get_object (builder,
                                                         "game-chat-say-button"));
        GtkImage *whisper_icon = GTK_IMAGE (gtk_builder_get_object (builder,
                                                                "whisper-icon"));
        GtkToolButton *whisper_button =
                GTK_TOOL_BUTTON (gtk_builder_get_object (builder,
                                                         "game-chat-whisper-button"));
        GtkRequisition requisition;
        gchar *icon_path;
        GdkPixbuf *pixbuf;

        if (say_icon && say_button) {
                gtk_widget_size_request (GTK_WIDGET (say_button), &requisition);
                icon_path = g_build_filename (pixmaps_dir, "icons", "say.svg",
                                              NULL);
                pixbuf = gdk_pixbuf_new_from_file_at_scale (icon_path,
                                                            requisition.width,
                                                            requisition.height,
                                                            FALSE,
                                                            NULL);
                if (pixbuf) {
                        gtk_image_set_from_pixbuf (say_icon, pixbuf);
                        gtk_tool_button_set_icon_widget (say_button,
                                                         GTK_WIDGET (say_icon));
                } else {
                        display_error (_("Failed to load image `%s'!"),
                                       icon_path);
                        g_free (icon_path);
                        return FALSE;
                }
                g_free (icon_path);
        } else {
                display_error (_("Icon `%s' not found in UI definition!"),
                               "say-icon");
                return FALSE;
        }

        if (whisper_icon && whisper_button) {
                gtk_widget_size_request (GTK_WIDGET (whisper_button),
                                         &requisition);
                icon_path = g_build_filename (pixmaps_dir, "icons",
                                              "whisper.svg",
                                              NULL);
                pixbuf = gdk_pixbuf_new_from_file_at_scale (icon_path,
                                                            requisition.width,
                                                            requisition.height,
                                                            FALSE,
                                                            NULL);
                if (pixbuf) {
                        gtk_image_set_from_pixbuf (whisper_icon, pixbuf);
                        gtk_tool_button_set_icon_widget (whisper_button,
                                                         GTK_WIDGET (whisper_icon));
                } else {
                        display_error (_("Failed to load image `%s'!"),
                                       icon_path);
                        g_free (icon_path);
                        return FALSE;
                }
                g_free (icon_path);
        } else {
                display_error (_("Icon `%s' not found in UI definition!"),
                               "say-icon");
                return FALSE;
        }

        return TRUE;
}

/* Signal handlers.  */
static void
gibbon_game_chat_on_combo_change (GibbonGameChat *self, GtkComboBox *combo)
{
        g_return_if_fail (self == singleton);
        g_return_if_fail (GTK_IS_COMBO_BOX (combo));
}

static void
gibbon_game_chat_on_tool_button_toggle (GibbonGameChat *self,
                                        GtkToggleToolButton *button)
{
        g_return_if_fail (self == singleton);
        g_return_if_fail (GTK_IS_TOGGLE_TOOL_BUTTON (button));
}

